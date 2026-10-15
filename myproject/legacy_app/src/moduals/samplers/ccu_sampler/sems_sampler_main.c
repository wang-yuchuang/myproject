/********************************************************************************
* Copyright(c) Shenzhen Infypower Co., Ltd.
* @author
* @brief
* @history
*******************************************************************************/

// #define _TRACE_PRINT_ // SEMS
#include "sems_sampler_main.h"

SemsInfo s_SemsInfo;
A_BLOB_SEMS_CLIENTINFO* s_MMClients;
int			s_nMMClientsTotal = 0;
COMMPORT * 	s_pUDPComm = NULL;

static BOOL ReOpen_QuicklyCommPort_UDP_SERVER(void)
{
	int nErrCode = 0;
	COMM_UDP_SET  sUdpClientInfo;

    sUdpClientInfo.nNonBlock = 1;	//非阻塞
	sUdpClientInfo.sPeerInfo.nPeerType = 0;
    sUdpClientInfo.sPeerInfo.nPeerPort = 10005;
    // strncpy(sUdpClientInfo.sPeerInfo.szAddr, inet_ntoa(*(struct in_addr *)s_MMClients[nInComingCCU_Idx].stClientInfo.szAddr), sizeof(sUdpClientInfo.sPeerInfo.szAddr));//一对一  
    strncpy(sUdpClientInfo.sPeerInfo.szAddr, "0.0.0.0", sizeof(sUdpClientInfo.sPeerInfo.szAddr));//一对一  

	s_SemsInfo.ClientsUdpInfo.hClientUDPPort = s_pUDPComm->Open(
		(char*)&sUdpClientInfo,
		"127.0.0.1:9989",
		COMM_SERVER_MODE, 500, &nErrCode);
	// printf(" hClientUDPPort = [%x]\n",s_SemsInfo.ClientsUdpInfo.hClientUDPPort);
	if(ERR_COMM_OK != nErrCode)
	{
		s_SemsInfo.ClientsUdpInfo.bSocketCreatFail = TRUE;
		// printf("creat udp socket failed!!!\n");
		return FALSE;
	}
	// printf("Rece a TCP client[%s] Then open a UDP socket---peerInfo[%s:%d]\n",
	// 	inet_ntoa(*(struct in_addr *)s_MMClients[0].stClientInfo.szAddr),
	// 	sUdpClientInfo.sPeerInfo.szAddr,
	// 	sUdpClientInfo.sPeerInfo.nPeerPort);
	return TRUE;
}

static BOOL Check_And_ReOpen_CommPort_SEMS(SAMPLER* pSampler)
{
	int nErrCode = 0;
	strncpy(pSampler->pUsedCommPort->info.szPortAttri, "sems_tcp_port", sizeof(pSampler->pUsedCommPort->info.szPortAttri));
	strncpy(pSampler->info.szSamplerSetting, "127.0.0.1:9988", sizeof(pSampler->info.szSamplerSetting));
	pSampler->hCommPort = pSampler->pUsedCommPort->Open(
		pSampler->pUsedCommPort->info.szPortAttri,
		pSampler->info.szSamplerSetting,
		COMM_SERVER_MODE, 500, &nErrCode);
	if(ERR_COMM_OK != nErrCode)
	{
		return FALSE;
	}

	if(pSampler->hCommPort != NULL)
	{
		int nMaxClient = MAX_SEMS_CLIENTS_NUM;
		pSampler->pUsedCommPort->Control(pSampler->hCommPort, 
			COMM_SET_MAX_CLIENTS, &nMaxClient, sizeof(int));
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

static void* _Thread_MMClientComm(void* pArgs)
{
	A_BLOB_SEMS_CLIENTINFO *pClient = (A_BLOB_SEMS_CLIENTINFO *)pArgs;
	ROUGH_CCU_DATA* pCCU = pClient->pCCU;
	SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
	TRACEI("_Thread_MMClientComm %d pClient:%X ip:%s(port:%d)",
			pClient->idxClientEms, pClient, inet_ntoa(*(struct in_addr *)pClient->stClientInfo.szAddr), pClient->stClientInfo.nPeerPort);

	int nClientIdx = pClient->idxClientEms;
	// int n = 0;
	int nErr = 0;
	pClient->tmLastRcvHeatbeat = _NOW_;

	while(!pClient->bNeedExitThread)
	{
		TRACE(">>> _Thread_MMClientComm--------");
		nClientIdx = pClient->idxClientEms;
		if (pClient->bCommFailSemsClient)
		{
			if(pClient->emRunStatus == MMCLITNT_COMM_STAT_NORMAL)
			{
				if(pClient->hClientPort != NULL)
				{
					TRACEE("EMS Client[%d %s:%d] Comm Failed tmLastRcvLSYS >= %d Close Port",
						nClientIdx, inet_ntoa(*(struct in_addr *)pClient->stClientInfo.szAddr),
						pClient->stClientInfo.nPeerPort, MAX_S_COMMFAIL_SEMS);
					pSampler->pUsedCommPort->Close(pClient->hClientPort);
					pClient->hClientPort = NULL;
				}
				pClient->bClientFirstConn = FALSE;
				pClient->emRunStatus = MMCLITNT_COMM_STAT_FAILED;
				pClient->bNeedRegister = TRUE;
			}

			Sleep_Ex(200);
			continue;
		}
		else
		{
			pClient->emRunStatus = MMCLITNT_COMM_STAT_NORMAL;
		}
		//Wait Register
		if(pClient->bNeedRegister)
		{
			if(Send_61CFGG(nClientIdx) && Send_6204CFGS_LSYSPeriod(nClientIdx))
			{
				TRACEI("_Thread_MMClientComm have got 61CFGG and 6204CFGS_LSYSPeriod bNeedRegister = FALSE");
				pClient->bNeedRegister = FALSE;
			}
			else
			{
				TRACEW("_Thread_MMClientComm have not got 61CFGG wait");
				Sleep_Ex(100);
				continue;
			}
		}
		//now read and parse CCU broadcast frame
		nErr = Mutex_Lock(pClient->hLock, 50);
		if(nErr == ERR_MUTEX_OK)
		{
			Wait_ACCU_TCPReply(nClientIdx, NULL, 0);
			Mutex_Unlock(pClient->hLock);
		}
		else
		{
			TRACEE("_Thread_MMClientComm Mutex_Lock err:%d Wait_ACCU_TCPReply", nErr);
		}
		//check if need send USYS
		if (ElapseMS(pClient->tmLastSentUSYS) >= 1000.0f)
		{
			Send_A6USYS(nClientIdx);
			pClient->tmLastSentUSYS = GetCurrentTime();
		}
		// LJH251106 同步主从参数
		if (ElapseS(pClient->tmSyncCFGG) > 30*60)
		{
			Send_61CFGG(nClientIdx);
		}
		HasParamSetCFGS* pHasParamSetCFGp = &pCCU->semsSMInfos[nClientIdx].hasParamSetCFGS;
		BOOL bSync = FALSE;
		if (pHasParamSetCFGp->bStopSOC_ChrgSet || pHasParamSetCFGp->bStopSOC_DiscSet)
		{
			TRACEI("_Thread_MMClientComm hasParamSetCFGS:%d Send_6205CFGS_StopSoc", pCCU->semsSMInfos[nClientIdx].hasParamSetCFGS.data);
			bSync = TRUE;
			if (Send_6205CFGS_StopSoc(nClientIdx))
			{
				pHasParamSetCFGp->bStopSOC_ChrgSet = 0;
				pHasParamSetCFGp->bStopSOC_DiscSet = 0;
			}
		}
		if (pHasParamSetCFGp->bSendCmdByLC1stOnlySet && !bSync)
		{
			TRACEI("_Thread_MMClientComm hasParamSetCFGS:%d Send_6206CFGS_SendCmdByLC1stOnly", pCCU->semsSMInfos[nClientIdx].hasParamSetCFGS.data);
			if (Send_6206CFGS_SendCmdByLC1stOnly(nClientIdx))
			{
				pHasParamSetCFGp->bSendCmdByLC1stOnlySet = 0;
			}
		}
		//added by Jimmy 20210401 for CCU-TCP firmware updt
		// if(pCCU->bNeed_EnterCCUUpdt[nClientIdx])
		// {
		// 	Mutex_Lock(pClient->hLock, WAIT_INFINITE);
		// 	pCCU->bNeed_EnterCCUUpdt[nClientIdx] = FALSE;
		// 	// DoTCP_CCU_Upgrade(nClientIdx);
		// 	Mutex_Unlock(pClient->hLock);

		// 	pCCU->emCCUState = CCU_STATE_IDLE;
		// 	for(n = 0; n < MAX_DISP_GUN_NUM; n++)
		// 	{
		// 		UINT nLogicGunIdx = 0xFF;
		// 		nLogicGunIdx = GunPort_PhyMap2Logic(pCCU->pGunMapTable, nClientIdx*2 + n);
		// 		if(nLogicGunIdx == 0xFF) 
		// 		{
		// 			continue;
		// 		}
		// 		if(pCCU->gunData[nLogicGunIdx].emGunChargingState != GUN_STATE_NOT_INSTALL)
		// 		{
		// 			pCCU->gunData[nLogicGunIdx].emGunChargingState = GUN_STATE_IDLE;
		// 		}
		// 	}
		// 	RefreshGun_AndSysAvailState(pCCU);
		// }

		//judge if need sync RTC 移到LSYS校时，减少延时
		// nErr = Mutex_Lock(pClient->hLock, 50);
		// if(nErr == ERR_MUTEX_OK)
		// {
		// 	Mutex_Unlock(pClient->hLock);
		// 	if (ElapseMSBcdTimeStmp(&pCCU->strLSYSInfo.branchNodeData[nClientIdx].b6LSYS_SEMS.tmStamp) >= 1000)
		// 	{
		// 		Send_6202CFGS_RTC(nClientIdx);
		// 	}
		// }
		// else
		// {
		// 	TRACEE("_Thread_MMClientComm Mutex_Lock err:%d RTC", nErr);
		// }
		
		Sleep_Ex(100);
		TRACE("_Thread_MMClientComm >>>");
	}

	return NULL;
}

// 信号只来源于485电表Sampler
void* _thread_SEMS_Process_SemFrom485(void* pArgs)
{
	SAMPLER* pSampler = (SAMPLER *)pArgs;
	ROUGH_CCU_DATA* pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;
	// DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	// EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;

	// ERRCDDE_DPR err;
	// VAR_VALUE varV_Cab;
	// VAR_VALUE varV_Grid;
	int n = 0;

	if (pCCU->nSemsSemCCUWith485Meter == -1)
	{
		pthread_exit(NULL);
		LogOut(MAIN_MODULE_REPORTER, "_thread_SEMS_Process_SemFrom485", LOG_TYPE_ERROR, 
			"SEMID FAILED!!!");
	}

	while (!pSampler->bExit)
	{
		sem_wait_op(pCCU->nSemsSemCCUWith485Meter);   //block wait

		TRACEI("_thread_SEMS_Process_SemFrom485 it is SEMS time[%0.3f]~~\n",GetCurrentTime());
		// LogOut(MAIN_MODULE_MAIN, "QuickChannel", LOG_TYPE_WARING,
		// 	"it is SEMS Get SemFrom485 !");

		pCCU->bQuickChannle = TRUE;
		Ems_Mgmt_Main_SEMS(pCCU);
		// printf("_thread_SEMS_Process_SemFrom485 pEMS->emBatt_WorkModeReq_2 = [%d]\n",pEMS->emBatt_WorkModeReq);
		for (n=0; n < pCCU->emsMgmtData.nNodeNum; n++)
		{
			if ((s_MMClients[n].bCommFailSemsClient) || (s_SemsInfo.ClientsUdpInfo.bSocketCreatFail))
			{
				TRACEE("Client bCommFailSemsClient or bSocketCreatFail!!!!\n");
				continue;
			}
			//UDP快速响应通道；
			Send_6330UctlWorkPwrAuto_UDP(pCCU, STATU_ACDC_INVERTER, n, MAX_INTV_MS_CONTINUE_WORK);
		}

	}
}
BOOL Initialize_EMS_STATION(HANDLE hSamplerSelf)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
	ROUGH_CCU_DATA* pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;
	EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
	int portId = 11;	//udp

	ERRCDDE_DPR err;
	VAR_VALUE varV;
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1, 43, &varV, 4);
	pEMS->nNodeNum = (int)((err==ERR_DPR_OK) ? varV.ulValue : 0);
	pEMS->nNodeNum = MIN(pEMS->nNodeNum, MAX_SEMS_CLIENTS_NUM);
	pEMS->nNodeNum = MAX(pEMS->nNodeNum, 1);

	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 407, 401, &varV, 4);
	pEMS->bCabMeter_SEMS = ((err==ERR_DPR_OK) ? (varV.emValue>0?TRUE:FALSE) : FALSE);

	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 408, 401, &varV, 4);
	pEMS->bGMeter_SEMS = ((err==ERR_DPR_OK) ? (varV.emValue>0?TRUE:FALSE) : FALSE);

	char szFirstIP_EMSClient[MAX_LEN_SZIP] = "";
	varV.blobValue = (BYTE *)szFirstIP_EMSClient;
	pCCU->pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 1, 44, &varV, sizeof(szFirstIP_EMSClient));
	s_SemsInfo.ulIP_FirstClient = ntohl(inet_addr(szFirstIP_EMSClient));

	TRACEI("Initialize_EMS_STATION nNodeNum=[%d] bAutoChrgEnable:%d %d "
		"fTargetPwr_4Chrg_SEMS:%.1f fTargetPwr_4Dischrg_SEMS:%.1f Offset:%.1f %.1f "
		"enableREMSCtrl:%d enableAntiOvld_REMS:%d enableAntiRflx_REMS:%d bCabMeter_SEMS:%d bGMeter_SEMS:%d "
		"szFirstIP_EMSClient:%s %04X",
		pEMS->nNodeNum, pEMS->bAutoChrgEnable, pEMS->bAutoDischrgEnable,
		pEMS->fTargetPwr_4Chrg_SEMS, pEMS->fTargetPwr_4Dischrg_SEMS, pEMS->fTargetPwr_4ChrgOffset_SEMS, pEMS->fTargetPwr_4DischrgOffset_SEMS,
		pEMS->enableREMSCtrl, pEMS->enableAntiOvld_REMS, pEMS->enableAntiRflx_REMS, pEMS->bCabMeter_SEMS,  pEMS->bGMeter_SEMS,
		szFirstIP_EMSClient, s_SemsInfo.ulIP_FirstClient);

	//Now init the ydn msg info
	NLMSG_FRAME_INFO_SEMS *pp = &s_SemsInfo.nlFrameInfoSEMS[0];
	INIT_NL_MSGINFO(pp, NL_MSG_CFGG,	Parse_CFGG_SEMS,		0x61,	40);
	INIT_NL_MSGINFO(pp, NL_MSG_CFGS,	Parse_CFGS_SEMS,		0x62,	2);
	INIT_NL_MSGINFO(pp, NL_MSG_UCTL,	Parse_UCTL_SEMS,		0x63,	2);
	// INIT_NL_MSGINFO(pp, NL_MSG_RTIF,	Parse_RTIF_SEMS,		0x64,	18);
	INIT_NL_MSGINFO(pp, NL_MSG_PCSINFO,	Parse_PCS_INFO,			0x65,	2);
	INIT_NL_MSGINFO(pp, NL_MSG_USYS,	NULL,					0xA6,	0);
	INIT_NL_MSGINFO(pp, NL_MSG_LSYS,	Parse_LSYS_SEMS,		0xB6,	15);
	INIT_NL_MSGINFO(pp, NL_MSG_LDEV,	Parse_LDEV_SEMS,		0xC6,	4);
	
	s_MMClients = s_SemsInfo.MMClients;
	ZERO_POBJS(s_MMClients, MAX_SEMS_CLIENTS_NUM);
	int n;
    for(n = 0; n < MAX_SEMS_CLIENTS_NUM; n++)
	{
		s_MMClients[n].pCCU = pCCU;
		s_MMClients[n].tmLastRcvHeatbeat = _NOW_ + MAX_S_COMMFAIL_SEMS; // for first time
		s_MMClients[n].idxClientEms = 0xFF;
		s_MMClients[n].emRunStatus = MMCLITNT_COMM_STAT_FAILED;

		s_MMClients[n].bClientFirstConn = TRUE;
		s_MMClients[n].bNeedRegister = TRUE;
	}

	if (!Check_And_ReOpen_CommPort_SEMS(pSampler))
	{
		TRACEE("Initialize_EMS_STATION Check_And_ReOpen_CommPort_SEMS failed <%s:%s>",
			pSampler->pUsedCommPort->info.szPortAttri, pSampler->info.szSamplerSetting);
		return FALSE;
	}
	Initialize_SEMS_StateM(hSamplerSelf);

	s_pUDPComm = NEW(COMMPORT,1);
    if (s_pUDPComm == NULL) 
	{
        TRACEE("s_pUDPComm Memory allocation failed\n");
        return FALSE;
    }
	if (GetCommPort_ById((HANDLE)pSampler, portId, s_pUDPComm) == NULL)
	{
		TRACEE("Initialize_EMS_STATION:GetCommPort_ById:%d failed", portId);
		return FALSE;
	}
	TRACED("Initialize_EMS_STATION--szPortName=[%s]-szPortAttri=[%s]-szPortDriver=[%s]-!!! \n",\
		s_pUDPComm->info.szPortName,\
		s_pUDPComm->info.szPortAttri,\
		s_pUDPComm->info.szPortDriver);

	//connect to client by UDP to quickly response	
	if (!ReOpen_QuicklyCommPort_UDP_SERVER())
	{
		TRACEE("Sample_EMS_STATION ReOpen_QuicklyCommPort_UDP failed Idx");
	}
	//init ccu and 485_meter sampler sem
	pCCU->nSemsSemCCUWith485Meter = Creat_SemID(SEM_CCU_485_PATH, SEM_CCU_485_PROJ_ID, 1, IPC_CREAT | 0666);
	TRACED("SMESccuSampler Init semid = [%d]\n",pCCU->nSemsSemCCUWith485Meter);
	LogOut(MAIN_MODULE_MAIN, "SEMS_CCU_TCP_SAMP", LOG_TYPE_INFO,
		"SEMS_CCU_TCP_SAMP semid Init --> [%d] !\n",pCCU->nSemsSemCCUWith485Meter);
	if (pCCU->nSemsSemCCUWith485Meter != -1)
	{
		Init_Sem(pCCU->nSemsSemCCUWith485Meter, 0, SETVAL, 0);
	}
	
	//----- Create A Thread To Process the sem-----//
	struct sched_param param;
	pthread_attr_t attr;
	pthread_t hThread;

	pthread_attr_init(&attr);
	param.sched_priority = 98;	//数值越大，优先级越高。
	pthread_attr_setschedpolicy(&attr,SCHED_RR);//实时调度策略
	pthread_attr_setschedparam(&attr,&param);//设置优先级
	pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);//要使优先级其作用必须要有这句话

	if(pthread_create(&hThread, &attr, _thread_SEMS_Process_SemFrom485,
		(void *)pSampler) != 0) //0-Success
	{
		TRACE("----- Create _thread_SEMS_Process_SemFrom485 Failed -----\n");
		LogOut(MAIN_MODULE_REPORTER, "_thread_SEMS_Process_SemFrom485", LOG_TYPE_ERROR, 
			"Init _thread_SEMS_Process_SemFrom485 FAILED!!!");
		pthread_attr_destroy(&attr);
		return FALSE;  
	}
	
	TRACED("Initialize_EMS_STATION<%s:%s>", pSampler->pUsedCommPort->info.szPortAttri, pSampler->info.szSamplerSetting);
	return TRUE;
}

int Sample_EMS_STATION(HANDLE hSamplerThis)
{
	TRACEW(">>> Sample_EMS_STATION START >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");

	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	ROUGH_CCU_DATA* pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;
	int n;

	// char szFoundCCUBin[64] = {0};
	// if(FindMatchFile_FromDir(CCU_FIRMWARE_UPGRADE_PATH, "IMSU_CCU", szFoundCCUBin) && s_bFirstTimeBoot)
	// {
	// 	char szCmd[64];
	// 	snprintf(szCmd, 64, "rm -rf %s*", CCU_FIRMWARE_UPGRADE_PATH);
	// 	_SYSTEM(szCmd);
	// }
	// s_bFirstTimeBoot = FALSE;

	//now handle coming client connecting...
	HANDLE hClient = pSampler->pUsedCommPort->Accept(pSampler->hCommPort);
	if(hClient != NULL)
	{
		COMM_PEER_INFO stClientInfo;
		pSampler->pUsedCommPort->Control(hClient, COMM_GET_PEER_INFO, &stClientInfo, sizeof(COMM_PEER_INFO));
		TRACEI("Sample_EMS_STATION Accept Client[%s, Port:%d] nInstalledCCUsNum[%d]", 
			inet_ntoa(*(struct in_addr *)stClientInfo.szAddr), stClientInfo.nPeerPort, 
			pCCU->emsMgmtData.nNodeNum);
		ULONG ulComingIP = ntohl(*(ULONG *)stClientInfo.szAddr); // "192.168.1.101":C0A80165
		// static char szArrCCU_IP_Addr[MAX_SEMS_CLIENTS_NUM][MAX_LEN_SZIP] = {
		// 	"192.168.1.101",
		// 	"192.168.1.102",
		// 	"192.168.1.103",
		// 	"192.168.1.104",
		// 	"192.168.1.105",
		// };
		int nInComingCCU_Idx = 0xFF;
		for (n = 0; n < pCCU->emsMgmtData.nNodeNum; n++)
		{
			//if(ulComingIP == ntohl((ULONG)inet_addr(szArrCCU_IP_Addr[n])))
			if (ulComingIP == (s_SemsInfo.ulIP_FirstClient + (ULONG)n))
			{
				nInComingCCU_Idx = n;
				// TRACEI("Find CCU-Client [%04X, %s] Success nInComingCCU_Idx:%d", ulComingIP, szArrCCU_IP_Addr[n], nInComingCCU_Idx);
				TRACEI("Find CCU-Client [%04X] Success nInComingCCU_Idx:%d", ulComingIP, nInComingCCU_Idx);
				break;
			}
			else
			{
				// TRACEE("Find CCU-Client [%04X, %s] Failed", ulComingIP, szArrCCU_IP_Addr[n]);
				TRACEE("Find CCU-Client [%04X, %d] Failed", ulComingIP, n);
			}
		}
		if(nInComingCCU_Idx != 0xFF)  //Find CCU
		{
			A_BLOB_SEMS_CLIENTINFO *pClient = &s_MMClients[nInComingCCU_Idx];
			if(pClient->bClientFirstConn)
			{
				ZERO_POBJS(pClient, 1);
				pClient->pCCU = pCCU;
				pClient->tmLastRcvHeatbeat = _NOW_;
				pClient->idxClientEms = nInComingCCU_Idx;

				pClient->emRunStatus = MMCLITNT_COMM_STAT_NORMAL;
				pClient->hClientPort = hClient;
				pClient->hLock = Mutex_Create(TRUE);
				pClient->stClientInfo = stClientInfo;

				if (pthread_create((pthread_t*)&pClient->hThread,
						NULL, _Thread_MMClientComm, (void *)pClient) == 0)
				{
					s_nMMClientsTotal++;

					TRACEX("Create CCU-Client [%d, %d] done...\n", 
						pClient->idxClientEms, s_nMMClientsTotal);

					LogOut(MAIN_MODULE_REPORTER, "CCU_Samp", LOG_TYPE_INFO, 
						"Create CCU-Client Addr[%d] TotalClients[%d]!", 
						pClient->idxClientEms, s_nMMClientsTotal);
				}
				else
				{
					TRACEE("Create CCU-Client Thread Failed <%d>", nInComingCCU_Idx);
					pSampler->pUsedCommPort->Close(hClient);
					hClient = NULL;
				}
				// //connect to client by UDP to quickly response	
				// if (!ReOpen_QuicklyCommPort_UDP_SERVER(nInComingCCU_Idx))
				// {
				// 	TRACEE("Sample_EMS_STATION ReOpen_QuicklyCommPort_UDP failed Idx[%d]",nInComingCCU_Idx);
				// }
			}
			else
			{
				if(pClient->hClientPort != NULL)
				{
					TRACEW("Sample_EMS_STATION Found a repeated Client Addr[%d]! close old sock", nInComingCCU_Idx);
					pSampler->pUsedCommPort->Close(pClient->hClientPort);
					pClient->hClientPort = NULL;
				}
				pClient->tmLastRcvHeatbeat = _NOW_;

				pClient->emRunStatus = MMCLITNT_COMM_STAT_NORMAL;
				pClient->hClientPort = hClient;
				
				pClient->bCommFailSemsClient = FALSE;
				pClient->bNeedRegister = TRUE;
				LogOut(MAIN_MODULE_REPORTER, "CCU_Samp", LOG_TYPE_INFO, 
					"Found a repeated Client Addr[%d]!", nInComingCCU_Idx);

				TRACEX("Found a repeated Client Addr[%d]!\n", nInComingCCU_Idx);
			}
		}
		else
		{
			TRACEE("Sample_EMS_STATION Rcv Tcp Conn, But Not Found a Vaild Client!\n");
			//Not Find CCU, Closed
			pSampler->pUsedCommPort->Close(hClient);
			hClient = NULL;
		}
	}

	Refresh_SysAvailState(pCCU);
	// A_BLOB_SINGLE_STBATT_RUNINFO
	Update_SysDataInfo(pCCU);
	pCCU->bQuickChannle = FALSE;
	Ems_Mgmt_Main_SEMS(pCCU);
	pCCU->emErrCode_MgmtCfg = Check_MgmtPlanCfg(pCCU);
 	CalcMPPTGenerateKWH(pCCU);
	CalcDCLoadCostKWH(pCCU);
	PushAllData(pCCU);
	Prepare_ESRunData(pCCU);
	Manage_ESStoreData(pCCU);
	TRACEW(">>> Sample_EMS_STATION END>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	return Sample_SEMS_StateM(hSamplerThis);
}
