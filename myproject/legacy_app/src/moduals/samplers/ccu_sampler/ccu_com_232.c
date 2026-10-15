// #define _TRACE_PRINT_ // PSC
#include "ccu_sampler_main.h"

#define  TCP_CLIENT_READ_MIN_INTERV_MSEC  50

// #define TRACE  TRACED
// #define TRACEX TRACED

BOOL CCU_Check_And_ReOpen_CommPort(SAMPLER* pSampler, BOOL bModuleUpdate)
{
	int nErrCode = 0;
	// if(pSampler->hCommPort != NULL)
	// {
	// 	pSampler->pUsedCommPort->Close(pSampler->hCommPort);
	// 	pSampler->hCommPort = NULL;
	// }

	// int portId = 101; // IMMU3 /dev/ttyO1 J2 dn1 dn2
	int portId = 102; // BMU03 /dev/ttyO2
	static COMMPORT s_UsedCommPort232;
	pSampler->pUsedCommPort = &s_UsedCommPort232;
	if (GetCommPort_ById((HANDLE)pSampler, portId, pSampler->pUsedCommPort) == NULL)
	{
		pSampler->pUsedCommPort = NULL;
		TRACEE("CCU_Check_And_ReOpen_CommPort_232:GetCommPort_ById:%d failed", portId);
		return FALSE;
	}
	strncpy(pSampler->info.szSamplerSetting, "1000000,N,8,1", sizeof(pSampler->info.szSamplerSetting));
	// strncpy(pSampler->info.szSamplerSetting, "19200,N,8,1", sizeof(pSampler->info.szSamplerSetting));
	//strncpy(pSampler->info.szSamplerSetting, "9600,N,8,1", sizeof(pSampler->info.szSamplerSetting));
	TRACED("CCU_Check_And_ReOpen_CommPort_232: <%s:%s>", pSampler->pUsedCommPort->info.szPortAttri, pSampler->info.szSamplerSetting);

	if(!bModuleUpdate)
	{
	pSampler->hCommPort = pSampler->pUsedCommPort->Open(
		pSampler->pUsedCommPort->info.szPortAttri,
		pSampler->info.szSamplerSetting,
		COMM_CLIENT_MODE, CCU_232_READ_TIMEOUT_SETT, &nErrCode);
	}
	else
	{
		pSampler->hCommPort = pSampler->pUsedCommPort->Open(
			pSampler->pUsedCommPort->info.szPortAttri,
			pSampler->info.szSamplerSetting,
			COMM_CLIENT_MODE, 50, &nErrCode);
	}

	if((ERR_COMM_OK != nErrCode) || (pSampler->hCommPort == NULL))
	{
		pSampler->hCommPort = NULL;
		TRACEE("CCU_Check_And_ReOpen_CommPort_232:Check_And_ReOpen_CommPort<%s:%s> failed",
			pSampler->pUsedCommPort->info.szPortAttri, pSampler->info.szSamplerSetting);
		return FALSE;
	}

	pSampler->pUsedCommPort->Control(pSampler->hCommPort, COMM_SET_SERIAL_485_To232, NULL, 0);
	TRACED("CCU_Check_And_ReOpen_CommPort_232:Check_And_ReOpen_CommPort<%s:%s>",
		pSampler->pUsedCommPort->info.szPortAttri, pSampler->info.szSamplerSetting);
	return TRUE;
}

BOOL CCU_Wait_ACCU_Reply_RS232(ROUGH_CCU_DATA* pCCU, NLMSG_FRAME_INFO* pFramInfo, int nTimeOut_mSec,
								BYTE* pDataSend, int nDataLenSend)
{
	UNUSED(nDataLenSend);
	// SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;

	BYTE byBuff[CCU_BUFF_LEN_232];
	memset(byBuff, 0, CCU_BUFF_LEN_232);
	int nReadBytes = 0;
	int nTotalReadBytes = 0;
	BYTE* pReadPos = byBuff;
	BYTE* pParsePos = pReadPos;
	NLMSG_FRAME_INFO* pFrm = NULL;
	BYTE* pFrmData = NULL;
	USHORT usFrmDataLen = 0;
	// BOOL bHasFoundFrm = FALSE;
	UINT nWaitmSec = 0;
	// int nLastErrorCode = 0;
	int nParseLen = 0;
	BOOL bHasMoreFrm = FALSE;
	BOOL bSuccess = FALSE;
	int nReadChunkDataCount = 0;
	BOOL bFoundResult = FALSE;
	double dtmStart = GetCurrentTime();
	do
	{
		nReadBytes = CCU_ReadData(pCCU, pReadPos, CCU_BUFF_LEN_232 - nTotalReadBytes);
		nTotalReadBytes += nReadBytes;
		nParseLen += nReadBytes;
#ifdef _TRACE_PRINT_
		if (nReadBytes < 1)
		{
			TRACE("CCU_Wait_ACCU_Reply_RS232 not Read nReadBytes:%d reply[%d] from CCU, nTimeout_ms:[%d] ElapseMS:%d frm:[%s] pFramInfo:%x nDataLenSend:%d",
				nReadBytes, nTotalReadBytes, nTimeOut_mSec, ElapseMS(dtmStart), printableBytes(pReadPos, nReadBytes), pFramInfo, nDataLenSend);
		}
		else
		{
			TRACE("CCU_Wait_ACCU_Reply_RS232 Read nReadBytes:%d reply[%d] from CCU, nTimeout_ms:[%d] ElapseMS:%d frm:[%s]", 
				nReadBytes, nTotalReadBytes, nTimeOut_mSec, ElapseMS(dtmStart), printableBytes(pReadPos, nReadBytes));
		}
#endif
		bHasMoreFrm = FALSE;
		//if((nTotalReadBytes >= NLMSG_MIN_LEN) && ((nReadBytes > 0) || bHasMoreFrm))
		while(nParseLen >= NLMSG_MIN_LEN)
		{
			pFrm = Find_NLMsgFrame(pCCU, pParsePos, nParseLen, &pFrmData, &usFrmDataLen);
			if(pFrm == NULL)
			{
				nParseLen -= (usFrmDataLen + NLMSG_MIN_LEN);
				// TRACEW("CCU_Wait_ACCU_Reply_RS232 Find_NLMsgFrame NULL break nParseLen:%d", nParseLen);
				break;
			}
			TRACE("CCU_Wait_ACCU_Reply_RS232 Find_NLMsgFrame msgTp:%d byCmd:%02X", pFrm->msgTp, pFrm->byCmd);
			// switch (pFrm->byCmd)
			// {
			// 	case 0x01: // CFGG
			// 	printf("CCU_Wait_ACCU_Reply_RS232 cmd:%02X [CFGG] byBuff<%s>\n", pFrm->byCmd, printableBytes((char*)pParsePos, usFrmDataLen+NLMSG_MIN_LEN));
			// 		break;
			// 	case 0x02: // CFGS
			// 	printf("CCU_Wait_ACCU_Reply_RS232 cmd:%02X [CFGS] byBuff<%s>\n", pFrm->byCmd, printableBytes((char*)pParsePos, usFrmDataLen+NLMSG_MIN_LEN));
			// 		break;
				// case 0x03: // UCTL
				// printf("CCU_Wait_ACCU_Reply_RS232 cmd:%02X [UCTL] sub:%02X byBuff<%s>\n", pFrm->byCmd, *pFrmData, printableBytes((char*)byBuff, nReadBytes));
				// 	break;
			// 	case 0x04: // RTIF
			// 	printf("CCU_Wait_ACCU_Reply_RS232 cmd:%02X [RTIF] byBuff<%s>\n", pFrm->byCmd, printableBytes((char*)pParsePos, usFrmDataLen+NLMSG_MIN_LEN));
			// 		break;
			// 	// case 0x12: // USIZ
			// 	// 	printf("CCU_Wait_ACCU_Reply_RS232 cmd:%02X [USIZ] byBuff<%s>\n", pFrm->byCmd, printableBytes((char*)byBuff, nReadBytes));
			// 	// 	break;
			// 	// case 0x13: 	// UDTA
			// 	// 	printf("CCU_Wait_ACCU_Reply_RS232 cmd:%02X [UDTA] byBuff<%s>\n", pFrm->byCmd, printableBytes((char*)byBuff, nReadBytes));
			// 	// 	break;
			// 	case 0x23: // PCSG
			// 	printf("CCU_Wait_ACCU_Reply_RS232 cmd:%02X [PCSG] byBuff<%s>\n", pFrm->byCmd, printableBytes((char*)byBuff, nReadBytes));
			// 		break;
			// 	case 0x24: // PCSS
			// 	printf("CCU_Wait_ACCU_Reply_RS232 cmd:%02X [PCSS] byBuff<%s>\n", pFrm->byCmd, printableBytes((char*)byBuff, nReadBytes));
			// 		break;
			// 	// case 0x2D:	// time
			// 	// printf("CCU_Wait_ACCU_Reply_RS232 cmd:%02X [0x2D] byBuff<%s>\n", pFrm->byCmd, printableBytes((char*)byBuff, nReadBytes));
			// 	// 	break;
			// 	// case 0xB0: // LSYS
			// 	// printf("CCU_Wait_ACCU_Reply_RS232 cmd:%02X [LSYS] byBuff<%s>\n", pFrm->byCmd, printableBytes((char*)byBuff, nReadBytes));
			// 	// 	break;
			// 	default:
			// 		printf("CCU_Wait_ACCU_Reply_RS232 cmd:%02X [OK]  byBuff<%s>\n", pFrm->byCmd, printableBytes((char*)byBuff, nReadBytes));
			// 		break;
			// }
#ifdef _TRACE_PRINT_
			switch (pFrm->byCmd)
			{
				case 0x01: // CFGG
				TRACED("CCU_Wait_ACCU_Reply_RS232 cmd:%02X CFGG byBuff<%s>", pFrm->byCmd, printableBytes(pParsePos, usFrmDataLen+NLMSG_MIN_LEN));
					break;
				case 0x02: // CFGS
				TRACED("CCU_Wait_ACCU_Reply_RS232 cmd:%02X CFGS byBuff<%s>", pFrm->byCmd, printableBytes(pParsePos, usFrmDataLen+NLMSG_MIN_LEN));
					break;
				case 0x03: // UCTL
				TRACEI("CCU_Wait_ACCU_Reply_RS232 cmd:%02X UCTL sub:%02X byBuff<%s>", pFrm->byCmd, *pFrmData, printableBytes(byBuff, nReadBytes));
					break;
				case 0x04: // RTIF
				TRACED("CCU_Wait_ACCU_Reply_RS232 cmd:%02X RTIF byBuff<%s>", pFrm->byCmd, printableBytes(pParsePos, usFrmDataLen+NLMSG_MIN_LEN));
					break;
				case 0xB0: // LSYS
				TRACE("CCU_Wait_ACCU_Reply_RS232 cmd:%02X LSYS byBuff<%s>", pFrm->byCmd, printableBytes(byBuff, nReadBytes));
					break;
				case 0x23: // PCSG
				TRACED("CCU_Wait_ACCU_Reply_RS232 cmd:%02X PCSG byBuff<%s>", pFrm->byCmd, printableBytes(byBuff, nReadBytes));
					break;
				case 0x24: // PCSS
				TRACED("CCU_Wait_ACCU_Reply_RS232 cmd:%02X PCSS byBuff<%s>", pFrm->byCmd, printableBytes(byBuff, nReadBytes));
					break;

				default:
				TRACED("CCU_Wait_ACCU_Reply_RS232 cmd:%02X byBuff<%s>", pFrm->byCmd, printableBytes(byBuff, nReadBytes));
					break;
			}
#endif
			pCCU->tmLastRecvedData = _NOW_;
			// printf("has received data from CCU\n");
			// bHasFoundFrm = TRUE;
			if((pFrm->msgTp == NL_MSG_CFGS) || (pFrm->msgTp == NL_MSG_UCTL))
			{
				if((usFrmDataLen >= 2) && (pDataSend!=NULL && *pFrmData == pDataSend[0])) //subCmd
				{
					TRACEX("------Now get CFGS/UTRL reply for CCU, subCmd[%d, %d, [%d,%d]]---\n",
						pFrm->byCmd, usFrmDataLen, (BYTE)*pFrmData, pDataSend[0]);
					bSuccess = (pFrm->pfnParser(pCCU, pFrmData, usFrmDataLen) == 0) ? TRUE : FALSE;
					if (pFrm->msgTp == NL_MSG_CFGS)
					{
						bFoundResult = TRUE;
					}
				}
			}
			else if((pFrm == pFramInfo) && (pFramInfo != NULL))
			{
				bSuccess = (pFrm->pfnParser(pCCU, pFrmData, usFrmDataLen) == 0) ? TRUE : FALSE;
				bFoundResult = TRUE;
			}
			// else if((pFrm->msgTp == NL_MSG_CFGG) && (s_stCCUTCP_RunRough.pCCU->emChargerType == CHARGER_TYPE_SPLIT_DISPENSER))
			// {
			// 	//add by Duke 20240509, dispenser need sync CCU setting which changed by station upper
			// 	pFrm->pfnParser(&s_stCCUTCP_RunRough, pFrmData, usFrmDataLen);
			// }
			else
			{
				// LJH241101 MU3 收到的数据都解析
				pFrm->pfnParser(pCCU, pFrmData, usFrmDataLen);
			}
			//find pos move
			pParsePos = pFrmData + usFrmDataLen + 3; //3 = 2byte-CheckSum + 1byte-EOI
			nParseLen -= (usFrmDataLen + NLMSG_MIN_LEN);
		}

		// //Add by Duke 20240415, for new split charger date transparent to station controller.
		// if((s_stCCUTCP_RunRough.pCCU->emChargerType == CHARGER_TYPE_SPLIT_DISPENSER) && (nReadBytes >= NLMSG_MIN_LEN))
		// {
		// 	SendTransparentData2Station(pReadPos, nReadBytes);
		// }
		if(nParseLen > 0) //added by Jimmy 20220110 as to check chunk data	
		{
			bHasMoreFrm = TRUE;
		}
		else if(bFoundResult) //if in setting, we should skip if no more data
		{
			break;
		}

		pReadPos += nReadBytes;		
		if(nTotalReadBytes >= CCU_BUFF_LEN_232)
		{
			break;
		}
		nTimeOut_mSec -= TCP_CLIENT_READ_MIN_INTERV_MSEC;
		nWaitmSec += TCP_CLIENT_READ_MIN_INTERV_MSEC;
		// LJH241101 MU3 由Sample发送
		// if((nWaitmSec > 2000)/* && (!bHasFoundFrm)*/)
		// {
		// 	//in case it waits the IMSU-D will consider comm-fail
		// 	TRACEX("----NOW neeed ReCHECK the USYS----\n");
		// 	nWaitmSec = 0;
		// 	Send_USYS(pCCU);
		// }
		//if((nReadBytes > 0) /*&& (!bHasFoundFrm) */&& (nTimeOut_mSec <= 0))
		if(bHasMoreFrm && (nTimeOut_mSec <= 0))
		{
			TRACE("CCU_Wait_ACCU_Reply_RS232 bHasMoreFrm nReadChunkDataCount:%d", nReadChunkDataCount);
			nReadChunkDataCount++;
			// printf("----[%.3f]: Read out ChUNNNNNK data for frm->[%d], nReadChunkDataCount = %d, nParseLen = %d, nTotalReadBytes = %d, nTimeOut_mSec = %d------\n",
			// 	GetCurrentTime(), (pFramInfo != NULL) ? pFramInfo->byCmd : -9999,
			// 	nReadChunkDataCount, nParseLen, nTotalReadBytes, nTimeOut_mSec);
			if(nReadChunkDataCount < 20) //read 1 sec more
			{
				nTimeOut_mSec = 1; //give it one more chance if read chunk data
			}
			else
			{
				break;
			}
		}
	} while (((int)ElapseMS(dtmStart)) < nTimeOut_mSec);

	// printf("----[%.3f]: The parse  for frm->[%d] result => %d, and parse-left-len => %d----\n", GetCurrentTime(),
	// 	(pFramInfo != NULL) ? pFramInfo->byCmd : -9999,
	// 	(pFramInfo != NULL) ? bSuccess : bHasFoundFrm, nParseLen);

	return (pFramInfo != NULL) ? bSuccess : TRUE;
}

BOOL DoTCPSession(ROUGH_CCU_DATA* pCCU, NL_MSGTYPE emMsgTp, BYTE* pData, int nDataLen, int nTimeout_ms)
{
	// TRACED("ccu_com_232:DoTCPSession emMsgTp:%d nDataLen:%d nTimeout_ms:%d", emMsgTp, nDataLen, nTimeout_ms);
	// LJH240730 IMMU3 Serial
	if (emMsgTp >= NL_MSG_Max)
	{
		TRACEE("ccu_com_232:DoTCPSession emMsgTp >= NL_MSG_Max %d", emMsgTp);
		return FALSE;
	}
	
	BYTE byBuff[CCU_BUFF_LEN_232];
	memset(byBuff, 0, CCU_BUFF_LEN_232);
	NLMSG_FRAME_INFO* pFramInfo = &pCCU->nlFrameInfo[emMsgTp];
	int nSendLen = MakeNLMSG_SendFrame(pFramInfo->byCmd, (USHORT)nDataLen, pData, byBuff);
	//Sleep_Ex(50); //if send to frequently data can get lost
	//Mutex_Lock(pCCU->hSamplerLock, WAIT_INFINITE);

	//added by Jimmy 20210122 to clear read buffer before doing comm // LJH240730 IMMU3 Serial 不清空，有数据就处理
	//pSampler->pUsedCommPort->Control(pSampler->hCommPort, COMM_PURGE_RXCLEAR, NULL, 0);
	//int nWriteLen = pSampler->pUsedCommPort->Write(pSampler->hCommPort, byBuff, nSendLen);
	int nWriteLen = CCU_WriteData(pCCU, byBuff, nSendLen);
	//Sleep_Ex(50); //Jimmy 20190528: debug with Ilock, if send to frequently data can get lost
	if(nWriteLen != nSendLen)
	{
		TRACEE("ccu_com_232:DoTCPSession nWriteLen!=nSendLen %d %d emMsgTp:%d", nWriteLen, nSendLen, emMsgTp);
		return FALSE;
	}
#ifdef _TRACE_PRINT_
	// // UCTL
	// BYTE byCmd = byBuff[3];
	// if (byCmd==0x03 && byBuff[4]>=0x31 && byBuff[4]<=0x34)
	// {
	// 	TRACEI("ccu_com_232:DoTCPSession Write UCTL Group AC/DC byCmd<%02X> Control nSendLen<%d> byBuff<%s>",
	// 		byCmd, nSendLen, printableBytes(byBuff, nSendLen));
	// }
	// else
	// {
	// 	// if (byCmd != 0x01)
	// 	{
	// 		TRACED("ccu_com_232:DoTCPSession Write byCmd<%02X> nSendLen<%d> bySendBuff<%s>",
	// 			byCmd, nSendLen, printableBytes(byBuff, nSendLen));
	// 	}
	// }
#endif
	// LJH240730 IMMU3

	if(emMsgTp == NL_MSG_PCSSET && pCCU->tagCCUUpgradeParam.strEmsUpCcuInfo.emUpdateStep != DOWNLOAD_IDLE)
	{
		Sleep(nTimeout_ms);

		return TRUE;
	}

	return CCU_Wait_ACCU_Reply_RS232(pCCU, pFramInfo, nTimeout_ms, pData, nDataLen);
}
