#include "../reporters_base.h"
#include"modbus.h"
#include <signal.h>

//#define TRACE	printf("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), printf

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
			//TRACE("Info:Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_ALARM * pAlmEv = (EVENT_ALARM*)pEvData;
			if (nEvDataSize != sizeof(EVENT_ALARM))
			{
				return EVENT_HANDED_FAILED;
			}
 		}
		break;
		case EVENT_TYPE_FIRMWARE_UPDATE:
		{
			TRACEX("ModbusTcp_Event_Handler  _EVENT_TYPE = EVENT_TYPE_FIRMUPDATE \t emMsgType = %d, size[%d,%d] \n", 
				emMsgType, nEvDataSize, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
			EVENT_FIRMWAREUPDATE_STATUS * pFirmUpSta = (EVENT_FIRMWAREUPDATE_STATUS*)pEvData;
			if (nEvDataSize != sizeof(EVENT_FIRMWAREUPDATE_STATUS))
			{
				return EVENT_HANDED_FAILED;
			}
		}
		break;
		
		case EVENT_TYPE_DIAGNOSTIC_UPLOAD:
		{
			TRACEX("ModbusTcp_Event_Handler  _EVENT_TYPE = EVENT_TYPE_DIAGNOSUPLOAD \t emMsgType = %d \n", emMsgType);
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
			TRACEX("ModbusTcp_Event_Handler  _EVENT_TYPE = EVENT_TYPE_EXIT, EXIT_TYPE = %d \n", pEvReboot->nExitType);
 
		}
		break;
		
		case EVENT_TYPE_CFG_CHANGED:
		{
			TRACEX("ModbusTCP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_PARAM_CHG * pParamChg = (EVENT_PARAM_CHG*)pEvData;
			if (nEvDataSize != sizeof(EVENT_PARAM_CHG))
			{
				return EVENT_HANDED_FAILED;
			}

			if ((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 26))
			{
				/*TRACEX("Config Changed Evevt \n");
				valGet.nGetSize = sizeof(A_BLOB_ES_BACKEND_CFG);
				valGet.val.blobValue = (BYTE*)&pModbus->blobBackendSet;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 26, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("ERROR: OCPP settings can not be retrieved\n");
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
				TRACEX("nId=[%d] nSubId=[%d] nCfgBMSNum=[%d]\n", pParamChg->parmPTR.nId, pParamChg->parmPTR.nSubId, pModbus->nCfgBMSNum);
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
					TRACEX("ERROR: EMS Mgmt Plan Get Failed!\n");
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

static BOOL AcceptClientConnect(REPORTER *pReporter)
{
 	MODBUS_DATA_STU* pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;

	HANDLE hDataComm;
	COMM_TIMEOUTS timeOut;

	/* to get client IP */
	COMM_PEER_INFO cpi;   
	DWORD dwPeerIP = 0;
	char *szClientIP;

	int nErrCode;
  
	if(pModbus->bAcceptConnOK)
	{
		return TRUE;
	}
	else
	{
		TRACE(" Waiting Accept Client Connect !!!\n");

		pReporter->pUsedCommPort->Control(pReporter->hCommPort, COMM_GET_LAST_ERROR, (char *)&nErrCode, sizeof(int));

		//timeOut.nReadTimeout = 5000;
		//timeOut.nWriteTimeout = 1000;
		//pReporter->pUsedCommPort->Control(pThis->hPort, COMM_SET_TIMEOUTS, &timeOut, sizeof(COMM_TIMEOUTS));
		
		pReporter->pUsedCommPort->Control(pReporter->hCommPort, COMM_GET_LAST_ERROR, (char *)&nErrCode, sizeof(int));
		hDataComm = pReporter->pUsedCommPort->Accept(pReporter->hCommPort);
		if (hDataComm == NULL)
		{
			// /* get last err code */
			// pReporter->pUsedCommPort->Control(pReporter->hCommPort, COMM_GET_LAST_ERROR, (char *)&nErrCode, sizeof(int));
			// TRACE("\nnErrCode1 is: %d", nErrCode);

			// if (nErrCode == ERR_COMM_TIMEOUT)
			// {
			// 	TRACE("Accept Timeout !!!\n");
			// }
			// else  /* for other err, exit programme */
			// {
			// 	TRACE("Accept failed nErrCode =%d\n", nErrCode);
			// 	pReporter->pUsedCommPort->Close(pReporter->hCommPort);
			// 	pReporter->bExit = TRUE;	
			// }
			return FALSE;
		} /* end of if (hDataComm == NULL) */

		pModbus->hMDDataComm = hDataComm;
		
		pModbus->bAcceptConnOK = TRUE;
		/* 
		pReporter->pUsedCommPort->Control(hDataComm, COMM_GET_PEER_INFO, (char *)&cpi, sizeof(COMM_PEER_INFO));
		if (nErrCode == ERR_COMM_OK)
		{
			// szAddr is in network format.
			dwPeerIP = *(DWORD *)cpi.szAddr;
			szClientIP = inet_ntoa(*(struct in_addr *)&dwPeerIP);
		
			char szLogText[MODBUS_LOG_TEXT_LEN];
			sprintf(szLogText, "Client %s is accepted", szClientIP);
			TRACE("szLogText = %s \n",szLogText);	
		}	
		*/
		return TRUE;
	}
	return FALSE;
}


static BOOL FindAFrame(REPORTER *pReporter, char *pDstBuf, int *piLen)
{
	int iRet = FALSE;
	int iSrcPos, iSrcLen;
	char *pSrcBuf;
	MODBUS_DATA_STU* pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;

	iSrcPos = pModbus->iCurPos;
	iSrcLen = pModbus->iLen;
	//iSrcLen = strlen(pThis->sReceivedBuff);
	pSrcBuf = pModbus->sReceivedBuff;
	*piLen = 0;

	if (iSrcPos < iSrcLen)
	{
		//need extra code
		if((iSrcLen) > MAX_MODBUSFRAME_LEN)
		{
			pModbus->iCurPos =0;
			pModbus->iLen = 0;
			TRACE("***Error: FindAFrame iSrcLen greater than MAX_MODBUSFRAME_LEN.\n");
			return FALSE;
		}
		
		iRet = TRUE;
		memcpy(pDstBuf, pSrcBuf + iSrcPos, (size_t)iSrcLen);
		*piLen = iSrcLen;
		iSrcPos = iSrcLen;

		if (iSrcPos < iSrcLen)
		{
			TRACE("Split a frame!\n");
		}
	}

	pModbus->iCurPos = iSrcPos;
	return iRet;
}

static int ReadDataFromCommPort(REPORTER *pReporter, char *pBuffer, int *piLen)
{
	int nErrCode = ERR_COMM_TIMEOUT;
	int	bFindAFrame = FALSE;

	COMM_TIMEOUTS timeOut;
	MODBUS_DATA_STU* pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;

	if (!FindAFrame(pReporter, pBuffer, piLen))
	{

		//timeOut.nReadTimeout = MODBUS_READCOM_TIMEOUT;
		//timeOut.nWriteTimeout = MODBUS_WRITECOM_TIMEOUT ;
		//pReporter->pUsedCommPort->Control(pReporter->hCommPort, COMM_SET_TIMEOUTS, &timeOut, sizeof(COMM_TIMEOUTS));
		
		pModbus->iCurPos = 0;
		pModbus->iLen = pReporter->pUsedCommPort->Read(pModbus->hMDDataComm, pModbus->sReceivedBuff, MAX_MODBUSFRAME_LEN);
		
		pReporter->pUsedCommPort->Control(pReporter->hCommPort, COMM_GET_LAST_ERROR, (char *)&nErrCode, sizeof(int));

		bFindAFrame = FindAFrame(pReporter, pBuffer, piLen);

		if(pModbus->iLen == 0)
		{
			//客户端断开连接后，直接销毁连接，防止再次连接时出现端口复用
			pModbus->bAcceptConnOK = FALSE;
			pReporter->pUsedCommPort->Close(pModbus->hMDDataComm);
			nErrCode = ERR_COMM_CONNECTION_BROKEN;
		}
		else
		{
			nErrCode = (bFindAFrame) ?  ERR_COMM_OK : ERR_COMM_TIMEOUT;
		}
	 
		//TRACE("Tcp_Read Data !!!  *piLen =%d\n",  *piLen);
		// if (nErrCode == ERR_COMM_OK)
		// {
		// 	int i;
		// 	for(i = 0; i < pModbus->iLen; i++)
		// 	{
		// 		printf("%02x ",pBuffer[i]);
		// 	}
		// 	printf("\n");
		// }	
	}

	TRACE("nErrCode=%d bFindAFrame =%d pThis->iCurPos=%d pThis->iLen=%d \n",\
		nErrCode, bFindAFrame,pModbus->iCurPos,pModbus->iLen);
	return nErrCode;
}


static BOOL Process_Modbus_Frame(REPORTER *pReporter, char *pBuffer, int nDataLen )
{
	MODBUS_FRAME_TYPE frameType,getdataresult;
	unsigned char pCmdData[MAX_MODBUSFRAME_LEN];
	unsigned char pMBAPhead[MODBUS_MBAP_LEN];
	int  iFunctionCode, iAddr, nNeedWriteLen, nWriteBytes;
	MODBUS_DATA_STU* pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;

	unsigned char	szCmdRespBuff[MAX_MODBUSFRAME_LEN]={'\0'};		// command reply data buf 


	iAddr = (int)(pBuffer[MODBUS_MBAP_LEN - 1]);
	iFunctionCode = (int)(pBuffer[MODBUS_MBAP_LEN]);

	frameType = Modbus_ParseFrame(pReporter, pBuffer, nDataLen, pCmdData, pMBAPhead);

	switch (frameType)
	{
	     case MODBUS_NOR:  
			if((iFunctionCode == MODBUS_GETDATA_CMD)||
			(iFunctionCode == MODBUS_GETDATA_CMD_04))
			{
			    getdataresult = Modbus_03CmdParseAndHandle(pCmdData);
			}
			else if(iFunctionCode == MODBUS_SETSINGLEDATA_CMD)
			{
			    getdataresult = Modbus_06CmdParseAndHandle(pReporter, pCmdData);
			}
			else if(iFunctionCode == MODBUS_SETMULTIDATA_CMD)
			{
			    getdataresult = Modbus_10CmdParseAndHandle(pReporter, pCmdData);
			}
			else
			{
			    getdataresult = MODBUS_FUNCCODEERR;
			}
			break;

		default:  //unexpected frame event
		    break;
	}
	if(frameType == MODBUS_FRAME_ERR) 
	{
		return FALSE;
	}
	
	Modbus_PackResponseFrame(pReporter , getdataresult, iAddr,iFunctionCode,pCmdData,pMBAPhead,
	    szCmdRespBuff, &nNeedWriteLen);
 
	nWriteBytes = pReporter->pUsedCommPort->Write(pModbus->hMDDataComm, szCmdRespBuff, nNeedWriteLen);
	if(nWriteBytes < nNeedWriteLen)		
	{
		return FALSE;
	}			
	return TRUE;
}

/*==========================================================================*
 * FUNCTION : _ThreadEntry_ModbusComm_WithInfy
 * PURPOSE  : 
 * CALLS    :  
 *			  
 * CALLED BY: 
 * ARGUMENTS: void* pArg : 
 * RETURN   : DWORD 
 * COMMENTS : 
 * CREATOR  :					2019-08-11 9:25:10
 *==========================================================================*/
static DWORD _ThreadEntry_ModbusComm_WithInfy(void* pArg)
{
	REPORTER* pReporter = (REPORTER*)pArg;
	MODBUS_DATA_STU* pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;
	PARAM_VAL_GET valGet;
	VAR_VALUE varV;
 	int nErrCode, nLen = 0;
	char sBuffer[MAX_MODBUSFRAME_LEN] = {'\0'};
	int nComStatus = 0;
	TRACE("_ThreadEntry_ModbusComm_WithInfy : Start Enter \n");
	while (!pReporter->bExit)
	{
		RUN_THREAD_HEARTBEAT();
		time_t tmNow = _NOW_;

		if(!AcceptClientConnect(pReporter))
		{
			if(ABS(tmNow - pModbus->tmLastConnctTime) >= CONNET_OVERTIME_SEC)
			{//客户端5秒内都没有连接报通信中断
				varV.emValue = 1;
				pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
					1, 48, &varV, 4, 100);//通信状态	
				
				valGet.nGetSize = 4;
				pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 
					1001, 3902, &valGet, sizeof(PARAM_VAL_GET));
				if(valGet.val.emValue == EMSCtrlMode_Remote) // 通信中断，转为本地EMS，报REMS中断
				{
					varV.emValue = EMSCtrlMode_Local;
					pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
						1001, 3902, &varV, 4, 100);

					varV.emValue = 1; // abnormal 
					pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
						1001, 3908, &varV, 4, 100);
				}
			}
			Sleep(1000);
			continue;
		}
		pModbus->tmLastConnctTime = tmNow;
		nErrCode = ReadDataFromCommPort(pReporter, sBuffer, &nLen);
 		switch (nErrCode)
		{
			case ERR_COMM_OK/* constant-expression */:
				{
					TRACE("ReadDataFromPort OK nLen=[%d]!!!!\n", nLen);
					Process_Modbus_Frame(pReporter, sBuffer , nLen);
					pModbus->nReadFailtTimes = 0;
					nComStatus = 0;

					valGet.nGetSize = 4;
					pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 
						1001, 3902, &valGet, sizeof(PARAM_VAL_GET));
					if(valGet.val.emValue == EMSCtrlMode_Remote) // 通信恢复，转为上级EMS，消除REMS中断告警
					{
						varV.emValue = 0; // normal  
						pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
							1001, 3908, &varV, 4, 100); // 恢复REMS中断告警
					}

					break;
				}
			case ERR_COMM_CONNECTION_BROKEN :
			case ERR_COMM_TIMEOUT :
			default:
				pModbus->nReadFailtTimes++;
				if(pModbus->nReadFailtTimes > MAX_READ_FAIL_TIMES)
				{
					varV.fValue = 0.0;
					pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, //通信中断后，立马切断输出
						1001, 3901, &varV, 4, 100);
					nComStatus = 1;
					pModbus->bAcceptConnOK = FALSE;
					pReporter->pUsedCommPort->Close(pModbus->hMDDataComm);
				}
				break;
		}

		varV.emValue = nComStatus;
		pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
		1, 48, &varV, 4, 100);//通信状态

		//TRACE("Circle Reading Data From Tcp Comm Port\n");
		valGet.nGetSize = 4;
		ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
			1, 54, &valGet, sizeof(PARAM_VAL_GET));
		if (err != ERRCODE_DAI_OK)
		{
			printf("Get Modbus Tcp cp manu fail2 \n");
			return FALSE;
		}
		pModbus->emManufacture = valGet.val.emValue ? MODBUS_TCP_GHSN : MODBUS_TCP_INFYPOWER;

		if(abs(tmNow - pModbus->tmLastDataRefresh) > TM_DATA_REFRESH_INTERVAL)
		{
			pModbus->tmLastDataRefresh = tmNow;

			if(pModbus->emManufacture == MODBUS_TCP_INFYPOWER)
			{
				Refresh_SigMapData(pReporter);
			}
			else if(pModbus->emManufacture == MODBUS_TCP_GHSN)
			{
				Refresh_SigMapData_GHSN(pReporter);
			}
			Stuff_Modbus_ArrayData(pReporter);
		}
		Sleep(100);
	}
	
	pReporter->pUsedCommPort->Close(pReporter->hCommPort);
	return THREAD_CANCEL_ALL;
}

static BOOL InitTcpCommPort(REPORTER *pReporter)
{
	int iRetryTimes = 0;
	int nErrCode;
	char szEndUrl[256] = "127.0.0.1:502";
	BOOL bRet = FALSE;

	for(iRetryTimes = 0; iRetryTimes < 3; iRetryTimes++)
	{
		RUN_THREAD_HEARTBEAT();
		pReporter->hCommPort = pReporter->pUsedCommPort->Open("Modbus_Tcp",
			szEndUrl, COMM_SERVER_MODE, 5000, &nErrCode); //5 sec timeout 
		TRACE("22222  szEndUrl=%s\n", szEndUrl);
		
		TRACE("RunAsServer : nErrCode =%d \n",nErrCode);
		switch (nErrCode)
		{
		    case ERR_COMM_STD_PORT:	
				{
					TRACE("Open Comm Port failed  ID invalid");
					bRet = FALSE;
					break;
				}

		    case ERR_COMM_OPENING_PARAM:
				{
 					TRACE("Open Comm Port failed Param is invalid");
			 		bRet = FALSE;
 					break;
				}

		   case ERR_OK:
		   		{
				    TRACE("Open Port OK!!!!\n");
					bRet = TRUE;
					iRetryTimes = 3;  /* break while loop */
					break;
				}

		   default:  /* do nothing, just retry */
				Sleep(1000); //yield
				break;
		}
	}
	return bRet;
}

typedef struct tagEvHandTags
{
	int nEvTag;
	int nMsgTag;

}EVHAND_TAGS;

/*==========================================================================*
 * FUNCTION : Initialize
 * PURPOSE  : 
 * CALLS    :  
 *			  
 * CALLED BY: 
 * ARGUMENTS: HANDLE pReporterThis : 
 * RETURN   : BOOL 
 * COMMENTS : 
 * CREATOR  :					2019-08-11 9:25:10
 *==========================================================================*/
DLLExport BOOL  Initialize(HANDLE pReporterThis)
{
	TRACE("Initialize Modbus Reporter !!! \n");
	REPORTER* pReporter = (REPORTER*)pReporterThis;
	MODBUS_DATA_STU *pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;
	DWORD dwExitCode;
 
	//Create Data 
	pReporter->pDataHeap = NEW(MODBUS_DATA_STU, 1);
	if(pReporter->pDataHeap == NULL)
	{
		TRACE("Memory Full For Modbus data structs !\n");
		return FALSE;
	}

	PARAM_VAL_GET valGet;
	valGet.nGetSize = 4;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
		1, 54, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		// printf("Get Modbus Tcp cp manu fail1 \n");
		return FALSE;
	}
	pModbus->emManufacture = valGet.val.emValue ? MODBUS_TCP_GHSN : MODBUS_TCP_INFYPOWER;

	if(pModbus->emManufacture == MODBUS_TCP_INFYPOWER)// infy modbus tcp
	{
		dwExitCode = InitModbusDataMap(pReporter);
	}
	else if(pModbus->emManufacture == MODBUS_TCP_GHSN)// Gang Hua Shu Neng
	{
		dwExitCode = InitModbusDataMap_GHSN(pReporter);
	}
	else
	{
		return FALSE;
	}

	if(dwExitCode != ERR_CFG_OK)
	{
		TRACE("dwExitCode != SERVICE_EXIT_NONE !\n");
		return FALSE;
	}
	
	if(!InitTcpCommPort(pReporter))
	{
		return FALSE;
	}
	pModbus->tmLastConnctTime = _NOW_;
	pReporter->bExit = FALSE;
	
	pReporter->hThread = RunThread_Create("ModbusCommInfy",
		(RUN_THREAD_START_PROC)_ThreadEntry_ModbusComm_WithInfy,
		(void *)(pReporter),
		(DWORD*)&pReporter->bExit);
 
	if(pReporter->hThread == NULL)
	{
		TRACE("ERROR : Create OCPPComm thread Failed \n");
		return FALSE;
	}
	
	EVHAND_TAGS tags[] =
	{
			{ EVENT_TYPE_ALARM,				EVENT_MSG_BEGIN | EVENT_MSG_END },
			{ EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN },
 			{ EVENT_TYPE_FIRMWARE_UPDATE,	EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
			{ EVENT_TYPE_DIAGNOSTIC_UPLOAD,EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
			{ EVENT_TYPE_SYSTEM_EXIT,		EVENT_MSG_END},
 	};

	int n = 0;
	for (n = 0; n < ITEM_OF(tags); n++)
	{
		pReporter->pDAI->Regsiter_EvHandler("mdbstcp_EventHand",
			pReporterThis,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}	

	return TRUE;
}


