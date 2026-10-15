// #define _TRACE_PRINT_
#include "ccu_sampler_main.h"

void CCU_TCP_Close_Client(ROUGH_CCU_DATA* pCCU)
{
	TRACEE("ccu_sampler_main:CCU_TCP_CloseClient");
	SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
	if (pCCU->hDataComm != NULL)
	{
		pSampler->pUsedCommPort->Close(pCCU->hDataComm);
		pCCU->hDataComm = NULL;
	}
}

// static BOOL ReOpen_TCPPort(ROUGH_CCU_DATA* pCCU)
// {
// 	SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
// 	//should close the client sock first
// 	CCU_TCP_Close_Client(pCCU);
// 	CCU_Close_CommPort(pSampler);

//     //open
// 	int nErrCode = 0;
// 	pSampler->hCommPort = pSampler->pUsedCommPort->Open(
// 		pSampler->pUsedCommPort->info.szPortAttri,
// 		pSampler->info.szSamplerSetting,
// 		COMM_SERVER_MODE, 100, &nErrCode);
// 	if(pSampler->hCommPort != NULL)
// 	{
// 		TRACEX("---------now Re-Open the serverTCPPort OOOOOOOOOOOK--------------\n");
// 		//max support only one client
// 		int nMaxClient = 1;
// 		pSampler->pUsedCommPort->Control(pSampler->hCommPort, 
// 			COMM_SET_MAX_CLIENTS, &nMaxClient, sizeof(int));
// 	}
// 	else
// 	{
// 		TRACEX("---------now Re-Open the serverTCPPort FFFFFFFFFFFFFFailed--------------\n");
// 	}
	
//     return  (pSampler->hCommPort == NULL) ? FALSE : TRUE;
// }

BOOL CCU_Check_And_ReOpen_CommPort(SAMPLER* pSampler, BOOL bModuleUpdate)
{
	UNUSED(bModuleUpdate);

	//Now open the port
	int nErrCode = 0;
	pSampler->hCommPort = pSampler->pUsedCommPort->Open(
		pSampler->pUsedCommPort->info.szPortAttri,
		pSampler->info.szSamplerSetting,
		COMM_SERVER_MODE, 100, &nErrCode);
	if((ERR_COMM_OK != nErrCode) || (pSampler->hCommPort == NULL))
	{
		pSampler->hCommPort = NULL;
		TRACEE("CCU_Check_And_ReOpen_CommPort_eth:Check_And_ReOpen_CommPort<%s> failed", pSampler->pUsedCommPort->info.szPortAttri);
		return FALSE;
	}
	return TRUE;

	// //changed TCP address
	// // if(s_stCCUTCP_RunRough.bIsChanged_WorkdMode/* ||
	// // 	(s_stCCUTCP_RunRough.emClient_RegisteredStatus == CCU_REGISTERED_ADDR_CONFLICT)*/)
	// // {
	// // 	TRACED("----Now re-assign the network settings---------\n");
	// // 	if(!ReAssign_NetworkSettings(FALSE))
	// // 	{
	// // 		TRACED("----Re-assign the network settings FAILED---------\n");
	// // 		Sleep(5000);
	// // 		return FALSE;
	// // 	}
	// // 	ReOpen_TCPPort(pSampler); //just close the port so that to be reopen later
	// // 	s_stCCUTCP_RunRough.bIsChanged_WorkdMode = FALSE;
	// // }
	
    // if(pSampler->hCommPort == NULL)
    // {
	// 	TRACED("----Now re-Open the serverport---------\n");
    //     if(!ReOpen_TCPPort(pSampler))
    //     {
	// 		TRACED("----Re-Open the serverport FAILED---------\n");
	// 		// s_stCCUTCP_RunRough.bIsChanged_WorkdMode = TRUE;
    //         return FALSE;
    //     }
    // }
    // //try receive handle if no client
    // if(pCCU->hDataComm == NULL)
    // {
	// 	//printf("----Now begin accpet the client[%.3f]---------\n", GetCurrentTime());
	// 	int nAcceptTImeout = 2000; //wait max 2*10 = 12s for re-connection
	// 	do
	// 	{
	// 		pCCU->hDataComm = pSampler->pUsedCommPort->Accept(pSampler->hCommPort);
	// 		nAcceptTImeout -= 100; //TCP_CLIENT_READ_MIN_INTERV_MSEC;
	// 		if(pCCU->hDataComm != NULL)
	// 		{
	// 			break;
	// 		}
	// 		Sleep_Ex(100);
	// 	} while (nAcceptTImeout > 0);

	// 	// printf("----Now end accpet the client[%.3f] => %s---------\n", GetCurrentTime(),
	// 	// 	(pCCU->hDataComm != NULL) ? "SUCCCCCCC" : "FAILLLL");
		
    //     if(pCCU->hDataComm != NULL)
	// 	{
	// 		//Jimmy note 20231113: 不再区分IP
	// 		//if(IsClientIP_Matched())
	// 		//{
	// 			// s_stCCUTCP_RunRough.tmLastReceivedLSYS = 0;
	// 			// s_stCCUTCP_RunRough.emClient_RegisteredStatus = CCU_REGISTERED_NONE;
	// 			// s_stCCUTCP_RunRough.pCCU->nCommFailTimes = 0;
	// 			// s_stCCUTCP_RunRough.bDetected_ClientDisconnected = FALSE;
	// 		//}
	// 		// else
	// 		// {
	// 		// 	TRACEX("----Refused the comingIP because of NOT matched!!!!----\n");
	// 		// 	Close_ClientConnection();
	// 		// }			
	// 	}
	// 	else
	// 	{
	// 		int nLastErrorCode = -9999;
	// 		pSampler->pUsedCommPort->Control(pSampler->hCommPort, 
	// 				COMM_GET_LAST_ERROR, &nLastErrorCode, sizeof(int));
	// 		if(nLastErrorCode != -9999)
	// 		{
	// 			TRACEF("ccu_tcp_handler::CheckIf_TCPClient_Connected Accept ERROR => %d---", nLastErrorCode);
	// 		}			
	// 	}
    // }

    // return  (pCCU->hDataComm == NULL) ? FALSE : TRUE;    
}

// void CCU_Handle_CommFail(ROUGH_CCU_DATA* pCCU, BOOL bCommFailed)
// {
// 	TRACEX("---We have got CCU fail-times[%d]---\n", pCCU->nCommFailTimes);
//     static BOOL s_bIsCommFail = FALSE;
//     static int s_nCommFaileResetCounter = 0;
//     if(!bCommFailed)
// 	{
// 		s_bIsCommFail = FALSE;
// 		//pCCU->nCommFailTimes = 0; //can NOT set = 0 here！！！！！！！
// 		s_nCommFaileResetCounter = 0;
// 		return;
// 	}
// 	else 
// 	{
// 		pCCU->nCommFailTimes++;
// 		TRACEX("=====the CCU comm-fail times=>%d=====\n", pCCU->nCommFailTimes);
// 	}

// 	//added by Jimmy 20230529 to avoid wrong CCU-commFail
// 	if(s_stCCUTCP_RunRough.bDetected_ClientDisconnected)
// 	{
// 		Close_ClientConnection(); //do not need close the server connection
// 		pCCU->nCommFailTimes += 3;
// 	}

// 	//added by Jimmy 20211221 to solve the first time boot CCU-com-fail problem
// 	int MAX_COMMFAILTIMES_4CCU = pCCU->stSysStatus.bAlreadyGot5042_FromCCU ? 10 : 15;
// 	if((pCCU->nCommFailTimes >= /*10*/MAX_COMMFAILTIMES_4CCU)/* || 
// 		s_stCCUTCP_RunRough.bDetected_ClientDisconnected*/)
// 	{
// 		TRACEX("-----------CCU has comm fail => %d---------\n", s_stCCUTCP_RunRough.bDetected_ClientDisconnected);
// 		//ReOpen_TCPPort(TRUE); //close the sesssion so that it can be re-opened
// 		Close_ClientConnection(); //do not need close the server connection

// 		if(!s_bIsCommFail)
// 		{
// 			//Jimmy: do not need re-assign it but only close the client-connection
// 			//s_stCCUTCP_RunRough.bIsChanged_WorkdMode = TRUE; //need re-assign the network so that to avoid IP wrong changed
// 			s_bIsCommFail = TRUE;
// 			LogOut(MAIN_MODULE_SAMPLER, "CCU_TCPSamp", LOG_TYPE_ERROR, "CCU TCP comm FAILED => %d, please check comm-port cable!!!",
// 				s_stCCUTCP_RunRough.bDetected_ClientDisconnected);	
// 		}	
			
// 		pCCU->nCommFailTimes = MAX_COMMFAILTIMES_4CCU;
// 		pCCU->bCommFail = TRUE;
// 		pCCU->bNeedUpdate_41BootUpMsg = TRUE;
// 		//added by Jimmy 20210126: if already comm with CCU, we should reset it in case CCU commfail in 3min

// 		//added by wangdong 20231221: if comm is fail, we need reset Manual Ctrl status 
// 		pCCU->stSysStatus.bIsManualCtrlMode = 0;
		
// 		//----(removed)Jimmy added 20201219: may need reopen the comm-port every 5min
// 		static time_t s_tmLastReopenComm = 0;
// 		time_t tmNow = _NOW_;		
// 		//if(ABS(tmNow - s_tmLastReopenComm) >= 300)
// 		if(ABS(tmNow - s_tmLastReopenComm) >= 30) //30*3s timeout for reset when no CCU comm
// 		{
// 			if(pCCU->stSysStatus.bAlreadyGot5042_FromCCU)
// 			{
// 				TRACEX("------------Now close the ALLLLLLLLLport because of TCP comFAIL---------\n");
// 				s_stCCUTCP_RunRough.bIsChanged_WorkdMode = TRUE; //close the server so that it can be re-opened
// 				s_tmLastReopenComm = tmNow;
// 				BOOL bReOpenOK = CheckIf_TCPClient_Connected();
// 				LogOut(MAIN_MODULE_SAMPLER, "CCU_TCPSamp", LOG_TYPE_ERROR, "Reopen the CCU-TCP port or re-accept the CCU-TCP client => %s!!!",
// 					bReOpenOK ? "SUCCESS" : "FAILED");	
// 				s_nCommFaileResetCounter++;
// 				if(s_nCommFaileResetCounter > 3)
// 				{
// 					TRACEE("ccu_tcp_handler::Handle_CCUCommFail reboot!");
// 					//_SYSTEM("reboot &"); //need reboot system
// 					LogOut(MAIN_MODULE_SAMPLER, "CCU_TCPSamp", LOG_TYPE_ERROR, 
// 						"!!!Now we will RESET the system because of NO response from CCU!!!");
// 					//gracefully reboot system
// 					EVENT_SYSTEM_EXIT evExit;
// 					evExit.nExitType = SYS_INTERNAL_ERROR;
// 					SendMsg(pCCU->hSampler, EVENT_TYPE_SYSTEM_EXIT, EVENT_MSG_BEGIN, 
// 						&evExit, sizeof(EVENT_SYSTEM_EXIT));
// 				}				
// 			}
// 		}	
// 	}
// 	else if(pCCU->nCommFailTimes >= 6)
// 	{
// 		if(pCCU->bCommFail)
// 		{
// 			return;
// 		}
// 		LogOut(MAIN_MODULE_SAMPLER, "CCU_TCPSamp", LOG_TYPE_WARING, "CCU TCP comm failed times[%d], please check SW!", pCCU->nCommFailTimes);
// 	}
// }

BOOL DoTCPSession(ROUGH_CCU_DATA* pCCU, NL_MSGTYPE emMsgTp, BYTE* pData, int nDataLen, int nTimeout_ms)
{
	int 		i = 0, n = 0;
 	BYTE 		byBuff[TCP_BUFF_LEN];
	SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;

	int nSendLen = MakeNLMSG_SendFrame(pCCU->nlFrameInfo[emMsgTp].byCmd, (USHORT)nDataLen, pData, byBuff);

#if 0
	char szLogTxt[256];
	memset(szLogTxt, 0, sizeof(szLogTxt));
	for (i = 0; i < nDataLen && (i * 3) < 250; ++i)
	{
		sprintf(&szLogTxt[i*3], "%02X ", pData[i]);
	}
	
	char szTitle[32] = {0};
	sprintf(szTitle, "MU2 to CCU NL_MSGTYPE: %d", emMsgTp);
	//LogOut(MAIN_MODULE_SAMPLER, szTitle, LOG_TYPE_INFO,  szLogTxt);
	LOG_INFO("%s %s\n", szTitle, szLogTxt);
#endif
 
	//Sleep_Ex(50); //if send to frequently data can get lost
	//Mutex_Lock(pCCU->hSamplerLock, WAIT_INFINITE);
	if(CCU_WriteData(pCCU, byBuff, nSendLen) != nSendLen)
	{
 		//Mutex_Unlock(pCCU->hSamplerLock);
		TRACE("Error:write Failed, != nSendLen %d\n", nSendLen);
		return FALSE;
	}
	//printf("\n_________Now Succeffully write msg to CCU len[%d]_________\n",nSendLen);
	
	
	//for(n = 0; n < nSendLen; n++)
	//{
	//	printf("%02X-", byBuff[n]);
	//}
	//printf("\n__________End of print_________\n");
	

	//Now wait for the answer
	//nTimeout_ms = 2000; //for test purpose
	BOOL bRTN = FALSE;
	BOOL bFoundReply = FALSE;
	while ((nTimeout_ms >= 0) && (!bFoundReply))
	{
		memset(byBuff, 0, TCP_BUFF_LEN);
		// int nReadBytes = CCU_ReadData(pCCU, byBuff, sizeof(byBuff));
		int nReadBytes = 0;
		TRACE("\n------DoTCPSession----Read back reply[%d, %d] from CCU[%d]--------\n", \
				nReadBytes, nTimeout_ms);
		
		//for(i =0;i < nReadBytes ;i++)
		//{
		//	printf("%02X",byBuff[i]);
		//}
		//printf("\n");
		

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
					break;
				}
				TRACEI("DoTCPSession_eth Find_NLMsgFrame byCmd:%02X msgTp:%d", pFrm->byCmd, pFrm->msgTp);
#ifdef _TRACE_PRINT_
				switch (pFrm->byCmd)
				{
					case 0x01: // CFGG
					TRACED("DoTCPSession_eth cmd:%02X CFGG byBuff<%s>", pFrm->byCmd, printableBytes(pFoundPos, usDataLen+NLMSG_MIN_LEN));
						break;
					case 0x02: // CFGS
					TRACED("DoTCPSession_eth cmd:%02X CFGS byBuff<%s>", pFrm->byCmd, printableBytes(pFoundPos, usDataLen+NLMSG_MIN_LEN));
						break;
					case 0x03: // UCTL
					TRACEI("DoTCPSession_eth cmd:%02X UCTL sub:%02X byBuff<%s>", pFrm->byCmd, *pDataPos, printableBytes(byBuff, nReadBytes));
						break;
					case 0x04: // RTIF
					TRACED("DoTCPSession_eth cmd:%02X RTIF byBuff<%s>", pFrm->byCmd, printableBytes(pFoundPos, usDataLen+NLMSG_MIN_LEN));
						break;
					case 0xB0: // LSYS
					TRACED("DoTCPSession_eth cmd:%02X LSYS byBuff<%s>", pFrm->byCmd, printableBytes(byBuff, nReadBytes));
						break;
					case 0x23: // PCSG
					TRACED("DoTCPSession_eth cmd:%02X PCSG byBuff<%s>", pFrm->byCmd, printableBytes(byBuff, nReadBytes));
						break;
					case 0x24: // PCSS
					TRACED("DoTCPSession_eth cmd:%02X PCSS byBuff<%s>", pFrm->byCmd, printableBytes(byBuff, nReadBytes));
						break;

					default:
					TRACED("DoTCPSession_eth cmd:%02X byBuff<%s>", pFrm->byCmd, printableBytes(byBuff, nReadBytes));
						break;
				}
#endif
				if(pFrm->byCmd == pCCU->nlFrameInfo[emMsgTp].byCmd)
				{
					bFoundReply = TRUE;
					if((emMsgTp == NL_MSG_CFGS) || (emMsgTp == NL_MSG_UCTL))
					{
						TRACEX("------Now get CFGS/UTRL reply for CCU, subCmd[%d, %d, [%d,%d]]---\n",
							pFrm->byCmd, usDataLen, (BYTE)*pDataPos, pData[0]);
						if((usDataLen == 2) && (*pDataPos == pData[0])) //subCmd
						{
  							bRTN = (pFrm->pfnParser(pCCU, pDataPos, usDataLen) == 0) ? TRUE : FALSE;
						}
					}
					else
					{
 						bRTN = (pFrm->pfnParser(pCCU, pDataPos, usDataLen) == 0) ? TRUE : FALSE;
					}
				}
				else
				{
					//TRACE("pFrm->byCmd =%02X bRTN=%d\n",pFrm->byCmd, bRTN);
					pCCU->nCommFailTimes = 0;
					pFrm->pfnParser(pCCU, pDataPos, usDataLen);
				}
				
				nReadBytes -= (usDataLen + NLMSG_MIN_LEN);
				pFoundPos += (usDataLen + NLMSG_MIN_LEN);
			} while (nReadBytes >= NLMSG_MIN_LEN);
		}

		Sleep_Ex(50);
		nTimeout_ms -= 50;
	}

	//Mutex_Unlock(pCCU->hSamplerLock);

	return bRTN;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Recv_Tcp_CCU_Msg
*  CREATOR       :  
*  CREATED DATE  : 2020-9-19 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Recv_Tcp_CCU_Msg(ROUGH_CCU_DATA* pCCU)
{
	SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;

	BYTE byBuff[TCP_BUFF_LEN];
	memset(byBuff, 0, TCP_BUFF_LEN);
	int nReadBytes = CCU_ReadData(pCCU, byBuff, sizeof(byBuff));
	/*if(nReadBytes > 0)
	{
		
		TRACE("------LSYSLCHG: Read back reply nReadBytes[%d]\n", nReadBytes);
		int i;		
		for(i =0;i < nReadBytes ;i++)
		{
			printf("%02X-",byBuff[i]);
		}
		printf("\n");
		
	}*/
	
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
				TRACE("---Get Frm Is NULL---\n");
				break;
			}
			else if((pFrm->byCmd == pCCU->nlFrameInfo[NL_MSG_LSYS].byCmd) ||
					(pFrm->byCmd == pCCU->nlFrameInfo[NL_MSG_RTIF].byCmd) ||
					(pFrm->byCmd == pCCU->nlFrameInfo[NL_MSG_LDEV].byCmd) ||
					(pFrm->byCmd == pCCU->nlFrameInfo[NL_MSG_GCAN].byCmd))
			{
				//TRACE("Do TCP Parsing byCmd=[%02X]\n",pFrm->byCmd);
 				//DoTCPFrmParsing(pCCU, pFrm, pDataPos, usDataLen);
				pFrm->pfnParser(pCCU, pDataPos, usDataLen);

			}
			else
			{
				TRACE("Info: Maybe Other Msg byCmd=[%02X]\n", pFrm->byCmd);
			}
			
			
			nReadBytes -= usDataLen + NLMSG_MIN_LEN;
			pFoundPos += usDataLen + NLMSG_MIN_LEN;
		} while (nReadBytes >= NLMSG_MIN_LEN);
	}

	return;
}
