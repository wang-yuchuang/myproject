#include "inter_slave_main.h"


#define  INIT_SLV_MSG_INFO(_pMsg,_emMsgType,_SMsgNo,_RMsgNo, _pfnSHand,_pfnRHand,_bHasSubCmd) \
					do{\
					((_pMsg)->emMsgType = (_emMsgType), \
					(_pMsg)->bySCmdNo = (_SMsgNo), \
					(_pMsg)->byRCmdNo = (_RMsgNo), \
					(_pMsg)->pfnSHandler = (_pfnSHand), \
					(_pMsg)->pfnRHandler = (_pfnRHand), \
					(_pMsg)->bHasSubCmd = (_bHasSubCmd), \
					(_pMsg)++);\
					}while(0)

#define MAKE_SLV_FRAME(pos, frm) \
					do{\
						pos[0] = (SLV_PROT_SOI), \
						pos[1] = (SLV_PROT_SOI), \
						pos[2] = (frm.bySrcAddr), \
						pos[3] = (frm.byDestAddr), \
						pos[4] = (frm.byCmd), \
						pos[5] = ((BYTE)((frm.ulDataLen) >> 8)), \
						pos[6] = ((BYTE)(frm.ulDataLen)); \
						memcpy(pos + 7, frm.byData, frm.ulDataLen); \
						pos[frm.ulDataLen + 7] = ((BYTE)((frm.ulCalcSum) >> 8)), \
						pos[frm.ulDataLen + 8] = ((BYTE)(frm.ulCalcSum)); \
						pos[frm.ulDataLen + 9] = (SLV_PROT_EOI);\
					}while(0)


static USHORT MakeChkSum(BYTE byCmd, USHORT usDataLen, BYTE* byData)
{
	USHORT ulCalcSum = (USHORT)byCmd + usDataLen;
	int n;
	for(n = 0; n < usDataLen; n++)
	{
		ulCalcSum += byData[n];
	}

	ulCalcSum &= 0xFFFF;
	//ulCalcSum = (~ulCalcSum) + 1;
	return ulCalcSum;
}


static int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE	emEvType,
	_EVENT_MSG	emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int			nMaxBlockMSec)
{	
	SAMPLER* pSampler = (SAMPLER*)hModuleSeif;
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;
	BYTE bySendBuff[TCP_BUFF_LEN];
	memset(bySendBuff, 0, TCP_BUFF_LEN);
	int nDataSize = 0, nDataLen = 0;

	TRACE("Event_Handler:emEvType=%d emMsgType=%d nEvDataSize=%d\n",\
		emEvType,emMsgType,nEvDataSize);
	
	Mutex_Lock(pSLV->hSamplerLock, WAIT_INFINITE); //Added Lock

	if(emEvType == EVENT_TYPE_CFG_CHANGED)
	{
		if(nEvDataSize != sizeof(EVENT_PARAM_CHG))
		{
			return EVENT_HANDED_FAILED;
		}

		EVENT_PARAM_CHG *pCfgChange = (EVENT_PARAM_CHG*)pEvData;
		if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 26))
		{
			TRACE("A_BLOB_ES_BACKEND_CFG blobBackendCfg Changed, Sync To Master!\n");
			nDataSize = pSLV->msgFrameInfo[_S_C2S_CFGNTF_MSG].pfnSHandler((void *)pSampler, bySendBuff, nDataLen); 
			PackAndWriteSession(pSampler, _S_C2S_CFGNTF_MSG, bySendBuff, nDataSize);  
		}
	}
	else if(emEvType == EVENT_TYPE_SAVE_ES_STATDATA)
	{
		if((nEvDataSize != sizeof(EVENT_ES_SAVE_STATDATA)) || (emMsgType != EVENT_MSG_STATUS_CHANGED))
		{
			printf("nEvDataSize=%d EVENT_ES_SAVE_STATDATA=%d\n", nEvDataSize, sizeof(EVENT_ES_SAVE_STATDATA));
			return EVENT_HANDED_FAILED;
		}
		
		printf("-----------now EVENT_MSG_STATUS_CHANGED for EVENT_ES_SAVE_STATDATA-----------\n");

	}

	Mutex_Unlock(pSLV->hSamplerLock);
	return EVENT_HANDED_OK;

}

static BOOL Init_SLV_MsgInfo(ROUGH_SLAVE_DATA* pSLV)
{
	//Now init the msg info
	SLV_MSG_FRM_INFO *pp = &(pSLV->msgFrameInfo[0]);

	INIT_SLV_MSG_INFO(pp, _S_C2S_CFGNTF_MSG ,	0x01,	0x02,	PrepareCfgNtfCmd01,		ExtractCfgNtfCmd02,		FALSE);
	INIT_SLV_MSG_INFO(pp, _S_C2S_SYSINFO_MSG,	0x03,	0x04,	PrepareSysInfoCmd03,	ExtractSysInfoCmd04,	FALSE);
	INIT_SLV_MSG_INFO(pp, _S_C2S_BMS_MSG,		0x05,	0x06,	PrepareBmsCmd05,		ExtractBmsCmd06,		FALSE);
	INIT_SLV_MSG_INFO(pp, _S_C2S_ENGFLOW_MSG,	0x07,	0x08,	PrepareEngFlowCmd07,	ExtractEngFlowCmd08,	TRUE);
	INIT_SLV_MSG_INFO(pp, _S_C2S_SIGRECT_MSG,	0x09,	0x0A,	PrepareSigRectCmd09,	ExtractSigRectCmd0A,	FALSE);
	INIT_SLV_MSG_INFO(pp, _S_C2S_BATTCAB_MSG,	0x0B,	0x0C,	PrepareBattCabCmd0B,	ExtractBattCabCmd0C,	FALSE);
	INIT_SLV_MSG_INFO(pp, _S_C2S_STATISDATA_MSG,0x0D,	0x0E,	PrepareStaticDataCmd0D,	ExtractStaticDataCmd0E,	FALSE);

	INIT_SLV_MSG_INFO(pp, _S_S2C_SETCFG_MSG,	0xA2,	0xA1,	PrepReplySetCfgCmdA2,	ExcuteSetCfgCmdA1,		TRUE);
	INIT_SLV_MSG_INFO(pp, _S_S2C_CTRL_MSG,		0xA4,	0xA3,	PrepReplyCtrlCmdA4,		ExcuteCtrlCmdA3,		TRUE);
	INIT_SLV_MSG_INFO(pp, _S_S2C_APPSIZE_MSG,	0xA6,	0xA5,	PrepReplyAppSizeCmdA6,	ExcuteAppSizeCmdA5,		FALSE);
	INIT_SLV_MSG_INFO(pp, _S_S2C_APPUPD_MSG,	0xA8,	0xA7,	PrepReplyAppUpdCmdA8,	ExcuteAppUpdCmdA7,		FALSE);
	INIT_SLV_MSG_INFO(pp, _S_S2C_CHKSUM_MSG,	0xAA,	0xA9,	PrepReplyChkSumCmdAA,	ExcuteChkSumCmdA9,		FALSE);
	
	return TRUE;
 } 


static void Init_SLV_RoughData(ROUGH_SLAVE_DATA* pSLV)
{
	int n = 0;
	for(n = 0; n < 2; n++)
	{
		ZERO_POBJS(&pSLV->nodeCalData[n], 1);
	}
	return;
}


DLLExport BOOL Initialize(HANDLE hSamplerSelf)
{
	TRACE("111 Enter Inter Slave Sampler!!!\n");
	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
	pSampler->pDataHeap = (void*)NEW(ROUGH_SLAVE_DATA, 1);
	if(pSampler->pDataHeap == NULL)
	{
		return FALSE;
	}
	
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;
	ZERO_POBJS(pSLV, 1);

	pSLV->hSamplerLock = pSampler->hSamplerLock; //Init lock
	pSLV->hSampler = hSamplerSelf;
	pSLV->pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	

	VAR_VALUE varV;
	pSLV->pDPR->Get(hSamplerSelf, GETTYPE_ST_VAR, 1, 8, &varV, 4);
	pSLV->bySlaveAddr = varV.emValue; //From Slave-1、2、3
	TRACE("pSLV->bySlaveAddr=%d\n", pSLV->bySlaveAddr);


	Init_SLV_MsgInfo(pSLV);

	Init_SLV_RoughData(pSLV);
	
	EVHAND_TAGS tags[] = 
	{
		{EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN},
		//{EVENT_TYPE_USER_AUTHORIZE,		EVENT_MSG_BEGIN/*|EVENT_MSG_END*/}, 
		//{EVENT_TYPE_ABILITY_CHANGE,	EVENT_MSG_BEGIN | EVENT_MSG_END},
		//{EVENT_TYPE_SYSTEM_EXIT,		EVENT_MSG_BEGIN},
		{EVENT_TYPE_SAVE_ES_STATDATA,	EVENT_MSG_STATUS_CHANGED},

	};
	int n;
	for(n = 0; n < ITEM_OF(tags); n++)
	{
		pSLV->pDPR->Register_EvHandler("Slave_EvHandler",
			hSamplerSelf,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}


	return TRUE;
}


static BOOL TryConnect2Server(SAMPLER* pSampler)
{
	int nErrCode;
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;
	BYTE szURL[64]={'\0'};
	snprintf(szURL, 64, "192.168.100.100:6003");
	
	//Step1. Judge hCommPort Statu
	if(pSampler->hCommPort != NULL)
	{
		nErrCode = pSampler->pUsedCommPort->Close(pSampler->hCommPort);
		TRACE("Close The Comm Port ErrCode =[%d]\n", nErrCode);
		pSampler->hCommPort = NULL;
		return FALSE;
	}

	//!!! Need Added Timeout Judge, Process it later !!!
	
	//Step2. if Client DisConnect, Try Connect

	pSampler->hCommPort = pSampler->pUsedCommPort->Open(
			"Slave Port", 
			szURL, COMM_CLIENT_MODE, 1000, &nErrCode);

	TRACE("Connect2Server:Open nErrCode=%d szURL=[%s]\n",nErrCode,szURL);
			
	if(ERR_COMM_OK != nErrCode)
	{
		return FALSE;
	}

	if(pSampler->hCommPort == NULL)
	{	
		if(ERR_COMM_TIMEOUT != nErrCode)
		{
			return FALSE;
		}	
	}
	else
	{
		TRACE("Connect To Server Success\n");
		COMM_TIMEOUTS timeOut;
		timeOut.nReadTimeout = 100;
		timeOut.nWriteTimeout = 100;
		timeOut.nIntervalTimeout = 50;
		pSampler->pUsedCommPort->Control(pSampler->hCommPort,COMM_SET_TIMEOUTS,&timeOut, sizeof(COMM_TIMEOUTS));
	}	
	
	TRACE("Connect2Server : nErrCode =%d \n",nErrCode);
	return TRUE;
}


int Make_SLV_SendFrame(SLV_MSG_FRM_INFO* pFramInfo, IN USHORT usDataSize, IN BYTE* pData, IN BYTE bySlaveAddr, OUT BYTE *sendBuff)
{
	int n, nSendBuffLen = MIN_SLV_FRM_LEN;
	memset(sendBuff, 0, TCP_BUFF_LEN);
	BYTE *pCurrPos = sendBuff;

	A_SLV_FRM_INFO frm;
	ZERO_POBJS(&frm, 1);

	frm.bySrcAddr = bySlaveAddr;
	frm.byDestAddr = SLV_DST_ADDR;
	frm.byCmd = pFramInfo->bySCmdNo;
 	frm.ulDataLen = usDataSize;

	memcpy(frm.byData, pData, usDataSize);
	
	//Calc chksum
	frm.ulCalcSum = MakeChkSum(frm.byCmd, frm.ulDataLen, frm.byData);

	nSendBuffLen += frm.ulDataLen;

	MAKE_SLV_FRAME((pCurrPos), frm);

	//TRACE("Make_SlvMSG_SendFrame: usDataSize=%d nSendBuffLen=%d bySlaveAddr=%02X bySCmdNo=%02X\n",\
	//		usDataSize,\
	//		nSendBuffLen,\
	//		bySlaveAddr,\
	//		pFramInfo->bySCmdNo);

	return nSendBuffLen;
}


static SLV_MSG_FRM_INFO* Find_SLV_RecvFrame(IN SAMPLER *pSampler, IN BYTE* pRaw, 
		IN int nReadLen, OUT BYTE **ppDataPos, OUT USHORT *pusDataLen)
{
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA *)pSampler->pDataHeap;
 	
	BYTE* byPos = pRaw;
	USHORT nParsedDataLen = 0;
	int n= 0, m = 0;
	SLV_MSG_FRM_INFO* pFoundMsg = NULL;

	//Notice: only find the first frame, if have multi frames, ignore the followed others
	//frame format: SOI-1 SOI-1 SRC-1 DST-1 CMD-1 [LEN-2] [DATA-x] [CHKSUM-2] EOI-1

	for(n = 0; n < nReadLen - MIN_SLV_FRM_LEN + 1; n++)
	{
		//TRACE("byPos[n]=%02X byPos[n+1]=%02X byPos[n+2]=%02X byPos[n+3]=%02X byPos[n+4]=%02X\n",\
		//	byPos[n], byPos[n+1], byPos[n+2], byPos[n+3], byPos[n+4]);

		if((byPos[n] == (BYTE)SLV_PROT_SOI) && 
			(byPos[n + 1] == (BYTE)SLV_PROT_SOI) &&
			(byPos[n + 2] == (BYTE)SLV_DST_ADDR))
			//&& (byPos[n + 3] == (BYTE)pClient->nClientAddr))
		{
 			for(m = 0; m < _S_MAX_NUM_MSG; m++)
			{
				if(pSLV->msgFrameInfo[m].byRCmdNo == byPos[n + 4])
				{
					pFoundMsg = &(pSLV->msgFrameInfo[m]);
					break;
				}
			}

			//TRACE("Find Cmd ok, checkLen: byPos[n + 5]=%d byPos[n + 6]=%d\n",\
			//byPos[n + 5],byPos[n + 6]);
			//now check data length;
			nParsedDataLen = ((USHORT)byPos[n + 5] << 8) + byPos[n + 6];
			if(nParsedDataLen > TCP_BUFF_LEN)
			{
				TRACE("Found Msg[%02X] but the datalen[%d] is INVALID\n", byPos[n + 4], nParsedDataLen);
				pFoundMsg = NULL;
				continue;
			}
			
			//now check chksum
			USHORT ulCalcSum = MakeChkSum(pFoundMsg->byRCmdNo, nParsedDataLen, byPos + n + 7);
			USHORT ulRealSum = ((USHORT)byPos[n + 7 + nParsedDataLen] << 8) + byPos[n + 8 + nParsedDataLen];
			if(ulCalcSum != ulRealSum)
			{
				TRACE("Found Msg[%02X] but the chkSum[%d, %d] is DIFFERENT\n", byPos[n + 4], 
					ulCalcSum, ulRealSum);
				pFoundMsg = NULL;
				continue;
			}
			
			//EOI
			if(byPos[n + 9 + nParsedDataLen] != SLV_PROT_EOI)
			{
				TRACE("Found Msg[%02X] but the EOI[%02X] is INVALID\n", byPos[n + 4], 
					byPos[n + 9 + nParsedDataLen]);
				pFoundMsg = NULL;
				continue;
			}
			//now succeccfully get the frame
			*ppDataPos = byPos + n + 7;
			*pusDataLen = nParsedDataLen;

			//TRACE("Success Get Frame: nParsedDataLen =%d \n",nParsedDataLen);
			break;
		}
	}	
	

	return pFoundMsg;
}


BOOL PackAndWriteSession(SAMPLER* pSampler, SLV_MSG_TYPE emMsgTp, BYTE* pData, USHORT nDataSize)
{
	int i;
	BYTE byBuff[TCP_BUFF_LEN];
	memset(byBuff, 0, TCP_BUFF_LEN);

	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;

	int nSendLen = Make_SLV_SendFrame(&pSLV->msgFrameInfo[emMsgTp], nDataSize, pData, pSLV->bySlaveAddr, byBuff);
	
	
	TRACE("\n----Now begin sending msg to Master Cabinet: [%s] nSendLen=%d-------\n", byBuff, nSendLen);
	
	//for(i =0; i<nSendLen; i++)
	//{
	//	printf("%02X",byBuff[i]);
	//}
	//printf("\n");  
	
 
	int nWriteDataLen = pSampler->pUsedCommPort->Write(pSampler->hCommPort, (char*)byBuff, nSendLen);
 	if(nWriteDataLen != nSendLen)
	{
		TRACE("PackAndWriteSession Write Failed:nWriteDataLen =[%d] nSendLen=[%d]\n", nWriteDataLen, nSendLen);
		//log_cat("");
		pSLV->nCommWriteFailTimes++;
		if(pSLV->nCommWriteFailTimes > SLV_COMMFAIL_TIMES)
		{
			pSLV->nCommWriteFailTimes = 0;
			pSLV->emSLVConnStatus = DIS_CONNECTED;
 		}
 		return FALSE;
	}
	else
	{
		pSLV->nCommWriteFailTimes = 0;
	}
	

	//TRACE("Write To Tcp Port Success:nWriteDataLen=%d \n", nWriteDataLen);
	return TRUE;
}

BOOL ReadDataAndParse(SAMPLER *pSampler, IN int nTimeout_ms, IN BYTE byRecvCmd)
{
	int i;
	BOOL bRet = FALSE;
	BYTE *pDataPos;
	USHORT usDataLen;
	BYTE byBuff[TCP_BUFF_LEN];
	memset(byBuff, 0, TCP_BUFF_LEN);
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;
 
 	while(nTimeout_ms >= 0)
	{
		nTimeout_ms -= 100;
		int nReadBytes = pSampler->pUsedCommPort->Read(pSampler->hCommPort, (char*)byBuff, TCP_BUFF_LEN); 
		TRACE("\n--------ReadDataAndParse-- reply[%d] from Master Cabinet: [%s] nTimeout_ms=%d-------\n", nReadBytes, byBuff,nTimeout_ms);
		//for(i =0;i < nReadBytes ;i++)
		//{
		//	printf("%02X",byBuff[i]);
		//}
		//printf("\n");
		
		if(nReadBytes >= MIN_SLV_FRM_LEN)
		{
			bRet = TRUE;
			pSLV->nCommReadFailTimes = 0;
			//now try to find the frame and parse it
			BYTE* pFoundPos = byBuff;
			do
			{
				SLV_MSG_FRM_INFO* pFrm = Find_SLV_RecvFrame(pSampler, pFoundPos, nReadBytes, &pDataPos, &usDataLen);
				if(pFrm == NULL)
				{
					break;
				}
				else
				{
					TRACE("Found ok,pFrm->byRCmdNo=%d, nReadBytes=%d, usDataLen=%d byRecvCmd =%02x bySCmdNo=%d emMsgType=%d\n", \
						pFrm->byRCmdNo, nReadBytes, usDataLen, byRecvCmd, pFrm->bySCmdNo,pFrm->emMsgType);

					if(byRecvCmd == INVALID_RECV_CMD)
					{
						pFrm->pfnRHandler(pSampler, pDataPos, usDataLen);
					}
					else
					{
						if(pFrm->byRCmdNo == byRecvCmd)
						{
							if((pFrm->emMsgType == _S_S2C_SETCFG_MSG) || (pFrm->emMsgType ==_S_S2C_CTRL_MSG))
							{
								return (pFrm->pfnRHandler(pSampler, pDataPos, usDataLen) ==0) ? TRUE : FALSE;
							}
							else
							{
								pFrm->pfnRHandler(pSampler, pDataPos, usDataLen);
							}
						}
						else
						{
							pFrm->pfnRHandler(pSampler, pDataPos, usDataLen);
						}	
					}
				}
				nReadBytes -= usDataLen + MIN_SLV_FRM_LEN;
				pFoundPos += usDataLen + MIN_SLV_FRM_LEN;
			} while (nReadBytes >= MIN_SLV_FRM_LEN);
		}
	}	
	if(bRet == FALSE)
	{
		pSLV->nCommReadFailTimes++;
		if(pSLV->nCommReadFailTimes > SLV_COMMFAIL_TIMES)
		{
			pSLV->nCommReadFailTimes = 0;
			pSLV->emSLVConnStatus = DIS_CONNECTED;
			Init_SLV_RoughData(pSLV); 
		}
	}
	else
	{
		pSLV->nCommReadFailTimes = 0;
	}
	return bRet;
}

static BOOL WaitConnectAndRegister_Ready(SAMPLER* pSampler)
{
	int nDataLen = 0, nDataSize = 0;
	BYTE bySendBuff[TCP_BUFF_LEN];
	memset(bySendBuff, 0, TCP_BUFF_LEN);
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;

	//Judge Comm Ok/Failed ,if fail alarm
	static SLV_CONN_STATUS s_emOldConnStatus = SYNC_PENDING; //First Time Not Connected success,this statu always DISCONNECTED,Comm fail alarm not accur
	static int s_nConnStatuOnLineTimes = 0;
	if((pSLV->emSLVConnStatus != s_emOldConnStatus) &&
		((pSLV->emSLVConnStatus == DIS_CONNECTED) || (pSLV->emSLVConnStatus == SYNC_SUCCESS)))
	{
		s_nConnStatuOnLineTimes++;
		if(s_nConnStatuOnLineTimes >=5)
		{
			s_nConnStatuOnLineTimes = 0;
			s_emOldConnStatus = pSLV->emSLVConnStatus;
			VAR_VALUE varV;
			varV.emValue = (s_emOldConnStatus == DIS_CONNECTED) ? 1 : 0;
			Push_VarValue(SETTYPE_RAW_SP, pSampler, 1, 14, varV);
			TRACE("varV.emValue =%d s_emOldConnStatus=%d\n",\
				varV.emValue, s_emOldConnStatus);
 		}
	}
	else
	{
		s_nConnStatuOnLineTimes = 0;
	}
	
	if(pSLV->emSLVConnStatus == SYNC_SUCCESS)
	{
		//TRACE("emSLVConnStatus =%d-[Success]\n",pSLV->emSLVConnStatus);
		return TRUE;
	}

	// Step1. 跟服务器建立连接
	Mutex_Lock(pSLV->hSamplerLock, WAIT_INFINITE); //Added Lock
	TRACE("emSLVConnStatus=%d nCommReadFailTimes=%d nCommWriteFailTimes=%d\n",\
		pSLV->emSLVConnStatus,\
		pSLV->nCommReadFailTimes,\
		pSLV->nCommWriteFailTimes);

	if(pSLV->emSLVConnStatus == DIS_CONNECTED)
	{
		Mutex_Unlock(pSLV->hSamplerLock); //do not lock here in case the event_handler will be blocked
		TRACE("TryConnect2Server Enter Start:\n");
		TryConnect2Server(pSampler);
		TRACE("TryConnect2Server End. \n");
		if (pSampler->hCommPort == NULL)
		{
 			Sleep(5000);
 		}
		else
		{
			pSLV->emSLVConnStatus = CONNECTED;
		}
		return FALSE;
	}
	//Step2. 注册命令发送给主柜，主柜完成链接建立后; 在发送一次用于分体柜参数同步到主柜
	else if(pSLV->emSLVConnStatus == CONNECTED)
	{	
		//从柜发送命令给主柜，主柜根据地址判断是否建立连接
		nDataSize = pSLV->msgFrameInfo[_S_C2S_CFGNTF_MSG].pfnSHandler((void *)pSampler, bySendBuff, nDataLen); 
		PackAndWriteSession(pSampler, _S_C2S_CFGNTF_MSG, bySendBuff, nDataSize);  
 		ReadDataAndParse(pSampler, 100, INVALID_RECV_CMD); // No Need reply ASAP
		TRACE("--- CONNECTED --- nDataSize =%d \n",nDataSize);
 	}
	else if(pSLV->emSLVConnStatus == SYNC_PENDING)
	{
		ReadDataAndParse(pSampler, 100, INVALID_RECV_CMD);
		TRACE("--- SYNC_PENDING ---\n");
 	}
	else if(pSLV->emSLVConnStatus == SYNC_SUCCESS)
	{
		//建立连接且参数同步成功后，将从柜的后台配置参数发给主柜
		nDataSize = pSLV->msgFrameInfo[_S_C2S_CFGNTF_MSG].pfnSHandler((void *)pSampler, bySendBuff, nDataLen); 
		PackAndWriteSession(pSampler, _S_C2S_CFGNTF_MSG, bySendBuff, nDataSize);  
		TRACE("--- SYNC_SUCCESS ---\n");
 	}
	else  
	{
		pSLV->emSLVConnStatus == DIS_CONNECTED;
		TRACE("--- SYNC_FAILED ---\n");
 	}

	Mutex_Unlock(pSLV->hSamplerLock);
	return (pSLV->emSLVConnStatus == SYNC_SUCCESS) ? TRUE : FALSE;
}


void PushAllSlaveData(SAMPLER* pSampler)
{	
	int n = 0;
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;
	VAR_VALUE varV;

	for(n = 0; n < 2; n++)
	{
		Push_BlobValue(SETTYPE_LOGIC_SP, pSampler, 221 + n, 203, (void*)&pSLV->nodeCalData[n], sizeof(A_BLOB_BRANCH_NODE_CALC_DATA), BLOB_TYPE_BRANCH_NODE_CALC_DATA);
 	}

	varV.emValue = pSLV->emMasterCCUState;
	Push_VarValue(SETTYPE_LOGIC_SP, pSampler, 1, 20, varV);

	return;
}


int Sample(HANDLE hSamplerThis)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;
	time_t tmNOW1 = _NOW_;
	int nDataSize=0, n = 0, nDataLen = 0;
	BYTE bySendBuff[TCP_BUFF_LEN];
	memset(bySendBuff,0, TCP_BUFF_LEN);

	//Step1. Waiting Connect And Register
 	if(!WaitConnectAndRegister_Ready(pSampler))
	{
		Sleep(500);
		PushAllSlaveData(pSampler);
		return ERR_SAMPLE_OK;
	}

		//Step2. Read Requst/Reply Data From Master Cabinet 
 	if(Mutex_Lock(pSLV->hSamplerLock, 100) == ERR_MUTEX_OK)
	{
		ReadDataAndParse(pSampler, 100, INVALID_RECV_CMD);
		Mutex_Unlock(pSLV->hSamplerLock);
	}
	//Step3. Cycle Ask System&Gun Info From Master Cabinet
	if(fabs(tmNOW1 - pSLV->tmLastSent_SysInfo) >= 1.0)
	{
		pSLV->tmLastSent_SysInfo = tmNOW1;
		if(Mutex_Lock(pSLV->hSamplerLock, 100) == ERR_MUTEX_OK)
		{
			nDataSize = pSLV->msgFrameInfo[_S_C2S_SYSINFO_MSG].pfnSHandler((void *)pSampler, bySendBuff, nDataLen); 
			PackAndWriteSession(pSampler, _S_C2S_SYSINFO_MSG, bySendBuff, nDataSize); // PackFrame And Write to hCommPort

			Mutex_Unlock(pSLV->hSamplerLock);
			TRACE("Start _S_C2S_SYSINFO_MSG PackAndWriteSession \n");
		}
	}

	
	//Step4. Cycle Send BMS Data
	if(fabs(tmNOW1 - pSLV->tmLastSent_SingleBMS) >= 10.0)
	{
		pSLV->tmLastSent_SingleBMS = tmNOW1;
		if(Mutex_Lock(pSLV->hSamplerLock, 100) == ERR_MUTEX_OK)
		{
			nDataSize = pSLV->msgFrameInfo[_S_C2S_BMS_MSG].pfnSHandler((void *)pSampler, bySendBuff, nDataLen); 
			PackAndWriteSession(pSampler, _S_C2S_BMS_MSG, bySendBuff, nDataSize); // PackFrame And Write to hCommPort

			Mutex_Unlock(pSLV->hSamplerLock);
			TRACE("Start _S_C2S_BMS_MSG PackAndWriteSession \n");
		}
	}
	
	//Step5. Cycle Send Energy Flow Data
	if(fabs(tmNOW1 - pSLV->tmLastSent_EngFlow) >= 5.0)
	{
		pSLV->tmLastSent_EngFlow = tmNOW1;
		if(Mutex_Lock(pSLV->hSamplerLock, 100) == ERR_MUTEX_OK)
		{
			nDataSize = pSLV->msgFrameInfo[_S_C2S_ENGFLOW_MSG].pfnSHandler((void *)pSampler, bySendBuff, nDataLen); 
			PackAndWriteSession(pSampler, _S_C2S_ENGFLOW_MSG, bySendBuff, nDataSize); // PackFrame And Write to hCommPort

			Mutex_Unlock(pSLV->hSamplerLock);
			TRACE("Start _S_C2S_ENGFLOW_MSG PackAndWriteSession \n");
		}
	}

	//Step6. Cycle Send Single Rect Info
	if(fabs(tmNOW1 - pSLV->tmLastSent_SigRect) >= 10.0)
	{
		pSLV->tmLastSent_SigRect = tmNOW1;
		if(Mutex_Lock(pSLV->hSamplerLock, 100) == ERR_MUTEX_OK)
		{
			nDataSize = pSLV->msgFrameInfo[_S_C2S_SIGRECT_MSG].pfnSHandler((void *)pSampler, bySendBuff, nDataLen); 
			PackAndWriteSession(pSampler, _S_C2S_SIGRECT_MSG, bySendBuff, nDataSize); // PackFrame And Write to hCommPort

			Mutex_Unlock(pSLV->hSamplerLock);
			TRACE("Start _S_C2S_SIGRECT_MSG PackAndWriteSession \n");
		}
	}

	//Step7. Cycle Send Battery Cabinet
	if(fabs(tmNOW1 - pSLV->tmLastSent_BattCab) >= 10.0)
	{
		pSLV->tmLastSent_BattCab = tmNOW1;
		if(Mutex_Lock(pSLV->hSamplerLock, 100) == ERR_MUTEX_OK)
		{
			nDataSize = pSLV->msgFrameInfo[_S_C2S_BATTCAB_MSG].pfnSHandler((void *)pSampler, bySendBuff, nDataLen); 
			PackAndWriteSession(pSampler, _S_C2S_BATTCAB_MSG, bySendBuff, nDataSize); 

			Mutex_Unlock(pSLV->hSamplerLock);
			TRACE("Start _S_C2S_BATTCAB_MSG PackAndWriteSession \n");
		}
	}

		//Step7. Cycle Send Battery Cabinet
	if(fabs(tmNOW1 - pSLV->tmLastSent_StaticData) >= 10.0)
	{
		pSLV->tmLastSent_StaticData = tmNOW1;
		if(Mutex_Lock(pSLV->hSamplerLock, 100) == ERR_MUTEX_OK)
		{
			nDataSize = pSLV->msgFrameInfo[_S_C2S_STATISDATA_MSG].pfnSHandler((void *)pSampler, bySendBuff, nDataLen); 
			PackAndWriteSession(pSampler, _S_C2S_STATISDATA_MSG, bySendBuff, nDataSize); 

			Mutex_Unlock(pSLV->hSamplerLock);
			TRACE("Start _S_C2S_STATISDATA_MSG PackAndWriteSession \n");
		}
	}


	PushAllSlaveData(pSampler);
	
	//TRACE("Inter Slave Running\n");
	return ERR_SAMPLE_OK;
}

int Control(HANDLE hSamplerThis, PARAM_TYPE	emParaType,
	int			nDevId,
	int			nParamId,
	VAR_VALUE	*pVarVal,
	int			nValSize,
	int			nTimeOut)
{
	TRACE("emParaType =%d nDevId=%d nParamId=%d nValSize=%d \n",emParaType, nDevId,nParamId,nValSize);
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;
	BYTE byDataBuff[TCP_BUFF_LEN];
	memset(byDataBuff, 0, TCP_BUFF_LEN);
	
	VAR_VALUE varV;
	int nRet = ERR_CONTROL_OK;
	int nDataSize = 0;
	int n;
	
	
	if(Mutex_Lock(pSLV->hSamplerLock, nTimeOut) != ERR_MUTEX_OK)
	{
		return ERR_CONTROL_TIMEOUT;
	}

	if(emParaType == PARAM_TYPE_SETTING)
	{
		TRACE("emParaType =%d\n",emParaType);
	}
	else //Control
	{

	} 
	Mutex_Unlock(pSLV->hSamplerLock);
	return ERR_CONTROL_OK;
	
}

DLLExport int JigTest(HANDLE hSamplerThis, void* pJigData)
{
	return 0;
}

	

