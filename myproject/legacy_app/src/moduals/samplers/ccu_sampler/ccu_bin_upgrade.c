#include "ccu_sampler_main.h"

static void Update_RectUpgradeProcess(IN ROUGH_CCU_DATA* pCCU, int nProcess);

static void MakeShort(short sVal, OUT BYTE* pData)
{
	*(BYTE*)(pData) = (BYTE)(sVal >> 8);
	*(BYTE*)(pData + 1) = (BYTE)(sVal & 0xFF);
}

static void MakeUlong(ULONG ulVal, OUT BYTE* pData)
{
	*(BYTE*)(pData)= (BYTE)(ulVal >> 24);
	*(BYTE*)(pData + 1) = (BYTE)(ulVal >> 16);
	*(BYTE*)(pData + 2) = (BYTE)(ulVal >> 8);
	*(BYTE*)(pData + 3) = (BYTE)(ulVal & 0xFF);
}

typedef union tagShortVVV
{
	short 	sVal;
	BYTE 	byVal[2];
}SHORT_VAR;

static short Bytes2Short(BYTE* pBytes, int nBytes)
{
	int nFlag = 1;
	SHORT_VAR s_Val;

	if((pBytes[0] & 0x80) == 0x80)
	{
		//TRACE("Parse Negtive Value\n");
		nFlag = -1;
		s_Val.byVal[1] = (~pBytes[0]) & 0x7F;
		s_Val.byVal[0] = (~pBytes[1]) + 1;
	}
	else 
	{
		//TRACE("Parse Positive Value\n");
		nFlag = 1;
		s_Val.byVal[1] = pBytes[0];
		s_Val.byVal[0] = pBytes[1];
	}
	//TRACE("pBytes[0]=%02X, pBytes[1]=%02X s_Val.sVal=%d\n", pBytes[0], pBytes[1], s_Val.sVal);
	return nFlag*s_Val.sVal;
}

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

static ULONG MakeFileChkSum(ULONG usDataLen, BYTE* byData)
{
	ULONG ulCalcSum = 0;
	int n = 0;
	for(n = 0; n < usDataLen; n++)
	{
		ulCalcSum += byData[n];
	}

	ulCalcSum &= 0xFFFF;
	//ulCalcSum = (~ulCalcSum) + 1;

	return ulCalcSum;
}

static void CAN_StuffHead4PCSUpgrade(BYTE byPortId, BYTE byMsgId, BYTE byDestAddr, BYTE bySrcAddr, BYTE *pbyFrame)
{
	*(pbyFrame + 0) = byPortId;
	*(pbyFrame + 1) = byMsgId;
	*(pbyFrame + 2) = byDestAddr;
	*(pbyFrame + 3) = bySrcAddr;	
}

static USHORT u16crcTable[260];
static USHORT u16Calc1CRC(BYTE* byData, USHORT usLen)
{
	int i;

	// cal the crc
	USHORT uiCRC = 0;
	USHORT index = 0;
	for (i = 0; i < usLen; i++)
	{
		index = (USHORT)((USHORT)(uiCRC ^ byData[i]) & 0x00FF);
		uiCRC = (USHORT)((uiCRC >> 8) ^ u16crcTable[index]);
	}

    return uiCRC;
}

#define	SEND_DATA_CYCLE	6

static BOOL SendModuleSingleMsg(ROUGH_CCU_DATA* pCCU)
{
	CCU_UPGRADE_PARAM* pUpgradeParam = &pCCU->tagCCUUpgradeParam;
	BYTE* pbyUpfateFile = pUpgradeParam->strUpdownFileInfo.pbyReadBuf;
	USHORT	usCrc;
	int i=0;
	int nRet=0;

	int n;// for printf

	BYTE byModuleId = pUpgradeParam->strEmsUpCcuInfo.byCurrentModuleId;

	BYTE pbySendBuf[32]={'\0'};
	BYTE byUpdate_Data[10]={'\0'};	

	switch(pUpgradeParam->strEmsUpCcuInfo.emUpdateStep)
	{
		case DOWNLOAD_SLIENCE: // 多个模块升级 下发静默 报文
		{
			if(pUpgradeParam->strEmsUpCcuInfo.uSlience == 0)
			{
				byUpdate_Data[0] = 0x11;
				byUpdate_Data[1] = 0x1E;
				byUpdate_Data[7] = 0xA2;
				CAN_StuffHead4PCSUpgrade(0x02, 0xA4, 0x3F, 0xF0, pbySendBuf);
				memcpy(pbySendBuf + 4, byUpdate_Data, 8);
				// DoTCPSession(pCCU, NL_MSG_PCSSET, pbySendBuf, 12, SEND_DATA_CYCLE);
				// byUpdate_Data[7] = 0xA1;
				// memcpy(pbySendBuf + 4, byUpdate_Data, 8);
				for(i=0; i<3; ++i)
				{
					DoTCPSession(pCCU, NL_MSG_PCSSET, pbySendBuf, 12, SEND_DATA_CYCLE); // 建议发3次
				}
				
				pUpgradeParam->strEmsUpCcuInfo.uSlience = 1;
				
			}
			else if(pUpgradeParam->strEmsUpCcuInfo.uSlience == 2)
			{
				pUpgradeParam->strEmsUpCcuInfo.emUpdateStep = DOWNLOAD_TRIGGER;
			}
		}
			break;
		case DOWNLOAD_TRIGGER:
		{
			if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_DCDC)
			{
				byUpdate_Data[7] = 0x01;
			}
			else if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_PFC)
			{
				byUpdate_Data[7] = 0x02;
			}
		
			CAN_StuffHead4PCSUpgrade(0x02, 0x7F, byModuleId, 0xF0, pbySendBuf);
			memcpy(pbySendBuf + 4, byUpdate_Data, 8);
			DoTCPSession(pCCU, NL_MSG_PCSSET, pbySendBuf, 12, SEND_DATA_CYCLE);
			pUpgradeParam->strEmsUpCcuInfo.emUpdateStep = DOWNLOAD_SEND;
			// 等待收到 1. 027FF000 00 00 00 00 00 00 00 01/02
			// 等待收到 2. 027FF000 00 00 00 00 00 06 00 01/02, 存储 00 00 00 06 地址值存
		}
			break;
		case DOWNLOAD_SEND:
		{
			if(pUpgradeParam->strEmsUpCcuInfo.uTrigger2 == 1) // uTrigger2 只有 1、2、3 三种情况
			{	// 1 只会读取 6 字节
				memcpy(byUpdate_Data, pbyUpfateFile + pUpgradeParam->strEmsUpCcuInfo.uAddrCalc, 6);
				USHORT usNeedReadLen = byUpdate_Data[4]<<8 | byUpdate_Data[5];
				usCrc = u16Calc1CRC(pbyUpfateFile+pUpgradeParam->strEmsUpCcuInfo.uAddrCalc+pUpgradeParam->strEmsUpCcuInfo.usReadLenCalc, usNeedReadLen);// (crc校验);
				byUpdate_Data[6] = usCrc >> 8;
				byUpdate_Data[7] = usCrc & 0xFF;
				CAN_StuffHead4PCSUpgrade(0x1F, pUpgradeParam->strEmsUpCcuInfo.byPCScmd, byModuleId, 0xF0, pbySendBuf); // DC/PFC 下载canid 00 是addr
				memcpy(pbySendBuf + 4, byUpdate_Data, 8);
				DoTCPSession(pCCU, NL_MSG_PCSSET, pbySendBuf, 12, SEND_DATA_CYCLE);
				pUpgradeParam->strEmsUpCcuInfo.nHasSendLen = pUpgradeParam->strEmsUpCcuInfo.uAddrCalc+pUpgradeParam->strEmsUpCcuInfo.usReadLenCalc;
				
				pUpgradeParam->strEmsUpCcuInfo.emUpdateStep = DOWNLOAD_WAIT;

			}
			else if(pUpgradeParam->strEmsUpCcuInfo.uTrigger2 == 2)
			{	// 2 可能读取 0x02~0x100 个字节
				if((pUpgradeParam->strEmsUpCcuInfo.usReadLenCalc / 8) > 0)
				{
					pUpgradeParam->strEmsUpCcuInfo.usReadLenCalc -= 8;
					if(pUpgradeParam->strEmsUpCcuInfo.usReadLenCalc == 0)
					{
						pUpgradeParam->strEmsUpCcuInfo.emUpdateStep = DOWNLOAD_WAIT;
					}

					memcpy(byUpdate_Data, pbyUpfateFile+pUpgradeParam->strEmsUpCcuInfo.uAddrCalc, 8);
					CAN_StuffHead4PCSUpgrade(0x1F, pUpgradeParam->strEmsUpCcuInfo.bySendStartAddr++, byModuleId, 0xF0, pbySendBuf);
					memcpy(pbySendBuf + 4, byUpdate_Data, 8);
					DoTCPSession(pCCU, NL_MSG_PCSSET, pbySendBuf, 12, SEND_DATA_CYCLE);
					pUpgradeParam->strEmsUpCcuInfo.uAddrCalc += 8;
					pUpgradeParam->strEmsUpCcuInfo.nHasSendLen = pUpgradeParam->strEmsUpCcuInfo.uAddrCalc;
				}
				else if(pUpgradeParam->strEmsUpCcuInfo.usReadLenCalc % 8 > 0) // 还剩几个字节没传完
				{	
					memcpy(byUpdate_Data, pbyUpfateFile+pUpgradeParam->strEmsUpCcuInfo.uAddrCalc, pUpgradeParam->strEmsUpCcuInfo.usReadLenCalc);
					CAN_StuffHead4PCSUpgrade(0x1F, pUpgradeParam->strEmsUpCcuInfo.bySendStartAddr++, byModuleId, 0xF0, pbySendBuf);
					memcpy(pbySendBuf + 4, byUpdate_Data, 8);
					pUpgradeParam->strEmsUpCcuInfo.uAddrCalc += pUpgradeParam->strEmsUpCcuInfo.usReadLenCalc;
					pUpgradeParam->strEmsUpCcuInfo.nHasSendLen = pUpgradeParam->strEmsUpCcuInfo.uAddrCalc;
					pUpgradeParam->strEmsUpCcuInfo.usReadLenCalc = 0;
					pUpgradeParam->strEmsUpCcuInfo.emUpdateStep = DOWNLOAD_WAIT;
					DoTCPSession(pCCU, NL_MSG_PCSSET, pbySendBuf, 12, SEND_DATA_CYCLE);
				}

			}
			else if(pUpgradeParam->strEmsUpCcuInfo.uTrigger2 == 3)
			{	// 数据包传输Err，回到 2重发
				pUpgradeParam->strEmsUpCcuInfo.uTrigger2 = 2;

				if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_DCDC)
				{
					pUpgradeParam->strEmsUpCcuInfo.bySendStartAddr = 0x10 + (pUpgradeParam->strEmsUpCcuInfo.uAddr03 - pUpgradeParam->strEmsUpCcuInfo.uAddr) / 8; 
				}
				else if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_PFC)
				{
					pUpgradeParam->strEmsUpCcuInfo.bySendStartAddr = 0x50 + (pUpgradeParam->strEmsUpCcuInfo.uAddr03 - pUpgradeParam->strEmsUpCcuInfo.uAddr) / 8; 
				}

				pUpgradeParam->strEmsUpCcuInfo.uAddrCalc = pUpgradeParam->strEmsUpCcuInfo.uAddr03;
				pUpgradeParam->strEmsUpCcuInfo.usReadLenCalc = pUpgradeParam->strEmsUpCcuInfo.usReadLen03;

			}
		}				
			break;
		case DOWNLOAD_WAIT:
		{
			if(pUpgradeParam->strEmsUpCcuInfo.nHasSendLen >= pUpgradeParam->strUpdownFileInfo.nTotalFileSize)
			{
				// printf("Upgrade over\n");
				pUpgradeParam->strEmsUpCcuInfo.byHasUpModNum++;
				pUpgradeParam->strEmsUpCcuInfo.emUpdateStep = DOWNLOAD_CHECK_UP_NUM;
			}

			Sleep(SEND_DATA_CYCLE);

		}
			break;
		case DOWNLOAD_CHECK_UP_NUM:
		{
			if(pUpgradeParam->strEmsUpCcuInfo.byHasUpModNum >= pCCU->pNewSet.ccuSettings.byNumGrp_1) // 全部模块都升级完了
			{
				pUpgradeParam->strEmsUpCcuInfo.emUpdateStep = DOWNLOAD_SUCCESS;
			}
			else // 还有未升级的，恢复一下参数继续
			{
				pUpgradeParam->strEmsUpCcuInfo.uSlience = 0;
				pUpgradeParam->strEmsUpCcuInfo.nHasSendLen = 0;
				// pUpgradeParam->strEmsUpCcuInfo.uTrigger1 = 0; // 恢复
				pUpgradeParam->strEmsUpCcuInfo.uTrigger2 = 0; // 恢复

				pUpgradeParam->strEmsUpCcuInfo.byCurrentModuleId = pUpgradeParam->strEmsUpCcuInfo.byHasUpModNum;

				pUpgradeParam->strEmsUpCcuInfo.emUpdateStep = DOWNLOAD_SLIENCE;

				Sleep(1000); // 等待第一个模块进入app发送0757，再进入静默
			}
		}

			break;
		case DOWNLOAD_SUCCESS:
		{
			// 关闭静默模式
			byUpdate_Data[0] = 0x11;
			byUpdate_Data[1] = 0x1E;
			byUpdate_Data[7] = 0xA0;
			CAN_StuffHead4PCSUpgrade(0x02, 0xA4, 0x3F, 0xF0, pbySendBuf);
			memcpy(pbySendBuf + 4, byUpdate_Data, 8);
			pUpgradeParam->strEmsUpCcuInfo.uSlience = 0;
			
			DoTCPSession(pCCU, NL_MSG_PCSSET, pbySendBuf, 12, SEND_DATA_CYCLE);

			pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus = FIRMUPDATE_Downloaded; // 跳出外层状态机

			pUpgradeParam->strEmsUpCcuInfo.emUpdateStep = DOWNLOAD_FINISH;
			
			nRet = ERR_CONTROL_OK;
		}
			break;

		default:
			break;
	}

	// printf("UpgradeProcess->[%x]/[%x]\n", 
	// printf("UpgradeProcess->[%d]/[%d]\n", 
	// pUpgradeParam->strEmsUpCcuInfo.nHasSendLen, 
	// pUpgradeParam->strUpdownFileInfo.nTotalFileSize);

	pCCU->nUpgradeProcess = ((pUpgradeParam->strEmsUpCcuInfo.byHasUpModNum*pUpgradeParam->strUpdownFileInfo.nTotalFileSize
								+ pUpgradeParam->strEmsUpCcuInfo.nHasSendLen)*100) / (pUpgradeParam->strUpdownFileInfo.nTotalFileSize * pCCU->pNewSet.ccuSettings.byNumGrp_1);
	Update_RectUpgradeProcess(pCCU, pCCU->nUpgradeProcess);


	BOOL bRet = (nRet == ERR_CONTROL_OK) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;

	return bRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Send_DebugMsg_2UI
*  CREATOR       : John
*  CREATED DATE  : 2023-01-17 11:38:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Send_DebugMsg_2UI(IN ROUGH_CCU_DATA* pCCU, BOOL bFinish, BOOL bError, BOOL bLog, char *pszFormat, ...)
{
	EVENT_UI_NOTICE evNote;
	ZERO_POBJS(&evNote, 1);
	evNote.emNotoceType = UI_NOTICETYPE_IMSU_UPDATE;
	evNote.byData[0] = bFinish; //LCD will judge if need to end debug
	char szDebug[128];
	va_list arglist;
	va_start(arglist, pszFormat);
	vsnprintf(szDebug, 128, pszFormat, arglist);
	va_end(arglist);
	memcpy(&evNote.byData[1], szDebug, 128);
	
	if(bLog)
	{
		LogOut(MAIN_MODULE_SAMPLER, "CCUTcp-Upgrade", 
			bError ? LOG_TYPE_ERROR : LOG_TYPE_WARING, szDebug);
	}
	static time_t tmLastSendMsg = 0;
	time_t tmNow = _NOW_;
	if(tmNow - tmLastSendMsg < 2)
	{
		//avoid send msg too quickly
		Sleep(1000);
	}
	tmLastSendMsg = _NOW_;

	SendMsg(pCCU->hSampler, EVENT_TYPE_UI_NOTICE, EVENT_MSG_BEGIN, 
		&evNote, sizeof(EVENT_UI_NOTICE));
}

static BOOL SendIMSUSingleMsg(ROUGH_CCU_DATA* pCCU)
{
	CCU_UPGRADE_PARAM* pUpgradeParam = &pCCU->tagCCUUpgradeParam; 

	BOOL 	bRet = TRUE;
	ULONG 	ulSendLen = 0;
	UINT  	uiMsgId = 0;
	BYTE 	*pDataPos = NULL;
#ifdef IMMU3
	BYTE 	byData[CCU_BUFF_LEN_232];
	memset(byData, 0, CCU_BUFF_LEN_232);
#else
	BYTE 	byData[TCP_BUFF_LEN];
	memset(byData, 0, TCP_BUFF_LEN);
#endif
	
	uiMsgId = pUpgradeParam->tagMstUpgradeParam.nMsgId;  
	pDataPos = pUpgradeParam->strUpdownFileInfo.pbyReadBuf + pUpgradeParam->strUpdownFileInfo.nDataOffset;
	if(pDataPos == NULL)
	{
		TRACEX("Err: pDataPos is NULL nDataOffset=[%d]!\n",pUpgradeParam->strUpdownFileInfo.nDataOffset);
		return FALSE;
	}

	MakeShort(uiMsgId, byData);
    ulSendLen += 2;

    memcpy(&byData[ulSendLen], pDataPos, pUpgradeParam->ulSingleMsgLen);
	ulSendLen += pUpgradeParam->ulSingleMsgLen;

	//-----Step2. Send Data, Then Wait 300ms For CCU to Recv Data-----// 
	int nRet = DoTCPSession(pCCU, NL_MSG_UDTA, byData, ulSendLen, 300) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;

	bRet = (nRet == ERR_CONTROL_OK) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;

	return bRet;
}

#ifdef IMMU3
	#define CYCLE_SEND_TIMES 	1000
	#define MAX_IMSU_SINGLE_MSGLEN 	200
#else
	#define CYCLE_SEND_TIMES 	10
	#define MAX_IMSU_SINGLE_MSGLEN 	700
#endif

static BOOL  SendUpgradeIMSUFile(ROUGH_CCU_DATA* pCCU)
{
	SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
	CCU_UPGRADE_PARAM* pUpgradeParam = &pCCU->tagCCUUpgradeParam; 

	ULONG ulRemainLen = 0;
	BOOL bRet = TRUE;
	int n = 0;

	//Recalculate the MsgId And DataOffset of The BinFile
	if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_IMSU)
	{
	pUpgradeParam->tagMstUpgradeParam.nMsgId = pUpgradeParam->tagMstUpgradeParam.nRecvMsgId + 1;
	pUpgradeParam->strUpdownFileInfo.nDataOffset = MAX_IMSU_SINGLE_MSGLEN * pUpgradeParam->tagMstUpgradeParam.nRecvMsgId;

	TRACE("---StartTime=%ld--- nFileDataLen =%d nDataOffset=%d\n", _NOW_, 
		pUpgradeParam->strUpdownFileInfo.nTotalFileSize, pUpgradeParam->strUpdownFileInfo.nDataOffset);
	for(n = 0; n < CYCLE_SEND_TIMES; n++)
	{
		// printf("DataOffset = [%d] TotalFileSize = [%d]\n", pUpgradeParam->strUpdownFileInfo.nDataOffset, pUpgradeParam->strUpdownFileInfo.nTotalFileSize);
		if(pUpgradeParam->strUpdownFileInfo.nDataOffset < pUpgradeParam->strUpdownFileInfo.nTotalFileSize)
		{
			ulRemainLen = pUpgradeParam->strUpdownFileInfo.nTotalFileSize - pUpgradeParam->strUpdownFileInfo.nDataOffset;
			pUpgradeParam->ulSingleMsgLen = (ulRemainLen > MAX_IMSU_SINGLE_MSGLEN) ? MAX_IMSU_SINGLE_MSGLEN : ulRemainLen;

			bRet = SendIMSUSingleMsg(pCCU);
			if(bRet == ERR_CONTROL_FAILED)
			{
				n--;
				continue;
			}
			pUpgradeParam->tagMstUpgradeParam.nMsgId++;
			pUpgradeParam->strUpdownFileInfo.nDataOffset += pUpgradeParam->ulSingleMsgLen;
			
			TRACE("nFileDataLen=[%d] nRemainLen=[%d]nSingleMsgLen=[%d]nDataOffset=[%d]\n", \
				pUpgradeParam->strUpdownFileInfo.nTotalFileSize,ulRemainLen,\
				pUpgradeParam->ulSingleMsgLen, pUpgradeParam->strUpdownFileInfo.nDataOffset);	
		}
		else
		{
			TRACE("Info: nTotalFileSize=%d nDataOffset=%d\n", \
				pUpgradeParam->strUpdownFileInfo.nTotalFileSize, pUpgradeParam->strUpdownFileInfo.nDataOffset);
			break;
		}

#ifdef IMMU3
			pCCU->nUpgradeProcess = 100*pUpgradeParam->strUpdownFileInfo.nDataOffset/(pUpgradeParam->strUpdownFileInfo.nTotalFileSize+1);
			Update_RectUpgradeProcess(pCCU, pCCU->nUpgradeProcess);
#endif
		}
	}
	else // Module Update
	{

		pUpgradeParam->tagMstUpgradeParam.nMsgId = pUpgradeParam->tagMstUpgradeParam.nRecvMsgId + 1;
		pUpgradeParam->strUpdownFileInfo.nDataOffset = MAX_IMSU_SINGLE_MSGLEN * pUpgradeParam->tagMstUpgradeParam.nRecvMsgId;
	
		TRACE("---StartTime=%ld--- nFileDataLen =%d nDataOffset=%d\n", _NOW_, 
			pUpgradeParam->strUpdownFileInfo.nTotalFileSize, pUpgradeParam->strUpdownFileInfo.nDataOffset);
		for(n = 0; n < CYCLE_SEND_TIMES; n++)
		{
			// printf("DataOffset = [%d] TotalFileSize = [%d]\n", pUpgradeParam->strUpdownFileInfo.nDataOffset, pUpgradeParam->strUpdownFileInfo.nTotalFileSize);
			if(pUpgradeParam->strUpdownFileInfo.nDataOffset < pUpgradeParam->strUpdownFileInfo.nTotalFileSize)
			{
				ulRemainLen = pUpgradeParam->strUpdownFileInfo.nTotalFileSize - pUpgradeParam->strUpdownFileInfo.nDataOffset;
				pUpgradeParam->ulSingleMsgLen = (ulRemainLen > MAX_IMSU_SINGLE_MSGLEN) ? MAX_IMSU_SINGLE_MSGLEN : ulRemainLen;
	
				bRet = SendIMSUSingleMsg(pCCU);
				if(bRet == ERR_CONTROL_FAILED)
				{
					n--;
					continue;
				}
				pUpgradeParam->tagMstUpgradeParam.nMsgId++;
				pUpgradeParam->strUpdownFileInfo.nDataOffset += pUpgradeParam->ulSingleMsgLen;
				
				TRACE("nFileDataLen=[%d] nRemainLen=[%d]nSingleMsgLen=[%d]nDataOffset=[%d]\n", \
					pUpgradeParam->strUpdownFileInfo.nTotalFileSize,ulRemainLen,\
					pUpgradeParam->ulSingleMsgLen, pUpgradeParam->strUpdownFileInfo.nDataOffset);	
			}
			else
			{
				TRACE("Info: nTotalFileSize=%d nDataOffset=%d\n", \
					pUpgradeParam->strUpdownFileInfo.nTotalFileSize, pUpgradeParam->strUpdownFileInfo.nDataOffset);
				break;
			}
	
			#ifdef IMMU3
				pCCU->nUpgradeProcess = 100*pUpgradeParam->strUpdownFileInfo.nDataOffset/(pUpgradeParam->strUpdownFileInfo.nTotalFileSize+1);
				Update_RectUpgradeProcess(pCCU, pCCU->nUpgradeProcess);
			#endif
		}
	}

 	return bRet;
}


static void SendUSIZCmdToCCU(ROUGH_CCU_DATA* pCCU)
{
	CCU_UPGRADE_PARAM* pUpgradeParam = &pCCU->tagCCUUpgradeParam; 

	BYTE 	byData[50]={'\0'};
	int 	nStuffLen = 0;
	
	MakeUlong(pUpgradeParam->strUpdownFileInfo.nTotalFileSize, byData);
	nStuffLen += 4;

	byData[nStuffLen] = (BYTE)pCCU->blobUpdCmd.emUpdtType;

#ifdef IMMU3
	if(pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus == UPGRADE_FINISHED)
	{
		// printf("USIZ:0xFF\n");
		byData[nStuffLen] = 0xFF;		
	}
#endif

	nStuffLen += 1;

	TRACE("nTotalFileSize=[%d], emUpdtType=[%d] fDCDC=[%f] fPFC=[%f] fIMSU=[%f] fBMS=[%f]\n", \
		pUpgradeParam->strUpdownFileInfo.nTotalFileSize, pCCU->blobUpdCmd.emUpdtType,\
		pCCU->blobUpdCmd.fDCDC_SWVer_4Updt,\
		pCCU->blobUpdCmd.fPFC_SWVer_4Updt,\
		pCCU->blobUpdCmd.fIMSU_SWVer_4Updt,\
		pCCU->blobUpdCmd.fBMS_SWVer_4Updt);

	char szBinFileName[32]="\0";
	switch (pCCU->blobUpdCmd.emUpdtType)
	{
		case UPDT_TP_IMSU:
			memcpy(byData + 5, "IMSU_APP.bin", 32);
			break;
		case UPDT_TP_PFC:
			// if(pCCU->blobUpdCmd.fPFC_SWVer_4Updt <= 0.1)
			// {
			// 	return;
			// }
			snprintf(szBinFileName, 32, "V%.0f_U1.bin", pCCU->blobUpdCmd.fPFC_SWVer_4Updt);
			memcpy(byData + 5, szBinFileName, 32);
			break;
		case UPDT_TP_DCDC:
			// if(pCCU->blobUpdCmd.fDCDC_SWVer_4Updt <= 0.1)
			// {
			// 	return;
			// }
			snprintf(szBinFileName, 32, "V%.0f_U2.bin", pCCU->blobUpdCmd.fDCDC_SWVer_4Updt);
			memcpy(byData + 5, szBinFileName, 32);
			break;
		
		case UPDT_TP_ABU_U1:
			// if(pCCU->blobUpdCmd.fPFC_SWVer_4Updt <= 0.1)
			// {
			// 	return;
			// }
			snprintf(szBinFileName, 32, "V%.0f_ABU_U1.bin", pCCU->blobUpdCmd.fPFC_SWVer_4Updt);
			memcpy(byData + 5, szBinFileName, 32);
			break;

		case UPDT_TP_ABU_U2:
			// if(pCCU->blobUpdCmd.fDCDC_SWVer_4Updt <= 0.1)
			// {
			// 	return;
			// }
			snprintf(szBinFileName, 32, "V%.0f_ABU_U2.bin", pCCU->blobUpdCmd.fDCDC_SWVer_4Updt);
			memcpy(byData + 5, szBinFileName, 32);
			break;

		case UPDT_TP_EU2_1 :
			//if(pCCU->blobUpdCmd.fPFC_SWVer_4Updt <= 0.1)
			//{
			//	return;
			//}
			snprintf(szBinFileName, 32, "V%.0f_EU2_1.bin", pCCU->blobUpdCmd.fPFC_SWVer_4Updt);
			memcpy(byData + 5, szBinFileName, 32);
			break;
		case UPDT_TP_EU2_2 :
			//if(pCCU->blobUpdCmd.fPFC_SWVer_4Updt <= 0.1)
			//{
			//	return;
			//}
			snprintf(szBinFileName, 32, "V%.0f_EU2_2.bin", pCCU->blobUpdCmd.fPFC_SWVer_4Updt);
			memcpy(byData + 5, szBinFileName, 32);
			break;
		case UPDT_TP_PFC_BOOT :
			snprintf(szBinFileName, 32, "V%.0f_PFC_BOOT.bin", pCCU->blobUpdCmd.fPFC_SWVer_4Updt);
			memcpy(byData + 5, szBinFileName, 32);
			break;
		case UPDT_TP_DCDC_BOOT :
			snprintf(szBinFileName, 32, "V%.0f_DCDC_BOOT.bin", pCCU->blobUpdCmd.fDCDC_SWVer_4Updt);
			memcpy(byData + 5, szBinFileName, 32);
			break;
		case UPDT_TP_MPPT_U1 :
			snprintf(szBinFileName, 32, "V%.0f_MPPT_U1.bin", pCCU->blobUpdCmd.fPFC_SWVer_4Updt);
			memcpy(byData + 5, szBinFileName, 32);
			break;
		case UPDT_TP_MPPT_U2 :
			snprintf(szBinFileName, 32, "V%.0f_MPPT_U2.bin", pCCU->blobUpdCmd.fDCDC_SWVer_4Updt);
			memcpy(byData + 5, szBinFileName, 32);
			break;
		default:
			return;
			break;
	}
	//char *pFind = strstr(pCCU->blobUpdCmd.szDCDCFilePath, "V");
	/*if((pFind != NULL) 
	|| (pCCU->blobUpdCmd.emUpdtType == )
	|| (pCCU->blobUpdCmd.emUpdtType == UPDT_TP_PFC)
	|| (pCCU->blobUpdCmd.emUpdtType == UPDT_TP_DCDC))
	{
		if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_IMSU)
		{
		}
		else if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_PFC)
		{
			//memcpy(byData + 5, pFind + 1, 32);
			memcpy(byData + 5, "V167_U1.bin", 32);
		}
		else if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_DCDC)
		{
			memcpy(byData + 5, "V267_U2.bin", 32);
		}
		else
		{
			return;
		}*/

		nStuffLen += 32;

		ULONG ulCalcSum = MakeFileChkSum(pUpgradeParam->strUpdownFileInfo.nTotalFileSize, pUpgradeParam->strUpdownFileInfo.pbyReadBuf);
		byData[nStuffLen] = (BYTE)(ulCalcSum >> 8);
		nStuffLen += 1;

		byData[nStuffLen] = (BYTE)(ulCalcSum & 0xFF);
		nStuffLen += 1;

		TRACE("szBinFileName=%s byData=%s ulCalcSum=%d nStuffLen=%d\n",\
			szBinFileName, byData, ulCalcSum, nStuffLen);
		// printf("szBinFileName=[%s] byData=%s ulCalcSum=%d nStuffLen=%d\n",\
		// 	szBinFileName, byData, ulCalcSum, nStuffLen);
		LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_INFO, "szBinFileName=[%s] byData=%s ulCalcSum=%d nStuffLen=%d\n",\
			szBinFileName, byData, ulCalcSum, nStuffLen);

		int nRet = DoTCPSession(pCCU, NL_MSG_USIZ, byData, nStuffLen, 500) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;
	//}
	//else
	//{
	//	printf("Error: CAnt Not Find U1 or U2!!!\n");
	//}
	return;
}

static void InitUpgradeParam(ROUGH_CCU_DATA* pCCU)
{
	CCU_UPGRADE_PARAM* pUpgradeParam = &pCCU->tagCCUUpgradeParam; 

	pUpgradeParam->tagMstUpgradeParam.nRecvMsgId = 0;
	pUpgradeParam->tagMstUpgradeParam.nMsgId = 0;
	pUpgradeParam->tagMstUpgradeParam.bInterSlvReady4Upgrade = FALSE;
	pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus = FIRMUPDATE_Idle;  
	pUpgradeParam->strUpdownFileInfo.nDataOffset = 0;

	// bmu03
	pUpgradeParam->strEmsUpCcuInfo.uSlience = 0;
	// pUpgradeParam->strEmsUpCcuInfo.uTrigger1 = 0;
	pUpgradeParam->strEmsUpCcuInfo.uTrigger2 = 0;

	pUpgradeParam->strEmsUpCcuInfo.uAddr03 = 0;
	pUpgradeParam->strEmsUpCcuInfo.usReadLen03 = 0;

	pUpgradeParam->strEmsUpCcuInfo.uAddr = 0;
	pUpgradeParam->strEmsUpCcuInfo.uAddrCalc = 0;

	pUpgradeParam->strEmsUpCcuInfo.usReadLen = 0;
	pUpgradeParam->strEmsUpCcuInfo.usReadLenCalc = 0;
	
	pUpgradeParam->strEmsUpCcuInfo.byCurrentModuleId = 0;
	pUpgradeParam->strEmsUpCcuInfo.nHasSendLen = 0;
	pUpgradeParam->strEmsUpCcuInfo.byHasUpModNum = 0;

	if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_DCDC)
	{
		pUpgradeParam->strEmsUpCcuInfo.byPCScmd = 0x08;
		pUpgradeParam->strEmsUpCcuInfo.bySendStartAddr = 0x10; // 10~2F
	}
	else if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_PFC)
	{
		pUpgradeParam->strEmsUpCcuInfo.byPCScmd = 0x48;
		pUpgradeParam->strEmsUpCcuInfo.bySendStartAddr = 0x50; // 50~6F
	}

	
	return;
}

static int ReadBinFileFromPath4USB(ROUGH_CCU_DATA* pCCU)
{
	CCU_UPGRADE_PARAM* pUpgradeParam = &pCCU->tagCCUUpgradeParam; 

	FILE *fp = fopen(pCCU->blobUpdCmd.szDCDCFilePath, "r");
	if(fp == NULL)
	{
  		TRACE("Err: Open File=[%s] Fail!\n", pCCU->blobUpdCmd.szDCDCFilePath);
		return -1;
	}

	fseek(fp, 0L, SEEK_END);
	size_t nFilelen = ftell(fp);
	if((nFilelen < MIN_BINFILE_SIZE) || (nFilelen > MAX_BINFILE_SIZE))
	{
		TRACEX("Err: FileLen[%d] is incorrect\n", nFilelen);
		fclose(fp);
		return -2;
	}
	
	BYTE *pBuff = NEW(BYTE, nFilelen);
	if(pBuff == NULL)
	{
 		TRACE("Err: Malloc FileSize Fault!!!\n");
		fclose(fp);
		return -3;
	}

	//DO NOT forget to go back to file head!!!
	fseek(fp, 0L, SEEK_SET);
	//now read all data into memory
	fread(pBuff, nFilelen, 1, fp); 
	fclose(fp);

	pUpgradeParam->strUpdownFileInfo.pbyReadBuf = pBuff;

	if(pUpgradeParam->strUpdownFileInfo.pbyReadBuf == NULL)
	{
		printf("Error: Read pUpgradeParam->strUpdownFileInfo.pbyReadBuf == NULL\n");
	}

	int n = 0; 
	for( n = 0; n < 100; n++)
	{
		printf("%02X",pUpgradeParam->strUpdownFileInfo.pbyReadBuf[n]);
	}
	printf("\n");

	pUpgradeParam->strUpdownFileInfo.nTotalFileSize = nFilelen;

	printf("Info:Read Bin File ok, nTotalFileSize=%d\n", pUpgradeParam->strUpdownFileInfo.nTotalFileSize);
	

	InitUpgradeParam(pCCU);

	return TRUE;
}

static void Update_RectUpgradeProcess(IN ROUGH_CCU_DATA* pCCU, int nProcess)
{
	CCU_UPGRADE_PARAM* pUpgradeParam = &pCCU->tagCCUUpgradeParam; 

	VAR_VALUE varV;
	varV.lValue = nProcess;
	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 200, 85, varV);

 	if(pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus == UPGRADE_FINISHED ||
		pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus == UPGRADE_IDLE)
	{
		varV.emValue = 0;
	}
	else
	{
		varV.emValue = 4; // Pending
	}

	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 200, 81, varV);
	printf("emUpgradeStatus = [%d]\n", pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus);
	varV.emValue = (pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus <= UPGRADE_WRITE_REQUEST) ? pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus : 4;
	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 200, 86, varV); //Distinguish from battery-side upgrades
	return;
}

static BOOL Wait_GCAN(ROUGH_CCU_DATA* pCCU)
{
	SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;

	BYTE byBuff[4096];
	memset(byBuff, 0, 4096);
	int nReadBytes = CCU_ReadData(pCCU, byBuff, sizeof(byBuff));

	// int i;
	// printf("readBytes:\n");
	// for(i=0; i<nReadBytes; ++i)
	// {
	// 	printf("%02X ", byBuff[i]);
	// }
	// printf("\n");

	
	if(nReadBytes >= NLMSG_MIN_LEN)
	{
		//now try to find the frame and parse it
		BYTE* pFoundPos = byBuff;
		do
		{
			BYTE *pDataPos;
			USHORT usDataLen;
			NLMSG_FRAME_INFO* pFrm = Find_NLMsgFrame(pCCU, pFoundPos, nReadBytes, &pDataPos, &usDataLen);
			if(pFrm == NULL)
			{
				// printf("---Get Frm Is NULL---\n");
				break;
			}
			else if(pFrm->byCmd == pCCU->nlFrameInfo[NL_MSG_GCAN].byCmd)
			{
				// printf("cmd[%02X]", pFrm->byCmd);
				pFrm->pfnParser(pCCU, pDataPos, usDataLen);
				return TRUE;
			}
			else
			{
				// printf("Info: Maybe Other Msg byCmd=[%02X]\n", pFrm->byCmd);
			}
			
			
			nReadBytes -= usDataLen + NLMSG_MIN_LEN;
			pFoundPos += usDataLen + NLMSG_MIN_LEN;
		} while (nReadBytes >= NLMSG_MIN_LEN);
	}

	return FALSE;
}



/*********************************************************************************
*  
*  FUNCTION NAME : HandleIMSUUpgrading
*  CREATOR       : John
*  CREATED DATE  : 2023-01-17 11:38:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void HandleIMSUUpgrading(IN ROUGH_CCU_DATA* pCCU)
{
	SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
	CCU_UPGRADE_PARAM* pUpgradeParam = &pCCU->tagCCUUpgradeParam; 
	BOOL bSendFileRet = FALSE;
	int nProcess = 0;
	//TRACE("--- Enter Upgrading ---\n");
	char buffer[64]; 
	char szFile[16]={0};
	//sprintf(szFile, "%s" , (pCCU->blobUpdCmd.emUpdtType == UPDT_TP_IMSU) ? "IMSU" : ((pCCU->blobUpdCmd.emUpdtType == UPDT_TP_PFC) ? "U1" : "U2"));
	switch (pCCU->blobUpdCmd.emUpdtType)
	{
		case UPDT_TP_IMSU:
			snprintf(szFile, 16, "IMSU[V%.1f]", pCCU->blobUpdCmd.fIMSU_SWVer_4Updt);
			break;
		case UPDT_TP_PFC:
			snprintf(szFile, 16, "PCS_PFC[V%.1f]", pCCU->blobUpdCmd.fPFC_SWVer_4Updt);
			break;
		case UPDT_TP_DCDC:
			snprintf(szFile, 16, "PCS_DCDC[V%.1f]", pCCU->blobUpdCmd.fDCDC_SWVer_4Updt);
			break;
		case UPDT_TP_ABU_U1:
			snprintf(szFile, 16, "ABU_U1[V%.1f]", pCCU->blobUpdCmd.fPFC_SWVer_4Updt);
			break;
		case UPDT_TP_ABU_U2:
			snprintf(szFile, 16, "ABU_U2[V%.1f]", pCCU->blobUpdCmd.fDCDC_SWVer_4Updt);
			break;
		case UPDT_TP_EU2_1:
			snprintf(szFile, 16, "EU2_1[V%.1f]", pCCU->blobUpdCmd.fIMEU2_SWVer_4Updt);
			break;
		case UPDT_TP_EU2_2:
			snprintf(szFile, 16, "EU2_2[V%.1f]", pCCU->blobUpdCmd.fIMEU2_SWVer_4Updt);
			break;
		case UPDT_TP_PFC_BOOT:
			snprintf(szFile, 16, "PCS_PFC_BOOT[V%.1f]", pCCU->blobUpdCmd.fPFC_SWVer_4Updt);
			break;
		case UPDT_TP_DCDC_BOOT:
			snprintf(szFile, 16, "PCS_DCDC_BOOT[V%.1f]", pCCU->blobUpdCmd.fDCDC_SWVer_4Updt);
			break;
		case UPDT_TP_MPPT_U1:
			snprintf(szFile, 16, "MPPT_U1[V%.1f]", pCCU->blobUpdCmd.fPFC_SWVer_4Updt);
			break;
		case UPDT_TP_MPPT_U2:
			snprintf(szFile, 16, "MPPT_U2[V%.1f]", pCCU->blobUpdCmd.fDCDC_SWVer_4Updt);
			break;
		default:
			break;
	}

	switch(pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus)
	{
		case UPGRADE_READBINFILE: 
		{

		#ifdef IMMU3
			BYTE z;
			USHORT i = 0;
			memset(u16crcTable, 0, 256);
			for (i = 0; i < 256; i++)
			{
					z = (BYTE)((i ^ (i >> 1)) & 0x00FF);
					z = (BYTE)((z ^ (z >> 2)) & 0x00FF);
					z = (BYTE)((z ^ (z >> 4)) & 0x00FF);
					u16crcTable[i] = (USHORT)(((z & 1)) ? 0xC001 : 0);
					u16crcTable[i] ^= (USHORT)((i << 6) ^ (i << 7));
			}

			if(pCCU->blobUpdCmd.emUpdtType != UPDT_TP_IMSU)
			{
				while(!CCU_Check_And_ReOpen_CommPort(pSampler, TRUE)) {};
			}



		#endif

			LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_INFO, "1=upgrade read [%s] bin file", szFile);
			nProcess = 253;//Please Wait
			pCCU->nUpgradeProcess = nProcess;
			BOOL bRet = TRUE;
			if(!pCCU->blobUpdCmd.bIs4Web)
			{
				bRet = ReadBinFileFromPath4USB(pCCU);
			}
			else
			{
				InitUpgradeParam(pCCU);
			}

			TRACE("### emSlvUpdateStatus=[%d] nDataOffset=[%d] bIs4Web=[%d]\n",\
					pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus,\
					pUpgradeParam->strUpdownFileInfo.nDataOffset,\
					pCCU->blobUpdCmd.bIs4Web);

			if(bRet == TRUE)
			{
				pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus = UPGRADE_WAIT_BEGIN;
				Send_DebugMsg_2UI(pCCU, FALSE, FALSE, TRUE,"Ready:CCU [%s] Read File OK->Wait_Start.", szFile);
				LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_INFO, "1=upgrade read [%s] bin file ok", szFile);
				TRACE("Info: Read  File Success, Now Goto Wait_Start\n");
			}
			else
			{
				pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus = UPGRADE_IDLE;
				Send_DebugMsg_2UI(pCCU, TRUE, TRUE, TRUE,"Fault:CCU [%s] Read File Failed bRet=[%d].", szFile, bRet);
				LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_ERROR, "1=upgrade read bin [%s] file failed, End Upgrade", szFile);
				TRACE("Err: Read File Failed,Eixt Upgrade!nRet=[%d]\n", bRet);
			}
			break;
		}
		case UPGRADE_WAIT_BEGIN:
		{
			LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_INFO, "2==upgrade wait begin");
			nProcess = 253;//Please Wait
			pCCU->nUpgradeProcess = nProcess;
			if((ABS(_NOW_ - pUpgradeParam->tagMstUpgradeParam.tm_StartUpgrade) > 30))
			{
				pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus = UPGRADE_FINISHED;
				// printf("Err: Enter Upgrading Mode TimeOut !!!\n");
				LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_ERROR, "2==upgrade failed cause by TimeOut");
				TRACE("Err: Enter Upgrading Mode TimeOut !!!\n");
				break;
			}

			if(pUpgradeParam->tagMstUpgradeParam.bInterSlvReady4Upgrade == FALSE)
			{
				SendUSIZCmdToCCU(pCCU);
				Send_DebugMsg_2UI(pCCU, FALSE, FALSE, TRUE, "Wait_Start:CCU[%s]Wait Upgrade Mode,Send USIZ", szFile);
				pUpgradeParam->strEmsUpCcuInfo.emUpdateStep = DOWNLOAD_SLIENCE;
			}
			else
			{
				TRACE("111 emSlvUpdateStatus=[%d] nDataOffset=[%d]\n",\
					pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus,\
					pUpgradeParam->strUpdownFileInfo.nDataOffset);
				LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_INFO, "2==upgrade change status to 3->pending");
				pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus = UPGRADE_PENDING;
				pCCU->strLSYSInfo.emUpgradingTotSchedule = 1;	// Upgrading
				TRACE("Info: Enter Upgrading Mode Ok !!!\n");
				Send_DebugMsg_2UI(pCCU, FALSE, FALSE, TRUE, "Start:CCU [%s] Start Upgrading...!", szFile);
			}
			break;
		}
		case UPGRADE_PENDING:
		{
			// if(ABS(_NOW_ - pUpgradeParam->tagMstUpgradeParam.tm_StartUpgrade) > 15*60) //Over 30Min, Juup To Finished
			if(ABS(_NOW_ - pUpgradeParam->tagMstUpgradeParam.tm_StartUpgrade) > 15*60*2) // 15min*4module
			{
				nProcess = 255;//Upgrade Failed
				pCCU->nUpgradeProcess = nProcess;
				pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus = UPGRADE_FINISHED;
				// printf("Err:Pending send sector Data TimeOut !!!\n");
				LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_ERROR, "3===upgrade failed cause by TimeOut");
				TRACE("Err:Pending send sector Data TimeOut !!!\n");
				break;
			}

			TRACE("222 emSlvUpdateStatus=[%d] nDataOffset=[%d]\n",\
				pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus,\
				pUpgradeParam->strUpdownFileInfo.nDataOffset);

			if((pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus == FIRMUPDATE_Downloading)||
				((pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus == FIRMUPDATE_Idle)&&(pUpgradeParam->strUpdownFileInfo.nDataOffset==0)))
			{
				nProcess = 100*pUpgradeParam->strUpdownFileInfo.nDataOffset/(pUpgradeParam->strUpdownFileInfo.nTotalFileSize+1);
				// if (pCCU->blobUpdCmd.emUpdtType != UPDT_TP_IMSU)
				// {
				// 	nProcess /= 2;
				// }
				Send_DebugMsg_2UI(pCCU, FALSE, FALSE, FALSE, "Upgrading:CCU [%s] Rate[%d%%]DownIndex=[%d]", 
					 szFile, nProcess ,pUpgradeParam->tagMstUpgradeParam.nRecvMsgId);

#ifdef IMMU2
				bSendFileRet = SendUpgradeIMSUFile(pCCU);
#else
				if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_IMSU)
				{
					bSendFileRet = SendUpgradeIMSUFile(pCCU);
				}
				else
				{
					struct timeval tv;
					// printf("while OVER emUpdateStep==[%d]\n", pUpgradeParam->strEmsUpCcuInfo.emUpdateStep);
					static int n=0;
					while(pUpgradeParam->strEmsUpCcuInfo.emUpdateStep != DOWNLOAD_FINISH)// && n<10)
					{

						// printf("hasSendLen = %d\n", pUpgradeParam->strEmsUpCcuInfo.nHasSendLen);

						bSendFileRet = SendModuleSingleMsg(pCCU);
						// int nErr = Mutex_Lock(pCCU->hSamplerLock, 10);
						// if(nErr == ERR_MUTEX_OK)
						// {
							// TRACEX("Now trying to receive CCU_Wait_ACCU_Reply_RS232");
							// gettimeofday(&tv, NULL);

							// long milliseconds = tv.tv_sec*1000 + tv.tv_usec/1000;
							// printf("Milliseconds = [%ld]\n", milliseconds);
						int i;
						BOOL ret = FALSE;
						for(i=0; i<3; ++i)
						{						
							if(!((pUpgradeParam->strEmsUpCcuInfo.uTrigger2 == 2)
								&& (pUpgradeParam->strEmsUpCcuInfo.emUpdateStep == DOWNLOAD_SEND)))
							{
								ret = Wait_GCAN(pCCU);
								// CCU_Wait_ACCU_Reply_RS232(pCCU, NULL, 0, NULL, 0);
							}
							if(ret == TRUE)
							{
								break;
							}
						}

						n++;

						// // if(n >= 20)
						if(n >= 10000)
						{

						// 	printf("UpgradeProcess->[%d]/[%d]\n", 
						// 		pUpgradeParam->strEmsUpCcuInfo.nHasSendLen, 
						// 		pUpgradeParam->strUpdownFileInfo.nTotalFileSize);

							RUN_THREAD_HEARTBEAT();
							n = 0;
						}

					}
				}
#endif
				if (bSendFileRet == ERR_CONTROL_FAILED)
				{
					// printf("Error: SendUpgradeIMSUFile Failed\n");
					nProcess = 255;	//Upgrade Failed
					pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus = UPGRADE_FINISHED;
					TRACE("Err:Send File Data Failed, Jump to UPGRADE_Finished!\n");
					Send_DebugMsg_2UI(pCCU, FALSE, TRUE, TRUE,"Upgradinging->Finished:CCU [%s] Send File Data Failed.",  szFile);
				}

				//Send_DebugMsg_2UI(pCCU, FALSE, FALSE, FALSE, "Upgrading:CCU [%s] Rate[%d%%]DownIndex=[%d]", 
				//	 szFile,nProcess,pUpgradeParam->tagMstUpgradeParam.nRecvMsgId);
			}
			else
			{
	#ifdef IMMU2
				// printf("Warn:CCU [%s] Upgrading, please wait!\n", szFile);
				if(pCCU->strLSYSInfo.emUpgradingTotSchedule == 3)
				{
					nProcess = 255;//Upgrade Failed
					pCCU->nUpgradeProcess = nProcess;
					pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus = UPGRADE_FINISHED;
					pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus = FIRMUPDATE_DownloadFailed;
					Send_DebugMsg_2UI(pCCU, FALSE, FALSE, TRUE,"Upgradinging->Faild:CCU [%s]Enter Finished.",  szFile);
					LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_ERROR, "3===cause by UpgradingTotSchedule failed");
					TRACE("Err:Pending send sector Data TimeOut !!!\n");
					sleep(2);
					break;
				}

				if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_IMSU)
				{
					nProcess = 254;
					pCCU->nUpgradeProcess = nProcess;
					pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus = UPGRADE_FINISHED;
					TRACE("Warn:CCU Rmt State Not in Upgrading, Jump to UPGRADE_Finished!\n");
					LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_INFO, "3===upgrade [%s] process finished", szFile);
					Send_DebugMsg_2UI(pCCU, FALSE, FALSE, TRUE,"Upgradinging->Finished:CCU [%s]Enter Finished.",  szFile);
				}
				else 
				{
					nProcess = pCCU->strLSYSInfo.byUpgradingSubIDSchedule;
					// printf("Warn:CCU [%s] Upgrading, please wait!\n", szFile);

					TRACE("Warn:CCU [%s] Upgrading, please wait!\n", szFile);
					if(pCCU->strLSYSInfo.byUpgradingSubIDSchedule == 100 && pCCU->strLSYSInfo.emUpgradingTotSchedule == 2)
					{
						nProcess = 254;
						// pCCU->strLSYSInfo.emUpgradingTotSchedule = 0;
						pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus = UPGRADE_FINISHED;
						LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_INFO, "3===upgrade [%s] process finished", szFile);
						Send_DebugMsg_2UI(pCCU, FALSE, FALSE, TRUE,"Upgradinging->Finished:CCU [%s]Enter Finished.",  szFile);
						sleep(2);
					}
				}
	#else
				if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_IMSU)
				{
					// printf("Warn:CCU [%s] Upgrading, please wait!\n", szFile);
					if(pCCU->strLSYSInfo.emUpgradingTotSchedule == 3)
					{
						nProcess = 255;//Upgrade Failed
						pCCU->nUpgradeProcess = nProcess;
						pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus = UPGRADE_FINISHED;
						pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus = FIRMUPDATE_DownloadFailed;
						Send_DebugMsg_2UI(pCCU, FALSE, FALSE, TRUE,"Upgradinging->Faild:CCU [%s]Enter Finished.",  szFile);
						LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_ERROR, "3===cause by UpgradingTotSchedule failed");
						TRACE("Err:Pending send sector Data TimeOut !!!\n");
						sleep(2);
						break;
					}

					nProcess = 254;
					pCCU->nUpgradeProcess = nProcess;
					pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus = UPGRADE_FINISHED;
					TRACE("Warn:CCU Rmt State Not in Upgrading, Jump to UPGRADE_Finished!\n");
					LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_INFO, "3===upgrade [%s] process finished", szFile);
					Send_DebugMsg_2UI(pCCU, FALSE, FALSE, TRUE,"Upgradinging->Finished:CCU [%s]Enter Finished.",  szFile);
				}
				else	// 模块升级
				{
					if(pUpgradeParam->strEmsUpCcuInfo.emUpdateStep == DOWNLOAD_FINISH) // 模块升级成功
					{
						printf("Sucess 1\n");
						nProcess = 254;
						pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus = UPGRADE_FINISHED;
						LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_INFO, "3===upgrade [%s] process finished", szFile);
						Send_DebugMsg_2UI(pCCU, FALSE, FALSE, TRUE,"Upgradinging->Finished:CCU [%s]Enter Finished.",  szFile);
					}
					else // 升级失败
					{
						printf("ERROR 2\n");
						nProcess = 255;//Upgrade Failed
						pCCU->nUpgradeProcess = nProcess;
						pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus = UPGRADE_FINISHED;
						pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus = FIRMUPDATE_DownloadFailed;
						Send_DebugMsg_2UI(pCCU, FALSE, FALSE, TRUE,"Upgradinging->Faild:CCU [%s]Enter Finished.",  szFile);
						LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_ERROR, "3===cause by UpgradingTotSchedule failed");
						TRACE("Err:Pending send sector Data TimeOut !!!\n");
						sleep(2);
						break;
					}
				}
	#endif
			}
 			break;
		}
		case  UPGRADE_FINISHED:
		{
#ifdef IMMU3
			if(pCCU->blobUpdCmd.emUpdtType != UPDT_TP_IMSU) // 给模块升级
			{
				// printf(" Send USIZ OVER\n");
				SendUSIZCmdToCCU(pCCU); // 要回 0xFF
				while(!CCU_Check_And_ReOpen_CommPort(pSampler, FALSE)) {};
			}
			
			pCCU->bIsReceivedCFGG_FromLCCU = FALSE;

#endif


			LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_INFO, "4=====UPGRADE_FINISHED");
			nProcess = (pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus==FIRMUPDATE_Downloaded) ? 254 : 255;//254 - Upgrade Finished
			pCCU->nUpgradeProcess = nProcess;
			TRACE("Info:FINISHED [%s] SlvUpdateStatus=[%d]\n",\
				(pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus==FIRMUPDATE_Downloaded)?"Upgrade Success":"Upgrade Failed",\
				pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus);

			Send_DebugMsg_2UI(pCCU, TRUE, FALSE, TRUE, "Finished:CCU [%s] [%s] ReptStat=[%d]!",
				 szFile, (pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus==FIRMUPDATE_Downloaded)?"Upgrade Success":"Upgrade Failed",
				pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus);
			LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_INFO, "Info:FINISHED [%s] SlvUpdateStatus=[%d]\n",\
				(pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus==FIRMUPDATE_Downloaded)?"Upgrade Success":"Upgrade Failed",\
				pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus);

			VAR_VALUE varV;
			varV.ulValue = (unsigned long)_NOW_;
			Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 200, 83, varV);
 
			snprintf(buffer, 64, "Idle");
			Push_TextValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 200 , 87, buffer, 64);

			pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus = UPGRADE_IDLE;
			SAFELY_DELETE(pUpgradeParam->strUpdownFileInfo.pbyReadBuf);
			pUpgradeParam->tagMstUpgradeParam.nRecvMsgId = 0;
			pUpgradeParam->tagMstUpgradeParam.nMsgId = 0;
			pUpgradeParam->tagMstUpgradeParam.bInterSlvReady4Upgrade = FALSE;
			pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus = FIRMUPDATE_Idle;  
			pUpgradeParam->strUpdownFileInfo.nDataOffset = 0;
			break;
		}
		default:
		{
			LogOut(MAIN_MODULE_SAMPLER, "upgrade", LOG_TYPE_ERROR, "5=====upgrade default failed, Ohter UpDown Statu, emSlvUpdateStatus=[%d]", pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus);
			nProcess = 255;//Upgrade Failed
			pCCU->nUpgradeProcess = nProcess;

			VAR_VALUE varV;
			varV.ulValue = (unsigned long)_NOW_;
			Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 200, 83, varV);

			snprintf(buffer, 64, "Idle");
			Push_TextValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 200 , 87, buffer, 64);
			
			pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus = UPGRADE_IDLE;
			SAFELY_DELETE(pUpgradeParam->strUpdownFileInfo.pbyReadBuf);
			 TRACE("Ohter UpDown Statu, emUpgradeStatus=[%d] \n", pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus);
			pUpgradeParam->tagMstUpgradeParam.nRecvMsgId = 0;
			pUpgradeParam->tagMstUpgradeParam.nMsgId = 0;
			pUpgradeParam->tagMstUpgradeParam.bInterSlvReady4Upgrade = FALSE;
			pUpgradeParam->tagMstUpgradeParam.emSlvUpdateStatus = FIRMUPDATE_Idle;  
			pUpgradeParam->strUpdownFileInfo.nDataOffset = 0;
			break;
		}
	}
	
#ifdef IMMU3
		Update_RectUpgradeProcess(pCCU, pCCU->nUpgradeProcess);
#else
		Update_RectUpgradeProcess(pCCU, nProcess);
#endif

	return ;
}


int Parse_USIZ(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID)
{
	TRACEX("Enter Parse_USIZ\n");
	pCCU->tagCCUUpgradeParam.tagMstUpgradeParam.bInterSlvReady4Upgrade =  (((int)pData[0]) == 1) ? TRUE : FALSE;
 	pData +=1;
	
	TRACEX("Beging parsing USIZ for bInterSlvReady4Upgrade=%d\n",\
		pCCU->tagCCUUpgradeParam.tagMstUpgradeParam.bInterSlvReady4Upgrade);
	return 0;
}

int Parse_UDTA(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID)
{
	pCCU->tagCCUUpgradeParam.tagMstUpgradeParam.nRecvMsgId = Bytes2Ulong(pData, 2); 
	pData +=2;

	pCCU->tagCCUUpgradeParam.tagMstUpgradeParam.emSlvUpdateStatus = (FIRM_UPDATESTT)pData[0];  
	pData +=1;

	TRACE("Beging parsing UDTA for nRecvMsgId=%d emSlvUpdateStatus=%d\n", \
		pCCU->tagCCUUpgradeParam.tagMstUpgradeParam.nRecvMsgId,\
	pCCU->tagCCUUpgradeParam.tagMstUpgradeParam.emSlvUpdateStatus);

	return 0;
}


static BOOL CheckandRead_BinFile(ROUGH_CCU_DATA* pCCU, int emUpdtType)
{
	CCU_UPGRADE_PARAM* pUpgradeParam = &pCCU->tagCCUUpgradeParam; 

	char* szFile;
	if(emUpdtType == UPDT_TP_IMSU)	
	{
		szFile = pCCU->blobUpdCmd.szIMSUFilePath;
	}
	else if(emUpdtType == UPDT_TP_PFC)
	{
		szFile = pCCU->blobUpdCmd.szPFCFilePath;
	}
	else if(emUpdtType == UPDT_TP_DCDC)
	{
		szFile = pCCU->blobUpdCmd.szDCDCFilePath;
	}
	else if(emUpdtType == UPDT_TP_ABU_U1)	
	{
		szFile = pCCU->blobUpdCmd.szPFCFilePath;
	}
	else if(emUpdtType == UPDT_TP_ABU_U2)
	{
		szFile = pCCU->blobUpdCmd.szDCDCFilePath;
	}
	else if(emUpdtType == UPDT_TP_EU2_1)
	{
		szFile = pCCU->blobUpdCmd.szIMEU2FilePath;
	}
 	else if(emUpdtType == UPDT_TP_MPPT_U1)
	{
		szFile = pCCU->blobUpdCmd.szPFCFilePath;
	}
 	else if(emUpdtType == UPDT_TP_MPPT_U2)
	{
		szFile = pCCU->blobUpdCmd.szDCDCFilePath;
	} 
	else
	{
		return FALSE;
	}

	FILE*fp = fopen(szFile,"rb");
	if(fp == NULL)
	{
		TRACEX("Can NOT open the file[%s]\n", szFile);
		return FALSE;
	}

	fseek(fp,0L,SEEK_END);
	size_t nFilelen = ftell(fp);
	if((nFilelen < MIN_BINFILE_SIZE) || (nFilelen > MAX_BINFILE_SIZE))
	{
		TRACEX("FileLen[%d] is incorrect\n", nFilelen);
		fclose(fp);
		return FALSE;
	}
	
	//now allocate memory for the file 
	BYTE *pBuff = NEW(BYTE, nFilelen);
	if(pBuff == NULL)
	{
		TRACEX("CAN NOT allocate memory for FileLen[%d]\n", nFilelen);
		fclose(fp);
		return FALSE;
	}
	//DO NOT forget to go back to file head!!!
	fseek(fp,0L,SEEK_SET);
	//now read all data into memory
	fread(pBuff, nFilelen, 1, fp); 
	fclose(fp);
	//if(bIsDCDC)
	//{
	//	pUpgradeParam->strUpdownFileInfo.pbyReadBuf = pBuff;
	//	pUpgradeParam->strUpdownFileInfo.nTotalFileSize = nFilelen;
	//}
	//else
	//{
	//	pUpgradeParam->strUpdownFileInfo.pbyReadBuf = pBuff;
	//	pUpgradeParam->strUpdownFileInfo.nTotalFileSize = nFilelen;
	//}	

	pUpgradeParam->strUpdownFileInfo.pbyReadBuf = pBuff;
	pUpgradeParam->strUpdownFileInfo.nTotalFileSize = nFilelen;

	TRACE("Read bin file[%s] successfully, and fileSize[%d]\n", szFile, nFilelen);

	return TRUE;
}

BOOL Prepare_CCUBin_Updte(IN ROUGH_CCU_DATA* pCCU, int nTimeout_mSec)
{	
	BOOL bRet = TRUE;
	//----Step1: now check and read the bin files DCDC/PFC/IMSU-D
	if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_PFC)
	{
		bRet = CheckandRead_BinFile(pCCU, pCCU->blobUpdCmd.emUpdtType);
		if(bRet == FALSE)
		{
			return FALSE;
		}
	}

	if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_DCDC)
	{
		bRet = CheckandRead_BinFile(pCCU, pCCU->blobUpdCmd.emUpdtType);
		if(bRet == FALSE)
		{
			return FALSE;
		}
	}

	if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_IMSU)
	{
		bRet = CheckandRead_BinFile(pCCU, pCCU->blobUpdCmd.emUpdtType);
		if(bRet == FALSE)
		{
			return FALSE;
		}
	}

	if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_ABU_U1)
	{
		bRet = CheckandRead_BinFile(pCCU, pCCU->blobUpdCmd.emUpdtType);
		if(bRet == FALSE)
		{
			return FALSE;
		}
	}

	if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_ABU_U2)
	{
		bRet = CheckandRead_BinFile(pCCU, pCCU->blobUpdCmd.emUpdtType);
		if(bRet == FALSE)
		{
			return FALSE;
		}
	}

	if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_EU2_1)
	{
		bRet = CheckandRead_BinFile(pCCU, pCCU->blobUpdCmd.emUpdtType);
		if(bRet == FALSE)
		{
			return FALSE;
		}
	}

	if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_MPPT_U1)
	{
		bRet = CheckandRead_BinFile(pCCU, pCCU->blobUpdCmd.emUpdtType);
		if(bRet == FALSE)
		{
			return FALSE;
		}
	}

	if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_MPPT_U2)
	{
		bRet = CheckandRead_BinFile(pCCU, pCCU->blobUpdCmd.emUpdtType);
		if(bRet == FALSE)
		{
			return FALSE;
		}
	}

	TRACE("-------Now evrything is ready for rect-updt, mode[%d], updtTy[%d]----\n", 
		pCCU->blobUpdCmd.emUpdtMode, pCCU->blobUpdCmd.emUpdtType);
	//LogOut(MAIN_MODULE_SAMPLER, "CCU-Updt", LOG_TYPE_WARING,
	//		 "Ready for ccu-upgrade, mode=>[%s, %s], targetVer=>[%.3f, %.3f]...", 
	//		 (pCCU->blobUpdCmd.emUpdtMode == RECT_UPDT_MODE_FORCE_4SINGLE) ? "Force" : "Normal",
	//		 (pCCU->blobUpdCmd.emUpdtType == UPDT_TP_DCDC) ? "DC_Only" : 
	//		 ((pCCU->blobUpdCmd.emUpdtType == UPDT_TP_PFC) ? "PFC_Only" : "DC_and_PFC"),
	//		 pUpdtCmd->fDCDC_SWVer_4Updt, pUpdtCmd->fPFC_SWVer_4Updt);


	return TRUE;
}

