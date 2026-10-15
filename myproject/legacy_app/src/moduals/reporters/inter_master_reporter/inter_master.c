#include "../reporters_base.h"
#include "inter_master.h"
#include <signal.h>


INTER_CLIENT 			s_Clients[MAX_SUPPORTED_CLIENTS];
static int s_nCLientTotal = 0;

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

static INTER_CLIENT *FindClient(int nWhereFrom)
{
	INTER_CLIENT *pClient = NULL;
	int n;
	if( (nWhereFrom < 0) || (nWhereFrom >= MAX_SUPPORTED_CLIENTS))
	{
		return NULL;
	}
	for(n = 0; n < MAX_SUPPORTED_CLIENTS; n++)
	{
		if(n == nWhereFrom)
		{
			pClient = &s_Clients[n];
			break;
		}
	}
	return pClient;
} 

static int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE	emEvType,
	_EVENT_MSG	emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int			nMaxBlockMSec)
{
	REPORTER* pReporter = (REPORTER*)hModuleSeif;
 	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	INTER_CLIENT *pClient;
	int n;
	BYTE byBuff[TCP_BUFF_LEN];
	memset(byBuff, 0, TCP_BUFF_LEN);

	TRACEX("Master: emEvType=%d emMsgType=%d nEvDataSize=%d\n",\
		emEvType,emMsgType,nEvDataSize);
 	switch (emEvType)
	{
		 
		case EVENT_TYPE_CFG_CHANGED:
		{
			if(nEvDataSize != sizeof(EVENT_PARAM_CHG))
			{
				return EVENT_HANDED_FAILED;
			}
			EVENT_PARAM_CHG *pCfgChange = (EVENT_PARAM_CHG*)pEvData;
			TRACE("parmPTR.nId=%d parmPTR.nSubId=%d\n",\
				pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId);
			if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 26))
			{
				TRACE("Recv ES Cfg Changed !!!\n");
				for(n =0; n < s_nCLientTotal; n++)
				{
					pClient = &s_Clients[n];
					pClient->emSyncStatus = MASTER_SYNC_PENDING; //1-Pending should SYNC to Slave Cabinet
				}
			}
			else if((pCfgChange->parmPTR.nId == 200) && (pCfgChange->parmPTR.nSubId == 201))
			{
				TRACE("Recv EMS Cfg Changed !!!\n");
				for(n =0; n < s_nCLientTotal; n++)
				{
					pClient = &s_Clients[n];
					pClient->emSyncStatus = MASTER_SYNC_PENDING_EMSCFG; //1-Pending should SYNC to Slave Cabinet
				}
			}
			else if((pCfgChange->parmPTR.nId == 500) && (pCfgChange->parmPTR.nSubId == 501))
			{
				TRACE("Recv Batt Mgmt Changed !!!\n");
				for(n =0; n < s_nCLientTotal; n++)
				{
					pClient = &s_Clients[n];
					pClient->emSyncStatus = MASTER_SYNC_PENDING_BATTMGMT; //1-Pending should SYNC to Slave Cabinet
				}
			}
		}
		break;

		case EVENT_TYPE_FIRMWARE_UPDATE:
			{
				TRACE("Inter Master EventMsgType = %d\n", emMsgType);
				if((nEvDataSize != sizeof(EVENT_FIRMWAREUPDATE_STATUS)) || (emMsgType != EVENT_MSG_STATUS_CHANGED))
				{
					return EVENT_HANDED_FAILED;
				}
				
				EVENT_FIRMWAREUPDATE_STATUS	*pFirmUpdt = (EVENT_FIRMWAREUPDATE_STATUS *)pEvData;
				if(pFirmUpdt->emUpdateStatus != FIRMUPDATE_Installing)
				{
					return EVENT_HANDED_FAILED;
				}

				for(n =0; n < s_nCLientTotal; n++)
				{
					pClient = &s_Clients[n];
					pClient->tagMstUpgradeParam.emUpgradeStatus = (pClient->tagMstUpgradeParam.emUpgradeStatus == UPDOWN_PENDING) ? 
											pClient->tagMstUpgradeParam.emUpgradeStatus : UPDOWN_WAIT_BEGIN;   
					pClient->tagMstUpgradeParam.tm_StartUpgrade = _NOW_;
				}
				TRACE("Start Remote Update Slave App emUpgradeStatus=%d s_nCLientTotal=%d\n",\
					 pClient->tagMstUpgradeParam.emUpgradeStatus, s_nCLientTotal);
			}
			break;
		default:
			break;
	}
	return EVENT_HANDED_OK;
}


static int  Parse_Frame(IN BYTE* pRaw, IN int nReadLen, OUT int *nClientAddr)
{
	int n= 0;
	int nCmdType = 0;

 	BYTE* byPos = pRaw;
	USHORT nParsedDataLen = 0;

	//Notice: only find the first frame, if have multi frames, ignore the followed others
	//frame format: SOI-1 SOI-1 SRC-1 DST-1 CMD-1 [LEN-2] [DATA-x] [CHKSUM-2] EOI-1

	for(n = 0; n < nReadLen - MIN_MST_FRM_LEN + 1; n++)
	{		
		TRACEX("byPos[n]=%02X byPos[n+1]=%02X byPos[n+2]=%02X byPos[n+3]=%02X byPos[n+4]=%02X\n",\
			byPos[n], byPos[n+1], byPos[n+2], byPos[n+3], byPos[n+4]);

		if((byPos[n] == (BYTE)MST_PROT_SOI) && 
			(byPos[n + 1] == (BYTE)MST_PROT_SOI) &&
			(byPos[n + 3] == (BYTE)MST_SRC_ADDR))
		{
			*nClientAddr =(int)byPos[n + 2];
			if((*nClientAddr == 0) || (*nClientAddr == MST_SRC_ADDR))
			{
				printf("Wrong Addr\n");
				continue;
			}
			nCmdType = (int)byPos[n + 4] ;

			TRACE("Success Get Frame: *nClientAddr =%d nCmdType=%d\n",*nClientAddr, nCmdType);
			break;
		}
	}	
	
	TRACE("nCmdType =%d \n",nCmdType);
	return nCmdType;
}

static BOOL ReplaceOldClient(INTER_CLIENT *pClient, HANDLE hDataComm)
{
	pClient->hDataComm = hDataComm;
	pClient->emStatus = CLIENT_STATU_INSTALLED;
	pClient->emSyncStatus = MASTER_SYNC_START;
	pClient->nCommFailTimes = 0;
	return TRUE;
}


BOOL Check_ValidClient(REPORTER* pReporter, HANDLE hDataComm, int *nClientAddr)
{
	COMMPORT* pComm = pReporter->pUsedCommPort;
	int i, n;
	int nReadBuffLen = 0 , nTryTimes = 20; 
	BYTE szReadBuffer[TCP_BUFF_LEN];
	memset(szReadBuffer, 0, TCP_BUFF_LEN);
	int nClient_Addr;
	VAR_VALUE varV;
	BOOL bFoundLastConnectAddr = FALSE;
	while(nTryTimes > 0)
	{
		nTryTimes--;
		nReadBuffLen = pComm->Read(hDataComm, szReadBuffer, TCP_BUFF_LEN);
		if(nReadBuffLen > MIN_MST_FRM_LEN)
		{
			//TRACE("Master Read DataLen=[%d]:\n", nReadBuffLen );
			//for(i =0; i < nReadBuffLen; i++)
			//{
			//	printf("%02X", szReadBuffer[i]);
			//}
			//printf("\n");
			
			if(Parse_Frame(szReadBuffer, nReadBuffLen, &nClient_Addr) == MST_NTF_CMD)
			{
				for(n = 0; n < MAX_SUPPORTED_CLIENTS; n++)
				{
					if(nClient_Addr == s_Clients[n].nClientAddr)
					{
						bFoundLastConnectAddr = TRUE;
						break;
					}
				}

				for(n = 0; n < MAX_SUPPORTED_CLIENTS; n++)
				{
					if(nClient_Addr == s_Clients[n].nClientAddr) //已经创建过的，则只能替换，不能再创建新的线程
					{
						if(CLIENT_STATU_COMM_FAIL == s_Clients[n].emStatus)
						{
							varV.emValue = 0;
							pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
								1, 15, &varV, 4, 100);

							//this means the client is re-connecting, so replace the old
							TRACE("Reconnecting ,Replace Old One s_Clients[%d].emStatus=%d  byClient_Addr=%d\n", \
								n, s_Clients[n].emStatus, nClient_Addr);
							
							if(s_Clients[n].hDataComm != NULL)
							{
								pComm->Close(s_Clients[n].hDataComm);
								s_Clients[n].hDataComm = NULL;
							}
							ReplaceOldClient(&s_Clients[n], hDataComm);
							return FALSE;
						}
						else
						{
							//conflicted ,Alarm 
							TRACE("conflicted ,Alarm  s_Clients[%d].emStatus =%d\n", n, s_Clients[n].emStatus);							
							varV.emValue = 1;
							pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
								1, 15, &varV, 4, 100);

							if(hDataComm != NULL)
							{
								pComm->Close(hDataComm);
								hDataComm = NULL;
							}
							return FALSE;
						}		
					}
					else 
					{
						if(bFoundLastConnectAddr == FALSE) //Not Found LastTime Connect Addr
						{
							TRACE("Need Create New Thread s_Clients[%d].nClientAddr =%d \n", n, s_Clients[n].nClientAddr);
							*nClientAddr = nClient_Addr;
							return TRUE;
						}
					}		
				}			
			}
			else
			{
				TRACE("nTryTimes =%d \n",nTryTimes);
			}	
		}
		Sleep(100);
	}

	return FALSE;
}

#define MAKE_MST_FRAME(pos, frm) \
					do{\
						pos[0] = (MST_PROT_SOI), \
						pos[1] = (MST_PROT_SOI), \
						pos[2] = (frm.bySrcAddr), \
						pos[3] = (frm.byDestAddr), \
						pos[4] = (frm.byCmd), \
						pos[5] = ((BYTE)((frm.ulDataLen) >> 8)), \
						pos[6] = ((BYTE)(frm.ulDataLen)); \
						memcpy(pos + 7, frm.byData, frm.ulDataLen); \
						pos[frm.ulDataLen + 7] = ((BYTE)((frm.ulCalcSum) >> 8)), \
						pos[frm.ulDataLen + 8] = ((BYTE)(frm.ulCalcSum)); \
						pos[frm.ulDataLen + 9] = (MST_PROT_EOI);\
					}while(0)

static int Make_MST_SendFrame(MST_MSG_FRM_INFO *pMsgFrm, IN USHORT usDataSize, IN BYTE *pData, OUT BYTE *sendBuff)
{
	int n, nSendBuffLen = MIN_MST_FRM_LEN;
	memset(sendBuff, 0, TCP_BUFF_LEN);
	BYTE *pCurrPos = sendBuff;

	A_MST_FRM_INFO frm;
	ZERO_POBJS(&frm, 1);

	frm.bySrcAddr = MST_SRC_ADDR;
	frm.byDestAddr = (BYTE)1;
	frm.byCmd = pMsgFrm->bySCmdNo;
 	frm.ulDataLen = usDataSize;

	memcpy(frm.byData, pData, usDataSize);
	
	//Calc chksum
	frm.ulCalcSum = MakeChkSum(frm.byCmd, frm.ulDataLen, frm.byData);

	nSendBuffLen += frm.ulDataLen;

	MAKE_MST_FRAME((pCurrPos), frm);

	TRACE("Make_MST_SendFrame: usDataSize=%d nSendBuffLen=%d frm.byCmd=%02x\n",\
			usDataSize,\
			nSendBuffLen,\
			frm.byCmd);

	return nSendBuffLen;
}

BOOL Send_MST_Frame(INTER_CLIENT *pClient,  MASTER_MSG_TYPE emMsgTp, BYTE* pData, USHORT nDataSize)
{
	int i;
	BYTE byBuff[TCP_BUFF_LEN];
	memset(byBuff, 0 , TCP_BUFF_LEN);

	REPORTER* pReporter = (REPORTER*)pClient->pReporter;

	int nSendLen = Make_MST_SendFrame(&pClient->msgFrameInfo[emMsgTp], nDataSize, pData, byBuff);

	TRACE("\n-----------Now begin sending msg to Client=[%d] Slave Cabinet: [%s]----------\n",\
			pClient->nClientAddr, byBuff);
	/*for(i =0; i<nSendLen; i++)
	{
		printf("%02X",byBuff[i]);
	}
	printf("\n");  
	*/
	
	int nWriteDataLen = pReporter->pUsedCommPort->Write(pClient->hDataComm, (char*)byBuff, nSendLen);
 	if(nWriteDataLen != nSendLen)
	{
		TRACE("Send_MST_Frame Write Failed:nWriteDataLen =[%d] nSendLen=[%d]\n", nWriteDataLen, nSendLen);
		//LogOut(MAIN_MODULE_REPORTER, "InterMaster", LOG_TYPE_ERROR, "ClientId=%d nWriteDataLen=%d nSendLen=%d",pClient->nClientAddr,nWriteDataLen,nSendLen);
 		return FALSE;
	}
	TRACE("Write To Slave Cabinet Success:nWriteDataLen=%d \n", nWriteDataLen);
	return TRUE;
}


static MST_MSG_FRM_INFO* Find_Master_RecvFrame(IN INTER_CLIENT *pClient, IN BYTE* pRaw, 
		IN int nReadLen, OUT BYTE **ppDataPos, OUT USHORT *pusDataLen)
{
	REPORTER* pReporter = (REPORTER*)pClient->pReporter;
	BYTE* byPos = pRaw;
	USHORT nParsedDataLen = 0;
	int n= 0, m = 0;
	MST_MSG_FRM_INFO* pFoundMsg = NULL;

	//Notice: only find the first frame, if have multi frames, ignore the followed others
	//frame format: SOI-1 SOI-1 SRC-1 DST-1 CMD-1 [LEN-2] [DATA-x] [CHKSUM-2] EOI-1

	for(n = 0; n < nReadLen - MIN_MST_FRM_LEN + 1; n++)
	{
		
		TRACE("byPos[n]=%02X byPos[n+1]=%02X byPos[n+2]=%02X byPos[n+3]=%02X byPos[n+4]=%02X\n",\
			byPos[n], byPos[n+1], byPos[n+2], byPos[n+3], byPos[n+4]);

		if((byPos[n] == (BYTE)MST_PROT_SOI) && 
			(byPos[n + 1] == (BYTE)MST_PROT_SOI) &&
			//(byPos[n + 2] == (BYTE)pClient->nClientAddr)&&
			(byPos[n + 3] == (BYTE)MST_SRC_ADDR))
		{
 			for(m = 0; m < _M_MAX_NUM_MSG; m++)
			{
				if(pClient->msgFrameInfo[m].byRCmdNo == byPos[n + 4])
				{
					pFoundMsg = &(pClient->msgFrameInfo[m]);
					break;
				}
			}

			TRACE("Find Cmd ok, checkLen: byPos[n + 5]=%d byPos[n + 6]=%d\n",\
			byPos[n + 5],byPos[n + 6]);
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
			if(byPos[n + 9 + nParsedDataLen] != MST_PROT_EOI)
			{
				TRACE("Found Msg[%02X] but the EOI[%02X] is INVALID\n", byPos[n + 4], 
					byPos[n + 9 + nParsedDataLen]);
				pFoundMsg = NULL;
				continue;
			}
			//now succeccfully get the frame
			*ppDataPos = byPos + n + 7;
			*pusDataLen = nParsedDataLen;

			TRACE("Success Get Frame: nParsedDataLen =%d \n",nParsedDataLen);
			break;
		}
	}	
	
	return pFoundMsg;
}


BOOL Handle_ClientNormalRequest(INTER_CLIENT* pClient)
{
	REPORTER* pReporter = (REPORTER*)pClient->pReporter;
	int nReadBytes = pClient->nReadBuffLen;

	if(nReadBytes >= MIN_MST_FRM_LEN)
	{
		//now try to find the frame and parse it
		BYTE* pFoundPos = pClient->szReadBuffer;
		do
		{
			BYTE *pDataPos;
			USHORT usDataLen;
			MST_MSG_FRM_INFO* pFrm = Find_Master_RecvFrame(pClient, pFoundPos, nReadBytes, &pDataPos, &usDataLen);
			if(pFrm == NULL)
			{
				break;
			}
			else
			{
				TRACE("Found Master Frame ok, pFrm->byRCmdNo=%d, nReadBytes=%d, usDataLen=%d\n", \
					pFrm->byRCmdNo, nReadBytes, usDataLen);

				pFrm->pfnRHandler(pClient, pDataPos, usDataLen);

			}
				
			nReadBytes -= usDataLen + MIN_MST_FRM_LEN;
			pFoundPos += usDataLen + MIN_MST_FRM_LEN;

		} while (nReadBytes >= MIN_MST_FRM_LEN);
	}

	return TRUE;
}

static void Init_SlaveNodeStuffData(INTER_CLIENT *pClient)
{
	REPORTER *pReporter = pClient->pReporter;

	VAR_VALUE varV;
	ERRCODE_DAI err;
	
	varV.fValue = 0;
 	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
							1, 32, &varV, 4, 100);
	varV.fValue = 0;
	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
							1, 33, &varV, 4, 100);
	varV.fValue = 0;
	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
							1, 34, &varV, 4, 100);

	A_BLOB_BRANCH_GRP_RECTDATA  blobRectGrp_1;
	ZERO_POBJS(&blobRectGrp_1, 1);
	varV.blobValue = &blobRectGrp_1;
 	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
					213, 201, &varV, sizeof(A_BLOB_BRANCH_GRP_RECTDATA), 1000);
	
	A_BLOB_BRANCH_GRP_RECTDATA  blobRectGrp_2;
	ZERO_POBJS(&blobRectGrp_2, 1);
	varV.blobValue = &blobRectGrp_2;
 	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
					214, 201, &varV, sizeof(A_BLOB_BRANCH_GRP_RECTDATA), 1000);
	

	TRACE("--- Init_SlaveNodeStuffData ---\n");

	return;
}

static BOOL SyncSysSetting2Slave(INTER_CLIENT *pClient)
{
	TRACE("SyncSysSetting2Slave Start \n");
	REPORTER *pReporter = pClient->pReporter;
	ERRCODE_DAI err;
	PARAM_VAL_GET valGet;
	int nTimeOut = 1000;
	int nSendLen = 0;
	
	BYTE bySubCmd;

 	BYTE byBuff[TCP_BUFF_LEN];
	memset(byBuff, 0, TCP_BUFF_LEN);

	//pClient->emSyncStatus = MASTER_SYNC_PENDING;  
	 
	if((pClient->emSyncStatus == MASTER_SYNC_PENDING_EMSCFG)||(pClient->emSyncStatus == MASTER_SYNC_START))
	{
		TRACE("--Sync EMS--\n");
		A_BLOB_EMS_CONFIG_PLAN 		blobEmsCfg;
 		ZERO_POBJS(&blobEmsCfg, 	1);
 		valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
		valGet.val.blobValue = (BYTE*)&blobEmsCfg;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			200, 201, &valGet, sizeof(PARAM_VAL_GET));

		TRACE("nTotal_EmsLmt_Sectors=%d tmStart=%ld nInterval=%d emBatt_WorkMode=%d\n", \
			blobEmsCfg.nTotal_EmsLmt_Sectors, \
			blobEmsCfg.emsLmtSectors[0].tmStart,\
			blobEmsCfg.emsLmtSectors[0].nInterval,\
			blobEmsCfg.emsLmtSectors[0].emBatt_WorkMode);

		if(blobEmsCfg.nTotal_EmsLmt_Sectors > 0)
		{
			int nEMSBlobLen = sizeof(A_BLOB_EMS_CONFIG_PLAN);
			nSendLen = 0;

			byBuff[nSendLen] = 0x01;
			nSendLen += 1;

			byBuff[nSendLen] = (BYTE)(nEMSBlobLen >> 8);
			nSendLen += 1;

			byBuff[nSendLen] = (BYTE)(nEMSBlobLen & 0xFF);
			nSendLen += 1;

			memcpy(&byBuff[nSendLen], &blobEmsCfg , sizeof(A_BLOB_EMS_CONFIG_PLAN));

			nSendLen += nEMSBlobLen;

			//-----Step2. Send Data-----// 
			Send_MST_Frame(pClient, _M_S2C_SETCFG_MSG, byBuff, nSendLen);

			TRACE(" nEMSBlobLen =%d nSendLen =%d End \n", nEMSBlobLen, nSendLen);
		} 
	} 
	
	//if((pClient->emSyncStatus == MASTER_SYNC_PENDING_BATTMGMT) || (pClient->emSyncStatus == MASTER_SYNC_START))
	//{
	//	TRACE("--Sync Batt Mgmt--\n");
	//	A_BLOB_BATT_MGMT_PLAN blobBattMgmt;
	//	ZERO_POBJS(&blobBattMgmt, 	1);
	//
	//	valGet.nGetSize = sizeof(A_BLOB_BATT_MGMT_PLAN);
	//	valGet.val.blobValue = (BYTE*)&blobBattMgmt;
	//	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
	//		500, 501, &valGet, sizeof(PARAM_VAL_GET));
	//
	//	if(blobBattMgmt.nTotal_STBattLmt_Sectors > 0)
	//	{
	//		int nBattMgmtBlobLen = sizeof(A_BLOB_BATT_MGMT_PLAN);
	//		nSendLen = 0;
	//
	//		byBuff[nSendLen] = 0x02;
	//		nSendLen += 1;
	//
	//		byBuff[nSendLen] = (BYTE)(nBattMgmtBlobLen >> 8);
	//		nSendLen += 1;
	//
	//		byBuff[nSendLen] = (BYTE)(nBattMgmtBlobLen & 0xFF);
	//		nSendLen += 1;
	//
	//		memcpy(&byBuff[nSendLen], &blobBattMgmt , sizeof(A_BLOB_BATT_MGMT_PLAN));
	//
	//		nSendLen += nBattMgmtBlobLen;
	//
	//		//-----Step2. Send Data-----// 
	//		Send_MST_Frame(pClient, _M_S2C_SETCFG_MSG, byBuff, nSendLen);
	//
	//		TRACE(" nEMSBlobLen =%d nSendLen =%d End \n", nBattMgmtBlobLen, nSendLen); 
	//	}
	//}

	TRACE("No diffrence, Sync OK\n");
	//此处再主动发一下SYNC成功的命令，这条命令对于分体柜很重要
	pClient->emSyncStatus = MASTER_SYNC_FINISHED;
	//0-Start, 1-Pending, 2-Finished
	byBuff[0] = (BYTE)pClient->emSyncStatus;

	//-----Step2. Send Data-----// 
	Send_MST_Frame(pClient, _M_C2S_CFGNTF_MSG, byBuff, 1);
	
 	return TRUE;
}


BOOL ReadAndHandleRecvData(INTER_CLIENT *pClient)
{
	REPORTER *pReporter = pClient->pReporter;
	COMMPORT *pComm = pReporter->pUsedCommPort;
	VAR_VALUE varV;
	pClient->nReadBuffLen = pComm->Read(pClient->hDataComm, pClient->szReadBuffer, TCP_BUFF_LEN);
	if(pClient->nReadBuffLen > 0)
	{	
		if(pClient->emStatus != CLIENT_STATU_COMM_OK)
		{
			varV.emValue = 2; //Normal
			pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
							1, (144+pClient->nClientAddr-1), &varV, 4, 100);
		}
		pClient->nCommFailTimes = 0;
		pClient->emStatus = CLIENT_STATU_COMM_OK;
		TRACEX("Master Cabinet[%d] Read DataLen=[%d]:\n", pClient->nClientAddr, pClient->nReadBuffLen );

		//int i;
		//for(i =0;i < pClient->nReadBuffLen; i++)
		//{
		//	printf("%02X", pClient->szReadBuffer[i]);
		//}
		//printf("\n");
		
		//Now handling the client requst
		Handle_ClientNormalRequest(pClient);
 	}
	else
	{
		pClient->nCommFailTimes++;
		if(pClient->nCommFailTimes > 50)//10S
		{
			if(pClient->emStatus != CLIENT_STATU_COMM_FAIL)
			{
				varV.emValue = 1; //Alarm
				pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
								1, (144+pClient->nClientAddr-1), &varV, 4, 100);
				Init_SlaveNodeStuffData(pClient);   
			
			}
			pClient->nCommFailTimes = 0;
			pClient->emStatus = CLIENT_STATU_COMM_FAIL;	
			pClient->emSyncStatus = MASTER_SYNC_START;
			if(pClient->hDataComm != NULL)
			{
				pClient->pReporter->pUsedCommPort->Close(pClient->hDataComm);
				pClient->hDataComm = NULL;  //Close后该指针可能不是NULL
			}
		}
		TRACEX("nClientAddr=[%d] Comm Failed,->emStatus=%d ->nCommFailTimes=%d\n",\
				pClient->nClientAddr,\
				pClient->emStatus,\
				pClient->nCommFailTimes);
	}

	return TRUE;
}

static void NotifyInterMasterEnterUpgrade(IN INTER_CLIENT *pClient)
{
	BYTE bySendBuf[10];
	memset(bySendBuf, 0, 10);
	
	MakeUlong(pClient->strUpdownFileInfo.nTotalFileSize, bySendBuf);

  	//-----Step2. Send Data-----// 
	Send_MST_Frame(pClient, _M_S2C_APPSIZE_MSG, bySendBuf, 4);

	ReadAndHandleRecvData(pClient);	

	TRACE("NotifyInterMasterEnterUpgrade ...\n");
	return;
}

static BOOL SendSingleRpkg(INTER_CLIENT *pClient, BYTE *pDataPos, int nSingleMsgLen)
{
	BOOL bRet = TRUE;
	int nSendLen = 0;
	int nMsgId = pClient->tagMstUpgradeParam.nMsgId + 1;
	BYTE bySendBuf[TCP_BUFF_LEN];
	memset(bySendBuf, 0,  TCP_BUFF_LEN);
	
	MakeShort(nMsgId, bySendBuf);
    nSendLen += 2;

    memcpy(&bySendBuf[2], pDataPos, nSingleMsgLen);
	nSendLen += nSingleMsgLen;

	//-----Step2. Send Data-----// 
	Send_MST_Frame(pClient, _M_S2C_APPUPD_MSG, bySendBuf, nSendLen);

	ReadAndHandleRecvData(pClient);	

	if(nMsgId != pClient->tagMstUpgradeParam.nRecvMsgId)
	{
		bRet = FALSE;
		TRACE("nClientAddr=[%d]-Wrong MsgID: nRecvMsgId=%d nMsgId=%d\n",pClient->nClientAddr, pClient->tagMstUpgradeParam.nRecvMsgId, nMsgId);
	}
	
	TRACE("nClientAddr=[%d]-Send OK : nRecvMsgId=%d nMsgId=%d\n",pClient->nClientAddr, pClient->tagMstUpgradeParam.nRecvMsgId, nMsgId);

	return bRet;
}


static BOOL  SendUpgradeFileRpkg(INTER_CLIENT *pClient, BYTE *pbyReadBuf, ULONG nTotalFileSize)
{
	#define RETRY_SEND_TIMES 	3
	#define MAX_SINGLE_MSGLEN 	8100
	BOOL bRet = TRUE;
	BOOL bSingleMsgRet = FALSE;
	int nDataOffset = 0;
	int nRemainLen = 0;
 	int nSingleMsgLen = 0;
 	int nFailTimes = 0;
	BYTE *pDataPos;

	TRACE("---StartTime=%ld--- nFileDataLen =%d nRemainLen=%d nDataOffset=%d\n",_NOW_, nTotalFileSize,nRemainLen,nDataOffset);
	for(nDataOffset = 0; nDataOffset < nTotalFileSize; )
	{
		nRemainLen = nTotalFileSize - nDataOffset;
        pDataPos = pbyReadBuf + nDataOffset;
        nSingleMsgLen = (nRemainLen > MAX_SINGLE_MSGLEN) ? MAX_SINGLE_MSGLEN : nRemainLen;
        if(pDataPos == NULL)
        {
			TRACE("DataPos Is NULL, Go Back\n");
			bRet = FALSE;
            break;
        }
        bSingleMsgRet = SendSingleRpkg(pClient, pDataPos, nSingleMsgLen);
        if(bSingleMsgRet == FALSE)
        {
            TRACE("nRet < 0, Transfer file Failure nFailTimes=%d\n",nFailTimes);
            nFailTimes++;
            if(nFailTimes < RETRY_SEND_TIMES)
            {
                continue;
            }
			else
            {
                bRet = FALSE;
                break;  
            }
        }
		else
		{
			nFailTimes = 0;
		}
		
        pClient->tagMstUpgradeParam.nMsgId++;
        nDataOffset += nSingleMsgLen;
        //TRACE("nFileDataLen =%d nRemainLen=%d nDataOffset=%d\n", nTotalFileSize,nRemainLen,nDataOffset);
	}
	TRACE("---EndTime=%ld---\n", _NOW_); 
	return bRet;
}


static void HandleRemoteUpgrading(IN INTER_CLIENT *pClient)
{
	BOOL bSendFileRet = FALSE;
 	TRACE("Enter HandleRemoteUpgrading!!!\n");
	if(pClient->tagMstUpgradeParam.emUpgradeStatus == UPDOWN_WAIT_BEGIN)
	{
		TRACE("Server : UPDOWN_WAIT_BEGIN\n");
		//Process Upgrade File
		pClient->strUpdownFileInfo.pbyReadBuf = NEW(BYTE, MAX_UPDOWN_FILESIZE);
		if(!pClient->strUpdownFileInfo.pbyReadBuf)
		{
			pClient->tagMstUpgradeParam.emUpgradeStatus = UPDOWN_IDLE;
			TRACE("Malloc FileSize Fault!!!\n");
			return;
		}
		memset(pClient->strUpdownFileInfo.pbyReadBuf, 0, MAX_UPDOWN_FILESIZE);

		pClient->strUpdownFileInfo.nFileHandle = open(LOCAL_UPGRADE_FIRMWARE_NAME, O_RDONLY);
		if(pClient->strUpdownFileInfo.nFileHandle < 0)
		{
			pClient->tagMstUpgradeParam.emUpgradeStatus = UPDOWN_IDLE;
			SAFELY_DELETE(pClient->strUpdownFileInfo.pbyReadBuf);
			TRACE("Open Upgrade File Failure!\n");
			return;
		}
		else
		{
			pClient->strUpdownFileInfo.nTotalFileSize = read(pClient->strUpdownFileInfo.nFileHandle,
												pClient->strUpdownFileInfo.pbyReadBuf,
												MAX_UPDOWN_FILESIZE);
			TRACE("Read Upgrade File Success, nTotalFileSize=%d\n", pClient->strUpdownFileInfo.nTotalFileSize);
			close(pClient->strUpdownFileInfo.nFileHandle);

			// Init UpgradeParam
			pClient->tagMstUpgradeParam.nMsgId = 0;
			pClient->tagMstUpgradeParam.bInterSlvReady4Upgrade = FALSE;
			pClient->tagMstUpgradeParam.emUpgradeStatus = UPDOWN_PENDING;
			pClient->tagMstUpgradeParam.emSlvUpdateStatus = FIRMUPDATE_Idle;  
		}
	}
	else if(pClient->tagMstUpgradeParam.emUpgradeStatus == UPDOWN_PENDING)
	{
		TRACE("Server : UPDOWN_PENDING\n");
		if(ABS(_NOW_ - pClient->tagMstUpgradeParam.tm_StartUpgrade) > 10)
		{
			pClient->tagMstUpgradeParam.emUpgradeStatus = UPDOWN_FINISHED;
			TRACE("On_Pending TimeOut !!!\n");
			return;
		}

		if(pClient->tagMstUpgradeParam.bInterSlvReady4Upgrade)
		{	
			pClient->tagMstUpgradeParam.bInterSlvReady4Upgrade = FALSE;
			bSendFileRet = SendUpgradeFileRpkg(pClient, pClient->strUpdownFileInfo.pbyReadBuf, pClient->strUpdownFileInfo.nTotalFileSize);
			if(bSendFileRet == TRUE)
			{
				TRACE("Remote Send Upgrade File Success !\n");
				LogOut(MAIN_MODULE_REPORTER, "Inter_Master", LOG_TYPE_INFO,
					"Send firmware SUCCESSFULLY to ClientAddr[%d] SlvUpdateStatus=[%d]!", pClient->nClientAddr,pClient->tagMstUpgradeParam.emSlvUpdateStatus);
			}
			else
			{
				TRACE("Remote Send Upgrade File Failed !\n");
				LogOut(MAIN_MODULE_REPORTER, "Inter_Master", LOG_TYPE_INFO,
					"Send firmware Failed to ClientAddr[%d] SlvUpdateStatus=[%d]!", pClient->nClientAddr,pClient->tagMstUpgradeParam.emSlvUpdateStatus);
			}	
			//shoule Notify SysSampler Upgrade Result
			pClient->tagMstUpgradeParam.emUpgradeStatus = UPDOWN_FINISHED;
		}
		else
		{
			NotifyInterMasterEnterUpgrade(pClient);
		}
			
	}
	else if(pClient->tagMstUpgradeParam.emUpgradeStatus == UPDOWN_FINISHED)
	{
		TRACE("Server : UPDOWN_FINISHED\n");
		pClient->tagMstUpgradeParam.emUpgradeStatus = UPDOWN_IDLE;
		SAFELY_DELETE(pClient->strUpdownFileInfo.pbyReadBuf);
	}
	else  
	{
		pClient->tagMstUpgradeParam.emUpgradeStatus = UPDOWN_IDLE;
	}
	return;
}

static DWORD _ThreadEntry_ClientComm(void* pArg)
{
	int i = 0;
	INTER_CLIENT* pClient = (INTER_CLIENT*)pArg;
	REPORTER *pReporter = pClient->pReporter;
	COMMPORT *pComm = pReporter->pUsedCommPort;
	BYTE bySendBuff[TCP_BUFF_LEN]={'\0'};
	VAR_VALUE varV;
	while (!pClient->bNeedExit)
	{
		Sleep(100);
		time_t _tmNow = GetCurrentTime();
		if(pClient->emStatus == CLIENT_STATU_COMM_FAIL)
		{
			//wait for another session
			TRACE("wait for another session:nClientAddr=%d emStatus=%d\n", \
				pClient->nClientAddr, pClient->emStatus);
			Sleep(500);
			continue;
		}
		
		if(Mutex_Lock(pClient->hMutex, 100) == ERR_MUTEX_OK)
		{
			if(pClient->emSyncStatus != MASTER_SYNC_FINISHED)
			{
				if(!SyncSysSetting2Slave(pClient))
				{
					ReadAndHandleRecvData(pClient);	
					TRACEX("Here Waitting Sysc Sys Setting Finished :nSyncTimes=%d\n", pClient->nSyncTimes);
					pClient->nSyncTimes++;
					if(pClient->nSyncTimes >=100) //OVER sync 100 Times
					{
						pClient->nSyncTimes = 100;
						varV.emValue = 1; //Sync alarm
						pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
							1, (149+pClient->nClientAddr-1), &varV, 4, 100);
					}
					Mutex_Unlock(pClient->hMutex);
					continue;
				}
				else
				{
					varV.emValue = 0; //Normal
					pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
								1, (149+pClient->nClientAddr-1), &varV, 4, 100);
				}
			}	
			Mutex_Unlock(pClient->hMutex);
		}

		if(Mutex_Lock(pClient->hMutex, 100) == ERR_MUTEX_OK)
		{
			ReadAndHandleRecvData(pClient);
			Mutex_Unlock(pClient->hMutex);
		}
 		TRACE("Cycle Running In _ThreadEntry_ClientComm nClientAddr=%d _tmNow =%ld\n", pClient->nClientAddr, _tmNow);

		/*
		if(Mutex_Lock(pClient->hMutex, 1000) == ERR_MUTEX_OK)
		{
			if(pClient->tagMstUpgradeParam.emUpgradeStatus != UPDOWN_IDLE)
			{
				HandleRemoteUpgrading(pClient);
			}
			Mutex_Unlock(pClient->hMutex);
		}
		*/
	}

	
	//clean
	pComm->Close(pClient->hDataComm);
}

static BOOL Create_ClientSession(INTER_CLIENT *pClient)
{
	int nRet;
	static int nHasCreateThreadNum = 0;
 
	if(pClient->emStatus != CLIENT_STATU_NOT_INSTALLED 
		|| nHasCreateThreadNum >= MAX_SUPPORTED_CLIENTS)
	{
		TRACE("pthread Already Created, Go Back nHasCreateThreadNum=%d!!!\n",nHasCreateThreadNum);
		return FALSE;
	}

	pthread_t hClientThread[MAX_SUPPORTED_CLIENTS];
	nRet= pthread_create( &hClientThread[nHasCreateThreadNum], 
							NULL,
							(PTHREAD_START_ROUTINE)_ThreadEntry_ClientComm,
							(void *)pClient);
	if(nRet < 0)
	{
		LogOut(MAIN_MODULE_REPORTER, "Inter_Master", LOG_TYPE_ERROR,
				"Create Current Thread Num=[%d] Failed", nHasCreateThreadNum);
			return FALSE;
	}
	else
	{
		nHasCreateThreadNum++;
		pClient->emStatus = CLIENT_STATU_INSTALLED;
		LogOut(MAIN_MODULE_REPORTER, "Inter_Master", LOG_TYPE_INFO,
				"Create Thread=[%lu] Success,TotalCreateNum=[%d]",hClientThread[nHasCreateThreadNum-1],nHasCreateThreadNum);
	}
	
	return TRUE;
}


static DWORD _ThreadEntry_ClientMgmt(void* pArg)
{
	REPORTER* pReporter = (REPORTER*)pArg;
	HANDLE hCommPort = pReporter->hCommPort;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	int nReadBytes = 0;
	int n, nErrCode, nClientAddr;
	HANDLE hDataComm;
	
	while(!pReporter->bExit)
	{
		RUN_THREAD_HEARTBEAT();
		if(s_nCLientTotal > MAX_SUPPORTED_CLIENTS)
		{
			TRACE("continue: So Much Clients:s_nCLientTotal =%d\n",s_nCLientTotal);
			Sleep(1000);
			continue;
		}

		//Step1. Wait client connecting
		int hDataComm = pComm->Accept(hCommPort);
		if(hDataComm != NULL)
		{
			TRACE("Accept hCommPort Success!!!\n");
			if(Check_ValidClient(pReporter, hDataComm, &nClientAddr))
			{	
				if(s_nCLientTotal >= MAX_SUPPORTED_CLIENTS)
				{
					Sleep(100);
					continue;
				}
				s_Clients[s_nCLientTotal].hDataComm = hDataComm;
				s_Clients[s_nCLientTotal].pReporter = pReporter;
				s_Clients[s_nCLientTotal].bNeedExit = pReporter->bExit;
				s_Clients[s_nCLientTotal].nClientAddr = nClientAddr;
				s_Clients[s_nCLientTotal].nMapClientAddr = s_nCLientTotal;

				Create_ClientSession(&s_Clients[s_nCLientTotal]);
				
				s_nCLientTotal++;
				TRACE("Client Comm is Valid nClientAddr=%d nMapTotal=%d\n",nClientAddr, s_Clients[s_nCLientTotal].nMapClientAddr);
			}
			else
			{
				TRACE("Replace or InValid\n");
			}			
		}
		else
		{
			/*
			//get last err code 
			pComm->Control(hCommPort, COMM_GET_LAST_ERROR, (char *)&nErrCode, sizeof(int));
			if (nErrCode == ERR_COMM_TIMEOUT)
			 {
			 	TRACE("time=[%ld] Accept Timeout !!! \n", _NOW_);
			 }
			 else  // for other err, exit programme 
			 {
			 	TRACE("Accept failed nErrCode =%d\n", nErrCode);
			 }
			*/
		}
		Sleep(100);
	}


	for(n = 0; n < s_nCLientTotal; n++)
	{
		s_Clients[n].bNeedExit = TRUE;
	}

	TRACE("_ThreadEntry_Inter MasterComm: Exit\n");

	return THREAD_CANCEL_ALL;
}

#define  INIT_MST_MSG_INFO(_pMsg,_emMsgType,_SMsgNo,_RMsgNo, _pfnSHand,_pfnRHand,_bHasSubCmd) \
					do{\
					((_pMsg)->emMsgType = (_emMsgType), \
					(_pMsg)->bySCmdNo = (_SMsgNo), \
					(_pMsg)->byRCmdNo = (_RMsgNo), \
					(_pMsg)->pfnSHandler = (_pfnSHand), \
					(_pMsg)->pfnRHandler = (_pfnRHand), \
					(_pMsg)->bHasSubCmd = (_bHasSubCmd), \
					(_pMsg)++);\
					}while(0)

static BOOL Init_Master_MsgInfo(INTER_CLIENT *pClient)
{
	//Now init the msg info
	MST_MSG_FRM_INFO *pp = &(pClient->msgFrameInfo[0]);
	INIT_MST_MSG_INFO(pp, _M_C2S_CFGNTF_MSG,	0x02,	0x01,	PrepReplyCfgNtfCmd02,		ExcuteCfgNtfCmd01,		FALSE);
	INIT_MST_MSG_INFO(pp, _M_C2S_SYSINFO_MSG,	0x04,	0x03,	PrepReplySysInfoCmd04,		ExcuteSysInfoCmd03,		FALSE);
 	INIT_MST_MSG_INFO(pp, _M_C2S_BMS_MSG,		0x06,	0x05,	PrepReplyBmsCmd06,			ExcuteBmsCmd05,			FALSE);
	INIT_MST_MSG_INFO(pp, _M_C2S_ENGFLOW_MSG,	0x08,	0x07,	PrepReplyEngFlowCmd08,		ExcuteEngFlowCmd07,		FALSE);
	INIT_MST_MSG_INFO(pp, _M_C2S_SIGRECT_MSG,	0x0A,	0x09,	PrepReplySigRectCmd0A,		ExcuteSigRectCmd09,		FALSE);
	INIT_MST_MSG_INFO(pp, _M_C2S_BATTCAB_MSG,	0x0C,	0x0B,	PrepReplyBattCabCmd0C,		ExcuteBattCabCmd0B,		FALSE);
	INIT_MST_MSG_INFO(pp, _M_C2S_STATISDATA_MSG,0x0E,	0x0D,	PrepReplyStatisDataCmd0E,	ExcuteStaticDataCmd0D,	FALSE);


	INIT_MST_MSG_INFO(pp, _M_S2C_SETCFG_MSG,	0xA1,	0xA2,	PrepareSetCfgCmdA1,		ExtractSetCfgCmdA2,			TRUE);
	INIT_MST_MSG_INFO(pp, _M_S2C_CTRL_MSG,		0xA3,	0xA4,	PrepareCtrlCmdA3,		ExtractCtrlCmdA4,			TRUE);
	INIT_MST_MSG_INFO(pp, _M_S2C_APPSIZE_MSG,	0xA5,	0xA6,	PrepareAppSizeCmdA5,	ExtractAppSizeCmdA6,		FALSE);
	INIT_MST_MSG_INFO(pp, _M_S2C_APPUPD_MSG,	0xA7,	0xA8,	PrepareAppUpdCmdA7,		ExtractAppUpdCmdA8,			FALSE);

	return TRUE;
}

DLLExport BOOL Initialize(HANDLE pReporterThis)
{
	REPORTER* pReporter = (REPORTER*)pReporterThis;
	
	//Init Stu Parameter
	int n = 0;
	for(n = 0; n < MAX_SUPPORTED_CLIENTS; n++)
	{
		s_Clients[n].hMutex = Mutex_Create(TRUE);
		//s_Clients[n].hqMsg2Sent = Queue_Create(MAX_MST_EVENT_Q_SIZE, sizeof(MASTER_SEND_EVENT), 0);
		s_Clients[n].pReporter = pReporter;
		if((s_Clients[n].hMutex == NULL) /*|| (s_Clients[n].hqMsg2Sent == NULL)*/)
		{
			TRACE("ERROR : Fail to Create Mutex and Queue \n");
			return FALSE;
		}

		s_Clients[n].emStatus = CLIENT_STATU_NOT_INSTALLED;
		s_Clients[n].nCommFailTimes = 0;
		s_Clients[n].nClientAddr = 0;
		s_Clients[n].nMapClientAddr = 0;
		s_Clients[n].nSyncTimes = 0;
		s_Clients[n].bHasCardAuth = FALSE;
		s_Clients[n].tagMstUpgradeParam.emUpgradeStatus = UPDOWN_IDLE;
 		Init_Master_MsgInfo(&s_Clients[n]);
	}

	//Open Comm Port
	int nErrCode = 0;
	char szURL[64]={'\0'};
	//IP Addr And Port for Server
	snprintf(szURL, 64, "192.168.100.100:6003");
	pReporter->hCommPort = pReporter->pUsedCommPort->Open(
		pReporter->info.szReporterName,
		szURL, COMM_SERVER_MODE, 100, &nErrCode);
	if(ERR_COMM_OK != nErrCode)
	{
		LogOut(MAIN_MODULE_SAMPLER, "Inter_Master", LOG_TYPE_ERROR, "Open Port[%s] Failed", szURL);
		return FALSE;
	}
	TRACE("Inter_Master: szURL=%s \n", szURL);
	pReporter->bExit = FALSE;
	pReporter->hThread = RunThread_Create("Inter_MasterComm",
		(RUN_THREAD_START_PROC)_ThreadEntry_ClientMgmt,
		(void *)(pReporter),
		(DWORD*)&pReporter->bExit);
		
	if(pReporter->hThread == NULL)
	{
		TRACEX("ERROR : Create Inter MasterComm thread Failed \n");
		return FALSE;
	}

	//Init ev handler
	EVHAND_TAGS tags[] =
	{
		{ EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN },
		{EVENT_TYPE_USER_AUTHORIZE,		EVENT_MSG_END}, //from CR or OCPP
		{ EVENT_TYPE_FIRMWARE_UPDATE,	EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
		{ EVENT_TYPE_SYSTEM_EXIT,		EVENT_MSG_END}, 
	};

	for (n = 0; n < ITEM_OF(tags); n++)
	{
		pReporter->pDAI->Regsiter_EvHandler("Master_CommEventHand",
			pReporterThis,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}
 
	return TRUE;
}



