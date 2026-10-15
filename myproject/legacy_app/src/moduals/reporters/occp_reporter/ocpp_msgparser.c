#include "ocpp.h"


unsigned int  myAtoi(char *str)
{
	UINT res = 0; // Initialize result 

	// Iterate through all characters of input string and 
	// update result 
	int i;
	for (i = 0; str[i] != '\0'; ++i)
		res = res * 10 + str[i] - '0';

	// return result. 
	return res;
}

static OCPP_MSG_TYPE FindMsg_ByName(OCPP_DATA_STU* pOCPP, char* szName, MSG_CALL_TYPE callType)
{
	int n;
	for(n = 0; n < Max_OCPP_Msgs; n++)
	{
		//TRACEX("szName :%s \t pOCPP->stMsgInfoList[n].szMsgName : %s \n", szName, pOCPP->stMsgInfoList[n].szMsgName);
		if(strcmp(pOCPP->stMsgInfoList[n].szMsgName, szName) == 0)
		{
			if(((pOCPP->stMsgInfoList[n].emMsgType == DataTransfer_CP) && (callType != CallResult)) ||
				((pOCPP->stMsgInfoList[n].emMsgType == DataTransfer_CS) && (callType != Call)))
			{
				continue;
			}
			TRACEX("szName :%s \t pOCPP->stMsgInfoList[n].szMsgName : %s  %d \n", szName, pOCPP->stMsgInfoList[n].szMsgName, pOCPP->stMsgInfoList[n].emMsgType);
			return pOCPP->stMsgInfoList[n].emMsgType;
		}
	}
	return Msg_Unkown;
}

static BOOL FindMsg_ByUniqueid(OCPP_DATA_STU* pOCPP, unsigned char* szuniqueId)
{
	int n;
	UINT nUniqID = (UINT) myAtoi(szuniqueId); // don't use atoi() its issue
	
	for(n = 0; n < MIN(MAX_OCPP_SEND_RETRIES, pOCPP->activeCallMsg.nRetryTimes); n++)
	{
		//TRACEX("FindMsg_ByUniqueid Active messsage %lu  nUniqID %lu \n", pOCPP->activeCallMsg.nUniqMsgId[n], nUniqID);
		if(pOCPP->activeCallMsg.nUniqMsgId[n] == nUniqID)
		{
			TRACEX("FindMsg_ByUniqueid %d %d %d \n",n, pOCPP->activeCallMsg.nUniqMsgId[n], nUniqID);
			return TRUE;
		}
	}
	return FALSE;
}


int Parse_OCPPFrames(OCPP_DATA_STU* pOCPP)
{
	char *pszPos = pOCPP->szReadBuffer;
	char *pszEnd = NULL;

	OCPP_MSG_TYPE msgType;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	while (pOCPP->nReadBuffLen > 0)
	{
		cJSON *json = NULL;
		TRACE("IN Paser Function %s  %d \n", pOCPP->szReadBuffer,pOCPP->nReadBuffLen);
		json = cJSON_ParseWithOpts(pszPos,&pszEnd, FALSE);
		
		if (!json)
		{
			TRACEX("Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
			TRACEX(" Here IS the buffer %s \n", pszPos);
			return -1;
		}
		

		cJSON *arrCall = cJSON_GetArrayItem(json, 0);
		cJSON *arrUniqId = cJSON_GetArrayItem(json, 1);
		if((arrCall == NULL) || (arrUniqId == NULL))
		{
			TRACEX("88888Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
			TRACEX(" 888888Here IS the buffer %s \n", pszPos);
			cJSON_Delete(json);
			return -1;
		}

		TRACE("Recv_Packet: %s \n Next Object %s \n ", pszPos, pszEnd);

		if((arrCall->valueint == Call))
		{
			cJSON *arrMsgName = cJSON_GetArrayItem(json, 2);
			cJSON *arrContent = cJSON_GetArrayItem(json, 3);

			if ((arrContent == NULL) || (arrMsgName == NULL))
			{
				// Need to send Error Msg back to Server 
				//return -1;
				cJSON_Delete(json);		//do not forget to destroy the json
				TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszPos - pszEnd));
				pOCPP->nReadBuffLen -= (pszEnd - pszPos);
				TRACEX("Len pOCPP->nReadBuffLen : %d \n  ", pOCPP->nReadBuffLen);
				pszPos = pszEnd;
				continue;
			}

			msgType = FindMsg_ByName(pOCPP, arrMsgName->valuestring, Call); 

			if(pOCPP->emOCPPConnStatus != Ready)
			{
				//Jimmy note 20190215: UK ocpp test, it sends Boot.Accept along with ChangeConfig msg, so need to be able to handle this situation
				if(((pOCPP->emOCPPConnStatus == BootNotify_Pending) || pOCPP->bBootNotifyDone) && 
					((msgType == GetConfiguration) || (msgType == ChangeConfiguration) || (msgType == TriggerMessage)))
				{
					TRACEX("ServerCall And BootNotification_Pending\tMsg Type = %d\n", msgType);
					pOCPP->pCurrJson = arrContent;
					sprintf(pOCPP->szHandlingUniqID, "%s", arrUniqId->valuestring);
					//TRACEX("ServerCall And BootNotification_Pending\tszHandlinguniqueId= %s\n", pOCPP->szHandlingUniqID);
					pOCPP->stMsgInfoList[msgType].pfnMsgHandler(pOCPP);
				}
				//else
				//{
				//	cJSON_Delete(json);		//do not forget to destroy the json
				//	//TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszPos - pszEnd));
				//	pOCPP->nReadBuffLen -= (pszEnd - pszPos);
				//	//TRACEX("Len pOCPP->nReadBuffLen : %d \n  ", pOCPP->nReadBuffLen);
				//	pszPos = pszEnd;
				//	continue;
				//}
			}
			else
			{
				//now handling cmd
				pOCPP->pCurrJson = arrContent;
				sprintf(pOCPP->szHandlingUniqID, "%s", arrUniqId->valuestring);
				//JimmyNote 20190307: if use cJSON_Print, MUST free the string after done
				//TRACEX("!!!!!Server Call = Msg [%s], Cont = [%s]\n", pOCPP->stMsgInfoList[msgType].szMsgName,
				//	cJSON_Print(json));

				if(msgType != Msg_Unkown)
				{
					pOCPP->stMsgInfoList[msgType].pfnMsgHandler(pOCPP);
					pOCPP->tmLastSuccessExchange = _NOW_;
				}
				else
				{
					char szErrMsgName[256] = {0};
					sprintf(szErrMsgName, "Unkown Messege -> [%s]!", arrMsgName->valuestring);
					Send_CallError(pOCPP, Msg_Unkown, CallError_FormationViolation, szErrMsgName, NULL);
				}
			}
		}
		else if((arrCall->valueint == CallResult))
		{
			TRACEX("Recv CallResult \n");
			cJSON *arrContent = cJSON_GetArrayItem(json, 2);
			
			if ((arrCall == NULL) || (arrUniqId == NULL))
			{
				Send_CallError(pOCPP, Msg_Unkown, CallError_FormationViolation, "CallType or UniqId is missing", NULL);
				cJSON_Delete(json);		//do not forget to destroy the json
				TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszPos - pszEnd));
				pOCPP->nReadBuffLen -= (pszEnd - pszPos);
				TRACEX("Len pOCPP->nReadBuffLen : %d \n  ", pOCPP->nReadBuffLen);
				pszPos = pszEnd;
				continue;
			}

			if(!pOCPP->bHasActiveMsg)
			{
				//return FALSE;
				cJSON_Delete(json);		//do not forget to destroy the json
				TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszPos - pszEnd));
				pOCPP->nReadBuffLen -= (pszEnd - pszPos);
				TRACEX("Len pOCPP->nReadBuffLen : %d \n  ", pOCPP->nReadBuffLen);
				pszPos = pszEnd;
				continue;
			}

			TRACEX("Call Result unique ID = %s \n", arrUniqId->valuestring);
			if (!FindMsg_ByUniqueid(pOCPP,arrUniqId->valuestring))
			{
				TRACEX("&&&&&& Unique ID not matched");
				//return -1;
				cJSON_Delete(json);		//do not forget to destroy the json
				TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszPos - pszEnd));
				pOCPP->nReadBuffLen -= (pszEnd - pszPos);
				TRACEX("Len pOCPP->nReadBuffLen : %d \n  ", pOCPP->nReadBuffLen);
				pszPos = pszEnd;
				continue;
			}
			
			msgType = pOCPP->activeCallMsg.msgData.emMsgType;
			TRACEX("Recv Msg Type = %d Connector = %d\n", msgType, pOCPP->activeCallMsg.msgData.nBelongedConn);
			int connNo = pOCPP->activeCallMsg.msgData.nBelongedConn;

			if(msgType == Authorize) 
			{
				TRACEX("Recv Pkt Authorize: %s \n", pszPos);
				cJSON *idTagInfoObject = NULL;
				cJSON *temp1 = NULL;

				pOCPP->bAuthorizeInProcess = FALSE;

				memset(pOCPP->stAuthData.szParentIdTag, 0, 20);
				pOCPP->stAuthData.dtExpiryDate = 0;

				idTagInfoObject = cJSON_GetObjectItem(arrContent, "idTagInfo");

				if (idTagInfoObject == NULL)
				{
					// Just Discard the response so that request will be resent
					TRACEX("idTagInfo missing in Authorize Response \n");
					cJSON_Delete(json);		
					TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszEnd - pszPos));
					pOCPP->nReadBuffLen -= (pszEnd - pszPos);
					TRACEX("Len pOCPP->nReadBuffLen : %d \n  ", pOCPP->nReadBuffLen);
					pszPos = pszEnd;
					continue;
				}

				temp1 = cJSON_GetObjectItem(idTagInfoObject, "status");
				if (temp1 == NULL)
				{
					// Just Discard the response so that request will be resent
					TRACEX("Status missing in Authorize Response \n");
					cJSON_Delete(json);
					TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszEnd - pszPos));
					pOCPP->nReadBuffLen -= (pszEnd - pszPos);
					TRACEX("Len pOCPP->nReadBuffLen : %d \n  ", pOCPP->nReadBuffLen);
					pszPos = pszEnd;
					continue;
				}
				if (cJSON_String == temp1->type) // requuired to avoid the Recv Segmetation Fault
				{
					pOCPP->stAuthData.emAuthorizationStatus = AT_Accpted;
					if (strcmp(temp1->valuestring, "Accepted") == 0)
					{
						pOCPP->stAuthData.emAuthorizationStatus = AT_Accpted;
					}
					else if (strcmp(temp1->valuestring, "Blocked") == 0)
					{
						pOCPP->stAuthData.emAuthorizationStatus = AT_Blocked;
					}
					else if (strcmp(temp1->valuestring, "Expired") == 0)
					{
						pOCPP->stAuthData.emAuthorizationStatus = AT_Expired;
					}
					else if (strcmp(temp1->valuestring, "Invalid") == 0)
					{
						pOCPP->stAuthData.emAuthorizationStatus = AT_Invalid;
					}
					//else if (strcmp(temp1->valuestring, "ConcurrentTx") == NULL) //that could not happen
					//{
					//	pOCPP->stAuthData.emAuthorizationStatus = AT_ConcurrentTx;
					//}
				}
				if (temp1 = cJSON_GetObjectItem(idTagInfoObject, "parentIdTag"))
				{
					if (cJSON_String == temp1->type)
					{
						memcpy(pOCPP->stAuthData.szParentIdTag, temp1->valuestring, MIN(20, strlen(temp1->valuestring)));
					}
				}

				if (temp1 = cJSON_GetObjectItem(idTagInfoObject, "expiryDate"))
				{
					if (cJSON_String == temp1->type)
					{
						pOCPP->stAuthData.dtExpiryDate = Parse_OCPPTime(temp1->valuestring);
					}
				}

				//Now Call Set function to notify other moduals for the result
				VAR_VALUE varV;
				A_BLOB_UPDATE_AUTHSTAT_ANDCACHE update;

				memcpy(&update.newAuthData, &pOCPP->stAuthData, sizeof(AUTHORIZATION_DATA));

				update.emBlobType = BLOB_TYPE_UPDATE_AUTHSTATUS_ANDCACHE;
				update.emUpdateType = AUTH_UPDATE_AUTHORIZATION_REQ;
				varV.blobValue = &update;
				ERRCODE_DAI err = pReporter->pDAI->Set(
					(HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 301, 303, &varV,
					sizeof(A_BLOB_UPDATE_AUTHSTAT_ANDCACHE), 0);
				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("AuthorizationData  Uploading Failed \n");
				}
			}
			else if (msgType == BootNotification)
			{
				TRACEX("Recv Pkt BootNotification %s \n",pszPos);
				cJSON *temp1 = NULL;
				cJSON *pResp = arrContent;
				if (temp1 = cJSON_GetObjectItem(pResp, "currentTime"))
				{
					if (cJSON_String == temp1->type)
					{
						time_t tm = Parse_OCPPTime(temp1->valuestring);

						if (ABS(tm - _NOW_) > 1) //sync RTC
						{
							VAR_VALUE varV;
							varV.dtValue = tm;
							if(pOCPP->TimeCalibration)
							{
								pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 1, &varV, 4, 0);
							}
							
						}
					}

					if (temp1 = cJSON_GetObjectItem(pResp, "interval"))
					{
						if (cJSON_Number == temp1->type)
						{
							pOCPP->stOCPPSettings.nHearBeatInterv_RW = temp1->valueint;
							VAR_VALUE varV;
							pOCPP->stOCPPSettings.emBlobType = BLOB_TYPE_OCPP_CONFIGURATIONS;
							varV.blobValue = &(pOCPP->stOCPPSettings);
							pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
								1, 16, &varV, sizeof(A_BLOB_OCPP_CONFIGS), 1000);
						}
					}

					if (temp1 = cJSON_GetObjectItem(pResp, "status"))
					{
						if (cJSON_String == temp1->type) // required to Avoid Segmetation Fault 
						{
							if (strcmp(temp1->valuestring , "Accepted") == 0)
							{
								TRACEX("Recv Pkt BootNotification Accepted \n");
								pOCPP->bBootNotifyDone = TRUE;
							}
							else if (strcmp(temp1->valuestring, "Pending") == 0)
							{
								pOCPP->bBootNotifyDone = FALSE;
								pOCPP->emOCPPConnStatus = BootNotify_Pending;
							}
							else if (strcmp(temp1->valuestring, "Rejected") == 0)
							{
								pOCPP->bBootNotifyDone = FALSE;
								pOCPP->emOCPPConnStatus = BootNotify_Rejected;
							}
						}
					}

					if(pOCPP->emOCPPConnStatus == Ready)
					{
						//now should update OCPP heartbeat interval
						VAR_VALUE varV;
						pOCPP->stOCPPSettings.emBlobType = BLOB_TYPE_OCPP_CONFIGURATIONS;
						varV.blobValue = &(pOCPP->stOCPPSettings);
						pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 16, 
							&varV, sizeof(A_BLOB_OCPP_CONFIGS), 0);
					}
				}
			}
			else if(msgType == HeartBeat)
			{
				cJSON *temp1 = NULL;
				cJSON *pResp = arrContent;
				if (temp1 = cJSON_GetObjectItem(pResp, "currentTime"))
				{
					if (cJSON_String == temp1->type)
					{
						time_t tm = Parse_OCPPTime(temp1->valuestring);

						char szTime[32];
						char szTime1[32];
						TRACEX("Now time, is [%s], sync time is[%s]\n",
							TimeToString(_NOW_, TIME_CHN_FMT, szTime, 32),
							TimeToString(tm, TIME_CHN_FMT, szTime1, 32));
						if (ABS(tm - _NOW_) > 1) //sync RTC
						{
							VAR_VALUE varV;
							varV.dtValue = tm;
							if(pOCPP->TimeCalibration)
							{
								pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 1, &varV, 4, 0);
							}
							
						}
					}
				}
			}
			else if (msgType == StartTransaction)
			{
				TRACEX("Recv Pkt StartTransaction: %s\n", pszPos);
				BOOL bRejected = FALSE;

				int nGunIdx = pOCPP->activeCallMsg.msgData.nBelongedConn;
				
				int nTransID = 0;
				
				cJSON *idTagInfoObject = NULL;
				cJSON *temp1 = NULL;
				
				AUTHORIZATION_DATA authData;
				ZERO_POBJS(&authData, 1);
				idTagInfoObject = cJSON_GetObjectItem(arrContent, "idTagInfo");

				if (idTagInfoObject == NULL)
				{
					// Just Discard the response so that request will be resent
					TRACEX("idTagInfo missing in StartTrans Response \n");
					cJSON_Delete(json);
					TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszEnd - pszPos));
					pOCPP->nReadBuffLen -= (pszEnd - pszPos);
					TRACEX("Len pOCPP->nReadBuffLen : %d \n  ", pOCPP->nReadBuffLen);
					pszPos = pszEnd;
					continue;
				}

				temp1 = cJSON_GetObjectItem(idTagInfoObject, "status");

				if (temp1 == NULL)
				{
					// Just Discard the response so that request will be resent
					TRACEX("Status missing in StartTransaction Response \n");
					cJSON_Delete(json);
					TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszEnd - pszPos));
					pOCPP->nReadBuffLen -= (pszEnd - pszPos);
					TRACEX("Len pOCPP->nReadBuffLen : %d \n  ", pOCPP->nReadBuffLen);
					pszPos = pszEnd;
					continue;
				}
				if (cJSON_String == temp1->type)
				{
					if (strcmp(temp1->valuestring, "Accepted") == 0)
					{
						authData.emAuthorizationStatus = AT_Accpted;
					}
					else if (strcmp(temp1->valuestring, "Blocked") == 0)
					{
						authData.emAuthorizationStatus = AT_Blocked;
						bRejected = TRUE;
					}
					else if (strcmp(temp1->valuestring, "Expired") == 0)
					{
						authData.emAuthorizationStatus = AT_Expired;
						bRejected = TRUE;
					}
					else if (strcmp(temp1->valuestring, "Invalid") == 0)
					{
						authData.emAuthorizationStatus = AT_Invalid;
						bRejected = TRUE;
					}
					else if (strcmp(temp1->valuestring, "ConcurrentTx") == 0)
					{
						authData.emAuthorizationStatus = AT_ConcurrentTx;
						//bRejected = TRUE;
					}
				}

				if (temp1 = cJSON_GetObjectItem(idTagInfoObject, "parentIdTag"))
				{
					if (cJSON_String == temp1->type)
					{
						memcpy(authData.szParentIdTag, temp1->valuestring, MIN(20, strlen(temp1->valuestring)));
					}
				}

				if (temp1 = cJSON_GetObjectItem(idTagInfoObject, "expiryDate"))
				{
					if (cJSON_String == temp1->type)
					{
						authData.dtExpiryDate = Parse_OCPPTime(temp1->valuestring);
					}

				}

				temp1 = cJSON_GetObjectItem(arrContent, "transactionId");
				if (cJSON_Number == temp1->type)
				{
					nTransID = temp1->valueint;
				}
				else
				{
					TRACEX("transactionId missing in StartTransaction Response \n");
					cJSON_Delete(json);
					TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszEnd - pszPos));
					pOCPP->nReadBuffLen -= (pszEnd - pszPos);
					TRACEX("Len pOCPP->nReadBuffLen : %d \n  ", pOCPP->nReadBuffLen);
					pszPos = pszEnd;
					continue;
				}

				//Jimmy note 20190413: even accepted, should update the cache, because the idTag may come from server remoteStart.req
				//Update auth data for local list/cache
				VAR_VALUE varV;
				A_BLOB_UPDATE_AUTHSTAT_ANDCACHE update;
				//get idTag from runInfo
				A_BLOB_RUN_CHARGE_INFO  runChgInfo;
				PARAM_VAL_GET valGet;
				valGet.nGetSize = sizeof(A_BLOB_RUN_CHARGE_INFO);
				valGet.val.blobValue = (BYTE*)&runChgInfo;
				pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
					201 + nGunIdx, 107 , &valGet, sizeof(PARAM_VAL_GET));
				memcpy(authData.szIdTag, runChgInfo.szUserID, 20);
				memcpy(&update.newAuthData, &authData, sizeof(AUTHORIZATION_DATA));
				update.emBlobType = BLOB_TYPE_UPDATE_AUTHSTATUS_ANDCACHE;
				update.emUpdateType = AUTH_UPDATE_STARTTHANS_CONF;
				update.emWhichConn_2Update = (UINT)nGunIdx;
				varV.blobValue = &update;
				pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 301, 303, &varV,
					sizeof(A_BLOB_UPDATE_AUTHSTAT_ANDCACHE), 0);

				if (bRejected)
				{
					TRACEX("Transction should be blocked, bStopTransOnInvalid = %d, IsActive = %d \n",
						pOCPP->stOCPPSettings.bStopTransOnInvalidId_RW, (pOCPP->activeCallMsg.msgData.emSrc == CALL_SRC_InProcessTrans));
					//Pause Deliver the Energy for invalidId card
					if(pOCPP->stOCPPSettings.bStopTransOnInvalidId_RW && (pOCPP->activeCallMsg.msgData.emSrc == CALL_SRC_InProcessTrans))
					{
						//if (pOCPP->stConnectorData[connNo].emConnctorStauts == Conn_InCharging)
						//in case it is historical TRANS Session, we do not need to care
						if(pOCPP->stConnectorData[nGunIdx].blobInHandling_ChargeInfo.tmChargeStart == pOCPP->activeCallMsg.msgData.tmRelatedTransStart)
						{
							A_BLOB_CHARGE_STOP_CMD  RemoteStop;
							RemoteStop.emBlobType = BLOB_TYPE_CHARGE_STOP_CMD;
							RemoteStop.nStopBy = CHARGE_STOP_BY_SERVER;
							RemoteStop.nStopReason = CHARGE_STOP_REASON_DEAUTH;

							varV.blobValue = &RemoteStop;
							pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
								201 + connNo, 102, &varV, sizeof(RemoteStop), 0);

							//Jimmy Note 20181208: according to OCPP doc(use case) we only pause the transaction, shall authrize again for stopping it
							//VAR_VALUE varV;
							//varV.emValue = 1;
							//pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
							//	201 + connNo, 110, &varV, 4, 1000);
						}
					}
				}

				//Now Call Set function to notify other moduals for the result
				A_BLOB_UPDATE_CHARGEMETER_STAT update_Meter;
				update_Meter.emBlobType = BLOB_TYPE_UPDATE_CHARG_STAT_METER;
				update_Meter.emSenderType = NOTIFY_BY_OCPP_REPORTER;
				update_Meter.emUpdateType = CMUPDATE_StartTrans;
				update_Meter.nGunIdx = nGunIdx + 1;
				update_Meter.nTransactionID = nTransID;
				update_Meter.tmChargeStart = pOCPP->activeCallMsg.msgData.tmRelatedTransStart;
				varV.blobValue = &update_Meter;
				if(pOCPP->activeCallMsg.msgData.emSrc == CALL_SRC_HisTrans)
				{
					TRACE("----Received startTrans-conf, so send back to sysSampler,nNewTrans = %d-----\n", nTransID);
					pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 1, 7, &varV,
						sizeof(A_BLOB_UPDATE_CHARGEMETER_STAT), 0);
					pOCPP->recInHandling_HistoryChargeRec.nSessionID = nTransID;
				}
				else
				{
					TRACE("----Received startTrans-conf, so send back to CCUSampler, nNewTrans = %d, tmRelatedTransStart = %d-----\n", nTransID, update_Meter.tmChargeStart);
					pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 201 + nGunIdx, 105, &varV,
						sizeof(A_BLOB_UPDATE_CHARGEMETER_STAT), 0);
					pOCPP->stConnectorData[connNo].blobInHandling_ChargeInfo.nSessionID = nTransID;
				}
			}
			else if (msgType == StopTransaction) 
			{
				TRACEX("Recv Pkt StopTransaction: %s \n", pszPos);
				cJSON *temp1 = NULL;
				cJSON *idTagInfoObject = NULL;
				VAR_VALUE varV;
			
				idTagInfoObject = cJSON_GetObjectItem(arrContent, "idTagInfo");
				if(idTagInfoObject != NULL)
				{
					if (cJSON_Object == idTagInfoObject->type)
					{
						A_BLOB_UPDATE_AUTHSTAT_ANDCACHE update;
						ZERO_POBJS(&update, 1);
						temp1 = cJSON_GetObjectItem(idTagInfoObject, "status");
						if (temp1 == NULL)
						{
							// Just Discard the response so that request will be resent
							TRACEX("Status missing in StopTransaction Response \n");
							cJSON_Delete(json);
							TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszEnd - pszPos));
							pOCPP->nReadBuffLen -= (pszEnd - pszPos);
							TRACEX("Len pOCPP->nReadBuffLen : %d \n  ", pOCPP->nReadBuffLen);
							pszPos = pszEnd;
							continue;
						}
						if (cJSON_String == temp1->type)
						{
							update.newAuthData.emAuthorizationStatus = AT_Accpted;
							if (strcmp(temp1->valuestring, "Accepted") == 0)
							{
								update.newAuthData.emAuthorizationStatus = AT_Accpted;

							}
							else if (strcmp(temp1->valuestring, "Blocked") == 0)
							{
								update.newAuthData.emAuthorizationStatus = AT_Blocked;

							}
							else if (strcmp(temp1->valuestring, "Expired") == 0)
							{
								update.newAuthData.emAuthorizationStatus = AT_Expired;

							}
							else if (strcmp(temp1->valuestring, "Invalid") == 0)
							{
								update.newAuthData.emAuthorizationStatus = AT_Invalid;

							}
						}
						//else if (strcmp(temp1->valuestring, "ConcurrentTx") == 0) //should not happen
						//{
						//	pOCPP->stAuthData.emAuthorizationStatus = AT_ConcurrentTx;
						//}

						if (temp1 = cJSON_GetObjectItem(idTagInfoObject, "parentIdTag"))
						{
							if (cJSON_String == temp1->type)
							{
								memcpy(update.newAuthData.szParentIdTag, temp1->valuestring, MIN(20, strlen(temp1->valuestring)));
							}
						}

						if (temp1 = cJSON_GetObjectItem(idTagInfoObject, "expiryDate"))
						{
							if (cJSON_String == temp1->type)
							{
								update.newAuthData.dtExpiryDate = Parse_OCPPTime(temp1->valuestring);
							}
						}

						//Now Call Set function to notify other moduals for the result
						memcpy(&update, &pOCPP->stAuthData, sizeof(AUTHORIZATION_DATA));
						update.emBlobType = BLOB_TYPE_UPDATE_AUTHSTATUS_ANDCACHE;
						update.emUpdateType = AUTH_UPDATE_STOPTRANS_CONF;
						varV.blobValue = &update;
						pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 301, 303, &varV,
							sizeof(A_BLOB_UPDATE_AUTHSTAT_ANDCACHE), 0);
					}
				}
				//update to CCU sampler
				A_BLOB_UPDATE_CHARGEMETER_STAT update;
				int nGunIdx = pOCPP->activeCallMsg.msgData.nBelongedConn;
				update.emBlobType = BLOB_TYPE_UPDATE_CHARG_STAT_METER;
				update.emSenderType = NOTIFY_BY_OCPP_REPORTER;
				update.emUpdateType = CMUPDATE_StopTrans;
				update.nGunIdx = nGunIdx + 1;
				update.tmChargeStart = pOCPP->activeCallMsg.msgData.tmRelatedTransStart;
				varV.blobValue = &update;
				if(pOCPP->activeCallMsg.msgData.emSrc == CALL_SRC_HisTrans)
				{
					TRACE("----Received stop-conf, so send back to sysSampler-----\n");
					pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 1, 7, &varV,
						sizeof(A_BLOB_UPDATE_CHARGEMETER_STAT), 0);
				}
				else
				{
					TRACE("----Received stop-conf, so send back to CCUSampler-----\n");
					pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 201 + nGunIdx, 105, &varV,
						sizeof(A_BLOB_UPDATE_CHARGEMETER_STAT), 0);
				}

			}
			else if (msgType == MeterValues)
			{
				// empty Response 
				if(pOCPP->activeCallMsg.msgData.emSrc == CALL_SRC_Normal)
				{
					//if(pOCPP->activeCallMsg.msgData.nBelongedConn == Connctor_0)
					//{
					//	pOCPP->tmLastSuccessSent_ClockAlignMeterval = _NOW_;
					//}
					//else
					//{
					//	pOCPP->tmLastSuccessSent_ClockAlignMeterval = _NOW_;
					//}
				}
				else //if(pOCPP->activeCallMsg.msgData.emSrc == CALL_SRC_InProcessTrans)
				{
					//update to CCU sampler
					VAR_VALUE varV;
					A_BLOB_UPDATE_CHARGEMETER_STAT update;
					int nGunIdx = pOCPP->activeCallMsg.msgData.nBelongedConn;
					update.emBlobType = BLOB_TYPE_UPDATE_CHARG_STAT_METER;
					update.emSenderType = NOTIFY_BY_OCPP_REPORTER;
					update.emUpdateType = CMUPDATE_SampleMeterVal;
					update.nGunIdx = nGunIdx + 1;
					update.tmMeterVal_Sampled = pOCPP->activeCallMsg.msgData.tmSampled; //this is very important for CCU sampler to identify the time and update DB
					update.tmChargeStart = pOCPP->activeCallMsg.msgData.tmRelatedTransStart;
					varV.blobValue = &update;
					if(pOCPP->activeCallMsg.msgData.emSrc == CALL_SRC_HisTrans)
					{
						TRACE("----Received meter-conf, so send back to sysSampler-----\n");
						pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 1, 7, &varV,
							sizeof(A_BLOB_UPDATE_CHARGEMETER_STAT), 0);
					}
					else
					{
						TRACE("----Received meter-conf, so send back to CCUSampler-----\n");
						pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 201 + nGunIdx, 105, &varV,
							sizeof(A_BLOB_UPDATE_CHARGEMETER_STAT), 0);
					}
				}
			}
			else if (msgType == StatusNotification)
			{
				// empty Response 
				TRACEX("Status Response Recv for conn[%d] \n", pOCPP->activeCallMsg.msgData.nBelongedConn);
				//Jimmy note20190301:There will be problem if we set status here, so replace it in Thread main entry function
				//if(pOCPP->activeCallMsg.msgData.nBelongedConn == Connctor_0)
				//{
				//	pOCPP->bCPStatusChanged = FALSE;
				//}
				//else
				//{
				//	int nGunIdx = pOCPP->activeCallMsg.msgData.nBelongedConn;
				//	if(nGunIdx > Connctor_0 && (nGunIdx < Max_Connctors))
				//	{
				//		//pOCPP->stConnectorData[nGunIdx - 1].bConn_StatusChanged = FALSE;
				//		if(pOCPP->stConnectorData[nGunIdx].bConn_StatusChanged > 0)
				//		{
				//			pOCPP->stConnectorData[nGunIdx].bConn_StatusChanged--;
				//		}
				//	}
				//}
			}
			else if (msgType == DiagnosticsStatusNotification)
			{
				// empty Response 
				if((pOCPP->emDiagnosUploadStatus == DIAGNOUPLOAD_Uploaded) || 
					(pOCPP->emDiagnosUploadStatus == DIAGNOUPLOAD_UploadFailed))
				{
					pOCPP->emDiagnosUploadStatus = DIAGNOUPLOAD_Idle;
				}
			}
			else if (msgType == FirmwareStatusNotification)
			{
				// empty Response 
				if((pOCPP->emFirmwareUpgradeStatus == FIRMUPDATE_InstallFailed) || 
					(pOCPP->emFirmwareUpgradeStatus == FIRMUPDATE_Installed) ||
					pOCPP->emFirmwareUpgradeStatus == FIRMUPDATE_DownloadFailed)
				{
					pOCPP->emFirmwareUpgradeStatus = FIRMUPDATE_Idle;
				}
			}
			pOCPP->bHasActiveMsg = FALSE;
			pOCPP->nConnErrTimes = 0;
			pOCPP->tmLastSuccessExchange = _NOW_;
		}
		else //CallError
		{
			// [<MessageTypeId>, ��<UniqueId>��, ��<errorCode>��, ��<errorDescription>��,{<errorDetails>}]
			cJSON *json_uniqId = cJSON_GetArrayItem(json, 1);
			cJSON *json_errorCode = cJSON_GetArrayItem(json, 2);
			cJSON *json_errorDescription = cJSON_GetArrayItem(json, 3);
			//cJSON *json_errorDetails = cJSON_GetArrayItem(json,4);

			if((json_uniqId != NULL) && (json_errorCode != NULL) && (json_errorDescription != NULL))
			{
				//static const char *Error_Code[] = 
				//{
				//	"NotImplemented",
				//	"NotSupported",
				//	"InternalError",
				//	"ProtocolError",
				//	"SecurityError",
				//	"FormationViolation",
				//	"PropertyConstraintViolation",
				//	"OccurenceConstraintViolation",
				//	"TypeConstraintViolation",
				//	"GenericError",
				//};

				//int m;
				//BOOL bIsErrMsg = FALSE;
				//if(json_errorCode->valuestring != NULL)
				//{
				//	for(m = 0;m < ITEM_OF(Error_Code); m++)
				//	{
				//		if(strcmp(Error_Code[m], json_errorCode->valuestring) == 0)
				//		{
				//			bIsErrMsg = TRUE;
				//			break;
				//		}
				//	}
				//}

				//if(bIsErrMsg)
				{
					if(pOCPP->bHasActiveMsg)
					{
						if(FindMsg_ByUniqueid(pOCPP, json_uniqId->valuestring))
						{
							LogOut(MAIN_MODULE_REPORTER, "OCPP", LOG_TYPE_ERROR,
								"Received MsgCallError from CS for [%s], errCode[%s], ErrDescription = [%s]", 
								pOCPP->stMsgInfoList[pOCPP->activeCallMsg.msgData.emMsgType].szMsgName, 
								json_errorCode->valuestring,
								json_errorDescription->valuestring);

							pOCPP->activeCallMsg.msgData.nMsgErrorRecievedCount++;

							//here we handle the Trans-Related msg error attempts
							if((pOCPP->activeCallMsg.msgData.emMsgType == StartTransaction) || 
								(pOCPP->activeCallMsg.msgData.emMsgType == StopTransaction) ||
								((pOCPP->activeCallMsg.msgData.emMsgType == MeterValues) && (pOCPP->activeCallMsg.msgData.emSrc != CALL_SRC_Normal)))
							{
								pOCPP->activeCallMsg.msgData.tmLastErrorRespArrived = _NOW_;
								if(pOCPP->activeCallMsg.msgData.nMsgErrorRecievedCount < pOCPP->stOCPPSettings.nTransactionMessageAttempts_RW)
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
								else //only try specified times, or else just omit the msg
								{
									//update to CCU sampler
									VAR_VALUE varV;
									A_BLOB_UPDATE_CHARGEMETER_STAT update;
									int nGunIdx = pOCPP->activeCallMsg.msgData.nBelongedConn;
									update.emBlobType = BLOB_TYPE_UPDATE_CHARG_STAT_METER;
									update.emSenderType = NOTIFY_BY_OCPP_REPORTER;
									update.emUpdateType = CMUPDATE_Omited; //should be omitted
									update.nGunIdx = nGunIdx + 1;
									update.tmChargeStart = pOCPP->activeCallMsg.msgData.tmRelatedTransStart;
									varV.blobValue = &update;
									if(pOCPP->activeCallMsg.msgData.emSrc == CALL_SRC_HisTrans)
									{
										TRACE("----Received error trans-related-conf, so send back to sysSampler-----\n");
										pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 1, 7, &varV,
											sizeof(A_BLOB_UPDATE_CHARGEMETER_STAT), 0);
									}
									else
									{
										TRACE("----Received error trans-related-conf, so send back to sysSampler-----\n");
										pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 201 + nGunIdx, 105, &varV,
											sizeof(A_BLOB_UPDATE_CHARGEMETER_STAT), 0);
									}
								}
							}
							else if (pOCPP->activeCallMsg.msgData.emMsgType == Authorize)
							{
								//Jimmy added 20190702 for OCTT test: when received CallError, shall set this to FALSE
								//or else another Auth shall not be started any longer
								pOCPP->bAuthorizeInProcess = FALSE;
							}							

							//no need to send it again in case error happen again;
							pOCPP->bHasActiveMsg = FALSE;
							pOCPP->nConnErrTimes = 0;
						}
					}
					else
					{
						//just discard it?
					}
				}
			}
		}

		cJSON_Delete(json);		//do not forget to destroy the json
		//now continue parsing for another frame

		TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszPos - pszEnd));
		pOCPP->nReadBuffLen -= (pszEnd - pszPos);
		TRACEX("Len pOCPP->nReadBuffLen : %d \n  ", pOCPP->nReadBuffLen);
		pszPos = pszEnd;
	}

	return 0;
}

