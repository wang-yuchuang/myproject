#include "../../reporters_base.h"
#include "mbe_dev_mqtt.h"

//Jimmy add for debug only
#ifdef _DEBUG
#define ___TEST_MQTT_MBE__	1
#endif

static void ReCreate_Client_AndInit(MQTT_MBE_DATA_ROUGH* pMqtt);
static int s_Conn_ButNoAuth_TotalTimes = 0; //the conn OK but no-auth times 

typedef enum
{
	MQTT_MBE_LOG_RECREAT,
	MQTT_MBE_LOG_CONNECT,
	MQTT_MBE_LOG_DISCONNECT,
	MQTT_MBE_LOG_BEGIN_SUBALL,
	MQTT_MBE_LOG_SUBALLOK,
	MQTT_MBE_LOG_SEND,
	MQTT_MBE_LOG_RECEIVE
}MQTT_MBE_LOG_TYPE;
static void LOG_MQTT(MQTT_MBE_LOG_TYPE nLogType, char *pszFormat, ...)
{
	//logContent
	char szBuf[4096];
	memset(szBuf, 0, 4096);
	va_list arglist;
	va_start(arglist, pszFormat);
	//vsnprintf(szBuf + strlen(szBuf), sizeof(szBuf), pszFormat, arglist);
	vsnprintf(szBuf, 4096, pszFormat, arglist);
	va_end(arglist);

	char szTime[32];
	memset(szTime, 0, 32);
	double fLogTime = GetCurrentTime();
	if(nLogType == MQTT_MBE_LOG_RECREAT)
	{
		LogOut_MBE("[%s][MQTT-Create]:%s", AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32), szBuf);
	}
	else if(nLogType == MQTT_MBE_LOG_CONNECT)
	{
		LogOut_MBE("[%s][MQTT-Connect]:%s", AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32), szBuf);
	}
	else if(nLogType == MQTT_MBE_LOG_DISCONNECT)
	{
		LogOut_MBE("[%s][MQTT-Discon]:%s", AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32), szBuf);
	}
	else if(nLogType == MQTT_MBE_LOG_SEND)
	{
		LogOut_MBE("[%s][MQTT-Send]:%s", AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32), szBuf);
	}
	else if(nLogType == MQTT_MBE_LOG_RECEIVE)
	{
		LogOut_MBE("[%s][MQTT-Recv]:%s", AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32), szBuf);
	}
	else if(nLogType == MQTT_MBE_LOG_SUBALLOK)
	{
		LogOut_MBE("[%s][MQTT-SubAll]:%s", AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32), szBuf);
	}
	else if(nLogType == MQTT_MBE_LOG_BEGIN_SUBALL)
	{
		LogOut_MBE("[%s][MQTT-BeginSub]:%s", AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32), szBuf);
	}
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
	MQTT_MBE_DATA_ROUGH* pMbeDev = (MQTT_MBE_DATA_ROUGH*)pReporter->pDataHeap;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	int n;
	RunThread_Heartbeat(pReporter->hThread); //in case the thread no response
	switch (emEvType)
	{
	case EVENT_TYPE_CHARGING:
		{
			TRACEX("MaintBE: Event_Handler  _EVENT_TYPE = EVENT_TYPE_CHARGING \t emMsgType = %d \n", emMsgType);
			EVENT_CHARGING * pEvChag = (EVENT_CHARGING*)pEvData;
			if (nEvDataSize != sizeof(EVENT_CHARGING))
			{
				return EVENT_HANDED_FAILED;
			}
			if(pEvChag->emGunNum > MAX_SUPPORT_GUN_NUM)
			{
				return EVENT_HANDED_FAILED;
			}

			pEvChag->emGunNum -= 1; //this is from 1~3

			if((EVENT_MSG_BEGIN == emMsgType) || (EVENT_MSG_END == emMsgType))
			{
				// if((pEvChag->emChangeType == CHG_TYPE_GUN_INCHARGE) && 
				// 	(pEvChag->emGunNum >= 0) && (pEvChag->emGunNum < Max_Connctors))
				// {
				// 	Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
				// 	pOCPP->emHandlingConnector = pEvChag->emGunNum;
				// 	pOCPP->bTransMsgFromHistory = FALSE;
				// 	TRACEX("pEvChag->emGunNum = %d \n", pEvChag->emGunNum);
				// 	//Now call the msg handler to create the msg to send
				// 	if(EVENT_MSG_END == emMsgType)
				// 	{
				// 		A_BLOB_RUN_CHARGE_INFO  StopTx;
				// 		PARAM_VAL_GET valGet;
				// 		valGet.nGetSize = sizeof(A_BLOB_RUN_CHARGE_INFO);
				// 		valGet.val.blobValue = (BYTE*)&StopTx;
				// 		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				// 			pEvChag->emGunNum + 201, 107, &valGet, sizeof(PARAM_VAL_GET));
				// 		//if(!StopTx.extraData.bHasStoppedByDeAuth) //in this case we already send it once Suspend the EVSE, do not send it repeatedly
				// 		if(StopTx.em_V2X_Status >= V2X_STATE_V2B)
				// 		{
				// 			pOCPP->stMsgInfoList[C_252_DischgSummary].pfnMsgHandler((void*)pOCPP);
				// 		}
				// 		else
				// 		{
				// 			pOCPP->stMsgInfoList[C_203_Summary].pfnMsgHandler((void*)pOCPP);
				// 		}
				// 	}
				// 	else
				// 	{
				// 		//reset 601 seq -> 0
				// 		pOCPP->stConnectorData[pEvChag->emGunNum].n_601_Sequence = 0;

				// 		A_BLOB_RUN_CHARGE_INFO  StartTx;
				// 		PARAM_VAL_GET valGet;
				// 		valGet.nGetSize = sizeof(A_BLOB_RUN_CHARGE_INFO);
				// 		valGet.val.blobValue = (BYTE*)&StartTx;
				// 		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				// 			pEvChag->emGunNum + 201, 107, &valGet, sizeof(PARAM_VAL_GET));
				// 		if(StartTx.em_V2X_Status >= V2X_STATE_V2B)
				// 		{
				// 			pOCPP->stConnectorData[pEvChag->emGunNum].bIsGun_InDischarge = TRUE;
				// 		}
				// 		else
				// 		{
				// 			pOCPP->stConnectorData[pEvChag->emGunNum].bIsGun_InDischarge = FALSE;
				// 		}
						
				// 		//pOCPP->stMsgInfoList[StartTransaction].pfnMsgHandler((void*)pOCPP);
				// 	}
				// 	// (EVENT_MSG_BEGIN == emMsgType) ? pOCPP->stMsgInfoList[StartTransaction].pfnMsgHandler((void*)pOCPP) :
				// 	// 	pOCPP->stMsgInfoList[StopTransaction].pfnMsgHandler((void*)pOCPP);
				// 	// if((emMsgType == EVENT_MSG_END) && (pOCPP->emOCPPConnStatus == Ready)) //when end of charging, will send out this once 
				// 	// {
				// 	// 	pOCPP->stConnectorData[pEvChag->emGunNum].bConn_StatusChanged++;
				// 	// 	//pOCPP->stConnectorData[pEvChag->emGunNum].nGunChargeStatus = GUN_STATE_IDLE;
				// 	// 	//pOCPP->emHandlingConnector = pEvChag->emGunNum;
				// 	// 	//pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP);
				// 	// }
				// 	Mutex_Unlock(pOCPP->hMutex);
				// }
			}
			else if(EVENT_MSG_STATUS_CHANGED == emMsgType)
			{
				if(pEvChag->emGunNum == (BYTE)0)
				{
					TRACEX("MaintBE: Status Change Event for The Connnector 0 pEvChag->emChangeType[%d] \n ",
						pEvChag->emChangeType);

					// if(pOCPP->tmLastCPStatusChange == 0)
					// {
					// 	return EVENT_HANDED_FAILED;
					// }

					//handel exit
					if(CHG_TYPE_SYS_INEXIT == pEvChag->emChangeType)
					{
						//Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
						//pOCPP->stMsgInfoList[C_102_StatusNotif].pfnMsgHandler((void*)pOCPP);
						pMbeDev->bIsNoticed_Exit = TRUE;
						//pOCPP->emOCPPConnStatus = Disconnected;
						//Mutex_Unlock(pOCPP->hMutex);
						return EVENT_HANDED_OK;
					}

					// A_BLOB_CAHRGEPOINT_STATUSINFO StatusNotiData;
					// valGet.nGetSize = sizeof(A_BLOB_CAHRGEPOINT_STATUSINFO);
					// valGet.val.blobValue = (BYTE*)&StatusNotiData;
					// err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
					// 	1, 10, &valGet, sizeof(PARAM_VAL_GET));
					// if (err != ERRCODE_DAI_OK)
					// {
					// 	return EVENT_HANDED_FAILED;
					// }

					// if((StatusNotiData.emChargePointStatus != pOCPP->blobCPStatusInfo.emChargePointStatus) ||
					// 	(StatusNotiData.emChargePointErrcode != pOCPP->blobCPStatusInfo.emChargePointErrcode))
					// {
					// 	Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
					// 	//pOCPP->nCurrChargeStatusHandled_Conn = pEvChag->emGunNum; // for Zero connector 
					// 	//pOCPP->byCuurConn_ChgType = pEvChag->emChangeType;
					// 	memcpy(&pOCPP->blobCPStatusInfo, &StatusNotiData, sizeof(A_BLOB_CAHRGEPOINT_STATUSINFO));
					// 	pOCPP->bCPStatusChanged = TRUE;
					// 	//Now call the msg handler to create the msg to send
					// 	//pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP);
					// 	Mutex_Unlock(pOCPP->hMutex);
					// }
				}
				else
				{
					// TRACEX("Status Change Event for The Connnectors [%d] \n ", pEvChag->emGunNum + 1);
					// //Jimmy note20190301: always refresh all the statuses, to avoid faulted transfer from Finishing to Preparing
					// BOOL bStatusChange = FALSE;
					// //if(pEvChag->emChangeType == CHG_TYPE_GUN_PLUG)
					// {
					// 	valGet.nGetSize = 4;
					// 	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
					// 		201 + pEvChag->emGunNum, 102, &valGet, sizeof(PARAM_VAL_GET));
					// 	if (err != ERRCODE_DAI_OK)
					// 	{
					// 		return EVENT_HANDED_FAILED;
					// 	}
					// 	if(pOCPP->stConnectorData[pEvChag->emGunNum].nGunPlugStatus != valGet.val.emValue)
					// 	{
					// 		bStatusChange = TRUE;
					// 	}
					// 	//pluged / unpluged
					// 	pOCPP->stConnectorData[pEvChag->emGunNum].nGunPlugStatus = valGet.val.emValue;
					// }
					// //else if(pEvChag->emChangeType == CHG_TYPE_GUN_PREPARE_STAT)
					// {
					// 	valGet.nGetSize = 4;
					// 	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
					// 		201 + pEvChag->emGunNum, 99, &valGet, sizeof(PARAM_VAL_GET));
					// 	if (err != ERRCODE_DAI_OK)
					// 	{
					// 		return EVENT_HANDED_FAILED;
					// 	}
					// 	if(pOCPP->stConnectorData[pEvChag->emGunNum].nGunPrepareStatus != valGet.val.emValue)
					// 	{
					// 		bStatusChange = TRUE;
					// 	}
					// 	//prepare status
					// 	pOCPP->stConnectorData[pEvChag->emGunNum].nGunPrepareStatus = valGet.val.emValue;
					// }
					// //else
					// {
					// 	valGet.nGetSize = 4;
					// 	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
					// 		201 + pEvChag->emGunNum, 101, &valGet, sizeof(PARAM_VAL_GET));
					// 	if (err != ERRCODE_DAI_OK)
					// 	{
					// 		return EVENT_HANDED_FAILED;
					// 	}
					// 	if(pOCPP->stConnectorData[pEvChag->emGunNum].nGunChargeStatus != valGet.val.emValue)
					// 	{
					// 		bStatusChange = TRUE;
					// 	}
					// 	pOCPP->stConnectorData[pEvChag->emGunNum].nGunChargeStatus = valGet.val.emValue;
					// }
					// //added by Jimmy 20190606 for suspendEVSE status change
					// {
					// 	valGet.nGetSize = 4;
					// 	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
					// 		201 + pEvChag->emGunNum, 120, &valGet, sizeof(PARAM_VAL_GET));
					// 	if (err == ERRCODE_DAI_OK)
					// 	{
					// 		if(pOCPP->stConnectorData[pEvChag->emGunNum].nGunSuspendEVSESatus != valGet.val.emValue)
					// 		{
					// 			bStatusChange = TRUE;
					// 		}
					// 		pOCPP->stConnectorData[pEvChag->emGunNum].nGunSuspendEVSESatus = valGet.val.emValue;
					// 	}
					// }

				// 	TRACEX("The gun[%d] statuses = [%d, %d, %d], bStatusChg = %d\n", pEvChag->emGunNum + 1,
				// 		pOCPP->stConnectorData[pEvChag->emGunNum].nGunPlugStatus,
				// 		pOCPP->stConnectorData[pEvChag->emGunNum].nGunPrepareStatus,
				// 		pOCPP->stConnectorData[pEvChag->emGunNum].nGunChargeStatus,
				// 		bStatusChange);
				// 	if(!bStatusChange)
				// 	{
				// 		return EVENT_HANDED_FAILED;
				// 	}

				// 	//if first time for connector, main entry will handle this
				// 	if(pOCPP->stConnectorData[pEvChag->emGunNum].tmLastConn_StatusChange == 0)
				// 	{
				// 		return EVENT_HANDED_FAILED;
				// 	}

				// 	Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
				// 	//pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP, TRUE);
				// 	//pOCPP->stConnectorData[pEvChag->emGunNum].bConn_StatusChanged = TRUE;
				// 	if(pOCPP->emOCPPConnStatus == Ready)
				// 	{
				// 		//trans-related msg should be handled urgently
				// 		// if(pOCPP->stConnectorData[pEvChag->emGunNum].nGunChargeStatus >= GUN_STATE_WAITING_START)
				// 		// {
				// 		// 	pOCPP->emHandlingConnector = pEvChag->emGunNum;
				// 		// 	pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP);
				// 		// }
				// 		// else
				// 		{
				// 			TRACEX("NEEEEEE[%d] bStatusChg = %d\n", pEvChag->emGunNum + 1, pOCPP->stConnectorData[pEvChag->emGunNum].bConn_StatusChanged);
				// 			pOCPP->stConnectorData[pEvChag->emGunNum].bConn_StatusChanged++;
				// 		}
				// 	}
				// 	Mutex_Unlock(pOCPP->hMutex);
				}
			}
		}
		break;
	case EVENT_TYPE_FIRMWARE_UPDATE:
		{
			TRACEX("MaintBE: Event_Handler  _EVENT_TYPE = EVENT_TYPE_FIRMUPDATE \t emMsgType = %d, size[%d,%d] \n", 
				emMsgType, nEvDataSize, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
			EVENT_FIRMWAREUPDATE_STATUS * pFirmUpSta = (EVENT_FIRMWAREUPDATE_STATUS*)pEvData;
			if (nEvDataSize != sizeof(EVENT_FIRMWAREUPDATE_STATUS))
			{
				return EVENT_HANDED_FAILED;
			}
			Mutex_Lock(pMbeDev->hMutex, WAIT_INFINITE);
			pMbeDev->emFirmwareUpgradeStatus = pFirmUpSta->emUpdateStatus;
			TRACEX("MaintBE: emFirmwareUpgradeStatus =%d\n",pMbeDev->emFirmwareUpgradeStatus);
			//Now call the msg handler to create the msg to send
			pMbeDev->bIsStatusChanged = TRUE;
			Mutex_Unlock(pMbeDev->hMutex);
		}
		break;
	case EVENT_TYPE_DIAGNOSTIC_UPLOAD:
		{
			TRACEX("OCPP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_DIAGNOSUPLOAD \t emMsgType = %d \n", emMsgType);
			EVENT_DIAGNOUPLOAD_STATUS * pDiagUpSta = (EVENT_DIAGNOUPLOAD_STATUS*)pEvData;
			if (nEvDataSize != sizeof(EVENT_DIAGNOUPLOAD_STATUS))
			{
				return EVENT_HANDED_FAILED;
			}
			Mutex_Lock(pMbeDev->hMutex, WAIT_INFINITE);
			pMbeDev->emDiagnosUploadStatus = pDiagUpSta->emUploadStatus;
			pMbeDev->bIsStatusChanged = TRUE;
			//Now call the msg handler to create the msg to send
			//pOCPP->stMsgInfoList[C_407_StatOfDiag].pfnMsgHandler((void*)pOCPP);
			Mutex_Unlock(pMbeDev->hMutex);
		}
		break;
	case EVENT_TYPE_ABILITY_CHANGE:
		{
			TRACEX("MaintBE: Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_ABILITY_CHG * pEvAbChg = (EVENT_ABILITY_CHG*)pEvData;
			if (nEvDataSize != sizeof(EVENT_ABILITY_CHG))
			{
				return EVENT_HANDED_FAILED;
			}
			if(pEvAbChg->nWhichGun > MAX_SUPPORT_GUN_NUM)
			{
				return EVENT_HANDED_FAILED;
			}

			pMbeDev->bIsStatusChanged = TRUE;

			//pEvAbChg->nWhichGun -= 1; //this is from 1~3
			
			// if(pEvAbChg->nWhichGun == Connctor_0)
			// {
			// 	TRACEX("Ability Change Event for The Connnector 0  \n ");
			// 	A_BLOB_CAHRGEPOINT_STATUSINFO StatusNotiData;
			// 	valGet.nGetSize = sizeof(A_BLOB_CAHRGEPOINT_STATUSINFO);
			// 	valGet.val.blobValue = (BYTE*)&StatusNotiData;
			// 	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			// 		1, 10, &valGet, sizeof(PARAM_VAL_GET));
			// 	if (err != ERRCODE_DAI_OK)
			// 	{
			// 		return EVENT_HANDED_FAILED;
			// 	}
			// 	if((StatusNotiData.emChargePointStatus != pOCPP->blobCPStatusInfo.emChargePointStatus) ||
			// 		(StatusNotiData.emChargePointErrcode != pOCPP->blobCPStatusInfo.emChargePointErrcode))
			// 	{
			// 		Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
			// 		//pOCPP->nCurrChargeStatusHandled_Conn = pEvChag->emGunNum; // for Zero connector 
			// 		//pOCPP->byCuurConn_ChgType = pEvChag->emChangeType;
			// 		memcpy(&pOCPP->blobCPStatusInfo, &StatusNotiData, sizeof(A_BLOB_CAHRGEPOINT_STATUSINFO));
			// 		pOCPP->bCPStatusChanged = TRUE;
			// 		//Now call the msg handler to create the msg to send
			// 		//pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP);
			// 		Mutex_Unlock(pOCPP->hMutex);
			// 	}
			// }
			// else
			// {
			// 	TRACEX("Ability Change Event for The Connnectors [%d] \n ", pEvAbChg->nWhichGun);
			// 	valGet.nGetSize = 4;
			// 	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			// 		201 + pEvAbChg->nWhichGun, 101, &valGet, sizeof(PARAM_VAL_GET));
			// 	if (err != ERRCODE_DAI_OK)
			// 	{
			// 		return EVENT_HANDED_FAILED;
			// 	}
			// 	if(pOCPP->stConnectorData[pEvAbChg->nWhichGun].nGunChargeStatus == valGet.val.emValue)
			// 	{
			// 		return EVENT_HANDED_FAILED;
			// 	}
			// 	pOCPP->stConnectorData[pEvAbChg->nWhichGun].nGunChargeStatus = valGet.val.emValue;

			// 	//Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
			// 	//pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP, TRUE);
			// 	//pOCPP->stConnectorData[pEvAbChg->nWhichGun].bConn_StatusChanged = TRUE;
			// 	if(pOCPP->emOCPPConnStatus == Ready)
			// 	{
			// 		pOCPP->stConnectorData[pEvAbChg->nWhichGun].bConn_StatusChanged++;
			// 	}
			// 	//Mutex_Unlock(pOCPP->hMutex);
			// }

		}
		break;

	case EVENT_TYPE_SYSTEM_EXIT:
		{
			EVENT_SYSTEM_EXIT* pEvReboot = (EVENT_SYSTEM_EXIT*)pEvData;
			if(nEvDataSize != sizeof(EVENT_SYSTEM_EXIT))
			{
				return EVENT_HANDED_FAILED;
			}
			TRACEX("MaintBE: Event_Handler  _EVENT_TYPE = EVENT_TYPE_EXIT, EXIT_TYPE = %d \n", pEvReboot->nExitType);
			//Jimmy Note20190112: no need to skip BootNotif
			//if(pEvReboot->nExitType != SYS_EXIT_HARD_REBOOT)
			//{
			//	pOCPP->bBootNotifyDone = TRUE;
			//}

			//pOCPP->bWaitRebootMsg_Ready = TRUE;
			pMbeDev->bIsNoticed_Exit = TRUE;
		}
		break;

	case EVENT_TYPE_CFG_CHANGED:
		{
			TRACEX("MaintBE: Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_PARAM_CHG * pParamChg = (EVENT_PARAM_CHG*)pEvData;
			if (nEvDataSize != sizeof(EVENT_PARAM_CHG))
			{
				return EVENT_HANDED_FAILED;
			}

			//Jimmy note: this need to be implemented whenever parameter changes 
			if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 81)) //server addr change
			{
				PARAM_VAL_GET valGet;
				valGet.nGetSize = 32;
				char szMqttServerAddr[32];
				valGet.val.blobValue = (BYTE*)szMqttServerAddr;
				ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 81, &valGet, sizeof(PARAM_VAL_GET));
				if (err == ERRCODE_DAI_OK)
				{
					if(strcmp(szMqttServerAddr, pMbeDev->szMqttServerAddress)!= 0)
					{
						snprintf(pMbeDev->szMqttServerAddress, 32, "%s", szMqttServerAddr);
						pMbeDev->emMqttCommStatus = MBE_COMM_WAIT_REBUILD_CLIENT;
						pMbeDev->emOCPP_Disconn_Reason = MBEMQTT_COMM_FAIL_REASON_SERVERADDR_CHANGED;
					}	
				}	
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 17))
			{
				/* Jusge if SN has been changed from New BOOTINFO */
				PARAM_VAL_GET valGet;
				A_BLOB_CHARGER_BOOTINFO bootInfo;
				valGet.nGetSize = sizeof(A_BLOB_CHARGER_BOOTINFO);
				valGet.val.blobValue = (BYTE*)&bootInfo;
				ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 17, &valGet, sizeof(PARAM_VAL_GET));
				
				if(err == ERRCODE_DAI_OK)
				{
					if(strcmp(bootInfo.szChargePointSerualNuml, pMbeDev->szDevSN) != 0)
					{
						snprintf(pMbeDev->szDevSN, 25, "%s",  bootInfo.szChargePointSerualNuml);
						pMbeDev->emMqttCommStatus = MBE_COMM_WAIT_REBUILD_CLIENT;
						pMbeDev->emOCPP_Disconn_Reason = MBEMQTT_COMM_FAIL_REASON_SN_CHANGED;
					}
				}
			}
			// else if ((pParamChg->parmPTR.nSubId == 26))
			// {
			// 	// Read ESiOCPP Config 
			// 	TRACEX("Config Changed Evevt \n");
			// 	PARAM_VAL_GET valGet;
			// 	valGet.nGetSize = sizeof(A_BLOB_ES_iOCPP_CONFIGURAION);
			// 	valGet.val.blobValue = (BYTE*)&pOCPP->stESiOCPPSettings;
			// 	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			// 		1, 26, &valGet, sizeof(PARAM_VAL_GET));
			// 	if (err != ERRCODE_DAI_OK)
			// 	{
			// 		TRACEX("ERROR: OCPP settings can not be retrieved\n");
			// 		return FALSE;
			// 	}
			// }
		}
		break;
	case EVENT_TYPE_ALARM:
		{
			TRACEX("MBE_Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_ALARM * pAlmEv = (EVENT_ALARM*)pEvData;
			if (nEvDataSize != sizeof(EVENT_ALARM))
			{
				return EVENT_HANDED_FAILED;
			}

			Mutex_Lock(pMbeDev->hMutex, WAIT_INFINITE);		

			int kkk;
			int nFoundIdx;
			MBE_DEV_ALARM* pMbeAlm;
			if((EVENT_MSG_BEGIN == emMsgType) || (EVENT_MSG_RESTORE == emMsgType))
			{		
				pMbeDev->bIsStatusChanged = TRUE;
				TRACEX("------------3434343434344444444444545454545---------\n");
				pMbeAlm = pMbeDev->stCurrStatusData.actAlmTable;		
				nFoundIdx = -1;
				for(kkk = 0; kkk < pMbeDev->stCurrStatusData.nActiveAlmTotal; kkk++, pMbeAlm++)
				{
					if((pMbeAlm->nAlarmId == pAlmEv->nAlarmId) && 
						(pMbeAlm->nDevId == pAlmEv->nDevId))
					{
						nFoundIdx = kkk;
						pMbeAlm->tmStart = pAlmEv->tmStart;
						break;
					}
				}

				if(nFoundIdx == -1)
				{
					if(pMbeDev->stCurrStatusData.nActiveAlmTotal >= MBE_DEV_MAX_SUPPORT_ACTALM)
					{
						pMbeAlm = &pMbeDev->stCurrStatusData.actAlmTable[MBE_DEV_MAX_SUPPORT_ACTALM - 1];						
					}
					else
					{
						pMbeAlm = &pMbeDev->stCurrStatusData.actAlmTable[pMbeDev->stCurrStatusData.nActiveAlmTotal];
						pMbeDev->stCurrStatusData.nActiveAlmTotal++;
					}
					pMbeAlm->nAlarmId = pAlmEv->nAlarmId;
					pMbeAlm->nDevId = pAlmEv->nDevId;
					pMbeAlm->tmStart = pAlmEv->tmStart;
					// printf("-----------NEWALAMR[%d] : [%d, %d, %d]--------\n", pCurr102Stat->nActiveAlmTotal,
					// 	pMbeAlm->nAlarmId, pMbeAlm->nDevId, pMbeAlm->tmStart);
				}
			}
			else 
			{
				pMbeAlm = pMbeDev->stCurrStatusData.actAlmTable;	
				nFoundIdx = -1;
				for(kkk = 0; kkk < pMbeDev->stCurrStatusData.nActiveAlmTotal; kkk++, pMbeAlm++)
				{
					if((pMbeAlm->nAlarmId == pAlmEv->nAlarmId) && 
						(pMbeAlm->nDevId == pAlmEv->nDevId))
					{
						nFoundIdx = kkk;
						break;
					}
				}
				if(nFoundIdx != -1)
				{
					TRACEX("------------536767676878786767676777676767---------\n");
					pMbeDev->bIsStatusChanged = TRUE;
					pMbeAlm = &pMbeDev->stCurrStatusData.actAlmTable[nFoundIdx];
					// printf("-----------ENDALAMR[%d] : [%d, %d, %d]--------\n", pCurr102Stat->nActiveAlmTotal,
					// 	pMbeAlm->nAlarmId, pMbeAlm->nDevId, pMbeAlm->tmStart);
					//now remove that alarm from list
					for(kkk = nFoundIdx; kkk < pMbeDev->stCurrStatusData.nActiveAlmTotal - 1; kkk++)
					{
						memmove(&pMbeDev->stCurrStatusData.actAlmTable[kkk], &pMbeDev->stCurrStatusData.actAlmTable[kkk + 1], sizeof(MBE_DEV_ALARM));
					}
					pMbeDev->stCurrStatusData.nActiveAlmTotal--;
				}				
			}

			Mutex_Unlock(pMbeDev->hMutex);
		}
		break;
	default:
		break;
	}
	return EVENT_HANDED_OK;
}

// static void onSubMsg_Delivered(void *context, MQTTAsync_token token) 
// {
// 	MQTT_MBE_DATA_ROUGH *pMbeDev = (MQTT_MBE_DATA_ROUGH*)context;
//     if(pMbeDev == NULL)
//     {
// 		TRACEX("Callback_Delivered: pMqtt is NULL\n"); 
//         return;  
//     }
//  	// TRACEX("Message with token value %d delivery confirmed. clientid=[%s] pubtopic=[%s] szSubTopic=[%s]\n", \
//     //         dt ,pMqtt->szClientID, pMqtt->szPubTopic, pMqtt->szSubTopic);
//     // Callback_Deliveredtoken = dt;
// 	int n;
// 	for(n = 0; n < MAX_NUM_MBE_MSG_PUB; n++)
// 	{
// 		if(pMbeDev->stPubMsgList[n].msgToken == token)
// 		{
// 			pMbeDev->stPubMsgList[n].tmLastSent = _NOW_;
// 			break;
// 		}
// 	}
// }
//Jimmy note: !!!!!do NOT use 'free' but need use 'MQTTAsync_free' because the lib re-defined the HEAP
#define MQTT_SAFE_DELETE(p)	\
	do { if ((p) != NULL) { MQTTAsync_free(p); (p) = NULL; } } while(0)
static int SafeFree_ArrivedMsg(char *topicName, MQTTAsync_message *message)
{
	//Jimmy note: should return > 0 so as to resolve the double free problem
	if(message != NULL)
	{
		MQTTProperties_free(&message->properties);
		MQTT_SAFE_DELETE(message->payload);
		MQTT_SAFE_DELETE(message);	
	}	
	MQTT_SAFE_DELETE(topicName);
	//TRACEX("-------Now free the ASYNCCCCCCCC messages!!!!!!!!------\n");
	// MQTTAsync_freeMessage(&message); 
	// MQTTAsync_free(topicName);
	return 1;
}

static int onSubMsg_Arrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    MQTT_MBE_DATA_ROUGH *pMbeDev = (MQTT_MBE_DATA_ROUGH*)context;
	if(pMbeDev == NULL)
	{
		return;
	}    
	REPORTER* pReporter = (REPORTER*)pMbeDev->hReporter;
	if((message == NULL) || (context == NULL))
	{
		TRACEX("__________RRRRRRRRRRRRRRRRRR_________\n");
		return SafeFree_ArrivedMsg(topicName, message);
	}

    char *pszPos = (char*)message->payload;
    //char *pszEnd = NULL;

	//filter those invalid topic/messages
	if((message->payloadlen < 0) || (topicName == NULL)/* || (topicLen < 3)*/) //topicLen maybe 0 as of the remaing topics in queue
	{
		TRACEX("INVALIDE topic ->[%s, %d], message->payloadlen->[%d]\n", (topicName == NULL) ? "NULL" : topicName, 
			topicLen, message->payloadlen);
		return SafeFree_ArrivedMsg(topicName, message);
	}

	TRACEX("topicsssss[%s, %d]-arravied, payloadlen=[%d], payload->[%s]\n", 
		topicName, topicLen, message->payloadlen, 
		(message->payload == NULL) ? "NULL" : message->payload);
	
	LOG_MQTT(MQTT_MBE_LOG_RECEIVE, "topic->[%s], msg->[%s]", topicName, 
		(message->payload == NULL) ? "NULL" : message->payload);
	
	if((pMbeDev->emMqttCommStatus == MBE_COMM_DISCONECTED) ||
		(pMbeDev->emMqttCommStatus == MBE_COMM_CONNECTED) ||
		(pMbeDev->emMqttCommStatus == MBE_COMM_SUBSCRIBING) ||
		(pMbeDev->emMqttCommStatus == MBE_COMM_WAIT_REBUILD_CLIENT))
	{
		TRACEX("----MBE-Client is NOT ready to received ANY topic, emMqttCommStatus = [%d]!!!----\n", pMbeDev->emMqttCommStatus);
		return SafeFree_ArrivedMsg(topicName, message);
	}

	//now find the msg according to topic name
	MBE_DEV_SUB_MESSAGE* pFoundSubMsg = NULL;
	int n;
	for(n = 0; n < MAX_NUM_MBE_MSG_SUB; n++)
	{
		if(strcmp(topicName, pMbeDev->stSubMsgList[n].szTopicName) == 0)
		{
			pFoundSubMsg = &pMbeDev->stSubMsgList[n];
			break;
		}
	}

	if(pFoundSubMsg == NULL)
	{		
		TRACEX("WRONG TOPIC FOUND!!!");
        return SafeFree_ArrivedMsg(topicName, message);
	}	
    
	TRACEX("---Successfully find the subcribed-msg->[%s] and now execute the parser of it---\n", pFoundSubMsg->szMsgName);

	if(pszPos == NULL)
    {       
		if(pFoundSubMsg->emMsgId == MBE_MSG_TRIG_STAT)
		{
			pMbeDev->pCurrJson_Sub = NULL;
			pFoundSubMsg->tmLastArrived = _NOW_;
			pMbeDev->emInHandling_SubMsg = pFoundSubMsg->emMsgId;
			pFoundSubMsg->pfnParser((void*)pMbeDev);
		}
		else
		{
			 TRACEX("Wrong Msg message->payload is NULL: \n out!!");
		}

        return SafeFree_ArrivedMsg(topicName, message);
    }
    
    if(message->payloadlen <= 0)
    {        
		if(pFoundSubMsg->emMsgId == MBE_MSG_TRIG_STAT)
		{
			pMbeDev->pCurrJson_Sub = NULL;
			pFoundSubMsg->tmLastArrived = _NOW_;
			pMbeDev->emInHandling_SubMsg = pFoundSubMsg->emMsgId;
			pFoundSubMsg->pfnParser((void*)pMbeDev);
		}
		else
		{
			TRACEX("Wrong Msg message->paylen=[%d] \n out!!", message->payloadlen);
		}
        return SafeFree_ArrivedMsg(topicName, message);
    }

    //pMqtt->nReadBuffLen = message->payloadlen;
	//memset(pMqtt->szReadBuffer, 0 ,MAX_MQTT_BUFFER_LEN);
	//memcpy(pMqtt->szReadBuffer, pszPos, pMqtt->nReadBuffLen);
	//DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_READ, pMqtt->szReadBuffer);

    //while(pMqtt->nReadBuffLen > 0)
    //{
        //cJSON *json = NULL;
        //json = cJSON_ParseWithOpts(pszPos,&pszEnd, FALSE);
		pMbeDev->pCurrJson_Sub = cJSON_Parse(pszPos);
        if(pMbeDev->pCurrJson_Sub == NULL)
        {
            TRACEX("Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
            TRACEX(" Here IS the buffer %s \n", pszPos);
            return SafeFree_ArrivedMsg(topicName, message);
        }

		pMbeDev->emInHandling_SubMsg = pFoundSubMsg->emMsgId;
		pFoundSubMsg->tmLastArrived = _NOW_;
		pFoundSubMsg->pfnParser((void*)pMbeDev);
        cJSON_Delete(pMbeDev->pCurrJson_Sub);     //!!! do not forget to destroy the json
        //now continue parsing for another frame
        // TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszPos - pszEnd));
        // pMqtt->nReadBuffLen -= (pszEnd - pszPos);
        // TRACEX("Len pMqtt->nReadBuffLen : %d \n  ", pMqtt->nReadBuffLen);
        // pszPos = pszEnd;
    //}    
	return SafeFree_ArrivedMsg(topicName, message);
}

static void onConnLost(void *context, char *cause)
{
    MQTT_MBE_DATA_ROUGH *pMbeDev = (MQTT_MBE_DATA_ROUGH*)context;
	if(pMbeDev == NULL)
	{
		return;
	}    
    TRACEX("\n################Warning:Connection lost cause: %s######################\n", (cause == NULL) ? "NULL" : cause);
 	pMbeDev->bIs_MqttConnOK = FALSE;
	
	pMbeDev->emMqttCommStatus = MBE_COMM_DISCONECTED;
	pMbeDev->emOCPP_Disconn_Reason = MBEMQTT_COMM_FAIL_SERVER_LOSTCONN;
	LOG_MQTT(MQTT_MBE_LOG_DISCONNECT, "Lost connection to broker!!!");

	//Log_ConnectStatuCode(pMqtt);
}

// static void onDisconnect_ClientOK(void* context, MQTTAsync_successData* response)
// {       
//     MQTT_MBE_DATA_ROUGH *pMbeDev = (MQTT_MBE_DATA_ROUGH*)context;  
//     pMbeDev->emMqttCommStatus = MBE_COMM_DISCONECTED;
// 	if(pMbeDev->emOCPP_Disconn_Reason == MBEMQTT_COMM_FAIL_REASON_SN_CHANGED)
// 	{
// 		ReCreate_Client_AndInit(pMbeDev);
// 	}
// 	//Log_ConnectStatuCode(pMqtt);
// }

static void Disconnect_MqttClientComm(MQTT_MBE_DATA_ROUGH *pMbeDev)
{       
    TRACEX("---Handle_DisconnectComm---\n");
    int rc = MQTTASYNC_SUCCESS;
    MQTTAsync client = pMbeDev->client;
    //MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;

	//Jimmy added 20221102 to unsub all old subscribtions before new-sub
	rc = MQTTAsync_unsubscribe(client, "mbe/#", NULL);
	LOG_MQTT(MQTT_MBE_LOG_DISCONNECT, "Now UnSubscibe All-Msgs before disconnect, rc->%d", rc);
	Sleep(1000);

	pMbeDev->tmLastSend_DisConn = _NOW_;  
	pMbeDev->emMqttCommStatus = MBE_COMM_DISCONECTED;
    //opts.onSuccess = onDisconnect_ClientOK;
    //opts.context = client;	
    //if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS)
	if((rc = MQTTAsync_disconnect(client, NULL)) != MQTTASYNC_SUCCESS)
    {
        TRACEX("Failed to Disconnect, return code %d\n", rc);
		//return;
    }	
	LOG_MQTT(MQTT_MBE_LOG_DISCONNECT, "reason->[%d], rc->%d", pMbeDev->emOCPP_Disconn_Reason, rc);
	pMbeDev->bIs_MqttConnOK = FALSE;//means already disconn
}

//static void onSubscribe_OK(void* context, MQTTAsync_successData* response)
//static void onSubscribe_OK(void* context, MQTTAsync_successData5* response)
static void onSubscribe_OK(void* context, void* response)
{
    MQTT_MBE_DATA_ROUGH *pMbeDev = (MQTT_MBE_DATA_ROUGH*)context;
    if(pMbeDev == NULL)
    {
        TRACE("onSubscribe: context is NULL\n");
        return;
    }
	TRACEX("Info: Topic Subscribe succeeded\n");

	int n;
	BOOL bAllSubScribeOK = TRUE;
	MQTTAsync_token token = pMbeDev->bIsMqttVersion5 ? ((MQTTAsync_successData5*)response)->token :
			((MQTTAsync_successData*)response)->token;
	for(n = 0; n < MAX_NUM_MBE_MSG_SUB; n++)
	{
		if(pMbeDev->stSubMsgList[n].msgToken == token)
		{
			TRACEX("----[%d], token[%d], topicName -> [%s] Subscribe succeeded!----\n", 
				pMbeDev->stSubMsgList[n].emMsgId, token, pMbeDev->stSubMsgList[n].szTopicName);
			pMbeDev->stSubMsgList[n].bIsSubscribedOK = TRUE;
		}	
		
		if(!pMbeDev->stSubMsgList[n].bIsSubscribedOK)
		{
			bAllSubScribeOK = FALSE;
		}	
	}	

	if(bAllSubScribeOK)
	{
		LOG_MQTT(MQTT_MBE_LOG_SUBALLOK, "OK. Now ready for AUTH!!!");	
		TRACEX("-----NOW all the subscribing are DONE!!!!----\n");
		if(MBE_COMM_SUBSCRIBING == pMbeDev->emMqttCommStatus)
		{
			pMbeDev->emMqttCommStatus = MBE_COMM_WAIT_AUTH;
		}		
	}
}

//static void onSubscribe_Failed(void* context, MQTTAsync_failureData* response)
//static void onSubscribe_Failed(void* context, MQTTAsync_failureData5* response)
// static void onSubscribe_Failed(void* context, void* response)
// {
//     if(context == NULL)
//     {
//         TRACEX("onSubscribeFailure: context is NULL\n");
//     }
// 	//will handle it later, we think this can not happen normally
// }

static char* MakeTopic_4SubMsg(MQTT_MBE_DATA_ROUGH *pMbeDev, MBE_DEV_SUB_MSGID emMsgId)
{
	MBE_DEV_SUB_MESSAGE* pSubMsg = &pMbeDev->stSubMsgList[emMsgId];
#ifdef ___TEST_MQTT_MBE__
#define MBE_SERVER_TOPIC_PREFIX_TEST	"mbe/ser-test"
	snprintf(pSubMsg->szTopicName, 64, MBE_SERVER_TOPIC_PREFIX_TEST"/%s", pSubMsg->szMsgName);
#else
	if(MBE_MSG_BRSD_ALL == emMsgId)
	{
		snprintf(pSubMsg->szTopicName, 64, MBE_SERVER_TOPIC_PREFIX"/%s", pSubMsg->szMsgName);
	}
	else if(MBE_MSG_BRSD_ME == emMsgId)
	{
		snprintf(pSubMsg->szTopicName, 64, MBE_SERVER_TOPIC_PREFIX"/%s/%s", pSubMsg->szMsgName, pMbeDev->szDevSN);
	}
	else
	{
		snprintf(pSubMsg->szTopicName, 64, MBE_SERVER_TOPIC_PREFIX"/%s/%s", pMbeDev->szCurrClientId, pSubMsg->szMsgName);
	}
#endif
	return pSubMsg->szTopicName;
}

#define INIT_AMQTT_SUBMSG(_pMbeDev, _pMsg, _emMsgId, _szMsgName, _nQoS, _pfnMsgHandler) \
	do{\
	(_pMsg)->emMsgId = (_emMsgId); \
	snprintf((_pMsg)->szMsgName, 16,  (_szMsgName)); \
	(_pMsg)->qos = (_nQoS); \
	(_pMsg)->pfnParser = (_pfnMsgHandler); \
	MakeTopic_4SubMsg((_pMbeDev), _emMsgId); \
	(_pMsg)->bIsSubscribedOK = FALSE;  \
	(_pMsg)->msgToken = (-99999); \
	(_pMsg)++; \
	}while(0)

static void MakeSubTopicInfo_List_ForMany(MQTT_MBE_DATA_ROUGH *pMbeDev, char ppszTopiclist[][128], int pnQosList[])
{
	//then init all the sub msg topic info
	MBE_DEV_SUB_MESSAGE* p = &(pMbeDev->stSubMsgList[0]);
	INIT_AMQTT_SUBMSG(pMbeDev, p, 	MBE_MSG_BRSD_ALL, 	"brsd",			QoS_0,		Handle_Topic_Brsd);
	INIT_AMQTT_SUBMSG(pMbeDev, p, 	MBE_MSG_BRSD_ME, 	"brsd",			QoS_0,		Handle_Topic_Brsd);
	INIT_AMQTT_SUBMSG(pMbeDev, p, 	MBE_MSG_AUTH_OK, 	"auth.ok",		QoS_0,		Handle_Topic_AuthOK);
	INIT_AMQTT_SUBMSG(pMbeDev, p, 	MBE_MSG_AUTH_NOK, 	"auth.nok",		QoS_0,		Handle_Topic_AuthNOK);
	INIT_AMQTT_SUBMSG(pMbeDev, p, 	MBE_MSG_TRIG_STAT, 	"trig.stat",	QoS_0,		Handle_Topic_TrigStat);
	INIT_AMQTT_SUBMSG(pMbeDev, p, 	MBE_MSG_CFG_REQ, 	"cfg.req",		QoS_0,		Handle_Topic_CfgReq);
	INIT_AMQTT_SUBMSG(pMbeDev, p, 	MBE_MSG_PARAM_REQ, 	"param.req",	QoS_0,		Handle_Topic_ParamReq);
	INIT_AMQTT_SUBMSG(pMbeDev, p, 	MBE_MSG_SET_REQ, 	"set.req",		QoS_0,		Handle_Topic_SetReq);
	INIT_AMQTT_SUBMSG(pMbeDev, p, 	MBE_MSG_CTRL_REQ, 	"ctrl.req",		QoS_0,		Handle_Topic_CtrlReq);
	INIT_AMQTT_SUBMSG(pMbeDev, p, 	MBE_MSG_ERR_SERVER, "error",		QoS_0,		Handle_Topic_ServError);
	//now make the topic list
	int n;
	for(n = 0; n < MAX_NUM_MBE_MSG_SUB; n++)
	{
		TRACEX("------[%d]: Now make Topic -> [%s]-----\n", n + 1, pMbeDev->stSubMsgList[n].szTopicName);
		snprintf(ppszTopiclist[n], 128, "%s", pMbeDev->stSubMsgList[n].szTopicName);
		pnQosList[n] = pMbeDev->stSubMsgList[n].qos;
	}
}

static void Subscrible_AllTopics(MQTT_MBE_DATA_ROUGH *pMbeDev)
{               
    int rc = 0;
 	MQTTAsync client = pMbeDev->client;  
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    
    // TRACEX("Subscribing to topic %s\nfor client %s using QoS%d\n\n", \
    //         pMqtt->szSubTopic, pMqtt->szClientID, QOS);
	if(pMbeDev->bIsMqttVersion5)
	{
		opts.onSuccess5 = onSubscribe_OK;
		//opts.onFailure5 = onSubscribe_Failed;		
	}
	else
	{
		opts.onSuccess = onSubscribe_OK;
		//opts.onFailure = onSubscribe_Failed;		
	}
	
	opts.context = pMbeDev;

	char szTopicList[MAX_NUM_MBE_MSG_SUB][128];
	int QosList[MAX_NUM_MBE_MSG_SUB];
	TRACEX("-------Now begin preparing needed subscribed all topics!!!-----\n");
	MakeSubTopicInfo_List_ForMany(pMbeDev, szTopicList, QosList);
	pMbeDev->tmLast_CallSubscribe = _NOW_;
	pMbeDev->emMqttCommStatus = MBE_COMM_SUBSCRIBING;
	TRACEX("-------Now begin subscribing all topics!!!-----\n");
	//now call the sub-many api to sub all needed msg
	//--------DON't know why sub-Many NOT work
    // if((rc = MQTTAsync_subscribeMany(client, MAX_NUM_MBE_MSG_SUB, szTopicList, QosList, &opts)) != MQTTASYNC_SUCCESS)
	// {
	// 	TRACEX("Failed to start subscribe, return code %d\n", rc);
	// 	return;
	// }
	LOG_MQTT(MQTT_MBE_LOG_BEGIN_SUBALL, "Now begin subscribe ALL-msgs from broker!!!");	

	int n;
	for(n = 0; n < MAX_NUM_MBE_MSG_SUB; n++)
	{
		if(pMbeDev->stSubMsgList[n].bIsSubscribedOK)
		{
			continue;
		}
		//opts.token = pMbeDev->stSubMsgList[n].emMsgId + 1; //just for identify for subscribing
		TRACEX("[%d]: Now to start subscribe ->[%s], token[%d]\n", n+1, szTopicList[n], opts.token);
		if ((rc = MQTTAsync_subscribe(client, szTopicList[n], QosList[n], &opts)) != MQTTASYNC_SUCCESS)
		{
			TRACEX("Failed to start subscribe ->[%s], return code %d\n", szTopicList[n], rc);
			LOG_MQTT(MQTT_MBE_LOG_BEGIN_SUBALL, "Failed to start subscribe ->[%s], return code %d\n", szTopicList[n], rc);	
			//Jimmy: don't return, just continue and check it in callback
			//return;
		}
		pMbeDev->stSubMsgList[n].msgToken = opts.token; //for match usage
	}	

	//needs to reset all tokens so that the match will still work after reconnection
	for(n = 0; n < MAX_NUM_MBE_MSG_PUB; n++)
	{
		pMbeDev->stPubMsgList[n].msgToken = -9999;
	}

    //TRACEX("Good rc from subscribe rc=[%d] MQTTASYNC_SUCCESS\n", rc);
}

static void onPubMsg_OK(void* context, void* resp)
{	
	MQTT_MBE_DATA_ROUGH *pMbeDev = (MQTT_MBE_DATA_ROUGH*)context;
	//Jimmy note: for QoS == 0, we use topic to match; for QoS > 0, the topic is NULL so we use token to match
	// TRACEX("----The token[%d], topic[%s], payloadlen->[%d], Qos[%d], payload[%s], received a pubMsgOK event!!!---\n",
	// 	response->token,
	// 	(response->alt.pub.destinationName == NULL) ? "NULL" : response->alt.pub.destinationName,
	// 	response->alt.pub.message.payloadlen, response->alt.pub.message.qos,
	// 	(response->alt.pub.message.payload == NULL) ? "NULL" : response->alt.pub.message.payload);

	if((pMbeDev->emMqttCommStatus == MBE_COMM_DISCONECTED) ||
		(pMbeDev->emMqttCommStatus == MBE_COMM_CONNECTED) ||
		(pMbeDev->emMqttCommStatus == MBE_COMM_SUBSCRIBING) ||
		(pMbeDev->emMqttCommStatus == MBE_COMM_WAIT_REBUILD_CLIENT))
	{
		return;
	}

   	int n;
	MBE_DEV_PUB_MESSAGE* pMatchedPubMsg = NULL;
	for(n = 0; n < MAX_NUM_MBE_MSG_PUB; n++)
	{
		//Jimmy: accorindg to src, the topic can be used here to match
		//and the token is not useful for QoS == 0
		if(pMbeDev->bIsMqttVersion5)
		{
			MQTTAsync_successData5* response = (MQTTAsync_successData5*)resp;
			if(response->alt.pub.destinationName != NULL)
			{
				if(strcmp(response->alt.pub.destinationName, pMbeDev->stPubMsgList[n].szTopicName) == 0)
				{
					pMatchedPubMsg = &pMbeDev->stPubMsgList[n];
					break;
				}
			}
			else if(response->token > 0)  //should be from 1
			{
				if(pMbeDev->stPubMsgList[n].msgToken == response->token)
				{
					pMatchedPubMsg = &pMbeDev->stPubMsgList[n];
					break;
				}			
			}
		}
		else
		{
			MQTTAsync_successData* response = (MQTTAsync_successData*)resp;
			if(response->alt.pub.destinationName != NULL)
			{
				if(strcmp(response->alt.pub.destinationName, pMbeDev->stPubMsgList[n].szTopicName) == 0)
				{
					pMatchedPubMsg = &pMbeDev->stPubMsgList[n];
					break;
				}
			}
			else if(response->token > 0)  //should be from 1
			{
				if(pMbeDev->stPubMsgList[n].msgToken == response->token)
				{
					pMatchedPubMsg = &pMbeDev->stPubMsgList[n];
					break;
				}			
			}
		}

	}

	//handled the match 
	if(pMatchedPubMsg != NULL)
	{
		TRACEX("----The Msg[%s] has been pubOK!!!---\n", pMatchedPubMsg->szMsgName);
		pMatchedPubMsg->tmLastPubOK = _NOW_;
		//reset status-change bit for Status msg
		if(pMatchedPubMsg->emMsgId == MBE_MSG_STATUS)
		{
			TRACEX("-------Status OOOOOOOOOK = FASSSSSSSSSSSSSSSLE----\n");
			pMbeDev->bIsStatusChanged = FALSE;
		}
		else if(pMatchedPubMsg->emMsgId == MBE_MSG_AUTH_REQ)
		{
			pMbeDev->emMqttCommStatus = MBE_COMM_AUTHING;
		}		
	}

}

// void onPubMsg_Fail(void* context, MQTTAsync_failureData* response)
// {
//     if(context == NULL)
//     {
//             TRACEX("ExecFailCall_onSend: context is NULL\n");
//     }
// 	TRACEX("Send Publish Msg failed, rc %d\n", response ? response->code : 0);
//     return;
// }

void Publish_AMqttJsonMsg(MQTT_MBE_DATA_ROUGH* pMbeDev, MBE_DEV_PUB_MSGID emPubMsgId, cJSON* json)
{
    int rc = 0;
	REPORTER* pReporter = (REPORTER*)pMbeDev->hReporter;
    MQTTAsync client = pMbeDev->client;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;

	if(pMbeDev->bIsMqttVersion5)
	{
		opts.onSuccess5 = onPubMsg_OK;
	}
	else
	{
		opts.onSuccess = onPubMsg_OK;
	}   	
    //opts.onFailure = onPubMsg_Fail;
    opts.context = pMbeDev;
	opts.token = pMbeDev->stSubMsgList[emPubMsgId].emMsgId + 100; //use msgId for token
	//convert json -> string
	char * szPayload = cJSON_Print(json);
	cJSON_Delete(json);
	//set payload for topic
    pubmsg.payload = szPayload;           
    pubmsg.payloadlen = strlen(szPayload); 
    pubmsg.qos = pMbeDev->stPubMsgList[emPubMsgId].qos;
    pubmsg.retained = 0;
#ifdef ___TEST_MQTT_MBE__
#define MBE_DEVICE_TOPIC_PREFIX_TEST	"mbe/dev-test"
	snprintf(pMbeDev->stPubMsgList[emPubMsgId].szTopicName, 128, MBE_DEVICE_TOPIC_PREFIX_TEST"/%s/%s", pMbeDev->szCurrClientId, 
		pMbeDev->stPubMsgList[emPubMsgId].szMsgName);
#else
	snprintf(pMbeDev->stPubMsgList[emPubMsgId].szTopicName, 128, MBE_DEVICE_TOPIC_PREFIX"/%s/%s", pMbeDev->szCurrClientId, 
		pMbeDev->stPubMsgList[emPubMsgId].szMsgName);
#endif
	//added by Jimmy 20220915 in case of lost-comm during publishing...
	static int s_nErrCounter = 0;
	//now send out the msg
    if ((rc = MQTTAsync_sendMessage(client, pMbeDev->stPubMsgList[emPubMsgId].szTopicName, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        TRACEX("Failed to start sendTopic->[%s], payload=[%s], return code %d\n",
			pMbeDev->stPubMsgList[emPubMsgId].szTopicName, pubmsg.payload, rc);
        //rc = -1;
        //Need ReConnect ????
        //exit(EXIT_FAILURE);
		s_nErrCounter++;
		if(s_nErrCounter >= 3)
		{
			pMbeDev->emMqttCommStatus = MBE_COMM_DISCONECTED;
			pMbeDev->emOCPP_Disconn_Reason = MBEMQTT_COMM_FAIL_PUB_ERROR;			
		}
    }
	else
	{
		s_nErrCounter = 0;
		pMbeDev->stPubMsgList[emPubMsgId].tmLastSent = _NOW_;
		//from souce code, the opts.token may be rewrite with the lib
		if(opts.token > 0)
		{
			pMbeDev->stPubMsgList[emPubMsgId].msgToken = opts.token;//for match purpose
		}		
	}

	LOG_MQTT(MQTT_MBE_LOG_SEND, "topic->[%s], msg->[%s], rc->%d", pMbeDev->stPubMsgList[emPubMsgId].szTopicName, 
		(pubmsg.payload == NULL) ? "NULL" : pubmsg.payload, rc);

    TRACEX("mqtt sendTopic->[%s], qos->[%d], token ->[%d], rc=[%d], pubmsg.payload=[%s]\n", \
        pMbeDev->stPubMsgList[emPubMsgId].szTopicName, pubmsg.qos, opts.token, rc, pubmsg.payload);

	//DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_WRITE, pMqtt->szSendBuffer);
	free(szPayload);
}

//static void onMqtt_ConnectOK(void* context, MQTTAsync_successData5* response)
static void onMqtt_ConnectOK(void* context, void* resp)
{
    MQTT_MBE_DATA_ROUGH *pMbeDev = (MQTT_MBE_DATA_ROUGH*)context;
	if(pMbeDev == NULL)
	{
		return;
	}    
    //MQTTAsync client = pMqtt->client;
	UNUSED(resp);
    TRACEX("---Successful connection---\n");
	pMbeDev->emMqttCommStatus = MBE_COMM_CONNECTED;
	pMbeDev->bIs_MqttConnOK = TRUE;
	pMbeDev->nConnFailedTimes = 0;
	LOG_MQTT(MQTT_MBE_LOG_CONNECT, "client->[%s], MqttVersion->[%s] OK!", 
		pMbeDev->szCurrClientId, pMbeDev->bIsMqttVersion5 ? "5.0" : "3.1.1");	
	//Log_ConnectStatuCode(pMqtt);
}

//static void onMqtt_ConnectFailed(void* context, MQTTAsync_failureData5* response)
static void onMqtt_ConnectFailed(void* context, void* resp)
{
    MQTT_MBE_DATA_ROUGH *pMqtt = (MQTT_MBE_DATA_ROUGH*)context;	
	if(pMqtt == NULL)
	{
		return;
	}    
    // TRACEX("ExecFailCall_onConnectFailure:Connect failed, for reson -> [%d, %s]\n",
	// 		response ? response->code : -99999,
	// 		response ? MQTTAsync_strerror(response->code) : "none");
	pMqtt->nConnFailedTimes++;
	if(pMqtt->nConnFailedTimes <= 5)
	{
		if(pMqtt->bIsMqttVersion5)
		{
			MQTTAsync_failureData5* response = (MQTTAsync_failureData5*)resp;
			LOG_MQTT(MQTT_MBE_LOG_CONNECT, "client->[%s] FAILED for reason -> [%s], Failed Times -> %d!", pMqtt->szCurrClientId,
				response ? MQTTAsync_strerror(response->code) : "UNKOWN", pMqtt->nConnFailedTimes);					
		}
		else
		{
			MQTTAsync_failureData* response = (MQTTAsync_failureData*)resp;
			LOG_MQTT(MQTT_MBE_LOG_CONNECT, "client->[%s] FAILED for reason -> [%s], Failed Times -> %d!", pMqtt->szCurrClientId,
				response ? MQTTAsync_strerror(response->code) : "UNKOWN", pMqtt->nConnFailedTimes);		
		}
	}
	else if(pMqtt->nConnFailedTimes > 30)
	{
		pMqtt->nConnFailedTimes = 30;
	}
}

static BOOL Wait_Mqtt_CommReady(REPORTER* pReporter)
{
	MQTT_MBE_DATA_ROUGH* pMqtt = (MQTT_MBE_DATA_ROUGH*)pReporter->pDataHeap;
	int rc = 0;
	static int s_nErrorCounter = 0;	
	TRACEX("-------emMqttCommStatus=[%d]-------------\n", pMqtt->emMqttCommStatus);
	//handle the connection to mqtt broker
	if(pMqtt->emMqttCommStatus == MBE_COMM_DISCONECTED)
    {	
		//disconnect old connection if have
		if(pMqtt->bIs_MqttConnOK)
		{
			Disconnect_MqttClientComm(pMqtt);
		}
		
		pMqtt->bIsMqttConn_Ready = FALSE;
		//when reject, should not reconnect immediately but wait 30sec
		if((ABS(_NOW_ - pMqtt->stSubMsgList[MBE_MSG_AUTH_NOK].tmLastArrived) < 30) && 
			(pMqtt->nAuthReject_Times >= 3))
		{
			TRACEX("---------Now disconnt[%d], ERRRRRRRRR11111111-------\n", s_Conn_ButNoAuth_TotalTimes);
			return FALSE;
		}

		//to avoid to reconnect too frequenty (max 15 * 30 ~= 7.5min)
		int RECONN_WAIT_MAX_TIMEOUT_SEC = s_Conn_ButNoAuth_TotalTimes * MBE_MQTT_CONNET_TIMEOUT;
		RECONN_WAIT_MAX_TIMEOUT_SEC = MAX(10, RECONN_WAIT_MAX_TIMEOUT_SEC);
		//added by Jimmy to avoid conn-fail too frequently
		RECONN_WAIT_MAX_TIMEOUT_SEC = MAX(RECONN_WAIT_MAX_TIMEOUT_SEC, pMqtt->nConnFailedTimes * 10);
		if(ABS(_NOW_ - pMqtt->tmLastSend_Conn) >= RECONN_WAIT_MAX_TIMEOUT_SEC)
		{
			pMqtt->tmLastSend_Conn = _NOW_;
			//added by Jimmy 
			if(pMqtt->bIsMqttVersion5)
			{
				MQTTProperties props = MQTTProperties_initializer;
				MQTTProperty property;	
				property.identifier = MQTTPROPERTY_CODE_SESSION_EXPIRY_INTERVAL;
				property.value.integer4 = 0;
				MQTTProperties_add(&props, &property);
				pMqtt->conn_opts.connectProperties = &props;
			}	
			if ((rc = MQTTAsync_connect(pMqtt->client, &pMqtt->conn_opts)) != MQTTASYNC_SUCCESS)
			{		
				if(pMqtt->nConnFailedTimes < 30)
				{
					pMqtt->nConnFailedTimes++;
				}		
				TRACEX("Error: Failed to MQTTAsync connect, return code %d\n", rc);
			}
			// else
			// {				
			// 	TRACEX("CALL MQTTAsync connect OK %d\n", rc);		
			// }	
			if((rc != MQTTASYNC_SUCCESS) && (pMqtt->nConnFailedTimes <= 10))
			{
				LOG_MQTT(MQTT_MBE_LOG_CONNECT, "now trying to connect to server, rc->%d, FAILED-TIMES->[%d]", 
					rc, pMqtt->nConnFailedTimes);				
			}		
		}
		else
		{
			TRACEX("---------Now disconnt[%d], ERRRRRRRRR222222222-------\n", s_Conn_ButNoAuth_TotalTimes);
		}		
		//needs to return here!!!
		return FALSE;
    }
	else if(pMqtt->emMqttCommStatus == MBE_COMM_CONNECTED)
	{
		//init
		s_nErrorCounter = 0;	
		//max 30times * 60 sec for reconneting
		if(s_Conn_ButNoAuth_TotalTimes < 30)	
		{
			s_Conn_ButNoAuth_TotalTimes++;		//consider no-auth default
		}		
		pMqtt->nAuthReject_Times = 0;
		pMqtt->bIsNeed_Pause_Publish = FALSE;
		pMqtt->bIsStatusChanged = TRUE;
		pMqtt->bHasPending_CtrlParam = FALSE;
		pMqtt->bHasPending_Params_2Set = FALSE; //needs to reset it in case of error
		int n;
		//clear arrv-time for every sub-msg
		for(n = 0; n < MAX_NUM_MBE_MSG_SUB; n++)
		{
			pMqtt->stSubMsgList[n].tmLastArrived = 0;
		}	

		//now first thing is to subscribe
		Subscrible_AllTopics(pMqtt);
	}
	else if(pMqtt->emMqttCommStatus == MBE_COMM_SUBSCRIBING)
	{
		if(ABS(_NOW_ - pMqtt->tmLast_CallSubscribe) > 10)
		{
			Subscrible_AllTopics(pMqtt);
			pMqtt->emOCPP_Disconn_Reason = MBEMQTT_COMM_FAIL_REASON_SCRIBED_TIMEOUT;
			s_nErrorCounter++;
		}	
		// else
		// {
		// 	pMqtt->emOCPP_Disconn_Reason = MBEMQTT_COMM_FAIL_REASON_SCRIBED_TIMEOUT;
		// 	s_nErrorCounter++;
		// }	
	}
	else if(pMqtt->emMqttCommStatus == MBE_COMM_WAIT_AUTH)
	{
		pMqtt->stPubMsgList[MBE_MSG_AUTH_REQ].pfnHandler(pMqtt);
		pMqtt->emMqttCommStatus = MBE_COMM_AUTHING;
	}
	else if(pMqtt->emMqttCommStatus == MBE_COMM_AUTHING)
	{
		static time_t s_tmLastSentAuthReq = 0;
		if(ABS(_NOW_ - pMqtt->stPubMsgList[MBE_MSG_AUTH_REQ].tmLastSent) >= 15)
		{
			if(s_tmLastSentAuthReq != pMqtt->stPubMsgList[MBE_MSG_AUTH_REQ].tmLastSent)
			{
				pMqtt->emOCPP_Disconn_Reason = MBEMQTT_COMM_FAIL_REASON_AUTH_TIMEOUT;
				s_tmLastSentAuthReq = pMqtt->stPubMsgList[MBE_MSG_AUTH_REQ].tmLastSent;
				pMqtt->stPubMsgList[MBE_MSG_AUTH_REQ].pfnHandler(pMqtt);
				s_nErrorCounter++;
			}	
			else if(ABS(_NOW_ - pMqtt->stPubMsgList[MBE_MSG_AUTH_REQ].tmLastSent) >= 60)
			{
				//should not happen in real case
				pMqtt->emOCPP_Disconn_Reason = MBEMQTT_COMM_FAIL_REASON_AUTH_TIMEOUT;
				s_nErrorCounter++;
			}				
		}
	}
	else if(pMqtt->emMqttCommStatus == MBE_COMM_AUTH_REJECTED)
	{
		TRACEX("-------THE MQTT-Client was REJJECTED[%d]!!!!--------\n", pMqtt->nAuthReject_Times);
		if(pMqtt->nAuthReject_Times >= 3)
		{
			pMqtt->emOCPP_Disconn_Reason = MBEMQTT_COMM_FAIL_REASON_REFUSED;
			s_nErrorCounter = 5;
		}
		pMqtt->emMqttCommStatus = MBE_COMM_AUTHING; //MBE_COMM_WAIT_AUTH; //on reject we send auth again until 3-times fail	
	}
	else if(pMqtt->emMqttCommStatus == MBE_COMM_READY)
	{
		TRACEX("-------THE MQTT-Client is READY!!!!--------\n");
		pMqtt->bIsMqttConn_Ready = TRUE;
		s_Conn_ButNoAuth_TotalTimes = 0; //clear it as auth done!!!
		return TRUE;
	}
	else if(pMqtt->emMqttCommStatus == MBE_COMM_WAIT_REBUILD_CLIENT)
	{
		//if(ABS(_NOW_ - pMqtt->tmLastSend_DisConn) >= 10)
		{			
			ReCreate_Client_AndInit(pMqtt);
			pMqtt->nAuthReject_Times = 0;
			pMqtt->emMqttCommStatus = MBE_COMM_DISCONECTED;
			//pMqtt->nAuthFailed_Times++;
		}		
	}

	if(s_nErrorCounter >= 5)
	{
		//disconn the comm forcely
		TRACEX("----Now needs to disconnecte MQTT-conn so as to reconnect");
		if(pMqtt->bIs_MqttConnOK)
		{
			Disconnect_MqttClientComm(pMqtt);
		}
		Sleep(1000);
		pMqtt->emMqttCommStatus = MBE_COMM_DISCONECTED;
	}

	return FALSE;
}

static int GetMBEDev_WorkStatus(MQTT_MBE_DATA_ROUGH* pMqtt)
{
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	int nWorkMode = 0;
	PARAM_VAL_GET valGet;
	valGet.nGetSize = 4;

	typedef enum tagCCUState
	{
		CCU_STATE_IDLE,
		CCU_STATE_LOCKED,
		CCU_STATE_FAULTED,
		CCU_STATE_AllBusy, //all guns are in charing
		CCU_STATE_PendingCharge, //one or more gun is charging
		CCU_STATE_EXITING, //now exiting
	}CCU_STATE;

	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			1, 4, &valGet, sizeof(PARAM_VAL_GET));
	if(valGet.val.emValue == CCU_STATE_IDLE)
	{
		nWorkMode = 0; //idle
	}
	else if(valGet.val.emValue == CCU_STATE_AllBusy)
	{
		nWorkMode = 1; //all charging
	}
	else if(valGet.val.emValue == CCU_STATE_PendingCharge)
	{
		nWorkMode = 2; //partly charging
	}
	else if(valGet.val.emValue == CCU_STATE_LOCKED)
	{
		nWorkMode = 12; //disabled
	}
	else //if(valGet.val.emValue == CCU_STATE_FAULTED)
	{
		nWorkMode = 11; //fault
	}
	//now check the fw update status
	if(pMqtt->emFirmwareUpgradeStatus == FIRMUPDATE_Installing)
	{
		nWorkMode = 13; //pending fw update
	}
	else if(pMqtt->emFirmwareUpgradeStatus == FIRMUPDATE_Downloading)
	{
		nWorkMode = 3; //pending fw downloading
	}
	else if(pMqtt->emDiagnosUploadStatus == DIAGNOUPLOAD_Uploading)
	{
		nWorkMode = 4; //pending diagnostic upoading
	}

	return nWorkMode;
}

BOOL Judge_IfNeed_SendStatus(REPORTER* pReporter)
{
	MQTT_MBE_DATA_ROUGH* pMqtt = (MQTT_MBE_DATA_ROUGH*)pReporter->pDataHeap;
	MBE_MQTT_STATUSMSG_DATA* pStatusData = &pMqtt->stCurrStatusData;

	//version info
	PARAM_VAL_GET valGet;
	A_BLOB_CHARGER_BOOTINFO bootInfo;
	valGet.nGetSize = sizeof(A_BLOB_CHARGER_BOOTINFO);
	valGet.val.blobValue = (BYTE*)&bootInfo;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 17, &valGet, sizeof(PARAM_VAL_GET));	
	if (err == ERRCODE_DAI_OK)
	{
		if(strcmp(bootInfo.szFirmwareVer_Ex, pStatusData->szVerInfo) != 0)
		{
			snprintf(pStatusData->szVerInfo, 50, "%s", bootInfo.szFirmwareVer_Ex);
			TRACEX("-----11111111111111111----\n");
			pMqtt->bIsStatusChanged = TRUE;
		}
	}

	//work status
	int nWorkMode = GetMBEDev_WorkStatus(pMqtt);
	if(nWorkMode != pStatusData->emWorkStatus)
	{
		pStatusData->emWorkStatus = nWorkMode;
		TRACEX("-----222222222222222222----\n");
		pMqtt->bIsStatusChanged = TRUE;
	}

	//fault staus
	int nFaultStaus = pStatusData->emFaultyStat;
	if(pStatusData->nActiveAlmTotal <= 0)
	{
		nFaultStaus = 0;
	}
	else
	{
		if(pStatusData->emWorkStatus == 11)
		{
			nFaultStaus = 1;
		}
		else
		{
			nFaultStaus = 3;
			//if sub-device is fault, needs set -> 2, will handle it later
		}
	}
	if(nFaultStaus != pStatusData->emFaultyStat)
	{
		pStatusData->emFaultyStat = nFaultStaus;
		TRACEX("-----3333333333333333----\n");
		pMqtt->bIsStatusChanged = TRUE;		
	}

	//vpn status	
	static time_t s_tmLastRefreshStatus = 0;//added by Jimmy 20220905 to avoid send _SYSTEM cmd too frequently
	if(ABS(_NOW_ - s_tmLastRefreshStatus) >= 1)
	{
		int nVPNStatus = -1;
		int nRet = _SYSTEM_EX("ps | grep -v grep | grep -c vcon_client");
		if(nRet < 1)
		{
			nVPNStatus = 0; //not started yet
			//Jimmy added 20221031: just in case of wrong judge
			nRet = _SYSTEM_EX("netstat -anp | grep -c vcon_client");
			if(nRet > 0)
			{
				nVPNStatus = 1; //(pStatusData->emVPNStat == 2) ? 2 : 1; //if history is 2, then return 2
			}
		}
		//else
		if(nVPNStatus != 0)
		{
			nVPNStatus = 1; //started
			//check vonn thru netport status??
			nRet = _SYSTEM_EX("netstat -anp | grep 'vcon_client' | grep -c ESTABLISHED");
			if(nRet > 0)
			{
				nVPNStatus = 2; //connected
			}
		}
		
		if(nVPNStatus != pStatusData->emVPNStat)
		{
			pStatusData->emVPNStat = nVPNStatus;
			TRACEX("-----444444444444444----\n");
			pMqtt->bIsStatusChanged = TRUE;		
		}
		s_tmLastRefreshStatus = _NOW_;
	}	

	//juege if needs send periodically
	if((!pMqtt->bIsStatusChanged)/* && (!pMqtt->bIsNeed_Pause_Publish)*/)
	{
		//nStatusReport_Interval->0 means auto -> 30min
		int nSendInterv_min = pMqtt->nStatusReport_Interval;
		if(nSendInterv_min < 30)
		{
			nSendInterv_min = 60; //default 1hr
		}
		else if(nSendInterv_min > 1440)
		{
			nSendInterv_min = 1440;
		}
		if(ABS(_NOW_ - pMqtt->stPubMsgList[MBE_MSG_STATUS].tmLastPubOK) >= (nSendInterv_min * 60))
		{
			TRACEX("-----5555555555555555----\n");
			pMqtt->bIsStatusChanged = TRUE;
		}
	}

	//don't pause status for longer than 120min
	static int s_nPauseCounter = 0;
	if(pMqtt->bIsNeed_Pause_Publish)
	{
		if(ABS(_NOW_ - pMqtt->stPubMsgList[MBE_MSG_STATUS].tmLastSent) > (120 * 60))
		{
			s_nPauseCounter++;
			if(s_nPauseCounter > 60)
			{
				pMqtt->bIsNeed_Pause_Publish = FALSE;
				s_nPauseCounter = 0;
			}
		}		
	}
	else
	{
		s_nPauseCounter = 0;
	}

	//judge the execute-cmd-status
	int nExeCmdStat = 0;
	if(pMqtt->bHasPending_Params_2Set || pMqtt->bHasPending_CtrlParam)
	{
		nExeCmdStat = 4; //pending set/ctrl
	}	
	else if((pMqtt->emFirmwareUpgradeStatus == FIRMUPDATE_Downloading) ||
		(pMqtt->emFirmwareUpgradeStatus == FIRMUPDATE_Installing))
	{
		nExeCmdStat = 1; //fw ftp downloading
	}	
	else if(pMqtt->emDiagnosUploadStatus == DIAGNOUPLOAD_Uploading)
	{
		nExeCmdStat = 2; //diagnotic ftp uploading
	}	

	if(pMqtt->stCurrStatusData.emRemoteCmd_ExeStat != nExeCmdStat)
	{
		pMqtt->bIsStatusChanged = TRUE;
		pMqtt->stCurrStatusData.emRemoteCmd_ExeStat = nExeCmdStat;
	}
	else if((pMqtt->tmLastSent_RemoteExeCmd_Busy != 0) && (nExeCmdStat == 0))
	{
		if(ABS(_NOW_ - pMqtt->tmLastSent_RemoteExeCmd_Busy) > 10)
		{
			pMqtt->bIsStatusChanged = TRUE;
		}
	}

	return pMqtt->bIsNeed_Pause_Publish ? FALSE : pMqtt->bIsStatusChanged;
}

static void Make_AndSend_Set_Conf(MQTT_MBE_DATA_ROUGH* pMqtt, BOOL bPending)
{
	cJSON* pJson2Pub = cJSON_CreateObject();
	cJSON* pJSuccess_KeyValArr = NULL;
	cJSON* pJFailed_KeyArr = NULL;
	cJSON* pJUnsupport_KeyArr = NULL;
	int n;
	int nResult = bPending ? 1 : 0;
	for(n = 0; n < pMqtt->nPending_Params_Count; n++)
	{
		MBE_MQTT_ASIG_KEY_VAL* pKeyValItem = &pMqtt->stPendingParams_KeyValList[n];
		if(!pKeyValItem->bIsParsedOK)
		{
			if(!bPending)
			{
				nResult = 4;
			}
			if(pJUnsupport_KeyArr == NULL)
			{
				pJUnsupport_KeyArr = cJSON_CreateArray();
			}
			cJSON_AddItemToArray(pJUnsupport_KeyArr, cJSON_CreateString(pKeyValItem->szRawKeyName));	
			continue;
		}
		if(pKeyValItem->nExecuteResult <= MBEDEV_EXESET_RESULT_NONE)
		{
			continue;
		}
		else if(pKeyValItem->nExecuteResult == MBEDEV_EXESET_RESULT_SUCCESS)
		{
			if(pJSuccess_KeyValArr == NULL)
			{
				pJSuccess_KeyValArr = cJSON_CreateArray();
			}
			cJSON* pJKeyVal = cJSON_CreateObject();
			cJSON_AddStringToObject(pJKeyVal, "key", pKeyValItem->szRawKeyName);
			cJSON_AddStringToObject(pJKeyVal, "val", pKeyValItem->szVal); //send back the raw val
			cJSON_AddItemToArray(pJSuccess_KeyValArr, pJKeyVal);
		}
		else if(pKeyValItem->nExecuteResult == MBEDEV_EXESET_RESULT_FAILED)
		{
			if(!bPending)
			{
				nResult = 3;
			}
			if(pJFailed_KeyArr == NULL)
			{
				pJFailed_KeyArr = cJSON_CreateArray();
			}
			cJSON_AddItemToArray(pJFailed_KeyArr, cJSON_CreateString(pKeyValItem->szRawKeyName));	
		}
		else if(pKeyValItem->nExecuteResult == MBEDEV_EXESET_RESULT_WRONG)
		{
			if(!bPending)
			{
				nResult = 4;
			}
			if(pJUnsupport_KeyArr == NULL)
			{
				pJUnsupport_KeyArr = cJSON_CreateArray();
			}
			cJSON_AddItemToArray(pJUnsupport_KeyArr, cJSON_CreateString(pKeyValItem->szRawKeyName));	
		}
	}

	cJSON_AddNumberToObject(pJson2Pub, "result", nResult);
	if(pJSuccess_KeyValArr != NULL)
	{
		cJSON_AddItemToObject(pJson2Pub, "updt_paramvals", pJSuccess_KeyValArr);
	}
	if(pJFailed_KeyArr != NULL)
	{
		cJSON_AddItemToObject(pJson2Pub, "failed_params", pJFailed_KeyArr);
	}
	if(pJUnsupport_KeyArr != NULL)
	{
		cJSON_AddItemToObject(pJson2Pub, "unsupported_params", pJUnsupport_KeyArr);
	}

	if(strlen(pMqtt->szSenderId_4PendingParamSet) > 0)
	{
		cJSON_AddStringToObject(pJson2Pub, "sender_id", pMqtt->szSenderId_4PendingParamSet);
	}
	
	Publish_AMqttJsonMsg(pMqtt, MBE_MSG_SET_CONF, pJson2Pub);
}

static void Handle_Pending_SetCtrlCmd_4Mqtt(MQTT_MBE_DATA_ROUGH* pMqtt)
{
	if(pMqtt->bHasPending_CtrlParam)
	{		
		ExeCute_DevCtrlCmd(pMqtt);
		pMqtt->bHasPending_CtrlParam = FALSE;
	}

	if(!pMqtt->bHasPending_Params_2Set)
	{
		return;
	}

	//now handle the param-set one by one
	int n;
	int nExeTimeTotal_Sec = 0;
	int nResult = 0;
	int nTimeDiff = 0;
	for(n = 0; n < pMqtt->nPending_Params_Count; n++)
	{
		time_t tmBeforeExe = _NOW_;
		MBE_MQTT_ASIG_KEY_VAL* pKeyValItem = &pMqtt->stPendingParams_KeyValList[n];
		pKeyValItem->nExecuteResult = Execute_AParamSet_Item(pMqtt, pKeyValItem, 10000);
		nTimeDiff = _NOW_ - tmBeforeExe;
		if((nTimeDiff >= 1) && (nTimeDiff < 30))
		{
			nExeTimeTotal_Sec += nTimeDiff;
		}	
		if((nExeTimeTotal_Sec >= 3) && (n < pMqtt->nPending_Params_Count - 1))	
		{
			//send cmd-pending status -> server
			//pMqtt->stCurrStatusData.emRemoteCmd_ExeStat = 4;
			//pMqtt->stPubMsgList[MBE_MSG_STATUS].pfnHandler(pMqtt);
			//send set.conf 
			Make_AndSend_Set_Conf(pMqtt, TRUE);		
		}
	}
	//now send a finish conf
	Make_AndSend_Set_Conf(pMqtt, FALSE);
	//clear this flag so as to accept new commands
	pMqtt->bHasPending_Params_2Set = FALSE;
}

static void Mqtt_Trace_Callback(enum MQTTASYNC_TRACE_LEVELS level, char* message)
{
	//if ((strstr(message, "onnect") && !strstr(message, "isconnect")) || level == MQTTASYNC_TRACE_ERROR)
		printf("[%.3f -> Mqttt-Trace] : %d, %s\n", GetCurrentTime(), level, message);
}


static void ReCreate_Client_AndInit(MQTT_MBE_DATA_ROUGH* pMqtt)
{
	//disconnect old connection if have
	if(pMqtt->bIs_MqttConnOK)
	{
		Disconnect_MqttClientComm(pMqtt);
	}

	//for test purpose
	// MQTTAsync_setTraceCallback(Mqtt_Trace_Callback);
	// MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_MINIMUM);

	//destroy old client if have
	if(pMqtt->client != NULL)
	{
		TRACEX("----Remove old MQTT client before create!----\n");
		MQTTAsync_destroy(&pMqtt->client);
	}

    int rc = 0;    
    MQTTAsync client;

	//first init the ClientId using latest DevSN(already refreshed)
	snprintf(pMqtt->szCurrClientId, 64, "%s|%s", pMqtt->szDevSN, pMqtt->szDevMAC);
	TRACEX("-----szDevSN->(%s),szDevMAC->(%s), szCurrClientId->(%s)----\n",
			pMqtt->szDevSN, pMqtt->szDevMAC, pMqtt->szCurrClientId);
	pMqtt->nConnFailedTimes = 0;
	pMqtt->nAuthReject_Times = 0;
	s_Conn_ButNoAuth_TotalTimes = 0;
	//create client
//#define MBE_MQTT_SERVER_ADDR	"tcp://xmbe.infypower.com:55000" //tcp://47.243.110.116:55000" //should use 'tcp://' prefix accoring to src code
    //changed by Jimmy 20221112
	char szMqttCompleteServerAddress[64];
	//snprintf(szMqttCompleteServerAddress, 64, "tcp://%s:55000", pMqtt->szMqttServerAddress);
	//snprintf(szMqttCompleteServerAddress, 64, "ssl://%s:55001", pMqtt->szMqttServerAddress);
	snprintf(szMqttCompleteServerAddress, 64, "mqtts://%s:55001", pMqtt->szMqttServerAddress);
	MQTTAsync_createOptions createOptions = MQTTAsync_createOptions_initializer;
	createOptions.MQTTVersion = MQTTVERSION_5; //using mqtt5
	//rc = MQTTAsync_create(&client, szMqttCompleteServerAddress, pMqtt->szCurrClientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	rc = MQTTAsync_createWithOptions(&client, szMqttCompleteServerAddress, pMqtt->szCurrClientId, 
		MQTTCLIENT_PERSISTENCE_NONE, NULL, &createOptions);
	TRACEX("-----------MQTTAsync_create rc=[%d]--- \n", rc);
    pMqtt->client = client;
	//!!!!!!Jimmy note 20221115: found a libpaho_maqtt bug that when server kicked out the client, the client never noticed the discocnnection for 
	//MQTT 5.0 so we have to use MQTT 3.1.1
	pMqtt->bIsMqttVersion5 = FALSE;
	LOG_MQTT(MQTT_MBE_LOG_RECREAT, "url->[%s], rc->%d, Mqtt-Version->[%s]", szMqttCompleteServerAddress, rc,
		pMqtt->bIsMqttVersion5 ? "5.0" : "3.1.1");

 	//init conn-options	
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	if(pMqtt->bIsMqttVersion5)
	{
		MQTTAsync_connectOptions conn_opts_5 = MQTTAsync_connectOptions_initializer5;
		memcpy(&conn_opts, &conn_opts_5, sizeof(MQTTAsync_connectOptions));
	}	 
    MQTTAsync_SSLOptions sslopts = MQTTAsync_SSLOptions_initializer;
    sslopts.enableServerCertAuth = 0;
    //sslopts.trustStore = "cacert.pem";
    sslopts.sslVersion = MQTT_SSL_VERSION_TLS_1_2;

   	conn_opts.keepAliveInterval = 30;
	conn_opts.connectTimeout = MBE_MQTT_CONNET_TIMEOUT;
	if(pMqtt->bIsMqttVersion5)
	{
		conn_opts.cleanstart = 1; //this is for mqtt5 and for mqtt5 cleansession should be 0
		MQTTProperties props = MQTTProperties_initializer;
		MQTTProperty property;	
		property.identifier = MQTTPROPERTY_CODE_SESSION_EXPIRY_INTERVAL;
		property.value.integer4 = 0; //should be 0 to clean session after reconnection
		MQTTProperties_add(&props, &property); 
		conn_opts.connectProperties = &props;
		conn_opts.cleansession = 0;
		conn_opts.onSuccess5 = onMqtt_ConnectOK;
    	conn_opts.onFailure5 = onMqtt_ConnectFailed;
	}	
	else
	{
		conn_opts.cleanstart = 0;
		conn_opts.cleansession =  1;// should be 0 form MQTT5, 1 means not to save information from a previous connection of the same client
		conn_opts.onSuccess = onMqtt_ConnectOK;
		conn_opts.onFailure = onMqtt_ConnectFailed;
	}
    conn_opts.context = pMqtt;
    conn_opts.username = pMqtt->szMqttClient_UserName; 
    conn_opts.password = pMqtt->szMqttClient_Passwd;  	
    //-----Init SSL Encrpt Parameter-----//
	memcpy(&pMqtt->sslopts, &sslopts, sizeof(MQTTAsync_SSLOptions));
    conn_opts.ssl = &pMqtt->sslopts;
	//conn_opts.ssl->verify = 1;
	memcpy(&pMqtt->conn_opts, &conn_opts, sizeof(MQTTAsync_connectOptions));

    rc = MQTTAsync_setCallbacks(pMqtt->client, pMqtt, onConnLost, onSubMsg_Arrived, NULL);

	TRACEX("-----------MQTTAsync_setCallbacks rc=[%d]--- \n", rc);
}

static DWORD _ThreadEntry_MqttComm(void* pArg)
{
	TRACE("-----------_ThreadEntry_MqttComm: Start Enter------- \n");

	REPORTER* pReporter = (REPORTER*)pArg;
	HANDLE hPort = pReporter->hCommPort;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	MQTT_MBE_DATA_ROUGH* pMqtt = (MQTT_MBE_DATA_ROUGH*)pReporter->pDataHeap;
    //PARAM_VAL_GET 	valGet;
	//ERRCODE_DAI 	err;

	ReCreate_Client_AndInit(pMqtt);

	BOOL bIsNeddRefresh_Status = FALSE;
	BOOL bIsMqttServerCommOK = FALSE;
	VAR_VALUE varV;
	varV.emValue = 99;
   	while (!pReporter->bExit)
	{
		RUN_THREAD_HEARTBEAT();

		if(!Wait_Mqtt_CommReady(pReporter))
		{
			Sleep(500);
			continue;
		}
		else
		{
			bIsMqttServerCommOK = TRUE; //if once connected, will consider conneced all the time
		}
	
		//now notify the old-mbe to close connection
		if(varV.emValue != bIsMqttServerCommOK)
		{
			varV.emValue = bIsMqttServerCommOK;
			pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
						1, 18, &varV, 4, 0);			
		}
    
		Mutex_Lock(pMqtt->hMutex, WAIT_INFINITE);	
		bIsNeddRefresh_Status = Judge_IfNeed_SendStatus(pReporter);
		Mutex_Unlock(pMqtt->hMutex);
		if(bIsNeddRefresh_Status)
		{
			if((ABS(_NOW_ - pMqtt->stPubMsgList[MBE_MSG_STATUS].tmLastSent) >= 2) && 
				(ABS(_NOW_ - pMqtt->tmLastSentTrigStatus) >= 2)) //in case of frequent sending
			{
				//now send status
				Mutex_Lock(pMqtt->hMutex, WAIT_INFINITE);
				pMqtt->stPubMsgList[MBE_MSG_STATUS].pfnHandler(pMqtt);
				Mutex_Unlock(pMqtt->hMutex);
			}			
		}

		//pending set/ctrl needs to be handled here async
		Handle_Pending_SetCtrlCmd_4Mqtt(pMqtt);

		//TRACEX("-----------mqtt_MainRun rc=[%d]--- \n", rc);
        Sleep(100);
    }

    MQTTAsync_destroy(&pMqtt->client);
    SAFELY_DELETE(pMqtt); //Release Allocate Memory

	TRACEX("-----------_ThreadEntry_MqttComm: Exit--------------\n");
	return THREAD_CANCEL_ALL;
}


#define INIT_AMQTT_PUBMSG(_pMsg, _emMsgId, _szMsgName, _nQoS, _pfnMsgHandler) \
	do{\
	(_pMsg)->emMsgId = (_emMsgId); \
	snprintf((_pMsg)->szMsgName, 16,  (_szMsgName)); \
	(_pMsg)->qos = (_nQoS); \
	(_pMsg)->pfnHandler = (_pfnMsgHandler); \
	(_pMsg)->msgToken = (-999999); \
	(_pMsg)++; \
	}while(0)

static void InitAll_MqttPubMsgsInfo(MQTT_MBE_DATA_ROUGH *pMbeDev)
{
	//first init the ClientId
	snprintf(pMbeDev->szCurrClientId, 64, "%s|%s", pMbeDev->szDevSN, pMbeDev->szDevMAC);
	//then init all the sub msg topic info
	MBE_DEV_PUB_MESSAGE* p = &(pMbeDev->stPubMsgList[0]);
	//auth.req can NOT use QoS=2, because there can be re-send unexpectedly
	INIT_AMQTT_PUBMSG(p, 	MBE_MSG_AUTH_REQ, 	"auth.req",		/*QoS_2*/QoS_0,		Prepare_Topic_AuthReq);
	INIT_AMQTT_PUBMSG(p, 	MBE_MSG_STATUS, 	"status",		QoS_0,		Prepare_Topic_Status);
	INIT_AMQTT_PUBMSG(p, 	MBE_MSG_CFG_CONF, 	"cfg.conf",		QoS_0,		NULL); //no handler
	INIT_AMQTT_PUBMSG(p, 	MBE_MSG_PARAM_CONF, "param.conf",	QoS_0,		NULL); //no handler
	INIT_AMQTT_PUBMSG(p, 	MBE_MSG_SET_CONF, 	"set.conf",		QoS_0,		NULL); //no handler
	INIT_AMQTT_PUBMSG(p, 	MBE_MSG_CTRL_CONF, 	"ctrl.conf",	QoS_0,		NULL); //no handler
	INIT_AMQTT_PUBMSG(p, 	MBE_MSG_ERR_DEV, 	"error",		QoS_0,		NULL); //no handler
}

DLLExport BOOL Initialize(HANDLE pReporterThis)
{
	TRACEX("---------------Initialize MQTT Start !!!--------------- \n");
	REPORTER* pReporter = (REPORTER*)pReporterThis;

	//in release mode, we don't start mqtt yet
//#ifndef ___TEST_MQTT_MBE__
//	return TRUE;
//#endif

	//Create data
	pReporter->pDataHeap = NEW(MQTT_MBE_DATA_ROUGH, 1);
	if(pReporter->pDataHeap == NULL)
	{
		TRACEX("Memory full for MQTT dataStructs!\n");
		return FALSE;
	}

	MQTT_MBE_DATA_ROUGH* pMqtt = (MQTT_MBE_DATA_ROUGH*)pReporter->pDataHeap;
	ZERO_POBJS(pMqtt, 1);

	pMqtt->hReporter = pReporterThis;
	pMqtt->emMqttCommStatus = MBE_COMM_DISCONECTED;       

	pMqtt->hMutex = Mutex_Create(TRUE);
	if(pMqtt->hMutex == NULL)
	{
		TRACEX("MaintBE-MQTT ERROR : Fail to Create Mutex and Queue \n");
		return FALSE;
	}

	//now get the SN and MAC(mac won't change)
	PARAM_VAL_GET valGet;
	//mqtt server addr
	valGet.nGetSize = 32;
	char szMqttServerAddr[32];
	valGet.val.blobValue = (BYTE*)szMqttServerAddr;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 81, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		return FALSE;
	}
	snprintf(pMqtt->szMqttServerAddress, 32, "%s", szMqttServerAddr);
	//sn
	A_BLOB_CHARGER_BOOTINFO bootInfo;
	valGet.nGetSize = sizeof(A_BLOB_CHARGER_BOOTINFO);
	valGet.val.blobValue = (BYTE*)&bootInfo;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 17, &valGet, sizeof(PARAM_VAL_GET));	
	if (err != ERRCODE_DAI_OK)
	{
		return FALSE;
	}
	snprintf(pMqtt->szDevSN, 32, "%s", bootInfo.szChargePointSerualNuml);
	//mac
	A_BLOB_NETWORKINFO netInfo;
	valGet.nGetSize = sizeof(A_BLOB_NETWORKINFO);
	valGet.val.blobValue = (BYTE*)&netInfo;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 3, &valGet, sizeof(PARAM_VAL_GET));	
	if(err != ERRCODE_DAI_OK)
	{
		return FALSE;
	}
	snprintf(pMqtt->szDevMAC, 18, "%s", netInfo.netInfo_eht0.szMacAddr);
#define MBEDEV_MQTTCLEINT_AUTH_USER		"admin"
#define MBEDEV_MQTTCLEINT_AUTH_PASSWD	"infy13418913186"
	sprintf(pMqtt->szMqttClient_UserName, MBEDEV_MQTTCLEINT_AUTH_USER);
	sprintf(pMqtt->szMqttClient_Passwd, MBEDEV_MQTTCLEINT_AUTH_PASSWD);

	//now init the publish messges info
    InitAll_MqttPubMsgsInfo(pMqtt); 

	pReporter->bExit = FALSE;
	pReporter->hThread = RunThread_Create("MBEMqtt",
		(RUN_THREAD_START_PROC)_ThreadEntry_MqttComm,
		(void *)(pReporter),
		(DWORD*)&pReporter->bExit);
		
	if(pReporter->hThread == NULL)
	{
		TRACE("ERROR : Create MqttComm thread Failed \n");
		return FALSE;
	}

typedef struct
{
	int nEvTag;
	int nMsgTag;
}EVHAND_TAGS;	

	//-----Init event handler-----//
	EVHAND_TAGS tags[] =
	{
		{ EVENT_TYPE_ALARM,				EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_RESTORE},
		{ EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN },
 		{ EVENT_TYPE_FIRMWARE_UPDATE,	EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
		{ EVENT_TYPE_DIAGNOSTIC_UPLOAD, EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
		{ EVENT_TYPE_ABILITY_CHANGE,	EVENT_MSG_BEGIN | EVENT_MSG_END }, //do not treat this, use status change event instead
		{ EVENT_TYPE_SYSTEM_EXIT,		EVENT_MSG_END},
 	};

    int n = 0;
	for (n = 0; n < ITEM_OF(tags); n++)
	{
		pReporter->pDAI->Regsiter_EvHandler("Mbe_Mqtt_EventHand",
			pReporterThis,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}
	
	return TRUE;
}
