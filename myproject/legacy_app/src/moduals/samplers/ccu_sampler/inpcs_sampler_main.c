// #define _TRACE_PRINT_ // PSC
#include "inpcs_sampler_main.h"
// #define TRACE  TRACED
// #define TRACEX TRACED

BOOL Initialize_SCADA_INPCS(HANDLE hSamplerSelf)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
	ROUGH_CCU_DATA* pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;
	pCCU->emsMgmtData.nNodeNum = 1;

	if(!CCU_Check_And_ReOpen_CommPort(pSampler))
	{
		TRACEF("Initialize_SCADA_INPCS CCU_Check_And_ReOpen_CommPort failed <%s:%s>", pSampler->pUsedCommPort->info.szPortAttri, pSampler->info.szSamplerSetting);
		return FALSE;
	}
	if (pCCU->emSystemType == SYSTYPE_SCADA_INPCS235KW)
	{
		return Initialize_IN235kwPCS_StateM(hSamplerSelf);
	}
	return Initialize_INPCS_StateM(hSamplerSelf);
}

int Sample_SCADA_INPCS(HANDLE hSamplerSelf)
{
	// TRACEW(">>> Sample_SCADA_INPCS");
	// printf(">>>>>>>>>>>>>>>>>>>>>>> Sample_SCADA_INPCS >>>>>>>>>>>>>>>>>>>>>>\n");
	// time_t now;
    // time(&now);
    // struct tm *local = localtime(&now);
    // char timeString[100];
    // strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", local);
    // printf("SCADA_INPCS 当前时间: %s\n", timeString);

	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
	ROUGH_CCU_DATA* pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;
	
	BYTE byData[5] = {'\0'};
	// LJH241101 MU3
#ifdef IMMU3
	{
		// unit test PCS 自发自收，测试串口
		// DoTCPSession(pCCU, NL_MSG_CFGG, byData, 0, 500);
		// return TRUE;
		
		// unit test PCS 调试
		// pCCU->bCommFail = FALSE;
		// pCCU->bIsReceivedCFGG_FromLCCU = TRUE;
		// pCCU->tmLastRecvedData = _NOW_;
	}
	if(pCCU->bCommFail)
	{
		if(pCCU->emCCUState != CCU_STATE_LOCKED)
		{
			pCCU->emCCUState = CCU_STATE_FAULTED;
			pCCU->emSystemFailReason = SYSFAIL_REASON_CCU_COMFAIL;
		}
	}
	//---Step1. Shoule Get LCCU Config ,then Process Data Communication---//
	if (!pCCU->bIsReceivedCFGG_FromLCCU)
	{
		// TRACED("ccu_sampler_main_232:CCU_Sample_RS232 nCommFailTimes:%d", pCCU->nCommFailTimes);
		DoTCPSession(pCCU, NL_MSG_CFGG, byData, 0, 500);
		pCCU->nCommFailTimes++;
		if(pCCU->nCommFailTimes >= 20)
		{
			pCCU->nCommFailTimes = 20;
			pCCU->bCommFail = TRUE;
			CCU_Close_CommPort(pSampler);
			// TRACEI("ccu_sampler_main_232:CCU_Sample_RS232 nCommFailTimes return FALSE");
			goto stateM;
		}
		if(!DoTCPSession(pCCU, NL_MSG_CFGG, byData, 0, 500))
		{
			TRACEW("ccu_sampler_main_232:CCU_Sample_RS232 !DoTCPSession return FALSE");
			goto stateM;
		}
		pCCU->nCommFailTimes = 0;
		pCCU->bCommFail = FALSE;
		pCCU->emCommStatus = CCU_COMMSTAT_NORMAL;
	}
	//TRACE("nCommFailTimes =%d emCommStatus=%d\n", pCCU->nCommFailTimes, pCCU->emCommStatus);
	//Judge if comm fail
	time_t tmNow = _NOW_;
	static int s_nCommFaileResetCounter = 0;
	if (ElapseS(pCCU->tmLastRecvedData) > LSYS_BROADCAST_PERIOD && pCCU->bIsReceivedCFGG_FromLCCU == FALSE)
	{
		pCCU->nCommFailTimes++;
		TRACED("ccu_sampler_main_232:CCU_Sample_RS232 We have got CCU fail-times[%d]", pCCU->nCommFailTimes);
		if(pCCU->nCommFailTimes >= 8) //1.2 * 8 ~= 10s for commFail judge
		{
			TRACED("ccu_sampler_main_232:CCU_Sample_RS232");
			pCCU->emCommStatus = CCU_COMMSTAT_FAILED;

			if(!pCCU->bCommFail)
			{
				LogOut(MAIN_MODULE_SAMPLER, "CCU232Samp", LOG_TYPE_ERROR, "CCU communication FAILED, please check comm-port cable!!!");	
				InitCCUStuffData(pCCU);
			}		
			pCCU->nCommFailTimes = 8;
			pCCU->bCommFail = TRUE;
			//pCCU->bNeedUpdate_41BootUpMsg = TRUE;
			//added by Jimmy 20210126: if already comm with CCU, we should reset it in case CCU commfail in 3min

			//----(removed)Jimmy added 20201219: may need reopen the comm-port every 5min
			static time_t s_tmLastReopenComm = 0;
			if (ElapseS(s_tmLastReopenComm) >= 30) //30*4s timeout for reset when no CCU comm
			{
				//if(pCCU->bIsReceivedCFGG_FromLCCU)
				{
					s_tmLastReopenComm = _NOW_;
					BOOL bReOpenOK = CCU_Check_And_ReOpen_CommPort(pSampler);
					LogOut(MAIN_MODULE_SAMPLER, "CCU232Samp", LOG_TYPE_ERROR, "Reopen the CCU-RS232 port => %s",
						bReOpenOK ? "SUCCESS" : "FAILED");
					s_nCommFaileResetCounter++;
					if(s_nCommFaileResetCounter > 4)
					{
						//_SYSTEM("reboot &"); //need reboot system
						LogOut(MAIN_MODULE_SAMPLER, "CCU232Samp", LOG_TYPE_ERROR, 
							"!!!Now we will RESET the system because of NO response from CCU-RS232!!!");
						//gracefully reboot system
						EVENT_SYSTEM_EXIT evExit;
						evExit.nExitType = SYS_INTERNAL_ERROR;
						SendMsg(pCCU->hSampler, EVENT_TYPE_SYSTEM_EXIT, EVENT_MSG_BEGIN, 
							&evExit, sizeof(EVENT_SYSTEM_EXIT));
					}				
				}
			}
			goto stateM;
		}
		
		if(pCCU->nCommFailTimes >= 3)
		{
			LogOut(MAIN_MODULE_SAMPLER, "CCU232Samp", LOG_TYPE_WARING, "CCU communication failed times[%d], please check SW!", pCCU->nCommFailTimes);
		}
	}
	else
	{
		s_nCommFaileResetCounter = 0;
		pCCU->nCommFailTimes = 0;
		pCCU->emCommStatus = CCU_COMMSTAT_NORMAL;
	}
	if(pCCU->emCommStatus != CCU_COMMSTAT_NORMAL)
	{
		TRACEF("ERR: emCommStatus=%d Not Normal, this should Not happened", pCCU->emCommStatus);
		goto stateM;
	}
	//Trying Recv LCCU Data LSYS & RTIF
	int nErr = Mutex_Lock(pCCU->hSamplerLock, 100);
	if(nErr == ERR_MUTEX_OK)
	{
		// TRACEX("Now trying to receive CCU_Wait_ACCU_Reply_RS232");
		CCU_Wait_ACCU_Reply_RS232(pCCU, NULL, 0, NULL, 0);
		Mutex_Unlock(pCCU->hSamplerLock);
	}
	
	//John-2021-08-26 ,Step Up Send USYS To CCU,Ensure CCU recv Battery Data ASAP
	if(ElapseS(pCCU->strLSYSInfo.tmLast_Sent_USYS) >= 1)
	{
		if(Mutex_Lock(pCCU->hSamplerLock, 100) == ERR_MUTEX_OK)
		{
			Send_USYS(pCCU);
			pCCU->strLSYSInfo.tmLast_Sent_USYS = tmNow;
			Mutex_Unlock(pCCU->hSamplerLock);
		}
	}

	/*if(Mutex_Lock(pCCU->hSamplerLock, 100) == ERR_MUTEX_OK)
	{
		//Judge_IfNeedDisConnectACLoadMCB(pCCU);
		Judge_IfNeedQueryPCS_Settings(pCCU);
		Mutex_Unlock(pCCU->hSamplerLock);	
	}*/
#else
	if(pCCU->hDataComm == NULL)
	{
		pCCU->hDataComm = pSampler->pUsedCommPort->Accept(pSampler->hCommPort);
		if(pCCU->hDataComm != NULL)
		{
			pCCU->emCommStatus = CCU_COMMSTAT_ACCEPTED;
			pCCU->nCommFailTimes = 0;
			TRACE("pCCU->hDataComm != NULL\n");
		}
		else
		{
			TRACEX("---pCCU->hDataComm is NULL, Should Continue Wait Accept---\n");
 			Sleep(500);
			
			pCCU->nCommFailTimes++;
			if(pCCU->nCommFailTimes >= 20)
			{
				TRACEX("-----------Accept is NULL, CCU is comm fail---------\n");
				pCCU->nCommFailTimes = 20;
				pCCU->bCommFail = TRUE;
			}

			goto stateM;
		}
	}

	//---Step1. Shoule Get LCCU Config ,then Process Data Communication---//
	if(pCCU->emCommStatus == CCU_COMMSTAT_ACCEPTED)
	{
		if(!DoTCPSession(pCCU, NL_MSG_CFGG, byData, 0, 500))
		{
			pCCU->nCommFailTimes++;
			if(pCCU->nCommFailTimes >= 20)
			{
				TRACED("-----------CFGG not Responded, CCU has comm fail---------\n");
				pCCU->nCommFailTimes = 20;
				pCCU->bCommFail = TRUE;
				if(pCCU->hDataComm != NULL)
				{
					TRACE("---Close hDataComm---\n");
					//pSampler->pUsedCommPort->Close(pCCU->hDataComm);
					CCU_TCP_Close_Client(pCCU);
				}
				pCCU->emCommStatus = CCU_COMMSTAT_FAILED;
			}
			goto stateM;
		}
		else
		{
			pCCU->nCommFailTimes = 0;
			pCCU->bCommFail = FALSE;
			pCCU->emCommStatus = CCU_COMMSTAT_NORMAL;
			TRACE("Info:Here Get CFGG ok!!!\n");
		}
	}

	//TRACE("nCommFailTimes =%d emCommStatus=%d\n", pCCU->nCommFailTimes, pCCU->emCommStatus);

	//Judge if comm fail
	time_t tmNow = _NOW_;
	if(tmNow - pCCU->strLSYSInfo.tmLastRecv_LSYS >= LSYS_BROADCAST_PERIOD)
	{
		pCCU->nCommFailTimes++;
		if(pCCU->nCommFailTimes >= 30) //30s
		{
			TRACEX("-----------CCU has comm fail[%ld, %d, %d]---------\n", 
				tmNow, pCCU->strLSYSInfo.tmLastRecv_LSYS,
				pCCU->nCommFailTimes);
			pCCU->nCommFailTimes = 30;
		 	
			if(pCCU->hDataComm != NULL)
			{
				InitCCUStuffData(pCCU);
				pSampler->pUsedCommPort->Close(pCCU->hDataComm);
				pCCU->hDataComm = NULL;
			}
			pCCU->emCommStatus = CCU_COMMSTAT_FAILED;
			goto stateM;
		}
	}
	else
	{
		pCCU->nCommFailTimes = 0;
		pCCU->emCommStatus = CCU_COMMSTAT_NORMAL;
		//TRACE("nCommFailTimes=%d, emCommStatus=%d\n",pCCU->nCommFailTimes,pCCU->emCommStatus);
	}
	if(pCCU->emCommStatus != CCU_COMMSTAT_NORMAL)
	{
		TRACEF("ERR: emCommStatus=%d Not Normal, this should Not happened", pCCU->emCommStatus);
		goto stateM;
	}
	//Trying Recv LCCU Data LSYS & RTIF
	int nErr = Mutex_Lock(pCCU->hSamplerLock, 100);
	if(nErr == ERR_MUTEX_OK)
	{
		//TRACEX("Now trying to receive LSYS_LCHG for Client\n");
		Recv_Tcp_CCU_Msg(pCCU);
		Mutex_Unlock(pCCU->hSamplerLock);
	}
	
	//John-2021-08-26 ,Step Up Send USYS To CCU,Ensure CCU recv Battery Data ASAP
	//if(ABS(tmNow - pCCU->strLSYSInfo.tmLast_Sent_USYS) >= 1.0)
	{
		if(Mutex_Lock(pCCU->hSamplerLock, 100) == ERR_MUTEX_OK)
		{
			Send_USYS(pCCU);
			pCCU->strLSYSInfo.tmLast_Sent_USYS = tmNow;
			Mutex_Unlock(pCCU->hSamplerLock);
		}
	}

	/*if(Mutex_Lock(pCCU->hSamplerLock, 100) == ERR_MUTEX_OK)
	{
		//Judge_IfNeedDisConnectACLoadMCB(pCCU);
		Judge_IfNeedQueryPCS_Settings(pCCU);
		Mutex_Unlock(pCCU->hSamplerLock);	
	}*/
#endif

	//if(pCCU->emCommStatus == CCU_COMMSTAT_NORMAL)
	{
		// OnHandle_STS_OnOffStatu(pCCU);

		// if(Mutex_Lock(pCCU->hSamplerLock, 100) == ERR_MUTEX_OK)
		// {
	 	// 	On_HandleHMI_On2OffGridEvent(pCCU);
		// 	Mutex_Unlock(pCCU->hSamplerLock);
		// }

		// OnHandle_BatCabFireProtAlm(pCCU);

		// OnHandle_PowerCabLighting(pCCU);

		// OnHandle_TrigCANMsg(pCCU);

		//Dealing with PCS relay and Batt Grp Input Contactor Linkage Errors
		// Onhandle_GrpInputContactor1_CtrlAndExcuteStatus(pCCU);
		// Onhandle_GrpInputContactor2_CtrlAndExcuteStatus(pCCU);
		// Onhandle_GrpInputContactor3_CtrlAndExcuteStatus(pCCU);
		// Onhandle_GrpInputContactor4_CtrlAndExcuteStatus(pCCU);
		// Onhandle_PCS_RelayAlarm(pCCU);

		//Dealing with MPPT and PV Contactor Errors
		// Onhandle_PVContactor1_CtrlAndExcuteStatus(pCCU);
		// Onhandle_PVContactor2_CtrlAndExcuteStatus(pCCU);
		// Onhandle_PVContactor3_CtrlAndExcuteStatus(pCCU);
		// Onhandle_PVContactor4_CtrlAndExcuteStatus(pCCU);
		// Onhandle_MPPT_SCAlarm(pCCU);

		if(pCCU->bNeedSyncTime2IMSU == TRUE || (_NOW_ - pCCU->tmLastSyncTime2IMSU) == 5)
		{
			// printf("\n---------IMSU-D:%d Sync Time to CCU --------\n");
			// BYTE byData[5];
 			UINT nTimeSecond = (UINT)_NOW_;
			byData[0] = 0x02;
			byData[1] = (BYTE)(nTimeSecond >> 24);
			byData[2] = (BYTE)(nTimeSecond >> 16);
			byData[3] = (BYTE)(nTimeSecond >> 8);
			byData[4] = (BYTE)nTimeSecond;
			DoTCPSession(pCCU, NL_MSG_CFGS, byData, 5, 1000);
			pCCU->bNeedSyncTime2IMSU = FALSE;
			pCCU->tmLastSyncTime2IMSU = _NOW_;
		}

		//Enter Upgrade Mode
		if(pCCU->tagCCUUpgradeParam.tagMstUpgradeParam.emUpgradeStatus != UPGRADE_IDLE)
		{
			//if(fabs(tmNow - pCCU->tagCCUUpgradeParam[pClient->nReal_CCUAddr].tmLastEnterUpgrade >= 0.3)) //Consider IMSU_D process Speed
			{
				//pCCU->tagCCUUpgradeParam[pClient->nReal_CCUAddr].tmLastEnterUpgrade = tmNow;
				HandleIMSUUpgrading(pCCU);
			}
		}
	}
	// else
	// {
	// 	TRACEF("ERR: emCommStatus=%d Not Normal, this should Not happened", pCCU->emCommStatus);
	// 	goto stateM;
	// }
	
	//TRACE("Now Running Here ,Do some Cycle Data Process!\n");
stateM:

	//TRACE("Info:---Running Here stateM---\n");
	Refresh_SysAvailState(pCCU);
	
	//TRACE("emCCUState=%d, emSystemFailReason=%d \n", pCCU->emCCUState, pCCU->emSystemFailReason );
 
	// Update_SysDataInfo(pCCU);
	// unit test 不需要EMS,后面删除
	// Ems_Mgmt_Main(pCCU);
	// pCCU->emErrCode_MgmtCfg = Check_MgmtPlanCfg(pCCU);
	
// LJH241101 MU3
#ifdef IMMU2
	//now Manage the LED status
	MangeLEDStatus(pCCU);
#endif

 	// CalcMPPTGenerateKWH(pCCU);
	
	// CalcDCLoadCostKWH(pCCU);
 
	PushAllData(pCCU);
 
	Prepare_ESRunData(pCCU);
 
	// Manage_ESStoreData(pCCU);
 
	//Manage_ECOMode(pCCU);

	if (pCCU->emSystemType == SYSTYPE_SCADA_INPCS235KW)
	{
		return Sample_IN235kwPCS_StateM(hSamplerSelf);
	}
	else
	{
		// LJH241111 PcsSCADA
		return Sample_INPCS_StateM(hSamplerSelf);
	}
}
