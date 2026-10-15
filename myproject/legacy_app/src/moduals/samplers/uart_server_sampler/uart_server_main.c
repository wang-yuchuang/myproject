//#define _TRACE_PRINT_
#include "uart_server_main.h"

static UART_SERVER_TCP_RUNROUGH* s_pUartServerTCPRough;

static int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE	emEvType,
	_EVENT_MSG	emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int			nMaxBlockMSec)
{
	SAMPLER* pSampler = (SAMPLER*)hModuleSeif;
	ST_UART_SERVER_ROUGHDATA *pUart = (ST_UART_SERVER_ROUGHDATA*)pSampler->pDataHeap;

	TRACEI("uart_Event_Handler--got EVENT[%d] emMsgType=[%d]--------\n", emEvType, emMsgType);
	 
	if(emEvType == EVENT_TYPE_KWHMETER_CURRENT_POS_NEG_SPFW) //restore resere info
	{
		if((nEvDataSize != sizeof(EVENT_METER_SAVE_DATA)) || (emMsgType != EVENT_MSG_RESTORE))
		{
			TRACEE("uart---nEvDataSize=%d EVENT_TYPE_KWHMETER_CURRENT_POS_NEG_SPFWSize=%d\n", nEvDataSize, sizeof(EVENT_METER_SAVE_DATA));
			return EVENT_HANDED_FAILED;
		}

		EVENT_METER_SAVE_DATA* pMeterKwhSave = (EVENT_METER_SAVE_DATA*)pEvData;
		if(pMeterKwhSave != NULL)
		{
			if (pMeterKwhSave->ndevid == 401)  //only 401 need save current SPFW kwh
			{
				pUart->meterData[0].nLastYearToSaveDB = pMeterKwhSave->nYear;   
				pUart->meterData[0].nLastMonthToSaveDB = pMeterKwhSave->nMonth;
				pUart->meterData[0].nLastDayToSaveDB = pMeterKwhSave->nDay;
				TRACEI("uart_Event_Handler-----received restore event DBLASTYear = [%d] DBLASTMonth = [%d] DBLASTDay = [%d] -----------\n",
					pUart->meterData[0].nLastYearToSaveDB,
					pUart->meterData[0].nLastMonthToSaveDB,
					pUart->meterData[0].nLastDayToSaveDB);
			}
		}
	}
	return EVENT_HANDED_OK;
}

static BOOL UART_SERVER_TCP_ReopenServerPort(UART_SERVER_TCP_RUNROUGH *pUartServerTCPRough)
{
    SAMPLER* pSampler = (SAMPLER*)pUartServerTCPRough->pUART->hSampler;
	int err = 0;

	if(NULL != pSampler->hCommPort)
	{
		pSampler->pUsedCommPort->Close(pSampler->hCommPort);
	}
    int nErrCode = 0;
	pSampler->hCommPort = pSampler->pUsedCommPort->Open(
		pSampler->pUsedCommPort->info.szPortAttri,
		pSampler->info.szSamplerSetting,
		COMM_SERVER_MODE, 100, &nErrCode);

	if(ERR_COMM_OK != nErrCode)
	{
		return FALSE;
	}
	TRACEI("open socket server [192.168.1.100:8899] success !\n");

	if(pSampler->hCommPort != NULL)
	{
		int nMaxClient = TCP_CLIENT_NUM_MAX;
		pSampler->pUsedCommPort->Control(pSampler->hCommPort, COMM_SET_MAX_CLIENTS, &nMaxClient, sizeof(int));
	}

	return TRUE;

}

static int UART_SERVER_TCP_CheckClientCommSuccNum(IN UART_SERVER_TCP_RUNROUGH *pUartServerTCPRough)
{
	int i = 0, iNum = 0;

	for(i = 0; i < TCP_CLIENT_NUM_MAX; i++)
	{
		if(CLITNT_COMM_STAT_NORMAL == pUartServerTCPRough->stTcpClientInfo[i].emRunCommStatus)
		{
			iNum++;
		}
	}

	return iNum;
}

static int UART_SERVER_TCP_CheckClientConnectIsExist(IN UART_SERVER_TCP_RUNROUGH* pUartServerTCPRough, IN HANDLE hClient, OUT int *iAddr)
{
	SAMPLER *pSampler = (SAMPLER *)pUartServerTCPRough->pUART->hSampler;

	// compare IP
	COMM_PEER_INFO stClientInfo;
	pSampler->pUsedCommPort->Control(hClient, 
		COMM_GET_PEER_INFO, &stClientInfo, sizeof(COMM_PEER_INFO));
	
	char *clientIP = inet_ntoa(*(struct in_addr *)stClientInfo.szAddr);

	TRACEW("----the Client[%s, %d] is coming now...------\n", clientIP, stClientInfo.nPeerPort);

	int i = 0, iIndex = 0;
	for(i = 0; i < TCP_CLIENT_NUM_MAX; i++)
	{
		if(0 == strcmp(clientIP, (char*)pUartServerTCPRough->szClientIP[i]))
		{
			iIndex = i;
			break;
		}
	}

	if(i >= TCP_CLIENT_NUM_MAX)
	{
		return -1;
	}

	*(int *)iAddr = (int)iIndex;

	if(FALSE == pUartServerTCPRough->stTcpClientInfo[iIndex].bIsRegistered)
	{
		return 1;
	}

	return 0;
}

static void* UART_SERVER_TCP_ClientCommThread(IN void* pArgs)
{
    UART_SERVER_TCP_CLIENT_INFO *pClientInfo = (UART_SERVER_TCP_CLIENT_INFO *)pArgs;
	UART_SERVER_TCP_RUNROUGH * pUartRunData = s_pUartServerTCPRough;
	ST_UART_SERVER_ROUGHDATA *pUart = pUartRunData->pUART;
	SAMPLER *pSampler = (SAMPLER *)pUart->hSampler;
	int errCode;

	// compare IP
	COMM_PEER_INFO stClientInfo;
	pSampler->pUsedCommPort->Control(pClientInfo->hClient, 
		COMM_GET_PEER_INFO, &stClientInfo, sizeof(COMM_PEER_INFO));

	char *clientIP = inet_ntoa(*(struct in_addr *)stClientInfo.szAddr);

	TRACEI("UART_SERVER_TCP_ClientCommThread szClientIP = %s ", clientIP);

	while(!pClientInfo->bNeedExitThread)
	{
		if (strcmp((const char *)clientIP, (const char *)pUartRunData->szClientIP[0]) == 0)  //modbus rtu TempHumidity 192.168.1.201
		{
			//串口#1添加应用场景参数选择
			TRACED("UART_SERVER_TCP_ClientCommThread szClientIP = %s ", clientIP);//edit by kuangyue 20251029
			TRACED("pUartRunData->bUartServerRunning_A = %d ", pUartRunData->bUartServerRunning_A);
			if(pUartRunData->bUartServerRunning_A == SERIALSERVER_TYPE_ALARMLIGHT)//告警灯场景
			{
				BOOL bAlarmStatus = GetFireAlarmStatus(pUart);
				if(bAlarmStatus == FALSE)
				{
					Sleep(500);
					continue;
				}
				AlarmLightMain(pUart, pClientInfo);
			}
			else if(pUartRunData->bUartServerRunning_A == SERIALSERVER_TYPE_TEMPHUMIL)//温控器场景
			{
				// read temperature and humidity
				errCode=ModbusSampMain(pUart, pClientInfo);
				if (errCode == 1)
				{
					Sleep(500);
					continue;
				}
				PushModbusValue(pUart);
			}
			else if(pUartRunData->bUartServerRunning_A == SERIALSERVER_TYPE_METER_DSZ71)//DSZ71电表场景
			{
				errCode= whMeter645_Sample(pUart,pClientInfo,METER_645_TYPE_DSZ71);
				if (errCode == 1)
				{
					Sleep(500);
					continue;
				}
				RS485_StuffChanl(pUart,METER_645_TYPE_DSZ71);    //load Data to Cfg
			}
			else if(pUartRunData->bUartServerRunning_A == SERIALSERVER_TYPE_METER_AMC96)//AMC96电表场景
			{
				errCode= whMeter645_Sample(pUart,pClientInfo,METER_645_TYPE_AMC96);
				if (errCode == 1)
				{
					Sleep(500);
					continue;
				}
				RS485_StuffChanl(pUart,METER_645_TYPE_AMC96);  
			}
		}
		else if (strcmp((const char *)clientIP, (const char *)pUartRunData->szClientIP[1]) == 0) //645 KWH_Meter  192.168.1.202
		{
			//串口#2添加应用场景参数选择
			if(pUartRunData->bUartServerRunning_B == SERIALSERVER_TYPE_METER_DSZ71)//DSZ71电表场景
			{
				errCode= whMeter645_Sample(pUart,pClientInfo,METER_645_TYPE_DSZ71);
				if (errCode == 1)
				{
					Sleep(500);
					continue;
				}
				RS485_StuffChanl(pUart,METER_645_TYPE_DSZ71);    //load Data to Cfg
			}

		}
		else if (strcmp((const char *)clientIP, (const char *)pUartRunData->szClientIP[2]) == 0) //645 MutilFuction Meter 192.168.1.203
		{
			//串口#3添加应用场景参数选择
			if(pUartRunData->bUartServerRunning_C == SERIALSERVER_TYPE_METER_AMC96)//AMC96电表场景
			{
				errCode= whMeter645_Sample(pUart,pClientInfo,METER_645_TYPE_AMC96);
				if (errCode == 1)
				{
					Sleep(500);
					continue;
				}
				RS485_StuffChanl(pUart,METER_645_TYPE_AMC96);  
			}
		}
		else if (strcmp((const char *)clientIP, (const char *)pUartRunData->szClientIP[3]) == 0)  //reserve   192.168.1.204
		{
			//串口#4添加应用场景参数选择
			if(pUartRunData->bUartServerRunning_D == SERIALSERVER_TYPE_ALARMLIGHT)//告警灯场景
			{
				BOOL bAlarmStatus = GetFireAlarmStatus(pUart);
				if(bAlarmStatus == FALSE)
				{
					Sleep(500);
					continue;
				}
				AlarmLightMain(pUart, pClientInfo);
			}
			//TRACED("======================192.168.1.204=========================\n");
		}

		Sleep(500);

	}


}

static void UART_SERVER_TCP_ClearClientCommInfo(IN UART_SERVER_TCP_CLIENT_INFO *pClientInfo)
{
	SAMPLER *pSampler = (SAMPLER *)s_pUartServerTCPRough->pUART->hSampler;

	if(NULL != pClientInfo->hClient)
	{
		pSampler->pUsedCommPort->Close(pClientInfo->hClient);
		pClientInfo->hClient = NULL;
	}
	pClientInfo->nCommFailTimes = 0;
	pClientInfo->emRunCommStatus = CLITNT_COMM_STAT_FAILED;

	TRACEW("Connect timeout or breaked, so close the client connect !\n");
}


static void UART_SERVER_TCP_CreateClientThread(IN HANDLE hClient, IN int nWhichClient)
{
	TRACEI("Enter UART_SERVER_TCP_CreateClientThread ! \n");
    UART_SERVER_TCP_RUNROUGH *pUartServerTCPRough = s_pUartServerTCPRough;
	UART_SERVER_TCP_CLIENT_INFO *pUartServerTCPClient = &pUartServerTCPRough->stTcpClientInfo[nWhichClient];

	pUartServerTCPClient->hClient = hClient;
	pUartServerTCPClient->bIsRegistered = TRUE;
	pUartServerTCPClient->emRunCommStatus = CLITNT_COMM_STAT_NORMAL;
	pUartServerTCPClient->nCommFailTimes = 0;
	pUartServerTCPClient->tmLastReceivedLSYS = GetCurrentTime();
	pUartServerTCPClient->nIndex = nWhichClient;

	if (pthread_create(&pUartServerTCPClient->hThread, \
			NULL, UART_SERVER_TCP_ClientCommThread, (void *)pUartServerTCPClient) == 0)
	{
		pUartServerTCPRough->nClientConnTotal++;

		TRACEW("Create CCU-Client [%d, %d] done...\n", nWhichClient, pUartServerTCPRough->nClientConnTotal);
		LogOut(MAIN_MODULE_REPORTER, "UART_SERVER_Samp", LOG_TYPE_INFO, "Create UART_SERVER-Client Addr[%d] TotalClients[%d]!", \
			nWhichClient, pUartServerTCPRough->nClientConnTotal);
	}
	else
	{
		pUartServerTCPClient->bIsRegistered = FALSE;
		TRACEW("Create CCU-Client [%d, %d] error...\n", nWhichClient, pUartServerTCPRough->nClientConnTotal);

		UART_SERVER_TCP_ClearClientCommInfo(pUartServerTCPClient);
	}

}


static void UART_SERVER_TCP_RestartClientThread(IN HANDLE hClient, IN int nWhichClient)
{
	UART_SERVER_TCP_RUNROUGH *pUartServerTCPRough = s_pUartServerTCPRough;
	UART_SERVER_TCP_CLIENT_INFO *pUartServerTCPClient = &pUartServerTCPRough->stTcpClientInfo[nWhichClient];
	ST_UART_SERVER_ROUGHDATA *pUart = (ST_UART_SERVER_ROUGHDATA *)pUartServerTCPRough->pUART;

	UART_SERVER_TCP_ClearClientCommInfo(pUartServerTCPClient);
			
	pUartServerTCPClient->hClient = hClient;
	pUartServerTCPClient->emRunCommStatus = CLITNT_COMM_STAT_NORMAL;
	pUartServerTCPClient->tmLastReceivedLSYS = GetCurrentTime();

	// TODO - Add lock
	// Mutex_Lock(pUartServerTCPClient->hMutexLock, WAIT_INFINITE);
	// //pUart->bNeedUpdateCFGGMsg[pUartServerTCPClient->nIndex] = TRUE;
	// Mutex_Unlock(pUartServerTCPClient->hMutexLock);

	TRACEW("Reconnect CCU-Client [%d, %d] done...\n", nWhichClient, pUartServerTCPRough->nClientConnTotal);
	LogOut(MAIN_MODULE_REPORTER, "UART_SERVER_Samp", LOG_TYPE_INFO, "Found a repeated Client Addr[%d]!", nWhichClient);
}

static int UART_SERVER_TCP_ServerAcceptComm(IN void *pArgs)
{
    UART_SERVER_TCP_RUNROUGH *pUartServerTCPRough = s_pUartServerTCPRough;
	SAMPLER* pSampler = (SAMPLER*)pUartServerTCPRough->pUART->hSampler;
	time_t tmNow = _NOW_;
	HANDLE hClient = NULL;
	int iRet = 0, iIndex = 0, iNum = 0;

	// reopen the server port
	iNum = UART_SERVER_TCP_CheckClientCommSuccNum(pUartServerTCPRough);
	tmNow = _NOW_;

	/*	Check the client list. If there is no client connection for 600 seconds, the port will be reset
		When the port is successfully reset or there is a client connection,
		tmLastCheckCliConnectOrReopenPort will be updated to avoid frequent port reset
	*/ 
	//TRACEI(" CLITNT_COMM_STAT_NORMAL = %d ",iNum);

	if((0 == iNum) && ((tmNow - pUartServerTCPRough->tmLastCheckCliConnectOrReopenPort) > 600))    //tmAcceptClient   Last Time active Port
	{
		if(FALSE == UART_SERVER_TCP_ReopenServerPort(pUartServerTCPRough))
		{
			Sleep_Ex(1000);
			LogOut(MAIN_MODULE_SAMPLER, "UART SERVER TCP Samp", LOG_TYPE_ERROR, 
				"UART_SERVER_TCP_ReopenServerPort Restart FAILED!!!");
			return -1;	   //restart server port err 			
		}
		pUartServerTCPRough->tmLastCheckCliConnectOrReopenPort = tmNow;
	}
	else if(iNum > 0)
	{
		pUartServerTCPRough->tmLastCheckCliConnectOrReopenPort = tmNow;
	}

	//now handle coming client connecting...
	hClient = pSampler->pUsedCommPort->Accept(pSampler->hCommPort);
	if(NULL == hClient)
	{
		//Sleep_Ex(200);   //200ms period
		return 0;    //no client to accept
	}

	iRet = UART_SERVER_TCP_CheckClientConnectIsExist(pUartServerTCPRough, hClient, &iIndex);

	if((iIndex < 0) || (iIndex >= TCP_CLIENT_NUM_MAX))
	{
		TRACEE("CheckClientConnectIsExist out iIndex is error! \n");
		return -1;
	}

	if(1 == iRet) // The client of this IP connects for the first time
	{
		TRACEI("new connect!!");
		UART_SERVER_TCP_CreateClientThread(hClient, iIndex);

	}
	else if(0 == iRet) // The client of this IP is reconnecting
	{
		TRACEW("repeat connect!!");
		UART_SERVER_TCP_RestartClientThread(hClient, iIndex);
	}
	else 
	{
		TRACEE("unknow connect!!");
		if(hClient != NULL)
		{
			pSampler->pUsedCommPort->Close(hClient);
			hClient = NULL;
		}			
	}
	return 1;    //accept success maybe new client or repeat client
}


BOOL UART_SERVER_TCP_InitSampler(IN ST_UART_SERVER_ROUGHDATA* pUart)
{
    TRACEI("Enter UART_SERVER_TCP_InitSampler ! \n");

    SAMPLER* pSampler = (SAMPLER*)pUart->hSampler;
    DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	UART_SERVER_TCP_RUNROUGH* pUartServerTCPRough = (void*)NEW(UART_SERVER_TCP_RUNROUGH, 1);
	if(NULL == pUartServerTCPRough)
	{	
		LogOut(MAIN_MODULE_SAMPLER, "UART SERVER TCP Samp", LOG_TYPE_ERROR, "New struct UART_SERVER_TCP_RUNROUGH FAILED!!!");
		return FALSE;
	}
	ZERO_POBJS(pUartServerTCPRough, 1);
    s_pUartServerTCPRough = pUartServerTCPRough; // 

    //attach the pointer
    pUartServerTCPRough->pUART = pUart;
	pUartServerTCPRough->nClientConnTotal = 0;

	//get install type 
	VAR_VALUE varV;
	pUart->pDPR->Get(pUart->hSampler, GETTYPE_ST_VAR, 901, 910, &varV, 4);
	pUart->bUartServerInstall = varV.emValue;
	pUart->pDPR->Get(pUart->hSampler, GETTYPE_ST_VAR, 901, 917, &varV, 4);//串口服务器#1场景选择
	pUartServerTCPRough->bUartServerRunning_A = varV.emValue;
	pUart->pDPR->Get(pUart->hSampler, GETTYPE_ST_VAR, 901, 918, &varV, 4);//串口服务器#2场景选择
	pUartServerTCPRough->bUartServerRunning_B = varV.emValue;
	pUart->pDPR->Get(pUart->hSampler, GETTYPE_ST_VAR, 901, 919, &varV, 4);//串口服务器#3场景选择
	pUartServerTCPRough->bUartServerRunning_C = varV.emValue;
	pUart->pDPR->Get(pUart->hSampler, GETTYPE_ST_VAR, 901, 920, &varV, 4);//串口服务器#4场景选择
	pUartServerTCPRough->bUartServerRunning_D = varV.emValue;
	TRACED("pUartRunData->bUartServerInstall = %d ", pUart->bUartServerInstall);
	memset(pUartServerTCPRough->szClientIP, 0, 4*20);
	memset(pUartServerTCPRough->stTcpClientInfo, 0, sizeof(pUartServerTCPRough->stTcpClientInfo));

	//if (pUart->bUartServerInstall)
	{
		varV.blobValue=pUartServerTCPRough->szClientIP[0]; //tempHumidity
		pUart->pDPR->Get(pUart->hSampler, GETTYPE_ST_VAR, 901, 911, &varV, 15);

		varV.blobValue=pUartServerTCPRough->szClientIP[1];//kwh meter
		pUart->pDPR->Get(pUart->hSampler, GETTYPE_ST_VAR, 901, 912, &varV, 15);

		varV.blobValue=pUartServerTCPRough->szClientIP[2];//power Quality analyzer
		pUart->pDPR->Get(pUart->hSampler, GETTYPE_ST_VAR, 901, 913, &varV, 15);

		varV.blobValue=pUartServerTCPRough->szClientIP[3];//reserve
		pUart->pDPR->Get(pUart->hSampler, GETTYPE_ST_VAR, 901, 914, &varV, 15);
	}

	// int j;
	// for (j=0;j<4;j++)
	// {
	// 	TRACEE("pUart->szClientIP[%d] = %s ",j,pUartServerTCPRough->szClientIP[j]);
	// }
	// TRACEI("UART_SERVER_TCP_InitSampler  pUart->bUartServerInstall = %d  ",pUart->bUartServerInstall);

    int i = 0;
	for(i = 0; i < TCP_CLIENT_NUM_MAX; i++)
	{
		pUartServerTCPRough->stTcpClientInfo[i].hClient = NULL;
		pUartServerTCPRough->stTcpClientInfo[i].emRunCommStatus = CLITNT_COMM_STAT_WAITCONNECT;
		pUartServerTCPRough->stTcpClientInfo[i].bNeedExitThread = FALSE;
		pUartServerTCPRough->stTcpClientInfo[i].hMutexLock = Mutex_Create(TRUE);
		if(pUartServerTCPRough->stTcpClientInfo[i].hMutexLock == NULL)
		{
			LogOut(MAIN_MODULE_SAMPLER, "UART SERVER TCP Samp", LOG_TYPE_ERROR, "Creat Mutex Lock FAILED!!!");
			return FALSE;
		}
	}

    //Now open the port	
	if(!UART_SERVER_TCP_ReopenServerPort(pUartServerTCPRough))
	{
		LogOut(MAIN_MODULE_SAMPLER, "UART SERVER TCP Samp", LOG_TYPE_ERROR, "Reopen Server Port FAILED!!!");
		return FALSE;
	}
	pUartServerTCPRough->tmLastCheckCliConnectOrReopenPort = _NOW_;
	
    return TRUE;

}

BOOL UartJudgeAndSaveToDB(ST_UART_SERVER_ROUGHDATA *pWhim_Rough,int nWhichWh)
{
	if (nWhichWh != 0)  //only 401
	{
		return 0;
	}

	time_t tmNow = _NOW_;
	struct tm gmTime;
	gmtime_r(&tmNow, &gmTime);
	TRACEW("uart---JudgeAndSaveToDB---CURRENTLASTYear = [%d] CURRENTLASTMonth = [%d] CURRENTLASTDay = [%d]\n",
		pWhim_Rough->meterData[0].nLastYearToSaveDB,pWhim_Rough->meterData[0].nLastMonthToSaveDB,pWhim_Rough->meterData[0].nLastDayToSaveDB);

	if (pWhim_Rough->meterData[0].nLastYearToSaveDB != gmTime.tm_year || 
		pWhim_Rough->meterData[0].nLastMonthToSaveDB != gmTime.tm_mon || 
		pWhim_Rough->meterData[0].nLastDayToSaveDB != gmTime.tm_mday )    //one day one record
	{

		EVENT_METER_SAVE_DATA Event_KwhMeterVar;
		memset(&Event_KwhMeterVar, 0, sizeof(EVENT_METER_SAVE_DATA));

		Event_KwhMeterVar.ndevid = 401+nWhichWh;
		if (sizeof(Event_KwhMeterVar.varEventWHMeterRough) == sizeof(pWhim_Rough->meterData[nWhichWh].varWHMeterRough))
		{		
			memcpy(Event_KwhMeterVar.varEventWHMeterRough,  pWhim_Rough->meterData[nWhichWh].varWHMeterRough, 
				sizeof(Event_KwhMeterVar.varEventWHMeterRough));
		}

		SendMsg(pWhim_Rough->hSampler, EVENT_TYPE_KWHMETER_CURRENT_POS_NEG_SPFW, EVENT_MSG_STATUS_CHANGED,
			&Event_KwhMeterVar, sizeof(EVENT_METER_SAVE_DATA));

		pWhim_Rough->meterData[0].nLastYearToSaveDB = gmTime.tm_year;
		pWhim_Rough->meterData[0].nLastMonthToSaveDB = gmTime.tm_mon;
		pWhim_Rough->meterData[0].nLastDayToSaveDB = gmTime.tm_mday;

		TRACEW("uart---JudgeAndSaveToDB---tm_year=[%d] tm_mon=[%d] tm_mday=[%d]  tm_hour=[%d] tm_min=[%d]",
			gmTime.tm_year,gmTime.tm_mon,gmTime.tm_mday,gmTime.tm_hour,gmTime.tm_min);
	}
	return 0;
}

DLLExport BOOL Initialize(HANDLE hSamplerSelf)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
    pSampler->pDataHeap = (void*)NEW(ST_UART_SERVER_ROUGHDATA, 1);
    if(pSampler->pDataHeap == NULL)
	{
		return FALSE;
	}
    ST_UART_SERVER_ROUGHDATA * pUartServerInit = pSampler->pDataHeap;
    ZERO_POBJS(pUartServerInit, 1);

    pUartServerInit->hSamplerLock = pSampler->hSamplerLock; //Init lock
	pUartServerInit->hSampler = hSamplerSelf;
	pUartServerInit->pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
    pUartServerInit->tmSysBoot = _NOW_;

    TRACEI("uart_server_main initialize pSampler->info.szSamplerSetting =  %s",pSampler->info.szSamplerSetting);   //192.168.137.201:8899

	int n;
	EVHAND_TAGS tags[] = 
	{
		{EVENT_TYPE_KWHMETER_CURRENT_POS_NEG_SPFW,	EVENT_MSG_RESTORE},
	};
	for(n = 0; n < ITEM_OF(tags); n++)
	{
		pUartServerInit->pDPR->Register_EvHandler("UART_KWHMeter_EvHandler",
			hSamplerSelf,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}

    if(!UART_SERVER_TCP_InitSampler(pUartServerInit))
	{
		return FALSE;
	}
	Init_TempHumiData(pUartServerInit);
	//init 645 
	whMeter_InitRoughData(pUartServerInit);

    return TRUE;
}

DLLExport int Sample(HANDLE hSamplerThis)
{
    SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
    ST_UART_SERVER_ROUGHDATA* pUartServerSample = (ST_UART_SERVER_ROUGHDATA*)pSampler->pDataHeap;

	UART_SERVER_TCP_ServerAcceptComm(NULL);	
    return ERR_SAMPLE_OK;

}


DLLExport int Control(HANDLE hSamplerThis , PARAM_TYPE	emParaType,
		int			nDevId,
		int			nParamId,
		VAR_VALUE	*pVarVal,
		int			nValSize,
		int			nTimeOut)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	UART_SERVER_TCP_RUNROUGH * pUartRunData = s_pUartServerTCPRough;
    ST_UART_SERVER_ROUGHDATA* pUartServerControl = (ST_UART_SERVER_ROUGHDATA*)pSampler->pDataHeap;
	
	TRACEW("i am uart server main----Control!!!!!!!!!!!\n");

	HANDLE hSamplerLock = ((SAMPLER*)pUartServerControl->hSampler)->hSamplerLock;
	if(Mutex_Lock(hSamplerLock, nTimeOut) != ERR_MUTEX_OK)
	{
		return ERR_CONTROL_TIMEOUT;
	}

	if(emParaType == PARAM_TYPE_SETTING)   //settings
	{	
		if(nParamId == 910)
		{
			if(pUartServerControl->bUartServerInstall != pVarVal->emValue)
			{
				pUartServerControl->bUartServerInstall = pVarVal->emValue;
			}
		}
		else if(nParamId == 911)
		{
			if(strcmp(pVarVal->blobValue, pUartRunData->szClientIP[0]) != 0)
			{
				strncpy(pUartRunData->szClientIP[0], (const char *)pVarVal->blobValue, 19);  
				TRACEI("now to setting the ip[0] new = %s ",pUartRunData->szClientIP[0]);
			}

		}
		else if(nParamId == 912)
		{
			if(strcmp(pVarVal->blobValue, pUartRunData->szClientIP[1]) != 0)
			{
				strncpy(pUartRunData->szClientIP[1], (const char *)pVarVal->blobValue, 19);  
				TRACEI("now to setting the ip[1] new = %s ",pUartRunData->szClientIP[1]);
			}

		}
		else if(nParamId == 913)
		{
			if(strcmp(pVarVal->blobValue, pUartRunData->szClientIP[2]) != 0)
			{
				strncpy(pUartRunData->szClientIP[2], (const char *)pVarVal->blobValue, 19);  
				TRACEI("now to setting the ip[2] new = %s ",pUartRunData->szClientIP[2]);
			}

		}
		else if(nParamId == 914)
		{
			if(strcmp(pVarVal->blobValue, pUartRunData->szClientIP[3]) != 0)
			{
				strncpy(pUartRunData->szClientIP[3], (const char *)pVarVal->blobValue, 19);  
				TRACEI("now to setting the ip[3] new = %s ",pUartRunData->szClientIP[3]);
			}

		}
		else if(nParamId == 401)   //install type
		{
			int nWhichWh = (nDevId - 401);
			if((nWhichWh < 0) || (nWhichWh >= MAX_WH_NUM))
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			if(nValSize != 4)
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			pUartServerControl->meterData[nWhichWh].emMeterType = (METER_TYPE)pVarVal->emValue;
		}
		else if(nParamId == 402)  //KWH factor
		{
			int nWhichWh =(nDevId - 401);
			if((nWhichWh < 0) || (nWhichWh >= MAX_WH_NUM))
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			if(nValSize != 4)
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			pUartServerControl->meterData[nWhichWh].fACMeter_PowerFactor = pVarVal->fValue;
		}
		else if(nParamId == 403)  //sn
		{
			int nWhichWh = (nDevId - 401);
			if((nWhichWh < 0) || (nWhichWh >= MAX_WH_NUM))
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			if(nValSize != 13)
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			UartServerGetMeterSN(pUartServerControl, nWhichWh, (char*)pVarVal->blobValue);
			TRACEE("control ----- SN = %s ",(char*)pVarVal->blobValue);
		}
		else if(nParamId == 404)  //protocol type
		{
			int nWhichWh = (nDevId - 401);
			if((nWhichWh < 0) || (nWhichWh >= MAX_WH_NUM))
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			if(nValSize != 4)
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			pUartServerControl->meterData[nWhichWh].emACMeter_ProtocolType = (ACMETER_PROTOTP)pVarVal->emValue;
		}
		else if(nParamId == 915)//edit by kuangyue 20251029
		{
			if(pUartServerControl->stTempAndHumiRead.bInstallType != pVarVal->emValue)
			{
				pUartServerControl->stTempAndHumiRead.bInstallType = pVarVal->emValue;
			}
		}
		else if(nParamId == 917)//串口服务器#1场景选择
		{
			if(pUartRunData->bUartServerRunning_A != pVarVal->emValue)
			{
				pUartRunData->bUartServerRunning_A = pVarVal->emValue;
			}
		}
		else if(nParamId == 918)//串口服务器#2场景选择
		{
			if(pUartRunData->bUartServerRunning_B != pVarVal->emValue)
			{
				pUartRunData->bUartServerRunning_B = pVarVal->emValue;
			}
		}
		else if(nParamId == 919)//串口服务器#3场景选择
		{
			if(pUartRunData->bUartServerRunning_C != pVarVal->emValue)
			{
				pUartRunData->bUartServerRunning_C = pVarVal->emValue;
			}
		}
		else if(nParamId == 920)//串口服务器#4场景选择
		{
			if(pUartRunData->bUartServerRunning_D != pVarVal->emValue)
			{
				pUartRunData->bUartServerRunning_D = pVarVal->emValue;
			}
		}
		// else if(nParamId == 405)   //baud
		// {
		// 	int nWhichWh = (nDevId - 401);
		// 	if((nWhichWh < 0) || (nWhichWh >= MAX_WH_NUM))
		// 	{
		// 		Mutex_Unlock(hSamplerLock);
		// 		return ERR_CONTROL_FAILED;
		// 	}
		// 	if(nValSize != 4)
		// 	{
		// 		Mutex_Unlock(hSamplerLock);
		// 		return ERR_CONTROL_FAILED;
		// 	}
		// 	TRACE("The kwhMeter[%d] baudrate is[%s]\n", nWhichWh, pUartServerControl->meterData[nWhichWh].szBaudrateSett);
		// }

	}

	else//control 
	{


	}

	Mutex_Unlock(hSamplerLock);
	return ERR_CONTROL_OK;


}


DLLExport int JigTest(HANDLE hSamplerThis, void* pJigData)
{
	TRACEW("i am uart server main----JigTest!!!!!!!!!!!\n");
	return 0;
}



