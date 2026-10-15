
#include "st_batt_sampler.h"

// #define TRACE	1 ? (void)0 : (void)printf //printf("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), printf

static BYTE AsciiToHex(BYTE Input)
{
	if (Input >= '0' && Input <= '9')
	{
		Input -= '0';
	}
	else if (Input >= 'A' && Input <= 'F')
	{
		Input -= 'A';
		Input += 10;
	}
	else if (Input >= 'a' && Input <= 'f')
	{
		Input -= 'a';
		Input += 10;
	}
	return Input;
}

static BYTE Ascii2ToHex(BYTE *pInput)
{
	// TRACE("pInput[0]=[%02X]pInput[1]=%02X\n", pInput[0], pInput[1]);

	return ((AsciiToHex(pInput[0])) << 4) + AsciiToHex(pInput[1]);
}

static void Init_UpgradeBMSParam(IN STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	UINT n = 0;

	pSTBatt_Rough->stUpgradeParam.nLast_StartMsgId = 0;
	pSTBatt_Rough->stUpgradeParam.nLocalSendMsgId = 0;

	for (n = 0; n < pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt; n++)
	{
		pSTBatt_Rough->stUpgradeParam.nRemtRunModel[n] = 0;
		pSTBatt_Rough->stUpgradeParam.nRemtDownIndex[n] = 0;
		pSTBatt_Rough->stUpgradeParam.emRemtReptStat[n] = Stat_Other;
	}

	return;
}

void CalSeed(BYTE bySrcSeed[], BYTE byDstData[])
{
	BYTE byXorArray[4]={0x50, 0x44, 0x4D, 0x20}; 
	BYTE byCalData[4];
	int i = 0;
	for (i = 0; i < 4; ++i)
	{
		byCalData[i] = bySrcSeed[i] ^ byXorArray[i % 4];
	}

	unsigned int uTemp = 0;
	uTemp = ((byCalData[2] & 0x03) << 6 ) | ((byCalData[3] & 0xfc) >> 2);
   	uTemp = (uTemp << 8) + ((byCalData[3] & 0x03) << 6) | ((byCalData[0] & 0x3f));
   	uTemp = (uTemp << 8) + ((byCalData[0] & 0xfc)) | ((byCalData[1] & 0xc0) >> 6);
   	uTemp = (uTemp << 8) + ((byCalData[1] & 0xfc)) | ((byCalData[2] & 0x03));

	byDstData[0] = (uTemp >> 24) & 0xff;
	byDstData[1] = (uTemp >> 16) & 0xff;
	byDstData[2] = (uTemp >> 8) & 0xff;
	byDstData[3] = uTemp & 0xff;
	
	return;
}


static UDS_RESPOND getBMSReplyStatus(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, BYTE bySID)
{
	int n = 0;
	BOOL bRcvRespone = FALSE;
	
	for (n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if (pSTBatt_Rough->stUpgradeParam.byData[n][0] != 0)
		{
			bRcvRespone = TRUE;
			break;
		}
	}

	if (!bRcvRespone)
		return SERVICE_NOT_RESPOND;

	BYTE byFrameType = pSTBatt_Rough->stUpgradeParam.byData[n][0] >> 4;

	if (UDS_FRAME_TYPE_SINGLE == byFrameType) // reply single frame
	{
		BYTE* pData = &pSTBatt_Rough->stUpgradeParam.byData[n][0];

		if (pData[1] == (bySID + UDS_REPLY_POSITIVE))
		{
			return SERVICE_POSITIVE_OK;
		}
		else if (bySID == UDS_SERVICE_ROUNTINE_CONTROL_31 || bySID == UDS_SERVICE_DOWNLOAD_36) 
		{
			if (UDS_REPLY_NEGATIVE == pData[1] &&  UDS_REPLY_PENDING == pData[3]) //Server busy, waiting for the next response frame.
			{
				return SERVICE_NEGATIVE_BUSY;
			}
				
		}
		
		return SERVICE_NEGATIVE_FAILED;
	}
	else if (UDS_FRAME_TYPE_FLOW_CONTROL == byFrameType) // reply stream control frame
	{
		BYTE byFrameStatus = pSTBatt_Rough->stUpgradeParam.byFrameStatus = pSTBatt_Rough->stUpgradeParam.byData[n][0] & 0x0f;

		if (byFrameStatus >= 0 && byFrameStatus <= 1)
		{
			pSTBatt_Rough->stUpgradeParam.byDataWaiteRespone = pSTBatt_Rough->stUpgradeParam.byData[n][1];
			pSTBatt_Rough->stUpgradeParam.byFrameInterval = pSTBatt_Rough->stUpgradeParam.byData[n][2];
			
			return SERVICE_POSITIVE_OK;
		}
	}
	
	return SERVICE_NEGATIVE_FAILED;
}

static BOOL getBMSReplyResult(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, BYTE bySID)
{
	time_t timeBegin = _NOW_;

	for (; ; )
	{
		if (ABS(_NOW_ - timeBegin) > UPGRADE_UDS_RECEIVE_TIMEOUT)
		{
			return FALSE;
		}

		YFY_ReadAndParseCanData(pSTBatt_Rough);

		GetBMSUpgradeStatus(pSTBatt_Rough);

		UDS_RESPOND repond = getBMSReplyStatus(pSTBatt_Rough, bySID);
		if (SERVICE_POSITIVE_OK == repond)
		{
			return TRUE;
		}
		else if (SERVICE_NEGATIVE_FAILED == repond)
		{
			return FALSE;
		}
		else if (SERVICE_NEGATIVE_BUSY == repond || SERVICE_NOT_RESPOND == repond)
		{
			Sleep(30);
			continue;
		}
		Sleep(30);
	}
	
	return FALSE;
}

static BOOL Send_C2B_UpgradeUDSLongData(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, STUDS_UPGRADE_PARAMETER* pParam, BYTE byBuf[], int iBufLen)
{
	int i = 0;
	BATT_UPGRADE_PARAM *pUpgradeParam = &pSTBatt_Rough->stUpgradeParam;

	STUDS_UPGRADE_PARAMETER* pUDSPara = pParam;

	if (pUDSPara->bySubServiceLen > 4)
		return FALSE;

	A_CAN_ND_BATT_FRAME frmSent;
	memset(frmSent.byData, 0xaa, sizeof(frmSent.byData));

	frmSent.uiFrmID = UPGRADE_FRAME_ID_UDS;
	frmSent.uiFrmID |= (pUpgradeParam->eDevType << 8);
	frmSent.uiFrmID |= pUpgradeParam->byDevAddr;
	
	frmSent.byData[0] = 0x10;

	unsigned short uLen = iBufLen + pUDSPara->bySubServiceLen + 1;
	frmSent.byData[0] |=  ((uLen >> 8) & 0x0f);
	frmSent.byData[1] = (uLen & 0xff);
	
	frmSent.byData[2] = pUDSPara->byServiceID;

	memcpy(&frmSent.byData[3], pUDSPara->bySubServiceID, pUDSPara->bySubServiceLen);

	int iRes = CAN_DATA_AREA_LEN - 3 - pUDSPara->bySubServiceLen;
	memcpy(&frmSent.byData[pUDSPara->bySubServiceLen + 3], byBuf, iRes);
			
	frmSent.uiDataLen = CAN_DATA_AREA_LEN;

	int iHadSendLen = iRes;
	
	ClearBattUpgradeRcvBuffer();

	Make_SendCmd_4CANFrame(&frmSent);

	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)frmSent.byRawData, CAN_FRAME_LEN);
	if (nWriteSize < 0)
	{
		TRACE("Err: Write To CAN Port Failed !!!\n");
		return FALSE;
	}

	Sleep(20);

	time_t timeBegin = _NOW_;

	for (; ; )
	{
		if (ABS(_NOW_ - timeBegin) > UPGRADE_UDS_RECEIVE_TIMEOUT)
		{
			return FALSE;
		}

		YFY_ReadAndParseCanData(pSTBatt_Rough);
		GetBMSUpgradeStatus(pSTBatt_Rough);
		UDS_RESPOND repond = getBMSReplyStatus(pSTBatt_Rough, pUDSPara->byServiceID);
		if (SERVICE_POSITIVE_OK == repond)
		{
			break;
		}
		else if (SERVICE_NEGATIVE_FAILED == repond)
		{
			return FALSE;
		}
		else if (SERVICE_NEGATIVE_BUSY == repond || SERVICE_NOT_RESPOND == repond)
		{
		Sleep(20);
			continue;
	}

		Sleep(20);
	}
	
	ClearBattUpgradeRcvBuffer();

	int iDataLen = iBufLen;

	int iFrameIdx = 1;
	unsigned long uSndCount = 0;
	while (iHadSendLen < iBufLen)
	{
		memset(frmSent.byData, 0xaa, sizeof(frmSent.byData));

		frmSent.byData[0] = 0x20;
		BYTE idx = (BYTE)(iFrameIdx % 16);
		frmSent.byData[0] |= idx;

		int iLen = (iHadSendLen + 7) <= iDataLen ? 7 : iDataLen - iHadSendLen;
		
		memcpy(&frmSent.byData[1], &byBuf[iHadSendLen], (size_t)iLen);
		
		++iFrameIdx;
		iHadSendLen += iLen;

		Make_SendCmd_4CANFrame(&frmSent);

		ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)frmSent.byRawData, CAN_FRAME_LEN);
	
		if (nWriteSize < 0)
		{
			TRACE("Err: Write To CAN Port Failed !!!\n");
			return FALSE;
		}
		else if (pUpgradeParam->byFrameInterval > 3)
		{
			Sleep(pUpgradeParam->byFrameInterval); 
		}
		else 
		{
			Sleep(3);
		}
	}

	return TRUE;
}



/*********************************************************************************
 *
 *  FUNCTION NAME :  Send_C2B_UpgradeCmd
 *  CREATOR       :
 *  CREATED DATE  : 2019-12-12 19:22:53
 *  DESCRIPTION   : //John 2021-06-28 Note:Just Use 0xFF(BraodCast) Instead PCSIdx
 *
 *  CHANGE HISTORY:
 *
 ***********************************************************************************/
static BOOL Send_C2B_UpgradeCmd(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, ID_CMD_ND_BATT emC2BCmd)
{
	BATT_UPGRADE_PARAM *pUpgradeParam = &pSTBatt_Rough->stUpgradeParam;
	A_CAN_ND_BATT_FRAME frmSent;
	memset(frmSent.byData, 0xaa, sizeof(frmSent.byData));
	BYTE *pDataPos = NULL;
	int n = 0;
	BOOL bRet = FALSE;

	frmSent.uiFrmID = UPGRADE_FRAME_ID_UDS;
	frmSent.uiFrmID |= (pUpgradeParam->eDevType << 8);
	frmSent.uiFrmID |= pUpgradeParam->byDevAddr;
	frmSent.uiDataLen = CAN_DATA_AREA_LEN;

	switch (emC2BCmd)
	{
	case _CAN_S_CMD_C2B_HANDSHAKE:
	{
		// Notify BMS to enter programming mode
		frmSent.byData[0] = 0x02;		   // signal frame, data length :2 byte
		frmSent.byData[1] = UDS_SERVICE_HANDSHAKE_10; // SID
		frmSent.byData[2] = 0x02;		   // sub function: Requesting BMS to enter programming mode

		break;
	}

	case _CAN_S_CMD_C2B_UNLOCK:
	{
		frmSent.byData[0] = 0x02;		// signal frame, data length :2 byte
		frmSent.byData[1] = UDS_SERVICE_UNLOCK_27; // SID
		frmSent.byData[2] = 0x11;		// sub function: Requesting BMS to enter unlock mode

		break;
	}

	case _CAN_S_CMD_C2B_EXCHANGE_KEY:
	{
		frmSent.byData[0] = 6; // signal frame, data length :2 byte
		frmSent.byData[1] = UDS_SERVICE_EXCHANGE_KEY_27;	  // SID
		frmSent.byData[2] = 0x12;

		CalSeed(pSTBatt_Rough->stUpgradeParam.bySeed, &frmSent.byData[3]);
		break;
	}

	case _CAN_S_CMD_C2B_WRITE_REQUEST:
	{
		BYTE byData[UPGRADE_WRITE_REQUEST_LEN] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
		
		STUDS_UPGRADE_PARAMETER UDSParam;
		UDSParam.byServiceID = UDS_SERVICE_WRITE_REQUEST_2E;
		UDSParam.bySubServiceID[0] = 0xf1;
		UDSParam.bySubServiceID[1] = 0x98;
		UDSParam.bySubServiceLen = 2;

		Send_C2B_UpgradeUDSLongData(pSTBatt_Rough, &UDSParam, byData, UPGRADE_WRITE_REQUEST_LEN);
		return TRUE;

	}

	case _CAN_S_CMD_C2B_WRITE_REQUEST_2:
	{
		frmSent.byData[0] = 0x07;		   // signal frame, data length :7 bytes
		frmSent.byData[1] = UDS_SERVICE_WRITE_REQUEST_2E; // SID
		frmSent.byData[2] = 0xf1;		   
		frmSent.byData[3] = 0x99;
		frmSent.byData[4] = 0x01;
		frmSent.byData[5] = 0x02;
		frmSent.byData[6] = 0x03;
		frmSent.byData[7] = 0x04;

		break;
	}

	case _CAN_S_CMD_C2B_ROUTINE_CONTROL:
	{
		STUDS_UPGRADE_PARAMETER UDSParam;
		UDSParam.byServiceID = UDS_SERVICE_ROUNTINE_CONTROL_31;
		UDSParam.bySubServiceID[0] = 0x01;
		UDSParam.bySubServiceID[1] = 0xff;
		UDSParam.bySubServiceID[2] = 0x00;
		UDSParam.bySubServiceLen = 3;

		BYTE byData[UPGRADE_ROUNTINE_CONTROL_LEN] = {0x44, 0x08, 0x00, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00};

		UINT uStartAddr = pUpgradeParam->uStartAddr;
		byData[1] = (uStartAddr >> 24) & 0xff;
		byData[2] = (uStartAddr >> 16) & 0xff;
		byData[3] = (uStartAddr >> 8) & 0xff;
		byData[4] = uStartAddr & 0xff;

		unsigned long ufileSize = pUpgradeParam->stUpdownFileInfo.nTotalFileSize;
		byData[5] = (ufileSize >> 24) & 0xff;
		byData[6] = (ufileSize >> 16) & 0xff;
		byData[7] = (ufileSize >> 8) & 0xff;
		byData[8] = ufileSize & 0xff;

		Send_C2B_UpgradeUDSLongData(pSTBatt_Rough, &UDSParam, byData, UPGRADE_ROUNTINE_CONTROL_LEN);

		return TRUE;
		
	}
	case _CAN_S_CMD_C2B_DOWNLOAD_REQUSET:
	{
		STUDS_UPGRADE_PARAMETER UDSParam;
		UDSParam.byServiceID = UDS_SERVICE_DOWNLOAD_REQUEST_34;
		UDSParam.bySubServiceID[0] = 0x00;
		UDSParam.bySubServiceID[1] = 0x44;
		UDSParam.bySubServiceLen = 2;

		BYTE byData[UPGRADE_DOWNLOAD_REQUEST_LEN] = {0x08, 0x00, 0xA0, 0x00, 0x00, 0x06, 0xf1, 0xfc};

		UINT uStartAddr = pUpgradeParam->uStartAddr;
		byData[0] = (uStartAddr >> 24) & 0xff;
		byData[1] = (uStartAddr >> 16) & 0xff;
		byData[2] = (uStartAddr >> 8) & 0xff;
		byData[3] = uStartAddr & 0xff;

		unsigned long ufileSize = pUpgradeParam->stUpdownFileInfo.nTotalFileSize;
		byData[4] = (ufileSize >> 24) & 0xff;
		byData[5] = (ufileSize >> 16) & 0xff;
		byData[6] = (ufileSize >> 8) & 0xff;
		byData[7] = ufileSize & 0xff;

		Send_C2B_UpgradeUDSLongData(pSTBatt_Rough, &UDSParam, byData, UPGRADE_DOWNLOAD_REQUEST_LEN);

		return TRUE;
	}

	case _CAN_S_CMD_C2B_DOWNLOAD_FILE:
	{
		BOOL bResult = FALSE;
		int iPos = 0;
		int iSendLen = 0;
		int i = 0;
		int  iFileSize = pUpgradeParam->stUpdownFileInfo.nTotalFileSize;
		BYTE* pBuf = pUpgradeParam->stUpdownFileInfo.pbyReadBuf;

		int iBlockLen = pSTBatt_Rough->stUpgradeParam.uMaxDownloadBlockLen;

		int iCount = iFileSize / iBlockLen;

		STUDS_UPGRADE_PARAMETER UDSParam;
		UDSParam.byServiceID = UDS_SERVICE_DOWNLOAD_36;
		UDSParam.bySubServiceID[0] = 1;
		UDSParam.bySubServiceLen = 1;

		for (i = 0; i < iCount; ++i)
		{
			iSendLen = iPos + iBlockLen <= iFileSize ? iBlockLen : (iFileSize - iPos);
			
			TRACE("DOWNLOAD_FILE i = %d iSendLen: %d iPos: %d servCount: %d \n", i, iSendLen, iPos, UDSParam.bySubServiceID[0]);
			int n = 0;
			
			for (n = 0; n <= 50; ++n) //Send Failed，Then Retry 50 Times
			{
				if (n > 0)
					TRACE("DOWNLOAD_FILE 0x36 idx: %02x Retry n = %d\n", UDSParam.bySubServiceID[0], n);

				if (Send_C2B_UpgradeUDSLongData(pSTBatt_Rough, &UDSParam, &pBuf[iPos], iSendLen))
				{
					bResult = getBMSReplyResult(pSTBatt_Rough, UDS_SERVICE_DOWNLOAD_36);
					if (bResult)
						break;
				}	
				
				int idx = 0;
				TRACE("DOWNLOAD_FILE  Negative Reply %02x %02x %02x %02x %02x %02x %02x %02x \n", pSTBatt_Rough->stUpgradeParam.byData[0][idx++], pSTBatt_Rough->stUpgradeParam.byData[0][idx++], pSTBatt_Rough->stUpgradeParam.byData[0][idx++], pSTBatt_Rough->stUpgradeParam.byData[0][idx++], pSTBatt_Rough->stUpgradeParam.byData[0][idx++], pSTBatt_Rough->stUpgradeParam.byData[0][idx++], pSTBatt_Rough->stUpgradeParam.byData[0][idx++], pSTBatt_Rough->stUpgradeParam.byData[0][idx++]);
					
				Sleep(100);
			}

			if (!bResult)
				return FALSE;

			iPos += iSendLen;
			++UDSParam.bySubServiceID[0];

			RUN_THREAD_HEARTBEAT(); //feed dog

			int nProcess = i * 100 / iCount;
			if (nProcess > 95)
				nProcess = 95;

			Update_BMSUpgradeProcess(pSTBatt_Rough, nProcess);
		}

		if (iPos >= iFileSize)
		{
			TRACE("DOWNLOAD_FILE iPos >= iFileSize return");
			return TRUE;
		}
			
		iSendLen = iFileSize - iPos;

		TRACE("Download out block, had send iSendLen = %d\n", iSendLen);

		if (iSendLen <= 5)//Data is short. Use Single Frame to Transmit.
		{
			TRACE("DOWNLOAD_FILE send remain data use signal frame \n");
			memset(frmSent.byData, 0xaa, sizeof(frmSent.byData));

			frmSent.byData[0] = 2 + iSendLen;		   
			frmSent.byData[1] = UDS_SERVICE_DOWNLOAD_36;
			frmSent.byData[2] = UDSParam.bySubServiceID[0];

			memcpy(&frmSent.byData[3], &pBuf[iPos], iSendLen);
			break;
		}
		else 
		{
			TRACE("DOWNLOAD_FILE send remain data use mulute fram, iPos: %d iSendLen: %d\n", iPos, iSendLen);
			
			for (n = 0; n <= 50; ++n) //Send Failed，Then Retry 50 Times
			{
				if (n > 0)
					TRACE("DOWNLOAD_FILE 0x36 idx: %02x Retry n = %d\n", UDSParam.bySubServiceID[0], n);
				if (Send_C2B_UpgradeUDSLongData(pSTBatt_Rough, &UDSParam, &pBuf[iPos], iSendLen))
				{
					bResult = getBMSReplyResult(pSTBatt_Rough, UDS_SERVICE_DOWNLOAD_36);
					if (bResult)
						break;
				}	
				int idx = 0;
				TRACE("DOWNLOAD_FILE  Negative Reply %02x %02x %02x %02x %02x %02x %02x %02x \n", pSTBatt_Rough->stUpgradeParam.byData[0][idx++], pSTBatt_Rough->stUpgradeParam.byData[0][idx++], pSTBatt_Rough->stUpgradeParam.byData[0][idx++], pSTBatt_Rough->stUpgradeParam.byData[0][idx++], pSTBatt_Rough->stUpgradeParam.byData[0][idx++], pSTBatt_Rough->stUpgradeParam.byData[0][idx++], pSTBatt_Rough->stUpgradeParam.byData[0][idx++], pSTBatt_Rough->stUpgradeParam.byData[0][idx++]);
				Sleep(100);
			}
			if (!bResult)
				return FALSE;
		}
		
		return TRUE;	
	}

	case _CAN_S_CMD_C2B_DOWNLOAD_EXIT:
	{
		frmSent.byData[0] = 0x02;		   
		frmSent.byData[1] = UDS_SERVICE_DOWNLOAD_EXIT_37; // SID
		frmSent.byData[2] = 0x01;		   

		break;
	}

	
	case _CAN_S_CMD_C2B_CHECK_FILE:
	{
		uint32_t uCurrentCrc = 0;

		Cal_Crc32Init(&uCurrentCrc);
		Cal_ComputeCrc32(&uCurrentCrc,  pUpgradeParam->stUpdownFileInfo.pbyReadBuf, pUpgradeParam->stUpdownFileInfo.nTotalFileSize);
		Cal_CrcFinalize(&uCurrentCrc);

		BYTE byData[UPGRADE_CRC32_DATA_LEN] = {0x00, 0x00, 0x00, 0x00};

		byData[0] = (uCurrentCrc >> 24) & 0xff;
		byData[1] = (uCurrentCrc >> 16) & 0xff;
		byData[2] = (uCurrentCrc >> 8) & 0xff;
		byData[3] = uCurrentCrc & 0xff;

		STUDS_UPGRADE_PARAMETER UDSParam;
		UDSParam.byServiceID = UDS_SERVICE_ROUNTINE_CONTROL_31;
		UDSParam.bySubServiceID[0] = 0x01;
		UDSParam.bySubServiceID[1] = 0x02;
		UDSParam.bySubServiceID[2] = 0x02;
		UDSParam.bySubServiceLen = 3;

		Send_C2B_UpgradeUDSLongData(pSTBatt_Rough, &UDSParam, byData, UPGRADE_CRC32_DATA_LEN);

		return TRUE;	
	}

	case _CAN_S_CMD_C2B_CHECK_COMPATIBILITY:
	{
		frmSent.byData[0] = 0x04;		   
		frmSent.byData[1] = UDS_SERVICE_ROUNTINE_CONTROL_31; // SID
		frmSent.byData[2] = 0x01;
		frmSent.byData[3] = 0xff;
		frmSent.byData[4] = 0x01;		   

		break;
	}

	case _CAN_S_CMD_C2B_RESET_ECU:
	{
		frmSent.byData[0] = 0x02;		   
		frmSent.byData[1] = UDS_SERVICE_RESET_ECU_11;
		frmSent.byData[2] = 0x01;

		break;
	}

	default:
		break;
	}
	
	Make_SendCmd_4CANFrame(&frmSent);
	
	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)frmSent.byRawData, CAN_FRAME_LEN);
	
	if (nWriteSize < 0)
	{
		TRACE("Err: Write To CAN Port Failed !!!\n");
		bRet = FALSE;
	}
	else
	{
		bRet = TRUE;
		Sleep(5); // Wait 10ms, BMS Req Send Single Msg Seperately
	}
	
	return bRet;
}

static UINT GetRemtDownIdx(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	UINT nRemtRepIdx = 0;
	int n = 0;
	BATT_UPGRADE_PARAM *pUpgradeParam = &pSTBatt_Rough->stUpgradeParam;

	nRemtRepIdx = pUpgradeParam->nRemtDownIndex[0];
	for (n = 0; n < pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt; n++)
	{
		nRemtRepIdx = MIN(nRemtRepIdx, pUpgradeParam->nRemtDownIndex[n]);
	}

	return nRemtRepIdx;
}

/*********************************************************************************
 *
 *  FUNCTION NAME :  isRecvRmtRepIdHadRevert
 *  CREATOR       :
 *  CREATED DATE  : 2019-12-12 19:22:53
 *  DESCRIPTION   : Current Recv RemtRepIdx Over 4096, Then MsgID shoulde Turn Back.
 *
 *  CHANGE HISTORY:
 *
 ***********************************************************************************/
static isRecvRmtRepIdHadRevert(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	BATT_UPGRADE_PARAM *pUpgradeParam = &pSTBatt_Rough->stUpgradeParam;
	BOOL bRet = FALSE;

	// int nLastStartMsgId = (pUpgradeParam->nLocalSendMsgId > MAX_CYCLE_SEND_TIMES) ?
	//	(pUpgradeParam->nLocalSendMsgId - MAX_CYCLE_SEND_TIMES):pUpgradeParam->nLocalSendMsgId;

	UINT nRemtRepIdx = GetRemtDownIdx(pSTBatt_Rough);
	if (nRemtRepIdx < (pUpgradeParam->nLast_StartMsgId % 4096))
	{
		bRet = TRUE;
		TRACE("nRemtRepIdx=[%d] nLast_StartMsgId=[%d] nLastStartMsgId%%4096=[%d]\n", \
			  nRemtRepIdx, pUpgradeParam->nLast_StartMsgId, pUpgradeParam->nLast_StartMsgId%4096);
	}

	return bRet;
}

/*********************************************************************************
 *
 *  FUNCTION NAME :  SendBMS_BinFileData
 *  CREATOR       :
 *  CREATED DATE  : 2019-12-12 19:22:53
 *  DESCRIPTION   :
 *
 *  CHANGE HISTORY:
 *
 ***********************************************************************************/
static BOOL SendBMS_BinFileData(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	BATT_UPGRADE_PARAM *pUpgradeParam = &pSTBatt_Rough->stUpgradeParam;

	BOOL bRet = TRUE;
	int n = 0, nRemainLen = 0;
	UINT nRemtDownIndex = GetRemtDownIdx(pSTBatt_Rough);
	// int nLastStartMsgId = (pUpgradeParam->nLocalSendMsgId > MAX_CYCLE_SEND_TIMES) ?
	//		(pUpgradeParam->nLocalSendMsgId - MAX_CYCLE_SEND_TIMES):pUpgradeParam->nLocalSendMsgId;

	if (isRecvRmtRepIdHadRevert(pSTBatt_Rough) == TRUE) // case1: Recv DataFrame Go back，then MsgID need /4096 + 1.
	{
		pUpgradeParam->nLocalSendMsgId = ((pUpgradeParam->nLast_StartMsgId / 4096) + 1) * 4096 + nRemtDownIndex;
		pUpgradeParam->stUpdownFileInfo.nDataOffset = pUpgradeParam->nLocalSendMsgId * 6;
	}
	else // case2:Recv DataFrame Not Goback.
	{
		pUpgradeParam->nLocalSendMsgId = ((pUpgradeParam->nLast_StartMsgId / 4096)) * 4096 + nRemtDownIndex;
		pUpgradeParam->stUpdownFileInfo.nDataOffset = pUpgradeParam->nLocalSendMsgId * 6;
		// if(pUpgradeParam->nLocalSendMsgId == 0)
		//{
		//	pUpgradeParam->nLocalSendMsgId = 1; //Start From 1
		// }
	}

	if (pUpgradeParam->stUpdownFileInfo.nDataOffset >= pUpgradeParam->stUpdownFileInfo.nTotalFileSize)
	{
		// Old Version Need Send This Cmd,New Version Not Need. this Logic is Not Good.
		Send_C2B_UpgradeCmd(pSTBatt_Rough, _CAN_S_CMD_C2B_DATAFINISH);
		TRACE("Info: Had Send TotalData Success!\n");
		return bRet;
	}

	TRACE("tmNow=[%ld]:nTotalFileSize=[%d] nLast_StartMsgId=[%d] LocalSendMsgId=[%d] nDataOffset=[%d] nRemtDownIndex=[%d]\n", \
		  _NOW_,
		  pUpgradeParam->stUpdownFileInfo.nTotalFileSize,\
		  pUpgradeParam->nLast_StartMsgId,\
		  pUpgradeParam->nLocalSendMsgId,\
		  pUpgradeParam->stUpdownFileInfo.nDataOffset,\
		  nRemtDownIndex);

	//Note:Save Last_SendMsgID, In Oder To Calc the Next Reality NextTime Start MsgID
	pUpgradeParam->nLast_StartMsgId = pUpgradeParam->nLocalSendMsgId;

	for (n = 0; n < MAX_CYCLE_SEND_TIMES; n++)
	{
		if (pUpgradeParam->stUpdownFileInfo.nDataOffset < pUpgradeParam->stUpdownFileInfo.nTotalFileSize)
		{
			nRemainLen = pUpgradeParam->stUpdownFileInfo.nTotalFileSize - pUpgradeParam->stUpdownFileInfo.nDataOffset;
			pUpgradeParam->nSingleMsgLen = (nRemainLen > MAX_BMS_SINGLE_MSGLEN) ? MAX_BMS_SINGLE_MSGLEN : nRemainLen;

			// Note:MsgID is Start From 1,And Current Recv MsgID is BMS Success Recv MsgID Number,So Need +1 to Send the Next MsgId
			pUpgradeParam->nLocalSendMsgId++;
			if (!Send_C2B_UpgradeCmd(pSTBatt_Rough, _CAN_S_CMD_C2B_SECTORDATA))
			{
				TRACE("Err: CAN Write Data Failed, So break!\n");
				break;
			}
			// If Send Success, Offset the DataByte
			pUpgradeParam->stUpdownFileInfo.nDataOffset += pUpgradeParam->nSingleMsgLen;
			//Cycle Write should Wait 10ms, BMS Req Single Msg Seperately Sending ,Process in Write()
		}
		else
		{
			TRACE("Info: Send_Finished nDataOffset=[%d] nTotalFileSize=[%d]!\n", \
				  pUpgradeParam->stUpdownFileInfo.nDataOffset ,\
				  pUpgradeParam->stUpdownFileInfo.nTotalFileSize);
			break;
		}
	}

	// TRACE("---StartTime=%ld--- nFileDataLen =%d nDataOffset=%d\n",\ _NOW_, nTotalFileSize, pUpgradeParam->stUpdownFileInfo.nDataOffset);
	return bRet;
}

/*********************************************************************************
 *
 *  FUNCTION NAME :
 *  CREATOR       :
 *  CREATED DATE  : 2019-12-12 19:22:53
 *  DESCRIPTION   :
 *
 *  CHANGE HISTORY:
 *
 ***********************************************************************************/
static void Send_C2B_Status(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nPCSIdx)
{
	int n = 0;

	BYTE byRawData[CAN_FRAME_LEN] = {0};
	BYTE *pbySendBuf = byRawData;
	BYTE bySubCmd = 0x8F;

	// TRACE("***Send_C2B_Status : nPCSIdx=[%d] \n", nPCSIdx);

	PackSendFrameData(pSTBatt_Rough, nPCSIdx, bySubCmd, pbySendBuf);

	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)pbySendBuf, CAN_FRAME_LEN);
	if (nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if ((pSTBatt_Rough->stuCanSave.stuStatMach[nPCSIdx].emSaveState == STAT_ON_PENDING) &&
			(pSTBatt_Rough->stuCanSave.emTrigMsgTp[nPCSIdx] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pSTBatt_Rough, nPCSIdx, pbySendBuf, 1);
		}
	}
	/*
	for(n = 0; n <nWriteSize; n++)
	{
		printf("%02X",pbySendBuf[n]);
	}
	printf("\n");
	*/
	return TRUE;
}


/*********************************************************************************
 *
 *  FUNCTION NAME :
 *  CREATOR       :
 *  CREATED DATE  : 2019-12-12 19:22:53
 *  DESCRIPTION   :
 *
 *  CHANGE HISTORY:
 *
 ***********************************************************************************/
void Send_DebugMsg_2UI(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, BOOL bFinish, BOOL bError, BOOL bLog, char *pszFormat, ...)
{

	SAMPLER *pSampler = (SAMPLER *)pSTBatt_Rough->hSampler;

	EVENT_UI_NOTICE evNote;
	ZERO_POBJS(&evNote, 1);
	evNote.emNotoceType = UI_NOTICETYPE_BMS_UPDT;
	evNote.byData[0] = bFinish; // LCD will judge if need to end debug
	char szDebug[128];
	va_list arglist;
	va_start(arglist, pszFormat);
	vsnprintf(szDebug, 128, pszFormat, arglist);
	va_end(arglist);
	memcpy(&evNote.byData[1], szDebug, 128);
	if (bLog)
	{
		LogOut(MAIN_MODULE_SAMPLER, "BAT-Updt",
			   bError ? LOG_TYPE_ERROR : LOG_TYPE_WARING, szDebug);
	}
	static time_t tmLastSendMsg = 0;
	time_t tmNow = _NOW_;
	if (tmNow - tmLastSendMsg < 2)
	{
		// avoid send msg too quickly
		Sleep(1000);
	}
	tmLastSendMsg = _NOW_;

	SendMsg((HANDLE)pSampler, EVENT_TYPE_UI_NOTICE, EVENT_MSG_BEGIN,
			&evNote, sizeof(EVENT_UI_NOTICE));
}

// void Update_RectUpgradeProcess(int nProcess)
//{
//	//Process Later
//	VAR_VALUE varV;
//	varV.lValue = nProcess;
//	//Push_VarValue(SETTYPE_LOGIC_SP, (HANDLE)pSampler, 250, 182, varV);
// }

static void TxtAsc2BinHex(BYTE *pHexBuff, BYTE *pAscBuff, int nFilelen)
{
	int n = 0;

	for (n = 0; n < (nFilelen / 2); n++)
	{
		*(pHexBuff++) = Ascii2ToHex(pAscBuff + n * 2);
	}

	if ((nFilelen % 2) != 0)
	{
		*(pHexBuff++) = (AsciiToHex(pAscBuff[nFilelen - 1]) << 4);
	}
	return ;
}

static void Delete_Char(BYTE *pStr, BYTE byParam1, BYTE byParam2)
{
	int i = 0,j = 0;

	for (i = 0, j = 0; pStr[i] != '\0'; i++)
	{
		if ((pStr[i] != byParam1) && (pStr[i] != byParam2))
		{
			pStr[j++] = pStr[i];
		}
	}

	pStr[j] = '\0';
	return;
}


#define MAX_LINE_SIZE 				1024
#define MIN_HEX_LINE_LEN 			11
#define TYPE_DATA_RECORD			0
#define TYPE_FILE_END				1
#define TYPE_EXTEND_SEGMENT_ADDR	2
#define TYPE_START_SEGMENT_ADDR		3
#define TYPE_EXTEND_LINEAR_ADDR		4
#define TYPE_START_LINEAR_ADDR		5

BYTE* GetLine(char* pOutLine, int iOutLineLen, BYTE* pInBuf)
{
	if (pInBuf[0] == 0)
	{
		return pInBuf;
	}

	int i = 0;
	while(pInBuf[i] !=  0x0d && pInBuf[i+1] != 0x0a && i < iOutLineLen)
	{
		pOutLine[i] = pInBuf[i];
		++i;
	}
	pOutLine[i] = 0;

	return pInBuf + i + 2;
}


static BOOL Hex2Bin(BYTE *pSrcBuf, BYTE *pDstBuf, HEX2BIN_INFO* pOutInfo)
{
	BOOL bRet = FALSE;
	BYTE byLine[MAX_LINE_SIZE];
    BYTE byDataStr[MAX_LINE_SIZE];
    BYTE byDataLineTmp[MAX_LINE_SIZE / 2];
    int iRecordNb = 0;
    unsigned int uNbBytes = 0, uOffset = 0, uSegment = 0, uLineAddr = 0;
    unsigned int uPhysAddr = 0, uStartAddress = -1;
    unsigned int uType = 0;
    unsigned int uTemp = 0;
    int iResult;
    BYTE byChecksum = 0;
    unsigned int uAddrTmp;
    BYTE *pDst = pDstBuf;
    BYTE *pFilin = pSrcBuf;

	int iAllLen = 0;

	for (; ;)
    {
		pOutInfo->iErrLine = ++iRecordNb;

		memset(byLine, 0, sizeof(byLine));
    	pFilin = GetLine(byLine, MAX_LINE_SIZE, pFilin);

		if (0 == strcmp(":00000001FF", byLine))
		{
			pOutInfo->iErrLine = 0;
			bRet = TRUE;
			break;
		}

		if (byLine[0] != ':' || strlen(byLine) < MIN_HEX_LINE_LEN)
		{
			bRet = FALSE;
			break;
		}
		
        memset(byDataStr, 0, sizeof(byDataStr));
        iResult = sscanf(byLine, ":%2x%4x%2x%s", &uNbBytes, &uOffset, &uType, byDataStr);
        if (iResult != 4)
		{
			TRACE("Error in line %d of hex file\n", iRecordNb);
			bRet = FALSE; 
			break;
		}

        memset(byDataLineTmp, 0, sizeof(byDataLineTmp));

        char *p = (char *)byDataStr;
		byChecksum = 0;
        int k = 0;
        for (k = 0; k < uNbBytes; k++)
        {
            iResult = sscanf(p, "%2x", &uTemp);
            if (iResult != 1)
            {
				bRet = FALSE;
                TRACE("ReadDataBytes: error in line %d of hex file\n", iRecordNb);
                break;
            }

            byDataLineTmp[k] = uTemp;
			byChecksum = byChecksum + uTemp;
            p += 2;
        }

		BYTE byChecksumRead = 0;
		iResult = sscanf(p, "%2x", &byChecksumRead);
		if (iResult != 1)
        {
			bRet = FALSE;
            TRACE("ReadDataBytes: error in line %d of hex file\n", iRecordNb);
            break;
        }

        byChecksum = uNbBytes + (uOffset >> 8) + (uOffset & 0xFF) + uType + byChecksum;
		BYTE byCalChecksum = (0x00 - byChecksum) & 0xff;
		if (byCalChecksum != byChecksumRead)
		{
			TRACE("Hex2Bin line %d Checksum error!\n", iRecordNb);
			TRACE("%s\n", byLine);
			TRACE("Checksum: %02x  byChecksumRead: %02x byCalChecksum: %02x \n", byChecksum, byChecksumRead, byCalChecksum);
			break;
		}
		
        switch (uType)
        {
        case TYPE_DATA_RECORD:
            if (uNbBytes == 0)
            {
                TRACE("0 byte length Data record ignored\n");
                break;
            }

            unsigned int uPhysAddr = uLineAddr + uSegment + uOffset;

            if (uPhysAddr < uStartAddress)
                uStartAddress = uPhysAddr;

            unsigned int uDataAddr = uPhysAddr - uStartAddress;
            memcpy(&pDst[uDataAddr], byDataLineTmp, uNbBytes);

			if (uDataAddr + uNbBytes > iAllLen)
				iAllLen = uDataAddr + uNbBytes;

            break;

        case TYPE_FILE_END:    
            break;

        case TYPE_EXTEND_SEGMENT_ADDR:
            uAddrTmp = ((byDataLineTmp[0] << 8) & 0xff00) | (byDataLineTmp[1] & 0xff) ;
            uSegment = (uAddrTmp << 4);
            break;
        case TYPE_START_SEGMENT_ADDR:
            break;

        case TYPE_EXTEND_LINEAR_ADDR:
			uAddrTmp = ((byDataLineTmp[0] << 8) & 0xff00) | (byDataLineTmp[1] & 0xff) ;
            uLineAddr = (uAddrTmp << 16);
            break;
        case TYPE_START_LINEAR_ADDR:
            break;
        default:
            TRACE("Unknown record type\n");
            break;
        }
    } 

	pOutInfo->iBinDataLen = iAllLen;
	pOutInfo->uStartAddr = uStartAddress;

    return bRet;
}



/*********************************************************************************
 *
 *  FUNCTION NAME :
 *  CREATOR       :
 *  CREATED DATE  : 2019-12-12 19:22:53
 *  DESCRIPTION   :
 *
 *  CHANGE HISTORY:
 *
 ***********************************************************************************/
#define MCBLK1_FIRMWARE_UPGRADE_PATH "/media/mmcblk0p1/"
#define SDA1_FIRMWARE_UPGRADE_PATH "/media/sda1/"
static int CheckandRead_BMSFile(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n = 0;
	char szFileName[64] = {0};
	char szBMSFileDir[126] = {0};

	if (!FindMatchFile_FromDir(MCBLK1_FIRMWARE_UPGRADE_PATH, "YFBMS_BMU", szFileName))
	{
		TRACE("Err: Not Found Upgrade BMS File in mmcblk0p1\n");
		if (!FindMatchFile_FromDir(SDA1_FIRMWARE_UPGRADE_PATH, "YFBMS_BMU", szFileName))
		{
			TRACE("Err: Not Found Upgrade BMS File in sda1\n");
			return -1;
		}
		else
		{
			snprintf(szBMSFileDir, sizeof(szBMSFileDir), "%s%s", SDA1_FIRMWARE_UPGRADE_PATH, szFileName);
		}
	}
	else
	{
		snprintf(szBMSFileDir, sizeof(szBMSFileDir), "%s%s", MCBLK1_FIRMWARE_UPGRADE_PATH, szFileName);
	}

	FILE *fp = fopen(szBMSFileDir, "r");
	if (fp == NULL)
	{
		TRACEX("Err: Can NOT open the file[%s]\n", szBMSFileDir);
		return -2;
	}

	fseek(fp, 0L, SEEK_END);
	size_t nFilelen = ftell(fp);
	if ((nFilelen < MIN_BMS_BINSIZE) || (nFilelen > MAX_BMS_BINSIZE))
	{
		TRACEX("Err: FileLen[%d] is incorrect\n", nFilelen);
		fclose(fp);
		return -3;
	}

	// now allocate memory for the file
	BYTE *pBuff = NEW(BYTE, nFilelen);
	if (pBuff == NULL)
	{
		TRACEX("Err: CAN NOT allocate memory for FileLen[%d]\n", nFilelen);
		fclose(fp);
		return -4;
	}
	// DO NOT forget to go back to file head!!!
	fseek(fp, 0L, SEEK_SET);
	// now read all data into memory
	fread(pBuff, nFilelen, 1, fp);
	fclose(fp);

	// beacause Read From .TXT is ASCII for the content,so need Exchange to HEX
	BYTE *pHexBuff = NEW(BYTE, nFilelen);
	if (pHexBuff == NULL)
	{
		TRACEX("Err: CAN NOT allocate memory for pHexBuff[%d]\n", nFilelen);
		return -5;
	}

	// Step1. Remove '\r\n'
	Delete_Char(pBuff, '\n', '\r');

	int nOrgLen = strlen(pBuff);

	// Step2. Exchange Txt Acs To Hex Format
	TxtAsc2BinHex(pHexBuff, pBuff, nOrgLen);

	SAFELY_DELETE(pBuff);

	int nHexFileLen = ((nOrgLen % 2) == 0) ? (nOrgLen / 2) : ((nOrgLen / 2) + 1);

	pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.pbyReadBuf = pHexBuff;
	pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.nTotalFileSize = nHexFileLen;
	pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.nDataOffset = 0;

	if (pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.nTotalFileSize % MAX_BMS_SINGLE_MSGLEN == 0)
	{
		pSTBatt_Rough->stUpgradeParam.nTotalMsgNum = pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.nTotalFileSize / MAX_BMS_SINGLE_MSGLEN;
	}
	else
	{
		pSTBatt_Rough->stUpgradeParam.nTotalMsgNum = pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.nTotalFileSize / MAX_BMS_SINGLE_MSGLEN + 1;
	}

	Init_UpgradeBMSParam(pSTBatt_Rough);
	// for(n = 0; n < 500; n++)
	//{
	//	printf("%02X",pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.pbyReadBuf[n]);
	//	if(n%100 == 0)
	//	{
	//		printf("\n");
	//	}
	// }

	TRACEX("Info: Read bin file[%s] successfully, and fileSize[%d]\n", szBMSFileDir, nFilelen);

	return TRUE;
}

/*********************************************************************************
 *
 *  FUNCTION NAME :
 *  CREATOR       :
 *  CREATED DATE  : 2019-12-12 19:22:53
 *  DESCRIPTION   :
 *
 *  CHANGE HISTORY:
 *
 ***********************************************************************************/
static BOOL isAllBMSEnterRmtUpgrade(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	BOOL bRet = FALSE;
	int n = 0;

	// For Test Added This
	// pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt = 1;

	for (n = 0; n < pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt; n++)
	{
		TRACE("GrpIdx=[%d] RemtRunModel = [%d]\n", n, pSTBatt_Rough->stUpgradeParam.nRemtRunModel[n]);
		if ((pSTBatt_Rough->stUpgradeParam.nRemtRunModel[n] == 1) || (pSTBatt_Rough->stUpgradeParam.nRemtRunModel[n] == 4)) // 0-Local Normal Upgrade Mode ,1-Means Remt Upgrade App Mode
		{
			bRet = TRUE;
			break;
		}
	}
	return bRet;
}


void Update_BMSUpgradeProcess(IN STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nProcess)
{
	BATT_UPGRADE_PARAM *pUpgradeParam = &pSTBatt_Rough->stUpgradeParam;

	VAR_VALUE varV;
	varV.lValue = nProcess;
	Push_VarValue(SETTYPE_LOGIC_SP, pSTBatt_Rough->hSampler, 200, 82, varV);

	varV.emValue = pUpgradeParam->emUpgradeStatus;
	Push_VarValue(SETTYPE_LOGIC_SP, pSTBatt_Rough->hSampler, 200, 81, varV);

	return;
}

/*********************************************************************************
 *
 *  FUNCTION NAME :
 *  CREATOR       :
 *  CREATED DATE  : 2019-12-12 19:22:53
 *  DESCRIPTION   :
 *
 *  CHANGE HISTORY:
 *
 ***********************************************************************************/
void HandleBatteryUpgrading(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	BATT_UPGRADE_PARAM *pUpgradeParam = &pSTBatt_Rough->stUpgradeParam;
	time_t tm_NOW = time(NULL);
	int n = 0;
	int nProcess = 253;
	int iRet = 0;
	char buffer[64]; 

	TRACE("%d pUpgradeParam->emUpgradeStatus = %d\n",__LINE__, (int)pUpgradeParam->emUpgradeStatus);
	switch (pUpgradeParam->emUpgradeStatus)
	{	
		case UPGRADE_READBINFILE:
		{
			nProcess = 253; // Please Wait
			BOOL bRet = TRUE;
			pUpgradeParam->bUpgradeResult = FALSE;

			char* Result;
			Result = strstr(pSTBatt_Rough->blobUpdCmd.file_name, "BSU");  //检查是不是 BSU远程升级
			if(Result != NULL)
			{
				if(SYSTYPE_ESS_LIQUIDED == pSTBatt_Rough->lcCfg.emSystemType)
				{
					Result = strstr(pSTBatt_Rough->blobUpdCmd.file_name, "BSU02U1F1");  //检查对液冷BSU升级文件名中 是否有液冷升级文件名，不是就结束升级
					if(Result == NULL)
					{
						pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
						Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "error:Liquid cooling system requires a liquid cooling BSU file");
						break;
					}
				}
				else if(SYSTYPE_ESS_AIR==pSTBatt_Rough->lcCfg.emSystemType)
				{
					Result = strstr(pSTBatt_Rough->blobUpdCmd.file_name, "BSU02U1F1");  //如果风冷升级BSU中，上传的是液冷BSU文件，结束升级
					if(Result != NULL)
					{
						pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
						Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "error:Air cooling system requires an air cooling BSU file");
						break;
					}
				}
			}

			if (!pSTBatt_Rough->blobUpdCmd.bIs4Web)
			{
				bRet = CheckandRead_BMSFile(pSTBatt_Rough);
			}
			else
			{
				Init_UpgradeBMSParam(pSTBatt_Rough);
			}

			if (!bRet)
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
				Send_DebugMsg_2UI(pSTBatt_Rough, TRUE, TRUE, TRUE, "Fault:Read BMS File Failed bRet=[%d]!", bRet);
				TRACE("Err: Read BMS File Failed,Eixt Upgrade! emUpgradeStatus=[%d] bRet=[%d]\n", pUpgradeParam->emUpgradeStatus, bRet);
			}
			char* IMEU3Name;
			IMEU3Name = strstr(pSTBatt_Rough->blobUpdCmd.file_name, "IMEU3");  //检查是不是 BSU远程升级
			pUpgradeParam->byDevAddr = (BYTE)pSTBatt_Rough->blobUpdCmd.iDevAddr;

			if (pUpgradeParam->byDevAddr >= BMU_MIN_ADDRESS && pUpgradeParam->byDevAddr <= BMU_MAX_ADDRESS)
			{
				pUpgradeParam->eDevType = DEV_BMU;
				pUpgradeParam->uStartAddr = BMU_PROGRAM_ADDRESS;
			}
			else if (pUpgradeParam->byDevAddr >= BSU_MIN_ADDRESS && pUpgradeParam->byDevAddr <= BSU_MAX_ADDRESS)
			{
				pUpgradeParam->eDevType = DEV_BSU;
				pUpgradeParam->uStartAddr = BSU_PROGRAM_ADDRESS;
			}
			else if (pUpgradeParam->byDevAddr >= BMU_AUX_MIN_ADDRESS && pUpgradeParam->byDevAddr <= BMU_AUX_MAX_ADDRESS)
			{
				pUpgradeParam->eDevType = DEV_BMU_AUX;
				pUpgradeParam->uStartAddr = BMU_AUX_PROGRAM_ADDRESS;
			}
			else if((pUpgradeParam->byDevAddr >= Air_IMEU3_MIN_ADDRESS && pUpgradeParam->byDevAddr <= Air_IMEU3_MAX_ADDRESS)&&(IMEU3Name != NULL))
			{
				//WLD01206 空调EU3升级地址和EU2地址冲突了，因此加了对文件名判断的条件
				pUpgradeParam->eDevType = DEV_IMEU3;
				pUpgradeParam->uStartAddr = IMEU3_AIR_ADDRESS;
			}
			else if (pUpgradeParam->byDevAddr >= IMEU2_MIN_ADDRESS && pUpgradeParam->byDevAddr <= IMEU2_MAX_ADDRESS)
			{
				pUpgradeParam->eDevType = DEV_IMEU2;
				pUpgradeParam->uStartAddr = IMEU2_PROGRAM_ADDRESS;
			}
			else if((pUpgradeParam->byDevAddr >= IMEU3_MIN_ADDRESS && pUpgradeParam->byDevAddr <= IMEU3_MAX_ADDRESS) || IMEU3_STSCAbinet_ADDRESS == pUpgradeParam->byDevAddr)
			{
				pUpgradeParam->eDevType = DEV_IMEU3;
				pUpgradeParam->uStartAddr = IMEU3_PROGRAM_ADDRESS;
			}
			else
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;

				TRACE("Err: UPGRADE Device Addr ERROR, byDevAddr(hex): %x\n", pUpgradeParam->byDevAddr);
				Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "error:upgrade device addr error! addr(hex):%x", pUpgradeParam->byDevAddr);
						
				break;
			}

			BYTE* pBufBinData = NEW(BYTE, pUpgradeParam->stUpdownFileInfo.nTotalFileSize);
			if (!pBufBinData)
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
				LogOut(MAIN_MODULE_SAMPLER, "BAT-Updt", LOG_TYPE_ERROR, "Malloc memory failed before Hex2Bin.");
				break;
			}

			memset(pBufBinData, 0, pUpgradeParam->stUpdownFileInfo.nTotalFileSize);

			HEX2BIN_INFO Info;
			memset(&Info, 0, sizeof(Info));

			bRet = Hex2Bin(pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.pbyReadBuf, pBufBinData, &Info);

			SAFELY_DELETE(pUpgradeParam->stUpdownFileInfo.pbyReadBuf);
			pUpgradeParam->stUpdownFileInfo.pbyReadBuf = pBufBinData;
			pUpgradeParam->stUpdownFileInfo.nTotalFileSize = Info.iBinDataLen;

			if (!bRet)
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
				Send_DebugMsg_2UI(pSTBatt_Rough, TRUE, TRUE, TRUE, "Fault: Failed to convert hex file to bin file!");
				TRACE("Err: Failed to convert hex file to bin file!,Eixt Upgrade! emUpgradeStatus=[%d] bRet=[%d]\n", pUpgradeParam->emUpgradeStatus, bRet);

				break;
			}

			if (pUpgradeParam->uStartAddr != Info.uStartAddr)
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;

				TRACE("Err: UPGRADE_READBINFILE ERROR, start address of hex file error! uStartAddr: %x\n", Info.uStartAddr);
				Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "error:start address of hex file error!");
						
				break;
			}

			ClearBattUpgradeRcvBuffer();
			pUpgradeParam->tm_StartUpgrade = _NOW_;
			pUpgradeParam->emUpgradeStatus = UPGRADE_HANDSHAKE;

			Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "Ready:Read BMS File OK->HandShake!");
			TRACE("Info: Read BMS File Success, Now Goto HandShake,emUpgradeStatus=[%d]\n", pUpgradeParam->emUpgradeStatus);
			
			break;
		}
		case UPGRADE_HANDSHAKE:
		{
			TRACE("Info: IMMU2 has been enter UPGRADE_HANDSHAKE step...\n");
			nProcess = 253; // Please Wait
			
			if ((ABS(_NOW_ - pUpgradeParam->tm_StartUpgrade) > 120))
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
				TRACE("Err: HandShake Cmd Failure:tm_NOW=[%ld] tm_StartUpgrade=[%ld]\n", tm_NOW, pUpgradeParam->tm_StartUpgrade);
				LogOut(MAIN_MODULE_SAMPLER, "BAT-Updt", LOG_TYPE_ERROR, "HandShake Cmd Failure");
				break;
			}

			TRACE("Info: IMMU2 send cmmd _CAN_S_CMD_C2B_HANDSHAKE\n");
			Send_C2B_UpgradeCmd(pSTBatt_Rough, _CAN_S_CMD_C2B_HANDSHAKE);

			BOOL bResult = getBMSReplyResult(pSTBatt_Rough, UDS_SERVICE_HANDSHAKE_10);
			if (bResult)
			{
				ClearBattUpgradeRcvBuffer();
		
				pUpgradeParam->tm_StartUpgrade = _NOW_;
				pUpgradeParam->emUpgradeStatus = UPGRADE_UNLOCK;

				TRACE("Info: BMS Reply _CAN_S_CMD_C2B_HANDSHAKE OK...\n");
				Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "Info:HandShake OK->Unlock!");

			}
		
			break;
		}

		case UPGRADE_UNLOCK:
		{
			TRACE("Info: IMMU2 has been enter UPGRADE_UNLOCK step...\n");
			nProcess = 253; // Please Wait
			
			if ((ABS(_NOW_ - pUpgradeParam->tm_StartUpgrade) > 120))
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
				TRACE("Err: UPGRADE_UNLOCK Cmd Failure:tm_NOW=[%ld] tm_StartUpgrade=[%ld]\n", tm_NOW, pUpgradeParam->tm_StartUpgrade);
				LogOut(MAIN_MODULE_SAMPLER, "BAT-Updt", LOG_TYPE_ERROR, "UPGRADE_UNLOCK Cmd Failure");
				break;
			}

			TRACE("Info: IMMU2 send cmmd _CAN_S_CMD_C2B_UNLOCK\n");
			Send_C2B_UpgradeCmd(pSTBatt_Rough, _CAN_S_CMD_C2B_UNLOCK);

			BOOL bResult = getBMSReplyResult(pSTBatt_Rough, UDS_SERVICE_UNLOCK_27);

			if (bResult)
			{
				// copy seed sended from BMS
				for (n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
				{
					BYTE recCode = pSTBatt_Rough->stUpgradeParam.byData[n][1];
					if ((recCode == (UDS_SERVICE_UNLOCK_27 + UDS_REPLY_POSITIVE)))
					{
						memcpy(pSTBatt_Rough->stUpgradeParam.bySeed, &pSTBatt_Rough->stUpgradeParam.byData[n][3], UPGRADE_SEED_LEN);
						break;
					}
				}

				ClearBattUpgradeRcvBuffer();
				
				pUpgradeParam->tm_StartUpgrade = _NOW_;
				pUpgradeParam->emUpgradeStatus = UPGRADE_EXCHANGE_KEY;

				TRACE("Info: BMS Reply _CAN_S_CMD_C2B_UNLOCK OK...\n");
				Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "Info:unlock OK->Exchanged Key!");

			}
			break;

		}
		case UPGRADE_EXCHANGE_KEY:
		{
			TRACE("Info: IMMU2 has been enter UPGRADE_EXCHANGE_KEY step...\n");
			nProcess = 253; // Please Wait
			
			if ((ABS(_NOW_ - pUpgradeParam->tm_StartUpgrade) > 120))
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
				TRACE("Err: UPGRADE_EXCHANGE_KEY Cmd Failure:tm_NOW=[%ld] tm_StartUpgrade=[%ld]\n", tm_NOW, pUpgradeParam->tm_StartUpgrade);
				LogOut(MAIN_MODULE_SAMPLER, "BAT-Updt", LOG_TYPE_ERROR, "UPGRADE_EXCHANGE_KEY Cmd Failure");
				break;
			}

			TRACE("Info: IMMU2 send cmmd _CAN_S_CMD_C2B_EXCHANGE_KEY\n");
			Send_C2B_UpgradeCmd(pSTBatt_Rough, _CAN_S_CMD_C2B_EXCHANGE_KEY);

			BOOL bResult = getBMSReplyResult(pSTBatt_Rough, UDS_SERVICE_EXCHANGE_KEY_27);
			if (bResult)
			{
				ClearBattUpgradeRcvBuffer();
				
				pUpgradeParam->tm_StartUpgrade = _NOW_;
				pUpgradeParam->emUpgradeStatus = UPGRADE_WRITE_REQUEST;

				TRACE("Info: BMS Reply _CAN_S_CMD_C2B_EXCHANGE_KEY OK...\n");
				Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "Info:Exchanged Key OK->Write request f198!");

			}

			break;
		}

		case UPGRADE_WRITE_REQUEST:
		{
			TRACE("Info: IMMU2 has been enter UPGRADE_WRITE_REQUEST step...\n");
			nProcess = 253; // Please Wait

			if ((ABS(_NOW_ - pUpgradeParam->tm_StartUpgrade) > 120))
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
				TRACE("Err: UPGRADE_WRITE_REQUEST Cmd Failure:tm_NOW=[%ld] tm_StartUpgrade=[%ld]\n", tm_NOW, pUpgradeParam->tm_StartUpgrade);
				LogOut(MAIN_MODULE_SAMPLER, "BAT-Updt", LOG_TYPE_ERROR, "UPGRADE_WRITE_REQUEST Cmd Failure");
				break;
			}

			TRACE("Info: IMMU2 send cmmd _CAN_S_CMD_C2B_WRITE_REQUEST\n");
			Send_C2B_UpgradeCmd(pSTBatt_Rough, _CAN_S_CMD_C2B_WRITE_REQUEST);

			BOOL bResult = getBMSReplyResult(pSTBatt_Rough, UDS_SERVICE_WRITE_REQUEST_2E);
			if (bResult)
			{
				ClearBattUpgradeRcvBuffer();
					
				pUpgradeParam->tm_StartUpgrade = _NOW_;
				pUpgradeParam->emUpgradeStatus = UPGRADE_WRITE_REQUEST_2;

				TRACE("Info: BMS Reply _CAN_S_CMD_C2B_WRITE_REQUEST OK...\n");
				Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "Info:Write request f198 OK->Write request f199!");

			}

			break;
		}

		case UPGRADE_WRITE_REQUEST_2:
		{
			TRACE("Info: IMMU2 has been enter UPGRADE_WRITE_REQUEST_2 step...\n");
			nProcess = 253; // Please Wait

			if ((ABS(_NOW_ - pUpgradeParam->tm_StartUpgrade) > 120))
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
				TRACE("Err: UPGRADE_WRITE_REQUEST_2 Cmd Failure:tm_NOW=[%ld] tm_StartUpgrade=[%ld]\n", tm_NOW, pUpgradeParam->tm_StartUpgrade);
				LogOut(MAIN_MODULE_SAMPLER, "BAT-Updt", LOG_TYPE_ERROR, "UPGRADE_WRITE_REQUEST_2 Cmd Failure");
				break;
			}

			TRACE("Info: IMMU2 send cmmd _CAN_S_CMD_C2B_WRITE_REQUEST_2\n");
			Send_C2B_UpgradeCmd(pSTBatt_Rough, _CAN_S_CMD_C2B_WRITE_REQUEST_2);

			BOOL bResult = getBMSReplyResult(pSTBatt_Rough, UDS_SERVICE_WRITE_REQUEST_2E);
			if (bResult)
			{
				ClearBattUpgradeRcvBuffer();
					
				pUpgradeParam->tm_StartUpgrade = _NOW_;
				pUpgradeParam->emUpgradeStatus = UPGRADE_ROUTINE_CONTROL;

				TRACE("Info: BMS Reply _CAN_S_CMD_C2B_WRITE_REQUEST_2 OK...\n");
				Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "Info:Write request f199 OK->Erase flash!");

			}

			break;
		}

		case UPGRADE_ROUTINE_CONTROL: 
		{
			TRACE("Info: IMMU2 has been enter UPGRADE_ROUTINE_CONTROL step...\n");
			nProcess = 253; // Please Wait
			if ((ABS(_NOW_ - pUpgradeParam->tm_StartUpgrade) > 120))
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
				TRACE("Err: UPGRADE_ROUTINE_CONTROL Cmd Failure:tm_NOW=[%ld] tm_StartUpgrade=[%ld]\n", tm_NOW, pUpgradeParam->tm_StartUpgrade);
				LogOut(MAIN_MODULE_SAMPLER, "BAT-Updt", LOG_TYPE_ERROR, "UPGRADE_ROUTINE_CONTROL Cmd Failure");
				break;
			}

			TRACE("Info: IMMU2 send cmmd _CAN_S_CMD_C2B_ROUTINE_CONTROL\n");
			Send_C2B_UpgradeCmd(pSTBatt_Rough, _CAN_S_CMD_C2B_ROUTINE_CONTROL);

			BOOL bResult = getBMSReplyResult(pSTBatt_Rough, UDS_SERVICE_ROUNTINE_CONTROL_31);
			if (bResult)
			{
				ClearBattUpgradeRcvBuffer();
					
				pUpgradeParam->tm_StartUpgrade = _NOW_;
				pUpgradeParam->emUpgradeStatus = UPGRADE_DOWNLOAD_REQUSET;

				TRACE("Info: BMS Reply _CAN_S_CMD_C2B_ROUTINE_CONTROL OK...\n");
				Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "Info:Erase flash OK->Download Request!");

			}

			break;
		}

		case UPGRADE_DOWNLOAD_REQUSET:
		{
			TRACE("Info: IMMU2 has been enter UPGRADE_DOWNLOAD_REQUSET step...\n");
			
			if ((ABS(_NOW_ - pUpgradeParam->tm_StartUpgrade) > 120))
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
				TRACE("Err: UPGRADE_DOWNLOAD_REQUSET Cmd Failure:tm_NOW=[%ld] tm_StartUpgrade=[%ld]\n", tm_NOW, pUpgradeParam->tm_StartUpgrade);
				LogOut(MAIN_MODULE_SAMPLER, "BAT-Updt", LOG_TYPE_ERROR, "UPGRADE_DOWNLOAD_REQUSET Cmd Failure");
				break;
			}

			TRACE("Info: IMMU2 send cmmd _CAN_S_CMD_C2B_DOWNLOAD_REQUSET\n");
			Send_C2B_UpgradeCmd(pSTBatt_Rough, _CAN_S_CMD_C2B_DOWNLOAD_REQUSET);

			BOOL bResult = getBMSReplyResult(pSTBatt_Rough, UDS_SERVICE_DOWNLOAD_REQUEST_34);
			if (bResult)
			{
				pSTBatt_Rough->stUpgradeParam.uMaxDownloadBlockLen = 0;
				
				for (n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
				{
					BYTE recCode = pSTBatt_Rough->stUpgradeParam.byData[n][1];
					if ((recCode == (UDS_SERVICE_DOWNLOAD_REQUEST_34 + UDS_REPLY_POSITIVE)))
					{
						pSTBatt_Rough->stUpgradeParam.uMaxDownloadBlockLen = pSTBatt_Rough->stUpgradeParam.byData[n][3] << 8 | pSTBatt_Rough->stUpgradeParam.byData[n][4];
						if(SYSTYPE_ESS_AIR==pSTBatt_Rough->lcCfg.emSystemType)
						{
							pSTBatt_Rough->stUpgradeParam.uMaxDownloadBlockLen -= 2;
						}
						else if(SYSTYPE_ESS_LIQUIDED == pSTBatt_Rough->lcCfg.emSystemType)
						{
							if((pUpgradeParam->eDevType != DEV_IMEU3) && (pUpgradeParam->eDevType != DEV_BSU)) // 液冷EU3和BSU OTA升级不需要 -2
							{
								pSTBatt_Rough->stUpgradeParam.uMaxDownloadBlockLen -= 2;
							}
						}

						break;
					}
				}

				ClearBattUpgradeRcvBuffer();
					
				pUpgradeParam->tm_StartUpgrade = _NOW_;
				pUpgradeParam->emUpgradeStatus = UPGRADE_DOWNLOAD_FILE;

				TRACE("Info: BMS Reply _CAN_S_CMD_C2B_DOWNLOAD_REQUSET OK...\n");
				Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "Info:Download Request OK->Download File, Upgrading...");

			}

			break;
		}
		case UPGRADE_DOWNLOAD_FILE:
		{
			TRACE("Info: IMMU2 has been enter UPGRADE_DOWNLOAD_FILE step...\n");
			//nProcess = 253; // Please Wait
			
			if ((ABS(_NOW_ - pUpgradeParam->tm_StartUpgrade) > 120))
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
				TRACE("Err: UPGRADE_DOWNLOAD_FILE Cmd Failure:tm_NOW=[%ld] tm_StartUpgrade=[%ld]\n", tm_NOW, pUpgradeParam->tm_StartUpgrade);
				LogOut(MAIN_MODULE_SAMPLER, "BAT-Updt", LOG_TYPE_ERROR, "UPGRADE_DOWNLOAD_FILE Cmd Failure");
				break;
			}
			

			TRACE("Info: IMMU2 send cmmd _CAN_S_CMD_C2B_DOWNLOAD_FILE\n");
			Send_C2B_UpgradeCmd(pSTBatt_Rough, _CAN_S_CMD_C2B_DOWNLOAD_FILE);
			
			nProcess = 95;
			
			BOOL bResult = getBMSReplyResult(pSTBatt_Rough, UDS_SERVICE_DOWNLOAD_36);

			if (bResult)
			{
				ClearBattUpgradeRcvBuffer();
					
				pUpgradeParam->tm_StartUpgrade = _NOW_;
				pUpgradeParam->emUpgradeStatus = UPGRADE_DOWNLOAD_EXIT;

				TRACE("Info: BMS Reply _CAN_S_CMD_C2B_DOWNLOAD_FILE OK...\n");
				Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "Info:Download File OK->Download Exit!");
			}
			else
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
				TRACE("Err: UPGRADE_DOWNLOAD_FILE Cmd Failure getBMSReplyResult false:tm_NOW=[%ld] tm_StartUpgrade=[%ld]\n", tm_NOW, pUpgradeParam->tm_StartUpgrade);
				LogOut(MAIN_MODULE_SAMPLER, "BAT-Updt", LOG_TYPE_ERROR, "UPGRADE_DOWNLOAD_FILE Cmd Failure getBMSReplyResult false");
				break;
			}

			break;
		}
		case UPGRADE_DOWNLOAD_EXIT:
		{
			TRACE("Info: IMMU2 has been enter UPGRADE_DOWNLOAD_EXIT step...\n");
			nProcess = 96; 

			if ((ABS(_NOW_ - pUpgradeParam->tm_StartUpgrade) > 120))
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
				TRACE("Err: UPGRADE_DOWNLOAD_EXIT Cmd Failure:tm_NOW=[%ld] tm_StartUpgrade=[%ld]\n", tm_NOW, pUpgradeParam->tm_StartUpgrade);
				LogOut(MAIN_MODULE_SAMPLER, "BAT-Updt", LOG_TYPE_ERROR, "UPGRADE_DOWNLOAD_EXIT Cmd Failure");
				break;
			}

			TRACE("Info: IMMU2 send cmmd _CAN_S_CMD_C2B_DOWNLOAD_EXIT\n");
			Send_C2B_UpgradeCmd(pSTBatt_Rough, _CAN_S_CMD_C2B_DOWNLOAD_EXIT);

			BOOL bResult = getBMSReplyResult(pSTBatt_Rough, UDS_SERVICE_DOWNLOAD_EXIT_37);
			if (bResult)
			{
				ClearBattUpgradeRcvBuffer();
					
				pUpgradeParam->tm_StartUpgrade = _NOW_;
				pUpgradeParam->emUpgradeStatus = UPGRADE_CHECK_FILE;

				TRACE("Info: BMS Reply _CAN_S_CMD_C2B_DOWNLOAD_EXIT OK...\n");
				Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "Info:Download Exit OK->Check File!");
			}		
			break;
		}

		case UPGRADE_CHECK_FILE:
		{
			TRACE("Info: IMMU2 has been enter UPGRADE_CHECK_FILE step...\n");
			nProcess = 97; 

			if ((ABS(_NOW_ - pUpgradeParam->tm_StartUpgrade) > 120))
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
				TRACE("Err: UPGRADE_CHECK_FILE Cmd Failure:tm_NOW=[%ld] tm_StartUpgrade=[%ld]\n", tm_NOW, pUpgradeParam->tm_StartUpgrade);
				LogOut(MAIN_MODULE_SAMPLER, "BAT-Updt", LOG_TYPE_ERROR, "UPGRADE_CHECK_FILE Cmd Failure");
				break;
			}
		
			TRACE("Info: IMMU2 send cmmd _CAN_S_CMD_C2B_CHECK_FILE\n");
			Send_C2B_UpgradeCmd(pSTBatt_Rough, _CAN_S_CMD_C2B_CHECK_FILE);

			BOOL bResult = getBMSReplyResult(pSTBatt_Rough, UDS_SERVICE_ROUNTINE_CONTROL_31);
			if (bResult)
			{
				ClearBattUpgradeRcvBuffer();
					
				pUpgradeParam->tm_StartUpgrade = _NOW_;
				pUpgradeParam->emUpgradeStatus = UPGRADE_CHECK_COMPATIBILITY;

				TRACE("Info: BMS Reply UDS_SERVICE_CHECK_FILE OK...\n");
				Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "Info:UPGRADE_CHECK_FILE OK->Compatibility Check");
			}				

			break;
		}
		case UPGRADE_CHECK_COMPATIBILITY:
		{
			TRACE("Info: IMMU2 has been enter UPGRADE_CHECK_COMPATIBILITY step...\n");
			nProcess = 98; 

			if ((ABS(_NOW_ - pUpgradeParam->tm_StartUpgrade) > 120))
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
				TRACE("Err: UPGRADE_CHECK_COMPATIBILITY Cmd Failure:tm_NOW=[%ld] tm_StartUpgrade=[%ld]\n", tm_NOW, pUpgradeParam->tm_StartUpgrade);
				LogOut(MAIN_MODULE_SAMPLER, "BAT-Updt", LOG_TYPE_ERROR, "UPGRADE_CHECK_COMPATIBILITY Cmd Failure");
				break;
			}

			TRACE("Info: IMMU2 send cmmd _CAN_S_CMD_C2B_CHECK_COMPATIBILITY\n");
			Send_C2B_UpgradeCmd(pSTBatt_Rough, _CAN_S_CMD_C2B_CHECK_COMPATIBILITY);

			BOOL bResult = getBMSReplyResult(pSTBatt_Rough, UDS_SERVICE_ROUNTINE_CONTROL_31);
			if (bResult)
			{
				ClearBattUpgradeRcvBuffer();
					
				pUpgradeParam->tm_StartUpgrade = _NOW_;
				pUpgradeParam->emUpgradeStatus = UPGRADE_RESET_ECU;

				TRACE("Info: BMS Reply _CAN_S_CMD_C2B_CHECK_COMPATIBILITY OK...\n");
				Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "Info:Compatibility Check OK->Reset ECU!");
			}
			

			break;
		}
		case UPGRADE_RESET_ECU:
		{
			TRACE("Info: IMMU2 has been enter UPGRADE_RESET_ECU step...\n");
			nProcess = 99; 

			if ((ABS(_NOW_ - pUpgradeParam->tm_StartUpgrade) > 120))
			{
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;
				TRACE("Err: UPGRADE_RESET_ECU Cmd Failure:tm_NOW=[%ld] tm_StartUpgrade=[%ld]\n", tm_NOW, pUpgradeParam->tm_StartUpgrade);
				break;
			}

			TRACE("Info: IMMU2 send cmmd _CAN_S_CMD_C2B_RESET_ECU\n");
			Send_C2B_UpgradeCmd(pSTBatt_Rough, _CAN_S_CMD_C2B_RESET_ECU);

			BOOL bResult = getBMSReplyResult(pSTBatt_Rough, UDS_SERVICE_RESET_ECU_11);
			if (bResult)
			{
				ClearBattUpgradeRcvBuffer();
					
				pUpgradeParam->tm_StartUpgrade = _NOW_;
				pUpgradeParam->emUpgradeStatus = UPGRADE_FINISHED;

				pUpgradeParam->bUpgradeResult = TRUE;
				TRACE("Info: BMS Reply _CAN_S_CMD_C2B_RESET_ECU OK...\n");
				Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "Info:Reset ECU OK, Upgrading Finished!");
			}		
			break;
		}

	
	case UPGRADE_FINISHED:
	{
		if (pUpgradeParam->bUpgradeResult)
		{
			nProcess = 254;
			Send_DebugMsg_2UI(pSTBatt_Rough, FALSE, FALSE, TRUE, "Info: BMS Upgrade Successfully.");
			TRACE("Info: BMS Upgrade Success\n");
		}
		else
		{
			nProcess = 255;
			Send_DebugMsg_2UI(pSTBatt_Rough, TRUE, FALSE, TRUE, "Info: BMS Upgrade Failed.");
			TRACE("Info: BMS Upgrade Failed\n");
		}

		VAR_VALUE varV;
		varV.ulValue = (unsigned long)_NOW_;
		Push_VarValue(SETTYPE_LOGIC_SP, pSTBatt_Rough->hSampler, 200, 83, varV);
		
   		snprintf(buffer, 64, "Idle");
	    Push_TextValue(SETTYPE_LOGIC_SP, pSTBatt_Rough->hSampler, 200 , 84, buffer, 64);

		pUpgradeParam->emUpgradeStatus = UPGRADE_IDLE;

	
		pSTBatt_Rough->stUpgradeParam.nLocalSendMsgId = 0;
		pSTBatt_Rough->stUpgradeParam.nLast_StartMsgId = 0;
		pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.nDataOffset = 0;
		pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.nTotalFileSize = 0;
		SAFELY_DELETE(pUpgradeParam->stUpdownFileInfo.pbyReadBuf);
		break;
	}
	default:
	{
		nProcess = 255; // Upgrade Failed
		pUpgradeParam->nRemtRunModel[n] = 0;
		pUpgradeParam->nRemtDownIndex[n] = 0;

		snprintf(buffer, 64, "Idle");
	    Push_TextValue(SETTYPE_LOGIC_SP, pSTBatt_Rough->hSampler, 200 , 84, buffer, 64);
		
		pSTBatt_Rough->stUpgradeParam.nLast_StartMsgId = 0;
		pSTBatt_Rough->stUpgradeParam.nLocalSendMsgId = 0;
		pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.nDataOffset = 0;
		pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.nTotalFileSize = 0;
		SAFELY_DELETE(pUpgradeParam->stUpdownFileInfo.pbyReadBuf);
		break;
	}
	}

	Update_BMSUpgradeProcess(pSTBatt_Rough, nProcess);
	
	return;
}


static BOOL CheckandRead_BinFile(STBATT_SAMPLER_ROUGH* pSTBatt_Rough)
{
	BATT_UPGRADE_PARAM *pUpgradeParam = &pSTBatt_Rough->stUpgradeParam;
	int emUpdtType = pSTBatt_Rough->blobUpdCmd.emUpdtType;

	char *szFile;

	if (emUpdtType == UPDT_TP_BMS)
	{
		szFile = pSTBatt_Rough->blobUpdCmd.szBMSFilePath;
	}
	else
	{
		return FALSE;
	}

	FILE *fp = fopen(szFile, "rb");
	if (fp == NULL)
	{
		TRACEX("Can NOT open the file[%s]\n", szFile);
		return FALSE;
	}

	fseek(fp, 0L, SEEK_END);
	size_t nFilelen = ftell(fp);
	if ((nFilelen < MIN_BINFILE_SIZE) || (nFilelen > MAX_BINFILE_SIZE))
	{
		TRACEX("FileLen[%d] is incorrect\n", nFilelen);
		fclose(fp);
		return FALSE;
	}

	// now allocate memory for the file
	BYTE *pBuff = NEW(BYTE, nFilelen);
	if (pBuff == NULL)
	{
		TRACEX("CAN NOT allocate memory for FileLen[%d]\n", nFilelen);
		fclose(fp);
		return FALSE;
	}
	// DO NOT forget to go back to file head!!!
	fseek(fp, 0L, SEEK_SET);
	//now read all data into memory
	fread(pBuff, nFilelen, 1, fp);
	fclose(fp);


	pUpgradeParam->stUpdownFileInfo.pbyReadBuf = pBuff;
	pUpgradeParam->stUpdownFileInfo.nTotalFileSize = nFilelen;
	pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.nDataOffset = 0;
	if (pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.nTotalFileSize % MAX_BMS_SINGLE_MSGLEN == 0)
	{
		pSTBatt_Rough->stUpgradeParam.nTotalMsgNum = pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.nTotalFileSize / MAX_BMS_SINGLE_MSGLEN;
	}
	else
	{
		pSTBatt_Rough->stUpgradeParam.nTotalMsgNum = pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.nTotalFileSize / MAX_BMS_SINGLE_MSGLEN + 1;
	}

	Init_UpgradeBMSParam(pSTBatt_Rough);

	TRACE("Read bin file[%s] successfully, and fileSize[%d]\n", szFile, nFilelen);

	return TRUE;
}


BOOL Prepare_BMSBin_Updte(IN STBATT_SAMPLER_ROUGH* pSTBatt_Rough)
{
	BOOL bRet = TRUE;
	//----Step1: now check and read the bin files DCDC/PFC/IMSU-D

	
	if(pSTBatt_Rough->blobUpdCmd.emUpdtType != UPDT_TP_BMS)
	{
		return FALSE;
	}

	bRet = CheckandRead_BinFile(pSTBatt_Rough);
	if (bRet == FALSE)
	{
		return FALSE;
	}

	TRACE("-------Now evrything is ready for BMS-updt, mode[%d], updtTy[%d]----\n",
		  pSTBatt_Rough->blobUpdCmd.emUpdtMode, pSTBatt_Rough->blobUpdCmd.emUpdtType);

	LogOut(MAIN_MODULE_SAMPLER, "BMS-Updt", LOG_TYPE_WARING,
		   "Ready for bms-upgrade, targetVer=>[%.3f]...", pSTBatt_Rough->blobUpdCmd.fBMS_SWVer_4Updt);

	return TRUE;
}
