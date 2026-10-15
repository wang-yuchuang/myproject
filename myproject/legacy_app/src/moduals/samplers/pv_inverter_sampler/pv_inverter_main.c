// #define _TRACE_PRINT_
#include "pv_inverter_main.h"
#include "pv_modbus_rtu.h"

static COMMPORT s_pRS485_1_Comm;
static COMMPORT s_pRS485_2_Comm;

int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE		emEvType,
	_EVENT_MSG		emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int				nMaxBlockMSec)
{
	SAMPLER* pSampler = (SAMPLER*)hModuleSeif;
	ST_PV_ROUGHDATA* pPV_INVERTER_Rough = (ST_PV_ROUGHDATA*)pSampler->pDataHeap;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	
	ERRCDDE_DPR err;
	VAR_VALUE 	varV;

	TRACE("-------PV INVERTER got EVENT[%d] nEvDataSize=[%d]--------\n", emEvType, nEvDataSize);
	 
	if(emEvType == EVENT_TYPE_CFG_CHANGED)
	{
		if(nEvDataSize != sizeof(EVENT_PARAM_CHG))
		{
			return EVENT_HANDED_FAILED;
		}

	}
	return EVENT_HANDED_OK;
}

PORT_THREAD_INFO* Get_Port_By_Type(ST_PV_ROUGHDATA *pPV_Rough, PHY_INTER_TYPE ePortType)
{
    if (ePortType >= _MAX_PHY_INTER_TYPE)
        return NULL;
        
    return &pPV_Rough->portThreadInfo[ePortType];
}

// 打开单个端口
BOOL Open_Single_Port(ST_PV_ROUGHDATA *pPV_Rough, PHY_INTER_TYPE ePortType)
{
	printf("Open Port[%d]...\n", ePortType);
    int nErrrCode;
    SAMPLER* pSampler = (SAMPLER*)pPV_Rough->hSampler;
    DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
    PORT_THREAD_INFO* pPortInfo = Get_Port_By_Type(pPV_Rough, ePortType);
    
    if (pPortInfo == NULL)
    {
		// printf("GetPortInfo failed for port type[%d]\n", ePortType);
        TRACEE("Invalid port type: %d\n", ePortType);
        return FALSE;
    }
    
    // 如果已经打开，先关闭
    if (pPortInfo->bIsOpened && pPortInfo->hPort != NULL)
    {
        pPortInfo->pCommPort->Close(pPortInfo->hPort);
        pPortInfo->hPort = NULL;
        pPortInfo->bIsOpened = FALSE;
    }
	// printf("Opening Port[%d]...\n", ePortType);
    
    pPortInfo->ePortType = ePortType;
    
    // 根据端口类型打开不同的端口
    if (ePortType == PHY_RS485_1)
    {
		printf("Opening RS485_1...\n");
        // RS485_1
        int portId = 3; // ttyS3
        if (GetCommPort_ById((HANDLE)pSampler, portId, &s_pRS485_1_Comm) == NULL)
        {
            TRACEE("GetCommPort_ById:%d failed\n", portId);
            return FALSE;
        }
        
        pPortInfo->pCommPort = &s_pRS485_1_Comm;
        strcpy(pPortInfo->szPortConfig, "9600,N,8,1");
        
        pPortInfo->hPort = pPortInfo->pCommPort->Open(
            "ttyS3", 
            pPortInfo->szPortConfig,
            COMM_CLIENT_MODE, 1000, &nErrrCode);
    }
    else if (ePortType == PHY_RS485_2)
    {
		printf("Opening RS485_2...\n");
        // RS485_2
        int portId = 4; // ttyS4
        if (GetCommPort_ById((HANDLE)pSampler, portId, &s_pRS485_2_Comm) == NULL)
        {
            TRACEE("GetCommPort_ById:%d failed\n", portId);
            return FALSE;
        }
        
        pPortInfo->pCommPort = &s_pRS485_2_Comm;
        strcpy(pPortInfo->szPortConfig, "9600,N,8,1");
        
        pPortInfo->hPort = pPortInfo->pCommPort->Open(
            "ttyS4", 
            pPortInfo->szPortConfig,
            COMM_CLIENT_MODE, 1000, &nErrrCode);
    }
    else if (ePortType >= PHY_UART_SERVER && ePortType <= PHY_UART_SERVER_4)
    {
		printf("Opening UartServer[%d]...\n", ePortType);
        // 串口服务器 1-4
		int portId = 7;		// tcp
        COMMPORT uartServerComm;
        int portOffset = ePortType - PHY_UART_SERVER;
        int targetPort = pPV_Rough->uartServerCfg.iStartPort + portOffset;
        
        char szServerAddr[128];
        sprintf(szServerAddr, "%s:%d", pPV_Rough->uartServerCfg.szIP, targetPort);
		printf("Opening UartServer[%d]: %s\n", portOffset + 1, szServerAddr);
        
        // 获取通用串口服务器通信端口
        if (GetCommPort_ById((HANDLE)pSampler, portId, &uartServerComm) == NULL)
        {
			// printf("GetCommPort for UartServer failed\n");
            TRACEE("GetCommPort for UartServer failed\n");
            return FALSE;
        }
        
        pPortInfo->pCommPort = (COMMPORT*)malloc(sizeof(COMMPORT));
        memcpy(pPortInfo->pCommPort, &uartServerComm, sizeof(COMMPORT));
        strcpy(pPortInfo->szPortConfig, szServerAddr);
        
        pPortInfo->hPort = pPortInfo->pCommPort->Open(
            pPortInfo->pCommPort->info.szPortAttri,
            szServerAddr,
            COMM_CLIENT_MODE, 1000, &nErrrCode);
            
        TRACEI("Open UartServer Port[%d]: %s\n", portOffset + 1, szServerAddr);
    }
    else
    {
        TRACEE("Unsupported port type: %d\n", ePortType);
        return FALSE;
    }
    
    if (ERR_COMM_OK != nErrrCode || pPortInfo->hPort == NULL)
    {
        TRACEE("Open port type[%d] failed, errcode=%d\n", ePortType, nErrrCode);
        pPortInfo->bIsOpened = FALSE;
        return FALSE;
    }
    
    pPortInfo->bIsOpened = TRUE;
    pPortInfo->uCommFailCount = 0;
    
    // 清空缓冲区
    pPortInfo->pCommPort->Control(pPortInfo->hPort, COMM_PURGE_RXCLEAR, NULL, 0);
    
	// printf("Open Port[%d] successfully\n", ePortType);
    TRACEI("Port type[%d] opened successfully\n", ePortType);
    return TRUE;
}

BOOL getBaudRate(ENU_INVERTER_BAUD_RATE eBaud, char szBaudrate[])
{
	if (BAUD_RATE_4800 == eBaud)
	{
		strcpy(szBaudrate, "4800,n,8,1");
	}
	else if (BAUD_RATE_115200 == eBaud)
	{
		strcpy(szBaudrate, "115200,n,8,1");	
	}
	else
	{
		strcpy(szBaudrate, "9600,n,8,1");
	}

	return TRUE;
}

BOOL setAllPVCmd(ST_PV_ROUGHDATA* pPvInverterSample, ENU_PV_CMD eCmd, SHORT* usCmdData)
{
	int kkk = 0;
	SHORT sPowerAverage = 0;

	if (pPvInverterSample->blobPVTotalinfo.nAvailablePVNums == 0)
	{
		TRACEE("NO Available PV!\n");
		return FALSE;
	}

	if (eCmd == PV_CMD_ActiveOutputPower || eCmd == PV_CMD_ReActiveOutputPower 
			||eCmd == PV_CMD_MaxActivePowerRate)
	{
		sPowerAverage = *usCmdData / pPvInverterSample->blobPVTotalinfo.nAvailablePVNums;
		usCmdData = &sPowerAverage;
	}

	for(kkk = 0; kkk < MAX_PVInverter_NUM; kkk++)
	{
		if(pPvInverterSample->stPVSettingParam[kkk].eInvertType == INVERTER_TYPE_NOT_INSTALL ||
			pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData.iWorkMode != PV_STATUS_RUNNING)
		{
			continue;
		}

		if (!setPVCmd(pPvInverterSample, eCmd, usCmdData,kkk))
		{
			TRACEE("setAllPVCmd - set[%d] Failed!\n",kkk);
			return FALSE;
		}
	}
	return TRUE;
}

// 初始化函数
DLLExport BOOL Initialize(HANDLE hSamplerSelf)
{
    SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
    pSampler->pDataHeap = (void*)NEW(ST_PV_ROUGHDATA, 1);
    if (pSampler->pDataHeap == NULL)
    {
        return FALSE;
    }
    
    ST_PV_ROUGHDATA* pPvInverterInit = pSampler->pDataHeap;
    ZERO_POBJS(pPvInverterInit, 1);
    
    pPvInverterInit->hSamplerLock = pSampler->hSamplerLock;
    pPvInverterInit->hSampler = hSamplerSelf;
    pPvInverterInit->pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
    pPvInverterInit->tmSysBoot = _NOW_;
    pPvInverterInit->bCommStatusOk = FALSE;
    pPvInverterInit->bSystemExit = FALSE;
    
    // 初始化全局数据互斥锁
    pthread_mutex_init(&pPvInverterInit->globalDataMutex, NULL);
    
    // 初始化所有端口信息
    int i;
    for (i = 0; i < MAX_PHY_PORT_NUM; i++)
    {
        pPvInverterInit->portThreadInfo[i].ePortType = (PHY_INTER_TYPE)i;
        pPvInverterInit->portThreadInfo[i].bIsOpened = FALSE;
        pPvInverterInit->portThreadInfo[i].hPort = NULL;
        pPvInverterInit->portThreadInfo[i].pCommPort = NULL;
        pPvInverterInit->portThreadInfo[i].bThreadRunning = FALSE;
        pPvInverterInit->portThreadInfo[i].bThreadShouldExit = FALSE;
        pPvInverterInit->portThreadInfo[i].pPvRoughData = pPvInverterInit;
        pPvInverterInit->portThreadInfo[i].uSampleCount = 0;
        pPvInverterInit->portThreadInfo[i].uCommFailCount = 0;
    }
    
    TRACEI("PV-Inverter initialize...\n");
	// printf("PV Sampler Initialized...\n");
    
    // 初始化逆变器数据
    Init_Pv_InverteData(pPvInverterInit);
    
    // 打开所有需要的端口
    if (!Open_All_Comm_Ports(pPvInverterInit))
    {
		// printf("PV Sampler Failed to open all comm ports\n");
        TRACEE("Failed to open all comm ports\n");
    }
    
    // 启动所有端口线程
    if (!Start_All_Port_Threads(pPvInverterInit))
    {
		// printf("PV Sampler Failed to start all port threads\n");
        TRACEE("Failed to start all port threads\n");
        Cleanup_All_Resources(pPvInverterInit);
        return FALSE;
    }
    
    // 注册事件处理器
    EVHAND_TAGS tags[] = 
    {
        {EVENT_TYPE_CFG_CHANGED, EVENT_MSG_BEGIN},
        // {EVENT_TYPE_SYSTEM_SHUTDOWN, EVENT_MSG_BEGIN},  // 新增系统关闭事件
    };
    
    int n;
    for (n = 0; n < ITEM_OF(tags); n++)
    {
        pPvInverterInit->pDPR->Register_EvHandler("PV_INVERTER_EvHandler",
            hSamplerSelf,
            Event_Handler,
            tags[n].nEvTag,
            tags[n].nMsgTag,
            FALSE);
    }
    
    return TRUE;
}

// 关闭单个端口
BOOL Close_Single_Port(ST_PV_ROUGHDATA *pPV_Rough, PHY_INTER_TYPE ePortType)
{
    PORT_THREAD_INFO* pPortInfo = Get_Port_By_Type(pPV_Rough, ePortType);
    
    if (pPortInfo == NULL)
        return FALSE;
    
    if (pPortInfo->bIsOpened && pPortInfo->hPort != NULL)
    {
        pPortInfo->pCommPort->Close(pPortInfo->hPort);
        pPortInfo->hPort = NULL;
        pPortInfo->bIsOpened = FALSE;
        
        // 如果是动态分配的串口服务器端口，释放内存
        if (ePortType >= PHY_UART_SERVER && ePortType <= PHY_UART_SERVER_4)
        {
            if (pPortInfo->pCommPort != NULL)
            {
                free(pPortInfo->pCommPort);
                pPortInfo->pCommPort = NULL;
            }
        }
        
        TRACEI("Port type[%d] closed\n", ePortType);
    }
    
    return TRUE;
}

// 打开所有需要的通信端口
BOOL Open_All_Comm_Ports(ST_PV_ROUGHDATA *pPV_Rough)
{
	// printf("PV Sampler Opening all comm ports...\n");
    int kkk;
    BOOL bNeedPorts[_MAX_PHY_INTER_TYPE] = {FALSE};
    
    // 遍历所有逆变器配置，确定需要打开哪些端口
    for (kkk = 0; kkk < MAX_PVInverter_NUM; kkk++)
    {
        if (pPV_Rough->stPVSettingParam[kkk].eInvertType != INVERTER_TYPE_NOT_INSTALL)
        {
            PHY_INTER_TYPE ePort = pPV_Rough->stPVSettingParam[kkk].ePhyInterface;
            if (ePort < _MAX_PHY_INTER_TYPE)
            {
                bNeedPorts[ePort] = TRUE;
            }
        }
    }
    
	// for (kkk = 0; kkk < _MAX_PHY_INTER_TYPE; kkk++)
	// {
	// 	printf("bNeedPorts[%d] = [%d]\n", kkk, bNeedPorts[kkk]);
	// }
    
	// 打开所有需要的端口
    BOOL bAllSuccess = TRUE;
    int i;
    for (i = 0; i < _MAX_PHY_INTER_TYPE; i++)
    {
        if (bNeedPorts[i])
        {
            if (!Open_Single_Port(pPV_Rough, (PHY_INTER_TYPE)i))
            {
                TRACEE("Failed to open port type: %d\n", i);
                bAllSuccess = FALSE;
            }
        }
    }
    
    return bAllSuccess;
}

// 关闭所有端口
BOOL Close_All_Comm_Ports(ST_PV_ROUGHDATA *pPV_Rough)
{
    int i;
    for (i = 0; i < _MAX_PHY_INTER_TYPE; i++)
    {
        Close_Single_Port(pPV_Rough, (PHY_INTER_TYPE)i);
    }
    
    return TRUE;
}

// 停止所有端口线程
BOOL Stop_All_Port_Threads(ST_PV_ROUGHDATA *pPV_Rough)
{
    int i;
    
    TRACEI("Stopping all port threads...\n");
    
    // 设置退出标志
    for (i = 0; i < _MAX_PHY_INTER_TYPE; i++)
    {
        PORT_THREAD_INFO* pPortInfo = &pPV_Rough->portThreadInfo[i];
        if (pPortInfo->bThreadRunning)
        {
            pPortInfo->bThreadShouldExit = TRUE;
        }
    }
    
    // 等待线程退出（最多等待10秒）
    int waitCount = 0;
    while (waitCount < 100)  // 10秒 = 100 * 100ms
    {
        BOOL bAllExited = TRUE;
        for (i = 0; i < _MAX_PHY_INTER_TYPE; i++)
        {
            if (pPV_Rough->portThreadInfo[i].bThreadRunning)
            {
                bAllExited = FALSE;
                break;
            }
        }
        
        if (bAllExited)
            break;
        
        Sleep(100);
        waitCount++;
    }
    
    // 销毁互斥锁
    for (i = 0; i < _MAX_PHY_INTER_TYPE; i++)
    {
        PORT_THREAD_INFO* pPortInfo = &pPV_Rough->portThreadInfo[i];
        if (pPortInfo->ePortType < _MAX_PHY_INTER_TYPE)
        {
            pthread_mutex_destroy(&pPortInfo->dataMutex);
        }
    }
    
    TRACEI("All port threads stopped\n");
    return TRUE;
}

// 清理所有资源
BOOL Cleanup_All_Resources(ST_PV_ROUGHDATA* pPvInverter)
{
    if (pPvInverter == NULL)
        return FALSE;
    
    TRACEI("Starting cleanup all resources...\n");
    
    // 设置退出标志
    pPvInverter->bSystemExit = TRUE;
    
    // 停止所有线程
    Stop_All_Port_Threads(pPvInverter);
    
    // 关闭所有端口
    Close_All_Comm_Ports(pPvInverter);
    
    // 销毁全局互斥锁
    pthread_mutex_destroy(&pPvInverter->globalDataMutex);
    
    TRACEI("All resources cleaned up\n");
    return TRUE;
}

// 根据设备配置设置波特率
BOOL setCommBaudRate(ST_PV_ROUGHDATA* pPvInverterSample, UINT inwhich)
{
    char szBaudrateSett[32] = {0};
    PHY_INTER_TYPE ePortType = pPvInverterSample->stPVSettingParam[inwhich].ePhyInterface;
    PORT_THREAD_INFO* pPortInfo = Get_Port_By_Type(pPvInverterSample, ePortType);
    
    if (pPortInfo == NULL || !pPortInfo->bIsOpened)
    {
        TRACEE("Port not opened for device[%d]\n", inwhich);
        return FALSE;
    }
    
    // 清空接收缓冲区
    pPortInfo->pCommPort->Control(pPortInfo->hPort, COMM_PURGE_RXCLEAR, NULL, 0);
    
    // 只有RS485端口需要动态切换波特率
    if (ePortType == PHY_RS485_1 || ePortType == PHY_RS485_2)
    {
        getBaudRate(pPvInverterSample->stPVSettingParam[inwhich].eBaudRate, szBaudrateSett);
        pPortInfo->pCommPort->Control(pPortInfo->hPort, COMM_SET_SERIAL_ATTRIB, 
                                      (void*)szBaudrateSett, strlen(szBaudrateSett));
    }
    
    return TRUE;
}

// 采样单个逆变器
int samplePVInverter(ST_PV_ROUGHDATA* pPvInverterSample, PV_INVERTER_DATA* pPVOutData, UINT inwhich)
{
    PHY_INTER_TYPE ePortType = pPvInverterSample->stPVSettingParam[inwhich].ePhyInterface;
    PORT_THREAD_INFO* pPortInfo = Get_Port_By_Type(pPvInverterSample, ePortType);
    
	// printf("samplePVInverter: inwhich=[%d], ePortType=[%d]\n", inwhich, ePortType);
	BOOL bIsSamplerValid = FALSE;
    if (pPortInfo == NULL || !pPortInfo->bIsOpened)
    {
        TRACEE("Port not opened for inverter[%d], port type=%d\n", inwhich, ePortType);
        pPvInverterSample->stPVRunData[inwhich].bIsCommFail = TRUE;
        return bIsSamplerValid;
    }
    
    setCommBaudRate(pPvInverterSample, inwhich);
    Sleep(10);
    
	
    // 根据逆变器类型调用对应的采样函数
	// printf("eInvertType[%d] = [%d]\n", inwhich, pPvInverterSample->stPVSettingParam[inwhich].eInvertType);
    if (INVERTER_TYPE_Growatt == pPvInverterSample->stPVSettingParam[inwhich].eInvertType)
    {
        bIsSamplerValid = samplePVInverterGrowatt(pPvInverterSample, pPVOutData, inwhich);
    }
    else if (INVERTER_TYPE_Huawei == pPvInverterSample->stPVSettingParam[inwhich].eInvertType)
    {
        bIsSamplerValid = samplePVInverterHuawei(pPvInverterSample, pPVOutData, inwhich);
    }
    else if (INVERTER_TYPE_Ginlong == pPvInverterSample->stPVSettingParam[inwhich].eInvertType)
    {
        bIsSamplerValid = samplePVInverterGinlong(pPvInverterSample, pPVOutData, inwhich);
    }
    else if (INVERTER_TYPE_Sungrow == pPvInverterSample->stPVSettingParam[inwhich].eInvertType)
    {
        bIsSamplerValid = samplePVInverterSungrow(pPvInverterSample, pPVOutData, inwhich);
    }
    else
    {
        return bIsSamplerValid;
    }

	return bIsSamplerValid;
}

// ==================== 端口采样线程函数 ====================
void* Port_Sample_Thread(void* pParam)
{
    PORT_THREAD_INFO* pPortInfo = (PORT_THREAD_INFO*)pParam;
    ST_PV_ROUGHDATA* pPV_Rough = pPortInfo->pPvRoughData;
    PHY_INTER_TYPE ePortType = pPortInfo->ePortType;
    
    TRACEI("Port[%d] sample thread started, TID=%lu\n", ePortType, pthread_self());
	// printf("Port[%d] sample thread started, TID=%lu\n", ePortType, pthread_self());
    
    pPortInfo->bThreadRunning = TRUE;
    
    // 线程主循环
    while (!pPortInfo->bThreadShouldExit && !pPV_Rough->bSystemExit)
    {
        // 检查端口是否打开
		// printf("Port[%d]->bIsOpened = [%d]\n", ePortType, pPortInfo->bIsOpened);
        if (!pPortInfo->bIsOpened)
        {
            TRACEW("Port[%d] not opened, attempting to reopen...\n", ePortType);
			// printf("Port[%d] not opened, attempting to reopen...\n", ePortType);
            if (Open_Single_Port(pPV_Rough, ePortType))
            {
                TRACEI("Port[%d] reopened successfully\n", ePortType);
            }
            else
            {
                Sleep(1000);  // 重试间隔1秒
                continue;
            }
        }
        
        // 遍历所有逆变器，找出属于当前端口的设备
        int kkk;
        BOOL bHasDevice = FALSE;
        for (kkk = 0; kkk < MAX_PVInverter_NUM; kkk++)
        {
            // 跳过未安装的设备
            if (pPV_Rough->stPVSettingParam[kkk].eInvertType == INVERTER_TYPE_NOT_INSTALL)
                continue;
            
            // 跳过不属于当前端口的设备
            if (pPV_Rough->stPVSettingParam[kkk].ePhyInterface != ePortType)
                continue;
            
            bHasDevice = TRUE;
            
            // DO切换模式特殊处理
            if ((pPV_Rough->emPvDataAcquisitonTypes == PV_DATA_ACQUISITION_DO_SWITCH) &&
                (pPV_Rough->blobPVTotalinfo.emDIstatus == DI_STATUS_TRANSFER))
            {
                // 让渡控制权，跳过采样
                pthread_mutex_lock(&pPortInfo->dataMutex);
                ZERO_POBJS(&pPV_Rough->stPVRunData[kkk], 1);
                pthread_mutex_unlock(&pPortInfo->dataMutex);
                continue;
            }
            
            // 采样该设备
            PV_INVERTER_DATA pvData;
            ZERO_POBJS(&pvData, 1);
            
            int valid = samplePVInverter(pPV_Rough, &pvData, kkk);
            // printf("samplePVInverter[%d]: valid=[%d]\n", kkk, valid);
            // 更新数据（加锁保护）
			if (valid)
			{
				pthread_mutex_lock(&pPortInfo->dataMutex);
            	memcpy(&pPV_Rough->stPVRunData[kkk].blobSinglePVdata.PVInverterData, 
                   &pvData, sizeof(PV_INVERTER_DATA));
            	pthread_mutex_unlock(&pPortInfo->dataMutex);
			}
            
            // 设备间隔
            Sleep(100);
        }
        
        if (bHasDevice)
        {
            pPortInfo->uSampleCount++;
            TRACED("Port[%d] sample completed, count=%u\n", ePortType, pPortInfo->uSampleCount);
        }
        
        // 采样周期（根据实际需求调整）
        Sleep(1000);  // 1秒采样周期
    }
    
    pPortInfo->bThreadRunning = FALSE;
    TRACEI("Port[%d] sample thread exited\n", ePortType);
    
    return NULL;
}

// 启动所有端口线程
BOOL Start_All_Port_Threads(ST_PV_ROUGHDATA *pPV_Rough)
{
	// printf("Start_All_Port_Threads=================\n");
    int i;
    BOOL bNeedPorts[_MAX_PHY_INTER_TYPE] = {FALSE};
    
    // 确定需要启动哪些端口线程
    for (i = 0; i < MAX_PVInverter_NUM; i++)
    {
        if (pPV_Rough->stPVSettingParam[i].eInvertType != INVERTER_TYPE_NOT_INSTALL)
        {
            PHY_INTER_TYPE ePort = pPV_Rough->stPVSettingParam[i].ePhyInterface;
            if (ePort < _MAX_PHY_INTER_TYPE)
            {
                bNeedPorts[ePort] = TRUE;
            }
        }
    }
    
    // 启动线程
    for (i = 0; i < _MAX_PHY_INTER_TYPE; i++)
    {
        if (bNeedPorts[i])
        {
            PORT_THREAD_INFO* pPortInfo = &pPV_Rough->portThreadInfo[i];
            
            if (pPortInfo->bThreadRunning)
            {
                TRACEW("Port[%d] thread already running\n", i);
                continue;
            }
            
            pPortInfo->ePortType = (PHY_INTER_TYPE)i;
            pPortInfo->pPvRoughData = pPV_Rough;
            pPortInfo->bThreadShouldExit = FALSE;
            
            // 初始化线程数据互斥锁
            pthread_mutex_init(&pPortInfo->dataMutex, NULL);
            
            // 创建线程
            int ret = pthread_create(&pPortInfo->threadId, NULL, Port_Sample_Thread, pPortInfo);
            if (ret != 0)
            {
                TRACEE("Failed to create thread for port[%d], error=%d\n", i, ret);
                pthread_mutex_destroy(&pPortInfo->dataMutex);
                return FALSE;
            }
            
            // 设置线程为分离状态（自动回收资源）
            pthread_detach(pPortInfo->threadId);
            
            // printf("Port[%d] thread created successfully, TID=%lu\n", i, pPortInfo->threadId);
            TRACEI("Port[%d] thread created successfully, TID=%lu\n", i, pPortInfo->threadId);
        }
    }
    
    return TRUE;
}

// 设置PV命令
BOOL setPVCmd(ST_PV_ROUGHDATA* pPvInverterSample, ENU_PV_CMD eCmd, SHORT* usCmdData, UINT inwhich)
{
    BOOL bRet = FALSE;
	// printf("setPVCmd[%d]cmd[%d] =  [%d]\n", inwhich, eCmd, *usCmdData);
    
    if (pPvInverterSample->stPVSettingParam[inwhich].eInvertType == INVERTER_TYPE_NOT_INSTALL ||
        pPvInverterSample->stPVRunData[inwhich].blobSinglePVdata.PVInverterData.iWorkMode != PV_STATUS_RUNNING)
    {
        return bRet;
    }
    
    PHY_INTER_TYPE ePortType = pPvInverterSample->stPVSettingParam[inwhich].ePhyInterface;
    PORT_THREAD_INFO* pPortInfo = Get_Port_By_Type(pPvInverterSample, ePortType);
    
    if (pPortInfo == NULL || !pPortInfo->bIsOpened)
    {
        TRACEE("Port not opened for command to inverter[%d]\n", inwhich);
        return FALSE;
    }
    
    // 加锁防止与采样线程冲突
    pthread_mutex_lock(&pPortInfo->dataMutex);
    
    setCommBaudRate(pPvInverterSample, inwhich);
    Sleep(10);
    
    if (INVERTER_TYPE_Growatt == pPvInverterSample->stPVSettingParam[inwhich].eInvertType)
    {
        bRet = setPVCmdGrowatt(pPvInverterSample, eCmd, usCmdData, inwhich);
    }
    else if (INVERTER_TYPE_Huawei == pPvInverterSample->stPVSettingParam[inwhich].eInvertType)
    {
        bRet = setPVCmdHuawei(pPvInverterSample, eCmd, usCmdData, inwhich);
    }
    else if (INVERTER_TYPE_Ginlong == pPvInverterSample->stPVSettingParam[inwhich].eInvertType)
    {
        bRet = setPVCmdGinlong(pPvInverterSample, eCmd, usCmdData, inwhich);
    }
    else if (INVERTER_TYPE_Sungrow == pPvInverterSample->stPVSettingParam[inwhich].eInvertType)
    {
        bRet = setPVCmdSungrow(pPvInverterSample, eCmd, usCmdData, inwhich);
    }
    
    pthread_mutex_unlock(&pPortInfo->dataMutex);
    
    return bRet;
}

// 初始化PV数据
int Init_Pv_InverteData(ST_PV_ROUGHDATA *pPV_Rough)
{
	// printf("Init_Pv_InverteData=================\n");
    int n;
    int kkk;
    VAR_VALUE varV;
    SAMPLER* pSampler = (SAMPLER*)pPV_Rough->hSampler;
    DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
    
    // 读取串口服务器配置
    varV.blobValue = (BYTE *)pPV_Rough->uartServerCfg.szIP;
    pDPR->Get(pSampler, GETTYPE_ST_VAR, 901, 921, &varV, 64);  // IP地址
    
    pDPR->Get(pSampler, GETTYPE_ST_VAR, 901, 922, &varV, 4);   // 起始端口
    pPV_Rough->uartServerCfg.iStartPort = varV.ulValue;
    
	// printf("Read UartServer Config: IP=%s, StartPort=%d\n", 
    //        pPV_Rough->uartServerCfg.szIP, 
    //        pPV_Rough->uartServerCfg.iStartPort);

	// strcpy(pPV_Rough->uartServerCfg.szIP, "192.168.1.201");
	// pPV_Rough->uartServerCfg.iStartPort = 8899;
    TRACEI("UartServer Config: IP=%s, StartPort=%d\n", 
           pPV_Rough->uartServerCfg.szIP, 
           pPV_Rough->uartServerCfg.iStartPort);
	// printf("Set UartServer Config: IP=%s, StartPort=%d\n", 
    //        pPV_Rough->uartServerCfg.szIP, 
    //        pPV_Rough->uartServerCfg.iStartPort);
    
    // 读取每个逆变器的配置
    for (kkk = 0; kkk < MAX_PVInverter_NUM; kkk++)
    {
        pDPR->Get(pPV_Rough->hSampler, GETTYPE_ST_VAR, 1051 + kkk, 453, &varV, 4);
        pPV_Rough->stPVSettingParam[kkk].eInvertType = (ENU_INVERTER_TYPE)varV.emValue;
        
        pDPR->Get(pPV_Rough->hSampler, GETTYPE_ST_VAR, 1051 + kkk, 457, &varV, 4);
        pPV_Rough->stPVSettingParam[kkk].eBaudRate = (ENU_INVERTER_BAUD_RATE)varV.emValue;
        
        pDPR->Get(pPV_Rough->hSampler, GETTYPE_ST_VAR, 1051 + kkk, 454, &varV, 4);
        pPV_Rough->stPVSettingParam[kkk].iAddr = varV.lValue;
        
        pPV_Rough->pDPR->Get(pPV_Rough->hSampler, GETTYPE_ST_VAR, 1051 + kkk, 442, &varV, 4);
        pPV_Rough->stPVSettingParam[kkk].iRatedPower = varV.lValue;
        
        pDPR->Get(pPV_Rough->hSampler, GETTYPE_ST_VAR, 1051 + kkk, 456, &varV, 4);
        pPV_Rough->stPVSettingParam[kkk].nGroupNum = varV.emValue;
        
        pDPR->Get(pPV_Rough->hSampler, GETTYPE_ST_VAR, 1051 + kkk, 455, &varV, 4);
        pPV_Rough->stPVSettingParam[kkk].ePhyInterface = (PHY_INTER_TYPE)varV.emValue;
        
        TRACEI("PV[%d]: Type=%d, BaudRate=%d, Addr=%d, PhyPort=%d\n",
               kkk,
               pPV_Rough->stPVSettingParam[kkk].eInvertType,
               pPV_Rough->stPVSettingParam[kkk].eBaudRate,
               pPV_Rough->stPVSettingParam[kkk].iAddr,
               pPV_Rough->stPVSettingParam[kkk].ePhyInterface);
		// printf("PV[%d]: Type=%d, BaudRate=%d, Addr=%d, PhyPort=%d\n",
        //        kkk,
        //        pPV_Rough->stPVSettingParam[kkk].eInvertType,
        //        pPV_Rough->stPVSettingParam[kkk].eBaudRate,
        //        pPV_Rough->stPVSettingParam[kkk].iAddr,
        //        pPV_Rough->stPVSettingParam[kkk].ePhyInterface);
    }
    
    pDPR->Get(pPV_Rough->hSampler, GETTYPE_ST_VAR, 1050, 439, &varV, 4);
    pPV_Rough->bEnableManual = varV.emValue;
    
    pDPR->Get(pPV_Rough->hSampler, GETTYPE_ST_VAR, 1050, 440, &varV, 4);
    pPV_Rough->emPvDataAcquisitonTypes = (PV_SATA_ACQUISITON_TYPES)varV.emValue;
    
#ifdef IMMU2
    pPV_Rough->blobPVTotalinfo.emDIstatus = DI_STATUS_RECYCLE;
    TRACEI("Get_IMMU2_DI = [%d]\n", Get_IMMU2_DI(IMMU2_DI1));
#endif
    return TRUE;
}

void Push_AllData(ST_PV_ROUGHDATA* pPvInverterSample)
{
	int n;
	int kkk;
	VAR_VALUE varV;
	SAMPLER* pSampler = (SAMPLER*)pPvInverterSample->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	for( kkk = 0; kkk < MAX_PVInverter_NUM; kkk++)
	{
		if(pPvInverterSample->stPVSettingParam[kkk].eInvertType == INVERTER_TYPE_NOT_INSTALL)
		{
			continue;
		}

		//alarm
		if (pPvInverterSample->stPVRunData[kkk].bIsCommFail)
		{
			ZERO_POBJS(&pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData, 1);
		}
		// printf("Push_AllData PV[%d]bIsCommFail = [%d]\n", kkk, pPvInverterSample->stPVRunData[kkk].bIsCommFail);
		varV.emValue = pPvInverterSample->stPVRunData[kkk].bIsCommFail;
		Push_VarValue(SETTYPE_RAW_SP, pSampler, 1051 + kkk, 3750, varV);

		//single running data
		A_BLOB_PV_INVERTER_DATA blobPVData;
		blobPVData.emBlobType = BLOB_TYPE_PV_INVERTER_DATA;
		// printf("rated power[%d] = [%d]\n", kkk, pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData.iRatedPower);
		memcpy(&blobPVData.PVInverterData, &pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData, sizeof(PV_INVERTER_DATA));
		Push_BlobValue(SETTYPE_RAW_SP, pPvInverterSample->hSampler, 1051 + kkk, 3699, (void*)&blobPVData, sizeof(A_BLOB_PV_INVERTER_DATA), BLOB_TYPE_PV_INVERTER_DATA);

	}

	// Total PV info
	Push_BlobValue(SETTYPE_RAW_SP, pPvInverterSample->hSampler, 1050, 3697, 
		(void*)&pPvInverterSample->blobPVTotalinfo, sizeof(A_BLOB_PV_INVERTER_DATA_TOTAL), BLOB_TYPE_PV_INVERTER_DATA_TOTAL);

	varV.emValue = pPvInverterSample->bDOSwitchStatus;
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 1050, 3751, varV);

}


void Refresh_PV_Toatl_Info(ST_PV_ROUGHDATA* pPvInverterSample)
{
	UINT 	kkk = 0;
	UINT	i = 0;
	float 	fPVSingleStrPower = 0.0;
	float 	fPVTotalStrPower = 0.0;
	// UINT	iActRateTmep = 0;
	// UINT 	iRatedPowerTemp = 0;
	float 	fTotalPVPowerTemp = 0.0;
	float 	fTotalPVInputTemp = 0.0;				
	float 	fTotalPVOutputActPowerTemp = 0.0;			
	float 	fDailyPowerGenerationSumTemp = 0.0;		
	float 	fTotalPowerGenerationSumTemp = 0.0;		
	float 	fPVDCTotalPowerGenerationSumTemp = 0.0;   
	UINT 	iTotalRatedPowerTemp = 0;
    UINT 	nInstalled = 0;
    UINT 	nRunning = 0;
    UINT 	nWaiting = 0;
    UINT 	nFault = 0;
	VAR_VALUE varV;
	SAMPLER* pSampler = (SAMPLER*)pPvInverterSample->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	for(kkk = 0; kkk < MAX_PVInverter_NUM; kkk++)
	{
		if(pPvInverterSample->stPVSettingParam[kkk].eInvertType == INVERTER_TYPE_NOT_INSTALL)
		{
			continue;
		}
		nInstalled ++;

		PV_INVERTER_DATA * PVData = &pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData;

		if (PVData->iWorkMode == PV_STATUS_RUNNING)
		{
			nRunning++;
		}
		else if (PVData->iWorkMode == PV_STATUS_WAITING)
		{
			nWaiting++;
		}
		else if (PVData->iWorkMode == PV_STATUS_FAULT || PVData->iWorkMode == PV_STATUS_NOTINSTALL)
		{
			nFault++;
			continue;
		}

		//单个逆变器 总PV组 电压 * 电流
		// fPVTotalStrPower = 0.0;
		// for (i = 0; i < MAX_PV_NUM; i++)
		// {
		// 	fPVSingleStrPower = PVData->fPVGroupVolt[i] * PVData->fPVGroupCurr[i];
		// 	fPVTotalStrPower += fPVSingleStrPower;
		// }
		// PVData->fRealTime_MaxActivePower = fPVTotalStrPower;	//W
		// iActRateTmep = PVData->iActivePowerRate;
		// iRatedPowerTemp = PVData->iRatedPower;  		//锦浪可能读不到额定功率，待确认
		// PVData->fRealTime_MaxActivePower = iActRateTmep * iRatedPowerTemp * 1.1 * 0.01;    //当前最大交流输出 = 逆变器最大输出百分比  * 额定功率 * 1.1

		//运行状态，上报额定功率
		PVData->fRealTime_MaxActivePower = (PVData->iWorkMode == PV_STATUS_RUNNING) ? PVData->iRatedPower : 0;
		fTotalPVPowerTemp += PVData->fRealTime_MaxActivePower;

		//光伏直流输入总功率
		fTotalPVInputTemp += PVData->fPVInputTotalPower;

		//逆变器当前输出交流有功功率 KW
		fTotalPVOutputActPowerTemp +=  PVData->fOutputActivePower;

		//当日发电量 kWh
		fDailyPowerGenerationSumTemp += PVData->fDailyPowerGeneration;

		//总共发电量 kWh
		fTotalPowerGenerationSumTemp += PVData->fTotalPowerGeneration;

		//直流输入总电量kWh
		fPVDCTotalPowerGenerationSumTemp += PVData->fPVDCTotalPowerGeneration;

		//额定功率 kw
		if (PVData->iWorkMode == PV_STATUS_RUNNING && PVData->iRatedPower > 0)
		iTotalRatedPowerTemp += PVData->iRatedPower;

	}

	
    if (nInstalled == 0) 	//全部未安装
	{
		pPvInverterSample->blobPVTotalinfo.emPvRunStatus = PV_STATUS_NOTINSTALL;
	}
    else if (nFault == nInstalled) //所有已安装的逆变器都故障
    {
		pPvInverterSample->blobPVTotalinfo.emPvRunStatus = PV_STATUS_FAULT;
	}
    else if (nRunning > 0) 	//有运行的优先判为运行
    {
		pPvInverterSample->blobPVTotalinfo.emPvRunStatus = PV_STATUS_RUNNING;
	}
    else if (nWaiting > 0) 	//有等待但无运行，则为等待
    {
		pPvInverterSample->blobPVTotalinfo.emPvRunStatus = PV_STATUS_WAITING;
	}

	// pPvInverterSample->blobPVTotalinfo.nAvailablePVNums = 				nInstalled - nFault;
	pPvInverterSample->blobPVTotalinfo.nAvailablePVNums = 				nRunning;
	pPvInverterSample->blobPVTotalinfo.fTotalPVPowerAbility = 			fTotalPVPowerTemp * 0.001; //kw
	pPvInverterSample->blobPVTotalinfo.fTotalPVInput = 					fTotalPVInputTemp * 0.001; //kw
	pPvInverterSample->blobPVTotalinfo.fDailyPowerGenerationSum = 		fDailyPowerGenerationSumTemp; 
	pPvInverterSample->blobPVTotalinfo.fTotalPowerGenerationSum = 		fTotalPowerGenerationSumTemp;
	pPvInverterSample->blobPVTotalinfo.fPVDCTotalPowerGenerationSum = 	fPVDCTotalPowerGenerationSumTemp;
	pPvInverterSample->blobPVTotalinfo.iTotalRatedPower = 				iTotalRatedPowerTemp * 0.001; //kw
	pPvInverterSample->blobPVTotalinfo.bEnableManualMode = 				pPvInverterSample->bEnableManual;

	//F电表信息   安装了电表且没有告警就读取电表信息；否则累加上报
	pDPR->Get(pSampler, GETTYPE_ALMSTATE, 406, 401, &varV, 4);
	BOOL bAlarmStatus = varV.emValue;

	pDPR->Get(pSampler, GETTYPE_ST_VAR, 406, 401, &varV, 4);
	BOOL bInstalled = varV.emValue;

	if (bInstalled == METER_TYPE_NotInstalled || bAlarmStatus != ALARM_STATUS_CEASED)
	{
		pPvInverterSample->blobPVTotalinfo.fTotalPVOutputActPower = 		fTotalPVOutputActPowerTemp * 0.001; //kw
	}
	else
	{
		pDPR->Get(pSampler, GETTYPE_SP_VAR, 406, 400, &varV, 4); 		//PV Meter Active Power
        pPvInverterSample->blobPVTotalinfo.fTotalPVOutputActPower = varV.fValue;   //KW
	}
}

//bRecycle = true (DO高电平,DI高)回收控制权  False (DO低电平，DI低)让出控制权
BOOL DO_Ctrl(ST_PV_ROUGHDATA* pPvInverterSample, BOOL bRecycle)
{
	UINT 	kkk = 0;
	UINT 	nPVCtrlReadTimes = 3;
	BOOL 	bPVCtrl_DIStatus = 0;
	UINT	nRet = FALSE;

	if (bRecycle)
	{
		Set_IMMU_DO(IOC_SET_DO1, 1);
		//read back for 3 times
		for(kkk = 0; kkk < nPVCtrlReadTimes; kkk++)
		{
#ifdef IMMU2
			bPVCtrl_DIStatus = Get_IMMU2_DI(IMMU2_DI1);
#endif
			if(bPVCtrl_DIStatus == 1)
			{
				break;
			}
		}
		
		if(bPVCtrl_DIStatus == 1)
		{
			nRet = TRUE;
		}
	}
	else
	{
		Set_IMMU_DO(IOC_SET_DO1, 0);
		//read back for 3 times
		for(kkk = 0; kkk < nPVCtrlReadTimes; kkk++)
		{
#ifdef IMMU2
			bPVCtrl_DIStatus = Get_IMMU2_DI(IMMU2_DI1);
#endif
			if(bPVCtrl_DIStatus == 0)
			{
				break;
			}
		}
		
		if(bPVCtrl_DIStatus == 0)
		{
			nRet = TRUE;
		}
	}

	if (nRet == TRUE)
	{
		pPvInverterSample->blobPVTotalinfo.emDIstatus = (bPVCtrl_DIStatus == 1) ? DI_STATUS_RECYCLE : DI_STATUS_TRANSFER;
		TRACED("DO_Ctrl-blobPVTotalinfo.emDIstatus = [%d]\n",pPvInverterSample->blobPVTotalinfo.emDIstatus);
	}

	return nRet;
}

BOOL DisPatch_PV_Output(ST_PV_ROUGHDATA* pPvInverterSample)
{
	int kkk = 0;
	int nActivePercent = 0;
	int nReActivePercent = 0;
	float fActiveAverage = 0.0;
	float fReActiveAverage = 0.0;
	SHORT sValue;

	if (pPvInverterSample->bEnableManual == TRUE)
	{
		return TRUE;
	}

	A_BLOB_PV_INVERTER_DATA_TOTAL * PVTotalinfo = &pPvInverterSample->blobPVTotalinfo;

	if (pPvInverterSample->emPvDataAcquisitonTypes != PV_DATA_ACQUISITION_DO_SWITCH)
	{
		pPvInverterSample->bDOSwitchStatus = 0;
	}

	// printf("RecvCtrlMode = [%d]\n", PVTotalinfo->RecvCtrlMode);
	// PVTotalinfo->RecvCtrlMode = CTRL_BY_AEMS;
	if (PVTotalinfo->RecvCtrlMode == CTRL_BY_DATA_ACQUISITION) //数采对接协议 开启防逆流模式 下发总功率
	{
		TRACEI("CtrlMode = CTRL_BY_DATA_ACQUISITION\n");
		// printf("CtrlMode = CTRL_BY_DATA_ACQUISITION\n");
		// printf("emPvDataAcquisitonTypes = [%d]\n", pPvInverterSample->emPvDataAcquisitonTypes);
		if(pPvInverterSample->emPvDataAcquisitonTypes == PV_DATA_ACQUISITION_NOT_INSTALLED) //不限制最大功率输出 255
		{
			TRACEI("DisPatch_PV_Output-NOT INSTALLED 255 NO LIMIT\n");
			// printf("DisPatch_PV_Output-NOT INSTALLED 255 NO LIMIT\n");
			for(kkk = 0; kkk < MAX_PVInverter_NUM; kkk++)
			{
				// printf("eInvertType[%d] == NULL = [%d], iWorkMode[%d] == NULL = [%d]\n",
				// 	kkk, pPvInverterSample->stPVSettingParam[kkk].eInvertType == NULL,
				// 	kkk, pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData.iWorkMode == NULL);
				// printf("eInvertType[%d] = [%d], iWorkMode[%d] = [%d]\n",
				// 	kkk, pPvInverterSample->stPVSettingParam[kkk].eInvertType,
				// 	kkk, pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData.iWorkMode);
				if(pPvInverterSample->stPVSettingParam[kkk].eInvertType == INVERTER_TYPE_NOT_INSTALL ||
					pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData.iWorkMode != PV_STATUS_RUNNING)
				{
					// printf("skip [%d]\n", kkk);
					continue;
				}

				if (pPvInverterSample->stPVSettingParam[kkk].eInvertType == INVERTER_TYPE_Growatt)
				{
					sValue = TRUE;
					if (!setPVCmd(pPvInverterSample, PV_CMD_AntiFlux_Enable, &sValue,kkk))
					{

						TRACEE("PV_CMD_AntiFlux_Enable to [%d]Failed !\n",kkk);
						return FALSE;
					}
				}

				
				if (!setPVCmd(pPvInverterSample, PV_CMD_ActiveOutputPower_NoLimit, NULL,kkk))
				{
					// printf("PV_CMD_ActiveOutputPower_NoLimit to [%d]Failed !\n",kkk);
					TRACEE("PV_CMD_ActiveOutputPower_NoLimit to [%d]Failed !\n",kkk);
					// return FALSE;
					continue;
				}
				
				// if (pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData.nReActivePowerPercen != 100)
				// {
				// 	if (!setPVCmd(pPvInverterSample, PV_CMD_ReActiveOutputPower_NoLimit, NULL,kkk))
				// 	{
				// 		TRACEE("PV_CMD_ReActiveOutputPower_NoLimit to [%d] Failed !\n",kkk);
				// 		return FALSE;
				// 	}
				// }
			}
		}
		else if(pPvInverterSample->emPvDataAcquisitonTypes == PV_DATA_ACQUISITION_DO_SWITCH)
		{
			TRACEI("DisPatch_PV_Output-DO CTRL\n");
			// if (pPvInverterSample->blobPVTotalinfo.emDIstatus == DI_STATUS_RECYCLE)
			{
				//让出控制权
				if (!DO_Ctrl(pPvInverterSample,FALSE))
				{
					pPvInverterSample->bDOSwitchStatus = 1;
					TRACEE("Give up Ctrl Failed !!!");
				}
				else
				{
					pPvInverterSample->bDOSwitchStatus = 0;
				}
			}

		}

		return TRUE;
	}
	else
	{
		if(pPvInverterSample->emPvDataAcquisitonTypes == PV_DATA_ACQUISITION_DO_SWITCH)
		{
			TRACEI("DisPatch_PV_Output-DO CTRL\n");
			// if (pPvInverterSample->blobPVTotalinfo.emDIstatus == DI_STATUS_TRANSFER)
			{
				//回收控制权
				if (!DO_Ctrl(pPvInverterSample,TRUE))
				{
					pPvInverterSample->bDOSwitchStatus = 1;
					TRACEE("Recycle Ctrl Failed !!!");
				}
				else
				{
					pPvInverterSample->bDOSwitchStatus = 0;
				}	
			}
		}
	}

	// printf("CtrlMode = CTRL_BY_AEMS+-+-+-+-+-+-+-\n");
	TRACEI("CtrlMode = CTRL_BY_AEMS\n");
	//平均下发
	// PVTotalinfo->fRecvActivePower = 30;
	fActiveAverage = PVTotalinfo->fRecvActivePower / PVTotalinfo->nAvailablePVNums;
	// fReActiveAverage = PVTotalinfo->fRecvReActivePower / PVTotalinfo->nAvailablePVNums;
	//按额定容量比例分配
	//按直流可用功率分配：按每台当前直流可用功率（或各 MPPT 可用功）分配，能保证不下发超过 PV 可供的功率
	//优先级/保序分配：给某台逆变器设优先级，先满足高优先级。

	for(kkk = 0; kkk < MAX_PVInverter_NUM; kkk++) 	//单独下发每一个逆变器模块
	{
		if(pPvInverterSample->stPVSettingParam[kkk].eInvertType == INVERTER_TYPE_NOT_INSTALL ||
			pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData.iWorkMode != PV_STATUS_RUNNING)
		{
			continue;
		}

		// if(PVTotalinfo->nRecvPVStatus != pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData.nOnOffStatus) //开关机状态
		// {
		// 	if (!setPVCmd(pPvInverterSample, (ENU_PV_CMD)PVTotalinfo->nRecvPVStatus, NULL,kkk))
		// 	{
		// 		TRACEE("OnOff to [%d] cmd[%d] Failed !\n",kkk,PVTotalinfo->nRecvPVStatus);
		// 		return FALSE;
		// 	}
		// }

		if (pPvInverterSample->stPVSettingParam[kkk].eInvertType == INVERTER_TYPE_Growatt)
		{
			sValue = TRUE;
			if (!setPVCmd(pPvInverterSample, PV_CMD_AntiFlux_Enable, &sValue, kkk))
			{
				// printf("Cause Growatt AntiFlux_Enable Failed !\n");
				TRACEE("PV_CMD_AntiFlux_Enable to [%d]Failed !\n",kkk);
				return FALSE;
			}
		}

		//转化成百分比
		int rated = pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData.iRatedPower * 0.001; //KW
		if (rated <= 0)
		{
			// printf("Cause rated power[%d] <= 0 !\n", kkk);
			return FALSE;
		}

		nActivePercent = (int)(fActiveAverage * 100.0f / rated);
		// printf("[Ctrl_PV_Output] ActivePercent[%d] = [%d]\n", kkk, nActivePercent);
		if (nActivePercent > 100)
		{
			nActivePercent = 100;
		}

		// nReActivePercent = (int)(fReActiveAverage * 100.0f / rated);
		// if (nReActivePercent > 100 || nReActivePercent < -100)
		// {
		// 	nReActivePercent = 100;
		// }
		// printf("nParallelAntiBackflowPowerLimit[%d] = [%d]\n", kkk, pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData.nParallelAntiBackflowPowerLimit);
		if (nActivePercent <= pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData.nParallelAntiBackflowPowerLimit)		//有功功率
		// if (ABS(nActivePercent - pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData.nParallelAntiBackflowPowerLimit) > 2)		//有功功率
		{
			sValue = (SHORT)PVTotalinfo->fRecvActivePower;
			if (!setPVCmd(pPvInverterSample, PV_CMD_ActiveOutputPower, &sValue,kkk))
			{
				// printf("Cause ActiveOutputPower[%d] != [%d] !\n", kkk, nActivePercent);
				TRACEE("ActiveOutputPower to [%d] VALUE[%hd] Failed !\n",kkk,sValue);
				// return FALSE;
				continue;
			}
		}
#ifdef LOG_FILEDEBUG
		static time_t stmLogFile;
		if (ElapseS(stmLogFile) > INTV_S_LOG_DEBUGFILE)
		{
			stmLogFile = _NOW_;
			LOGFD("pv pv_inverter_main:DisPatch_PV_Output kkk:%d nActivePercent:%d nParallelAntiBackflowPowerLimit:%u iWorkMode:%d",
				kkk, nActivePercent, pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData.nParallelAntiBackflowPowerLimit,
				pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData.iWorkMode);
		}
#endif

		// if (ABS(nReActivePercent - pPvInverterSample->stPVRunData[kkk].blobSinglePVdata.PVInverterData.nReActivePowerPercen) > 2)	//无功功率
		// {
		// 	sValue = (SHORT)PVTotalinfo->fRecvReActivePower;
		// 	if (!setPVCmd(pPvInverterSample, PV_CMD_ReActiveOutputPower, &sValue,kkk))
		// 	{
		// 		TRACEE("ReActiveOutputPower to [%d] VALUE[%hd] Failed !\n",kkk,sValue);
		// 		return FALSE;
		// 	}
		// }
		
	}

	return TRUE;
	
}

static UINT s_AllPvCommFailCnt = 0;

BOOL IsNeedReopenPort(ST_PV_ROUGHDATA* pPvInverterSample)
{
	BOOL bAllCommFail = TRUE;
	BOOL bHasInstalled = FALSE;
	int kkk;

	for (kkk = 0; kkk < MAX_PVInverter_NUM; kkk++)
	{
		if (pPvInverterSample->stPVSettingParam[kkk].eInvertType == INVERTER_TYPE_NOT_INSTALL)
		{
			continue;
		}

		bHasInstalled = TRUE;

		if (pPvInverterSample->stPVRunData[kkk].bIsCommFail == FALSE)
		{
			bAllCommFail = FALSE;
			break;
		}
	}

	if (bHasInstalled && bAllCommFail)
	{
		s_AllPvCommFailCnt++;
		TRACEW("s_AllPvCommFailCnt[%d]\n",s_AllPvCommFailCnt);
		if (s_AllPvCommFailCnt >= ALL_PV_COMM_FAIL_REOPEN_CNT)
		{
			TRACEE("All PV inverter comm failed %d times, reopen port!\n",
					s_AllPvCommFailCnt);

			pPvInverterSample->bCommStatusOk = FALSE;
			s_AllPvCommFailCnt = 0;
		}
	}
	else
	{
		s_AllPvCommFailCnt = 0;
	}

	return TRUE;
}

// 采样函数（主循环）- 现在只负责数据汇总和调度
DLLExport int Sample(HANDLE hSamplerThis)
{
	// printf(" i am pv-inverter Sample.......\n");
    SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
    ST_PV_ROUGHDATA* pPvInverterSample = (ST_PV_ROUGHDATA*)pSampler->pDataHeap;
	HANDLE hSamplerLock = pSampler->hSamplerLock;
    
    // 线程已经在后台采样，这里只做数据汇总和调度
    
	if (Mutex_Lock(hSamplerLock, 200) == ERR_MUTEX_OK)
    {
		// printf("refresh pv-inverter total info...<><><><><><>\n");
        // 汇总总信息
		Refresh_PV_Toatl_Info(pPvInverterSample);
		// 分配调度功率
        DisPatch_PV_Output(pPvInverterSample);
		// 推送数据到数据库
		Push_AllData(pPvInverterSample);
        Mutex_Unlock(hSamplerLock);
    }
	else
	{
		// printf("Mutex_Lock Failed !\n");
		TRACEE("Mutex_Lock Failed !\n");
	}
    
    // 判断是否需要重开端口（可以检查线程状态）
    IsNeedReopenPort(pPvInverterSample);
	sleep(1);
    
    return ERR_SAMPLE_OK;
}

DLLExport int Control(HANDLE hSamplerThis , PARAM_TYPE	emParaType,
		int			nDevId,
		int			nParamId,
		VAR_VALUE	*pVarVal,
		int			nValSize,
		int			nTimeOut)
{
	TRACEW(" i am pv-inverter Control....\n");
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
    ST_PV_ROUGHDATA* pPvInverterControl = (ST_PV_ROUGHDATA*)pSampler->pDataHeap;
	HANDLE hSamplerLock = ((SAMPLER*)pPvInverterControl->hSampler)->hSamplerLock;
	int nRet = ERR_CONTROL_OK;
	int kkk = 0;
	VAR_VALUE varV;

	if(Mutex_Lock(hSamplerLock, nTimeOut) != ERR_MUTEX_OK)
	{
		return ERR_CONTROL_TIMEOUT;
	}

	if(emParaType == PARAM_TYPE_SETTING)   //settings
	{
		TRACEW(" i am pv-inverter setting....\n");
		if(nParamId == 453)
		{
			int nWhichWh = (nDevId - 1051);
			if((nWhichWh < 0) || (nWhichWh >= MAX_PVInverter_NUM))
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			nRet = ERR_CONTROL_OK;
			pPvInverterControl->stPVSettingParam[nWhichWh].eInvertType = (ENU_INVERTER_TYPE)pVarVal->emValue;
		}
		else if (nParamId == 457)
		{
			int nWhichWh = (nDevId - 1051);
			if((nWhichWh < 0) || (nWhichWh >= MAX_PVInverter_NUM))
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			nRet = ERR_CONTROL_OK;
			pPvInverterControl->stPVSettingParam[nWhichWh].eBaudRate = (ENU_INVERTER_BAUD_RATE)pVarVal->emValue;
			
		}
		else if (nParamId == 454)
		{
			int nWhichWh = (nDevId - 1051);
			if((nWhichWh < 0) || (nWhichWh >= MAX_PVInverter_NUM))
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			nRet = ERR_CONTROL_OK;
			pPvInverterControl->stPVSettingParam[nWhichWh].iAddr = pVarVal->lValue;
			
		}
		else if (nParamId == 442)
		{
			int nWhichWh = (nDevId - 1051);
			if((nWhichWh < 0) || (nWhichWh >= MAX_PVInverter_NUM))
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			nRet = ERR_CONTROL_OK;
			pPvInverterControl->stPVSettingParam[nWhichWh].iRatedPower = pVarVal->lValue;
			
		}
		else if (nParamId == 456)
		{
			int nWhichWh = (nDevId - 1051);
			if((nWhichWh < 0) || (nWhichWh >= MAX_PVInverter_NUM))
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			nRet = ERR_CONTROL_OK;
			pPvInverterControl->stPVSettingParam[nWhichWh].nGroupNum = pVarVal->emValue;
		}
		else if (nParamId == 455)
		{
			int nWhichWh = (nDevId - 1051);
			if((nWhichWh < 0) || (nWhichWh >= MAX_PVInverter_NUM))
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			nRet = ERR_CONTROL_OK;
			pPvInverterControl->stPVSettingParam[nWhichWh].ePhyInterface = (PHY_INTER_TYPE)pVarVal->emValue;
			
		}
		else if (nParamId == 440) //本地数采控制安装类型
		{
			TRACEI("nDevId = %d\n",nDevId);
			if (nDevId != 1050)
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			nRet = ERR_CONTROL_OK;
			pPvInverterControl->emPvDataAcquisitonTypes = (PV_SATA_ACQUISITON_TYPES)pVarVal->emValue;
			TRACEI("[%d] DataAcquisition Types [%d]\n",nDevId,pPvInverterControl->emPvDataAcquisitonTypes);
		}
		else if (nParamId == 438)	//OnOff
		{
			TRACEI("nDevId = %d\n",nDevId);
			if (nDevId == 1050)
			{
				if (!setAllPVCmd(pPvInverterControl, (ENU_PV_CMD)pVarVal->emValue, NULL))
				{
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			else
			{
				int nWhichWh = (nDevId - 1051);
				TRACEI("[%d]  ONOFF[%d] \n",nWhichWh,pVarVal->emValue);
				if (!setPVCmd(pPvInverterControl, (ENU_PV_CMD)pVarVal->emValue, NULL,nWhichWh))
				{
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			nRet = ERR_CONTROL_OK;
		}
		else if (nParamId == 434)	//多机防逆流使能
		{
			TRACED("306-nDevId = %d\n",nDevId);
			SHORT enable = (SHORT)pVarVal->emValue;
			if (nDevId == 1050)
			{
				if (!setAllPVCmd(pPvInverterControl, PV_CMD_AntiFlux_Enable, &enable))
				{
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			else
			{
				int nWhichWh = (nDevId - 1051);
				TRACED("[%d]  Multi_AntiFlux_enable[%d] \n",nWhichWh,pVarVal->emValue);
				if (!setPVCmd(pPvInverterControl, PV_CMD_AntiFlux_Enable, &enable,nWhichWh))
				{
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			nRet = ERR_CONTROL_OK;
		}
		else if (nParamId == 435)	//多机防逆流功率限制值设置
		{
			TRACEI("308-nDevId = %d\n",nDevId);
			SHORT outputPower = (SHORT)pVarVal->fValue;
			if (nDevId == 1050)
			{
				if (!setAllPVCmd(pPvInverterControl, PV_CMD_ActiveOutputPower, &outputPower))
				{
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			else
			{
				int nWhichWh = (nDevId - 1051);
				TRACEI("[%d]pVarVal->fValue=%0.1f short_outputPower = %hd \n",nWhichWh,pVarVal->fValue,outputPower);
				if (!setPVCmd(pPvInverterControl, PV_CMD_ActiveOutputPower, &outputPower,nWhichWh))
				{
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			nRet = ERR_CONTROL_OK;
		}
		else if (nParamId == 436)	//逆变器最大输出有功功率百分比 03-0x03
		{
			TRACEI("0x03-nDevId = %d\n",nDevId);
			SHORT outputPower = (SHORT)pVarVal->fValue;
			if (nDevId == 1050)
			{
				if (!setAllPVCmd(pPvInverterControl, PV_CMD_MaxActivePowerRate, &outputPower))
				{
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			else
			{
				int nWhichWh = (nDevId - 1051);
				TRACEI("[%d]pVarVal->fValue=%0.1f short_outputPower = %hd \n",nWhichWh,pVarVal->fValue,outputPower);
				if (!setPVCmd(pPvInverterControl, PV_CMD_MaxActivePowerRate, &outputPower,nWhichWh))
				{
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			nRet = ERR_CONTROL_OK;
		}
		else if (nParamId == 437)	//无功功率
		{
			TRACEI("nDevId = %d\n",nDevId);
			SHORT ReOutputPower = (SHORT)pVarVal->fValue;
			if (nDevId == 1050)
			{
				if (!setAllPVCmd(pPvInverterControl, PV_CMD_ReActiveOutputPower, &ReOutputPower))
				{
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			else
			{
				int nWhichWh = (nDevId - 1051);
				TRACEI("[%d]pVarVal->fValue=%0.1f short_ReOutputPower = %hd \n",nWhichWh,pVarVal->fValue,ReOutputPower);
				if (!setPVCmd(pPvInverterControl, PV_CMD_ReActiveOutputPower, &ReOutputPower,nWhichWh))
				{
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			nRet = ERR_CONTROL_OK;

		}
		else if (nParamId == 618)	// 功率控制斜率
		{
			SHORT rate = (SHORT)(pVarVal->fValue * 100);
			if (nDevId == 1050)
			{
				if (!setAllPVCmd(pPvInverterControl, PV_CMD_ActivePowerSlope, &rate))
				{
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			else
			{
				int nWhichWh = (nDevId - 1051);
				if (!setPVCmd(pPvInverterControl, PV_CMD_ActivePowerSlope, &rate,nWhichWh))
				{
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			nRet = ERR_CONTROL_OK;
		}
		else if (nParamId == 619)	// 掉电保存
		{
			SHORT enable = (SHORT)pVarVal->emValue;
			if (nDevId == 1050)
			{
				if (!setAllPVCmd(pPvInverterControl, PV_CMD_PoweroffSave, &enable))
				{
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			else
			{
				int nWhichWh = (nDevId - 1051);
				if (!setPVCmd(pPvInverterControl, PV_CMD_PoweroffSave, &enable, nWhichWh))
				{
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			nRet = ERR_CONTROL_OK;
		}
		else if ((nParamId == 439)) //PV手动控制模式使能 0-不使能 1-使能
		{
			TRACEI("[%d] CtrlMode=%d \n",nDevId,pVarVal->emValue);
			if (nDevId != 1050)
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			// if (pPvInverterControl->bEnableManual != pVarVal->emValue)
			{
				pPvInverterControl->bEnableManual = pVarVal->emValue;
			}

			for(kkk = 0; kkk < MAX_PVInverter_NUM; kkk++)
			{
				if(pPvInverterControl->stPVSettingParam[kkk].eInvertType == INVERTER_TYPE_NOT_INSTALL)
				{
					continue;
				}
				varV.emValue = pVarVal->emValue;
				Push_VarValue(SETTYPE_INTERN_SETT, pSampler, 1051 + kkk, 439, varV);
			}

			nRet = ERR_CONTROL_OK;
		}
	}
	else	//control
	{
		TRACEW(" i am pv-inverter control....\n");
		if (nParamId == 534)	// Ctrl by ASEMS 1050, 534,
		{
			if(nValSize != sizeof(A_BLOB_PV_CTRL_BY_AEMS) || (nDevId != 1050))
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			A_BLOB_PV_CTRL_BY_AEMS* pCmd = (A_BLOB_PV_CTRL_BY_AEMS*)pVarVal->blobValue;

			pPvInverterControl->blobPVTotalinfo.RecvCtrlMode = pCmd->CtrlMode;
			pPvInverterControl->blobPVTotalinfo.nRecvPVStatus = pCmd->nPVStatus;
			pPvInverterControl->blobPVTotalinfo.fRecvActivePower = pCmd->fActivePower;
			pPvInverterControl->blobPVTotalinfo.fRecvReActivePower = pCmd->fReActivePower;
			// printf("[%d] CtrlMode=[%d] nPVStatus=[%d] fActivePower=[%0.1f] fReActivePower=[%0.1f] \n",
			// 	nDevId,pCmd->CtrlMode,pCmd->nPVStatus,pCmd->fActivePower,pCmd->fReActivePower);

			nRet = ERR_CONTROL_OK;
		}
		else if (nParamId == 529)	//同步时间  
		{
			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_INFO, 
				"Received PV Inverter Time sync command, synchronizing system time to device...");
			
			time_t sysTime = (time_t)time(NULL); 
			struct tm* timeInfo = localtime(&sysTime);
			char timeStr[64];
			snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d", timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday, 
			timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
			
			SHORT itime[6];
			itime[0] = timeInfo->tm_year + 1900;
			itime[1] = timeInfo->tm_mon + 1;
			itime[2] = timeInfo->tm_mday;
			itime[3] = timeInfo->tm_hour;
			itime[4] = timeInfo->tm_min;
			itime[5] = timeInfo->tm_sec;

			if (nDevId == 1050)
			{
				if (!setAllPVCmd(pPvInverterControl, PV_CMD_TIMESYSCTR, itime))
				{
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			else
			{
				int nWhichWh = (nDevId - 1051);
				if (!setPVCmd(pPvInverterControl, PV_CMD_TIMESYSCTR, itime, nWhichWh))
				{
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}	
			}
			nRet = ERR_CONTROL_OK;
		}
		else if (nParamId == 532)	//DO切换控制
		{
			if(nDevId != 1050)
			{
				Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}

			if(pVarVal->emValue == DI_STATUS_RECYCLE)
			{
				//回收控制权
				if (!DO_Ctrl(pPvInverterControl,TRUE))
				{
					TRACEE("Recycle Ctrl Failed !!!");
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}

			}
			else
			{
				//让出控制权
				if (!DO_Ctrl(pPvInverterControl,FALSE))
				{
					TRACEE("Give up Ctrl Failed !!!");
					Mutex_Unlock(hSamplerLock);
					return ERR_CONTROL_FAILED;
				}

			}
			nRet = ERR_CONTROL_OK;
		}
	}
	
	Mutex_Unlock(hSamplerLock);
	return nRet;

}


DLLExport int JigTest(HANDLE hSamplerThis, void* pJigData)
{
	printf("i am uart server main----JigTest!!!!!!!!!!!\n");

	return ERR_CONTROL_OK;
}
