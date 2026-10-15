//ocpp.c
#include "../reporters_base.h"
//#include "../cJSON.h"
#include "ocpp.h"
#include <signal.h>

static int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE	emEvType,
	_EVENT_MSG	emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int			nMaxBlockMSec)
{
	REPORTER* pReporter = (REPORTER*)hModuleSeif;
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pReporter->pDataHeap;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	int n;
	RunThread_Heartbeat(pReporter->hThread); //in case the thread no response
	switch (emEvType)
	{
	case EVENT_TYPE_USER_AUTHORIZE:
		{
			EVENT_AUTHORIZE * pAuthorizaData = (EVENT_AUTHORIZE*)pEvData;
			if (nEvDataSize != sizeof(EVENT_AUTHORIZE))
			{
				return EVENT_HANDED_FAILED;
			}
			else if(pOCPP->emOCPPConnStatus != Ready)
			{
				return EVENT_HANDED_FAILED;
			}
			else if(pOCPP->bAuthorizeInProcess) //another auth is in handing, ignore
			{
				return EVENT_HANDED_FAILED;
			}
			//Jimmy: for block/none-block mode difference, we can treat in the future
			TRACEX("OCPP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_USER_AUTHORIZE \t emMsgType = MSG_BEGIN %s \n", pAuthorizaData->szIdTocken);
			Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
			ZERO_POBJS(&pOCPP->stAuthData, 1);
			memcpy(pOCPP->stAuthData.szIdTag, pAuthorizaData->szIdTocken, 20);
			pOCPP->bAuthorizeInProcess = TRUE;
			//Now call the msg handler to create the msg to send
			pOCPP->stMsgInfoList[Authorize].pfnMsgHandler((void*)pOCPP);
			Mutex_Unlock(pOCPP->hMutex);
		}
		break;

	case EVENT_TYPE_CHARGING :
		{
			TRACEX("OCPP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_CHARGING \t emMsgType = %d \n", emMsgType);
			EVENT_CHARGING * pEvChag = (EVENT_CHARGING*)pEvData;
			if (nEvDataSize != sizeof(EVENT_CHARGING))
			{
				return EVENT_HANDED_FAILED;
			}
			if(pEvChag->emGunNum > Max_Connctors)
			{
				return EVENT_HANDED_FAILED;
			}

			pEvChag->emGunNum -= 1; //this is from 1~3

			if((EVENT_MSG_BEGIN == emMsgType) || (EVENT_MSG_END == emMsgType))
			{
				if((pEvChag->emChangeType == CHG_TYPE_GUN_INCHARGE) && 
					(pEvChag->emGunNum >= 0) && (pEvChag->emGunNum < Max_Connctors))
				{
					Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
					pOCPP->emHandlingConnector = pEvChag->emGunNum;
					pOCPP->bTransMsgFromHistory = FALSE;
					TRACEX("pEvChag->emGunNum = %d \n", pEvChag->emGunNum);
					//Now call the msg handler to create the msg to send
					if(EVENT_MSG_END == emMsgType)
					{
						A_BLOB_RUN_CHARGE_INFO  StopTx;
						PARAM_VAL_GET valGet;
						valGet.nGetSize = sizeof(A_BLOB_RUN_CHARGE_INFO);
						valGet.val.blobValue = (BYTE*)&StopTx;
						pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
							pEvChag->emGunNum + 201, 107, &valGet, sizeof(PARAM_VAL_GET));
						if(!StopTx.extraData.bHasStoppedByDeAuth) //in this case we already send it once Suspend the EVSE, do not send it repeatedly
						{
							pOCPP->stMsgInfoList[StopTransaction].pfnMsgHandler((void*)pOCPP);
						}
					}
					else
					{
						pOCPP->stMsgInfoList[StartTransaction].pfnMsgHandler((void*)pOCPP);
					}
					//(EVENT_MSG_BEGIN == emMsgType) ? pOCPP->stMsgInfoList[StartTransaction].pfnMsgHandler((void*)pOCPP) :
					//	pOCPP->stMsgInfoList[StopTransaction].pfnMsgHandler((void*)pOCPP);
					//if((emMsgType == EVENT_MSG_END) && (pOCPP->emOCPPConnStatus == Ready)) //when end of charging, will send out this once 
					//{
					//	pOCPP->stConnectorData[pEvChag->emGunNum].bConn_StatusChanged++;
					//	//pOCPP->stConnectorData[pEvChag->emGunNum].nGunChargeStatus = GUN_STATE_IDLE;
					//	//pOCPP->emHandlingConnector = pEvChag->emGunNum;
					//	//pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP);
					//}
					Mutex_Unlock(pOCPP->hMutex);
				}
			}
			else if(EVENT_MSG_STATUS_CHANGED == emMsgType)
			{
				if(pEvChag->emGunNum == (BYTE)Connctor_0)
				{
					TRACEX("Status Change Event for The Connnector 0 pEvChag->emChangeType[%d] \n ",
						pEvChag->emChangeType);

					if(pOCPP->tmLastCPStatusChange == 0)
					{
						return EVENT_HANDED_FAILED;
					}

					//handel exit
					if(CHG_TYPE_SYS_INEXIT == pEvChag->emChangeType)
					{
						Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
						pOCPP->emHandlingConnector = Connctor_0;
						pOCPP->emTriggerMsgType = Trigger_StatusNotification; //so that send it urgent
						pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP);
						pOCPP->bNoticedExitOCPP = TRUE;
						//pOCPP->emOCPPConnStatus = Disconnected;
						Mutex_Unlock(pOCPP->hMutex);
						return EVENT_HANDED_OK;
					}

					A_BLOB_CAHRGEPOINT_STATUSINFO StatusNotiData;
					valGet.nGetSize = sizeof(A_BLOB_CAHRGEPOINT_STATUSINFO);
					valGet.val.blobValue = (BYTE*)&StatusNotiData;
					err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
						1, 10, &valGet, sizeof(PARAM_VAL_GET));
					if (err != ERRCODE_DAI_OK)
					{
						return EVENT_HANDED_FAILED;
					}

					if((StatusNotiData.emChargePointStatus != pOCPP->blobCPStatusInfo.emChargePointStatus) ||
						(StatusNotiData.emChargePointErrcode != pOCPP->blobCPStatusInfo.emChargePointErrcode))
					{
						Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
						//pOCPP->nCurrChargeStatusHandled_Conn = pEvChag->emGunNum; // for Zero connector 
						//pOCPP->byCuurConn_ChgType = pEvChag->emChangeType;
						memcpy(&pOCPP->blobCPStatusInfo, &StatusNotiData, sizeof(A_BLOB_CAHRGEPOINT_STATUSINFO));
						pOCPP->bCPStatusChanged = TRUE;
						//Now call the msg handler to create the msg to send
						//pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP);
						Mutex_Unlock(pOCPP->hMutex);
					}
				}
				else if((pEvChag->emGunNum >= Connctor_1) && (pEvChag->emGunNum < Max_Connctors))
				{
					TRACEX("Status Change Event for The Connnectors [%d] \n ", pEvChag->emGunNum + 1);
					//Jimmy note20190301: always refresh all the statuses, to avoid faulted transfer from Finishing to Preparing
					BOOL bStatusChange = FALSE;
					//if(pEvChag->emChangeType == CHG_TYPE_GUN_PLUG)
					{
						valGet.nGetSize = 4;
						err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
							201 + pEvChag->emGunNum, 102, &valGet, sizeof(PARAM_VAL_GET));
						if (err != ERRCODE_DAI_OK)
						{
							return EVENT_HANDED_FAILED;
						}
						if(pOCPP->stConnectorData[pEvChag->emGunNum].nGunPlugStatus != valGet.val.emValue)
						{
							bStatusChange = TRUE;
						}
						//pluged / unpluged
						pOCPP->stConnectorData[pEvChag->emGunNum].nGunPlugStatus = valGet.val.emValue;
					}
					//else if(pEvChag->emChangeType == CHG_TYPE_GUN_PREPARE_STAT)
					{
						valGet.nGetSize = 4;
						err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
							201 + pEvChag->emGunNum, 99, &valGet, sizeof(PARAM_VAL_GET));
						if (err != ERRCODE_DAI_OK)
						{
							return EVENT_HANDED_FAILED;
						}
						if(pOCPP->stConnectorData[pEvChag->emGunNum].nGunPrepareStatus != valGet.val.emValue)
						{
							bStatusChange = TRUE;
						}
						//prepare status
						pOCPP->stConnectorData[pEvChag->emGunNum].nGunPrepareStatus = valGet.val.emValue;
					}
					//else
					{
						valGet.nGetSize = 4;
						err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
							201 + pEvChag->emGunNum, 101, &valGet, sizeof(PARAM_VAL_GET));
						if (err != ERRCODE_DAI_OK)
						{
							return EVENT_HANDED_FAILED;
						}
						if(pOCPP->stConnectorData[pEvChag->emGunNum].nGunChargeStatus != valGet.val.emValue)
						{
							bStatusChange = TRUE;
						}
						pOCPP->stConnectorData[pEvChag->emGunNum].nGunChargeStatus = valGet.val.emValue;
					}
					//added by Jimmy 20190606 for suspendEVSE status change
					{
						valGet.nGetSize = 4;
						err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
							201 + pEvChag->emGunNum, 120, &valGet, sizeof(PARAM_VAL_GET));
						if (err == ERRCODE_DAI_OK)
						{
							if(pOCPP->stConnectorData[pEvChag->emGunNum].nGunSuspendEVSESatus != valGet.val.emValue)
							{
								bStatusChange = TRUE;
							}
							pOCPP->stConnectorData[pEvChag->emGunNum].nGunSuspendEVSESatus = valGet.val.emValue;
						}
					}

					TRACEX("The gun[%d] statuses = [%d, %d, %d], bStatusChg = %d\n", pEvChag->emGunNum + 1,
						pOCPP->stConnectorData[pEvChag->emGunNum].nGunPlugStatus,
						pOCPP->stConnectorData[pEvChag->emGunNum].nGunPrepareStatus,
						pOCPP->stConnectorData[pEvChag->emGunNum].nGunChargeStatus,
						bStatusChange);
					if(!bStatusChange)
					{
						return EVENT_HANDED_FAILED;
					}

					//if first time for connector, main entry will handle this
					if(pOCPP->stConnectorData[pEvChag->emGunNum].tmLastConn_StatusChange == 0)
					{
						return EVENT_HANDED_FAILED;
					}

					Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
					//pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP, TRUE);
					//pOCPP->stConnectorData[pEvChag->emGunNum].bConn_StatusChanged = TRUE;
					if(pOCPP->emOCPPConnStatus == Ready)
					{
						//trans-related msg should be handled urgently
						if(pOCPP->stConnectorData[pEvChag->emGunNum].nGunChargeStatus >= GUN_STATE_WAITING_START)
						{
							pOCPP->emHandlingConnector = pEvChag->emGunNum;
							pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP);
						}
						else
						{
							TRACEX("NEEEEEE[%d] bStatusChg = %d\n", pEvChag->emGunNum + 1, pOCPP->stConnectorData[pEvChag->emGunNum].bConn_StatusChanged);
							pOCPP->stConnectorData[pEvChag->emGunNum].bConn_StatusChanged++;
						}
					}
					Mutex_Unlock(pOCPP->hMutex);
				}
			}
		}
		break;
	case EVENT_TYPE_METERVAL_UPDATE_IN_CHARGING:
		{
			TRACEX("OCPP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_METERVAL_UPDATE_IN_CHARGING \t emMsgType = %d \n", emMsgType);
			EVENT_METERVALUE_UPDATE* pEvMeter = (EVENT_METERVALUE_UPDATE*)pEvData;
			if (nEvDataSize != sizeof(EVENT_METERVALUE_UPDATE))
			{
				return EVENT_HANDED_FAILED;
			}
			int nGunIdx = pEvMeter->pMeterRecord->nGunNum - 1;
			if((nGunIdx >= Connctor_1) && (nGunIdx < Max_Connctors))
			{
				if(pEvMeter->pMeterRecord->tmChargeStart == 
					pOCPP->stConnectorData[nGunIdx].blobInHandling_ChargeInfo.tmChargeStart)
				{
					Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
					pOCPP->bTransMsgFromHistory = FALSE;
					if(pEvMeter->emMeteVal_UpdateType == METER_VAL_UPDATETYPE_BEGIN)
					{
						pOCPP->emHandlingConnector = nGunIdx;
						pOCPP->emHandlingMeterType = Transaction_Begin;
						memcpy(&pOCPP->currHandlingHisMeterVal, &pEvMeter->pMeterRecord->meterData.stMeterVal_Start,
							sizeof(A_CHARGE_METERVALUE_ITEM));
						pOCPP->stMsgInfoList[MeterValues].pfnMsgHandler((void*)pOCPP);
					}
					else if(pEvMeter->emMeteVal_UpdateType == METER_VAL_UPDATETYPE_END)
					{
						pOCPP->emHandlingConnector = nGunIdx;
						pOCPP->emHandlingMeterType = Transaction_End;
						memcpy(&pOCPP->currHandlingHisMeterVal, &pEvMeter->pMeterRecord->meterData.stMeterVal_Stop,
							sizeof(A_CHARGE_METERVALUE_ITEM));
						pOCPP->stMsgInfoList[MeterValues].pfnMsgHandler((void*)pOCPP);
					}
					else
					{
						int nIdx = pEvMeter->nUpdateIdx;
						if((nIdx >= 0) && (nIdx < MAX_HIS_STORE_METER_VALUES))
						{
							pOCPP->emHandlingConnector = nGunIdx;
							pOCPP->emHandlingMeterType = Sample_Periodic;
							memcpy(&pOCPP->currHandlingHisMeterVal, &pEvMeter->pMeterRecord->meterData.stMeterValList[nIdx],
								sizeof(A_CHARGE_METERVALUE_ITEM));
							pOCPP->stMsgInfoList[MeterValues].pfnMsgHandler((void*)pOCPP);
						}
					}
					Mutex_Unlock(pOCPP->hMutex);
				}
			}
		}
		break;
	case EVENT_TYPE_FIRMWARE_UPDATE:
		{
			TRACEX("OCPP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_FIRMUPDATE \t emMsgType = %d, size[%d,%d] \n", 
				emMsgType, nEvDataSize, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
			EVENT_FIRMWAREUPDATE_STATUS * pFirmUpSta = (EVENT_FIRMWAREUPDATE_STATUS*)pEvData;
			if (nEvDataSize != sizeof(EVENT_FIRMWAREUPDATE_STATUS))
			{
				return EVENT_HANDED_FAILED;
			}
			Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
			pOCPP->emFirmwareUpgradeStatus = pFirmUpSta->emUpdateStatus;
			//Now call the msg handler to create the msg to send
			pOCPP->stMsgInfoList[FirmwareStatusNotification].pfnMsgHandler((void*)pOCPP);
			Mutex_Unlock(pOCPP->hMutex);
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
			Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
			pOCPP->emDiagnosUploadStatus = pDiagUpSta->emUploadStatus;
			//Now call the msg handler to create the msg to send
			pOCPP->stMsgInfoList[DiagnosticsStatusNotification].pfnMsgHandler((void*)pOCPP);
			Mutex_Unlock(pOCPP->hMutex);
		}
		break;
	case EVENT_TYPE_ALARM:
		{
			TRACEX("OCPP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_ALARM * pAlmEv = (EVENT_ALARM*)pEvData;
			if (nEvDataSize != sizeof(EVENT_ALARM))
			{
				return EVENT_HANDED_FAILED;
			}
			//Meter interrupt alarm
			if((pAlmEv->nAlarmId == 401) && ((EVENT_MSG_BEGIN == emMsgType) || (EVENT_MSG_END == emMsgType)))
			{
				int nGunIdx = pAlmEv->nDevId - 401;
				if((nGunIdx >= Connctor_1) && (nGunIdx < Max_Connctors))
				{
					if((pOCPP->stConnectorData[nGunIdx].nGunChargeStatus == GUN_STATE_NOT_INSTALL) ||
						(pOCPP->stConnectorData[nGunIdx].nGunChargeStatus == GUN_STATE_FAULTED) ||
						(pOCPP->stConnectorData[nGunIdx].nGunChargeStatus == GUN_STATE_LOCKED))
					{
						return EVENT_HANDED_FAILED;
					}

					Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
					if(emMsgType == EVENT_MSG_END)
					{
						pOCPP->stConnectorData[nGunIdx].bMeterInterrupted = FALSE;
						pOCPP->stConnectorData[nGunIdx].bNeedSendMeter_Inter_Begin = FALSE;
						pOCPP->stConnectorData[nGunIdx].bNeedSendMeter_Inter_End = TRUE;
					}
					else
					{
						pOCPP->stConnectorData[nGunIdx].bMeterInterrupted = TRUE;
						pOCPP->stConnectorData[nGunIdx].bNeedSendMeter_Inter_Begin = TRUE;
						pOCPP->stConnectorData[nGunIdx].bNeedSendMeter_Inter_End = FALSE;
					}
					//Now call the msg handler to create the msg to send
					if(pOCPP->stConnectorData[nGunIdx].emMeterTp != METER_TYPE_NotInstalled)
					{
						pOCPP->emHandlingConnector = nGunIdx;
						pOCPP->emHandlingMeterType = Sample_Clock;
						pOCPP->stMsgInfoList[MeterValues].pfnMsgHandler((void*)pOCPP);
					}

					Mutex_Unlock(pOCPP->hMutex);
				}
			}
			else if((pAlmEv->nAlarmId == 4) && (EVENT_MSG_RESTORE == emMsgType))
			{
				//restore this value in case dpr to continue alarming
				VAR_VALUE varV;
				varV.emValue = 1;
				pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
					1, 5, &varV, 4, 100);
			}
		}
		break;
	case EVENT_TYPE_ABILITY_CHANGE:
		{
			TRACEX("OCPP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_ABILITY_CHG * pEvAbChg = (EVENT_ABILITY_CHG*)pEvData;
			if (nEvDataSize != sizeof(EVENT_ABILITY_CHG))
			{
				return EVENT_HANDED_FAILED;
			}
			if(pEvAbChg->nWhichGun > Max_Connctors)
			{
				return EVENT_HANDED_FAILED;
			}

			pEvAbChg->nWhichGun -= 1; //this is from 1~3

			if(pEvAbChg->nWhichGun == Connctor_0)
			{
				TRACEX("Ability Change Event for The Connnector 0  \n ");
				A_BLOB_CAHRGEPOINT_STATUSINFO StatusNotiData;
				valGet.nGetSize = sizeof(A_BLOB_CAHRGEPOINT_STATUSINFO);
				valGet.val.blobValue = (BYTE*)&StatusNotiData;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
					1, 10, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					return EVENT_HANDED_FAILED;
				}
				if((StatusNotiData.emChargePointStatus != pOCPP->blobCPStatusInfo.emChargePointStatus) ||
					(StatusNotiData.emChargePointErrcode != pOCPP->blobCPStatusInfo.emChargePointErrcode))
				{
					Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
					//pOCPP->nCurrChargeStatusHandled_Conn = pEvChag->emGunNum; // for Zero connector 
					//pOCPP->byCuurConn_ChgType = pEvChag->emChangeType;
					memcpy(&pOCPP->blobCPStatusInfo, &StatusNotiData, sizeof(A_BLOB_CAHRGEPOINT_STATUSINFO));
					pOCPP->bCPStatusChanged = TRUE;
					//Now call the msg handler to create the msg to send
					//pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP);
					Mutex_Unlock(pOCPP->hMutex);
				}
			}
			else
			{
				TRACEX("Ability Change Event for The Connnectors [%d] \n ", pEvAbChg->nWhichGun);
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
					201 + pEvAbChg->nWhichGun, 101, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					return EVENT_HANDED_FAILED;
				}
				if(pOCPP->stConnectorData[pEvAbChg->nWhichGun].nGunChargeStatus == valGet.val.emValue)
				{
					return EVENT_HANDED_FAILED;
				}
				pOCPP->stConnectorData[pEvAbChg->nWhichGun].nGunChargeStatus = valGet.val.emValue;

				//Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
				//pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP, TRUE);
				//pOCPP->stConnectorData[pEvAbChg->nWhichGun].bConn_StatusChanged = TRUE;
				if(pOCPP->emOCPPConnStatus == Ready)
				{
					pOCPP->stConnectorData[pEvAbChg->nWhichGun].bConn_StatusChanged++;
				}
				//Mutex_Unlock(pOCPP->hMutex);
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
			TRACEX("OCPP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_EXIT, EXIT_TYPE = %d \n", pEvReboot->nExitType);
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
			TRACEX("OCPP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_PARAM_CHG * pParamChg = (EVENT_PARAM_CHG*)pEvData;
			if (nEvDataSize != sizeof(EVENT_PARAM_CHG))
			{
				return EVENT_HANDED_FAILED;
			}

			//Jimmy note: this need to be implemented whenever parameter changes 
			if((pParamChg->parmPTR.nId == 1) && ((pParamChg->parmPTR.nSubId == 18) || 
				(pParamChg->parmPTR.nSubId == 2))) //server URL change or ChargerID changed
			{
				//PARAM_VAL_GET valGet;
				//valGet.nGetSize = 255;
				//valGet.val.blobValue = (BYTE*)&pOCPP->szServerURL;
				//pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				//	1, 18, &valGet, sizeof(PARAM_VAL_GET));

				valGet.nGetSize = 255;
				char szEndUrl[255];
				valGet.val.blobValue = (BYTE*)szEndUrl; //&pOCPP->szServerURL;
				pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 18, &valGet, sizeof(PARAM_VAL_GET));

				char szChargerID[64];
				valGet.nGetSize = 64;
				valGet.val.blobValue = (BYTE*)szChargerID;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 2, &valGet, sizeof(PARAM_VAL_GET));

				snprintf(pOCPP->szServerURL, 255, "%s/%s", szEndUrl, szChargerID);
				pOCPP->bBootNotifyDone = FALSE;
				pOCPP->emOCPP_Disconn_Reason = OCPP_COMM_FAIL_REASON_URL_CHANGED;
				pOCPP->emOCPPConnStatus = Disconnected; //need to reconnect to new server again
				//Percent_EncodeUrl(pOCPP->szServerURL); //Jimmy note 20181129: maybe do not need as URL should be correct when input on LCD
			}
			else if(pParamChg->parmPTR.nSubId == 401)
			{
				valGet.nGetSize = 4;
				for(n = 0; n < Max_Connctors; n++)
				{
					if((401 + n) == pParamChg->parmPTR.nId)
					{
						Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
						pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
							401 + n, 401, &valGet, sizeof(PARAM_VAL_GET));
						pOCPP->stConnectorData[n].emMeterTp =(METER_TYPE) valGet.val.emValue;
						Mutex_Unlock(pOCPP->hMutex);
						break;
					}
				}
			}
			else if ((pParamChg->parmPTR.nSubId == 16))
			{
				// Read OCPP Config 
				TRACEX("Config Changed Evevt \n");
				PARAM_VAL_GET valGet;
				valGet.nGetSize = sizeof(A_BLOB_OCPP_CONFIGS);
				valGet.val.blobValue = (BYTE*)&pOCPP->stOCPPSettings;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 16, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("ERROR: OCPP settings can not be retrieved\n");
					return FALSE;
				}
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 55))
			{
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					pParamChg->parmPTR.nId, pParamChg->parmPTR.nSubId, &valGet, sizeof(PARAM_VAL_GET));	
				if(err != ERRCODE_DAI_OK)	
				{
					return EVENT_HANDED_FAILED;
				}	
				else
				{
					pOCPP->TimeCalibration = ((TIME_CALIBRATION)valGet.val.emValue == TIME_OCPP)? TRUE : FALSE;
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
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pReporter->pDataHeap;
	if(pReporter->hCommPort != NULL)
	{
		nErrCode = pReporter->pUsedCommPort->Close(pReporter->hCommPort);
		TRACEX("Close The Comm Port errcode:%d\n", nErrCode);
		sprintf(szDump, "Close the comm port[%d], reason[%d]", nErrCode, pOCPP->emOCPP_Disconn_Reason);
		DUMP_OCPPMSG(pReporter, DUMP_TYPE_CLOSE_COMM, szDump);
		pReporter->hCommPort = NULL;
		s_nLastOpenErrCode = -999;
		return FALSE; //will delay 5 sec and have another try
	}

	//pReporter->hCommPort = pReporter->pUsedCommPort->Open(pReporter->info.szReporterSetting,
	//	"ws://121.40.165.18:8800", COMM_CLIENT_MODE, 5000, &nErrCode); //5 sec timeout for connecting...
	pReporter->hCommPort = pReporter->pUsedCommPort->Open(pReporter->info.szReporterSetting,
		pOCPP->szServerURL, COMM_CLIENT_MODE, 10000, &nErrCode); //10 sec timeout for connecting...
	TRACE("pReporter->info.szReporterSetting =%s pOCPP->szServerURL=%s\n",pReporter->info.szReporterSetting,pOCPP->szServerURL);
	
	static char s_strLastServerURL[255] = {0};
	if(s_nLastOpenErrCode != nErrCode) //Jimmy added 20190411 for only record once if the network is not alive
	{
		s_nLastOpenErrCode = nErrCode;
		memcpy(s_strLastServerURL, pOCPP->szServerURL, 255);
		sprintf(szDump, "Open the address[%s]: [%s]", pOCPP->szServerURL, (pReporter->hCommPort == NULL) ? "Failed" : "Successfylly");
		DUMP_OCPPMSG(pReporter, DUMP_TYPE_OPEN_COMM, szDump);
	}
	else if(strcmp(s_strLastServerURL, pOCPP->szServerURL) != 0) //Jimmy added 20190411 for only record once if the network is not alive
	{
		memcpy(s_strLastServerURL, pOCPP->szServerURL, 255);
		sprintf(szDump, "Open the NEW address[%s]: [%s]", pOCPP->szServerURL, (pReporter->hCommPort == NULL) ? "Failed" : "Successfylly");
		DUMP_OCPPMSG(pReporter, DUMP_TYPE_OPEN_COMM, szDump);
	}

	if(pReporter->hCommPort == NULL)
	{
		TRACEX("The open errcode:%d\n", nErrCode);
		if(nErrCode != ERR_COMM_TIMEOUT)
		{
			//LogOut("OpenGUI Comm Port Failed-->%d");
			return FALSE;
		}
	}
	else
	{
		COMM_TIMEOUTS timeOut;
		timeOut.nReadTimeout = 500;
		timeOut.nWriteTimeout = 100;
		timeOut.nIntervalTimeout = 50;
		pReporter->pUsedCommPort->Control(pReporter->hCommPort,COMM_SET_TIMEOUTS,&timeOut, sizeof(COMM_TIMEOUTS));
	}

	return TRUE;
}

static UINT Make_UniqID_OCPP(void)
{
	//time_t tmNow = _NOW_;
	//double tmPrecNow = GetCurrentTime();
	//double diffMsec= (tmPrecNow - tmNow) * 100.0;

	//UINT nUniqID = tmNow * 100 + (UINT)diffMsec;
#define OCPP_UNIQID_START	135268
	static UINT s_nOldUniqID = 0;
	if(s_nOldUniqID == 0)
	{
		s_nOldUniqID = (UINT)_NOW_ + OCPP_UNIQID_START; //just to mix the value
	}
	//static UINT s_nOldUniqID = OCPP_UNIQID_START;
	UINT nUniqID = s_nOldUniqID;

	s_nOldUniqID++;
	if(s_nOldUniqID == 0xFFFFFFFF)
	{
		s_nOldUniqID = OCPP_UNIQID_START;
	}

	TRACEX("---The uniqID = %lu----\n", nUniqID);

	return nUniqID;
}

//static char* MakeOCPP_CallFrame(OCPP_DATA_STU* pOCPP, OCPP_MSGCALL_SRC emSrc, OCPP_MSG_TYPE emMsgType, char* szJsonData, BOOL bResent)
static char* MakeOCPP_CallFrame(OCPP_DATA_STU* pOCPP, OCPP_CALL_MSG *pMsg, BOOL bResent)
{
	TRACEX("MakeOCPPCallFrame\n");

	pOCPP->bHasActiveMsg = TRUE;
	pOCPP->activeCallMsg.tmLastSent = _NOW_;
	if(!bResent)
	{
		//ZERO_POBJS(&pOCPP->activeCallMsg.msgData, 1);
		//memcpy(pOCPP->activeCallMsg.msgData.szJsonData, szJsonData, strlen(szJsonData));
		//pOCPP->activeCallMsg.msgData.emMsgType = emMsgType;
		//pOCPP->activeCallMsg.msgData.emSrc = emSrc;
		memcpy(&pOCPP->activeCallMsg.msgData, pMsg, sizeof(OCPP_CALL_MSG));
		pOCPP->activeCallMsg.nRetryTimes = 1;
	}
	else
	{
		pOCPP->activeCallMsg.nRetryTimes++;
	}
	
	UINT nMsgUniqID = Make_UniqID_OCPP(); //(UINT)(GetCurrentTime() * 1000);

	if(pOCPP->activeCallMsg.nRetryTimes > MAX_OCPP_SEND_RETRIES)
	{
		pOCPP->activeCallMsg.nRetryTimes = MAX_OCPP_SEND_RETRIES;
		pOCPP->activeCallMsg.nUniqMsgId[MAX_OCPP_SEND_RETRIES - 1] = nMsgUniqID;
	}
	else
	{
		int nWhich = (pOCPP->activeCallMsg.nRetryTimes <= 0) ? 0 : pOCPP->activeCallMsg.nRetryTimes - 1;
		pOCPP->activeCallMsg.nUniqMsgId[nWhich] = nMsgUniqID;
	}

	//need handle TransID specially
	if((pOCPP->activeCallMsg.msgData.emMsgType == StopTransaction) ||
		((pOCPP->activeCallMsg.msgData.emMsgType == MeterValues) && (pOCPP->activeCallMsg.msgData.emSrc != CALL_SRC_Normal)))
	{
		cJSON* pRestoreJson = cJSON_Parse(pOCPP->activeCallMsg.msgData.szJsonData);
		cJSON* pTransID_Json = cJSON_GetObjectItem(pRestoreJson, "transactionId");
		BOOL bNeedReplaceTranID = FALSE;	
		if(pTransID_Json)
		{
			int nOldTrans = pTransID_Json->valueint;
			int nNewTranID = nOldTrans;
			if(pOCPP->activeCallMsg.msgData.emSrc == CALL_SRC_InProcessTrans)
			{
				int nGunIdx = pOCPP->activeCallMsg.msgData.nBelongedConn;
				if(pOCPP->stConnectorData[nGunIdx].blobInHandling_ChargeInfo.nSessionID != nOldTrans)
				{
					nNewTranID = pOCPP->stConnectorData[nGunIdx].blobInHandling_ChargeInfo.nSessionID;
					cJSON* pTransIDJSON_New = cJSON_CreateNumber(nNewTranID);
					cJSON_ReplaceItemInObject(pRestoreJson, "transactionId", pTransIDJSON_New);
					bNeedReplaceTranID = TRUE;
					TRACEX("---Gun[%d], Trans %d-->%d----\n", nGunIdx + 1, nOldTrans, nNewTranID);
				}
			}
			else if(pOCPP->activeCallMsg.msgData.emSrc == CALL_SRC_HisTrans)
			{
				if(pOCPP->recInHandling_HistoryChargeRec.nSessionID != nOldTrans)
				{
					nNewTranID = pOCPP->recInHandling_HistoryChargeRec.nSessionID;
					cJSON* pTransIDJSON_New = cJSON_CreateNumber(nNewTranID);
					cJSON_ReplaceItemInObject(pRestoreJson, "transactionId", pTransIDJSON_New);
					bNeedReplaceTranID = TRUE;
				}
			}

			if(bNeedReplaceTranID)
			{
				memset(pOCPP->activeCallMsg.msgData.szJsonData, 0, OCPP_MAX_SENTMSG_SIZE);
				char *szData = 0;
				szData = cJSON_Print(pRestoreJson);
				memcpy(pOCPP->activeCallMsg.msgData.szJsonData, szData, 
					MIN(OCPP_MAX_SENTMSG_SIZE, strlen(szData)));

				TRACEX("-----Now found a new replaced OldtransID = %d, msgData = [%s]-----\n",
					nOldTrans, szData);

				free(szData);
			}
		}
		//Jimmy added 20190308: should delete it here
		cJSON_Delete(pRestoreJson);
	}

	//sprintf(pOCPP->activeCallMsg.nUniqMsgId, "%lu", );
	snprintf(pOCPP->szSendBuffer, OCPP_MAX_SENTMSG_SIZE, "[%d,\"%lu\",\"%s\",%s]",
		Call,
		nMsgUniqID,
		pOCPP->stMsgInfoList[pMsg->emMsgType].szMsgName,
		/*szJsonData*/ pOCPP->activeCallMsg.msgData.szJsonData);

	pOCPP->nSendBuffLen = strlen(pOCPP->szSendBuffer);

	TRACEX("The sent out of WS pkg is [%s], Len = %d\n", pOCPP->szSendBuffer, pOCPP->nSendBuffLen);

	//if(emSrc == CALL_SRC_HisTrans)
	//{
	//	if(emMsgType == StartTransaction)
	//	{
	//		pOCPP->emHistoryTransMsgHandleStatus = TRANS_MSG_HAND_StartTrans;
	//	}
	//	else if(emMsgType == StopTransaction)
	//	{
	//		pOCPP->emHistoryTransMsgHandleStatus = TRANS_MSG_HAND_StopTrans;
	//	}
	//	else if(emMsgType == MeterValues)
	//	{
	//		pOCPP->emHistoryTransMsgHandleStatus = TRANS_MSG_HAND_MeterVals;
	//	}
	//}
	//else if(emSrc == CALL_SRC_InProcessTrans)
	//{
	//	int nGunIdx = pOCPP->activeCallMsg.msgData.nBelongedConn;
	//	if(emMsgType == StartTransaction)
	//	{
	//		pOCPP->stConnectorData[nGunIdx].emInProcessTransMsgHandleStatus = TRANS_MSG_HAND_StartTrans;
	//	}
	//	else if(emMsgType == StopTransaction)
	//	{
	//		pOCPP->stConnectorData[nGunIdx].emInProcessTransMsgHandleStatus = TRANS_MSG_HAND_StopTrans;
	//	}
	//	else if(emMsgType == MeterValues)
	//	{
	//		pOCPP->stConnectorData[nGunIdx].emInProcessTransMsgHandleStatus = TRANS_MSG_HAND_MeterVals;
	//	}
	//}

	return pOCPP->szSendBuffer;
}

static BOOL Wait_Ready(REPORTER* pReporter)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pReporter->pDataHeap;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	time_t tmNow = _NOW_;
	static time_t s_tmLastSentBootNotif = 0;
	int n;

	static OCPP_CONN_STATUS s_emOldConnStatus = Connected;
	static int s_nConnStatusStableCount = 0;
	if((pOCPP->emOCPPConnStatus != s_emOldConnStatus) &&
		((pOCPP->emOCPPConnStatus == Disconnected) || (pOCPP->emOCPPConnStatus == Ready)))
	{
		s_nConnStatusStableCount++;
		if(s_nConnStatusStableCount >= 3)
		{
			s_nConnStatusStableCount = 0;
			s_emOldConnStatus = pOCPP->emOCPPConnStatus;
			VAR_VALUE varV;
			varV.emValue = (s_emOldConnStatus == Disconnected) ? 1 : 0;
			pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
				1, 5, &varV, 4, 100);
		}
	}
	else
	{
		s_nConnStatusStableCount = 0;
	}

	if(pOCPP->emOCPPConnStatus == Ready)
	{
		pOCPP->emOCPP_Disconn_Reason = OCPP_COMM_FAIL_REASON_NO_FAIL;
		return TRUE;
	}

	int nWaitResent_Boot_Sec = (pOCPP->stOCPPSettings.nHearBeatInterv_RW > 0) ? 
		pOCPP->stOCPPSettings.nHearBeatInterv_RW : 3; //if it is 0, charger uses 3 as default
	nWaitResent_Boot_Sec = MIN(nWaitResent_Boot_Sec, 10); //Jimmy note 20190215:if heartbeatInterv is very large, could be very slow to try again

	Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);

	TRACEX("Now OCPP status = %d, tmDiff = %d, nWaitResent_Boot_Sec = %d \n", 
		pOCPP->emOCPPConnStatus, ABS(tmNow - s_tmLastSentBootNotif) , nWaitResent_Boot_Sec);

	if(pOCPP->emOCPPConnStatus == Disconnected)
	{
		Mutex_Unlock(pOCPP->hMutex); //Jimmy note 20190412: do not lock here in case the event_handler will be blocked
		TRACEX("ThreadEntry_OCPPComm Enter\n");
		//pOCPP->emOCPPConnStatus = Disconnected;
		TryConnect2Server(pReporter);
		TRACEX("TryConnect2Server Out \n");
		if (pReporter->hCommPort == NULL)
		{
			pOCPP->emOCPP_Disconn_Reason = OCPP_COMM_FAIL_REASON_SERVRER_BROKEN;
			//Mutex_Unlock(pOCPP->hMutex);
			Sleep(6000);
			//return FALSE;
		}
		else
		{
			pOCPP->emOCPPConnStatus = Connected;
		}

		return FALSE;
	}
	else if(pOCPP->emOCPPConnStatus == Connected)
	{
		pOCPP->bHasActiveMsg = FALSE;
		pOCPP->bAuthorizeInProcess = FALSE; //added to clear the Auth Status in case auth not resending
		if (!pOCPP->bBootNotifyDone)
		{
			pOCPP->stMsgInfoList[BootNotification].pfnMsgHandler((void*)pOCPP);
			TRACEX("BootNotification Trigger \n");
			//if write error, then we need to goto disconnected again
			if(pOCPP->emOCPPConnStatus != Disconnected)
			{
				pOCPP->emOCPPConnStatus = Wait4BootNotify_Conf;
				s_tmLastSentBootNotif = tmNow;
			}
		}
		else
		{
			//Jimmy note 20190111: do not empty 'cause some msg need to be restored after connection recovered
			//Queue_Empty(pOCPP->hqMsg2Sent); //clear any history queued msg
			//Need trigger StatusNotif.req soon after reconnect
			if(!pOCPP->bCPStatusChanged)
			{
				pOCPP->tmLastCPStatusChange = 0;
			}
			for(n = 0; n < Max_Connctors; n++)
			{
				if(!pOCPP->stConnectorData[n].bConn_StatusChanged)
				{
					pOCPP->stConnectorData[n].tmLastConn_StatusChange = 0;
				}
			}
			pOCPP->tmLastSendHeartBeat = tmNow; //do not send heartbeat on first connected
			pOCPP->emOCPPConnStatus = Ready;
		}
	}
	else if(pOCPP->emOCPPConnStatus == Wait4BootNotify_Conf) //waiting BootNotic result
	{
		pOCPP->nReadBuffLen = pComm->Read(pReporter->hCommPort, pOCPP->szReadBuffer, OCPP_BUFFER_LEN);
		TRACEX("Wait bootNotif.conf nReadLen = %d\n", pOCPP->nReadBuffLen);
		if(pOCPP->nReadBuffLen > 0)
		{
			DUMP_OCPPMSG(pReporter, DUMP_TYPE_READ, pOCPP->szReadBuffer);
			Parse_OCPPFrames(pOCPP);
		}

		if(pOCPP->bBootNotifyDone)
		{
			pOCPP->emOCPPConnStatus = Connected;
		}
		else if(ABS(tmNow - s_tmLastSentBootNotif) > nWaitResent_Boot_Sec)
		{
			pOCPP->emOCPPConnStatus = Connected;
		}
	}
	else if(pOCPP->emOCPPConnStatus == BootNotify_Pending) //now we can handling OCPP configure setting things 
	{
		if((pOCPP->nReadBuffLen = pComm->Read(pReporter->hCommPort, pOCPP->szReadBuffer, OCPP_BUFFER_LEN)) > 0)
		{
			DUMP_OCPPMSG(pReporter, DUMP_TYPE_READ, pOCPP->szReadBuffer);
			Parse_OCPPFrames(pOCPP);
		}

		if(pOCPP->bBootNotifyDone)
		{
			pOCPP->emOCPPConnStatus = Connected;
		}
		else if(ABS(tmNow - s_tmLastSentBootNotif) > nWaitResent_Boot_Sec)
		{
			pOCPP->emOCPPConnStatus = Connected;
		}
	}
	else if(pOCPP->emOCPPConnStatus == BootNotify_Rejected) //now we can handling OCPP configure setting things 
	{
		static time_t s_tmLastRejected = 0;
		if(s_tmLastRejected == 0)
		{
			s_tmLastRejected = tmNow;
		}

		if(pOCPP->bBootNotifyDone)
		{
			pOCPP->emOCPPConnStatus = Connected;
		}
		else if(ABS(tmNow - s_tmLastSentBootNotif) > nWaitResent_Boot_Sec)
		{
			pOCPP->emOCPPConnStatus = Connected;
		}
		else if(ABS(tmNow - s_tmLastRejected) > 60)
		{
			LogOut(MAIN_MODULE_REPORTER, "OCPP", LOG_TYPE_ERROR,
				"BootNotif was refused by server, now disconnect and try again...");
			s_tmLastRejected = tmNow;
			pOCPP->emOCPP_Disconn_Reason = OCPP_COMM_FAIL_REASON_REFUSED;
			pOCPP->emOCPPConnStatus = Disconnected;
		}
	}

	Mutex_Unlock(pOCPP->hMutex);

	return (pOCPP->emOCPPConnStatus == Ready) ? TRUE : FALSE;
}

static void Handle_Call_Resend(OCPP_DATA_STU* pOCPP, COMMPORT* pComm)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	if(pOCPP->activeCallMsg.nRetryTimes >= MAX_OCPP_SEND_RETRIES)
	{
		//Jimmy note: according to OCPP doc, if did not get response for CS, charger should keep re-trying until response arrived.
		if((pOCPP->activeCallMsg.msgData.emMsgType == StartTransaction) || 
			(pOCPP->activeCallMsg.msgData.emMsgType == StopTransaction) ||
			((pOCPP->activeCallMsg.msgData.emMsgType == MeterValues) && (pOCPP->activeCallMsg.msgData.emSrc != CALL_SRC_Normal)))
		{
			//should put the messge back into the queue and continue trying
			if(pOCPP->activeCallMsg.msgData.emSrc == CALL_SRC_HisTrans)
			{
				Queue_Put(pOCPP->hqHistoryTrans, &pOCPP->activeCallMsg.msgData, TRUE);
			}
			else if(pOCPP->activeCallMsg.msgData.emSrc == CALL_SRC_InProcessTrans)
			{
				int nGunIdx = pOCPP->activeCallMsg.msgData.nBelongedConn;
				Queue_Put(pOCPP->stConnectorData[nGunIdx].hqInProcessTrans, &pOCPP->activeCallMsg.msgData, TRUE);
			}
		}

		TRACEX("----Resend Msg[%s] TIMEOUT!!!-----\n", pOCPP->stMsgInfoList[pOCPP->activeCallMsg.msgData.emMsgType].szMsgName);
		pOCPP->bHasActiveMsg = FALSE;
		//pOCPP->nConnErrTimes++;
		pOCPP->nConnErrTimes = MAX_OCPP_SEND_RETRIES; //Jimmy note: found that even disconnected, write can be still done, so here once resend expires 3 times, we disconnect.
	}
	else
	{
		TRACEX("----Resend Msg[%s] RetryTimes [%d], jSon [%s]-----\n", pOCPP->stMsgInfoList[pOCPP->activeCallMsg.msgData.emMsgType].szMsgName, 
			pOCPP->activeCallMsg.nRetryTimes, pOCPP->activeCallMsg.msgData.szJsonData);

		OCPP_CALL_MSG* pMsg = &pOCPP->activeCallMsg.msgData;
		if (pComm->Write(pReporter->hCommPort,
			MakeOCPP_CallFrame(pOCPP, pMsg, TRUE),
			pOCPP->nSendBuffLen))
		{
			DUMP_OCPPMSG(pReporter, DUMP_TYPE_WRITE, pOCPP->szSendBuffer);
			//sent successfully
			//it is possible that SERVER just does not reply but still can send data
			//pOCPP->nConnErrTimes = 0;
		}
		else
		{
			pOCPP->nConnErrTimes++;
		}
	}

	if (pOCPP->nConnErrTimes >= MAX_OCPP_SEND_RETRIES)
	{
		pOCPP->emOCPP_Disconn_Reason = OCPP_COMM_FAIL_REASON_TIMEOUT_RECEIVE;
		pOCPP->nConnErrTimes = 0;
		pOCPP->emOCPPConnStatus = Disconnected;
		TRACEX("emOCPPConnStatus : Disconnected \n");
	}
}

static HANDLE Dispatch_Q(OCPP_DATA_STU* pOCPP)
{
	if(pOCPP->nQUrgentMsgCount > 0)
	{
		pOCPP->nQUrgentMsgCount--;
		TRACEX("-------Now has urgent msg to send out[%d]-------\n", pOCPP->nQUrgentMsgCount);
		return pOCPP->hqMsg2Sent;
	}

	static HANDLE s_LastHanlded_InProcQ = NULL;
	HANDLE hqList[Max_Connctors + 1];
	int n;
	//Strategy: get queue from hqSent/hqHis/hqInProc in-turn
	if(s_LastHanlded_InProcQ == pOCPP->hqMsg2Sent)
	{
		hqList[0] = pOCPP->stConnectorData[Connctor_1].hqInProcessTrans;
		hqList[1] = pOCPP->stConnectorData[Connctor_2].hqInProcessTrans;
		hqList[2] = pOCPP->stConnectorData[Connctor_3].hqInProcessTrans;
		hqList[3] = pOCPP->hqMsg2Sent;
	}
	else if(s_LastHanlded_InProcQ == pOCPP->stConnectorData[Connctor_1].hqInProcessTrans)
	{
		hqList[0] = pOCPP->stConnectorData[Connctor_2].hqInProcessTrans;
		hqList[1] = pOCPP->stConnectorData[Connctor_3].hqInProcessTrans;
		hqList[2] = pOCPP->hqMsg2Sent;
		hqList[3] = pOCPP->stConnectorData[Connctor_1].hqInProcessTrans;
	}
	else if(s_LastHanlded_InProcQ == pOCPP->stConnectorData[Connctor_2].hqInProcessTrans)
	{
		hqList[0] = pOCPP->stConnectorData[Connctor_3].hqInProcessTrans;
		hqList[1] = pOCPP->hqMsg2Sent;
		hqList[2] = pOCPP->stConnectorData[Connctor_1].hqInProcessTrans;
		hqList[3] = pOCPP->stConnectorData[Connctor_2].hqInProcessTrans;
	}
	else // if(s_LastHanlded_InProcQ == NULL) || connetctor3
	{
		hqList[0] = pOCPP->hqMsg2Sent;
		hqList[1] = pOCPP->stConnectorData[Connctor_1].hqInProcessTrans;
		hqList[2] = pOCPP->stConnectorData[Connctor_2].hqInProcessTrans;
		hqList[3] = pOCPP->stConnectorData[Connctor_3].hqInProcessTrans;
	}

	HANDLE hq = NULL;
	for(n = 0; n < ITEM_OF(hqList); n++)
	{
		if(Queue_GetCount(hqList[n], NULL) > 0)
		{
			hq = hqList[n];
			s_LastHanlded_InProcQ = hq;
			break;
		}
	}

	if((hq == NULL) && (Queue_GetCount(pOCPP->hqHistoryTrans, NULL) > 0))
	{
		hq = pOCPP->hqHistoryTrans;
	}

	//TRACEX("------The dispatched Q is %d-----\n", hq);
	return hq;
}

static void Handle_Queued_Call(OCPP_DATA_STU* pOCPP, COMMPORT* pComm)
{
	OCPP_CALL_MSG stSentMsg;
	OCPP_CALL_MSG* pMsg = &stSentMsg;
	static HANDLE s_LastHanlded_InProcQ = NULL;
	//Strategy: get queue from hqSent/hqHis/hqInProc in-turn
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	time_t tmNow = _NOW_;

	HANDLE hq = Dispatch_Q(pOCPP);
	if(hq == NULL)
	{
		return;
	}

	if(Mutex_Lock(pOCPP->hMutex, 100) == ERR_MUTEX_OK)
	{
		if(Queue_Get(hq, &stSentMsg, FALSE, 100) == ERR_QUEUE_OK)
		{
			//Jimmy note: if trans-related msg, need to check attempts and delay
			//if((stSentMsg.emMsgType == StartTransaction) || 
			//	(stSentMsg.emMsgType == StopTransaction) ||
			//	((stSentMsg.emMsgType == MeterValues) && (stSentMsg.emSrc != CALL_SRC_Normal)))
			//{
			//	if(tmNow - stSentMsg.tmLastErrorRespArrived < pOCPP->stOCPPSettings.nTransactionMessageAttempts_RW)
			//	{
			//		//should put the message back into the queue and continue trying
			//		Queue_Put(hq, &stSentMsg, TRUE);
			//		Mutex_Unlock(pOCPP->hMutex);
			//		return;
			//	}
			//}

			if (pComm->Write(pReporter->hCommPort,
				MakeOCPP_CallFrame(pOCPP, pMsg, FALSE),
				pOCPP->nSendBuffLen))
			{
				DUMP_OCPPMSG(pReporter, DUMP_TYPE_WRITE, pOCPP->szSendBuffer);
				//sent succsessfully
				//pOCPP->nConnErrTimes = 0;
			}
			else
			{
				pOCPP->nConnErrTimes++;
				if (pOCPP->nConnErrTimes >= 3)
				{
					pOCPP->emOCPP_Disconn_Reason = OCPP_COMM_FAIL_REASON_TIMEOUT_RECEIVE;
					pOCPP->nConnErrTimes = 0;
					pOCPP->emOCPPConnStatus = Disconnected;
					TRACEX("emOCPPConnStatus : Disconnected \n");
				}
			}
			
		}
		Mutex_Unlock(pOCPP->hMutex);
	}
}

static BOOL PushHistoricalTrans_2Q(OCPP_DATA_STU* pOCPP)
{
	int n;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	QUERYCOND_CHARGERECORD recCond;
	QUERY_COND cond;
	recCond.nReportFlag = 0;
	cond.nArgSize = sizeof(QUERYCOND_CHARGERECORD);
	cond.pCondArg = &recCond;
	cond.nQueryCondType = QUERY_COND_REPORTED_FLAG;
	UINT nQueryCount = 1;
	ERRCODE_DAI errCode = pReporter->pDAI->Query((HANDLE)pReporter, QUERYTYPE_CHARGEREC, 
		cond, (void*)&pOCPP->recInHandling_HistoryChargeRec, &nQueryCount);
	if((errCode != ERRCODE_DAI_OK) || (nQueryCount < 1))
	{
		nQueryCount = 1;
		recCond.nReportFlag = REPORT_FLAG_OCPP_BEGIN;
		errCode = pReporter->pDAI->Query((HANDLE)pReporter, QUERYTYPE_CHARGEREC, 
			cond, (void*)&pOCPP->recInHandling_HistoryChargeRec, &nQueryCount);
	}

	TRACEX("-------Query history charger record fo OCPP, nQueryCount = %d----------\n", nQueryCount);
	if((errCode == ERRCODE_DAI_OK) && (nQueryCount > 0))
	{
		Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
		pOCPP->bTransMsgFromHistory = FALSE;
		pOCPP->emHandlingConnector = pOCPP->recInHandling_HistoryChargeRec.emGunNum - 1;
		if((pOCPP->emHandlingConnector < 0) ||
			(pOCPP->emHandlingConnector > Connctor_3))
		{
			//Jimmy: there is error, in future need think about how to handle this error, because if just skip it this time, another time will get same record!
			Mutex_Unlock(pOCPP->hMutex);
			return FALSE;
		}
		//Jimmy 20190319: just ignore the in-process session in case sending repeatedly
		int nConnIdx = pOCPP->emHandlingConnector;
		if((pOCPP->stConnectorData[nConnIdx].blobInHandling_ChargeInfo.tmChargeStart == 
			pOCPP->recInHandling_HistoryChargeRec.tmChargeStart) && 
			((Queue_GetCount(pOCPP->stConnectorData[nConnIdx].hqInProcessTrans, NULL) > 0) ||
			(pOCPP->stConnectorData[nConnIdx].nGunChargeStatus >= GUN_STATE_WAITING_START)))
		{
			Mutex_Unlock(pOCPP->hMutex);
			return FALSE;
		}
		
		if(pOCPP->recInHandling_HistoryChargeRec.nReportFlag == 0) //if has reported, should skip it
		{
			//pOCPP->emHistoryTransMsgHandleStatus = TRANS_MSG_HAND_WaitHandle;
			pOCPP->bTransMsgFromHistory = TRUE;
			pOCPP->stMsgInfoList[StartTransaction].pfnMsgHandler((void*)pOCPP);			
		}

		//Now we query the related meter values
		QUERYCOND_HISMETERVAL recMeter;
		recMeter.nGunIdx = pOCPP->recInHandling_HistoryChargeRec.emGunNum;
		recMeter.tmChargeStart = pOCPP->recInHandling_HistoryChargeRec.tmChargeStart;
		cond.nArgSize = sizeof(QUERYCOND_HISMETERVAL);
		cond.pCondArg = &recMeter;
		nQueryCount = 1;
		HIS_METER_VALUE_REC recHandling_HistoryMeterVals;
		ZERO_POBJS(&recHandling_HistoryMeterVals, 1);
		errCode = pReporter->pDAI->Query((HANDLE)pReporter, QUERYTYPE_METERVALUS, 
			cond, (void*)&recHandling_HistoryMeterVals, &nQueryCount);
		TRACEX("-------Query history meter record fo OCPP, errCode = %d, nQueryCount = %d----------\n", errCode, nQueryCount);
		//if (errCode != ERRCODE_DAI_OK)
		if(errCode == ERRCODE_DAI_OK)
		{
			if((recHandling_HistoryMeterVals.meterData.stMeterVal_Start.byValidElements > 0) && 
				(!recHandling_HistoryMeterVals.meterData.stMeterVal_Start.bIsReportedOK))
			{
				pOCPP->emHandlingMeterType = Transaction_Begin;
				memcpy(&pOCPP->currHandlingHisMeterVal, &recHandling_HistoryMeterVals.meterData.stMeterVal_Start,
					sizeof(A_CHARGE_METERVALUE_ITEM));
				pOCPP->bTransMsgFromHistory = TRUE;
				pOCPP->stMsgInfoList[MeterValues].pfnMsgHandler((void*)pOCPP);				
			}

			TRACEX("-------Query metervalues total = %d----------\n", recHandling_HistoryMeterVals.meterData.nMeterValTotal);
			int nMeterTotal = MIN(recHandling_HistoryMeterVals.meterData.nMeterValTotal, MAX_HIS_STORE_METER_VALUES);
			for(n = 0; n < nMeterTotal; n++)
			{
				if((recHandling_HistoryMeterVals.meterData.stMeterValList[n].byValidElements > 0) && 
					(!recHandling_HistoryMeterVals.meterData.stMeterValList[n].bIsReportedOK))
				{
					pOCPP->emHandlingMeterType = Sample_Periodic;
					memcpy(&pOCPP->currHandlingHisMeterVal, &recHandling_HistoryMeterVals.meterData.stMeterValList[n],
						sizeof(A_CHARGE_METERVALUE_ITEM));
					pOCPP->bTransMsgFromHistory = TRUE;
					pOCPP->stMsgInfoList[MeterValues].pfnMsgHandler((void*)pOCPP);					
				}
			}

			if((recHandling_HistoryMeterVals.meterData.stMeterVal_Stop.byValidElements > 0) && 
				(!recHandling_HistoryMeterVals.meterData.stMeterVal_Stop.bIsReportedOK))
			{
				pOCPP->emHandlingMeterType = Transaction_End;
				memcpy(&pOCPP->currHandlingHisMeterVal, &recHandling_HistoryMeterVals.meterData.stMeterVal_Stop,
					sizeof(A_CHARGE_METERVALUE_ITEM));
				pOCPP->bTransMsgFromHistory = TRUE;
				pOCPP->stMsgInfoList[MeterValues].pfnMsgHandler((void*)pOCPP);				
			}
		}

		//Jimmy added 20190702: if not end, do not handle here
		if((pOCPP->recInHandling_HistoryChargeRec.emChargeStatus == CONNN_CHARGE_END) &&
		(pOCPP->recInHandling_HistoryChargeRec.nReportFlag != REPORT_FLAG_OCPP_END)) //if has reported, should skip it
		{
			//pOCPP->emHistoryTransMsgHandleStatus = TRANS_MSG_HAND_WaitHandle;
			pOCPP->bTransMsgFromHistory = TRUE;
			pOCPP->stMsgInfoList[StopTransaction].pfnMsgHandler((void*)pOCPP);	
		}

		Mutex_Unlock(pOCPP->hMutex);

		return pOCPP->bTransMsgFromHistory;
	}

	return FALSE;
}

static DWORD _ThreadEntry_OCPPComm(void* pArg)
{
	REPORTER* pReporter = (REPORTER*)pArg;
	HANDLE hPort = pReporter->hCommPort;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	TRACEX("_ThreadEntry_OCPPComm: Start Enter \n");
	int nReadBytes = 0;
	int n;
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pReporter->pDataHeap;
	int nWaitRebootMsgCount = 0;
	time_t tmLastQueryHisChgRec = 0;
	
	while (!pReporter->bExit)
	{
		RUN_THREAD_HEARTBEAT();
		time_t tmNow = _NOW_;

		//Jimmy note20190112: there was misunderstanding, no need to skip BootNotif on SoftReset
		//wait 5 sec until receive Reboot msg, so as to decide whether need send BootNotification.req
		//if(!pOCPP->bWaitRebootMsg_Ready) 
		//{
		//	if(nWaitRebootMsgCount >= 50)
		//	{
		//		pOCPP->bWaitRebootMsg_Ready = TRUE;
		//	}
		//	else
		//	{
		//		nWaitRebootMsgCount++;
		//		Sleep(100);
		//		continue;
		//	}
		//}

		//static BOOL s_bReceiveReb = FALSE;
		//if(!s_bReceiveReb)
		//{
		//	s_bReceiveReb = TRUE;
		//	TRACEX("---Wait RebootMsg TotalTime->%d, and BootNotifDone = %d---\n",
		//		nWaitRebootMsgCount * 100, pOCPP->bBootNotifyDone);
		//}
		
		if(!Wait_Ready(pReporter))
		{
			Sleep(100);
			continue;
		}

		//Now ready
		if(pOCPP->bHasActiveMsg)
		{
			if(ABS(tmNow - pOCPP->activeCallMsg.tmLastSent) > pOCPP->nMsgCallTimeOut_Sec)
			{
				//Resend_Msg
				Handle_Call_Resend(pOCPP, pComm);
			}
		}
		else
		{
			Handle_Queued_Call(pOCPP, pComm);
		}

		//added by Jimmy 20190702 for OCTT test. if disconn, should skip going ahead
		if(pOCPP->emOCPPConnStatus == Disconnected)
		{
			continue;
		}
		else if(pOCPP->bNoticedExitOCPP)
		{
			if(pOCPP->nQUrgentMsgCount < 1)
			{
				TRACEX("-----Now ready for OCPP exiting-----\n");
				pOCPP->bNoticedExitOCPP = FALSE;
				pOCPP->emOCPP_Disconn_Reason = OCPP_COMM_FAIL_SYSTEM_EXIT;
				pOCPP->emOCPPConnStatus = Disconnected; //need handle urgent msg before exit
				continue;
			}
		}
		
		//Now read and parse
		pOCPP->emTriggerMsgType = Trigger_NoTriggle;
		pOCPP->nReadBuffLen = pComm->Read(pReporter->hCommPort, pOCPP->szReadBuffer, OCPP_BUFFER_LEN);
		if(pOCPP->nReadBuffLen> 0)
		{
			DUMP_OCPPMSG(pReporter, DUMP_TYPE_READ, pOCPP->szReadBuffer);
			Parse_OCPPFrames(pOCPP);
		}
		else if(pOCPP->nReadBuffLen == -1) //means server close session
		{
			TRACEX("Server Connection Error Close the Socket\n");
			pOCPP->nReadBuffLen = 0;      // need to make Zero Otherwise same Case Excuting Continue
			pOCPP->emOCPP_Disconn_Reason = OCPP_COMM_FAIL_REASON_SERVRER_BROKEN;
			pOCPP->emOCPPConnStatus = Disconnected;
			Sleep(100);
			continue;
		}

		//if(pOCPP->bHasActiveMsg)
		//{
		//	//while in waiting avtiveMsg, we do not handle other msg, or else there will be repeated msg in queue!
		//	Sleep(50);
		//	continue;
		//}
		
		//Now check status
		if(!pOCPP->bHasActiveMsg) //added by Jimmy 20210417 to avoid repeated StatusNotif.req when network not stable
		{
			for(n = Max_Connctors - 1; n >= Connctor_1; n--)
			{
				if(pOCPP->stConnectorData[n].nGunChargeStatus == GUN_STATE_NOT_INSTALL)
				{
					continue;
				}
				
				if(pOCPP->stConnectorData[n].bConn_StatusChanged)
				{
					TRACEX("Now gun[%d] status chg[%d, minDura = %d, tmNow[%d]\n", n+ 1,
						pOCPP->stConnectorData[n].bConn_StatusChanged, pOCPP->stOCPPSettings.nMinStatusDuration_RW, 
						tmNow, pOCPP->stConnectorData[n].tmLastConn_StatusChange);
				}

			if((pOCPP->stConnectorData[n].bConn_StatusChanged && 
				(ABS(tmNow - pOCPP->stConnectorData[n].tmLastConn_StatusChange) >= MAX(2, pOCPP->stOCPPSettings.nMinStatusDuration_RW))) || 
				(pOCPP->stConnectorData[n].tmLastConn_StatusChange == 0))
			{
				if(pOCPP->stConnectorData[n].tmLastConn_StatusChange == 0)
				{
					pOCPP->emTriggerMsgType = Trigger_StatusNotification; //use triggle so that first time could refresh status and send it urgent
				}
				pOCPP->emHandlingConnector = n;
				pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP);
				pOCPP->stConnectorData[n].tmLastConn_StatusChange = tmNow;
				if(pOCPP->stConnectorData[n].bConn_StatusChanged > 0)
				{
					pOCPP->stConnectorData[n].bConn_StatusChanged--;
				}
			}
		}
		if((pOCPP->bCPStatusChanged && (ABS(tmNow - pOCPP->tmLastCPStatusChange) >= MAX(2, pOCPP->stOCPPSettings.nMinStatusDuration_RW))) ||
			(pOCPP->tmLastCPStatusChange == 0))
		{
			if(pOCPP->tmLastCPStatusChange == 0)
			{
				pOCPP->emTriggerMsgType = Trigger_StatusNotification; //use triggle so that first time could refresh status and send it urgent
			}
			pOCPP->emHandlingConnector = Connctor_0;
			pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP);
			pOCPP->tmLastCPStatusChange = tmNow;
			if(pOCPP->bCPStatusChanged)
			{
				pOCPP->bCPStatusChanged = FALSE;
			}
			//TRACE("\n-----tmNow - pOCPP->tmLastCPStatusChange = %d, pOCPP->stOCPPSettings.nMinStatusDuration_RW = %d, pOCPP->bCPStatusChanged = %d----\n",
			//	tmNow - pOCPP->tmLastCPStatusChange, pOCPP->stOCPPSettings.nMinStatusDuration_RW, pOCPP->bCPStatusChanged);
		}

		//now check heartbeat
		if(!pOCPP->bHasActiveMsg)
		{
			if (((ABS(tmNow - pOCPP->tmLastSuccessExchange) >= pOCPP->stOCPPSettings.nHearBeatInterv_RW) &&
				(ABS(tmNow - pOCPP->tmLastSendHeartBeat) >= pOCPP->stOCPPSettings.nHearBeatInterv_RW)) || 
				(ABS(tmNow - pOCPP->tmLastSendHeartBeat) >= SEC_PER_DAY)) //OCPP dos recommend to send HEARTBEAT per 24-hour for time-sync
			{
				pOCPP->tmLastSendHeartBeat = tmNow;
				//TRACEX("UYB pOCPP->tmLastSuccessExchange %d pOCPP->stOCPPSettings.nHearBeatInterv_RW = %d \n", pOCPP->tmLastSuccessExchange, pOCPP->stOCPPSettings.nHearBeatInterv_RW);
				pOCPP->stMsgInfoList[HeartBeat].pfnMsgHandler((void*)pOCPP);
			}
			else if((pOCPP->stOCPPSettings.nWebSocketPingInterv_RW > 0) && 
				(ABS(tmNow - pOCPP->tmLastRecivedPongFromServer) >= pOCPP->stOCPPSettings.nWebSocketPingInterv_RW) &&
				(ABS(tmNow - pOCPP->tmLastSuccessExchange) >= pOCPP->stOCPPSettings.nWebSocketPingInterv_RW))
			{
				//now try to get pong from server
				pReporter->pUsedCommPort->Control(pReporter->hCommPort, COMM_GET_LASTTIME_RECEIVE_PONG, (void*)&pOCPP->tmLastRecivedPongFromServer, sizeof(timer_t));

				//need sent ws ping, if no pong then re-send it every 5 seconds
				if(ABS(tmNow - pOCPP->tmLastSendPingToServer) >= 5)
				{
					TRACEX("-----Now sending out a ws ping to ws server-----\n");
					pReporter->pUsedCommPort->Control(pReporter->hCommPort, COMM_SEND_WS_PING, NULL, 0);
					pOCPP->tmLastSendPingToServer = tmNow;

					if(ABS(tmNow - pOCPP->tmLastRecivedPongFromServer) > pOCPP->stOCPPSettings.nWebSocketPingInterv_RW)
					{
						TRACEX("no pong received[%d] \n", pOCPP->nConnErrTimes);
						pOCPP->nConnErrTimes++;
						if(pOCPP->nConnErrTimes >=3)
						{
							pOCPP->emOCPP_Disconn_Reason = OCPP_COMM_FAIL_REASON_NO_PONG;
							pOCPP->nConnErrTimes = 0;
							pOCPP->emOCPPConnStatus = Disconnected;
							TRACEX("no pong received, so disconnected \n");
							continue;
						}
					}
				}
			}
		}
		
		//Now check meter clock_aligned time
		if (pOCPP->stOCPPSettings.nClockAlignDataInterv_RW > 0) //0 means no need sending
		{
			if(ABS(tmNow - pOCPP->tmLastSuccessSent_ClockAlignMeterval) > pOCPP->stOCPPSettings.nClockAlignDataInterv_RW)
			{
				//TRACEX("DETECTED TIME TO SEND CLOCKALAIGNED METERVALS!!\n");
				pOCPP->tmLastSuccessSent_ClockAlignMeterval = tmNow;
				//Jimmy 20190218:�ݲ�֧���ܱ������������ϵͳ��Ҫ֧��ʱ������/**/
				//if(pOCPP->emMainPowerMeterTp != METER_TYPE_NotInstalled)
				//{
				//	TRACEX("Make clockalMeterVal for conn0,.type = %d\n", pOCPP->emMainPowerMeterTp);
				//	pOCPP->emHandlingConnector = Connctor_0;
				//	pOCPP->emHandlingMeterType = Sample_Clock;
				//	pOCPP->stMsgInfoList[MeterValues].pfnMsgHandler((void*)pOCPP);
				//}
				for (n = 0; n < Max_Connctors; n++)
				{
					if((pOCPP->stConnectorData[n].nGunChargeStatus == GUN_STATE_NOT_INSTALL) ||
						(pOCPP->stConnectorData[n].nGunChargeStatus == GUN_STATE_FAULTED) ||
						(pOCPP->stConnectorData[n].nGunChargeStatus == GUN_STATE_LOCKED))
					{
						continue;
					}
					else if((pOCPP->stConnectorData[n].emMeterTp == METER_TYPE_NotInstalled) ||
						pOCPP->stConnectorData[n].bMeterInterrupted)
					{
						continue;
					}
					//TRACEX("Make clockalMeterVal for conn%d,.type = %d\n", n+1, pOCPP->emMainPowerMeterTp);
					pOCPP->emHandlingConnector = n;
					pOCPP->emHandlingMeterType = Sample_Clock;
					pOCPP->stMsgInfoList[MeterValues].pfnMsgHandler((void*)pOCPP);
				}
			}
		}

	/*	
	 this part is require only if do not get EVSIDEDISCONNECT From Main Api for this bStopTransOnEvSideDisconn_RW Behaviour 
	if(tmNow - pOCPP->tmLastCheckedGunstatus > 10)
		{
			//Later We Discussed this 
			//Continue Checking Gun status and Stop Charging when tx is on going need to Stop Charging
			PARAM_VAL_GET valGet;
			valGet.nGetSize = 4;
			for (n = 0; n < Max_Connctors; n++)
			{
				ERRCODE_DAI err;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
					201 + n, 102, &valGet, sizeof(PARAM_VAL_GET));

				if (err == ERRCODE_DAI_OK)
				{
					pOCPP->stConnectorData[n].nGunPlugStatus = valGet.val.emValue;

					if (pOCPP->stOCPPSettings.bStopTransOnEvSideDisconn_RW == TRUE)
					{
						if ((pOCPP->activeCallMsg.msgData.emSrc == CALL_SRC_InProcessTrans) &&
							(pOCPP->stConnectorData[n].blobInHandling_ChargeInfo.tmChargeStart == pOCPP->activeCallMsg.msgData.tmRelatedTransStart))
						{
							A_BLOB_CHARGE_STOP_CMD  RemoteStop;
							RemoteStop.emBlobType = BLOB_TYPE_CHARGE_STOP_CMD;
							RemoteStop.nStopBy = CHARGE_STOP_BY_SERVER;
							RemoteStop.nStopReason = CHARGE_STOP_REASON_EVSIDE_DISCONNECT;

							VAR_VALUE varV;
							varV.blobValue = &RemoteStop;
							pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
								201 + n, 102, &varV, sizeof(RemoteStop), 0);
						}

					}
					else if (pOCPP->stOCPPSettings.bStopTransOnEvSideDisconn_RW == FALSE)
					{
						// doing nothing 
					}
				}
				else
				{
					continue;
				}
			}
			pOCPP->tmLastCheckedGunstatus = _NOW_;
		}*/

		//Now check historical msg queue
		static int nQueryInterv = 30;  //do query every 30 sec
		if((!pOCPP->bHasActiveMsg) && (ABS(tmNow - tmLastQueryHisChgRec) >= nQueryInterv))
		{
			TRACEX("Now we handle histor record, nQuerenInterv = %d\n", nQueryInterv);
			//BOOL bAllConnIdle = TRUE;
			//for(n = 0; n < Max_Connctors; n++)
			//{
			//	if(pOCPP->stConnectorData[n].emConnctorStauts == Conn_InCharging)
			//	{
			//		bAllConnIdle = FALSE;
			//		break;
			//	}
			//}
			//if(pOCPP->emHistoryTransMsgHandleStatus == TRANS_MSG_HAND_Idle)
			if(/*bAllConnIdle && */(Queue_GetCount(pOCPP->hqHistoryTrans, NULL) == 0))
			{
				tmLastQueryHisChgRec = tmNow;
				if(PushHistoricalTrans_2Q(pOCPP)) //means we have more data to handle
				{
					nQueryInterv = 5;
				}
				else
				{
					nQueryInterv = 30;
				}
			}
		}

		Sleep(100);
	}

	TRACEX("_ThreadEntry_OCPPComm: Exit\n");
	return THREAD_CANCEL_ALL;
}

#define INIT_OCPP_MSG(_pMsg,_emMsgType, _szMsgName, _emMsgSrc, _pfnMsgHandler) \
	do{\
	(_pMsg)->emMsgType = (_emMsgType); \
	memcpy((_pMsg)->szMsgName,(_szMsgName),sizeof((_szMsgName)));\
	(_pMsg)->emMsgSrc = (_emMsgSrc); \
	(_pMsg)->pfnMsgHandler = (_pfnMsgHandler); \
	(_pMsg)++; \
	}while(0)


void InitOCPPMsgInfo(OCPP_DATA_STU* pOCPP)
{
	OCPP_MSG_INFO* p = &(pOCPP->stMsgInfoList[0]);
	//!!!!!!!!!Notice Macro MUST be in the right order because we use it for INDEXING!!!!!!!!!/**/
	INIT_OCPP_MSG(p,	Authorize,						"Authorize",						OCPP_MSG_CP,		Prepare_Authorize);
	INIT_OCPP_MSG(p,	BootNotification,				"BootNotification",					OCPP_MSG_CP,		Prepare_BootNotification);
	INIT_OCPP_MSG(p,	ChangeAvailability,				"ChangeAvailability",				OCPP_MSG_CS,		Handle_ChangeAvailability);
	INIT_OCPP_MSG(p,	ChangeConfiguration,			"ChangeConfiguration",				OCPP_MSG_CS,		Handle_ChangeConfiguration);
	INIT_OCPP_MSG(p,	ClearCache,						"ClearCache",						OCPP_MSG_CS,		Handle_ClearCache);
	INIT_OCPP_MSG(p,	DataTransfer_CS,				"DataTransfer",						OCPP_MSG_CS,		Handle_DataTransfer_CS);
	INIT_OCPP_MSG(p,	DataTransfer_CP,				"DataTransfer",						OCPP_MSG_CP,		Prepare_DataTransfer_CP);
	INIT_OCPP_MSG(p,	GetConfiguration,				"GetConfiguration",					OCPP_MSG_CS,		Handle_GetConfiguration);
	INIT_OCPP_MSG(p,	HeartBeat,						"Heartbeat",						OCPP_MSG_CP,		Prepare_HeartBeat);
	INIT_OCPP_MSG(p,	MeterValues,					"MeterValues",						OCPP_MSG_CP,		Prepare_MeterValues);
	INIT_OCPP_MSG(p,	RemoteStartTransaction,			"RemoteStartTransaction",			OCPP_MSG_CS,		Handle_RemoteStartTransaction);
	INIT_OCPP_MSG(p,	RemoteStopTransaction,			"RemoteStopTransaction",			OCPP_MSG_CS,		Handle_RemoteStopTransaction);
	INIT_OCPP_MSG(p,	Reset,							"Reset",							OCPP_MSG_CS,		Handle_Reset);
	INIT_OCPP_MSG(p,	StartTransaction,				"StartTransaction",					OCPP_MSG_CP,		Prepare_StartTransaction);
	INIT_OCPP_MSG(p,	StatusNotification,				"StatusNotification",				OCPP_MSG_CP,		Prepare_StatusNotification);
	INIT_OCPP_MSG(p,	StopTransaction,				"StopTransaction",					OCPP_MSG_CP,		Prepare_StopTransaction);
	INIT_OCPP_MSG(p,	UnlockConnector,				"UnlockConnector",					OCPP_MSG_CS,		Handle_UnlockConnector);
	INIT_OCPP_MSG(p,	GetDiagnostics,					"GetDiagnostics",					OCPP_MSG_CS,		Handle_GetDiagnostics);
	INIT_OCPP_MSG(p,	DiagnosticsStatusNotification,	"DiagnosticsStatusNotification",	OCPP_MSG_CP,		Prepare_DiagnosticsStatusNotification);
	INIT_OCPP_MSG(p,	FirmwareStatusNotification,		"FirmwareStatusNotification",		OCPP_MSG_CP,		Prepare_FirmwareStatusNotification);
	INIT_OCPP_MSG(p,	UpdateFirmware,					"UpdateFirmware",					OCPP_MSG_CS,		Handle_UpdateFirmware);
	INIT_OCPP_MSG(p,	GetLocalListVersion,			"GetLocalListVersion",				OCPP_MSG_CS,		Handle_GetLocalListVersion);
	INIT_OCPP_MSG(p,	SendLocalList,					"SendLocalList",					OCPP_MSG_CS,		Handle_SendLocalList);
	INIT_OCPP_MSG(p,	CancelReservation,				"CancelReservation",				OCPP_MSG_CS,		Handle_CancelReservation);
	INIT_OCPP_MSG(p,	ReserveNow,						"ReserveNow",						OCPP_MSG_CS,		Handle_ReserveNow);
	INIT_OCPP_MSG(p,	ClearChargingProfile,			"ClearChargingProfile",				OCPP_MSG_CS,		Handle_ClearChargingProfile);
	INIT_OCPP_MSG(p,	GetCompositeSchedule,			"GetCompositeSchedule",				OCPP_MSG_CS,		Handle_GetCompositeSchedule);
	INIT_OCPP_MSG(p,	SetChargingProfile,				"SetChargingProfile",				OCPP_MSG_CS,		Handle_SetChargingProfile);
	INIT_OCPP_MSG(p,	TriggerMessage,					"TriggerMessage",					OCPP_MSG_CS,		Handle_TriggerMessage);
}

typedef struct tagEvHandTags
{
	int nEvTag;
	int nMsgTag;
}EVHAND_TAGS;

DLLExport BOOL Initialize(HANDLE pReporterThis)
{
	TRACEX("Initialize OCPP Start !! \n");
	
	REPORTER* pReporter = (REPORTER*)pReporterThis;
	int n;

	//Create data
	pReporter->pDataHeap = NEW(OCPP_DATA_STU, 1);
	if(pReporter->pDataHeap == NULL)
	{
		TRACEX("Memory full for OCPP dataStructs!\n");
		return FALSE;
	}

	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pReporter->pDataHeap;
	ZERO_POBJS(pOCPP, 1);
	pOCPP->hReporter = pReporterThis;
	InitOCPPMsgInfo(pOCPP);

	pOCPP->hMutex = Mutex_Create(TRUE);
	pOCPP->hqMsg2Sent = Queue_Create(MAX_NORMAL_MSG_QUEUE_SIZE, sizeof(OCPP_CALL_MSG), 0);
	pOCPP->hqHistoryTrans = Queue_Create(MAX_TRANS_RELATED_MSG_QUEUE_SIZE, sizeof(OCPP_CALL_MSG), 0);
	if((pOCPP->hMutex == NULL) || (pOCPP->hqMsg2Sent == NULL) || (pOCPP->hqHistoryTrans == NULL))
	{
		TRACEX("ERROR : Fail to Create Mutex and Queue \n");
		return FALSE;
	}

	for(n = 0; n < Max_Connctors; n++)
	{
		pOCPP->stConnectorData[n].nGunChargeStatus = GUN_STATE_NOT_INSTALL; //20190302: all connector's initial state should be not_install
		pOCPP->stConnectorData[n].hqInProcessTrans = Queue_Create(MAX_TRANS_RELATED_MSG_QUEUE_SIZE, sizeof(OCPP_CALL_MSG), 0);
		if(pOCPP->stConnectorData[n].hqInProcessTrans == NULL)
		{
			TRACEX("ERROR : Fail to Create RT Trans Queue \n");
			return FALSE;
		}
	}
	
	//20190223:Now initial the TSL root CA
	char	szRootCAFile[255] = {0};
	MakeFullFileName(szRootCAFile, \
		getenv(ENV_VARIABLE_APP_PATH), "config", "cacert.pem");
	pReporter->pUsedCommPort->Control(NULL, COMM_SET_WS_GLOBAL_TLS_ROOT_CAFILE, (void*)szRootCAFile, 0);

	pOCPP->nMsgCallTimeOut_Sec = 10; //default timeout seconds
	pOCPP->emFirmwareUpgradeStatus = FIRMUPDATE_Idle;

	//server URL , format[ end url + "/ChargerID"]
	PARAM_VAL_GET valGet;
	valGet.nGetSize = 255;
	char szEndUrl[255];
	valGet.val.blobValue = (BYTE*)szEndUrl; //&pOCPP->szServerURL;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 18, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: OCPP server URL can not be retrieved\n");
		return FALSE;
	}

	valGet.nGetSize = 4;
	for(n = 0; n < Max_Connctors; n++)
	{
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			401 + n, 401, &valGet, sizeof(PARAM_VAL_GET));
		if(err != ERRCODE_DAI_OK)
		{
			TRACEX("Get Meter[%d] Type error!\n", n);
			return FALSE;
		}
		pOCPP->stConnectorData[n].emMeterTp =(METER_TYPE) valGet.val.emValue;
	}

	char szChargerID[64];
	valGet.nGetSize = 64;
	valGet.val.blobValue = (BYTE*)szChargerID;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 2, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: OCPP chargerID can NOT be retrieved\n");
		return FALSE;
	}

	snprintf(pOCPP->szServerURL, 255, "%s/%s", szEndUrl, szChargerID);
	//snprintf(pOCPP->szServerURL, 255, "%s", szEndUrl); //simulator is not identifying chargerID section
	//Percent_EncodeUrl(pOCPP->szServerURL); //Jimmy note 20181129: maybe do not need as URL should be correct when input on LCD

	//Open comm port
	/*pReporter->hCommPort = NULL;	
	if(!TryConnect2Server(pReporter))
	{
		TRACEX("OCPP port config error!\n");
		return FALSE;
	}*/

	//Read OCPP settings
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 55, &valGet, sizeof(PARAM_VAL_GET));
	pOCPP->TimeCalibration = ((TIME_CALIBRATION)valGet.val.emValue == TIME_OCPP)? TRUE : FALSE; 
	
	valGet.nGetSize = sizeof(A_BLOB_OCPP_CONFIGS);
	valGet.val.blobValue = (BYTE*)&pOCPP->stOCPPSettings;
	 err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 16, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: OCPP settings can not be retrieved\n");
		return FALSE;
	}

	pReporter->bExit = FALSE;
	
	pReporter->hThread = RunThread_Create("OCPPComm",
		(RUN_THREAD_START_PROC)_ThreadEntry_OCPPComm,
		(void *)(pReporter),
		(DWORD*)&pReporter->bExit);
		
	if(pReporter->hThread == NULL)
	{
		TRACEX("ERROR : Create OCPPComm thread Failed \n");
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
			{EVENT_TYPE_METERVAL_UPDATE_IN_CHARGING,	EVENT_MSG_BEGIN},
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

void Send_Call(OCPP_DATA_STU* pOCPP, cJSON* pJson, OCPP_MSG_TYPE msgType, OCPP_MSGCALL_SRC emMsgSrc, int nConnNum, BOOL bUrgent)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	OCPP_CALL_MSG msg;
	ZERO_POBJS(&msg, 1);
	char *szData = 0;
	szData = cJSON_Print(pJson);
	TRACEX("MsgCall[%s], msgSrc = %d, Json_Print = %s\n",pOCPP->stMsgInfoList[msgType].szMsgName, emMsgSrc,
		szData);

	cJSON_Delete(pJson);

	//make the msg
	msg.emMsgType = msgType;
	msg.emSrc = emMsgSrc;
	msg.nBelongedConn = nConnNum;
	//memcpy(msg.szUniqMsgID, pOCPP->stSuspendedMsgList[connectorNo].nUniqMsgId, sizeof(pOCPP->stSuspendedMsgList[connectorNo].nUniqMsgId));
	//sprintf(msg.nUniqMsgId, "%lu", (unsigned int)(GetCurrentTime() * 1000));
	memset(msg.szJsonData, 0, OCPP_MAX_SENTMSG_SIZE);
	//memcpy(msg.szJsonData, szData, strlen(szData));
	snprintf(msg.szJsonData, OCPP_MAX_SENTMSG_SIZE, "%s", szData);
	//msg.nSentLength = strlen(szData);
	free(szData);

	HANDLE hq = pOCPP->hqMsg2Sent;
	if(emMsgSrc == CALL_SRC_HisTrans)
	{
		hq = pOCPP->hqHistoryTrans;
		//if(msgType == MeterValues)
		//{
		//	msg.tmRelatedTransStart = pOCPP->recHandling_HistoryMeterVals.tmChargeStart;
		//}
		//else
		{
			//Jimmy 20190305: metervalues should be the same as charge recode on Start time
			msg.tmRelatedTransStart = pOCPP->recInHandling_HistoryChargeRec.tmChargeStart;
			if(msgType == MeterValues)
			{
				msg.tmSampled = pOCPP->currHandlingHisMeterVal.tmSampled;
			}
		}
	}
	else if(emMsgSrc == CALL_SRC_InProcessTrans)
	{
		if((nConnNum < Connctor_1) || (nConnNum >= Max_Connctors))
		{
			return;
		}
		hq = pOCPP->stConnectorData[nConnNum].hqInProcessTrans;
		msg.tmRelatedTransStart = pOCPP->stConnectorData[nConnNum].blobInHandling_ChargeInfo.tmChargeStart;
		if(msgType == MeterValues)
		{
			msg.tmSampled = pOCPP->currHandlingHisMeterVal.tmSampled;
		}
		//if(msgType == StartTransaction)
		//{
		//	TRACEX("---msgType = %d, tmRelatedTransStart = %d---\n", msgType, msg.tmRelatedTransStart);
		//}
	}

	if(pOCPP->emOCPPConnStatus != Ready)
	{
		if((msgType == BootNotification) || ((pOCPP->emOCPPConnStatus == BootNotify_Pending) && ((msgType == GetConfiguration) ||
			(msgType == ChangeConfiguration) || (msgType == TriggerMessage)))) //can allow only these 3 types while pending
		{
			if(pReporter->pUsedCommPort->Write(pReporter->hCommPort,
				MakeOCPP_CallFrame(pOCPP, &msg, FALSE), pOCPP->nSendBuffLen) <= 0)
			{
				pOCPP->nConnErrTimes++;
				if (pOCPP->nConnErrTimes >= 3)
				{
					pOCPP->emOCPP_Disconn_Reason = OCPP_COMM_FAIL_REASON_TIMEOUT_RECEIVE;
					pOCPP->nConnErrTimes = 0;
					pOCPP->emOCPPConnStatus = Disconnected;
					TRACEX("emOCPPConnStatus : Disconnected \n");
				}
				//Queue_Put(hq, &msg, bUrgent);
				TRACEX("Write Boot-Related-Msg [%s] failed\n", pOCPP->szSendBuffer);
			}
			else
			{
				DUMP_OCPPMSG(pReporter, DUMP_TYPE_WRITE, pOCPP->szSendBuffer);
				//pOCPP->nConnErrTimes = 0;
				TRACEX("Write Boot-Related-Msg [%s] successfully\n", pOCPP->szSendBuffer);
			}
		}
		else if(emMsgSrc == CALL_SRC_InProcessTrans) //Jimmy 20190306: if disconnect and charging is going on, should keep the messages in the queue
		{
			//Jimmy added 20190319: shall keep sure there is only ONE session be stored in the trans queue!
			if(msgType == StartTransaction)
			{
				Queue_Empty(hq);
			}
			Queue_Put(hq, &msg, bUrgent);
			TRACEX("Queue_Put Inprocess Call Successfully into WebSocket %s \n", msg.szJsonData);
		}
		else //need handle it later
		{
			Queue_Put(hq, &msg, bUrgent);
		}
	}
	else
	{
		if(bUrgent && (hq == pOCPP->hqMsg2Sent))
		{
			pOCPP->nQUrgentMsgCount++;
		}
		//Jimmy added 20190319: if there are 2 sessions stored while off line, and connection is restored,
		//we should discard the old session(history list will handle it) in case queue is too old and new session is missing unexpectedly
		if((msgType == StartTransaction) && (emMsgSrc == CALL_SRC_InProcessTrans))
		{
			if(Queue_GetCount(hq, NULL) > 10) //give 10 is to leave some extra time for old session handling
			{
				LogOut(MAIN_MODULE_REPORTER, "OCPP", LOG_TYPE_WARING,
					"Offline charge session for gun[%d] is removed from transaction queue!", nConnNum + 1);
				Queue_Empty(hq);
			}
		}
		Queue_Put(hq, &msg, bUrgent);
		TRACEX("Queue_Put NORMAL Call Successfully into WebSocket %s \n", msg.szJsonData);
	}
}

void Send_CallResult(OCPP_DATA_STU* pOCPP, cJSON* pJson, OCPP_MSG_TYPE msgType)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	char *szData = 0;
	szData = cJSON_Print(pJson);
	TRACEX("Send_CallResult Json_Print = %s\n",szData);

	cJSON_Delete(pJson);

	snprintf(pOCPP->szSendBuffer, OCPP_MAX_SENTMSG_SIZE, "[%d,\"%s\",%s]",
		CallResult,
		pOCPP->szHandlingUniqID,
		szData);

	free(szData);

	pOCPP->nSendBuffLen = strlen(pOCPP->szSendBuffer);
	TRACEX("EVERYMSG == %s  Length = %d \n", pOCPP->szSendBuffer, pOCPP->nSendBuffLen);

	if (pReporter->pUsedCommPort->Write(pReporter->hCommPort,
		pOCPP->szSendBuffer, pOCPP->nSendBuffLen))
	{
		DUMP_OCPPMSG(pReporter, DUMP_TYPE_WRITE, pOCPP->szSendBuffer);
		TRACEX("Write CallResult Successfully into WebSocket\n");
	}
	else
	{
		LogOut(MAIN_MODULE_REPORTER, "OCPP", LOG_TYPE_ERROR,
			"MsgCallResult [%s] is NOT written...", pOCPP->stMsgInfoList[msgType].szMsgName);
		TRACEX("Write CallResult ERROR by WebSocket\n");
	}
}

void Send_CallError(OCPP_DATA_STU* pOCPP, OCPP_MSG_TYPE msgType, OCPP_CALLERROR_CODE emCode, const char* szErrDescrip, cJSON* jsonErrDetail)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	char *szErrDetails;
	if(jsonErrDetail != NULL)
	{
		szErrDetails = cJSON_Print(jsonErrDetail);
		cJSON_Delete(jsonErrDetail);
		TRACEX("Send_CallErrcor Json_PrintErrDetail = %s\n",szErrDetails);
	}
	else
	{
		szErrDetails = malloc(3);
		sprintf(szErrDetails, "{}");
	}

	static const char *Error_Code[] = 
	{
		"NotImplemented",
		"NotSupported",
		"InternalError",
		"ProtocolError",
		"SecurityError",
		"FormationViolation",
		"PropertyConstraintViolation",
		"OccurenceConstraintViolation",
		"TypeConstraintViolation",
		"GenericError",
	};

	snprintf(pOCPP->szSendBuffer, OCPP_MAX_SENTMSG_SIZE, "[%d,\"%s\",\"%s\",\"%s\",%s]",
		CallError,
		pOCPP->szHandlingUniqID,
		Error_Code[emCode],
		szErrDescrip,
		szErrDetails);

	free(szErrDetails);

	pOCPP->nSendBuffLen = strlen(pOCPP->szSendBuffer);
	TRACEX("EVERYMSG == %s  Length = %d \n", pOCPP->szSendBuffer, pOCPP->nSendBuffLen);

	if(pReporter->pUsedCommPort->Write(pReporter->hCommPort,
		pOCPP->szSendBuffer, pOCPP->nSendBuffLen))
	{
		DUMP_OCPPMSG(pReporter, DUMP_TYPE_WRITE, pOCPP->szSendBuffer);
		TRACEX("Write CallError Successfully into WebSocket\n");
		LogOut(MAIN_MODULE_REPORTER, "OCPP", LOG_TYPE_WARING,
			"MsgCallError for [%s], ErrDescription = [%s], is sent out...", (msgType == Msg_Unkown) ? "Unkown" :
			pOCPP->stMsgInfoList[msgType].szMsgName, szErrDescrip);
	}
	else
	{
		LogOut(MAIN_MODULE_REPORTER, "OCPP", LOG_TYPE_ERROR,
			"MsgCallError for [%s], ErrDescription = [%s], is NOT written...", (msgType == Msg_Unkown) ? "Unkown" :
			pOCPP->stMsgInfoList[msgType].szMsgName, szErrDescrip);
		TRACEX("Write CallResult ERROR by WebSocket\n");
	}
}

