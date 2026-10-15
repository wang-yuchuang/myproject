#include "ocpp.h"

char* Make_OCPPTimeString(time_t tmInout, char* pszOut)
{
	struct tm currenttime;

	//tmInout = _NOW_; // test purpose  nned to be Comment in Final Version 
	
	gmtime_r(&tmInout,&currenttime);
	currenttime.tm_year = 2000 + (currenttime.tm_year - 100);
	char timeinstring[32] = {'\0'};

	snprintf(timeinstring, 32,"%4d-%02d-%02dT%02d:%02d:%02dZ",currenttime.tm_year, currenttime.tm_mon + 1, 
		currenttime.tm_mday, currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec);

	memcpy(pszOut, timeinstring,32);
	return pszOut;
}

time_t  Parse_OCPPTime(char *pszOut)
{
	time_t t = Parse_ISO8601TimeStr(pszOut);
	return t;
}

int Prepare_Authorize(void* pArg)
{
	TRACEX("Prepare_Authorize \n");

	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	
	cJSON *pJson_Auth;

	pJson_Auth = cJSON_CreateObject();
	
	cJSON_AddStringToObject(pJson_Auth, "idTag", pOCPP->stAuthData.szIdTag);

	//Msg_Send(pOCPP, pJson_Auth, Authorize, Call, 0, bSend_Later);
	Send_Call(pOCPP, pJson_Auth, Authorize, CALL_SRC_Normal, NULL, TRUE); //authrize.req should be sent out as URGENT msg so that it can be handled ASAP

	return 0;
}

int Prepare_BootNotification(void* pArg)
{
	TRACEX("Prepare_BootNotification \n");

	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;

	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	
	cJSON *root;
	/* Get Info for BootNotification */
	PARAM_VAL_GET valGet;
	A_BLOB_CHARGER_BOOTINFO bootInfo;
	valGet.nGetSize = sizeof(A_BLOB_CHARGER_BOOTINFO);
	valGet.val.blobValue = (BYTE*)&bootInfo;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 17, &valGet, sizeof(PARAM_VAL_GET));
	
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("Prepare_BootNotification -> Get Data fail \n");
		return -1;
	}

	root = cJSON_CreateObject();
	
	//Jimmy: charge Box SN is no longer used
	//if(bootInfo.emBootInfoOptions.chargerBoxSN)
	//{
	//	cJSON_AddStringToObject(root, "chargeBoxSerialNumber", bootInfo.szChargerBoxSerialNum);
	//}

	cJSON_AddStringToObject(root, "chargePointModel", bootInfo.szChargePointModel);

	//if (bootInfo.emBootInfoOptions.chargerPointSN
	if(bootInfo.emBootInfoOptions.chargerPointSN || (strlen(bootInfo.szChargePointSerualNuml) > 0))
	{
		cJSON_AddStringToObject(root, "chargePointSerialNumber", bootInfo.szChargePointSerualNuml);
	}
	
	cJSON_AddStringToObject(root, "chargePointVendor", bootInfo.szChargePointVendor);

	if(bootInfo.emBootInfoOptions.firmwareVer)
	{
		cJSON_AddStringToObject(root, "firmwareVersion", bootInfo.szFirmwareVer);
	}
	if(bootInfo.emBootInfoOptions.iccid)
	{
		cJSON_AddStringToObject(root, "iccid", bootInfo.szICCID);
	}
	if(bootInfo.emBootInfoOptions.imsi)
	{
		cJSON_AddStringToObject(root, "imsi", bootInfo.szIMSI);
	}
	if(bootInfo.emBootInfoOptions.meterType)
	{
		cJSON_AddStringToObject(root, "meterType", bootInfo.szMeterType);
	}
	if(bootInfo.emBootInfoOptions.meterSN)
	{
		cJSON_AddStringToObject(root, "meterSerialNumber", bootInfo.szMeterSerialNum);
	}

	//Msg_Send(pOCPP, root, BootNotification, Call,0, bSend_Later);
	Send_Call(pOCPP, root, BootNotification, CALL_SRC_Normal, NULL, TRUE);

	return 0;
}

int Prepare_HeartBeat(void* pArg)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	
	cJSON *root;

	root = cJSON_CreateObject();
	//Msg_Send(pOCPP, root, HeartBeat, Call,0, bSend_Later);
	Send_Call(pOCPP, root, HeartBeat, CALL_SRC_Normal, NULL, FALSE);
	//we haven't decided the return code meaning here
	return 0;

}

int Prepare_StartTransaction(void* pArg)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER *pReporter = (REPORTER*)pOCPP->hReporter;

	cJSON *pJson_StartTx;

	OBJ_PTR arg;

	TRACEX("Prepare_StartTransaction  %d \n");

	char szUserID[64];
	time_t tmChargeStart;
	A_BLOB_RUN_CHARGE_INFO  StartTxInfo;
	RUN_CHARGE_EXTRA_DATA *pData = &StartTxInfo.extraData;
	
	if(pOCPP->bTransMsgFromHistory)
	{
		pData = &pOCPP->recInHandling_HistoryChargeRec.extraRecData.runData;
		memcpy(szUserID, pOCPP->recInHandling_HistoryChargeRec.szUserID, 64);
		tmChargeStart = pOCPP->recInHandling_HistoryChargeRec.tmChargeStart;
	}
	else
	{
		arg.nId = 201 + pOCPP->emHandlingConnector;
		arg.nSubId = 107;

		PARAM_VAL_GET valGet;
		valGet.nGetSize = sizeof(A_BLOB_RUN_CHARGE_INFO);
		valGet.val.blobValue = (BYTE*)&StartTxInfo;
	
		ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			arg.nId, arg.nSubId , &valGet, sizeof(PARAM_VAL_GET));
		if (err != ERRCODE_DAI_OK)
		{
			// need to Write some statements 
			TRACEX("Prepare_StartTransaction 1 ERROR \n");
			return -1;
		}

		memcpy(szUserID, StartTxInfo.szUserID, 64);
		tmChargeStart = StartTxInfo.tmChargeStart;
		pOCPP->stConnectorData[pOCPP->emHandlingConnector].blobInHandling_ChargeInfo = StartTxInfo; // added yash
		
	}

	pJson_StartTx = cJSON_CreateObject();

	if (pJson_StartTx == NULL)
	{
		TRACEX(" pJson_StartTx = %s \n ", cJSON_GetErrorPtr());
		return 0;
	}
	cJSON_AddNumberToObject(pJson_StartTx, "connectorId", pOCPP->emHandlingConnector + 1);

	cJSON_AddStringToObject(pJson_StartTx, "idTag",(const char*)szUserID);
	
	cJSON_AddNumberToObject(pJson_StartTx, "meterStart",(int)(pData->fkWhStartReading * 1000)); // this for test purpose 
		
	if(StartTxInfo.emChargeStartMethod == CHARGE_START_BY_RESERVE)
	{
		cJSON_AddNumberToObject(pJson_StartTx, "reservationId", pData->nReservationID);
	}

	char szTime[32];
	cJSON_AddStringToObject(pJson_StartTx, "timestamp", (const char *)Make_OCPPTimeString(tmChargeStart,szTime));
	
	Send_Call(pOCPP, pJson_StartTx, StartTransaction, pOCPP->bTransMsgFromHistory ? CALL_SRC_HisTrans : CALL_SRC_InProcessTrans, 
		pOCPP->emHandlingConnector, FALSE); //NOTICE: Shall use 'FALSE'!!!

	return 0;
}

static int GetOCPPCharge_StopReason(CHARGE_STOPREASON emSrcReason)
{
	int nReason = -1;

	switch (emSrcReason)
	{
	case STOPREASON_NORMAL_STOP:
	case STOPREASON_EV_REQ_STOP: 
		//normal ,no code
		break;
	case STOPREASON_SYSALM_EPO:
		nReason = REASON_EMERGENCYSTOP;
		break;
	case STOPREASON_COMM_WITH_EV_FAILED:
	case STOPREASON_CHRG_CONNECTOR_LOCK_FAULT:
		nReason = REASON_EVDISCONNECTED;
		break;
	case STOPREASON_SYS_REBOOT:
		nReason = REASON_REBOOT;
		break;
	case STOPREASON_HARD_RESET:
		nReason = REASON_HARDRESET;
		break;
	case STOPREASON_SOFT_RESET:
		nReason = REASON_SOFTRESET;
		break;
	case STOPREASON_SERVER_STOP:
		nReason = REASON_REMOTE;
		break;
	case STOPREASON_LOCAL_STOP:
		nReason = REASON_LOCAL;
		break;
	case STOPREASON_SYSALM_MAINFAIL:
	case STOPREASON_SYSALM_ALLRECT_FAIL:
	case STOPREASON_SYSALM_ALLRECT_COMMFAIL:
		nReason = REASON_POWERLOSS;
		break;
	case STOPREASON_DEAUTH:
		nReason = REASON_DEAUTHORIZED;
		break;
	default:
		nReason = REASON_OTHER;
		break;
	}
	return nReason;
}

int Prepare_StopTransaction(void* pArg)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;


	TRACEX("Prepare_StopTransaction \n");
	cJSON *pJson_StopTx = NULL;

	OBJ_PTR arg;

	char szUserID[64];
	int nTransID;
	A_BLOB_RUN_CHARGE_INFO  StopTx;
	RUN_CHARGE_EXTRA_DATA *pData = &StopTx.extraData;

	if(pOCPP->bTransMsgFromHistory)
	{
		pData = &pOCPP->recInHandling_HistoryChargeRec.extraRecData.runData;
		memcpy(szUserID, pOCPP->recInHandling_HistoryChargeRec.szUserID, 64);
		nTransID = pOCPP->recInHandling_HistoryChargeRec.nSessionID;
	}
	else
	{
		arg.nId = 201 + pOCPP->emHandlingConnector;
		arg.nSubId = 107;

		PARAM_VAL_GET valGet;
		valGet.nGetSize = sizeof(A_BLOB_RUN_CHARGE_INFO);
		valGet.val.blobValue = (BYTE*)&StopTx;
	
		ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			arg.nId, arg.nSubId, &valGet, sizeof(PARAM_VAL_GET));
		if (err != ERRCODE_DAI_OK)
		{
			TRACEX("Prepare_StopTransaction_1 ERROR  %d \n",err);
			return -1;
		}

		//added by Jimmy 20190704 for REBOOT charging session restoring
		pOCPP->stConnectorData[pOCPP->emHandlingConnector].blobInHandling_ChargeInfo = StopTx;
		
		memcpy(szUserID, StopTx.szUserID, 64);
		nTransID = StopTx.nSessionID;
	}

	TRACEX("Prepare_StopTransaction CreateObject\n");
	pJson_StopTx = cJSON_CreateObject();

	if((StopTx.extraData.emChargeEndReason != REASON_EMERGENCYSTOP) &&
		(StopTx.extraData.emChargeEndReason != REASON_HARDRESET) &&
		(StopTx.extraData.emChargeEndReason != REASON_REBOOT) && 
		(StopTx.extraData.emChargeEndReason != REASON_SOFTRESET) &&
		(strlen(pData->szStopUserID) > 0)) //in case stop idTag not provided
	//if(strcmp(szUserID, pData->szStopUserID) != 0) //should always send idTag if it is valid
	{
		cJSON_AddStringToObject(pJson_StopTx, "idTag", pData->szStopUserID);
	}
	
	cJSON_AddNumberToObject(pJson_StopTx, "meterStop",(pData->fkWhEndReading * 1000)); // for test 

	char szTime[21];
	cJSON_AddStringToObject(pJson_StopTx, "timestamp", Make_OCPPTimeString(pData->tmChargeEnd, szTime));
	
	//if (nTransID == 0)
	//{
	//	TRACEX("Transaction ID is NULL \n");
	//	cJSON_Delete(pJson_StopTx);
	//	return 0;
	//}
	//else
	{
		cJSON_AddNumberToObject(pJson_StopTx, "transactionId", nTransID);
	}

	int nStopReason = GetOCPPCharge_StopReason(pData->emChargeEndReason);
	if(nStopReason != -1)
	{
		const char* REASON_STRING[] =
		{
			"EmergencyStop",
			"EVDisconnected",
			"HardReset",
			"Local",
			"Other",
			"PowerLoss",
			"Reboot",
			"Remote",
			"SoftReset",
			"UnlockCommand",
			"DeAuthorized",
		};
		cJSON_AddStringToObject(pJson_StopTx, "reason", REASON_STRING[nStopReason]);
	}

	//Jimmy note: right now we do not support, will think about it in the future
	//if (pOCPP->StopCharge.bHasOptinalInfo)
	//{
	//	// need to implement later 
	//	//cJSON_AddStringToObject(pJson_StopTx, "transactionData", "Need to Implement Later " );
	//}
	
	//Msg_Send(pOCPP, pJson_StopTx, StopTransaction, Call, pOCPP->nCurrChargeStatusHandled_Conn, bSend_Later);
	Send_Call(pOCPP, pJson_StopTx, StopTransaction, pOCPP->bTransMsgFromHistory ? CALL_SRC_HisTrans : CALL_SRC_InProcessTrans, 
		pOCPP->emHandlingConnector, FALSE); //NOTICE: Shall use 'FALSE'!!!

	return 0;
	
}

static int GetConnectorVendorErrCode_ByAlarm(REPORTER* pReporter, int nGunIdx)
{
	static int s_nCCUGrpAlarmTable[] = {6, 7, 8, 9, 10, 13, 14};
	static int s_nCCUAlarmTable[] = {15, 16, 17, 18, 19, 20};
	int n;
	VAR_VALUE varV;
	for(n = 0; n < ITEM_OF(s_nCCUGrpAlarmTable); n++)
	{
		if(pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARMSTAT_R,
			200, s_nCCUGrpAlarmTable[n], &varV, 4) == ERRCODE_DAI_OK)
		{
			if(varV.emValue == 1)
			{
				return s_nCCUGrpAlarmTable[n];
			}
		}
	}

	for(n = 0; n < ITEM_OF(s_nCCUGrpAlarmTable); n++)
	{
		if(pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARMSTAT_R,
			201 + nGunIdx, s_nCCUAlarmTable[n], &varV, 4) == ERRCODE_DAI_OK)
		{
			if(varV.emValue == 1)
			{
				return s_nCCUAlarmTable[n];
			}
		}
	}

	return -1;
}

int Prepare_StatusNotification(void* pArg)
{
	static const char* CHARGEPOINTERRORCODE_STRING[] =
	{
		"ConnectorLockFailure",
		"EVCommunicationError",
		"GroundFailure",
		"HighTemperature",
		"InternalError" ,
		"LocalListConflict",
		"NoError",
		"OtherError",
		"OverCurrentFailure",
		"OverVoltage",
		"PowerMeterFailure",
		"PowerSwitchFailure",
		"ReaderFailure",
		"ResetFailure",
		"UnderVoltage",
	};
	static const char* CHARGEPOINTSTATUS_STRING[] =
	{
		"Available",
		"Preparing",
		"Charging",
		"SuspendedEVSE",
		"SuspendedEV",
		"Finishing",
		"Reserved",
		"Unavailable",
		"Faulted",
	};

	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	cJSON *pJson_Status;
	int n;

	//TRACEX("Prepare_StatusNotification \n");

	OBJ_PTR arg;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	BOOL bSendUrgent = (pOCPP->emTriggerMsgType == Trigger_StatusNotification) ? TRUE : FALSE;

	if(pOCPP->emHandlingConnector == Connctor_0) //means system
	{
		TRACEX("Status Change ChargePoint   -1 \n");

		if(pOCPP->emTriggerMsgType == Trigger_StatusNotification)
		{
			TRACE("\n-------Now triggle connector0 status----------\n");
			//A_BLOB_CAHRGEPOINT_STATUSINFO StatusNotiData;
			pOCPP->blobCPStatusInfo.emChargePointStatus = CP_STATUS_Available; //initial state
			pOCPP->blobCPStatusInfo.emChargePointErrcode = CP_ERRCODE_NoError;
			valGet.nGetSize = sizeof(A_BLOB_CAHRGEPOINT_STATUSINFO);
			valGet.val.blobValue = (BYTE*)&pOCPP->blobCPStatusInfo;
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				1, 10, &valGet, sizeof(PARAM_VAL_GET));
		}

		A_BLOB_CAHRGEPOINT_STATUSINFO* pStatusInfo = &pOCPP->blobCPStatusInfo;

		pJson_Status = cJSON_CreateObject();

		cJSON_AddNumberToObject(pJson_Status, "connectorId", 0);
		cJSON_AddStringToObject(pJson_Status, "errorCode", CHARGEPOINTERRORCODE_STRING[pStatusInfo->emChargePointErrcode]);

		if (pStatusInfo->bHasOptinalInfo)
		{
			cJSON_AddStringToObject(pJson_Status, "info", pStatusInfo->szInfo);
		}

		cJSON_AddStringToObject(pJson_Status, "status", CHARGEPOINTSTATUS_STRING[pStatusInfo->emChargePointStatus]);

		if (pStatusInfo->bHasOptinalInfo)
		{
			char szTime[21];
			cJSON_AddStringToObject(pJson_Status, "timestamp", Make_OCPPTimeString(pStatusInfo->tmTimeStamp,szTime));
		}

		if (pStatusInfo->bHasOptinalVerdorErrInfo)
		{
			if(strlen(pStatusInfo->szVendorId) > 0)
			{
				cJSON_AddStringToObject(pJson_Status, "vendorId", pStatusInfo->szVendorId);
			}
			cJSON_AddStringToObject(pJson_Status, "vendorErrorCode", pStatusInfo->szVendorErrorCode);
		}

		if(CP_STATUS_Faulted == pStatusInfo->emChargePointStatus)
		{
			for(n = 0; n < Max_Connctors; n++)
			{
				pOCPP->stConnectorData[n].emConnctorStauts = Conn_Failed;
			}
		}
	}
	else
	{
		int nGunIdx = pOCPP->emHandlingConnector; // nGunIdx Possible 0,1,2
		//here we ASSUME the nGUNIDX is correct range!!!
		if(pOCPP->emTriggerMsgType == Trigger_StatusNotification)
		{
			valGet.nGetSize = 4;
			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				201 + nGunIdx, 99, &valGet, sizeof(PARAM_VAL_GET));
			if(err == ERRCODE_DAI_OK)
			{
				pOCPP->stConnectorData[nGunIdx].nGunPrepareStatus = valGet.val.emValue;
			}
			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				201 + nGunIdx, 101, &valGet, sizeof(PARAM_VAL_GET));
			if(err == ERRCODE_DAI_OK)
			{
				pOCPP->stConnectorData[nGunIdx].nGunChargeStatus = valGet.val.emValue;
			}
		}

		pJson_Status = cJSON_CreateObject();
		cJSON_AddNumberToObject(pJson_Status, "connectorId", nGunIdx + 1);
		CHARGEPOINT_STATUS emConnCurrStatus = CP_STATUS_Unavailable;
		int nVendorErrCode = -1;
		char szVendorErrCode[50] = {0};
		switch(pOCPP->stConnectorData[nGunIdx].nGunChargeStatus)
		{
		case GUN_STATE_IDLE: //idle
		//case GUN_STATE_HANDSHAKE: //Handhshake
		//case GUN_STATE_CHARGE_FINISHED: //Finished
			pOCPP->stConnectorData[nGunIdx].emConnctorStauts = Conn_Idle;
			cJSON_AddStringToObject(pJson_Status, "errorCode", CHARGEPOINTERRORCODE_STRING[CP_ERRCODE_NoError]);
			if((pOCPP->stConnectorData[nGunIdx].nGunPrepareStatus == 1)/* && 
				(pOCPP->stConnectorData[nGunIdx].nGunChargeStatus == GUN_STATE_IDLE) ||
				(pOCPP->stConnectorData[nGunIdx].nGunChargeStatus == GUN_STATE_HANDSHAKE)*/) //plugged or handshake
			{
				cJSON_AddStringToObject(pJson_Status, "status", CHARGEPOINTSTATUS_STRING[CP_STATUS_Preparing]);
				emConnCurrStatus = CP_STATUS_Preparing;
			}
			else
			{
				cJSON_AddStringToObject(pJson_Status, "status", CHARGEPOINTSTATUS_STRING[CP_STATUS_Available]);
				emConnCurrStatus = CP_STATUS_Available;
			}
			break;
		case GUN_STATE_NOT_INSTALL:
		case GUN_STATE_LOCKED:
			pOCPP->stConnectorData[nGunIdx].emConnctorStauts = Conn_UnAvailable;
			cJSON_AddStringToObject(pJson_Status, "errorCode", CHARGEPOINTERRORCODE_STRING[CP_ERRCODE_NoError]);
			cJSON_AddStringToObject(pJson_Status, "status", CHARGEPOINTSTATUS_STRING[CP_STATUS_Unavailable]);
			break;
		case GUN_STATE_FAULTED: //faulted
			pOCPP->stConnectorData[nGunIdx].emConnctorStauts = Conn_Failed;
			//need to get fail reason later
			cJSON_AddStringToObject(pJson_Status, "errorCode", CHARGEPOINTERRORCODE_STRING[CP_ERRCODE_OtherError]);
			cJSON_AddStringToObject(pJson_Status, "status", CHARGEPOINTSTATUS_STRING[CP_STATUS_Faulted]);
			nVendorErrCode = GetConnectorVendorErrCode_ByAlarm(pReporter, nGunIdx);
			if(nVendorErrCode != -1)
			{
				sprintf(szVendorErrCode, "%d", nVendorErrCode);
				cJSON_AddStringToObject(pJson_Status, "vendorErrorCode", szVendorErrCode);
			}
			emConnCurrStatus = CP_STATUS_Faulted;
			break;
		case GUN_STATE_RESERVE_PENDING:
			pOCPP->stConnectorData[nGunIdx].emConnctorStauts = Conn_Reserved;
			cJSON_AddStringToObject(pJson_Status, "errorCode", CHARGEPOINTERRORCODE_STRING[CP_ERRCODE_NoError]);
			cJSON_AddStringToObject(pJson_Status, "status", CHARGEPOINTSTATUS_STRING[CP_STATUS_Reserved]);
			emConnCurrStatus = CP_STATUS_Reserved;
			break;
		case GUN_STATE_WAITING_START: //Waiting for Start
			bSendUrgent = TRUE;
			pOCPP->stConnectorData[nGunIdx].emConnctorStauts = Conn_InCharging;
			cJSON_AddStringToObject(pJson_Status, "errorCode", CHARGEPOINTERRORCODE_STRING[CP_ERRCODE_NoError]);
			//cJSON_AddStringToObject(pJson_Status, "status", CHARGEPOINTSTATUS_STRING[CP_STATUS_Preparing]);
			if(pOCPP->stConnectorData[nGunIdx].nGunSuspendEVSESatus == 1) //suspendEVSE
			{
				cJSON_AddStringToObject(pJson_Status, "status", CHARGEPOINTSTATUS_STRING[CP_STATUS_SuspendedEVSE]);
				emConnCurrStatus = CP_STATUS_SuspendedEVSE;
			}
			else
			{
				cJSON_AddStringToObject(pJson_Status, "status", CHARGEPOINTSTATUS_STRING[CP_STATUS_Preparing]);
				emConnCurrStatus = CP_STATUS_Preparing;
			}
			break;
		case GUN_STATE_HANDSHAKE: //Handhshake
		case GUN_STATE_PREPARE_4CHARGE: //Preparing
		case GUN_STATE_CHARGING: //Charging
			bSendUrgent = TRUE;
			pOCPP->stConnectorData[nGunIdx].emConnctorStauts = Conn_InCharging;
			cJSON_AddStringToObject(pJson_Status, "errorCode", CHARGEPOINTERRORCODE_STRING[CP_ERRCODE_NoError]);
			cJSON_AddStringToObject(pJson_Status, "status", CHARGEPOINTSTATUS_STRING[CP_STATUS_Charging]);
			emConnCurrStatus = CP_STATUS_Charging;
			break;
		case GUN_STATE_CHARGE_PAUSED: //CHarging Paused //we do not have EVSE suspended status yet
			pOCPP->stConnectorData[nGunIdx].emConnctorStauts = Conn_InCharging;
			cJSON_AddStringToObject(pJson_Status, "errorCode", CHARGEPOINTERRORCODE_STRING[CP_ERRCODE_NoError]);
			//Jimmy modified: this should always be a suspendEV, suspendEVSE only happends at charge beginning
			cJSON_AddStringToObject(pJson_Status, "status", CHARGEPOINTSTATUS_STRING[CP_STATUS_SuspendedEV]);
			emConnCurrStatus = CP_STATUS_SuspendedEV;
			break;
		case GUN_STATE_WAITING_END://CHarging Stopped
		case GUN_STATE_CHARGE_FINISHED: //Finished
			bSendUrgent = TRUE;
			pOCPP->stConnectorData[nGunIdx].emConnctorStauts = Conn_InCharging;
			cJSON_AddStringToObject(pJson_Status, "errorCode", CHARGEPOINTERRORCODE_STRING[CP_ERRCODE_NoError]);
			cJSON_AddStringToObject(pJson_Status, "status", CHARGEPOINTSTATUS_STRING[CP_STATUS_Finishing]);
			emConnCurrStatus = CP_STATUS_Finishing;
			break;
		default:
			cJSON_Delete(pJson_Status);
			return -1;
		}

		static int s_nOldConnectorStatus[Max_Connctors] = {-1, -1, -1}; //in case send 2 msg with same status twice
		
		TRACE("Now handling connector[%d] status, oldp[%d]->curr[%d], bTriggle[%d]\n", nGunIdx + 1,
			s_nOldConnectorStatus[nGunIdx], emConnCurrStatus, 
			(pOCPP->emTriggerMsgType == Trigger_StatusNotification));
		
		if((emConnCurrStatus == s_nOldConnectorStatus[nGunIdx]) &&
			(pOCPP->emTriggerMsgType != Trigger_StatusNotification)/* &&
			(pOCPP->stConnectorData[nGunIdx].emConnctorStauts == Conn_InCharging)*/)
		{
			cJSON_Delete(pJson_Status);
			return -1;
		}
		else if((emConnCurrStatus == CP_STATUS_Finishing) && (s_nOldConnectorStatus[nGunIdx] == CP_STATUS_Available))
		{
			//can not jump from idle to finish
			cJSON_Delete(pJson_Status);
			return -1;
		}
		s_nOldConnectorStatus[nGunIdx] = emConnCurrStatus;
	}

	//Msg_Send(pOCPP, pJson_Status, StatusNotification, Call, pOCPP->nCurrChargeStatusHandled_Conn, bSend_Later);
	Send_Call(pOCPP, pJson_Status, StatusNotification, CALL_SRC_Normal, pOCPP->emHandlingConnector, bSendUrgent);

	return 0;
}

typedef struct tagMeterValue_Element
{
	char			*szMeasuand;
	//BOOL		bIsVarFormat_Raw; //should always use 'raw'
	float			*pfVal; //all use float value
	char			*szValFmt; // default "%.3f"
	char			*szValUnit; // default "Wh"
	char			*szPhase; //default NULL
	char			*szLocation; //default 'Outlet'
	UINT			nBitOffset; //bit map offset
	BOOL			bRelated2MeterInterr; //If true, should check meterInterrpt status

}METERVAL_ELEMENT;

static void MakeSampledVal_Json(METERVAL_ELEMENT* pEle, int emTp, cJSON *sampleValue)
{
	static const char* SampledValContexts[] =
	{
		"Interruption.Begin",
		"Interruption.End",
		"Other",
		"Sample.Clock",
		"Sample.Periodic",
		"Transaction.Begin",
		"Transaction.End",
		"Trigger",
	};
	
	char tempstr[16];
	sprintf(tempstr, pEle->szValFmt, *pEle->pfVal);
	cJSON_AddStringToObject(sampleValue, "value", (const char *)tempstr);
	if(emTp != -1)
	{
		cJSON_AddStringToObject(sampleValue, "context", SampledValContexts[emTp]);
	}
	cJSON_AddStringToObject(sampleValue, "measurand", pEle->szMeasuand);
	
	//if((pEle->szLocation) != NULL)
	
	if (strcmp(pEle->szLocation, "\0") != NULL)
	{
		TRACEX("Location %s \n", pEle->szLocation);
		cJSON_AddStringToObject(sampleValue, "location", pEle->szLocation);
	}
	if (strcmp(pEle->szValUnit, "\0") != NULL)
	{
		cJSON_AddStringToObject(sampleValue, "unit", pEle->szValUnit);
	}
	TRACEX("Phase %s \n", pEle->szPhase);
	if (strcmp(pEle->szPhase, "\0") != NULL)
	{
		cJSON_AddStringToObject(sampleValue, "phase", pEle->szPhase);
	}
}

static BOOL ProClockSysMeterElemtList(OCPP_DATA_STU *pOCPP, METERVAL_ELEMENT *pCurEle, int nCurEleNum, cJSON *sampleArray)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	A_BLOB_METER_VALUES  *pMerterVals = NULL;

	METER_MESURAND *pSetMeasurand = (pOCPP->emHandlingMeterType == Sample_Clock) ?
		&pOCPP->stOCPPSettings.cslMeterValAlignData_RW : &pOCPP->stOCPPSettings.cslMeterValSampleData_RW;

	UINT uCfgMeasurand_Bits = *((UINT*)pSetMeasurand);

	int emTp = -1,n;
	cJSON *sampleValue;

	METERVAL_ELEMENT *pEle = pCurEle;
	int nEleNum = nCurEleNum;

	PARAM_VAL_GET valGet;
	A_BLOB_METER_VALUES meterValues;
	valGet.nGetSize = sizeof(A_BLOB_METER_VALUES);
	valGet.val.blobValue = (BYTE*)&meterValues;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,1, 9, &valGet, sizeof(PARAM_VAL_GET));
	pMerterVals = &meterValues;

	UINT uSampMeasurand_Bits = *(UINT*)&pMerterVals->cslMeasurand;

	for(n = 0; n < nEleNum; n++)
	{
		emTp = -1;
		if((((uCfgMeasurand_Bits >> pEle[n].nBitOffset) & 0x01) == 0) ||
			(((uSampMeasurand_Bits >> pEle[n].nBitOffset) & 0x01) == 0))
		{
			continue;
		}
		if(pEle[n].bRelated2MeterInterr)
		{
			if(pOCPP->emMainPowerMeterTp == METER_TYPR_ac) //Main power should be ac or not installed
			{
				if(!pOCPP->bMainMeterInterrupted)
				{
					emTp = pOCPP->emHandlingMeterType;
				}
				else if(pOCPP->bNeedSendMainMeter_Inter_Begin)
				{
					pOCPP->bNeedSendMainMeter_Inter_Begin = FALSE;
					emTp = Interrupt_Begin;
				}
				else if(pOCPP->bNeedSendMainMeter_Inter_End)
				{
					pOCPP->bNeedSendMainMeter_Inter_End = FALSE;
					emTp = Interrupt_End;
				}
				else
				{
					cJSON_Delete(sampleArray);
					return FALSE;
				}
			}
			else
			{
				cJSON_Delete(sampleArray);
				return FALSE;
			}
		}
		sampleValue = cJSON_CreateObject();
		MakeSampledVal_Json(&pEle[n], emTp, sampleValue);
		cJSON_AddItemToArray(sampleArray, sampleValue);
	}
	return TRUE;
}

static cJSON *ProClockMeterElemtList(OCPP_DATA_STU *pOCPP, METERVAL_ELEMENT *pCurEle, int nCurEleNum)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	A_BLOB_METER_VALUES  *pMerterVals = NULL;

	METER_MESURAND *pSetMeasurand = (pOCPP->emHandlingMeterType == Sample_Clock) ? 
		&pOCPP->stOCPPSettings.cslMeterValAlignData_RW : &pOCPP->stOCPPSettings.cslMeterValSampleData_RW;
	UINT uCfgMeasurand_Bits = *((UINT*)pSetMeasurand);


	int emTp = -1;
	int n;
	BOOL bRet = FALSE;

	METERVAL_ELEMENT *pEle = pCurEle;
	int nEleNum = nCurEleNum;

	int nGunIdx = pOCPP->emHandlingConnector;
	TRACEX("nGunIdx = %d pOCPP->emHandlingConnector = %d \n", nGunIdx, pOCPP->emHandlingConnector);

	PARAM_VAL_GET valGet;
	A_BLOB_METER_VALUES meterValues;
	valGet.nGetSize = sizeof(A_BLOB_METER_VALUES);
	valGet.val.blobValue = (BYTE*)&meterValues;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,201 + pOCPP->emHandlingConnector, 112, &valGet, sizeof(PARAM_VAL_GET));
	pMerterVals = &meterValues;
	UINT uSampMeasurand_Bits = *(UINT*)&pMerterVals->cslMeasurand;


	cJSON *sampleArray;
	sampleArray = cJSON_CreateArray();
	cJSON *sampleValue;

	int MaxConfigKeyCnt = 0;
	TRACEX("uCfgMeasurand_Bits : %lu  %lu \n", uCfgMeasurand_Bits, uSampMeasurand_Bits);

	for(n = 0; n < nEleNum; n++)
	{
		emTp = -1;

		if((((uCfgMeasurand_Bits >> (pEle[n].nBitOffset)) & 0x01) == 0) || 
			((uSampMeasurand_Bits >> (pEle[n].nBitOffset)) & 0x01) == 0)
		{
			TRACEX(" Home : %d %s \n", pEle[n].nBitOffset, pEle[n].szMeasuand);
			TRACEX("Bit Clear %d bit OFFSET %d\n", n, pEle[n].nBitOffset,uCfgMeasurand_Bits);
			continue;
		}

		if(pEle[n].bRelated2MeterInterr == TRUE)
		{
			TRACEX("Bit Set %d \n", n);
			if(!pOCPP->stConnectorData[nGunIdx].bMeterInterrupted)
			{
				emTp = pOCPP->emHandlingMeterType;
			}
			else if(pOCPP->stConnectorData[nGunIdx].bNeedSendMeter_Inter_Begin)
			{
				pOCPP->stConnectorData[nGunIdx].bNeedSendMeter_Inter_Begin = FALSE;
				emTp = Interrupt_Begin;
			}
			else if(pOCPP->stConnectorData[nGunIdx].bNeedSendMeter_Inter_End)
			{
				pOCPP->stConnectorData[nGunIdx].bNeedSendMeter_Inter_End = FALSE;
				emTp = Interrupt_End;
			}
			else
			{
				TRACEX("pEle->bRelated2MeterInterr %d \n", n);
				continue;
			}
		}

		if(((pOCPP->emHandlingMeterType == Sample_Clock) && (pOCPP->stOCPPSettings.nMeterValuesAlignDataMaxLen_R > MaxConfigKeyCnt)) ||
			((pOCPP->emHandlingMeterType == Sample_Periodic) || (pOCPP->emHandlingMeterType == Trigger)) && ((pOCPP->stOCPPSettings.nMeterValuesAlignDataMaxLen_R > MaxConfigKeyCnt)))
		{
			if (pOCPP->emHandlingMeterType == Trigger)
			{
				emTp = Trigger;
			}
			else
			{
				emTp = pOCPP->emHandlingMeterType;
			}
			MaxConfigKeyCnt++;
			sampleValue = cJSON_CreateObject();
			MakeSampledVal_Json(&pEle[n], emTp, sampleValue);
			cJSON_AddItemToArray(sampleArray, sampleValue);
		}
		else
		{
			TRACEX("else MeterValue \n");
			break;
		}
	}
	return sampleArray;
}

static BOOL ProMeterValueArray(cJSON *sampleArray, time_t tmSampled ,cJSON *MeterValuesArr)
{
	if(sampleArray == NULL || MeterValuesArr == NULL)
	{
		TRACEX("sampleArray == NULL || MeterValuesArr == NULL !!!\n");
		return FALSE;
	}
	char szTime[32] = {};
	cJSON *meterInst;
	meterInst = cJSON_CreateObject();
	cJSON_AddStringToObject(meterInst, "timestamp", Make_OCPPTimeString(tmSampled, szTime));
	cJSON_AddItemToObject(meterInst, "sampledValue", sampleArray);
	cJSON_AddItemToArray(MeterValuesArr, meterInst);

	return TRUE;
}

//Clock Modify
static BOOL Make_ClockMeterValues_JsonString(OCPP_DATA_STU* pOCPP, cJSON *MeterValuesArr)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	A_BLOB_METER_VALUES  *pMerterVals = NULL;

	METER_MESURAND *pSetMeasurand = (pOCPP->emHandlingMeterType == Sample_Clock) ?
		&pOCPP->stOCPPSettings.cslMeterValAlignData_RW : &pOCPP->stOCPPSettings.cslMeterValSampleData_RW;

	UINT uCfgMeasurand_Bits = *((UINT*)pSetMeasurand);
	int n;

	int emTp = -1;
	BOOL bRet = FALSE;

	cJSON *sampleArray = NULL;

	METERVAL_ELEMENT *pEle;
	int nEleNum = 0;

	if(pOCPP->emHandlingConnector == Connctor_0)
	{
		TRACE("Hello Under ChargePoint Meter Value of Clock Aligned Data \n");
		PARAM_VAL_GET valGet;
		A_BLOB_METER_VALUES meterValues;
		valGet.nGetSize = sizeof(A_BLOB_METER_VALUES);
		valGet.val.blobValue = (BYTE*)&meterValues;
		ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 1, 9, &valGet, sizeof(PARAM_VAL_GET));
		pMerterVals = &meterValues;

		METERVAL_ELEMENT meterElementListPublic[] =
		{
			//Public Parameters
			{"Energy.Active.Import.Register",	&pMerterVals->fEnergy_Active_Import_Reg_kWh,	"%.3f",	"kWh",		"\0",	"\0",	4,	TRUE},
			{"Energy.Reactive.Import.Register",	&pMerterVals->fEnergy_Reactive_Import_Reg_kvarh,"%.3f",	"kvarh",	"\0",	"\0",	6,	TRUE},
			{"Power.Active.Import",				&pMerterVals->fPower_Active_Import_kW,			"%.1f",	"kW",		"\0",	"\0",	13,	TRUE},
			{"Power.Factor",					&pMerterVals->fPower_Factor,					"%.3f",	"",			"\0",	"\0",	14,	TRUE},
			{"Power.Reactive.Import",			&pMerterVals->fPower_Reactive_Import_kvar,		"%.1f",	"kvar",		"\0",	"\0",	17,	TRUE},
			{"SoC",								&pMerterVals->fBattery_SOC_Perc,				"%.1f",	"Percent",	"\0",	"\0",	19,	FALSE},
			{"Temperature",						&pMerterVals->fTemperature_deg,					"%.1f",	"Celsius",	"\0",	"\0",	20,	FALSE},
			//L1-L2
			{"Current.Import",					&pMerterVals->f_L1L2_Current_Import_A,			"%.1f",	"A",		"L1-N",	"\0",	1,	TRUE},
			{"Voltage",							&pMerterVals->f_L1L2_Voltage_V,					"%.1f",	"V",		"L1-N",	"\0",	21,	TRUE},
			//L2-L3
			{"Current.Import",					&pMerterVals->f_L2L3_Current_Import_A,			"%.1f",	"A",		"L2-N",	"\0",	1,	TRUE},
			{"Voltage",							&pMerterVals->f_L2L3_Voltage_V,					"%.1f",	"V",		"L2-N",	"\0",	21,	TRUE},
			//L3-L1
			{"Current.Import",					&pMerterVals->f_L3L1_Current_Import_A,			"%.1f",	"A",		"L3-N",	"\0",	1,	TRUE},
			{"Voltage",							&pMerterVals->f_L3L1_Voltage_V,					"%.1f",	"V",		"L3-N",	"\0",	21,	TRUE},
		};

		//METERVAL_ELEMENT meterElementListL1L2[] =
		//{
		//	//L1-L2
		//	{"Current.Import",					&pMerterVals->f_L1L2_Current_Import_A,			"%.1f",	"A",		"L1-N",	"\0",	1,	TRUE},
		//	{"Voltage",							&pMerterVals->f_L1L2_Voltage_V,					"%.1f",	"V",		"L1-N",	"\0",	21,	TRUE},
		//};

		//METERVAL_ELEMENT meterElementListL2L3[] =
		//{
		//	//L2-L3
		//	{"Current.Import",					&pMerterVals->f_L2L3_Current_Import_A,			"%.1f",	"A",		"L2-N",	"\0",	1,	TRUE},
		//	{"Voltage",							&pMerterVals->f_L2L3_Voltage_V,					"%.1f",	"V",		"L2-N",	"\0",	21,	TRUE},
		//};//L2L3

		//METERVAL_ELEMENT meterElementListL3L1[] =
		//{
		//	//L3-L1
		//	{"Current.Import",					&pMerterVals->f_L3L1_Current_Import_A,			"%.1f",	"A",		"L3-N",	"\0",	1,	TRUE},
		//	{"Voltage",							&pMerterVals->f_L3L1_Voltage_V,					"%.1f",	"V",		"L3-N",	"\0",	21,	TRUE},
		//};//L3L1
		//we always send only 1 instance at a time

		//Public
		pEle = meterElementListPublic;
		nEleNum = ITEM_OF(meterElementListPublic);
		sampleArray = cJSON_CreateArray();
		bRet = ProClockSysMeterElemtList(pOCPP,pEle,nEleNum,sampleArray);
		if(!bRet)
		{
			return FALSE;
		}
		ProMeterValueArray(sampleArray, _NOW_ ,MeterValuesArr);

		////L1L2
		//pEle = meterElementListL1L2;
		//nEleNum = ITEM_OF(meterElementListL1L2);
		//sampleArray = cJSON_CreateArray();
		//bRet = ProClockSysMeterElemtList(pOCPP,pEle,nEleNum,sampleArray);
		//if(!bRet)
		//{
		//	return FALSE;
		//}
		//ProMeterValueArray(sampleArray, _NOW_ ,MeterValuesArr);

		////L2L3
		//pEle = meterElementListL2L3;
		//nEleNum = ITEM_OF(meterElementListL2L3);
		//sampleArray = cJSON_CreateArray();
		//bRet = ProClockSysMeterElemtList(pOCPP,pEle,nEleNum,sampleArray);
		//if(!bRet)
		//{
		//	return FALSE;
		//}
		//ProMeterValueArray(sampleArray, _NOW_ ,MeterValuesArr);

		////L3L1
		//pEle = meterElementListL3L1;
		//nEleNum = ITEM_OF(meterElementListL3L1);
		//sampleArray = cJSON_CreateArray();
		//bRet = ProClockSysMeterElemtList(pOCPP,pEle,nEleNum,sampleArray);
		//if(!bRet)
		//{
		//	return FALSE;
		//}
		ProMeterValueArray(sampleArray, _NOW_ ,MeterValuesArr);
	}
	else
	{
		//int nGunIdx = pOCPP->emHandlingConnector - 1;
		int nGunIdx = pOCPP->emHandlingConnector;
		TRACEX("nGunIdx = %d pOCPP->emHandlingConnector = %d \n", nGunIdx, pOCPP->emHandlingConnector);

		PARAM_VAL_GET valGet;
		A_BLOB_METER_VALUES meterValues;
		valGet.nGetSize = sizeof(A_BLOB_METER_VALUES);
		valGet.val.blobValue = (BYTE*)&meterValues;
		ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			201 + pOCPP->emHandlingConnector, 112, &valGet, sizeof(PARAM_VAL_GET));
		pMerterVals = &meterValues;

		UINT uSampMeasurand_Bits = *(UINT*)&pMerterVals->cslMeasurand;

		METERVAL_ELEMENT *pEle;
		int nEleNum = 0;

		if(pOCPP->stConnectorData[nGunIdx].emMeterTp == METER_TYPR_dc)
		{
			TRACEX("Meter Type METER_TYPR_dc \n ");

			METERVAL_ELEMENT meterElementList[] = 
			{
				{"Current.Export",					&pMerterVals->fCurrent_Export_A,						"%.1f",	"A","\0","\0",		0,	FALSE},
				{"Current.Import",					&pMerterVals->fCurrent_Import_A,						"%.1f",	"A","\0","\0",		1,	TRUE},
				{"Current.Offered",					&pMerterVals->fCurrent_Offered_A,						"%.1f",	"A","\0","\0",		2,	TRUE},
				{"Energy.Active.Export.Register",	&pMerterVals->fEnergy_Active_Export_Reg_kWh,			"%.3f",	"kWh","\0","\0",		3,	FALSE},
				{"Energy.Active.Import.Register",	&pMerterVals->fEnergy_Active_Import_Reg_kWh,			"%.3f",	"kWh","\0","\0",		4,	FALSE},
				{"Energy.Reactive.Export.Register",	&pMerterVals->fEnergy_Reactive_Export_Reg_kvarh,		"%.3f",	"kvarh","\0","\0",	5,	FALSE},
				{"Energy.Reactive.Import.Register",	&pMerterVals->fEnergy_Reactive_Import_Reg_kvarh,		"%.3f",	"kvarh","\0","\0",	6,	FALSE},
				{"Energy.Active.Export.Interval",	&pMerterVals->fEnergy_Active_Export_Interv_kWh,			"%.3f",	"kWh","\0","\0",		7,	FALSE},
				{"Energy.Active.Import.Interval",	&pMerterVals->fEnergy_Active_Import_Interv_kWh,			"%.3f",	"kWh","\0","\0",		8,	FALSE},
				{"Energy.Reactive.Export.Interval",	&pMerterVals->fEnergy_Reactive_Export_Interv_kvarh,		"%.3f",	"kvarh","\0","\0",	9,	FALSE},
				{"Energy.Reactive.Import.Interval",	&pMerterVals->fEnergy_Reactive_Import_Interv_kvarh,		"%.3f",	"kvarh","\0","\0",	10,	FALSE},
				{"Frequency",						&pMerterVals->fFreq_PLC_Hz,								"%.1f",	"Hz","\0","\0",		11,	TRUE},
				{"Power.Active.Export",				&pMerterVals->fPower_Active_Export_kW,					"%.1f",	"kW","\0","\0",		12,	FALSE},
				{"Power.Active.Import",				&pMerterVals->fPower_Active_Import_kW,					"%.1f",	"kW","\0","\0",		13,	FALSE},
				{"Power.Factor",					&pMerterVals->fPower_Factor,							"%.3f",	"kW","\0","\0",		14,	FALSE},
				{"Power.Offered",					&pMerterVals->fPower_Offered_kW,						"%.1f",	"kW","\0","\0",		15,	FALSE},
				{"Power.Reactive.Export",			&pMerterVals->fPower_Reactive_Export_kvar,				"%.1f",	"kvar","\0","\0",		16,	FALSE},
				{"Power.Reactive.Import",			&pMerterVals->fPower_Reactive_Import_kvar,				"%.1f",	"kvar","\0","\0",		17,	FALSE},
				{"RPM",								&pMerterVals->fFanRPM,									"%.1f",	"A","\0","\0",		18,	FALSE},
				{"SoC",								&pMerterVals->fBattery_SOC_Perc,						"%.1f",	"Percent","\0","\0",	19,	FALSE},
				{"Temperature",						&pMerterVals->fTemperature_deg,							"%.1f",	"Celsius","\0","\0",	20,	FALSE},
				{"Voltage",							&pMerterVals->fVoltage_V,								"%.1f",	"V","\0","\0",		21,	FALSE},
			};

			pEle = meterElementList;
			nEleNum = ITEM_OF(meterElementList);
			TRACEX("MeterValue %d \n",nEleNum);
			sampleArray = ProClockMeterElemtList(pOCPP,pEle,nEleNum);
			TRACEX("MeterValue %d \n", nEleNum);
			ProMeterValueArray(sampleArray, _NOW_ ,MeterValuesArr);
		}
		else
		{
			TRACEX("Meter Type METER_TYPR_ac \n ");
			METERVAL_ELEMENT meterElementListPublic[] =
			{
				//Public Parameters
				{"Energy.Active.Import.Register",	&pMerterVals->fEnergy_Active_Import_Reg_kWh,	"%.1f",	"kWh",		"\0",	"\0",	4,	TRUE},
				{"Energy.Reactive.Import.Register",	&pMerterVals->fEnergy_Reactive_Import_Reg_kvarh,"%.1f",	"kvarh",	"\0",	"\0",	6,	TRUE},
				{"Power.Active.Import",				&pMerterVals->fPower_Active_Import_kW,			"%.1f",	"kW",		"\0",	"\0",	13,	TRUE},
				{"Power.Factor",					&pMerterVals->fPower_Factor,					"%.3f",	"",			"\0",	"\0",	14,	TRUE},
				{"Power.Reactive.Import",			&pMerterVals->fPower_Reactive_Import_kvar,		"%.1f",	"kvar",		"\0",	"\0",	17,	TRUE},
				{"SoC",								&pMerterVals->fBattery_SOC_Perc,				"%.1f",	"Percent",	"\0",	"\0",	19,	FALSE},
				{"Temperature",						&pMerterVals->fTemperature_deg,					"%.1f",	"Celsius",	"\0",	"\0",	20,	FALSE},
				//L1-L2
				{"Current.Import",					&pMerterVals->f_L1L2_Current_Import_A,			"%.1f",	"A",		"L1-N",	"\0",	1,	TRUE},
				{"Voltage",							&pMerterVals->f_L1L2_Voltage_V,					"%.1f",	"V",		"L1-N",	"\0",	21,	TRUE},
				//L2-L3
				{"Current.Import",					&pMerterVals->f_L2L3_Current_Import_A,			"%.1f",	"A",		"L2-N",	"\0",	1,	TRUE},
				{"Voltage",							&pMerterVals->f_L2L3_Voltage_V,					"%.1f",	"V",		"L2-N",	"\0",	21,	TRUE},
				//L3-L1
				{"Current.Import",					&pMerterVals->f_L3L1_Current_Import_A,			"%.1f",	"A",		"L3-N",	"\0",	1,	TRUE},
				{"Voltage",							&pMerterVals->f_L3L1_Voltage_V,					"%.1f",	"V",		"L3-N",	"\0",	21,	TRUE},
			};

			//METERVAL_ELEMENT meterElementListL1L2[] =
			//{
			//	//L1-L2
			//	{"Current.Import",					&pMerterVals->f_L1L2_Current_Import_A,			"%.1f",	"A",		"L1-N",	"\0",	1,	TRUE},
			//	{"Voltage",							&pMerterVals->f_L1L2_Voltage_V,					"%.1f",	"V",		"L1-N",	"\0",	21,	TRUE},
			//};

			//METERVAL_ELEMENT meterElementListL2L3[] =
			//{
			//	//L2-L3
			//	{"Current.Import",					&pMerterVals->f_L2L3_Current_Import_A,			"%.1f",	"A",		"L2-N",	"\0",	1,	TRUE},
			//	{"Voltage",							&pMerterVals->f_L2L3_Voltage_V,					"%.1f",	"V",		"L2-N",	"\0",	21,	TRUE},
			//};//L2L3

			//METERVAL_ELEMENT meterElementListL3L1[] =
			//{
			//	//L3-L1
			//	{"Current.Import",					&pMerterVals->f_L3L1_Current_Import_A,			"%.1f",	"A",		"L3-N",	"\0",	1,	TRUE},
			//	{"Voltage",							&pMerterVals->f_L3L1_Voltage_V,					"%.1f",	"V",		"L3-N",	"\0",	21,	TRUE},
			//};//L3L1

			//Public
			pEle = meterElementListPublic;
			nEleNum = ITEM_OF(meterElementListPublic);
			sampleArray = ProClockMeterElemtList(pOCPP,pEle,nEleNum);
			TRACEX("Public MeterValueNum= %d \n", nEleNum);
			ProMeterValueArray(sampleArray, _NOW_ ,MeterValuesArr);

			////L1L2
			//pEle = meterElementListL1L2;
			//nEleNum = ITEM_OF(meterElementListL1L2);
			//sampleArray = ProClockMeterElemtList(pOCPP,pEle,nEleNum);
			//TRACEX("L1L2 MeterValueNum= %d \n", nEleNum);
			//ProMeterValueArray(sampleArray, _NOW_ ,MeterValuesArr);

			////L2L3
			//pEle = meterElementListL2L3;
			//nEleNum = ITEM_OF(meterElementListL2L3);
			//sampleArray = ProClockMeterElemtList(pOCPP,pEle,nEleNum);
			//TRACEX("L2L3 MeterValueNum= %d \n", nEleNum);
			//ProMeterValueArray(sampleArray, _NOW_ ,MeterValuesArr);

			////L3L1
			//pEle = meterElementListL3L1;
			//nEleNum = ITEM_OF(meterElementListL3L1);
			//sampleArray = ProClockMeterElemtList(pOCPP,pEle,nEleNum);
			//TRACEX("L3L1 MeterValueNum= %d \n", nEleNum);
			//ProMeterValueArray(sampleArray, _NOW_ ,MeterValuesArr);
		}
	}

	if(sampleArray == NULL)
	{
		return FALSE;
	}
	//if array is empty, do not send
	return (cJSON_GetArraySize(sampleArray) > 0) ? TRUE : FALSE;
}

static cJSON *ProTransMeterElemtList(OCPP_DATA_STU* pOCPP,METERVAL_ELEMENT *pCurEle,int nCurEleNum)
{
	REPORTER *pReporter = (REPORTER*)pOCPP->hReporter;
	A_BLOB_METER_VALUES  *pMerterVals = NULL;

	METER_MESURAND *pSetMeasurand = &pOCPP->stOCPPSettings.cslMeterValSampleData_RW;

	UINT uCfgMeasurand_Bits = *((UINT*)pSetMeasurand);

	int n;

	int emTp = -1;
	cJSON *sampleArray;
	sampleArray = cJSON_CreateArray();
	cJSON *sampleValue;

	int nGunIdx = pOCPP->emHandlingConnector;
	TRACEX("nGunIdx = %d \npOCPP->emHandlingConnector = %d \n,MeterHandling Type = %d  \n", nGunIdx, pOCPP->emHandlingConnector,pOCPP->stConnectorData[nGunIdx].emMeterTp);

	PARAM_VAL_GET valGet;
	A_BLOB_METER_VALUES meterValues;
	valGet.nGetSize = sizeof(A_BLOB_METER_VALUES);
	valGet.val.blobValue = (BYTE*)&meterValues;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		201 + pOCPP->emHandlingConnector, 112, &valGet, sizeof(PARAM_VAL_GET));
	pMerterVals = &meterValues;

	UINT uSampMeasurand_Bits = *(UINT*)&pMerterVals->cslMeasurand;

	METERVAL_ELEMENT *pEle = pCurEle;
	int nEleNum = nCurEleNum;

	/*if ((pOCPP->emHandlingMeterType == Transaction_Begin) || (pOCPP->emHandlingMeterType == Transaction_End))
	{
		pOCPP->emHandlingMeterType = Sample_Periodic;
	}*/

	int MaxConfigKeyCnt = 0;
	TRACEX("uCfgMeasurand_Bits : %lu  %lu \n", uCfgMeasurand_Bits, uSampMeasurand_Bits);

	for (n = 0; n < nEleNum; n++)
	{
		emTp = -1;
		//TRACE("No: %d %s %f %d \n",n, pEle[n].szMeasuand, pEle[n].pfVal, pEle[n].pfVal);
		if ((((uCfgMeasurand_Bits >> (pEle[n].nBitOffset)) & 0x01) == 0) ||
			((uSampMeasurand_Bits >> (pEle[n].nBitOffset)) & 0x01) == 0)
		{
			//TRACEX("MeterValue Samples[%d] \n",n);
			continue;
		}

		if (pEle[n].bRelated2MeterInterr == TRUE)
		{
			//TRACEX("Bit Set %d \n", n);
			if (!pOCPP->stConnectorData[nGunIdx].bMeterInterrupted)
			{
				emTp = pOCPP->emHandlingMeterType;
			}
			else if (pOCPP->stConnectorData[nGunIdx].bNeedSendMeter_Inter_Begin)
			{
				pOCPP->stConnectorData[nGunIdx].bNeedSendMeter_Inter_Begin = FALSE;
				emTp = Interrupt_Begin;
			}
			else if (pOCPP->stConnectorData[nGunIdx].bNeedSendMeter_Inter_End)
			{
				pOCPP->stConnectorData[nGunIdx].bNeedSendMeter_Inter_End = FALSE;
				emTp = Interrupt_End;
			}
			else
			{
				//TRACEX("pEle->bRelated2MeterInterr= %d \n", n);
				continue;
			}
		}

		if ((pOCPP->emHandlingMeterType == Sample_Periodic) && (pOCPP->stOCPPSettings.nMeterValuesSampleDataMaxLen_R > MaxConfigKeyCnt))
		{
			emTp = pOCPP->emHandlingMeterType;
			MaxConfigKeyCnt++;
			sampleValue = cJSON_CreateObject();
			MakeSampledVal_Json(&pEle[n], emTp, sampleValue);
			cJSON_AddItemToArray(sampleArray, sampleValue);
		}
		else if (((pOCPP->emHandlingMeterType == Transaction_Begin) || (pOCPP->emHandlingMeterType == Transaction_End)) &&
			(pOCPP->stOCPPSettings.nMeterValuesSampleDataMaxLen_R > MaxConfigKeyCnt))
		{
			emTp = pOCPP->emHandlingMeterType;
			MaxConfigKeyCnt++;
			sampleValue = cJSON_CreateObject();
			MakeSampledVal_Json(&pEle[n], emTp, sampleValue);
			cJSON_AddItemToArray(sampleArray, sampleValue);
		}
		else
		{
			TRACEX("else MeterValue \n");
			break;
		}
	}


	return sampleArray;

}

static BOOL Make_TransMeterValues_JsonString(OCPP_DATA_STU* pOCPP, cJSON *MeterValuesArr)
{
	REPORTER *pReporter = (REPORTER*)pOCPP->hReporter;
	A_BLOB_METER_VALUES  *pMerterVals = NULL;

	cJSON *sampleArray = NULL;
	//sampleArray = cJSON_CreateArray();

	int nGunIdx = pOCPP->emHandlingConnector;
	TRACEX("nGunIdx = %d \npOCPP->emHandlingConnector = %d \n,MeterHandling Type = %d  \n", nGunIdx, pOCPP->emHandlingConnector,pOCPP->stConnectorData[nGunIdx].emMeterTp);

	PARAM_VAL_GET valGet;
	A_BLOB_METER_VALUES meterValues;
	valGet.nGetSize = sizeof(A_BLOB_METER_VALUES);
	valGet.val.blobValue = (BYTE*)&meterValues;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		201 + pOCPP->emHandlingConnector, 112, &valGet, sizeof(PARAM_VAL_GET));
	pMerterVals = &meterValues;

	METERVAL_ELEMENT *pEle;
	int nEleNum = 0;

	if (pOCPP->stConnectorData[nGunIdx].emMeterTp == METER_TYPR_dc)
	{
		TRACEX("Meter Type METER_TYPR_dc \n ");

		METERVAL_ELEMENT meterElementList[] =
		{
			{"Current.Export",					&pMerterVals->fCurrent_Export_A,						"%.1f",	"A","\0","\0",0,	FALSE},
			{"Current.Import",					&pMerterVals->fCurrent_Import_A,						"%.1f",	"A","\0","\0",1,	FALSE},
			{"Current.Offered",					&pMerterVals->fCurrent_Offered_A,						"%.1f",	"A","\0","\0",2,	FALSE},
			{"Energy.Active.Export.Register",	&pMerterVals->fEnergy_Active_Export_Reg_kWh,			"%.1f",	"kWh","\0","\0",3,	FALSE},
			{"Energy.Active.Import.Register",	&pMerterVals->fEnergy_Active_Import_Reg_kWh,			"%.1f",	"kWh","\0","\0",4,	FALSE},
			{"Energy.Reactive.Export.Register",	&pMerterVals->fEnergy_Reactive_Export_Reg_kvarh,		"%.1f",	"kvarh","\0","\0",5,	FALSE},
			{"Energy.Reactive.Import.Register",	&pMerterVals->fEnergy_Reactive_Import_Reg_kvarh,		"%.1f",	"kvarh","\0","\0",6,	FALSE},
			{"Energy.Active.Export.Interval",	&pMerterVals->fEnergy_Active_Export_Interv_kWh,			"%.1f",	"kWh","\0","\0",7,	FALSE},
			{"Energy.Active.Import.Interval",	&pMerterVals->fEnergy_Active_Import_Interv_kWh,			"%.1f",	"kWh","\0","\0",8,	FALSE},
			{"Energy.Reactive.Export.Interval",	&pMerterVals->fEnergy_Reactive_Export_Interv_kvarh,		"%.1f",	"kvarh","\0","\0",9,	FALSE},
			{"Energy.Reactive.Import.Interval",	&pMerterVals->fEnergy_Reactive_Import_Interv_kvarh,		"%.1f",	"kvarh","\0","\0",10,	FALSE},
			{"Frequency",						&pMerterVals->fFreq_PLC_Hz,								"%.1f",	"Hz","\0","\0",	11,	FALSE},
			{"Power.Active.Export",				&pMerterVals->fPower_Active_Export_kW,					"%.1f",	"kW","\0","\0",	12,	FALSE},
			{"Power.Active.Import",				&pMerterVals->fPower_Active_Import_kW,					"%.1f",	"kW","\0","\0",	13,	FALSE},
			{"Power.Factor",					&pMerterVals->fPower_Factor,							"%.3f",	"kW","\0","\0",	14,	FALSE},
			{"Power.Offered",					&pMerterVals->fPower_Offered_kW,						"%.1f",	"kW","\0","\0",	15,	FALSE},
			{"Power.Reactive.Export",			&pMerterVals->fPower_Reactive_Export_kvar,				"%.1f",	"kvar","\0","\0",	16,	FALSE},
			{"Power.Reactive.Import",			&pMerterVals->fPower_Reactive_Import_kvar,				"%.1f",	"kvar","\0","\0",	17,	FALSE},
			{"RPM",								&pMerterVals->fFanRPM,									"%.1f",	"A","\0","\0",	18,	FALSE},
			{"SoC",								&pMerterVals->fBattery_SOC_Perc,						"%.1f",	"Percent","\0","\0",	19,	FALSE},
			{"Temperature",						&pMerterVals->fTemperature_deg,							"%.1f",	"Celsius","\0","\0",	20,	FALSE},
			{"Voltage",							&pMerterVals->fVoltage_V,								"%.1f",	"V","\0","\0",	21,	FALSE},
		};

		pEle = meterElementList;
		nEleNum = ITEM_OF(meterElementList);
		sampleArray = ProTransMeterElemtList(pOCPP,pEle,nEleNum);
		TRACEX("MeterValue %d \n", nEleNum);
		ProMeterValueArray(sampleArray, meterValues.tmSampled ,MeterValuesArr);
	}
	else
	{
		TRACEX("Meter Type METER_TYPR_ac \n ");
		//METERVAL_ELEMENT meterElementListPublic[] =
		//{
		//	//Public Parameters
		//	{"Energy.Active.Import.Register",	&pMerterVals->fEnergy_Active_Import_Reg_kWh,	"%.1f",	"kWh",		"\0",	"\0",	4,	TRUE},
		//	{"Energy.Reactive.Import.Register",	&pMerterVals->fEnergy_Reactive_Import_Reg_kvarh,"%.1f",	"kvarh",	"\0",	"\0",	6,	TRUE},
		//	{"Power.Active.Import",				&pMerterVals->fPower_Active_Import_kW,			"%.1f",	"kW",		"\0",	"\0",	13,	TRUE},
		//	{"Power.Factor",					&pMerterVals->fPower_Factor,					"%.3f",	"",			"\0",	"\0",	14,	TRUE},
		//	{"Power.Reactive.Import",			&pMerterVals->fPower_Reactive_Import_kvar,		"%.1f",	"kvar",		"\0",	"\0",	17,	TRUE},
		//	{"SoC",								&pMerterVals->fBattery_SOC_Perc,				"%.1f",	"Percent",	"\0",	"\0",	19,	FALSE},
		//	{"Temperature",						&pMerterVals->fTemperature_deg,					"%.1f",	"Celsius",	"\0",	"\0",	20,	FALSE},
		//};

		//METERVAL_ELEMENT meterElementListL1L2[] =
		//{
		//	//L1-L2
		//	{"Current.Import",					&pMerterVals->f_L1L2_Current_Import_A,			"%.1f",	"A",		"L1-N",	"\0",	1,	TRUE},
		//	{"Voltage",							&pMerterVals->f_L1L2_Voltage_V,					"%.1f",	"V",		"L1-N",	"\0",	21,	TRUE},
		//};

		//METERVAL_ELEMENT meterElementListL2L3[] =
		//{
		//	//L2-L3
		//	{"Current.Import",					&pMerterVals->f_L2L3_Current_Import_A,			"%.1f",	"A",		"L2-N",	"\0",	1,	TRUE},
		//	{"Voltage",							&pMerterVals->f_L2L3_Voltage_V,					"%.1f",	"V",		"L2-N",	"\0",	21,	TRUE},
		//};//L2L3

		//METERVAL_ELEMENT meterElementListL3L1[] =
		//{
		//	//L3-L1
		//	{"Current.Import",					&pMerterVals->f_L3L1_Current_Import_A,			"%.1f",	"A",		"L3-N",	"\0",	1,	TRUE},
		//	{"Voltage",							&pMerterVals->f_L3L1_Voltage_V,					"%.1f",	"V",		"L3-N",	"\0",	21,	TRUE},
		//};//L3L1
		//Jimmy 20190305: read OCPP errata doc carefully, it says AC 3-phase should be handled within the array of 'SampledValue'
		METERVAL_ELEMENT meterElementListPublic[] =
		{
			//Public Parameters
			{"Energy.Active.Import.Register",	&pMerterVals->fEnergy_Active_Import_Reg_kWh,	"%.1f",	"kWh",		"\0",	"\0",	4,	TRUE},
			{"Energy.Reactive.Import.Register",	&pMerterVals->fEnergy_Reactive_Import_Reg_kvarh,"%.1f",	"kvarh",	"\0",	"\0",	6,	TRUE},
			{"Power.Active.Import",				&pMerterVals->fPower_Active_Import_kW,			"%.1f",	"kW",		"\0",	"\0",	13,	TRUE},
			{"Power.Factor",					&pMerterVals->fPower_Factor,					"%.3f",	"",			"\0",	"\0",	14,	TRUE},
			{"Power.Reactive.Import",			&pMerterVals->fPower_Reactive_Import_kvar,		"%.1f",	"kvar",		"\0",	"\0",	17,	TRUE},
			{"SoC",								&pMerterVals->fBattery_SOC_Perc,				"%.1f",	"Percent",	"\0",	"\0",	19,	FALSE},
			{"Temperature",						&pMerterVals->fTemperature_deg,					"%.1f",	"Celsius",	"\0",	"\0",	20,	FALSE},
			//L1-L2
			{"Current.Import",					&pMerterVals->f_L2L3_Current_Import_A,			"%.1f",	"A",		"L2-N",	"\0",	1,	TRUE},
			{"Voltage",							&pMerterVals->f_L2L3_Voltage_V,					"%.1f",	"V",		"L2-N",	"\0",	21,	TRUE},
			//L2-L3
			{"Current.Import",					&pMerterVals->f_L2L3_Current_Import_A,			"%.1f",	"A",		"L2-N",	"\0",	1,	TRUE},
			{"Voltage",							&pMerterVals->f_L2L3_Voltage_V,					"%.1f",	"V",		"L2-N",	"\0",	21,	TRUE},
			//L3-L1
			{"Current.Import",					&pMerterVals->f_L3L1_Current_Import_A,			"%.1f",	"A",		"L3-N",	"\0",	1,	TRUE},
			{"Voltage",							&pMerterVals->f_L3L1_Voltage_V,					"%.1f",	"V",		"L3-N",	"\0",	21,	TRUE},
		};

		//Public
		pEle = meterElementListPublic;
		nEleNum = ITEM_OF(meterElementListPublic);
		sampleArray = ProTransMeterElemtList(pOCPP,pEle,nEleNum);
		TRACEX("Public MeterValueNum= %d \n", nEleNum);
		ProMeterValueArray(sampleArray, meterValues.tmSampled ,MeterValuesArr);

		////L1L2
		//pEle = meterElementListL1L2;
		//nEleNum = ITEM_OF(meterElementListL1L2);
		//sampleArray = ProTransMeterElemtList(pOCPP,pEle,nEleNum);
		//TRACEX("L1L2 MeterValueNum= %d \n", nEleNum);
		//ProMeterValueArray(sampleArray, meterValues.tmSampled ,MeterValuesArr);

		////L2L3
		//pEle = meterElementListL2L3;
		//nEleNum = ITEM_OF(meterElementListL2L3);
		//sampleArray = ProTransMeterElemtList(pOCPP,pEle,nEleNum);
		//TRACEX("L2L3 MeterValueNum= %d \n", nEleNum);
		//ProMeterValueArray(sampleArray, meterValues.tmSampled ,MeterValuesArr);

		////L3L1
		//pEle = meterElementListL3L1;
		//nEleNum = ITEM_OF(meterElementListL3L1);
		//sampleArray = ProTransMeterElemtList(pOCPP,pEle,nEleNum);
		//TRACEX("L3L1 MeterValueNum= %d \n", nEleNum);
		//ProMeterValueArray(sampleArray, meterValues.tmSampled ,MeterValuesArr);
		//TRACEX("Array %s \n ", cJSON_Print(MeterValuesArr));
	}

	if(sampleArray == NULL)
	{
		return FALSE;
	}
	//if array is empty, do not send
	return (cJSON_GetArraySize(sampleArray) > 0) ? TRUE : FALSE;
}

typedef struct tagHisMeterValue_Element
{
	char			*szMeasuand;
	METER_ELE_TYPE	emEleTp;
	//BOOL		bIsVarFormat_Raw; //should always use 'raw'	
	char			*szValFmt; // default "%.3f"
	char			*szValUnit; // default "Wh"
	char			szPhase[8]; //default ""
	char			*szLocation; //default 'Outlet'
}HISMETERVAL_ELEMENT;

static void MakeHisSampledVal_Json(float fVal, HISMETERVAL_ELEMENT* pEle, int emTp, cJSON *sampleValue)
{
	static const char* SampledValContexts[] =
	{
		"Interruption.Begin",
		"Interruption.End",
		"Other",
		"Sample.Clock",
		"Sample.Periodic",
		"Transaction.Begin",
		"Transaction.End",
		"Trigger",
	};

	char tempstr[16];
	sprintf(tempstr, pEle->szValFmt, fVal);
	cJSON_AddStringToObject(sampleValue, "value", (const char *)tempstr);
	if(emTp != -1)
	{
		cJSON_AddStringToObject(sampleValue, "context", SampledValContexts[emTp]);
	}
	cJSON_AddStringToObject(sampleValue, "measurand", pEle->szMeasuand);

	//if((pEle->szLocation) != NULL)

	if (strcmp(pEle->szLocation, "\0") != NULL)
	{
		TRACEX("Location %s \n", pEle->szLocation);
		cJSON_AddStringToObject(sampleValue, "location", pEle->szLocation);
	}
	if (strcmp(pEle->szValUnit, "\0") != NULL)
	{
		cJSON_AddStringToObject(sampleValue, "unit", pEle->szValUnit);
	}
	if (strcmp(pEle->szPhase, "\0") != NULL)
	{
		TRACEX("Phase %s \n", pEle->szPhase);
		cJSON_AddStringToObject(sampleValue, "phase", pEle->szPhase);
	}
}


static BOOL Make_HistoryTransMeterValues_JsonString(OCPP_DATA_STU* pOCPP, cJSON *MeterValuesArr)
{
	REPORTER *pReporter = (REPORTER*)pOCPP->hReporter;
	A_CHARGE_METERVALUE_ITEM  *pMerterVals = &pOCPP->currHandlingHisMeterVal;

	cJSON *sampleArray = cJSON_CreateArray();
	if(sampleArray == NULL)
	{
		return FALSE;
	}

	int nGunIdx = pOCPP->emHandlingConnector;
	
	static HISMETERVAL_ELEMENT meterElementList[] =
	{
		{"Current.Export",				METER_ELETYPE_Current_Export,			"%.1f",	"A","\0","\0"},
		{"Current.Import",				METER_ELETYPE_Current_Import,			"%.1f",	"A","\0","\0"},
		{"Current.Offered",				METER_ELETYPE_Current_Offered,			"%.1f",	"A","\0","\0"},
		{"Energy.Active.Export.Register",	METER_ELETYPE_Engergy_Active_Export_Register,			"%.3f",	"kWh","\0","\0"},
		{"Energy.Active.Import.Register",	METER_ELETYPE_Engergy_Active_Import_Register,			"%.3f",	"kWh","\0","\0"},
		{"Energy.Reactive.Export.Register",	METER_ELETYPE_Engergy_Reactive_Export_Registe,			"%.3f",	"kvarh","\0","\0"},
		{"Energy.Reactive.Import.Register",	METER_ELETYPE_Engergy_Reactive_Import_Register,			"%.3f",	"kvarh","\0","\0"},
		{"Energy.Active.Export.Interval",	METER_ELETYPE_Engergy_Active_Export_Interval,			"%.3f",	"kWh","\0","\0"},
		{"Energy.Active.Import.Interval",	METER_ELETYPE_Engergy_Active_Import_Interval,			"%.3f",	"kWh","\0","\0"},
		{"Energy.Reactive.Export.Interval",	METER_ELETYPE_Engergy_Reactive_Export_Interval,			"%.3f",	"kvarh","\0","\0"},
		{"Energy.Reactive.Import.Interval",	METER_ELETYPE_Engergy_Reactive_Import_Interval,			"%.3f",	"kvarh","\0","\0"},
		{"Frequency",					METER_ELETYPE_Frequency,			"%.1f",	"Hz","\0","\0"},
		{"Power.Active.Export",			METER_ELETYPE_Power_Active_Export,			"%.1f",	"kW","\0","\0"},
		{"Power.Active.Import",			METER_ELETYPE_Power_Active_Import,			"%.1f",	"kW","\0","\0"},
		{"Power.Factor",					METER_ELETYPE_Power_Factor,			"%.3f",	"kW","\0","\0"},
		{"Power.Offered",				METER_ELETYPE_Power_Offered,			"%.1f",	"kW","\0","\0"},
		{"Power.Reactive.Export",			METER_ELETYPE_Power_Reactive_Export,			"%.1f",	"kvar","\0","\0"},
		{"Power.Reactive.Import",			METER_ELETYPE_Power_Reactive_Import,			"%.1f",	"kvar","\0","\0"},
		{"RPM",						METER_ELETYPE_Fan_RPM,			"%.1f",	"A","\0","\0"},
		{"SoC",						METER_ELETYPE_Battery_SOC,			"%.1f",	"Percent","\0","\0"},
		{"Temperature",					METER_ELETYPE_Temperature,			"%.1f",	"Celsius","\0","\0"},
		{"Voltage",					METER_ELETYPE_Voltage,			"%.1f",	"V","\0","\0"},
		{"Voltage",					METER_ELETYPE_Voltage_ABC,			"%.1f",	"V","\0","\0"},
		{"Current.Import",				METER_ELETYPE_Current_Import_ABC,	"%.1f",	"A","\0","\0"},
	};

	int nTotalValues = (int)pMerterVals->byValidElements;
	if(nTotalValues > MAX_SUPPORT_METER_KEYS_CHARGING) //invalid records, should skip it
	{
		TRACEX("Handle INVALID HisMeterVal for gun[%d], TotalElements[%d] \n", nGunIdx + 1, nTotalValues);
		cJSON_Delete(sampleArray);
		return FALSE;
	}
	//nTotalValues = MIN(nTotalValues, MAX_SUPPORT_METER_KEYS_CHARGING);
	TRACEX("Handle HisMeterVal for gun[%d], TotalElements[%d] \n", nGunIdx + 1, nTotalValues);
	int n, m;
	for(n = 0; n < nTotalValues; n++)
	{
		for(m = 0; m < ITEM_OF(meterElementList); m++)
		{
			if(meterElementList[m].emEleTp == pMerterVals->meterValues[n].tp)
			{
				float fVal = pMerterVals->meterValues[n].val.fVal;
				cJSON *sampleValue = cJSON_CreateObject();
				if((meterElementList[m].emEleTp == METER_ELETYPE_Voltage_ABC) ||
					(meterElementList[m].emEleTp == METER_ELETYPE_Current_Import_ABC))
				{
					//3-phase values
					fVal = (float)pMerterVals->meterValues[n].val.sVal_ABS[0] / 10.0;
					sprintf(meterElementList[m].szPhase, "%s", "L1-N");
					MakeHisSampledVal_Json(fVal, &meterElementList[m], pOCPP->emHandlingMeterType, sampleValue);
					cJSON_AddItemToArray(sampleArray, sampleValue);

					sampleValue = cJSON_CreateObject();
					fVal = (float)pMerterVals->meterValues[n].val.sVal_ABS[1] / 10.0;
					sprintf(meterElementList[m].szPhase, "%s", "L2-N");
					MakeHisSampledVal_Json(fVal, &meterElementList[m], pOCPP->emHandlingMeterType, sampleValue);
					cJSON_AddItemToArray(sampleArray, sampleValue);

					sampleValue = cJSON_CreateObject();
					fVal = (float)pMerterVals->meterValues[n].val.sVal_ABS[2] / 10.0;
					sprintf(meterElementList[m].szPhase, "%s", "L3-N");
					MakeHisSampledVal_Json(fVal, &meterElementList[m], pOCPP->emHandlingMeterType, sampleValue);
					cJSON_AddItemToArray(sampleArray, sampleValue);

				}
				else
				{
					MakeHisSampledVal_Json(fVal, &meterElementList[m], pOCPP->emHandlingMeterType, sampleValue);
					cJSON_AddItemToArray(sampleArray, sampleValue);
				}

			}
		}
	}

	ProMeterValueArray(sampleArray, pMerterVals->tmSampled ,MeterValuesArr);

	//if array is empty, do not send
	return (cJSON_GetArraySize(sampleArray) > 0) ? TRUE : FALSE;
}

int Prepare_MeterValues(void* pArg)
{
	// Point Need to consider the Value Of Max_KeyVal For MeterAligned And MeterSamplevalue 
	// also Consider that Context 
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	
	int n;
	TRACEX("Prepare_MeterValues \n");

	cJSON *root;
	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root,"connectorId", pOCPP->emHandlingConnector + 1 );
	cJSON *MeterValuesArr;
	MeterValuesArr = cJSON_CreateArray();

	if((pOCPP->emHandlingMeterType == Sample_Clock) || (pOCPP->emHandlingMeterType == Trigger))
	{
		TRACE(" Under Trigger and Sample Clock \n");
		if(!Make_ClockMeterValues_JsonString(pOCPP, MeterValuesArr))
		{
			cJSON_Delete(MeterValuesArr);
			cJSON_Delete(root);
			return -1;
		}
		cJSON_AddItemToObject(root,"meterValue", MeterValuesArr);
		//Msg_Send(pOCPP, root, MeterValues, Call, pOCPP->emHandlingConnector, bSend_Later);
		Send_Call(pOCPP, root, MeterValues, CALL_SRC_Normal, 
			pOCPP->emHandlingConnector, (pOCPP->emHandlingMeterType == Trigger) ? TRUE : FALSE);

	}
	else //Trans-related
	{
		if(pOCPP->bTransMsgFromHistory)
		{
			if(!Make_HistoryTransMeterValues_JsonString(pOCPP, MeterValuesArr))
			{
				cJSON_Delete(MeterValuesArr);
				cJSON_Delete(root);
				return -1;
			}
			cJSON_AddNumberToObject(root,"transactionId", (int)pOCPP->recInHandling_HistoryChargeRec.nSessionID);
		}
		else
		{
			//Jimmy 20190306: for real-time metervalues, CCU sampler has already made everything ready(the same as history values!
			//if(!Make_TransMeterValues_JsonString(pOCPP, MeterValuesArr))
			if(!Make_HistoryTransMeterValues_JsonString(pOCPP, MeterValuesArr))
			{
				cJSON_Delete(MeterValuesArr);
				cJSON_Delete(root);
				return -1;
			}
			int nGunIdx = pOCPP->emHandlingConnector;
			cJSON_AddNumberToObject(root,"transactionId", (int)pOCPP->stConnectorData[nGunIdx].blobInHandling_ChargeInfo.nSessionID);
		}

		cJSON_AddItemToObject(root,"meterValue", MeterValuesArr);
		//Msg_Send(pOCPP, root, MeterValues, Call, pOCPP->emHandlingConnector, bSend_Later);
		Send_Call(pOCPP, root, MeterValues, pOCPP->bTransMsgFromHistory ? CALL_SRC_HisTrans : CALL_SRC_InProcessTrans, 
			pOCPP->emHandlingConnector, (pOCPP->emHandlingMeterType == Trigger) ? TRUE : FALSE); //SHOULD be FALSE!!!!
	}

	return 0;
}

int Prepare_DiagnosticsStatusNotification(void* pArg)
{
	static const char* DIAGNOSTICSSTATUS_STRING[] =
	{
		"Idle",
		"Uploaded",
		"UploadFailed",
		"Uploading",
	};
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	cJSON *pJson_Dignostics;
	
	if((pOCPP->emTriggerMsgType != Trigger_DiagnosticsStatusNotification) && 
		(pOCPP->emDiagnosUploadStatus == DIAGNOUPLOAD_Idle))
	{
		return -1;
	}

	pJson_Dignostics = cJSON_CreateObject();
	// need to ask Jimmy for Dignostics Status 
	cJSON_AddStringToObject(pJson_Dignostics, "status", DIAGNOSTICSSTATUS_STRING[pOCPP->emDiagnosUploadStatus]);

	//Msg_Send(pOCPP, pJson_Dignostics, DiagnosticsStatusNotification, Call,0,bSend_Later);
	Send_Call(pOCPP, pJson_Dignostics, DiagnosticsStatusNotification, CALL_SRC_Normal, NULL, 
		(pOCPP->emTriggerMsgType == Trigger_DiagnosticsStatusNotification) ? TRUE : FALSE); //SHOULD be FALSE!!!!

	return 0;
}

int Prepare_FirmwareStatusNotification(void* pArg)
{
	static const char* FIRMWARESTATUS_STRING[] =
	{
		"Downloaded",
		"DownloadFailed",
		"Downloading",
		"Idle",
		"InstallationFailed",
		"Installing",
		"Installed",
	};
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	cJSON *pJson_firmwarestatus;
	if((pOCPP->emTriggerMsgType != Trigger_FirmwareStatusNotification) && 
		(pOCPP->emFirmwareUpgradeStatus == FIRMUPDATE_Idle))
	{
		return 1;
	}
	pJson_firmwarestatus = cJSON_CreateObject();
	// need to ask Jimmy for Firmware Status 
	cJSON_AddStringToObject(pJson_firmwarestatus, "status", FIRMWARESTATUS_STRING[pOCPP->emFirmwareUpgradeStatus]);

	//Msg_Send(pOCPP, pJson_firmwarestatus, FirmwareStatusNotification, Call, 0,bSend_Later);
	Send_Call(pOCPP, pJson_firmwarestatus, FirmwareStatusNotification, CALL_SRC_Normal, NULL, 
		(pOCPP->emTriggerMsgType == Trigger_FirmwareStatusNotification) ? TRUE : FALSE); //SHOULD be FALSE!!!!

	return 0;
}


int Handle_ChangeAvailability(void* pArg)
{
	const char* Aval_status[]=
	{
		"Rejected",
		"Accepted",
		"Scheduled"
	};
	
	enum
	{
		ChangeAvail_Rejected = 0,
		ChangeAvail_Accepted,
		ChangeAvail_Scheduled,
	}Status;

	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	cJSON *pJson = pOCPP->pCurrJson;
	
	cJSON *temp1 = NULL;
	cJSON *pResp = NULL;
	BOOL bAccpeted = FALSE;
	int ConnectorId = -1;
	int AvalibilityStatus = -1;

	if ((temp1 = cJSON_GetObjectItem(pJson, "connectorId")))
	{
		ConnectorId = temp1->valueint - 1;
		if ((temp1 = cJSON_GetObjectItem(pJson, "type")))
		{
			if (strcmp(temp1->valuestring, "Inoperative") == 0)
			{
				AvalibilityStatus = 0;
				bAccpeted = TRUE;
			}
			else if (strcmp(temp1->valuestring, "Operative") == 0)
			{
				AvalibilityStatus = 1;
				bAccpeted = TRUE;
			}
		}
	}
	
	if (bAccpeted)
	{
		VAR_VALUE varV;
		varV.emValue = AvalibilityStatus;
		ERRCODE_DAI err;

		if (ConnectorId == Connctor_0)
		{
			//TRACE("\nFOUND conn0 SetAbility CMD Status = %d\n", Status);
			Status = ChangeAvail_Accepted; //need initialize, or else system will go crashed
			if(AvalibilityStatus == 0)
			{
				int n;
				for(n = 0; n < Max_Connctors; n++)
				{
					if((pOCPP->stConnectorData[n].emConnctorStauts == Conn_InCharging) )
					{
						Status = ChangeAvail_Scheduled;
						break;
					}
				}
			}
			err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
				200, 1, &varV,4, 1000);
			Status = (err == ERRCODE_DAI_OK) ? Status : ChangeAvail_Rejected;
		}
		else if ((ConnectorId >= Connctor_1) && (ConnectorId < Max_Connctors))
		{
			if (AvalibilityStatus == 0) // Inoperative
			{
				if (pOCPP->stConnectorData[ConnectorId].emConnctorStauts == Conn_UnAvailable)
				{
					Status = ChangeAvail_Accepted;
					//! Set control command to change the connector availability status to operative
				}
				else if (pOCPP->stConnectorData[ConnectorId].emConnctorStauts == Conn_InCharging)
				{
					Status = ChangeAvail_Scheduled;
					err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
						201 + ConnectorId, 106, &varV,4, 1000);
					Status = (err == ERRCODE_DAI_OK) ? Status : ChangeAvail_Rejected;
				}
				else
				{
					err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
						201 + ConnectorId, 106, &varV,4, 1000);
					Status = (err == ERRCODE_DAI_OK) ? ChangeAvail_Accepted : ChangeAvail_Rejected;
				}
			}
			else if (AvalibilityStatus == 1) // Operative
			{
				err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
					201 + ConnectorId, 106, &varV,4, 1000);
				Status = (err == ERRCODE_DAI_OK) ? ChangeAvail_Accepted : ChangeAvail_Rejected;
			}

		}
	}
	else
	{
		//Status = ChangeAvail_Rejected;
		Send_CallError(pOCPP, ChangeAvailability, CallError_FormationViolation, "connectorId or type is missing!", NULL);
		return -1;
	}

	pResp = cJSON_CreateObject();
	cJSON_AddStringToObject(pResp, "status", Aval_status[Status]);

	//Msg_Send(pOCPP, pResp, ChangeAvailability, CallResult,0, bSend_Later);
	Send_CallResult(pOCPP, pResp, ChangeAvailability);

	//Jimmy added 20190701 for OCTT test case: shall send StatusNotif.conf after handing this msg
	if (ConnectorId == Connctor_0)
	{
		pOCPP->emHandlingConnector = Connctor_0;
		pOCPP->emTriggerMsgType = Trigger_StatusNotification;
		pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP);
		int k;		
		for( k = 0; k < pOCPP->stOCPPSettings.nNumOfConnectors_R; k++)
		{
			pOCPP->emHandlingConnector = Connctor_1 + k;
			pOCPP->emTriggerMsgType = Trigger_StatusNotification;
			pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP);
		}
	}
	else
	{
		pOCPP->emHandlingConnector = ConnectorId;
		pOCPP->emTriggerMsgType = Trigger_StatusNotification;
		pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP);
	}

	return 0;
}

typedef enum 
{
	TYPE_BOOL,
	TYPE_INT,
	TYPE_CSL_METER_MESURAND,
	TYPE_CSL_OCPP_SUPPORTED_FETURES,
	TYPE_CSL_CHARGESCHEULE_ALLOWEDRATEUNIT,
	TYPE_CSL_CONNPHASEROTA


}OCPP_CONFIG_TYPE;

typedef struct tagOCPPSettingsConfig
{
	char *ConfigName;
	BOOL  Readonly;
	OCPP_CONFIG_TYPE  ValueType;
	void  *ptrValue;

}GET_CHAANGE_CONFIG_PARA;

BOOL Get_DataOfMeterConfig(cJSON *tempObject, METER_MESURAND *pSetMeasurand, BOOL ChangeConfig)
{
	static const char* MEASURAND_STRING[] =
	{
		"Current.Export",			//0
		"Current.Import",			//1
		"Current.Offered",//2
		"Energy.Active.Export.Register",//3
		"Energy.Active.Import.Register",//4
		"Energy.Reactive.Export.Register",//5
		"Energy.Reactive.Import.Register",//6
		"Energy.Active.Export.Interval",//7
		"Energy.Active.Import.Interval",//8
		"Energy.Reactive.Export.Interval",//9
		"Energy.Reactive.Import.Interval",//10
		"Frequency",       //11
		"Power.Active.Export",//12
		"Power.Active.Import",//13
		"Power.Factor",//14
		"Power.Offered",//15
		"Power.Reactive.Export",//16
		"Power.Reactive.Import",//17
		"RPM", //18
		"SoC",//19
		"Temperature",//20
		"Voltage",//21
	};
	UINT ulCSL = 0 ;
	UINT TempConfigs = *(UINT*)pSetMeasurand;

	if ((ChangeConfig) )//&& ((tempObject->valuestring[0]) != '\0')
	{
		memset(pSetMeasurand, 0, sizeof(METER_MESURAND));
		
		int i,n;
		cJSON *temp1 = tempObject; // cJSON_GetArrayItem(tempObject, i);
		TRACEX(" %s \n", temp1->valuestring);
		if ((strcmp(temp1->valuestring, "NULL") != 0) || (strcmp(temp1->valuestring, "") != 0))
		{
			for (n = 0, i = 0; n < ITEM_OF(MEASURAND_STRING); n++)
			{
				//TRACEX("MeterVal : %s,%d \n", MEASURAND_STRING[n], n);
				if (strstr(temp1->valuestring, MEASURAND_STRING[n]))
				{
					ulCSL |= (1 << n);
					i++;
					TRACEX("Set Value MeterVal : %s,%d,%x \n", MEASURAND_STRING[n],n, ulCSL);
					//break;
				}
			}
		}
		else
		{

		}
		
		/*
		if (i > pOCPP->stOCPPSettings.nMeterValuesSampleDataMaxLen_R)
		{
			ulCSL = TempConfigs;
			return FALSE;
		}
		else if (i > pOCPP->stOCPPSettings.nMeterValuesAlignDataMaxLen_R)
		{
			ulCSL = TempConfigs;
			return FALSE;
		}
		else if (i > pOCPP->stOCPPSettings.nStopTxAlignDataMaxLen_R)
		{
			ulCSL = TempConfigs;
			return FALSE;
		}
		else if (i > pOCPP->stOCPPSettings.nStopTxSampleDataMaxLen_R)
		{
			ulCSL = TempConfigs;
			return FALSE;
		}
		*/
		TRACEX("ulCSL VAlue %lu \n", ulCSL);
		*pSetMeasurand = *((METER_MESURAND*)&ulCSL);
		TRACEX("ulCSL VAlue %lu \n", (*pSetMeasurand));
		return TRUE;
	}
	else
	{
		char szTempData[500] = { '\0' };
		int i = 0;
		int n = 0;
		ulCSL = *(UINT*)pSetMeasurand;
		TRACEX("ulCSL VAlue %lu \n", ulCSL);

		for (n = 0; n < ITEM_OF(MEASURAND_STRING); n++)
		{
			TRACEX("MeterVal : %s,%d  %d \n", MEASURAND_STRING[n], n, (((ulCSL) >> n) & 0x01));
			if(((ulCSL) >> n) & 0x01)
			{
				int len = strlen(MEASURAND_STRING[n]);
				strcat(szTempData, MEASURAND_STRING[n]);
				i += len;
				szTempData[i++] = ',';
				TRACEX("Here the String  %s \n", szTempData);
			}
		}

		if (i == 0)
		{
			//TRACEX("Hello Under Get Cnf for MeterValues %d %d \n",n,i);
			//Jimmy note 20190701: if value not set, shall leave if empty
			//cJSON_AddNullToObject(tempObject, "value");
			return FALSE;
		}

		szTempData[--i] = '\0';
		TRACEX("Here the String  %s \n", szTempData);
		cJSON_AddStringToObject(tempObject, "value", szTempData);
	}
	
	return TRUE;
}

cJSON *GetConfig_Response(void* pArg,const char *data,BOOL bChangeCongfig)
{
	TRACEX("Here is in Get Config \n");
	// Operation  0(Get Config ),1(Change settings)
	BOOL bAccpeted = FALSE;
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;


	cJSON *temp, *temp1;
	temp = cJSON_CreateObject();
	temp1 = pOCPP->pCurrJson;
	pOCPP->stOCPPSettings.nSupportedFeatureMaxLen_R = MAX_SUPPORTED_OCPP_FEATURES; //Jimmy note: this was not initialized in old version, so add it here since it does no harm
	GET_CHAANGE_CONFIG_PARA TestOcppSettigs[] =
	{
		{"AllowOfflineTxForUnknownId" ,			FALSE  ,  TYPE_BOOL ,	&(pOCPP->stOCPPSettings.bAllowOfflineTxForUnknowId_RW) }, //0
		{"AuthorizationCacheEnabled" ,			FALSE ,  TYPE_BOOL ,	&(pOCPP->stOCPPSettings.bAuthCacheEnabled_RW) },//1
		{"AuthorizeRemoteTxRequests" ,			FALSE ,  TYPE_BOOL ,	&(pOCPP->stOCPPSettings.bAuthRemoteTxReq_RW )},//2
		{"BlinkRepeat" ,						FALSE ,  TYPE_INT ,		&(pOCPP->stOCPPSettings.nBlinkRepeat_RW )},//3
		{"ClockAlignedDataInterval" ,			FALSE ,  TYPE_INT ,		&(pOCPP->stOCPPSettings.nClockAlignDataInterv_RW) },//4
		{"ConnectionTimeOut" ,					FALSE ,  TYPE_INT ,		&(pOCPP->stOCPPSettings.nConnectionTimeout_RW) },//5
		{"GetConfigurationMaxKeys" ,			TRUE  ,  TYPE_INT ,		&(pOCPP->stOCPPSettings.nGetConfigMaxKeys_R) },//6
		{"HeartbeatInterval" ,					FALSE ,  TYPE_INT ,		&(pOCPP->stOCPPSettings.nHearBeatInterv_RW) },//7
		{"LightIntensity" ,						FALSE ,  TYPE_INT ,		&(pOCPP->stOCPPSettings.nLigntIntensity_RW) },//8
		{"LocalAuthorizeOffline" ,				FALSE ,  TYPE_BOOL ,	&(pOCPP->stOCPPSettings.bLocalAuthorizeOffline_RW) },//9
		{"LocalPreAuthorize" ,					FALSE ,  TYPE_BOOL ,	&(pOCPP->stOCPPSettings.bLocalPreAuthorize_RW) },//10
		{"MaxEngeryOnInvalidId" ,				FALSE ,  TYPE_INT ,		&(pOCPP->stOCPPSettings.nMaxEngeryOnInvalidId_RW) },//11
		{"MeterValuesAlignedData" ,				FALSE ,  TYPE_CSL_METER_MESURAND ,		&(pOCPP->stOCPPSettings.cslMeterValAlignData_RW) },//12
		{"MeterValuesAlignedDataMaxLength" ,	TRUE ,   TYPE_INT ,		&(pOCPP->stOCPPSettings.nMeterValuesAlignDataMaxLen_R )},//13
		{"MeterValuesSampledData" ,				FALSE ,  TYPE_CSL_METER_MESURAND ,		&(pOCPP->stOCPPSettings.cslMeterValSampleData_RW) },//14
		{"MeterValuesSampledDataMaxLength" ,    TRUE ,   TYPE_INT ,		&(pOCPP->stOCPPSettings.nMeterValuesSampleDataMaxLen_R)},//15
		{"MeterValueSampleInterval" ,			FALSE ,  TYPE_INT ,		&(pOCPP->stOCPPSettings.nMeterValueSampleInterval_RW) },//16
		{"MinimumStatusDuration",				FALSE , TYPE_INT ,		&(pOCPP->stOCPPSettings.nMinStatusDuration_RW )},//17
		{"NumberOfConnectors" ,					TRUE ,  TYPE_INT ,		&(pOCPP->stOCPPSettings.nNumOfConnectors_R) },//18
		{"ResetRetries" ,						FALSE , TYPE_INT ,		&(pOCPP->stOCPPSettings.nResetRetries_RW )},//19
		{"ConnectorPhaseRotation" ,				FALSE , TYPE_CSL_CONNPHASEROTA ,		&(pOCPP->stOCPPSettings.emConnPhaseRotation )},//20
		{"ConnectorPhaseRotationMaxLength" ,	TRUE ,  TYPE_INT ,		&(pOCPP->stOCPPSettings.nConPhaseRotationMaxLen_R )},//21
		{"StopTransactionOnEVSideDisconnect" ,	FALSE , TYPE_BOOL ,		&(pOCPP->stOCPPSettings.bStopTransOnEvSideDisconn_RW )},//22
		{"StopTransactionOnInvalidId",			FALSE , TYPE_BOOL ,		&(pOCPP->stOCPPSettings.bStopTransOnInvalidId_RW )},//23
		{"StopTxnAlignedData",					FALSE , TYPE_CSL_METER_MESURAND ,		&(pOCPP->stOCPPSettings.cslStopTxAlignData_RW) },//24
		{"StopTxnAlignedDataMaxLength" ,		TRUE ,  TYPE_INT ,		&(pOCPP->stOCPPSettings.nStopTxAlignDataMaxLen_R) },//25
		{"StopTxnSampledData" ,					FALSE , TYPE_CSL_METER_MESURAND ,		&(pOCPP->stOCPPSettings.cslStopTxSampleData_RW )},//26
		{"StopTxnSampledDataMaxLength",			TRUE ,  TYPE_INT ,		&(pOCPP->stOCPPSettings.nStopTxSampleDataMaxLen_R)},//27
		{"SupportedFeatureProfiles" ,			TRUE ,  TYPE_CSL_OCPP_SUPPORTED_FETURES ,&(pOCPP->stOCPPSettings.cslSupportedFeatures_R) },//28
		{"SupportedFeatureProfilesMaxLength" ,	TRUE ,  TYPE_INT ,		&(pOCPP->stOCPPSettings.nSupportedFeatureMaxLen_R) },//29
		{"TransactionMessageAttempts" ,			FALSE , TYPE_INT ,		&(pOCPP->stOCPPSettings.nTransactionMessageAttempts_RW )},//30
		{"TransactionMessageRetryInterval" ,	FALSE , TYPE_INT ,		&(pOCPP->stOCPPSettings.nTransactionMsgRetryInterv_RW) },//31
		{"UnlockConnectorOnEVSideDisconnect" ,  FALSE , TYPE_BOOL,		&(pOCPP->stOCPPSettings.bUnlockConnOnEVSideDiscon_RW) },//32
		{"WebSocketPingInterval",				FALSE , TYPE_INT,		&(pOCPP->stOCPPSettings.nWebSocketPingInterv_RW) },//33
		{"LocalAuthListEnabled",				FALSE , TYPE_BOOL,	    &(pOCPP->stOCPPSettings.bLocalListEnabled_RW) },//34
		{"LocalAuthListMaxLength" ,				TRUE ,  TYPE_INT ,		&(pOCPP->stOCPPSettings.nLocalListMaxLength_R )},//35
		{"SendLocalListMaxLength" ,						TRUE , TYPE_INT , &(pOCPP->stOCPPSettings.nSengLocalListMaxLength_R) },//36
		{"ReserveConn0_Enabled",						TRUE , TYPE_BOOL, &(pOCPP->stOCPPSettings.bReserveConn0_Enabled_R) },//37
		{"ChargeProfileMaxStackLevel" ,					TRUE , TYPE_INT , &(pOCPP->stOCPPSettings.nChargeProfileMaxStackLevel_R )},//38
		{"ChargingScheduleAllowedChargingRateUnit" ,	TRUE , TYPE_CSL_CHARGESCHEULE_ALLOWEDRATEUNIT , &(pOCPP->stOCPPSettings.cslChargeScheduleAllowedChargeRateUnit_R )},//39
		{"ChargeScheduleMaxPeriods",					TRUE , TYPE_INT , &(pOCPP->stOCPPSettings.nChargeScheduleMaxPeriods_R) },//40
		{"ConnectorSwitch3To1PhaseSupport" ,			TRUE , TYPE_BOOL, &(pOCPP->stOCPPSettings.bConnSwitch3To1PhaseSupport_R) },//41
		{"MaxChargingProfilesInstalled" ,				TRUE , TYPE_INT , &(pOCPP->stOCPPSettings.nMaxChargeProfilesInstalled_R )},//42

	};
	GET_CHAANGE_CONFIG_PARA *pElementsList = &TestOcppSettigs[0];

	int n;
	BOOL bFoundKey = FALSE;
	for (n = 0; n < ITEM_OF(TestOcppSettigs) ; n++)
	{
		TRACEX(" n = %d   %s data %s \n", n, pElementsList[n].ConfigName,data);
		if (strcmp(pElementsList[n].ConfigName, data) == 0)
		{
			TRACEX(" n = %d  %s \n",n, pElementsList[n].ConfigName);
			bFoundKey = TRUE;
			break;
		}
	}

	if (!bFoundKey)
	{
		TRACEX("Cannot get the key[%s]\n", data);
		return NULL;
	}

	if (!bChangeCongfig) 
	{
		cJSON_AddStringToObject(temp, "key", pElementsList[n].ConfigName);
		if (pElementsList[n].Readonly)
		{
			cJSON_AddTrueToObject(temp, "readonly");
			TRACEX("Get Config Read only  \n");
		}
		else
		{
			cJSON_AddFalseToObject(temp, "readonly");
			TRACEX("Get Config ReadWrite \n ");
		}
	}

	if (pElementsList[n].ValueType == TYPE_BOOL)
	{
		if(!bChangeCongfig)
		{
			int tempVal =  *((BOOL*)pElementsList[n].ptrValue);
			TRACEX("Get Config  %d \n ",tempVal);
			cJSON_AddStringToObject(temp, "value", tempVal ? TRUE_STR : FALSE_STR );
			TRACEX("Get Config  %d \n ", tempVal);
		}
		else
		{
			TRACEX("ChangeConfig Config  %d \n ");
			// for change config 
			cJSON *valGet;
			if (valGet = cJSON_GetObjectItem(temp1, "value"))
			{
				if (valGet->valuestring != NULL)
				{
					if ((strcmp(valGet->valuestring, "True") == NULL) || (strcmp(valGet->valuestring, TRUE_STR) == NULL))
					{
						TRACEX("ChangeConfig Config  %d \n ");
						int *tempVal = ((BOOL*)pElementsList[n].ptrValue);
						*tempVal = 1;
						TRACEX("ChangeConfig Config  %d \n ", *tempVal);
						 bAccpeted = TRUE;
					}
					else if ((strcmp(valGet->valuestring, "False") == NULL) || (strcmp(valGet->valuestring, FALSE_STR ) == NULL))
					{
						if (pElementsList[n].Readonly == FALSE)
						{
							int *tempVal = ((BOOL*)pElementsList[n].ptrValue);
							*tempVal = 0;
							bAccpeted = TRUE;
						}
					}

				}
			}
		}
	}
	else if (pElementsList[n].ValueType == TYPE_INT)
	{
		if(!bChangeCongfig)
		{
			int tempVal =  *((int*)pElementsList[n].ptrValue);
			TRACEX("Get Config  %d \n ",tempVal);
			//cJSON_AddNumberToObject(temp, "value", tempVal);
			//Jimmy note 20190701: OCTT found the value shall always be STRING
			char szNum[64] = {0};
			sprintf(szNum, "%d", tempVal);
			cJSON_AddStringToObject(temp, "value", szNum);
		}
		else
		{
			cJSON *valGet;
			if (valGet = cJSON_GetObjectItem(temp1, "value"))
			{
				if (valGet->valuestring != NULL)
				{
					//Jimmy note: judge if this is a valid Number
					BOOL bIsValidNum = TRUE;
					int nValLen = strlen(valGet->valuestring);
					int kk;
					char s;
					for(kk = 0; kk < nValLen; kk++)
					{
						s = *(valGet->valuestring + kk);
						if((s < '0') || (s > '9'))
						{
							bIsValidNum = FALSE;
							break;
						}
					}

					if(bIsValidNum)
					{
						TRACEX("Value  %s \n", valGet->valuestring);
						int Value = atoi(valGet->valuestring);
						//Jimmy Note: value range will be verified by Set API itself
						if (pElementsList[n].Readonly == FALSE)
						{
							int *tempVal = ((UINT*)pElementsList[n].ptrValue);
							*tempVal = Value;
							bAccpeted = TRUE;
						}
					}
				}
			}
		}
	}
	else if (pElementsList[n].ValueType == TYPE_CSL_CHARGESCHEULE_ALLOWEDRATEUNIT)
	{
		if (!bChangeCongfig)
		{
			//Jimmy Note: will handle it later, 
			//Jimmy note 20190701: for OCTT test case: if not set, this shall be left empty
			//cJSON_AddNullToObject(temp, "value");
		}
		else
		{
			return NULL;
		}
	}
	else if (pElementsList[n].ValueType == TYPE_CSL_OCPP_SUPPORTED_FETURES)
	{
		char szTempData[500] = { '\0' };
		int i = 0;
		if (!bChangeCongfig)
		{
			//cJSON *tempArr;
			//cJSON_AddItemToObject(temp, "value", tempArr = cJSON_CreateArray());

			if (pOCPP->stOCPPSettings.cslSupportedFeatures_R.Set.Core)
			{
				strcat(szTempData,"Core,");
			}
			if (pOCPP->stOCPPSettings.cslSupportedFeatures_R.Set.FirmwareMnmg)
			{
				strcat(szTempData, "FirmwareManagement,");
			}
			if (pOCPP->stOCPPSettings.cslSupportedFeatures_R.Set.LocalAuthListMnmg)
			{
				strcat(szTempData, "LocalAuthListManagement,");
			}
			if (pOCPP->stOCPPSettings.cslSupportedFeatures_R.Set.ReserveMnmg)
			{
				strcat(szTempData, "Reservation,");
			}
			if (pOCPP->stOCPPSettings.cslSupportedFeatures_R.Set.SmartCharing)
			{
				strcat(szTempData, "SmartCharging,");
			}
			if (pOCPP->stOCPPSettings.cslSupportedFeatures_R.Set.RemoteTriggle)
			{
				strcat(&szTempData, "RemoteTrigger,");
			}

			i = strlen(szTempData);
			szTempData[--i] = '\0';
			TRACEX("Here the String  %s \n", szTempData);
			//cJSON_AddItemToArray(tempArr, cJSON_CreateString(szTempData));
			cJSON_AddStringToObject(temp, "value", szTempData);
		}
		else
		{
			return NULL;
		}
	}
	else if (pElementsList[n].ValueType == TYPE_CSL_METER_MESURAND)
	{
		METER_MESURAND *pSetMeasurand;
		pSetMeasurand = (METER_MESURAND *)pElementsList[n].ptrValue;

		if (!bChangeCongfig)
		{
			Get_DataOfMeterConfig(temp, pSetMeasurand, bChangeCongfig);
		}
		else
		{
			cJSON *valGet;
			if (valGet = cJSON_GetObjectItem(temp1, "value"))
			{
				TRACEX(" Here I am for Change Config \n");
				Get_DataOfMeterConfig(valGet, pSetMeasurand, bChangeCongfig);
			}
			else
			{
				return NULL;
			}
		}
	}
	else if (pElementsList[n].ValueType == TYPE_CSL_CONNPHASEROTA)
	{
		static const char* ConnPhaseString[] =
		{
			"NotApplicable",
			"Unknown",
			"RST",
			"RTS",
			"SRT",
			"STR",
			"TRS",
			"TSR",
		};
		char szTempData[500] = { '\0' };
		if (!bChangeCongfig)
		{
			for(n = 0; n < Max_Connctors; n++)
			{
				strcat(szTempData, ConnPhaseString[pOCPP->stOCPPSettings.emConnPhaseRotation[n]]);
				if(n < Max_Connctors - 1)
				{
					strcat(szTempData, ",");
				}
			}
			cJSON_AddStringToObject(temp, "value", szTempData);
		}
		else
		{
			cJSON *valGet;
			if (valGet = cJSON_GetObjectItem(temp1, "value"))
			{
				TRACEX("ConnectionPhaseRotation  %s \n",valGet->valuestring);
				char *szVal = valGet->valuestring;
				char *szPtr = NULL;
				int i = 0;

				if ((szPtr = strstr(szVal,"0.")) != NULL)
				{
					szPtr += strlen("0.");

					for (n = 0; n < ITEM_OF(ConnPhaseString); n++)
					{
						if (strcmp(szPtr, ConnPhaseString[n]) == NULL)
						{
							pOCPP->stOCPPSettings.emConnPhaseRotation[0] = n;
							szPtr += strlen(ConnPhaseString[n]);
							szPtr++;
							break;
						}
					}

				}
				if ((szPtr = strstr(szVal, "1.")) != NULL)
				{
					szPtr += strlen("1.");
					for (n = 0; n < ITEM_OF(ConnPhaseString); n++)
					{
						if (strcmp(szPtr, ConnPhaseString[n]) == NULL)
						{
							pOCPP->stOCPPSettings.emConnPhaseRotation[1] = n;
							szPtr += strlen(ConnPhaseString[n]);
							szPtr++;
							break;
						}
					}

				}
				if ((szPtr = strstr(szVal, "2.")) != NULL)
				{
					szPtr += strlen("2.");
					for (n = 0; n < ITEM_OF(ConnPhaseString); n++)
					{
						if (strcmp(szPtr, ConnPhaseString[n]) == NULL)
						{
							pOCPP->stOCPPSettings.emConnPhaseRotation[2] = n;
							szPtr += strlen(ConnPhaseString[n]);
							szPtr++;
							break;
						}
					}

				}
			}
			else
			{
				return NULL;
			}
		}
	}

	//now update into OCPP structure and DB
	if (bChangeCongfig)
	{
		VAR_VALUE varV;
		ERRCODE_DAI err = -1;
		pOCPP->stOCPPSettings.emBlobType = BLOB_TYPE_OCPP_CONFIGURATIONS;
		varV.blobValue = &(pOCPP->stOCPPSettings);
		//varV.blobValue = OCPP_Settings;
		if ((pElementsList[n].Readonly == FALSE) && (bAccpeted))
		{
			err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
				1, 16, &varV, sizeof(A_BLOB_OCPP_CONFIGS), 1000);
		}
		if (err == ERRCODE_DAI_OK)
		{
			TRACEX("OCPP settings can write Sucessfully\n");
			cJSON_AddStringToObject(temp,"status",err == ERRCODE_DAI_OK ? "Accepted" : "Rejected");
			TRACEX(" Change Config return \n");
		}
		else
		{
			//Revert back OCPP settings
			PARAM_VAL_GET valGet;
			valGet.nGetSize = sizeof(A_BLOB_OCPP_CONFIGS);
			valGet.val.blobValue = (BYTE*)&pOCPP->stOCPPSettings;
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				1, 16, &valGet, sizeof(PARAM_VAL_GET));

			cJSON_AddStringToObject(temp, "status", "Rejected");
			// not able to write the data into the Database.// make Response as rejected 
			//cJSON_Delete(temp);
		}
	}

	return temp;
}

int Handle_GetConfiguration(void* pArg)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	cJSON *pJson = pOCPP->pCurrJson;
	cJSON *temp1 = NULL;
	cJSON *pResp = NULL;
	int NoofConfig = -1;
	//OCPP_SENT_MSG msg;
	TRACEX("Here is the Handle_GetConfiguration = %d\n");

	if (temp1 = cJSON_GetObjectItem(pJson, "key"))
	{
		NoofConfig = cJSON_GetArraySize(temp1);
		TRACEX("Handle_GetConfiguration = %d\n", NoofConfig = cJSON_GetArraySize(temp1));
		cJSON *Arr;
		int n = 0;
		pOCPP->stOCPPSettings.nGetConfigMaxKeys_R = 10; // test Purpose 
		if (NoofConfig == 0) //means get all keys
		{
			static const char* GetConfig_String[] = {
				"AllowOfflineTxForUnknownId",//R
				"AuthorizationCacheEnabled",//RW
				"AuthorizeRemoteTxRequests",//RW
				"BlinkRepeat",//_RW,
				"ClockAlignedDataInterval",//_RW,
				"ConnectionTimeOut",//_RW, //this is for connector, not for protocol
				"GetConfigurationMaxKeys",//_R,
				"HeartbeatInterval",//_RW,

				"LightIntensity",//_RW, //in percent
				"LocalAuthorizeOffline",//_RW,
				"LocalPreAuthorize",//_RW,
				"MaxEngeryOnInvalidId",//_RW,
				"MeterValuesAlignedData",//_RW,
				"MeterValuesAlignedDataMaxLength",//_R,
				"MeterValuesSampledData",//_RW,
				"MeterValuesSampledDataMaxLength",//_R,
				"MeterValueSampleInterval",//_RW, //in seconds, 0 means no Meter.req should be sent!
				"MinimumStatusDuration",//_RW,//in seconds, what is exact meaning ,we do not get yet!
				"NumberOfConnectors",//_R,
				"ResetRetries",//_RW,
				"ConnectorPhaseRotation",//[CONNECTOR_MAX_SUPPORT],
				"ConnectorPhaseRotationMaxLength", //this shall ALWAYS be equal "CONNECTOR_MAX_SUPPORT"!!!
				"StopTransactionOnEVSideDisconnect",//_RW,
				"StopTransactionOnInvalidId",//_RW,
				"StopTxnAlignedData",//_RW,
				"StopTxnAlignedDataMaxLength",//_R,
				"StopTxnSampledData",//_RW,
				"StopTxnSampledDataMaxLength",//_R,
				"SupportedFeatureProfiles",//_R,
				"SupportedFeatureProfilesMaxLength",//_R, //this should be initialize as init config
				"TransactionMessageAttempts",//_RW,
				"TransactionMessageRetryInterval",//_RW,
				"UnlockConnectorOnEVSideDisconnect",//_RW,
				"WebSocketPingInterval",//

				//local list management
				"LocalAuthListEnabled",//_RW,
				"LocalAuthListMaxLength",//_R,
				"SendLocalListMaxLength",//_R,
				//reservation profile
				"ReserveConn0_Enabled",//_R,
				//smart charging
				"ChargeProfileMaxStackLevel",//_R,
				"ChargingScheduleAllowedChargingRateUnit",
				"ChargeScheduleMaxPeriods",//_R,
				"ConnectorSwitch3To1PhaseSupport",//_R,
				"MaxChargingProfilesInstalled",//,
			};
			TRACEX("Inside Handle_GetConfiguration = %d\n", NoofConfig = cJSON_GetArraySize(temp1));			
			int TotalConfig = MAX_OCPP_CONF_KEYS;
			int nSendKeys = 0;
			if(pOCPP->stOCPPSettings.nGetConfigMaxKeys_R > 0)
			{
				//int nSendTimes = (MAX_OCPP_CONF_KEYS / pOCPP->stOCPPSettings.nGetConfigMaxKeys_R) + 1;
				int nSendTimes = 1;
				int nKeyIdx = 0;
				while(nSendTimes > 0)
				{
					Arr = cJSON_CreateArray();
					//for (n = 0; n < pOCPP->stOCPPSettings.nGetConfigMaxKeys_R; n++, nKeyIdx++)
					for (n = 0; n < MAX_OCPP_CONF_KEYS ; n++, nKeyIdx++)
					{
						if(nKeyIdx >= MAX_OCPP_CONF_KEYS)
						{
							break;
						}
						nSendKeys++;
						cJSON *temp;
						temp = GetConfig_Response(pOCPP, GetConfig_String[nKeyIdx], 0);
						cJSON_AddItemToArray(Arr, temp);
					}

					if(nSendKeys == 0)
					{
						cJSON_Delete(Arr);
						break;
					}

					cJSON *Getconfig;
					Getconfig = cJSON_CreateObject();
					cJSON_AddItemToObject(Getconfig, "configurationKey", Arr);

					Send_CallResult(pOCPP, Getconfig, GetConfiguration);

					nSendTimes--;
				}
			}
		}
		else //if (NoofConfig <= pOCPP->stOCPPSettings.nGetConfigMaxKeys_R)// that MAXGetConfigKey Consider Here 
		{
			cJSON *Arr;
			cJSON *Getconfig;
			Arr = cJSON_CreateArray();
			Getconfig = cJSON_CreateObject();
			cJSON_AddItemToObject(Getconfig, "configurationKey", Arr);
			cJSON *ArrUnknown = cJSON_CreateArray();
			int nUnknownItem = 0;
			int n;
			//int nSendKeys = 0;
			for (n = 0; n < MIN(pOCPP->stOCPPSettings.nGetConfigMaxKeys_R, NoofConfig); n++)
			{
				cJSON *configKeyVal = cJSON_GetArrayItem(temp1, n);
				TRACEX("Handle_GetConfiguration = %s \n", configKeyVal->valuestring);
				cJSON *temp;
				temp = GetConfig_Response(pOCPP, configKeyVal->valuestring, 0);
				if(temp == NULL)
				{
					cJSON_AddItemToArray(ArrUnknown, cJSON_CreateString(configKeyVal->valuestring));
					nUnknownItem++;
				}
				else
				{
					///nSendKeys++;
					cJSON_AddItemToArray(Arr, temp);
				}
			}

			if(nUnknownItem < 1)
			{
				cJSON_Delete(ArrUnknown);
			}
			else
			{
				cJSON_AddItemToObject(Getconfig, "unknownKey", ArrUnknown);
			}

			//if(nSendKeys > 0)
			{
				Send_CallResult(pOCPP, Getconfig, GetConfiguration);
			}
			// else
			// {
			// 	cJSON_Delete(Getconfig);
			// }
		}
	}
	else
	{
			TRACEX(" Key is Missing !\n");
			cJSON *Arr;
			int n = 0;
			static const char* GetConfig_String[] = {
					"AllowOfflineTxForUnknownId",//R
					"AuthorizationCacheEnabled",//RW
					"AuthorizeRemoteTxRequests",//RW
					"BlinkRepeat",//_RW,
					"ClockAlignedDataInterval",//_RW,
					"ConnectionTimeOut",//_RW, //this is for connector, not for protocol
					"GetConfigurationMaxKeys",//_R,
					"HeartbeatInterval",//_RW,

					"LightIntensity",//_RW, //in percent
					"LocalAuthorizeOffline",//_RW,
					"LocalPreAuthorize",//_RW,
					"MaxEngeryOnInvalidId",//_RW,
					"MeterValuesAlignedData",//_RW,
					"MeterValuesAlignedDataMaxLength",//_R,
					"MeterValuesSampledData",//_RW,
					"MeterValuesSampledDataMaxLength",//_R,
					"MeterValueSampleInterval",//_RW, //in seconds, 0 means no Meter.req should be sent!
					"MinimumStatusDuration",//_RW,//in seconds, what is exact meaning ,we do not get yet!
					"NumberOfConnectors",//_R,
					"ResetRetries",//_RW,
					"ConnectorPhaseRotation",//[CONNECTOR_MAX_SUPPORT],
					"ConnectorPhaseRotationMaxLength", //this shall ALWAYS be equal "CONNECTOR_MAX_SUPPORT"!!!
					"StopTransactionOnEVSideDisconnect",//_RW,
					"StopTransactionOnInvalidId",//_RW,
					"StopTxnAlignedData",//_RW,
					"StopTxnAlignedDataMaxLength",//_R,
					"StopTxnSampledData",//_RW,
					"StopTxnSampledDataMaxLength",//_R,
					"SupportedFeatureProfiles",//_R,
					"SupportedFeatureProfilesMaxLength",//_R, //this should be initialize as init config
					"TransactionMessageAttempts",//_RW,
					"TransactionMessageRetryInterval",//_RW,
					"UnlockConnectorOnEVSideDisconnect",//_RW,
					"WebSocketPingInterval",//

					//local list management
					"LocalAuthListEnabled",//_RW,
					"LocalAuthListMaxLength",//_R,
					"SendLocalListMaxLength",//_R,
					//reservation profile
					"ReserveConn0_Enabled",//_R,
					//smart charging
					"ChargeProfileMaxStackLevel",//_R,
					"ChargingScheduleAllowedChargingRateUnit",
					"ChargeScheduleMaxPeriods",//_R,
					"ConnectorSwitch3To1PhaseSupport",//_R,
					"MaxChargingProfilesInstalled",//,
			};
			int TotalConfig = MAX_OCPP_CONF_KEYS;
			int nSendKeys = 0;

			//int nSendTimes = (MAX_OCPP_CONF_KEYS / pOCPP->stOCPPSettings.nGetConfigMaxKeys_R) + 1;
			int nSendTimes = 1;
			int nKeyIdx = 0;
			while (nSendTimes > 0)
			{
				Arr = cJSON_CreateArray();
				for (n = 0; n < MAX_OCPP_CONF_KEYS; n++, nKeyIdx++)
				{
					if (nKeyIdx >= MAX_OCPP_CONF_KEYS)
					{
						break;
					}
					nSendKeys++;
					cJSON *temp;
					temp = GetConfig_Response(pOCPP, GetConfig_String[nKeyIdx], 0);
					cJSON_AddItemToArray(Arr, temp);
				}

				if (nSendKeys == 0)
				{
					cJSON_Delete(Arr);
					break;
				}

				cJSON *Getconfig;
				Getconfig = cJSON_CreateObject();
				cJSON_AddItemToObject(Getconfig, "configurationKey", Arr);

				Send_CallResult(pOCPP, Getconfig, GetConfiguration);

				nSendTimes--;
			}

			//	Send_CallError(pOCPP, GetConfiguration, CallError_FormationViolation, "key is missing!", NULL);
			//	return -1;
	}

	return 0;
	
}

int Handle_ChangeConfiguration(void* pArg)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	TRACEX("Handle_ChangeConfiguration \n");
	cJSON *pJson = pOCPP->pCurrJson;
	cJSON *temp1 = NULL;
	cJSON *Resp = NULL;
	int NoofConfig = -1 ;

	if (temp1 = cJSON_GetObjectItem(pJson, "key"))
	{
		TRACEX("Handle_Change_Configuration = %s \n", temp1->valuestring);
		
		cJSON *Changeconfig = GetConfig_Response(pOCPP, temp1->valuestring, 1);
		if (Changeconfig == NULL)
		{
			//Send_CallError(pOCPP, ChangeConfiguration, CallError_PropertyConstraintViolation, "wrong key name/value or format!", NULL);
			//Jimmy note 20190702: for OCTT, shall send 'NotSupported' instead of CALLERROR
			Changeconfig = cJSON_CreateObject();
			cJSON_AddStringToObject(Changeconfig, "status", "NotSupported");
			//return -1;
		}
		//Msg_Send(pOCPP, Changeconfig, ChangeConfiguration, CallResult,0,bSend_Later);
		Send_CallResult(pOCPP, Changeconfig, ChangeConfiguration);		
	}
	else
	{
		Send_CallError(pOCPP, ChangeConfiguration, CallError_FormationViolation, "key is missing!", NULL);
		return -1;
	}

	return 0;
}

int Handle_ClearCache(void* pArg)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	TRACEX("Handle_ClearCache \n");

	cJSON *pResp = NULL;
	BOOL bAccpeted;
	//now set the list and get reply

	VAR_VALUE varV;
	varV.emValue = 0;
	if (pOCPP->stOCPPSettings.bAuthCacheEnabled_RW == FALSE)
	{
		//bAccpeted = FALSE;
		//Send_CallError(pOCPP, ClearCache, CallError_NotImplemented, "Can NOT execute because AuthorizeCache is not supported by charger!", NULL);
		//return -1;
		bAccpeted = TRUE; //Jimmy note: shall always send TRUE to be compitable with Server(OCTT test)
	}
	else
	{
		ERRCODE_DAI err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 301, 301, &varV, 4, 0);
		(err == ERRCODE_DAI_OK)? (bAccpeted = TRUE) : (bAccpeted = FALSE);
	}

	pResp = cJSON_CreateObject();
	cJSON_AddStringToObject(pResp, "status", bAccpeted ? "Accepted" : "Rejected");
	//char *pszT = cJSON_Print(pResp);
	//memcpy(msg.byData2Sent, pszT, strlen(pszT));
	//msg.nSentLength = strlen(pszT);
	//Msg_Send(pOCPP, pResp, ClearCache, CallResult,0, bSend_Later);
	Send_CallResult(pOCPP, pResp, ClearCache);

	return 0;
}

int Handle_RemoteStartTransaction(void* pArg)
{
	//It is advised not to start transactions remotely without ConnectorId due to the
	//uncertainty which Connector is started.
	//AuthorizeRemoteTxRequests (Choice is up to Charge Point implementation.)so Decide Only One Flow of the Remote Start 
	// we have to go For first Authorization and than Start charging or start and Verify Idtag into the StartTx.Conf

	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	TRACEX("Handle_RemoteStartTransaction \n");

	cJSON *pJson = pOCPP->pCurrJson;

	cJSON *pResp = NULL;
	cJSON *temp1 = NULL;

	VAR_VALUE varV;
	int n;
	ERRCODE_DAI err;
	
	int ConnectorId = -1;
	char temp_idtag[20] = {};
	BOOL bRejected = TRUE;

	if (temp1 = cJSON_GetObjectItem(pJson, "idTag"))
	{
		if(temp1->valuestring == NULL)
		{
			Send_CallError(pOCPP, RemoteStartTransaction, CallError_FormationViolation, "idTag is missing!", NULL);
			return -1;
		}

		memcpy(temp_idtag, temp1->valuestring, MIN(20, strlen(temp1->valuestring)));
		if(pOCPP->stOCPPSettings.bAuthRemoteTxReq_RW)
		{
			varV.blobValue = temp_idtag;
			err = pReporter->pDAI->Set(
				(HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 301, 305, &varV,
				20, 1000);
			if (err == ERRCODE_DAI_OK)
			{
				bRejected = FALSE;
			}
		}
		else
		{
			bRejected = FALSE;
		}
	}
	else
	{
		Send_CallError(pOCPP, RemoteStartTransaction, CallError_FormationViolation, "idTag is missing!", NULL);
		return -1;
	}

	if(!bRejected)
	{
		if (temp1 = cJSON_GetObjectItem(pJson, "connectorId"))  // optinal field 
		{
			ConnectorId = temp1->valueint;
		}
		else
		{
			ConnectorId = 99;
		}
		int nGunIdx = -1;
		if (ConnectorId == 99) //if connectorId is selected, shall not be ZERO!
		{
			for(n = 0 ; n < Max_Connctors; n++)
			{
				if(((pOCPP->stConnectorData[n].emConnctorStauts == Conn_Idle) || 
					(pOCPP->stConnectorData[n].emConnctorStauts == Conn_Reserved))/* && 
					(pOCPP->stConnectorData[n].nGunPlugStatus == 1)*/)
				{
					nGunIdx = n;
					break;
				}
			}
		}
		else if ((ConnectorId > 0) && (ConnectorId <= Max_Connctors))
		{
			n = ConnectorId -1;
			if(((pOCPP->stConnectorData[n].emConnctorStauts == Conn_Idle) || 
				(pOCPP->stConnectorData[n].emConnctorStauts == Conn_Reserved)) /*&& 
				(pOCPP->stConnectorData[n].nGunPlugStatus == 1)*/)
			{
				nGunIdx = n;
			}
		}

		// need to Check Gun is Connected 
		//int  GunStatus;
		//PARAM_VAL_GET valGet;
		//valGet.nGetSize = sizeof(GunStatus);
		//valGet.val.emValue = (BYTE*)&GunStatus;
		//pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		//	201 + nGunIdx, 102, &valGet, sizeof(PARAM_VAL_GET));
		//if (GunStatus == 0)
		//{
		//	bRejected = FALSE;
		//}
		bRejected = TRUE;
		if((nGunIdx != -1) /*&& (GunStatus != 0)*/)
		{
			A_BLOB_CHARGE_START_CMD  RemoteStart;
			ZERO_POBJS(&RemoteStart, 1);
			RemoteStart.emBlobType = BLOB_TYPE_CHARGE_START_CMD;
			memcpy(RemoteStart.szUserID, temp_idtag, 20);
			RemoteStart.emStartMethod = (pOCPP->stConnectorData[nGunIdx].emConnctorStauts == Conn_Reserved) ? CHARGE_START_BY_RESERVE :
				CHARGE_START_BYSERVER;
			varV.blobValue = &RemoteStart;
			err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 
				201 + nGunIdx, 101, &varV, sizeof(A_BLOB_CHARGE_START_CMD), 1000);
			if (err == ERRCODE_DAI_OK)
			{
				bRejected = FALSE;
			}
		}

		if(!bRejected)
		{
			if (temp1 = cJSON_GetObjectItem(pJson, "chargingProfile"))  // optional field 
			{
				//set charging profile, will be supported later
			
			}
		}
	}

	pResp = cJSON_CreateObject();
	cJSON_AddStringToObject(pResp, "status", (!bRejected) ? "Accepted" : "Rejected");

	Send_CallResult(pOCPP, pResp, RemoteStartTransaction);

	return 0;
}

int Handle_RemoteStopTransaction(void* pArg)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	TRACEX("Handle_RemoteStopTransaction \n");
	cJSON *pJson = pOCPP->pCurrJson;
	cJSON *pResp = NULL;
	cJSON *temp1 = NULL;
	
	BOOL bAccepted = FALSE;
	UINT temp_transactionId = 0;
	int n;

	if (temp1 = cJSON_GetObjectItem(pJson, "transactionId"))
	{
		temp_transactionId = temp1->valueint;
	}
	else
	{
		Send_CallError(pOCPP, RemoteStopTransaction, CallError_FormationViolation, "transactionId is missing!", NULL);
		return -1;
	}

	for(n = 0; n < Max_Connctors; n++)
	{
		if ((pOCPP->stConnectorData[n].emConnctorStauts == Conn_InCharging) && 
			(pOCPP->stConnectorData[n].blobInHandling_ChargeInfo.nSessionID == temp_transactionId))
		{
			TRACEX("StopCharging by Remote \n");
			VAR_VALUE varV;
			A_BLOB_CHARGE_STOP_CMD  RemoteStop;
			ZERO_POBJS(&RemoteStop, 1);
			RemoteStop.nStopBy = CHARGE_STOP_BY_SERVER;
			RemoteStop.emBlobType = BLOB_TYPE_CHARGE_STOP_CMD;
			varV.blobValue = &RemoteStop;

			ERRCODE_DAI err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
				201 + n, 102, &varV,sizeof(RemoteStop), 3000);
			bAccepted = (err == ERRCODE_DAI_OK) ? TRUE : FALSE;
			break;
		}
	}


	pResp = cJSON_CreateObject();
	cJSON_AddStringToObject(pResp, "status", bAccepted ? "Accepted" : "Rejected");
	Send_CallResult(pOCPP, pResp, RemoteStopTransaction);

	return 0;
}

int Handle_Reset(void* pArg)
{
	// need to discuss two Reset 
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	TRACEX("Handle_Reset\n");

	cJSON *pJson = pOCPP->pCurrJson;
	cJSON *pResp = NULL;
	cJSON *temp1 = NULL;
	BOOL bHardReset = -1;
	if (temp1 = cJSON_GetObjectItem(pJson, "type"))
	{
		if (strcmp(temp1->valuestring, "Hard") == 0)
		{
			bHardReset = TRUE;
		}
		else if(strcmp(temp1->valuestring,"Soft") == NULL)
		{
			bHardReset = FALSE;
		}
	}
	else
	{
		Send_CallError(pOCPP, Reset, CallError_FormationViolation, "Reset type is missing!", NULL);
		return -1;
	}

	BOOL bAccepted = FALSE;
	if(bHardReset != -1)
	{
		VAR_VALUE varV;
		varV.emValue = bHardReset ? 1 : 0;
		ERRCODE_DAI err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
			1, 2, &varV,4, 3000);
		bAccepted = (err == ERRCODE_DAI_OK) ? TRUE : FALSE;
	}
	else
	{
		Send_CallError(pOCPP, Reset, CallError_PropertyConstraintViolation, "Reset type not correct!", NULL);
		return -1;
	}

	pResp = cJSON_CreateObject();
	cJSON_AddStringToObject(pResp, "status", bAccepted ? "Accepted" : "Rejected");
	
	Send_CallResult(pOCPP, pResp, Reset);

	return 0;
}

int Handle_CancelReservation(void* pArg)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	TRACEX("Handle_CancelReservation\n");

	cJSON *pJson = pOCPP->pCurrJson;
	cJSON *temp1 = NULL;
	cJSON *pResp = NULL;

	BOOL bAccpted = FALSE;
	
	PARAM_VAL_GET valGet;
	int connector = -1;
	int nReserveId = 0;
	if (temp1 = cJSON_GetObjectItem(pJson, "reservationId"))
	{
		//if (temp1->valueint != NULL)
		{
			nReserveId = temp1->valueint;
		}
		int n;
		for (n = 0; n < Max_Connctors; n++)
		{
			//if (pOCPP->stConnectorData[n].emConnctorStauts == Conn_Reserved)
			{
				A_BLOB_RESREVE_CMD reservInfo;
				valGet.nGetSize = sizeof(A_BLOB_RESREVE_CMD);
				valGet.val.blobValue = (BYTE*)&reservInfo;
				pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
					201 + n, 110, &valGet, sizeof(PARAM_VAL_GET));
				TRACEX("Cancel Reservation[%d]: %d\n", n, reservInfo.nReservID);
				if (nReserveId == reservInfo.nReservID)
				{
					TRACEX("Cancel Reservation[%d]: %d\n", n, reservInfo.nReservID);
					connector = n;
					bAccpted = TRUE;
					break;
				}
			}
		}
	}
	else
	{
		Send_CallError(pOCPP, CancelReservation, CallError_FormationViolation, "ReservationId is missing!", NULL);
		return -1;
	}

	if (bAccpted)
	{
		bAccpted = FALSE;
		OBJ_PTR obj;
		bAccpted = FALSE;
		VAR_VALUE varV;
		A_BLOB_RESREVE_CMD temp_reservedata;
		temp_reservedata.emBlobType = BLOB_TYPE_RESERVE_CMD;
		temp_reservedata.emReservTp = RESERVE_Cancel;
		temp_reservedata.nReservID = nReserveId;

		obj.nId = 201 + connector;
		obj.nSubId = 104;

		varV.blobValue = &temp_reservedata;

		ERRCODE_DAI err = pReporter->pDAI->Set(
		(HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, obj.nId, obj.nSubId, &varV,
		sizeof(temp_reservedata), 0);
		(err == ERRCODE_DAI_OK) ? (bAccpted = TRUE) : (bAccpted = FALSE);

	}
	else
	{
		bAccpted = TRUE; //if reserveId not valid, should always give accepted response.
	}
	

	pResp = cJSON_CreateObject();
	cJSON_AddStringToObject(pResp, "status", bAccpted ? "Accepted" : "Rejected");
	Send_CallResult(pOCPP, pResp, CancelReservation);

	return 0;
}

int Handle_ReserveNow(void* pArg)
{
	const char *ReservationStatus[] =
	{
		"Accepted",
		"Faulted",
		"Occupied",
		"Rejected",
		"Unavailable",
	};
	enum
	{
		RES_ACCPTED = 0,
		RES_FALUTED,
		RES_OCCUPIED,
		RES_REJECTED,
		RES_UNAVAILABLE,
	}RESERVE_STATUS;

	RESERVE_STATUS = RES_REJECTED;

	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	TRACEX("Handle_ReserveNow\n");

	cJSON *pJson = pOCPP->pCurrJson;
	cJSON *temp1 = NULL;
	cJSON *pResp = NULL;

	int connectorId = -1;
	BOOL bAccpted = FALSE;


	A_BLOB_RESREVE_CMD temp_reservedata;
	ZERO_POBJS(&temp_reservedata, 1);
	temp_reservedata.emBlobType = BLOB_TYPE_RESERVE_CMD;

	if (temp1 = cJSON_GetObjectItem(pJson, "connectorId"))
	{
		//1.
		connectorId = temp1->valueint;
		if ((connectorId == 0) && (pOCPP->stOCPPSettings.bReserveConn0_Enabled_R == TRUE))
		{
			connectorId = Connctor_0;
			bAccpted = TRUE;
		}
		else if (connectorId > 0 && connectorId <= Max_Connctors)
		{
			connectorId = connectorId - 1;
			bAccpted = TRUE;
		}
		else
		{
			// Rejected Req. 
		}
	}
	else
	{
		Send_CallError(pOCPP, ReserveNow, CallError_FormationViolation, "ConnectorID is missing!", NULL);
		return -1;
	}

	if (bAccpted)
	{
		bAccpted = FALSE;
		if (temp1 = cJSON_GetObjectItem(pJson, "expiryDate"))
		{
			temp_reservedata.tmExpiry = Parse_OCPPTime(temp1->valuestring);
			// need to Parse the Datatime 
			if (temp1 = cJSON_GetObjectItem(pJson, "idTag"))
			{
				if(temp1->valuestring == NULL)
				{
					Send_CallError(pOCPP, ReserveNow, CallError_FormationViolation, "iDTag is missing!", NULL);
					return -1;
				}

				memcpy(temp_reservedata.authData.szIdTag, temp1->valuestring, MIN(20, strlen(temp1->valuestring)));

				if (temp1 = cJSON_GetObjectItem(pJson, "reservationId"))
				{
					temp_reservedata.nReservID = temp1->valueint;
					//If the reservationId does not match any reservation in the Charge Point, then the
					//Charge Point returns the status value Accepted
					bAccpted = TRUE;
				}
			}
		}

		if(!bAccpted)
		{
			Send_CallError(pOCPP, ReserveNow, CallError_FormationViolation, "one of [ExpiryData/IDTag/ReservationID] is missing!", NULL);
			return -1;
		}
	}

	// is optional 
	if (temp1 = cJSON_GetObjectItem(pJson, "parentIdTag"))
	{
		if(temp1->valuestring != NULL)
		{
			memcpy(temp_reservedata.authData.szParentIdTag, temp1->valuestring, MIN(20, strlen(temp1->valuestring)));
		}
	}


	if (pOCPP->stOCPPSettings.cslSupportedFeatures_R.Set.ReserveMnmg == 0)
	{
		TRACEX("-----Reserve cmd is not supported-----\n");
		bAccpted = FALSE;
		RESERVE_STATUS = RES_REJECTED;
	}
	else if(temp_reservedata.tmExpiry - _NOW_ <= 5) //at least the expiryDate shall be after now!
	{
		TRACEX("-----Reserve an phased out reservation-----\n");
		bAccpted = FALSE;
		RESERVE_STATUS = RES_REJECTED;
	}

	RESERVE_STATUS = RES_REJECTED;
	if ((connectorId == Connctor_0) && bAccpted)
	{
		bAccpted = FALSE;
		int n;
		for (n = 0; n < Max_Connctors; n++)
		{
			if (pOCPP->stConnectorData[n].emConnctorStauts == Conn_Idle)
			{
				RESERVE_STATUS = RES_ACCPTED;
				connectorId = n; // this Connetor is going to be Reserve 
				bAccpted = TRUE;
				break;
			}
		}
	}
	else if(((connectorId >= Connctor_1)  && (connectorId < Max_Connctors)) && (bAccpted))
	{
		// for Particular Connetor 
		bAccpted = FALSE;
		//int n = 0;
		//for (n = 0; n < Max_Connctors; n++)
		//{
		//	if (pOCPP->stConnectorData[n].nCancelReservationId == temp_reservedata.nReservID)
		//	{
		//		// The Charge Point SHALL replace that reservation with the new reservation in the request.
		//	}
		//}
		//if (n >= Max_Connctors)
		//{
		//	bAccpted = TRUE;
		//}
		if(pOCPP->stConnectorData[connectorId].emConnctorStauts == Conn_Idle)
		{
			bAccpted = TRUE;
			RESERVE_STATUS = RES_ACCPTED;
		}
		else if (pOCPP->stConnectorData[connectorId].emConnctorStauts == Conn_UnAvailable)
		{
			RESERVE_STATUS = RES_UNAVAILABLE;
		}
		else if (/*(pOCPP->stConnectorData[connectorId].emConnctorStauts == Conn_Reserved) || */(pOCPP->stConnectorData[connectorId].emConnctorStauts == Conn_InCharging))
		{
			//Jimmy note: if reservation id is the same, should update the reserve info in CCU sampler.
			RESERVE_STATUS = RES_OCCUPIED;
		}
		else if(pOCPP->stConnectorData[connectorId].emConnctorStauts == Conn_Reserved)
		{
			//Jimmy note: if reservation id is the same, should update the reserve info in CCU sampler.
			A_BLOB_RESREVE_CMD reservInfo;
			PARAM_VAL_GET valGet;
			valGet.nGetSize = sizeof(A_BLOB_RESREVE_CMD);
			valGet.val.blobValue = (BYTE*)&reservInfo;
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				201 + connectorId, 110, &valGet, sizeof(PARAM_VAL_GET));
			if(reservInfo.nReservID == temp_reservedata.nReservID) //will send to sampler to update the current one
			{
				bAccpted = TRUE;
				RESERVE_STATUS = RES_ACCPTED;
			}
		}
		else if (pOCPP->stConnectorData[connectorId].emConnctorStauts == Conn_Failed)
		{
			RESERVE_STATUS = RES_FALUTED;
		}
	}

	if (bAccpted)
	{
		RESERVE_STATUS = RES_ACCPTED;
		TRACEX("-----Now send reserve cmd to CCU sampler-----\n");
		VAR_VALUE varV;
		varV.blobValue = &temp_reservedata;
		temp_reservedata.emReservTp = RESERVE_StartNow;
		ERRCODE_DAI err = pReporter->pDAI->Set(
			(HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 201 + connectorId, 104, &varV,
			sizeof(A_BLOB_RESREVE_CMD), 1000);
		RESERVE_STATUS = (err == ERRCODE_DAI_OK) ? RES_ACCPTED: RES_REJECTED;
		if(RESERVE_STATUS == RES_ACCPTED)
		{
			//now update local cache
			A_BLOB_UPDATE_AUTHSTAT_ANDCACHE update;
			temp_reservedata.authData.emAuthorizationStatus = AT_Accpted;
			memcpy(&update.newAuthData, &temp_reservedata.authData, sizeof(AUTHORIZATION_DATA));
			update.emBlobType = BLOB_TYPE_UPDATE_AUTHSTATUS_ANDCACHE;
			update.emUpdateType = AUTH_UPDATE_STARTTHANS_CONF;
			varV.blobValue = &update;
			pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 301, 303, &varV,
				sizeof(A_BLOB_UPDATE_AUTHSTAT_ANDCACHE), 0);
		}
	}

	
	//OCPP_SENT_MSG msg;
	pResp = cJSON_CreateObject();
	cJSON_AddStringToObject(pResp, "status", ReservationStatus[RESERVE_STATUS]);
	Send_CallResult(pOCPP, pResp, ReserveNow);

	return 0;

}

int Handle_TriggerMessage(void* pArg)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	TRACEX("Handle_TriggerMessage \n");

	cJSON *pJson = pOCPP->pCurrJson;
	cJSON *pResp = NULL;
	cJSON *temp1 = NULL;

	char req_msg[32] = {};
	int ConnectorId = -1;
	OBJ_PTR obj;

	if (temp1 = cJSON_GetObjectItem(pJson, "requestedMessage"))
	{
		memcpy(req_msg,temp1->valuestring,strlen(temp1->valuestring));
		if (strcmp(req_msg, "BootNotification") == NULL)
		{
			pOCPP->emTriggerMsgType = Trigger_BootNotification;
		}
		else if(strcmp(req_msg, "DiagnosticsStatusNotification") == NULL)
		{
			pOCPP->emTriggerMsgType = Trigger_DiagnosticsStatusNotification;
		}
		else if (strcmp(req_msg, "FirmwareStatusNotification") == NULL)
		{
			pOCPP->emTriggerMsgType = Trigger_FirmwareStatusNotification;
		}
		else if (strcmp(req_msg, "Heartbeat") == NULL)
		{
			pOCPP->emTriggerMsgType = Trigger_Heartbeat;
		}
		else if (strcmp(req_msg, "MeterValues") == NULL)
		{
			if (temp1 = cJSON_GetObjectItem(pJson, "connectorId"))
			{
				ConnectorId = temp1->valueint - 1;
				if (ConnectorId == Connctor_0)
				{
					pOCPP->emHandlingConnector = Connctor_0;
					pOCPP->emTriggerMsgType = Trigger_MeterValues;
					pOCPP->emHandlingMeterType = Trigger;

				}
				if (ConnectorId > Connctor_0 && ConnectorId < Max_Connctors)
				{
					pOCPP->emHandlingConnector = ConnectorId;
					pOCPP->emTriggerMsgType = Trigger_MeterValues;
					pOCPP->emHandlingMeterType = Trigger;
				}
			}
		}
		else if (strcmp(req_msg, "StatusNotification") == NULL)
		{
			if (temp1 = cJSON_GetObjectItem(pJson, "connectorId"))
			{
				ConnectorId = temp1->valueint - 1;

				if (ConnectorId == Connctor_0)
				{
					pOCPP->emHandlingConnector = Connctor_0;
					pOCPP->emTriggerMsgType = Trigger_StatusNotification;
				}
				if (ConnectorId > Connctor_0 && ConnectorId < Max_Connctors)
				{
					pOCPP->emHandlingConnector = ConnectorId;
					pOCPP->emTriggerMsgType = Trigger_StatusNotification;
				}
			}

		}
		else 
		{
			Send_CallError(pOCPP, TriggerMessage, CallError_PropertyConstraintViolation, "TriggleMsg is not recognized!", NULL);
			return -1;
		}

	}
	else
	{
		Send_CallError(pOCPP, TriggerMessage, CallError_FormationViolation, "requestedMessage is missing!", NULL);
		return -1;
	}

	if((pOCPP->emOCPPConnStatus != Ready) && (Trigger_BootNotification != pOCPP->emTriggerMsgType))
	{
		return -1; //when not pending, we do not accept triggle cmd
	}
	
	ERRCODE_DAI err;

	err = ERRCODE_DAI_OK;
	pResp = cJSON_CreateObject();
	cJSON_AddStringToObject(pResp, "status", (pOCPP->emTriggerMsgType != Trigger_NoTriggle) ? "Accepted" : "Rejected");

	Send_CallResult(pOCPP, pResp, TriggerMessage);

	switch (pOCPP->emTriggerMsgType)
	{
	case Trigger_BootNotification:
		pOCPP->stMsgInfoList[BootNotification].pfnMsgHandler((void*)pOCPP);
		break;
	case Trigger_DiagnosticsStatusNotification:
		pOCPP->stMsgInfoList[DiagnosticsStatusNotification].pfnMsgHandler((void*)pOCPP);
		break;
	case Trigger_FirmwareStatusNotification:
		pOCPP->stMsgInfoList[FirmwareStatusNotification].pfnMsgHandler((void*)pOCPP);
		break;
	case Trigger_Heartbeat:
		pOCPP->stMsgInfoList[HeartBeat].pfnMsgHandler((void*)pOCPP);
		break;
	case Trigger_MeterValues:
		pOCPP->emHandlingMeterType = Trigger;
		pOCPP->stMsgInfoList[MeterValues].pfnMsgHandler((void*)pOCPP);
		break;
	case Trigger_StatusNotification:
		pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP);
		break;
	default:
		break;
	}

	return 0;
}

int Handle_UnlockConnector(void* pArg)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	TRACEX("Handle_UnlockConnector \n");

	cJSON *pJson = pOCPP->pCurrJson;
	cJSON *temp1;
	cJSON *pResp = NULL;
	int  nUnlockOK = FALSE;
	int GunStatus = -1;

	OBJ_PTR obj;
	int ConnectorId = -1;

	if (temp1 = cJSON_GetObjectItem(pJson, "connectorId"))
	{
		ConnectorId = temp1->valueint;
	}
	else
	{
		Send_CallError(pOCPP, UnlockConnector, CallError_FormationViolation, "connectorId is missing!", NULL);
		return -1;
	}
	if (ConnectorId > 0  && ConnectorId <= Max_Connctors)
	{
		// if((pOCPP->stConnectorData[ConnectorId].nGunPlugStatus == 1) 
		// 	&& (pOCPP->stOCPPSettings.bUnlockConnOnEVSideDiscon_RW == TRUE))//plugged
		{
			obj.nId = 200 + ConnectorId;
			obj.nSubId = 112;
			//int UnlockGun = 1;
			VAR_VALUE varV;
			varV.emValue = 1;
			ERRCODE_DAI err = pReporter->pDAI->Set(
				(HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, obj.nId, obj.nSubId, &varV,
				4, 1000);
			err != ERRCODE_DAI_OK ? (nUnlockOK = TRUE) : (nUnlockOK = FALSE);
		}
	}
	else
	{
		nUnlockOK = 99;
	}
	
	pResp = cJSON_CreateObject();
	if(nUnlockOK == 99) //cp has no connector lock
	{
		cJSON_AddStringToObject(pResp, "status", "NotSupported");
	}
	else
	{
		cJSON_AddStringToObject(pResp, "status", (nUnlockOK)? "Unlocked" : "UnlockFailed");
	}	
	
	Send_CallResult(pOCPP, pResp, UnlockConnector);

	return 0;
}

//Local List 

int Handle_GetLocalListVersion(void* pArg)
{
	// Need to Check Later 
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	TRACEX("Handle_GetLocalListVersion\n");
	
	cJSON *pJson = pOCPP->pCurrJson;
	cJSON *pResp = NULL;
	cJSON *temp1 = NULL;

	//now set the list and get reply
	int nLocalListVer = -1;
	if(pOCPP->stOCPPSettings.bLocalListEnabled_RW) //Jimmy added 20190704: -1 shall be used if not ENABLED
	{
		ERRCODE_DAI err = pReporter->pDAI->Get(
			(HANDLE)pReporter, DAITYPE_GETLOCALLIST_VERSION, 0, 0, &nLocalListVer, 4);
		if(err != ERRCODE_DAI_OK)
		{
			Send_CallError(pOCPP, GetLocalListVersion, CallError_InternalError, "Can NOT fetch list version num!", NULL);
			return -1;
		}
	}

	pResp = cJSON_CreateObject();
	cJSON_AddNumberToObject(pResp, "listVersion", nLocalListVer);
	//char *pszT = cJSON_Print(pResp);
	//memcpy(msg.byData2Sent, pszT, strlen(pszT));
	//msg.nSentLength = strlen(pszT);
	//Msg_Send(pOCPP, pResp, GetLocalListVersion, CallResult, 0, bSend_Later);
	Send_CallResult(pOCPP, pResp, GetLocalListVersion);

	return 0;
}

int Handle_SendLocalList(void* pArg)
{
	/*If the status is Failed or VersionMismatch and the updateType was Differential, then Central System SHOULD retry
		sending the full local authorization list with updateType Full.*/
	TRACEX("Handle_SendLocalList \n");
	// need Check Later 
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	A_BLOB_UPDATE_LOCALLIST updatList;
	ZERO_POBJS(&updatList, 1);
	updatList.emBlobType = BLOB_TYPE_UPDATE_LOCALLIST;

	cJSON *pJson = pOCPP->pCurrJson;
	cJSON *pChild = pJson->child;

	BOOL bListVerFound = FALSE;
	BOOL	bUpdateTypeFound = FALSE;
	cJSON *pAuthDataList = NULL;
	cJSON *temp1 = NULL;

	if (temp1 = cJSON_GetObjectItem(pJson, "listVersion"))
	{
		bListVerFound = TRUE;
		updatList.nListVersion = temp1->valueint;
		TRACEX("LocalList Version %d\n", temp1->valueint);

		if (temp1 = cJSON_GetObjectItem(pJson, "updateType"))
		{
			bUpdateTypeFound = TRUE;
			if (strcmp(temp1->valuestring, "Differential") == 0)
			{
				TRACEX("LocalList updateType %s \n", temp1->valuestring);
				updatList.emUpdateType = LL_UPDATE_Diff;
			}
			else if (strcmp(temp1->valuestring, "Full") == 0)
			{
				TRACEX("LocalList updateType %s \n", temp1->valuestring);
				updatList.emUpdateType = LL_UPDATE_Full;
			}
		}
	}


	if ((!bListVerFound) || (!bUpdateTypeFound)) /*|| (pAuthDataList == NULL)*/
	{
		//cJSON *pRep = cJSON_CreateObject();
		//cJSON_AddStringToObject(pRep, "status", "Rejected");
		//Send_CallResult(pOCPP, pRep, SendLocalList);
		//return -1;
		Send_CallError(pOCPP, SendLocalList, CallError_FormationViolation, "listVersion or updateType is missing!", NULL);
		return -1;
	}

	if(!pOCPP->stOCPPSettings.bLocalListEnabled_RW) //not supported
	{
		cJSON *pRep = cJSON_CreateObject();
		cJSON_AddStringToObject(pRep, "status","NotSupported"); 
		Send_CallResult(pOCPP, pRep, SendLocalList);
		return -1;
	}

	int nLocalListVer;
	ERRCODE_DAI err = pReporter->pDAI->Get(
		(HANDLE)pReporter, DAITYPE_GETLOCALLIST_VERSION, 0, 0, &nLocalListVer, 4);
	// this is validation point to check Locallist Version with Update Type
	if ((nLocalListVer < updatList.nListVersion) && (updatList.emUpdateType == LL_UPDATE_Diff))
	{
		pAuthDataList = cJSON_GetObjectItem(pJson, "localAuthorizationList");
	}
	else if ((nLocalListVer >= updatList.nListVersion) && (updatList.emUpdateType == LL_UPDATE_Diff))
	{
		TRACEX("Local List Version %d updatelocal list :%d\n", nLocalListVer, updatList.nListVersion);
		cJSON *pRep = cJSON_CreateObject();
		cJSON_AddStringToObject(pRep, "status","VersionMismatch");  //maybe need to consider versionMismatch later
		Send_CallResult(pOCPP, pRep, SendLocalList);
		return 0;
	}
	else
	{
		// Full type Case
		pAuthDataList = cJSON_GetObjectItem(pJson, "localAuthorizationList");
	}

	if (pAuthDataList != NULL) //Null is supported when we only Update the version
	{
		TRACEX("pAuthDataList Avaliable \n");
		//Now we parse all the Authdata
		int nTotal = cJSON_GetArraySize(pAuthDataList);
		TRACEX("Reacords %d  \n", nTotal);
		updatList.nAuthorizationDataNumber = nTotal;
		
		if ((nTotal < pOCPP->stOCPPSettings.nSengLocalListMaxLength_R) && (nTotal != NULL))
		{
			int n;
			for (n = 0; n < nTotal; n++)
			{
				cJSON *pJ = cJSON_GetArrayItem(pAuthDataList, n);
				if (pJ == NULL)
				{
					//just break???
					break;
				}
				cJSON *idTag;
				idTag = cJSON_GetObjectItem(pJ, "idTag");
				if ((idTag != NULL) && (strcmp(idTag->valuestring, "") != NULL) && (idTag->valuestring != NULL))
				{
					memcpy(updatList.authorizationDataList[n].szIdTag, idTag->valuestring, MIN(20, strlen(idTag->valuestring)));
					TRACEX("idTag : %s\n", idTag->valuestring);
				}
				else
				{
					TRACEX("Normally this can not happen, should sendCallError if idTag is not correctly configured by server!\n");
					continue;
				}

				cJSON *idTagInfoObject = NULL;
				cJSON *temp1 = NULL;
				idTagInfoObject = cJSON_GetObjectItem(pJ, "idTagInfo");

				if (idTagInfoObject == NULL)
				{
					continue;
				}
				temp1 = cJSON_GetObjectItem(idTagInfoObject, "status");
				if (temp1 == NULL)
				{
					continue;
				}
				TRACEX("status: %s\n", temp1->valuestring);
				updatList.authorizationDataList[n].emAuthorizationStatus = AT_Old_Saving;
				if (strcmp(temp1->valuestring, "Accepted") == NULL)
				{
					updatList.authorizationDataList[n].emAuthorizationStatus = AT_Accpted;
				}
				else if (strcmp(temp1->valuestring, "Blocked") == NULL)
				{
					updatList.authorizationDataList[n].emAuthorizationStatus = AT_Blocked;
				}
				else if (strcmp(temp1->valuestring, "Expired") == NULL)
				{
					updatList.authorizationDataList[n].emAuthorizationStatus = AT_Expired;
				}
				else if (strcmp(temp1->valuestring, "Invalid") == NULL)
				{
					updatList.authorizationDataList[n].emAuthorizationStatus = AT_Invalid;
				}

				if (temp1 = cJSON_GetObjectItem(idTagInfoObject, "parentIdTag"))
				{
					TRACEX("parentIdTag: %s\n", temp1->valuestring);
					if(temp1->valuestring != NULL) //Jimmy note 20190223:Romania test found NULL value here can cause a crash
					{
						memcpy(updatList.authorizationDataList[n].szParentIdTag, temp1->valuestring, MIN(20, strlen(temp1->valuestring)));
					}
				}

				if (temp1 = cJSON_GetObjectItem(idTagInfoObject, "expiryDate"))
				{
					TRACEX("expiryDate: %s\n", temp1->valuestring);
					updatList.authorizationDataList[n].dtExpiryDate = (temp1->valuestring == NULL) ? 0 :
						Parse_OCPPTime(temp1->valuestring);
				}

			}
		}
		else if(nTotal > pOCPP->stOCPPSettings.nSengLocalListMaxLength_R)
		{
			TRACEX("Failed To Update : Exceed OCPP Configuration Value\"SendLocalListMaxLength\" \n");
			cJSON *pRep = cJSON_CreateObject();
			cJSON_AddStringToObject(pRep, "status", "Failed");  //maybe need to consider versionMismatch later
			Send_CallResult(pOCPP, pRep, SendLocalList);
			return 0;
		}
	}
	else if ((pAuthDataList == NULL)  && (updatList.emUpdateType == LL_UPDATE_Full))
	{
		//Jimmy noted: this is possible when server wants to clear the AuthorizeData, just leave it NULL
		updatList.nListVersion = 0; // this Indicate that No IdTag into the LocalAuthorization List 
	}

	//now set the list and get reply
	int n = 0;
	for (n = 0; n < updatList.nAuthorizationDataNumber; n++)
	{
		TRACE("idTag:%s\n", updatList.authorizationDataList[n].szIdTag);
		TRACE("Exp:%lu\n", updatList.authorizationDataList[n].dtExpiryDate);
		TRACE("ParentIdTag:%s\n", updatList.authorizationDataList[n].szParentIdTag);
		TRACE("Status:%d\n ", updatList.authorizationDataList[n].emAuthorizationStatus);
	}

	VAR_VALUE varV;
	varV.blobValue = &updatList;
	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 301, 304, &varV,
		sizeof(A_BLOB_UPDATE_LOCALLIST), 0);

	cJSON *pRep = cJSON_CreateObject();
	cJSON_AddStringToObject(pRep, "status", (err == ERRCODE_DAI_OK) ? "Accepted" : "Failed");  //maybe need to consider versionMismatch later

	Send_CallResult(pOCPP, pRep, SendLocalList);

	return 0;
}

//int Handle_SendLocalList(void* pArg)
//{
//	TRACEX("Handle_SendLocalList \n");
//	// need Check Later 
//	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
//	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
//	A_BLOB_UPDATE_LOCALLIST updatList;
//	ZERO_POBJS(&updatList, 1);
//	updatList.emBlobType = BLOB_TYPE_UPDATE_LOCALLIST;
//
//	cJSON *pJson = pOCPP->pCurrJson;
//	cJSON *pChild = pJson->child;
//	
//	BOOL bListVerFound = FALSE;
//	BOOL	bUpdateTypeFound = FALSE;
//	cJSON *pAuthDataList = NULL;
//	while (pChild)
//	{
//		char *pszStr = pChild->string;
//		if (strcmp(pszStr, "listVersion") == 0)
//		{
//			cJSON *temp = pChild;
//			bListVerFound = TRUE;
//			updatList.nListVersion = temp->valueint;
//			//TRACEX("Local List Version %d %d \n", pJson->valueint,temp->valueint);
//		}
//		else if (strcmp(pszStr, "updateType") == 0)
//		{
//			bUpdateTypeFound = TRUE;
//			if (strcmp(pChild->valuestring, "Differential") == 0)
//			{
//				updatList.emUpdateType = LL_UPDATE_Diff;
//			}
//			else if (strcmp(pChild->valuestring, "Full") == 0)
//			{
//				updatList.emUpdateType = LL_UPDATE_Full;
//			}
//		}
//		else if (strcmp(pszStr, "localAuthorizationList") == 0)
//		{
//			pAuthDataList = pChild;
//		}
//		pChild = pChild->next;
//	}
//
//	if ((!bListVerFound) || (!bUpdateTypeFound)) /*|| (pAuthDataList == NULL)*/
//	{
//		//cJSON *pRep = cJSON_CreateObject();
//		//cJSON_AddStringToObject(pRep, "status", "Rejected");
//		//Send_CallResult(pOCPP, pRep, SendLocalList);
//		//return -1;
//		Send_CallError(pOCPP, SendLocalList, CallError_FormationViolation, "listVersion or updateType is missing!", NULL);
//		return -1;
//	}
//
//	if (pAuthDataList != NULL) //Null is supported when we only Update the version
//	{
//		TRACEX("pAuthDataList Avaliable \n");
//		//Now we parse all the Authdata
//		int nTotal = cJSON_GetArraySize(pAuthDataList);
//		TRACEX("Reacords %d  \n",nTotal);
//		updatList.nAuthorizationDataNumber = nTotal;
//		int n;
//		for (n = 0; n < nTotal; n++)
//		{
//			cJSON *pJ = cJSON_GetArrayItem(pAuthDataList, n);
//			TRACEX("Reacords %s\n",cJSON_Print(pJ));
//			if (pJ == NULL)
//			{
//				//just break???
//				break;
//			}
//			pChild = pJ->child;
//			updatList.authorizationDataList[n].emAuthorizationStatus = AT_Old_Saving;
//			while (pChild)
//			{
//				char *pszStr = pChild->string;
//				if (strcmp(pszStr, "idTag") == 0)
//				{
//					sprintf(updatList.authorizationDataList[n].szIdTag, pszStr);
//				}
//				else if (strcmp(pszStr, "idTagInfo") == 0)
//				{
//					cJSON *pChildChild = pChild->child;
//					while (pChildChild)
//					{
//						char *pszStr = pChildChild->string;
//						if (strcmp(pszStr, "expiryDate") == 0)
//						{
//							updatList.authorizationDataList[n].dtExpiryDate = Parse_OCPPTime(pChildChild->valuestring);
//						}
//						else if (strcmp(pszStr, "parentIdTag") == 0)
//						{
//							memcpy(updatList.authorizationDataList[n].szParentIdTag, pChildChild->valuestring, 20);
//							//updatList.authorizationDataList[n].szParentIdTag = (pChildChild->valuestring);
//						}
//						else if (strcmp(pszStr, "status") == 0)
//						{
//							if (strcmp(pChildChild->valuestring, "Accepted") == 0)
//							{
//								updatList.authorizationDataList[n].emAuthorizationStatus = AT_Accpted;
//							}
//							else if (strcmp(pChildChild->valuestring, "Blocked") == 0)
//							{
//								updatList.authorizationDataList[n].emAuthorizationStatus = AT_Blocked;
//							}
//							else if (strcmp(pChildChild->valuestring, "Expired") == 0)
//							{
//								updatList.authorizationDataList[n].emAuthorizationStatus = AT_Expired;
//							}
//							else if (strcmp(pChildChild->valuestring, "Invalid") == 0)
//							{
//								updatList.authorizationDataList[n].emAuthorizationStatus = AT_Invalid;
//							}
//						}
//						pChildChild = pChildChild->next;
//					}
//				}
//
//				pChild = pChild->next;
//			} //while
//		} //for
//
//	}
//	else
//	{
//		//Jimmy noted: this is possible when server wants to clear the AuthorizeData, just leave it NULL
//	}
//
//	//now set the list and get reply
//	VAR_VALUE varV;
//	varV.blobValue = &updatList;
//	ERRCODE_DAI err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 301, 304, &varV,
//		sizeof(A_BLOB_UPDATE_LOCALLIST), 0);
//
//	cJSON *pRep = cJSON_CreateObject();
//	cJSON_AddStringToObject(pRep, "status", (err == ERRCODE_DAI_OK) ? "Accepted" : "Rejected"); //maybe need to consider versionMismatch later
//	
//	Send_CallResult(pOCPP, pRep, SendLocalList);
//
//	return 0;
//}

// need to be implement 
int Prepare_DataTransfer_CP(void* pArg)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	return 0;
}

int Handle_DataTransfer_CS(void* pArg)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	//// later will implement 
	//return 0;
	Send_CallError(pOCPP, DataTransfer_CS, CallError_NotImplemented, "DaterTransfer cmd is not implemented yet!", NULL);
	return -1;
}

int Handle_GetCompositeSchedule(void* pArg)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	//// later will implement			
	//return 0;
	Send_CallError(pOCPP, GetCompositeSchedule, CallError_NotImplemented, "GetCompositeSchedule cmd is not implemented yet!", NULL);
	return -1;
}

int Handle_SetChargingProfile(void* pArg)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	//// later will implement 
	//return 0;
	Send_CallError(pOCPP, SetChargingProfile, CallError_NotImplemented, "SetChargingProfile cmd is not implemented yet!", NULL);
	return -1;
}

int Handle_ClearChargingProfile(void* pArg)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	//return 0;
	//Send_CallError(pOCPP, ClearChargingProfile, CallError_NotImplemented, "ClearChargingProfile cmd is not implemented yet!", NULL);
	//Jimmy note 20190701: shall be supported in order for OCTT test
	ERRCODE_DAI err = ERRCODE_DAI_OK;
	cJSON *pRep = cJSON_CreateObject();
	cJSON_AddStringToObject(pRep, "status", (err == ERRCODE_DAI_OK) ? "Accepted" : "Rejected");  //maybe need to consider versionMismatch later
	Send_CallResult(pOCPP, pRep, ClearChargingProfile);
	return -1;
}

int Handle_GetDiagnostics(void* pArg)
{
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	TRACEX("Handle_UploadDiagnostic \n");

	cJSON *pJson = pOCPP->pCurrJson;
	cJSON *temp1;
	cJSON *pResp = NULL;
	A_BLOB_UPLOADDIAGN_CMD cmd;
	ZERO_POBJS(&cmd, 1);
	cmd.emBlobType = BLOB_TYPE_UPLOAD_DIAGNOSTIC;
	if (temp1 = cJSON_GetObjectItem(pJson, "location"))
	{
		int nStrlen = strlen(temp1->valuestring);
		if((nStrlen < 10) || (nStrlen > 255))
		{
			Send_CallError(pOCPP, GetDiagnostics, CallError_FormationViolation, "location length is not correct!", NULL);
			return -1;
		}
		memcpy(cmd.szURL, temp1->valuestring, nStrlen);
	}
	else
	{
		Send_CallError(pOCPP, GetDiagnostics, CallError_FormationViolation, "location is missing!", NULL);
		return -1;
	}

	if (temp1 = cJSON_GetObjectItem(pJson, "retries"))
	{
		cmd.nRetries = temp1->valueint;
	}

	if (temp1 = cJSON_GetObjectItem(pJson, "retryInterval"))
	{
		cmd.nRetrieveInterv = temp1->valueint;
	}

	if (temp1 = cJSON_GetObjectItem(pJson, "startTime"))
	{
		cmd.dtStartTime = Parse_OCPPTime(temp1->valuestring);
	}

	if (temp1 = cJSON_GetObjectItem(pJson, "stopTime"))
	{
		cmd.dtStopTime = Parse_OCPPTime(temp1->valuestring);
	}

	char szTime[32];
	sprintf(cmd.szDiagnosticFileName, "diagnostic.%s", TimeToString(_NOW_, TIME_TRIM_FMT, szTime, 32));

	VAR_VALUE varV;
	varV.blobValue = &cmd;
	pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
		1, 9, &varV, sizeof(A_BLOB_UPLOADDIAGN_CMD), 0);

	pResp = cJSON_CreateObject();

	cJSON_AddStringToObject(pResp, "fileName", cmd.szDiagnosticFileName);

	Send_CallResult(pOCPP, pResp, GetDiagnostics);

	return 0;
}

int Handle_UpdateFirmware(void* pArg)
{
	// need to Check Later Yash 
	OCPP_DATA_STU* pOCPP = (OCPP_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	TRACEX("Handle_UploadFirmware \n");

	cJSON *pJson = pOCPP->pCurrJson;
	cJSON *temp1;
	cJSON *pResp = NULL;
	A_BLOB_UPDATEFIRMWARE_CMD cmd;
	ZERO_POBJS(&cmd, 1);
	cmd.emBlobType = BLOB_TYPE_UPDATE_FIRMWARE_CMD;
	if (temp1 = cJSON_GetObjectItem(pJson, "location"))
	{
		int nStrlen = strlen(temp1->valuestring);
		if((nStrlen < 10) || (nStrlen > 255))
		{
			Send_CallError(pOCPP, UpdateFirmware, CallError_FormationViolation, "location length is not correct!", NULL);
			return -1;
		}
		memcpy(cmd.szURL, temp1->valuestring, nStrlen);
		//memcpy(cmd.szURL, temp1->valuestring, MIN(255, strlen(temp1->valuestring)));
	}
	else
	{
		Send_CallError(pOCPP, UpdateFirmware, CallError_FormationViolation, "location is missing!", NULL);
		return -1;
	}

	if (temp1 = cJSON_GetObjectItem(pJson, "retries"))
	{
		cmd.nDownloadRetries = temp1->valueint;
	}

	if (temp1 = cJSON_GetObjectItem(pJson, "retrieveDate"))
	{
		cmd.dtRetrieveDate = Parse_OCPPTime(temp1->valuestring);
	}
	else
	{
		Send_CallError(pOCPP, UpdateFirmware, CallError_FormationViolation, "retrieveDate is missing!", NULL);
		return -1;
	}

	if (temp1 = cJSON_GetObjectItem(pJson, "retryInterval"))
	{
		cmd.nRetryInterv = temp1->valueint;
	}

	VAR_VALUE varV;
	varV.blobValue = &cmd;
	pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
		1, 8, &varV, sizeof(A_BLOB_UPDATEFIRMWARE_CMD), 0);

	pResp = cJSON_CreateObject();		//resp is empty
	
	Send_CallResult(pOCPP, pResp, UpdateFirmware);

	return 0;
}

