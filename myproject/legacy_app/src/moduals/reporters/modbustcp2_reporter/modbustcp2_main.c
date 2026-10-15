#include "../reporters_base.h"  
#include "modbus.h"
#include <errno.h>  
#include <pthread.h>
#include <signal.h> 

// ==================== 配置参数 ====================  
#define MODBUS_TCP_PORT         503  
#define MODBUS_TCP_ADDR         "0.0.0.0"  
#define MAX_MODBUS_REGISTERS    0x84C0  // 与原代码保持一致  
#define DATA_REFRESH_INTERVAL   5       // 数据刷新间隔(秒)  
#define CONN_TIMEOUT_SEC        5       // 连接超时(秒)  
#define SELECT_TIMEOUT_MS       1000    // select超时(毫秒)
#define MODBUS_MSG_TIMEOUT      5

//#define printf	printf("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), printf

/*********************************************************************************
*  
*  FUNCTION NAME :  Event_Handler()
*  CREATOR       : John
*  CREATED DATE  : 2022-12-30 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE	    emEvType,
	_EVENT_MSG	    emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int			    nMaxBlockMSec)
{
	REPORTER* pReporter = (REPORTER*)hModuleSeif;
	MODBUS_DATA_STU* pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	int n = 0;

	RunThread_Heartbeat(pReporter->hThread); //in case the thread no response

	switch (emEvType)
	{
		case EVENT_TYPE_ALARM:
		{
			//printf("Info:Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_ALARM * pAlmEv = (EVENT_ALARM*)pEvData;
			if (nEvDataSize != sizeof(EVENT_ALARM))
			{
				return EVENT_HANDED_FAILED;
			}
 		}
		break;
		case EVENT_TYPE_FIRMWARE_UPDATE:
		{
			// printf("ModbusTcp_Event_Handler  _EVENT_TYPE = EVENT_TYPE_FIRMUPDATE \t emMsgType = %d, size[%d,%d] \n", 
			// 	emMsgType, nEvDataSize, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
			EVENT_FIRMWAREUPDATE_STATUS * pFirmUpSta = (EVENT_FIRMWAREUPDATE_STATUS*)pEvData;
			if (nEvDataSize != sizeof(EVENT_FIRMWAREUPDATE_STATUS))
			{
				return EVENT_HANDED_FAILED;
			}
		}
		break;
		
		case EVENT_TYPE_DIAGNOSTIC_UPLOAD:
		{
			// printf("ModbusTcp_Event_Handler  _EVENT_TYPE = EVENT_TYPE_DIAGNOSUPLOAD \t emMsgType = %d \n", emMsgType);
			EVENT_DIAGNOUPLOAD_STATUS * pDiagUpSta = (EVENT_DIAGNOUPLOAD_STATUS*)pEvData;
			if (nEvDataSize != sizeof(EVENT_DIAGNOUPLOAD_STATUS))
			{
				return EVENT_HANDED_FAILED;
			}
		}
		break;
		case EVENT_TYPE_SYSTEM_EXIT:
		{
			EVENT_SYSTEM_EXIT* pEvReboot = (EVENT_SYSTEM_EXIT*)pEvData;
			if(nEvDataSize != sizeof(EVENT_SYSTEM_EXIT))
			{
				return EVENT_HANDED_FAILED;
			}
			// printf("ModbusTcp_Event_Handler  _EVENT_TYPE = EVENT_TYPE_EXIT, EXIT_TYPE = %d \n", pEvReboot->nExitType);
 
		}
		break;
		
		case EVENT_TYPE_CFG_CHANGED:
		{
			// printf("ModbusTCP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_PARAM_CHG * pParamChg = (EVENT_PARAM_CHG*)pEvData;
			if (nEvDataSize != sizeof(EVENT_PARAM_CHG))
			{
				return EVENT_HANDED_FAILED;
			}

			if ((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 26))
			{
				/*printf("Config Changed Evevt \n");
				valGet.nGetSize = sizeof(A_BLOB_ES_BACKEND_CFG);
				valGet.val.blobValue = (BYTE*)&pModbus->blobBackendSet;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 26, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					printf("ERROR: OCPP settings can not be retrieved\n");
					return EVENT_HANDED_FAILED;
				}*/
			}
			else if((pParamChg->parmPTR.nId == 500) && (pParamChg->parmPTR.nSubId == 502))
			{
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					500, 502, &valGet, sizeof(PARAM_VAL_GET));
				pModbus->nCfgBMSNum = valGet.val.ulValue;
				if(pModbus->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
				{
					pModbus->nCfgBMSNum = MAX_SINGLE_STBATT_NUM;
				}
				// printf("nId=[%d] nSubId=[%d] nCfgBMSNum=[%d]\n", pParamChg->parmPTR.nId, pParamChg->parmPTR.nSubId, pModbus->nCfgBMSNum);
			}
			else if((pParamChg->parmPTR.nId == 200) && (pParamChg->parmPTR.nSubId == 201))
			{
				valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
				pModbus->emsPlan.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
				valGet.val.blobValue = (BYTE*)&pModbus->emsPlan;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					200, 201, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					// printf("ERROR: EMS Mgmt Plan Get Failed!\n");
					return EVENT_HANDED_FAILED;
				}
 			}
		}
		break;
	default:
		break;
	}
	return EVENT_HANDED_OK;
}

// ==================== 数据同步函数 ====================  
/**  
 * @brief 将内部数据数组同步到libmodbus映射  
 * @note 在数据刷新线程中调用，需要加锁  
 */ 
static void SyncDataToModbusMapping(MODBUS_DATA_STU *pModbus)  
{  
    if (!pModbus->mb_mapping)  
        return;  

    pthread_mutex_lock(&pModbus->data_mutex);  
    
    // 将szRespData1复制到libmodbus的寄存器映射  
    memcpy(pModbus->mb_mapping->tab_registers,   
           pModbus->szRespData1,   
           pModbus->mb_mapping->nb_registers * sizeof(uint16_t));
    memcpy(pModbus->mb_mapping->tab_input_registers,   
           pModbus->szRespData1,   
           pModbus->mb_mapping->nb_registers * sizeof(uint16_t));
    
    pthread_mutex_unlock(&pModbus->data_mutex);  
}

/**  
 * @brief 从libmodbus映射同步数据到内部数组  
 * @note 用于写操作后的数据回读  
 */  
static void SyncDataFromModbusMapping(MODBUS_DATA_STU *pModbus)  
{  
    if (!pModbus->mb_mapping)  
        return;  

    pthread_mutex_lock(&pModbus->data_mutex);  
    
    memcpy(pModbus->szRespData1,   
           pModbus->mb_mapping->tab_registers,  
           pModbus->mb_mapping->nb_registers * sizeof(uint16_t));  
    
    pthread_mutex_unlock(&pModbus->data_mutex);  
}  

// ==================== 数据刷新线程 ====================  
/**  
 * @brief 数据刷新线程入口函数  
 * @details 定期调用原有的数据刷新函数，并同步到libmodbus映射  
 */  
static DWORD _ThreadEntry_DataRefresh(void *pArg)  
{
    REPORTER *pReporter = (REPORTER*)pArg;  
    MODBUS_DATA_STU *pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;  
    PARAM_VAL_GET valGet;  
    time_t tmLast = 0;  

    // printf("Data Refresh Thread Started\n");  

    while (!pReporter->bExit)  
    {

        RUN_THREAD_HEARTBEAT();  
        
        time_t tmNow = _NOW_;  
        
        // 检查是否需要刷新数据  
        if (abs(tmNow - tmLast) >= DATA_REFRESH_INTERVAL)  
        {  
            tmLast = tmNow;  

            // 获取制造商类型  
            valGet.nGetSize = 4;  
            ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter,   
                DAITYPE_SETT_PARAMVALUE_RW, 1, 54, &valGet, sizeof(PARAM_VAL_GET));  
            
            if (err == ERRCODE_DAI_OK)  
            {  
                pModbus->emManufacture = valGet.val.emValue ? MODBUS_TCP_GHSN : MODBUS_TCP_INFYPOWER;  
            }  

            // 刷新函数
            if (pModbus->emManufacture == MODBUS_TCP_INFYPOWER)
            {
                Refresh_SigMapData(pReporter);
            }  
            else if (pModbus->emManufacture == MODBUS_TCP_GHSN)
            {
                Refresh_SigMapData_GHSN(pReporter);
            }

            // 数据填充
            Stuff_Modbus_ArrayData(pReporter);  

            // 同步数据到libmodbus映射  
            SyncDataToModbusMapping(pModbus);  

            // printf("Data Refreshed at %ld\n", tmNow);  
        }  

        Sleep(1000);  // 每秒检查一次  
    }  

    // printf("Data Refresh Thread Exited\n");  
    return THREAD_CANCEL_ALL;  
}

// ==================== 写寄存器处理 ====================  
/**  
 * @brief 处理单个寄存器写操作 (功能码06)  
 * @param pReporter Reporter句柄  
 * @param addr 寄存器地址  
 * @param value 写入的值  
 * @return BOOL 成功返回TRUE  
 */  
static BOOL HandleWriteSingleRegister(REPORTER *pReporter, int addr, int value)  
{  
    MODBUS_DATA_STU *pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;  
    MD_DATA_ITEM *pMdDataItem = NULL;  
    int iEquipID = 0;  
    BOOL bRet = FALSE;  

    // printf("Write Single Register: addr=0x%04X, value=0x%04X\n", addr, value);  

    // 查找寄存器配置  
    MODBUS_FRAME_TYPE frameType = Modbus_FindRegInfoForConfig(pReporter, addr, &iEquipID, &pMdDataItem);  
    
    if (frameType != MODBUS_NOR || !pMdDataItem)  
    {  
        // printf("ERROR: Register 0x%04X not found in config\n", addr);  
        return FALSE;  
    }  

    // 检查是否可写  
    if (pMdDataItem->nSigType != DAITYPE_CTRL_PARAMVALUE_RW &&   
        pMdDataItem->nSigType != DAITYPE_SETT_PARAMVALUE_RW)  
        
    {  
        // printf("ERROR: Register 0x%04X is read-only\n", addr);  
        return FALSE;  
    }  

    // 调用原有的设置函数  
    bRet = SetModbusSigValues(pReporter, pMdDataItem, value);  

    if (bRet)  
    {  
        // printf("Write Single Register Success: addr=0x%04X\n", addr);  
    }  
    else  
    {  
        // printf("ERROR: Write Single Register Failed: addr=0x%04X\n", addr);  
    }  

    return bRet;  
}  

/**  
 * @brief 处理多个寄存器写操作 (功能码10)  
 * @param pReporter Reporter句柄  
 * @param addr 起始寄存器地址  
 * @param nb 寄存器数量  
 * @param data 数据指针  
 * @return BOOL 成功返回TRUE  
 */  
static BOOL HandleWriteMultipleRegisters(REPORTER *pReporter, int addr, int nb, uint16_t *data)  
{  
    MODBUS_DATA_STU *pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;  
    BOOL bRet = TRUE;  
    int i;  

    // printf("Write Multiple Registers: addr=0x%04X, nb=%d\n", addr, nb);  

    // 特殊处理：EMS管理计划 (地址0x6200)  
    if (addr == 0x6200)  
    {  
        MD_DATA_ITEM *pMdDataItem = NULL;  
        int iEquipID = 0;  
        
        if (Modbus_FindRegInfoForConfig(pReporter, addr, &iEquipID, &pMdDataItem) == MODBUS_NOR)  
        {  
            // 构造pCmdData格式以兼容原函数  
            unsigned char pCmdData[MAX_MODBUSFRAME_LEN];  
            pCmdData[0] = HIBYTE(addr);  
            pCmdData[1] = LOBYTE(addr);  
            pCmdData[2] = HIBYTE(nb);  
            pCmdData[3] = LOBYTE(nb);  
            pCmdData[4] = nb * 2;  // 字节数  
            
            // 复制数据  
            for (i = 0; i < nb; i++)  
            {  
                pCmdData[5 + i*2] = HIBYTE(data[i]);  
                pCmdData[6 + i*2] = LOBYTE(data[i]);  
            }  
            
            bRet = SpecialProEMSMgmtPlan(pReporter, pMdDataItem, pCmdData, nb);  
            return bRet;  
        }  
    }  

    // 普通多寄存器写入：逐个处理  
    for (i = 0; i < nb; i++)  
    {  
        MD_DATA_ITEM *pMdDataItem = NULL;  
        int iEquipID = 0;  
        int currentAddr = addr + i;  

        // 查找配置  
        if (Modbus_FindRegInfoForConfig(pReporter, currentAddr, &iEquipID, &pMdDataItem) != MODBUS_NOR)  
        {  
            // printf("WARNING: Register 0x%04X not found, skipping\n", currentAddr);  
            continue;  
        }  

        // 检查是否可写  
        if (pMdDataItem->nSigType != DAITYPE_CTRL_PARAMVALUE_RW &&   
            pMdDataItem->nSigType != DAITYPE_SETT_PARAMVALUE_RW)  
        {  
            // printf("WARNING: Register 0x%04X is read-only, skipping\n", currentAddr);  
            continue;  
        }  

        // Float类型需要两个寄存器  
        if (pMdDataItem->emDataType == MD_GET_VAL_FLOAT && i+1 < nb)  
        {  
            // 合并两个寄存器为一个32位值  
            int iData = MAKELONG(data[i+1], data[i]);  
            if (!SetModbusSigValues(pReporter, pMdDataItem, iData))  
            {  
                // printf("ERROR: Write register 0x%04X failed\n", currentAddr);  
                bRet = FALSE;  
            }  
            i++;  // 跳过下一个寄存器  
        }  
        else  
        {  
            // 普通16位值  
            if (!SetModbusSigValues(pReporter, pMdDataItem, data[i]))  
            {  
                // printf("ERROR: Write register 0x%04X failed\n", currentAddr);  
                bRet = FALSE;  
            }  
        }  
    }  

    return bRet;  
}

// ==================== Modbus通信主线程 ====================  
/**  
 * @brief Modbus TCP通信主线程  
 * @details 使用libmodbus处理客户端连接和请求  
 */  
static DWORD _ThreadEntry_ModbusComm(void *pArg)  
{  
    REPORTER *pReporter = (REPORTER*)pArg;  
    MODBUS_DATA_STU *pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;  
    VAR_VALUE varV;  
    PARAM_VAL_GET valGet;  
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];  
    int rc;  
    fd_set refset;  
    fd_set rdset;  
    int fdmax;  
    struct timeval tv;  
    time_t tmLastActivity = _NOW_;  
    int nComStatus = 0;  

    // printf("Modbus Communication Thread Started\n");  

    // 设置主select集合  
    FD_ZERO(&refset);  
    FD_SET(pModbus->server_socket, &refset);  
    fdmax = pModbus->server_socket;  

    while (!pReporter->bExit)  
    {
        RUN_THREAD_HEARTBEAT();  
        
        rdset = refset;  
        tv.tv_sec = 0;  
        tv.tv_usec = SELECT_TIMEOUT_MS * 1000;  

        rc = select(fdmax + 1, &rdset, NULL, NULL, &tv);  
        if (rc == -1)  
        {  
            // printf("ERROR: select() failed: %s\n", strerror(errno));  
            if (errno == EINTR)  
                continue;  
            break;  
        }  

        time_t tmNow = _NOW_;  

        // 处理新连接  
        if (FD_ISSET(pModbus->server_socket, &rdset))  
        {  
            // printf("dealing with new connection...\n");
            socklen_t addrlen;  
            struct sockaddr_in clientaddr;  
            int newfd;  

            addrlen = sizeof(clientaddr);  
            newfd = accept(pModbus->server_socket, (struct sockaddr *)&clientaddr, &addrlen);
            
            // printf("newfd = [%d]\n", newfd);
            if (newfd == -1)  
            {  
                // printf("ERROR: accept() failed: %s\n", strerror(errno));  
            }  
            else  
            {  
                FD_SET(newfd, &refset);  
                if (newfd > fdmax)  
                    fdmax = newfd;  

                pModbus->client_socket = newfd;  
                pModbus->bAcceptConnOK = TRUE;  
                tmLastActivity = tmNow;  
                nComStatus = 0;  

                // printf("New client connection from %s:%d (socket=%d)\n",  
                    // inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), newfd);  

                // 通信恢复  
                varV.emValue = 0;  
                pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W,  
                    1, 23, &varV, 4, 100);  

                // // 恢复REMS中断告警（如果在远程模式）  
                // valGet.nGetSize = 4;  
                // pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,  
                //     1001, 3902, &valGet, sizeof(PARAM_VAL_GET));  
                // if (valGet.val.emValue == EMSCtrlMode_Remote)  
                // {  
                //     varV.emValue = 0;  // normal  
                //     pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W,  
                //         1001, 3908, &varV, 4, 100);  
                // }  
            }  
        }  

        // 处理客户端数据
		int master_socket;
        for (master_socket = pModbus->server_socket + 1; master_socket <= fdmax; master_socket++)  
        {  
            if (!FD_ISSET(master_socket, &rdset))  
                continue;  

            modbus_set_socket(pModbus->ctx, master_socket);  
            
            pthread_mutex_lock(&pModbus->data_mutex);  
            rc = modbus_receive(pModbus->ctx, query);  
            pthread_mutex_unlock(&pModbus->data_mutex);  

            // printf("master_socket[%d]::rc = [%d]\n", master_socket, rc);
            if (rc > 0)  
            {  
                // 收到有效请求  
                tmLastActivity = tmNow;  
                nComStatus = 0;  

                // 根据功能码处理  
                int function_code = query[modbus_get_header_length(pModbus->ctx)];  
                int addr = MODBUS_GET_INT16_FROM_INT8(query, modbus_get_header_length(pModbus->ctx) + 1);  

                // printf("Received request: function=0x%02X, addr=0x%04X\n", function_code, addr);  

                pthread_mutex_lock(&pModbus->data_mutex);  

                // 处理写请求  
                if (function_code == MODBUS_FC_WRITE_SINGLE_REGISTER)  
                {  
                    int value = MODBUS_GET_INT16_FROM_INT8(query, modbus_get_header_length(pModbus->ctx) + 3);  
                    
                    if (HandleWriteSingleRegister(pReporter, addr, value))  
                    {  
                        // 更新映射中的值  
                        if (addr < pModbus->mb_mapping->nb_registers)  
                        {  
                            pModbus->mb_mapping->tab_registers[addr] = value;  
                        }  
                        modbus_reply(pModbus->ctx, query, rc, pModbus->mb_mapping);  
                    }  
                    else  
                    {  
                        modbus_reply_exception(pModbus->ctx, query, MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE);  
                    }  
                }  
                else if (function_code == MODBUS_FC_WRITE_MULTIPLE_REGISTERS)  
                {  
                    int nb = MODBUS_GET_INT16_FROM_INT8(query, modbus_get_header_length(pModbus->ctx) + 3);  
                    uint16_t *data = (uint16_t *)(query + modbus_get_header_length(pModbus->ctx) + 6);  
                    
                    // 字节序转换  
                    uint16_t *converted_data = (uint16_t *)malloc(nb * sizeof(uint16_t));  
					int i;
                    for (i = 0; i < nb; i++)  
                    {  
                        converted_data[i] = MODBUS_GET_INT16_FROM_INT8(query,   
                            modbus_get_header_length(pModbus->ctx) + 6 + i * 2);  
                    }  
                    
                    if (HandleWriteMultipleRegisters(pReporter, addr, nb, converted_data))  
                    {  
                        // 更新映射中的值  
						int i;
                        for (i = 0; i < nb && (addr + i) < pModbus->mb_mapping->nb_registers; i++)  
                        {  
                            pModbus->mb_mapping->tab_registers[addr + i] = converted_data[i];  
                        }  
                        modbus_reply(pModbus->ctx, query, rc, pModbus->mb_mapping);  
                    }  
                    else  
                    {  
                        modbus_reply_exception(pModbus->ctx, query, MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE);  
                    }  
                    
                    free(converted_data);  
                }  
                else  
                {  
                    // 读请求或其他请求由libmodbus自动处理  
                    modbus_reply(pModbus->ctx, query, rc, pModbus->mb_mapping);  
                }  

                pthread_mutex_unlock(&pModbus->data_mutex);  
            }  
            else if (rc == -1)  
            {  
                // 连接断开或错误  
                // printf("Client disconnected (socket=%d): %s\n", master_socket, modbus_strerror(errno));  
                
                close(master_socket);  
                FD_CLR(master_socket, &refset);  
                
                if (master_socket == pModbus->client_socket)  
                {  
                    pModbus->bAcceptConnOK = FALSE;  
                    pModbus->client_socket = -1;  
                }  

                if (master_socket == fdmax)  
                {  
                    fdmax = pModbus->server_socket; 
					int i; 
                    for (i = pModbus->server_socket + 1; i < master_socket; i++)  
                    {  
                        if (FD_ISSET(i, &refset))  
                            fdmax = i;  
                    }  
                }  
            }  
        }  

        // 检查连接超时  
        // printf("bAcceptConnOK = [%d], diff = [%d]\n", pModbus->bAcceptConnOK, (tmNow - tmLastActivity));
        if ((tmNow - tmLastActivity) >= CONN_TIMEOUT_SEC)  
        {
            nComStatus = 1;  

            // // 切换到本地EMS模式  
            // valGet.nGetSize = 4;  
            // pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,  
            //     1001, 3902, &valGet, sizeof(PARAM_VAL_GET));  
            // if (valGet.val.emValue == EMSCtrlMode_Remote)  
            // {  
            //     varV.emValue = EMSCtrlMode_Local;  
            //     pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W,  
            //         1001, 3902, &varV, 4, 100);  

            //     varV.emValue = 1;  // abnormal  
            //     pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W,  
            //         1001, 3908, &varV, 4, 100);  
            // }  

            // tmLastActivity = tmNow;  // 重置计时器  
        }  

        // 更新通信状态  
        varV.emValue = nComStatus;  
        pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W,  
            1, 23, &varV, 4, 100); 

        Sleep(100); 
    }  

    // printf("Modbus Communication Thread Exited\n");  
    return THREAD_CANCEL_ALL;  
}

// ==================== 初始化函数 ====================  
/**  
 * @brief 初始化libmodbus TCP服务器  
 * @return BOOL 成功返回TRUE  
 */  
static BOOL InitModbusTcpServer(REPORTER *pReporter)  
{  
    MODBUS_DATA_STU *pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;  
    int ret;  
    PARAM_VAL_GET valGet; 

    // printf("Initializing Modbus TCP Server...\n");  

    valGet.nGetSize = 4;  
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,  
        1, 57, &valGet, sizeof(PARAM_VAL_GET));
    int port = valGet.val.ulValue;
    // printf("Modbus TCP Server Port: %d\n", port);
    // 创建TCP上下文  
    pModbus->ctx = modbus_new_tcp(MODBUS_TCP_ADDR, port);  
    if (pModbus->ctx == NULL)  
    {  
        // printf("ERROR: Unable to allocate libmodbus context: %s\n", modbus_strerror(errno));  
        return FALSE;  
    }  

    modbus_set_debug(pModbus->ctx, FALSE);

    // 设置响应超时 5s
    modbus_set_response_timeout(pModbus->ctx, 5, 0);

    // 创建寄存器映射
    // 数据图的使用方式限制了源数据只能使用保持寄存器，后续复制到输入寄存器
    pModbus->mb_mapping = modbus_mapping_new(0, 0, MAX_MODBUS_REGISTERS, MAX_MODBUS_REGISTERS);  
    if (pModbus->mb_mapping == NULL)  
    {  
        // printf("ERROR: Failed to allocate modbus mapping: %s\n", modbus_strerror(errno));  
        modbus_free(pModbus->ctx);  
        return FALSE;  
    }  

    // 初始化映射数据为 0xFFFF
    memset(pModbus->mb_mapping->tab_registers, 0xFF,   
            pModbus->mb_mapping->nb_registers * sizeof(uint16_t)); 
    memset(pModbus->mb_mapping->tab_input_registers, 0xFF,
            pModbus->mb_mapping->nb_registers * sizeof(uint16_t)); 

    // 监听TCP端口，最多2个连接  
    pModbus->server_socket = modbus_tcp_listen(pModbus->ctx, 2);
    if (pModbus->server_socket == -1)  
    {  
        // printf("ERROR: Unable to listen TCP connection: %s\n", modbus_strerror(errno));  
        modbus_mapping_free(pModbus->mb_mapping);  
        modbus_free(pModbus->ctx);  
        return FALSE;  
    }  

    // 初始化互斥锁  
    if (pthread_mutex_init(&pModbus->data_mutex, NULL) != 0)  
    {  
        // printf("ERROR: Failed to initialize mutex\n");  
        close(pModbus->server_socket);  
        modbus_mapping_free(pModbus->mb_mapping);  
        modbus_free(pModbus->ctx);  
        return FALSE;  
    }  

    pModbus->client_socket = -1;  
    pModbus->bAcceptConnOK = FALSE;  

    // printf("Modbus TCP Server initialized successfully on %s:%d\n",   
        //   MODBUS_TCP_ADDR, port);  

    return TRUE;  
}

typedef struct tagEvHandTags
{
	int nEvTag;
	int nMsgTag;

}EVHAND_TAGS;

DLLExport BOOL Initialize(HANDLE pReporterThis)
{
    // printf("Initialize Modbus Reporter with libmodbus\n");
    
    REPORTER *pReporter = (REPORTER*)pReporterThis;
    MODBUS_DATA_STU *pModbus = NULL;
    PARAM_VAL_GET valGet;
    ERRCODE_DAI err;
    DWORD dwExitCode;

    // 分配数据结构
    pReporter->pDataHeap = NEW(MODBUS_DATA_STU, 1);
    if (pReporter->pDataHeap == NULL)
    {
        // printf("ERROR: Memory allocation failed for Modbus data structures\n");
        return FALSE;
    }

    pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;
    memset(pModbus, 0, sizeof(MODBUS_DATA_STU));

    // 初始化时间戳
    pModbus->tmLastDataRefresh = _NOW_;
    pModbus->tmLastConnctTime = _NOW_;
    pModbus->nReadFailtTimes = 0;

    // 获取制造商类型
    valGet.nGetSize = 4;
    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
        1, 54, &valGet, sizeof(PARAM_VAL_GET));
    if (err != ERRCODE_DAI_OK)
    {
        // printf("ERROR: Failed to get manufacturer type\n");
        DELETE(pReporter->pDataHeap);
        return FALSE;
    }
    pModbus->emManufacture = valGet.val.emValue ? MODBUS_TCP_GHSN : MODBUS_TCP_INFYPOWER;

    // 初始化数据映射表
    if (pModbus->emManufacture == MODBUS_TCP_INFYPOWER)
    {
        dwExitCode = InitModbusDataMap(pReporter);
    }
    else if (pModbus->emManufacture == MODBUS_TCP_GHSN)
    {
        dwExitCode = InitModbusDataMap_GHSN(pReporter);
    }
    else
    {
        // printf("ERROR: Unknown manufacturer type: %d\n", pModbus->emManufacture);
        DELETE(pReporter->pDataHeap);
        return FALSE;
    }

    if (dwExitCode != ERR_CFG_OK)
    {
        // printf("ERROR: Data map initialization failed with code: %d\n", dwExitCode);
        DELETE(pReporter->pDataHeap);
        return FALSE;
    }

    // 初始化libmodbus TCP服务器
    if (!InitModbusTcpServer(pReporter))
    {
        // printf("ERROR: Failed to initialize Modbus TCP server\n");
        if (pModbus->pmdDataItems)
            DELETE(pModbus->pmdDataItems);
        DELETE(pReporter->pDataHeap);
        return FALSE;
    }

    pReporter->bExit = FALSE;

    // 创建数据刷新线程
    HANDLE hDataRefreshThread = RunThread_Create("ModbusDataRefresh",
        (RUN_THREAD_START_PROC)_ThreadEntry_DataRefresh,
        (void *)pReporter,
        (DWORD *)&pReporter->bExit);
    
    if (hDataRefreshThread == NULL)
    {
        // printf("ERROR: Failed to create data refresh thread\n");
        close(pModbus->server_socket);
        modbus_mapping_free(pModbus->mb_mapping);
        modbus_free(pModbus->ctx);
        pthread_mutex_destroy(&pModbus->data_mutex);
        if (pModbus->pmdDataItems)
            DELETE(pModbus->pmdDataItems);
        DELETE(pReporter->pDataHeap);
        return FALSE;
    }

    // 创建Modbus通信线程
    pReporter->hThread = RunThread_Create("ModbusCommLibmodbus",
        (RUN_THREAD_START_PROC)_ThreadEntry_ModbusComm,
        (void *)pReporter,
        (DWORD *)&pReporter->bExit);

    if (pReporter->hThread == NULL)
    {
        // printf("ERROR: Failed to create Modbus communication thread\n");
        RunThread_Destroy(hDataRefreshThread, 3000);
        close(pModbus->server_socket);
        modbus_mapping_free(pModbus->mb_mapping);
        modbus_free(pModbus->ctx);
        pthread_mutex_destroy(&pModbus->data_mutex);
        if (pModbus->pmdDataItems)
            DELETE(pModbus->pmdDataItems);
        DELETE(pReporter->pDataHeap);
        return FALSE;
    }

    // 注册事件处理器
    EVHAND_TAGS tags[] =
    {
        { EVENT_TYPE_ALARM,              EVENT_MSG_BEGIN | EVENT_MSG_END },
        { EVENT_TYPE_CFG_CHANGED,        EVENT_MSG_BEGIN },
        { EVENT_TYPE_FIRMWARE_UPDATE,    EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
        { EVENT_TYPE_DIAGNOSTIC_UPLOAD,  EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
        { EVENT_TYPE_SYSTEM_EXIT,        EVENT_MSG_END },
    };

    int n;
    for (n = 0; n < ITEM_OF(tags); n++)
    {
        pReporter->pDAI->Regsiter_EvHandler("mdbstcp_EventHand",
            pReporterThis,
            Event_Handler,
            tags[n].nEvTag,
            tags[n].nMsgTag,
            FALSE);
    }

    // printf("Modbus Reporter initialized successfully\n");
    return TRUE;
}
