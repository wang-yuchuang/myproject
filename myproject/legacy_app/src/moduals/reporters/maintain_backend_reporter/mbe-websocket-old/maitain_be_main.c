#include "../../reporters_base.h"
#include "maitain_be.h"
#include <signal.h>

void DUMP_MBELog(int nLogType, char* szLog)
{
	char szTime[32];
	memset(szTime, 0, 32);
	double fLogTime = GetCurrentTime();
	if(nLogType == MBE_LOGTYPE_OPENPORT)
	{
		LogOut_MBE("[%s][(MBE-WS-Old)OPEN-PORT]:%s", AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32), szLog);
	}
	else if(nLogType == MBE_LOGTYPE_CLOSEPORT)
	{
		LogOut_MBE("[%s][(MBE-WS-Old)CLOS-PORT]:%s", AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32), szLog);
	}
	else if(nLogType == MBE_LOGTYPE_SENDMSG)
	{
		LogOut_MBE("[%s][(MBE-WS-Old)SEND-AMSG]:%s", AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32), szLog);
	}
	else if(nLogType == MBE_LOGTYPE_RECVMSG)
	{
		LogOut_MBE("[%s][(MBE-WS-Old)RECV-AMSG]:%s", AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32), szLog);
	}
	else if(nLogType == MBE_LOGTYPE_SENDERR)
	{
		LogOut_MBE("[%s][(MBE-WS-Old)SEND-ERRR]:%s", AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32), szLog);
	}
}

static void Refresh_URL(REPORTER* pReporter)
{
	//server URL , format[ end url + "/SN"]	
	MAINTAINBE_DATA_STU* pOCPP = (MAINTAINBE_DATA_STU*)pReporter->pDataHeap;	
	PARAM_VAL_GET valGet;
	valGet.nGetSize = 255;
	char szEndUrl[255];
	valGet.val.blobValue = (BYTE*)szEndUrl; //&pOCPP->szServerURL;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 72, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: MaintBE server URL can not be retrieved\n");
		return;
	}
	
	A_BLOB_CHARGER_BOOTINFO bootInfo;
	valGet.nGetSize = sizeof(A_BLOB_CHARGER_BOOTINFO);
	valGet.val.blobValue = (BYTE*)&bootInfo;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 17, &valGet, sizeof(PARAM_VAL_GET));
	
	if (err != ERRCODE_DAI_OK)
	{
		return;
	}

	snprintf(pOCPP->szServerURL, 255, "%s/%s", szEndUrl, bootInfo.szChargePointSerualNuml);
	//for test purpose!
	//snprintf(pOCPP->szServerURL, 255, "%s", szEndUrl);
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
	MAINTAINBE_DATA_STU* pOCPP = (MAINTAINBE_DATA_STU*)pReporter->pDataHeap;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	int n;
	RunThread_Heartbeat(pReporter->hThread); //in case the thread no response
	switch (emEvType)
	{
		/*case EVENT_TYPE_CHARGING:
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
						Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
						pOCPP->stMsgInfoList[C_102_StatusNotif].pfnMsgHandler((void*)pOCPP);
						pOCPP->bNoticedExitOCPP = TRUE;
						//pOCPP->emOCPPConnStatus = Disconnected;
						Mutex_Unlock(pOCPP->hMutex);
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
		*/
		case EVENT_TYPE_FIRMWARE_UPDATE:
		{
			TRACEX("MaintBE: Event_Handler  _EVENT_TYPE = EVENT_TYPE_FIRMUPDATE \t emMsgType = %d, size[%d,%d] \n", 
				emMsgType, nEvDataSize, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
			EVENT_FIRMWAREUPDATE_STATUS * pFirmUpSta = (EVENT_FIRMWAREUPDATE_STATUS*)pEvData;
			if (nEvDataSize != sizeof(EVENT_FIRMWAREUPDATE_STATUS))
			{
				return EVENT_HANDED_FAILED;
			}
			//Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
			pOCPP->emFirmwareUpgradeStatus = pFirmUpSta->emUpdateStatus;
			TRACEX("MaintBE: emFirmwareUpgradeStatus =%d\n",pOCPP->emFirmwareUpgradeStatus);
			//Now call the msg handler to create the msg to send
			pOCPP->bCPStatusChanged = TRUE;
			//Mutex_Unlock(pOCPP->hMutex);
		}
		break;
	// case EVENT_TYPE_DIAGNOSTIC_UPLOAD:
	// 	{
	// 		TRACEX("OCPP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_DIAGNOSUPLOAD \t emMsgType = %d \n", emMsgType);
	// 		EVENT_DIAGNOUPLOAD_STATUS * pDiagUpSta = (EVENT_DIAGNOUPLOAD_STATUS*)pEvData;
	// 		if (nEvDataSize != sizeof(EVENT_DIAGNOUPLOAD_STATUS))
	// 		{
	// 			return EVENT_HANDED_FAILED;
	// 		}
	// 		Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
	// 		pOCPP->emDiagnosUploadStatus = pDiagUpSta->emUploadStatus;
	// 		//Now call the msg handler to create the msg to send
	// 		pOCPP->stMsgInfoList[C_407_StatOfDiag].pfnMsgHandler((void*)pOCPP);
	// 		Mutex_Unlock(pOCPP->hMutex);
	// 	}
	// 	break;
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

			pOCPP->bCPStatusChanged = TRUE;

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
			if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 72)) //server URL change or ChargerID changed
			{
				Refresh_URL(pReporter);
				if(strncmp(pOCPP->szServerURL, pOCPP->szServerURL_Old, 255)!= 0)
				{
					pOCPP->emOCPP_Disconn_Reason = MBE_COMM_FAIL_REASON_URL_CHANGED;
					pOCPP->emOCPPConnStatus = Disconnected; //need to reconnect to new server again
					//Percent_EncodeUrl(pOCPP->szServerURL); //Jimmy note 20181129: maybe do not need as URL should be correct when input on LCD
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
				
				if (err == ERRCODE_DAI_OK)
				{
					if(strncmp(bootInfo.szChargePointSerualNuml, pOCPP->szCurrChargerSN, 32) != 0)
					{
						Refresh_URL(pReporter);
						snprintf(pOCPP->szCurrChargerSN, 32,  "%s", bootInfo.szChargePointSerualNuml);
						pOCPP->emOCPP_Disconn_Reason = MBE_COMM_FAIL_REASON_CHGSN_CHANGED;
						pOCPP->emOCPPConnStatus = Disconnected; //need to reconnect to new server again
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

			MAINT_BE_STATUS_INFO* pCurr102Stat = &pOCPP->stCurr_102Info;
			int kkk;
			int nFoundIdx;
			MAINT_BE_ALARM* pMbeAlm;
			if((EVENT_MSG_BEGIN == emMsgType) || (EVENT_MSG_RESTORE == emMsgType))
			{		
				pMbeAlm = pCurr102Stat->actAlmTable;		
				nFoundIdx = -1;
				for(kkk = 0; kkk < pCurr102Stat->nActiveAlmTotal; kkk++, pMbeAlm++)
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
					if(pCurr102Stat->nActiveAlmTotal >= MAINBE_MAX_SUPPORT_ACTALM)
					{
						pMbeAlm = &pCurr102Stat->actAlmTable[MAINBE_MAX_SUPPORT_ACTALM - 1];
						//pCurr102Stat->nActiveAlmTotal = MAINBE_MAX_SUPPORT_ACTALM;						
					}
					else
					{
						pMbeAlm = &pCurr102Stat->actAlmTable[pCurr102Stat->nActiveAlmTotal];
						pCurr102Stat->nActiveAlmTotal++;
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
				pMbeAlm = pCurr102Stat->actAlmTable;	
				nFoundIdx = -1;
				for(kkk = 0; kkk < pCurr102Stat->nActiveAlmTotal; kkk++, pMbeAlm++)
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
					//pMbeAlm = &pCurr102Stat->actAlmTable[nFoundIdx];
					// printf("-----------ENDALAMR[%d] : [%d, %d, %d]--------\n", pCurr102Stat->nActiveAlmTotal,
					// 	pMbeAlm->nAlarmId, pMbeAlm->nDevId, pMbeAlm->tmStart);
					//now remove that alarm from list
					for(kkk = nFoundIdx; kkk < pCurr102Stat->nActiveAlmTotal - 1; kkk++)
					{
						memmove(&pCurr102Stat->actAlmTable[kkk], &pCurr102Stat->actAlmTable[kkk + 1], sizeof(MAINT_BE_ALARM));
					}
					pCurr102Stat->nActiveAlmTotal--;
				}				
			}
		}
		break;
	default:
		break;
	}
	return EVENT_HANDED_OK;
}

static BOOL TryConnect2Server(REPORTER* pReporter)
{
	int nErrCode;
	char szDump[256] = {0};
	static int s_nLastOpenErrCode = -999;
	MAINTAINBE_DATA_STU* pOCPP = (MAINTAINBE_DATA_STU*)pReporter->pDataHeap;
	if(pReporter->hCommPort != NULL)
	{
		nErrCode = pReporter->pUsedCommPort->Close(pReporter->hCommPort);
		TRACEX("MaintBE: Close The Comm Port errcode:%d\n", nErrCode);
		snprintf(szDump, 256, "Close the comm port[%d], reason[%d]", nErrCode, pOCPP->emOCPP_Disconn_Reason);
		DUMP_MBELog(MBE_LOGTYPE_CLOSEPORT, szDump);
		pReporter->hCommPort = NULL;
		s_nLastOpenErrCode = -999;
	}

	//pReporter->hCommPort = pReporter->pUsedCommPort->Open(pReporter->info.szReporterSetting,
	//	"ws://121.40.165.18:8800", COMM_CLIENT_MODE, 5000, &nErrCode); //5 sec timeout for connecting...
	pReporter->hCommPort = pReporter->pUsedCommPort->Open(pReporter->info.szReporterSetting,
		pOCPP->szServerURL, COMM_INFYMBE_MODE, 10000, &nErrCode); //10 sec timeout for connecting...
	//printf("MaintBE: pReporter->info.szReporterSetting =%s pOCPP->szServerURL=%s\n",pReporter->info.szReporterSetting,pOCPP->szServerURL);
	static BOOL s_bFirstTimeBoot = TRUE; //to record the BOOT time
	if(s_bFirstTimeBoot)
	{
		s_bFirstTimeBoot = FALSE;
		s_nLastOpenErrCode = nErrCode;
		memcpy(pOCPP->szServerURL_Old, pOCPP->szServerURL, 255);
		snprintf(szDump, 256, "!!!FirstTimeBoot!!! Now open the address[%s]: [%s]", pOCPP->szServerURL, (pReporter->hCommPort == NULL) ? "Failed" : "Successfylly");
		DUMP_MBELog(MBE_LOGTYPE_OPENPORT, szDump);
	}
	//static char s_strLastServerURL[255] = {0};
	else if(s_nLastOpenErrCode != nErrCode) //Jimmy added 20190411 for only record once if the network is not alive
	{
		s_nLastOpenErrCode = nErrCode;
		memcpy(pOCPP->szServerURL_Old, pOCPP->szServerURL, 255);
		snprintf(szDump, 256, "Open the address[%s]: [%s]", pOCPP->szServerURL, (pReporter->hCommPort == NULL) ? "Failed" : "Successfylly");
		DUMP_MBELog(MBE_LOGTYPE_OPENPORT, szDump);
	}
	else if(strncmp(pOCPP->szServerURL_Old, pOCPP->szServerURL, 255) != 0) //Jimmy added 20190411 for only record once if the network is not alive
	{
		memcpy(pOCPP->szServerURL_Old, pOCPP->szServerURL, 255);
		snprintf(szDump, 256, "Open the NEW address[%s]: [%s]", pOCPP->szServerURL, (pReporter->hCommPort == NULL) ? "Failed" : "Successfylly");
		DUMP_MBELog(MBE_LOGTYPE_OPENPORT, szDump);
	}

	if(pReporter->hCommPort == NULL)
	{
		//printf("MaintBE: The open errcode:%d\n", nErrCode);
		if(nErrCode != ERR_COMM_TIMEOUT)
		{
			//LogOut("OpenGUI Comm Port Failed-->%d");
			return FALSE;
		}
	}
	else
	{
		//printf("\n--------MaintBE: Open SUCESS-------\n");
		COMM_TIMEOUTS timeOut;
		timeOut.nReadTimeout = 500;
		timeOut.nWriteTimeout = 100;
		timeOut.nIntervalTimeout = 50;
		pReporter->pUsedCommPort->Control(pReporter->hCommPort,COMM_SET_TIMEOUTS,&timeOut, sizeof(COMM_TIMEOUTS));
	}

	pOCPP->bCPStatusChanged = TRUE;
	pOCPP->bVerInfo_Changed = TRUE;
	pOCPP->bHasActiveMsg = FALSE;

	return TRUE;
}

static BOOL Wait_Ready(REPORTER* pReporter)
{
	MAINTAINBE_DATA_STU* pOCPP = (MAINTAINBE_DATA_STU*)pReporter->pDataHeap;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	time_t tmNow = _NOW_;
	static time_t s_tmLastSentBootNotif = 0;
	int n;

	// static MAINTAINBE_COMM_STATUS s_emOldConnStatus = Connected;
	// static int s_nConnStatusStableCount = 0;
	// if((pOCPP->emOCPPConnStatus != s_emOldConnStatus) &&
	// 	((pOCPP->emOCPPConnStatus == Disconnected) || (pOCPP->emOCPPConnStatus == Ready)))
	// {
	// 	s_nConnStatusStableCount++;
	// 	if(s_nConnStatusStableCount >= 3)
	// 	{
	// 		s_nConnStatusStableCount = 0;
	// 		s_emOldConnStatus = pOCPP->emOCPPConnStatus;
	// 		VAR_VALUE varV;
	// 		varV.emValue = (s_emOldConnStatus == Disconnected) ? 1 : 0;
	// 		pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
	// 			1, 5, &varV, 4, 100);
	// 	}
	// }
	// else
	// {
	// 	s_nConnStatusStableCount = 0;
	// }

	if(pOCPP->emOCPPConnStatus == Ready)
	{
		pOCPP->emOCPP_Disconn_Reason = MBE_COMM_FAIL_REASON_NO_FAIL;
		return TRUE;
	}

	//int nWaitResent_Boot_Sec = 15; //(pOCPP->stESiOCPPSettings.nChargerStatusReportInterval > 0) ? 
		//pOCPP->stESiOCPPSettings.nChargerStatusReportInterval : 3; //if it is 0, charger uses 3 as default
	//nWaitResent_Boot_Sec = MIN(nWaitResent_Boot_Sec, 10); //Jimmy note 20190215:if heartbeatInterv is very large, could be very slow to try again

	Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);

	//TRACEX("Now Maint_BE status = %d, tmDiff = %d, nWaitResent_Boot_Sec = %d \n", 
	//	pOCPP->emOCPPConnStatus, ABS(tmNow - s_tmLastSentBootNotif) , nWaitResent_Boot_Sec);

	static int s_nNoResp_4Boot_Counter = 0;
	if(pOCPP->emOCPPConnStatus == Disconnected)
	{
		Mutex_Unlock(pOCPP->hMutex); //Jimmy note 20190412: do not lock here in case the event_handler will be blocked
		pOCPP->bBootNotifyDone = FALSE;
		TRACEX("ThreadEntry_MaintBEComm Enter\n");
		//pOCPP->emOCPPConnStatus = Disconnected;
		TryConnect2Server(pReporter);
		TRACEX("TryConnect2Server Out for MaintBE\n");
		if(pReporter->hCommPort == NULL)
		{
			pOCPP->emOCPP_Disconn_Reason = MBE_COMM_FAIL_REASON_SERVRER_BROKEN;
			//Mutex_Unlock(pOCPP->hMutex);
			Sleep(10000);
			//return FALSE;
		}
		else
		{
			pOCPP->emOCPPConnStatus = Connected;
		}

		s_nNoResp_4Boot_Counter = 0;
		return FALSE;
	}
	else if(pOCPP->emOCPPConnStatus == Connected)
	{
		if(s_nNoResp_4Boot_Counter >= 3)
		{
			//printf("-----No response for BOOT timeout, will re-connect just now!-----\n");
			pOCPP->emOCPPConnStatus = Disconnected;
			pOCPP->emOCPP_Disconn_Reason = MBE_COMM_FAIL_REASON_TOO_MANY_RETRY;
			Mutex_Unlock(pOCPP->hMutex);
			//Sleep((pOCPP->emOCPP_Disconn_Reason == MBE_COMM_FAIL_REASON_REFUSED) ? 120000 : 60000);
			return FALSE;
		}

		//send 100 everytime in case of SN mismatch
		//if(!pOCPP->bBootNotifyDone)
		{			
			pOCPP->stMsgInfoList[C_100_Register].pfnMsgHandler((void*)pOCPP);
			TRACEX("BootNotification Trigger for MaintBE\n");
			//if write error, then we need to goto disconnected again
			//if(pOCPP->emOCPPConnStatus != Disconnected)
			{
				pOCPP->emOCPPConnStatus = Wait4BootNotify_Conf;
				s_tmLastSentBootNotif = tmNow;
				s_nNoResp_4Boot_Counter++;
			}
		}
	}
	else if(pOCPP->emOCPPConnStatus == Wait4BootNotify_Conf) //waiting BootNotic result
	{
		pOCPP->nReadBuffLen = pComm->Read(pReporter->hCommPort, pOCPP->szReadBuffer, MBE_MAX_RECVBUFF_LEN);
		TRACEX("Wait bootNotif.conf for MaintBE nReadLen = %d\n", pOCPP->nReadBuffLen);
		if(pOCPP->nReadBuffLen > 0)
		{
			DUMP_MBELog(MBE_LOGTYPE_RECVMSG, pOCPP->szReadBuffer);
			Parse_MBE_Frames(pOCPP);
		}
		else if(pOCPP->nReadBuffLen == -1) //means server close session
		{
			pOCPP->emOCPP_Disconn_Reason = MBE_COMM_FAIL_REASON_SERVRER_BROKEN;
			pOCPP->emOCPPConnStatus = Disconnected;
		}

		if(pOCPP->bBootNotifyDone)
		{
			pOCPP->tmLast_SuccessExChange_101 = 0;
			pOCPP->tmLast_SuccessExChange_102 = 0;
			pOCPP->emOCPPConnStatus = Ready;
		}
		else if(ABS(tmNow - s_tmLastSentBootNotif) >= 10)
		{
			pOCPP->emOCPPConnStatus = Connected;
			//pOCPP->emOCPP_Disconn_Reason = ES_iOCPP_COMM_FAIL_REASON_TIMEOUT_RECEIVE;
			//pOCPP->emOCPPConnStatus = Disconnected;
		}
	}
	// else if(pOCPP->emOCPPConnStatus == BootNotify_Pending) //now we can handling OCPP configure setting things 
	// {
	// 	if((pOCPP->nReadBuffLen = pComm->Read(pReporter->hCommPort, pOCPP->szReadBuffer, OCPP_BUFFER_LEN)) > 0)
	// 	{
	// 		DUMP_OCPPMSG(pReporter, DUMP_TYPE_READ, pOCPP->szReadBuffer);
	// 		Parse_OCPPFrames(pOCPP);
	// 	}

	// 	if(pOCPP->bBootNotifyDone)
	// 	{
	// 		pOCPP->emOCPPConnStatus = Connected;
	// 	}
	// 	else if(ABS(tmNow - s_tmLastSentBootNotif) > nWaitResent_Boot_Sec)
	// 	{
	// 		pOCPP->emOCPPConnStatus = Connected;
	// 	}
	// }
	else if(pOCPP->emOCPPConnStatus == BootNotify_Rejected)
	{
		static time_t s_tmLastRejected = 0;
		pOCPP->emOCPP_Disconn_Reason = MBE_COMM_FAIL_REASON_REFUSED;
		if((s_tmLastRejected == 0) || 
			(ABS(tmNow - s_tmLastSentBootNotif) < 15))
		{
			s_tmLastRejected = tmNow;
		}
		else if(ABS(tmNow - s_tmLastRejected) >= 15) //reconnect in every 15 + 15sec
		{
			// LogOut(MAIN_MODULE_REPORTER, "ES_iOCPP", LOG_TYPE_ERROR,
			// 	"BootNotif was refused by server, now disconnect and try again...");
			s_tmLastRejected = tmNow;			
			pOCPP->emOCPPConnStatus = Connected;
		}
	}

	Mutex_Unlock(pOCPP->hMutex);

	//TRACEX("pOCPP->emOCPPConnStatus = %d \n", pOCPP->emOCPPConnStatus);

	return (pOCPP->emOCPPConnStatus == Ready) ? TRUE : FALSE;
}

static void Refresh_StatusInfo(REPORTER* pReporter)
{
	//added by Jimmy 20220905 to avoid send _SYSTEM cmd too frequently
 	static time_t s_tmLastRefreshStatus = 0;
 	if(ABS(_NOW_ - s_tmLastRefreshStatus) < 1)
 	{
 		return;
 	}	
 	s_tmLastRefreshStatus = _NOW_;

	MAINTAINBE_DATA_STU* pOCPP = (MAINTAINBE_DATA_STU*)pReporter->pDataHeap;
	//-----part1: refresh CURRNET status info-----
	int n;
	//int nGunChargeStatus[MAX_SUPPORT_GUN_NUM] = {-1, -1, -1};
	PARAM_VAL_GET valGet;
	valGet.nGetSize = 4;
	// for(n = 0; n < MAX_SUPPORT_GUN_NUM; n++)
	// {
	// 	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
	// 		201 + n, 101, &valGet, sizeof(PARAM_VAL_GET));
	// 	nGunChargeStatus[n] = valGet.val.emValue;			
	// }
	//-------sys run status------
	typedef enum tagCCUState
	{
		CCU_STATE_IDLE = 0,
		CCU_STATE_LOCKED,
		CCU_STATE_FAULTED,
		//CCU_STATE_AllBusy, //all guns are in charing
		//CCU_STATE_PendingCharge, //one or more gun is charging
		//CCU_STATE_EXITING, //now exiting
	}CCU_STATE;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			1, 4, &valGet, sizeof(PARAM_VAL_GET));
	if(valGet.val.emValue == CCU_STATE_IDLE)
	{
		pOCPP->stCurr_102Info.emSysRunStatus = 0;
	}
	//else if((valGet.val.emValue == CCU_STATE_AllBusy) || (valGet.val.emValue == CCU_STATE_PendingCharge))
	//{
	//	pOCPP->stCurr_102Info.emSysRunStatus = 1;
	//}
	else if(valGet.val.emValue == CCU_STATE_LOCKED)
	{
		pOCPP->stCurr_102Info.emSysRunStatus = 3;
	}
	else
	{
		pOCPP->stCurr_102Info.emSysRunStatus = 2;
	}
	//-------vpn run status------
	int nRet = _SYSTEM_EX("ps | grep -v grep | grep -c vcon_client");
	if(nRet < 1)
	{
		pOCPP->stCurr_102Info.emVPNConnStatus = 0; //not started yet
	}
	else
	{
		pOCPP->stCurr_102Info.emVPNConnStatus = 1; //started
		//check vonn thru netport status??
		nRet = _SYSTEM_EX("netstat -anp | grep 'vcon_client' | grep -c ESTABLISHED");
		if(nRet > 0)
		{
			pOCPP->stCurr_102Info.emVPNConnStatus = 2; //connected
		}
	}
	//-------------alarms-------------
	//will be handled in Event_Handler!!!

	//----------new-mbe-comm status------
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			1, 22, &valGet, sizeof(PARAM_VAL_GET));
	if(valGet.val.emValue == 1)
	{
		pOCPP->bIsNewMBE_MqttConnnected = TRUE;
	}
	
	//-----part2: judge if need send 102-----
	if(pOCPP->bCPStatusChanged)
	{
		return;
	}

	BOOL bStatusChanged = FALSE;
	//compare it byte by byte
	for(n = 0; n < sizeof(MAINT_BE_STATUS_INFO); n++)
	{
		BYTE byNew = *((BYTE*)&pOCPP->stCurr_102Info + n);
		BYTE byOld = *((BYTE*)&pOCPP->stLast_102Info + n);
		if(byNew != byOld)
		{
			bStatusChanged = TRUE;
			break;
		}
	}

	if(bStatusChanged)
	{
		pOCPP->bCPStatusChanged = TRUE;
		return;
	}	


	//Now judge if need update VerInfo	
	int err;
	if(!pOCPP->bVerInfo_Changed)
	{
		A_BLOB_NETWORKINFO s_OldNetInfo;
		valGet.nGetSize = sizeof(A_BLOB_NETWORKINFO);
		valGet.val.blobValue = (BYTE*)&s_OldNetInfo;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 3, &valGet, sizeof(PARAM_VAL_GET));
		if(err == ERRCODE_DAI_OK)
		{
			if((s_OldNetInfo.bIsEnable_LTEModem != pOCPP->blbCurrNetInfo.bIsEnable_LTEModem) ||
				(s_OldNetInfo.lteModemINfo.nSignal_Level != pOCPP->blbCurrNetInfo.lteModemINfo.nSignal_Level) ||
				(strncmp(s_OldNetInfo.lteModemINfo.szNetwork_Type, pOCPP->blbCurrNetInfo.lteModemINfo.szNetwork_Type,32) != 0) ||
				(s_OldNetInfo.netInfo_usb0.emNetPort_Status != pOCPP->blbCurrNetInfo.netInfo_usb0.emNetPort_Status) ||
				(ABS(s_OldNetInfo.lteModemINfo.nSignal_dBm - pOCPP->blbCurrNetInfo.lteModemINfo.nSignal_dBm) >= 10))
			{
				pOCPP->bVerInfo_Changed = TRUE;
				memcpy(&pOCPP->blbCurrNetInfo, &s_OldNetInfo, sizeof(A_BLOB_NETWORKINFO));
			}			
		}

		if(!pOCPP->bVerInfo_Changed)
		{
			//now judge CCU version change
			A_BLOB_CHARGERSPECIFIC_SETTINGS ccuSet;
			valGet.nGetSize = sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS);
			valGet.val.blobValue = (BYTE*)&ccuSet;
			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				200, 200, &valGet, sizeof(PARAM_VAL_GET));
			if(err == ERRCODE_DAI_OK)
			{
				if(strncmp(pOCPP->blbCurrCCUSet.ccuSettings.szSWVer, ccuSet.ccuSettings.szSWVer, 8) != 0)
				{
					pOCPP->bVerInfo_Changed = TRUE;
				}				
			}
		}
	}	
}

static void MonitorServSStatus_AndManageVPN(MAINTAINBE_DATA_STU* pOCPP)
{
	BOOL bNeedDisconn_VPN = FALSE;
	static char s_strServOldVer[32] = {0};
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	//added by Jimmy 20220905 in case re-sent the cmd too frequently
 	time_t tmNow = _NOW_;
 	static time_t s_tmLastSendDisconnVPN = 0;
 	if(ABS(tmNow - s_tmLastSendDisconnVPN) < 5)
 	{
 		return;
 	}

	//-------Step 1: judge if server sw version is changed (released new ver)
	if(strlen(pOCPP->szMBE_SWVer) > 0)
	{
		if(strncmp(pOCPP->szMBE_SWVer, s_strServOldVer, 32) != 0)
		{
			//printf("---the Serv-Ver changed [%s] => [%s]---\n", s_strServOldVer, pOCPP->szMBE_SWVer);
			snprintf(s_strServOldVer, 32,  "%s", pOCPP->szMBE_SWVer);
			bNeedDisconn_VPN = TRUE;
		}		
	}

	//-------Step 2: judge if disconnection from server for long than 300s
	//time_t tmNow = _NOW_;
	static int s_nDisconnCounter = 0;
	if(pOCPP->emOCPPConnStatus != Ready)  
	{		
		s_nDisconnCounter++;
		if(s_nDisconnCounter > 600)
		{
			bNeedDisconn_VPN = TRUE;
		}
	}
	//-------Step 3: judge if disconnection from server when VPN is used longer than 30min
	else if(ABS(tmNow - pOCPP->tmLastExe_VPNStartCmd) > 1800)
	{
		s_nDisconnCounter = 0;
		bNeedDisconn_VPN = TRUE;
	}
	else
	{
		s_nDisconnCounter = 0;
	}

	if(bNeedDisconn_VPN)
	{
		//diconn VPN if it was connected
		if(pOCPP->stCurr_102Info.emVPNConnStatus > 0)
		{
			s_tmLastSendDisconnVPN = tmNow;
			VAR_VALUE varV;	
			varV.emValue = 0;
			pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
					1, 22, &varV, 4, 0);			
		}
	}
}

static DWORD _ThreadEntry_MBEComm(void* pArg)
{
	REPORTER* pReporter = (REPORTER*)pArg;
	HANDLE hPort = pReporter->hCommPort;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	TRACEX("_ThreadEntry_MBEComm: Start Enter \n");
	int nReadBytes = 0;
	int n;
	MAINTAINBE_DATA_STU* pOCPP = (MAINTAINBE_DATA_STU*)pReporter->pDataHeap;
	int nWaitRebootMsgCount = 0;
	Sleep(10000); //sleep 10 sec to wait other tasks to be ready
	while (!pReporter->bExit)
	{
		RUN_THREAD_HEARTBEAT();
		time_t tmNow = _NOW_;

		//added by Jimmy 20220824
		if(pOCPP->bIsNewMBE_MqttConnnected)
		{
			if(pReporter->hCommPort != NULL)
			{
				int nErrCode = pReporter->pUsedCommPort->Close(pReporter->hCommPort);
				TRACEX("MaintBE: Close The Comm Port errcode:%d\n", nErrCode);
				char szDump[256] = {0};
				snprintf(szDump, 256, "Close the comm port[%d], because NEW MBE-Mqtt is in use!", nErrCode);
				DUMP_MBELog(MBE_LOGTYPE_CLOSEPORT, szDump);
				pReporter->hCommPort = NULL;
			}
			else
			{
				static BOOL s_bHasLogged = FALSE;
				if(!s_bHasLogged)
				{
					s_bHasLogged = TRUE;
					LogOut(MAIN_MODULE_REPORTER, "MBE-Old-WS", LOG_TYPE_WARING,
					 	"Found new MBE-Mqtt coming, so will close the old MBE channel...");
				}
			}
			Sleep(3600 * 1000);
			continue;
		}

		if(!Wait_Ready(pReporter))
		{
			Sleep(100);
			continue;
		}

		//if(!pOCPP->bHasActiveMsg)
		{
			//Now check status change
			Refresh_StatusInfo(pReporter);
			
			//added by Jimmy to monitor the ServerStatus and disconnet the VPN when needed
			MonitorServSStatus_AndManageVPN(pOCPP);			
		}

		//added by Jimmy 20190702 for OCTT test. if disconn, should skip going ahead
		if(pOCPP->emOCPPConnStatus == Disconnected)
		{
			Sleep(100);
			continue;
		}
		else if(pOCPP->bNoticedExitOCPP)
		{
			//if(pOCPP->nQUrgentMsgCount < 1)
			{
				//printf("-----Now ready for MBEComm exiting-----\n");
				pOCPP->bNoticedExitOCPP = FALSE;
				pOCPP->emOCPP_Disconn_Reason = MBE_COMM_FAIL_SYSTEM_EXIT;
				pOCPP->emOCPPConnStatus = Disconnected; //need handle urgent msg before exit
				Sleep(100);
				continue;
			}
		}
		
		//Now read and parse
		pOCPP->nReadBuffLen = pComm->Read(pReporter->hCommPort, pOCPP->szReadBuffer, MBE_MAX_RECVBUFF_LEN);
		if(pOCPP->nReadBuffLen > 0)
		{
			DUMP_MBELog(MBE_LOGTYPE_RECVMSG, pOCPP->szReadBuffer);
			Parse_MBE_Frames(pOCPP);
			Sleep(100);
			//continue; //commented by Jimmy 20220225
		}
		else if(pOCPP->nReadBuffLen == -1) //means server close session
		{
			TRACEX("Server Connection Error Close the Socket\n");
			pOCPP->nReadBuffLen = 0;      // need to make Zero Otherwise same Case Excuting Continue
			pOCPP->emOCPP_Disconn_Reason = MBE_COMM_FAIL_REASON_SERVRER_BROKEN;
			pOCPP->emOCPPConnStatus = Disconnected;
			Sleep(100);
			continue;
		}

		if(pOCPP->bHasActiveMsg)
		{
			//if no response from server in 15sec, we think the server is broken
			if(ABS(tmNow - pOCPP->tmLastSentCall) > 3)
			{
				pOCPP->tmLastSentCall = tmNow;
				static int s_nNoReplyCounter = 0;
				s_nNoReplyCounter++;
				if(s_nNoReplyCounter >= 4)
				{
					s_nNoReplyCounter = 0;
					pOCPP->emOCPP_Disconn_Reason = MBE_COMM_FAIL_REASON_TIMEOUT_RECEIVE;
					pOCPP->emOCPPConnStatus = Disconnected;
				}
			}
			//while in waiting avtiveMsg, we do not handle other msg, or else there will be repeated msg in queue!
			Sleep(50);
			continue;
		}		
		
		//need send call at least 1 sec interval
		if(ABS(tmNow - pOCPP->tmLastSentCall) < 1)
		{
			Sleep(100);
			continue;
		}

		//check if need send version info
		if(pOCPP->bVerInfo_Changed || (ABS(tmNow - pOCPP->tmLast_SuccessExChange_101) >= SEC_PER_DAY))
		{
			//added by Jimmy 20211028: ver-info should send in at least 5min to avoid too much data traffic 
			if(ABS(tmNow - pOCPP->tmLast_SuccessExChange_101) >= 300)
			{
				//pOCPP->bVerInfo_Changed = FALSE;			
				pOCPP->stMsgInfoList[C_101_VersionInfo].pfnMsgHandler((void*)pOCPP);
			}								
		}
		//check status change
		else if(pOCPP->bCPStatusChanged || (ABS(tmNow - pOCPP->tmLast_SuccessExChange_102) >= 3600))
		{
			//pOCPP->bCPStatusChanged = FALSE;			
			pOCPP->stMsgInfoList[C_102_StatusNotif].pfnMsgHandler((void*)pOCPP);					
		}
		
		//check ping-pong
		static int s_nNoPong_Counter = 0;
		if((ABS(tmNow - pOCPP->tmLastRecivedPongFromServer) >= 10) &&
				(ABS(tmNow - pOCPP->tmLastSuccessExchange) >= 10))
		{
			//now try to get pong from server
			pReporter->pUsedCommPort->Control(pReporter->hCommPort, COMM_GET_LASTTIME_RECEIVE_PONG, 
				(void*)&pOCPP->tmLastRecivedPongFromServer, sizeof(time_t));
			//Jimmy note20200609: just use the same timeout for normal comm judge
			if(ABS(tmNow - pOCPP->tmLastRecivedPongFromServer) >= 30)
			{	
				s_nNoPong_Counter++;
				TRACEX("no pong received[%d] \n", s_nNoPong_Counter);
				if(s_nNoPong_Counter >= 100) //30s + 10s
				{
					pOCPP->emOCPP_Disconn_Reason = MBE_COMM_FAIL_REASON_NO_PONG;
					s_nNoPong_Counter = 0;
					pOCPP->emOCPPConnStatus = Disconnected;
					TRACEX("no pong received, so disconnected \n");
				}
			}
			//need sent ws ping, if no pong then re-send it every 10 seconds
			if(ABS(tmNow - pOCPP->tmLastSendPingToServer) >= 10)
			{
				TRACEX("-----Now sending out a ws ping to ws server-----\n");
				pReporter->pUsedCommPort->Control(pReporter->hCommPort, COMM_SEND_WS_PING, NULL, 0);
				pOCPP->tmLastSendPingToServer = tmNow;
			}
		}
		else
		{
			s_nNoPong_Counter = 0;
		}

		Sleep(100);
	}

	TRACEX("_ThreadEntry_MBEComm: Exit\n");
	return THREAD_CANCEL_ALL;
}

#define INIT_MBE_MSG(_pMsg,_emMsgType, _nMsgId, _emMsgSrc, _pfnMsgHandler) \
	do{\
	(_pMsg)->emMsgType = (_emMsgType); \
	(_pMsg)->nMsgId = (_nMsgId); \
	(_pMsg)->emMsgSrc = (_emMsgSrc); \
	(_pMsg)->pfnMsgHandler = (_pfnMsgHandler); \
	(_pMsg)++; \
	}while(0)

typedef struct tagEvHandTags
{
	int nEvTag;
	int nMsgTag;
}EVHAND_TAGS;

DLLExport BOOL Initialize(HANDLE pReporterThis)
{
	TRACEX("Initialize MaintBE Start !! \n");
	
	REPORTER* pReporter = (REPORTER*)pReporterThis;
	int n;
	PARAM_VAL_GET valGet;


	//--------for test purpose
	//return TRUE;

	//Create data
	pReporter->pDataHeap = NEW(MAINTAINBE_DATA_STU, 1);
	if(pReporter->pDataHeap == NULL)
	{
		TRACEX("Memory full for MaintBE dataStructs!\n");
		return FALSE;
	}

	MAINTAINBE_DATA_STU* pOCPP = (MAINTAINBE_DATA_STU*)pReporter->pDataHeap;
	ZERO_POBJS(pOCPP, 1);
	pOCPP->hReporter = pReporterThis;
	
	MAINTAINBE_MSG_INFO* p = &(pOCPP->stMsgInfoList[0]);
	//!!!!!!!!!Notice Macro MUST be in the right order because we use it for INDEXING!!!!!!!!!/**/
	INIT_MBE_MSG(p,	C_100_Register,				100,			MaitainBe_MSG_CP,		Prepare_C100_Regsiter);
	INIT_MBE_MSG(p,	C_101_VersionInfo,			101,			MaitainBe_MSG_CP,		Prepare_C101_VersionInfo);
	INIT_MBE_MSG(p,	C_102_StatusNotif,			102,			MaitainBe_MSG_CP,		Prepare_C102_StatusNotif);
	INIT_MBE_MSG(p,	S_201_StartVPN,				201,			MaitainBe_MSG_CS,		Handle_S201_StartVPN);
	INIT_MBE_MSG(p,	S_202_StopVPN,				202,			MaitainBe_MSG_CS,		Handle_S202_StopVPN);

	pOCPP->hMutex = Mutex_Create(TRUE);
	if(pOCPP->hMutex == NULL)
	{
		TRACEX("MaintBE ERROR : Fail to Create Mutex and Queue \n");
		return FALSE;
	}
	
	//20190223:Now initial the TSL root CA
	char	szRootCAFile[255] = {0};
	MakeFullFileName(szRootCAFile, \
		getenv(ENV_VARIABLE_APP_PATH), "config", "cacert.pem");
	pReporter->pUsedCommPort->Control(NULL, COMM_SET_WS_GLOBAL_TLS_ROOT_CAFILE, (void*)szRootCAFile, 0);

	pOCPP->emFirmwareUpgradeStatus = FIRMUPDATE_Idle;

	//server URL , format[ end url + "/ChargerID"]	
	// valGet.nGetSize = 255;
	// char szEndUrl[255];
	// valGet.val.blobValue = (BYTE*)szEndUrl; //&pOCPP->szServerURL;
	// ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
	// 	1, 72, &valGet, sizeof(PARAM_VAL_GET));
	// if (err != ERRCODE_DAI_OK)
	// {
	// 	TRACEX("ERROR: MaintBE server URL can not be retrieved\n");
	// 	return FALSE;
	// }
	// snprintf(pOCPP->szServerURL, 255, "%s", szEndUrl);
	Refresh_URL(pReporter);
	memcpy(pOCPP->szServerURL_Old, pOCPP->szServerURL, 255);

	pReporter->bExit = FALSE;
	
	pReporter->hThread = RunThread_Create("MBE_CommThread",
		(RUN_THREAD_START_PROC)_ThreadEntry_MBEComm,
		(void *)(pReporter),
		(DWORD*)&pReporter->bExit);
		
	if(pReporter->hThread == NULL)
	{
		TRACEX("ERROR : Create MBE_CommThread thread Failed \n");
		return FALSE;
	}
	
	//Init ev handler
	EVHAND_TAGS tags[] =
	{
			{ EVENT_TYPE_ALARM,			EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_RESTORE},
			{ EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN },
			{ EVENT_TYPE_CHARGING,			EVENT_MSG_BEGIN | EVENT_MSG_END/* | EVENT_MSG_RESTORE */| EVENT_MSG_STATUS_CHANGED },
			{ EVENT_TYPE_USER_AUTHORIZE,	EVENT_MSG_BEGIN }, //from CR or OCPP
			{ EVENT_TYPE_FIRMWARE_UPDATE,	EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
			{ EVENT_TYPE_DIAGNOSTIC_UPLOAD,EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
			{ EVENT_TYPE_ABILITY_CHANGE,	EVENT_MSG_BEGIN | EVENT_MSG_END }, //do not treat this, use status change event instead
			{ EVENT_TYPE_SYSTEM_EXIT,		EVENT_MSG_END},
			//{EVENT_TYPE_METERVAL_UPDATE_IN_CHARGING,	EVENT_MSG_BEGIN},
	};

	for (n = 0; n < ITEM_OF(tags); n++)
	{
		pReporter->pDAI->Regsiter_EvHandler("OCPP_CommEventHand",
			pReporterThis,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}

	return TRUE;
}

static UINT Make_UniqID_MBE(void)
{
	//time_t tmNow = _NOW_;
	//double tmPrecNow = GetCurrentTime();
	//double diffMsec= (tmPrecNow - tmNow) * 100.0;

	//UINT nUniqID = tmNow * 100 + (UINT)diffMsec;
#define MBE_UNIQID_START	556892135
	static UINT s_nOldUniqID = 0;
	if(s_nOldUniqID == 0)
	{
		s_nOldUniqID = (UINT)_NOW_ + MBE_UNIQID_START; //just to mix the value
	}
	//static UINT s_nOldUniqID = OCPP_UNIQID_START;
	UINT nUniqID = s_nOldUniqID;

	s_nOldUniqID++;
	if(s_nOldUniqID == 0xFFFFFFFF)
	{
		s_nOldUniqID = MBE_UNIQID_START;
	}

	TRACEX("---The uniqID = %lu----\n", nUniqID);

	return nUniqID;
}


void Send_Call(MAINTAINBE_DATA_STU* pOCPP, MAINTAINBE_MSG_TYPE emMsgType, cJSON* pJson)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	char *szData = 0;
	szData = cJSON_Print(pJson);
	TRACEX("MaintBE: MsgCall[%d], msgSrc = %d, Json_Print = %s\n",pOCPP->stMsgInfoList[emMsgType].nMsgId, 
		pOCPP->stMsgInfoList[emMsgType].emMsgSrc,
		szData);

	cJSON_Delete(pJson);

	UINT nMsgUniqID = Make_UniqID_MBE(); //(UINT)(GetCurrentTime() * 1000);
	
	snprintf(pOCPP->szSendBuffer, MBE_MAX_SENBUFF_SIZE, "{\"nm\":\"A%d\",	\"uId\":\"%u\",	\"msg\":%s}",
		pOCPP->stMsgInfoList[emMsgType].nMsgId, nMsgUniqID, szData);

	pOCPP->nSendBuffLen = strlen(pOCPP->szSendBuffer);

	free(szData);

	char szTime[32];
	//printf("----[%s]: Send out MBE_CALL: %s \n", AcurTimeToString(GetCurrentTime(), TIME_CHN_FMT, szTime, 32), pOCPP->szSendBuffer);

    if(pReporter->pUsedCommPort->Write(pReporter->hCommPort, pOCPP->szSendBuffer, pOCPP->nSendBuffLen) > 0)
	{
		DUMP_MBELog(MBE_LOGTYPE_SENDMSG, pOCPP->szSendBuffer);
	}

	pOCPP->bHasActiveMsg = TRUE;	
	pOCPP->tmLastSentCall = _NOW_;
}

void Send_CallResult(MAINTAINBE_DATA_STU* pOCPP, MAINTAINBE_MSG_TYPE emMsgType, cJSON* pJson)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	char *szData = 0;
	szData = cJSON_Print(pJson);
	TRACEX("MaintBE: Send_CallResult Json_Print = %s\n",szData);

	cJSON_Delete(pJson);

	//UINT nMsgUniqID = Make_UniqID_MBE();
	
	snprintf(pOCPP->szSendBuffer, MBE_MAX_SENBUFF_SIZE, "{\"nm\":\"R%d\",	\"uId\":\"%s\",	\"msg\":%s}",
		pOCPP->stMsgInfoList[emMsgType].nMsgId, /*nMsgUniqID*/
		pOCPP->szHandlingUniqID, szData);

	pOCPP->nSendBuffLen = strlen(pOCPP->szSendBuffer);

	free(szData);

	TRACEX("MaintBE: EVERYMSG == %s  Length = %d \n", pOCPP->szSendBuffer, pOCPP->nSendBuffLen);

	if(pReporter->pUsedCommPort->Write(pReporter->hCommPort, pOCPP->szSendBuffer, pOCPP->nSendBuffLen) > 0)
	{
		DUMP_MBELog(MBE_LOGTYPE_SENDMSG, pOCPP->szSendBuffer);
	}

	pOCPP->bHasActiveMsg = FALSE;
}

void Send_CallError(MAINTAINBE_DATA_STU* pOCPP,  MAINTAINBE_MSG_TYPE emMsgType, MAINTAINBE_CALLERROR_CODE emCode, const char* szErrDescrip)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	snprintf(pOCPP->szSendBuffer, MBE_MAX_SENBUFF_SIZE, "{\"nm\":\"E%d\",	\"uId\":\"%s\",	\"msg\":{\"code\":%d, \"err_desp\":\"%s\"}}",
		pOCPP->stMsgInfoList[emMsgType].nMsgId,
		/*Make_UniqID_MBE()*/pOCPP->szHandlingUniqID,
		emCode, szErrDescrip);

	pOCPP->nSendBuffLen = strlen(pOCPP->szSendBuffer);
	TRACEX("MaintBE: EVERYMSG == %s  Length = %d \n", pOCPP->szSendBuffer, pOCPP->nSendBuffLen);

	pReporter->pUsedCommPort->Write(pReporter->hCommPort, pOCPP->szSendBuffer, pOCPP->nSendBuffLen);

	DUMP_MBELog(MBE_LOGTYPE_SENDERR, pOCPP->szSendBuffer);

	pOCPP->bHasActiveMsg = FALSE;
}

