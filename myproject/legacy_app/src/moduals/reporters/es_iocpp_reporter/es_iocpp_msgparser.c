#include "es_iocpp.h"


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

static ES_iOCPP_MSG_TYPE FindMsg_ByMsgId(ES_iOCPP_DATA_STU* pOCPP, int nMsgId, MSG_CALL_TYPE callType)
{
	int n;
	for(n = 0; n < Max_ES_iOCPP_Msgs; n++)
	{
		//TRACEX("szName :%s \t pOCPP->stMsgInfoList[n].szMsgName : %s \n", szName, pOCPP->stMsgInfoList[n].szMsgName);
		//if(strcmp(pOCPP->stMsgInfoList[n].szMsgName, szName) == 0)
		if(pOCPP->stMsgInfoList[n].nMsgId == nMsgId)
		{
			// if((pOCPP->stMsgInfoList[n].emMsgType == DataTransfer_CP) && (callType != CallResult) ||
			// 	((pOCPP->stMsgInfoList[n].emMsgType == DataTransfer_CS) && (callType != Call)))
			// {
			// 	continue;
			// }
			TRACEX("callType = %d, pOCPP->stMsgInfoList[%d].emMsgType\n", 
				callType, n,pOCPP->stMsgInfoList[n].emMsgType);

			return pOCPP->stMsgInfoList[n].emMsgType;
		}
	}

	return Msg_Unkown;
}

static BOOL FindMsg_ByUniqueid(ES_iOCPP_DATA_STU* pOCPP, unsigned char* szuniqueId)
{
	int n;
	UINT nUniqID = (UINT) myAtoi(szuniqueId); // don't use atoi() its issue
	
	for(n = 0; n < MIN(MAX_ES_iOCPP_SEND_RETRIES, pOCPP->activeCallMsg.nRetryTimes); n++)
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


int Parse_ESiOCPPFrames(ES_iOCPP_DATA_STU* pOCPP)
{
	char *pszPos = pOCPP->szReadBuffer;
	char *pszEnd = NULL;

	ES_iOCPP_MSG_TYPE msgType;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	MSG_CALL_TYPE  emCallType = 0;
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
		
		cJSON *sectionId = cJSON_GetObjectItem(json, "id");
		cJSON *sectionUniqKey = cJSON_GetObjectItem(json, "uniqKey");
		cJSON *sectionContent = cJSON_GetObjectItem(json, "content");
		if((sectionId == NULL) || (sectionUniqKey == NULL) || (sectionContent == NULL))
		{
			TRACEX("11111Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
			TRACEX(" 11111118Here IS the buffer %s \n", pszPos);
			cJSON_Delete(json);
			return -1;
		}
		else if((cJSON_String != sectionId->type) || (cJSON_String != sectionUniqKey->type) ||
			(cJSON_Object != sectionContent->type))
		{
			TRACEX("88888Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
			TRACEX(" 8888888Here IS the buffer %s \n", pszPos);
			cJSON_Delete(json);
			return -1;
		}

		if((sectionId->valuestring == NULL) || (sectionUniqKey->valuestring == NULL))
		{
			TRACEX("22222Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
			TRACEX(" 22222222Here IS the buffer %s \n", pszPos);
			cJSON_Delete(json);
			return -1;
		}

		if(strlen(sectionId->valuestring) != 4)
		{
			TRACEX("333333Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
			TRACEX(" 3333333333Here IS the buffer %s \n", pszPos);
			cJSON_Delete(json);
			return -1;
		}

		//now parse the id Section
		char c0 = *sectionId->valuestring;
		if(c0 == 'Q')
		{
			emCallType = Call;
		}
		else if(c0 == 'R')
		{
			emCallType = CallResult;
		}
		else if(c0 == 'E')
		{
			emCallType = CallError;
		}
		else
		{
			TRACEX("4444Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
			TRACEX(" 4444Here IS the buffer %s \n", pszPos);
			cJSON_Delete(json);
			return -1;
		}

		int nMsgId = myAtoi(sectionId->valuestring + 1);
		
		TRACE("Now get the MsgId->%d, callType->%d, Recv_Packet: %s \n Next Object %s \n ", nMsgId, emCallType, pszPos, pszEnd);

		if((emCallType == Call))
		{
			msgType = FindMsg_ByMsgId(pOCPP, nMsgId, Call); 
			TRACE("msgType =%d nMsgId=%d\n", msgType, nMsgId);
			if(pOCPP->emOCPPConnStatus != Ready)
			{
				//Jimmy note 20190215: UK ocpp test, it sends Boot.Accept along with ChangeConfig msg, so need to be able to handle this situation
				if((/*(pOCPP->emOCPPConnStatus == BootNotify_Pending) || */pOCPP->bBootNotifyDone) && 
					((msgType == S_411_RmtGetConfig) || (msgType == S_412_RmtSetConfig) || (msgType == S_111_Trigger)))
				{
					TRACEX("ServerCall And BootNotification_Pending\tMsg Type = %d\n", msgType);
					pOCPP->pCurrJson = sectionContent;
					snprintf(pOCPP->szHandlingUniqID, 36, "%s", sectionUniqKey->valuestring);
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
				pOCPP->pCurrJson = sectionContent;
				snprintf(pOCPP->szHandlingUniqID, 36, "%s", sectionUniqKey->valuestring);
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
					snprintf(szErrMsgName, 256, "Unkown Messege -> [%s]!", sectionId->valuestring);
					Send_CallError(pOCPP, Msg_Unkown, CallError_FormatError, szErrMsgName);
				}
			}
		}
		else if((emCallType == CallResult))
		{
			TRACEX("Recv CallResult \n");

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

			TRACEX("Call Result unique ID = %s \n", sectionUniqKey->valuestring);
			if (!FindMsg_ByUniqueid(pOCPP, sectionUniqKey->valuestring))
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
			if(pOCPP->stMsgInfoList[msgType].nMsgId != nMsgId)
			{
				TRACEX("&&&&&& MsgId ID not matched");
				//return -1;
				cJSON_Delete(json);		//do not forget to destroy the json
				TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszPos - pszEnd));
				pOCPP->nReadBuffLen -= (pszEnd - pszPos);
				TRACEX("Len pOCPP->nReadBuffLen : %d \n  ", pOCPP->nReadBuffLen);
				pszPos = pszEnd;
				continue;
			}
			TRACEX("Recv Msg Type = %d Connector = %d\n", msgType, pOCPP->activeCallMsg.msgData.nBelongedConn);
			//int connNo = pOCPP->activeCallMsg.msgData.nBelongedConn;

			if(msgType == C_100_BootNotif)
			{
				TRACEX("Recv Pkt BootNotification %s \n",pszPos);
				cJSON *temp1 = NULL;
				if (temp1 = cJSON_GetObjectItem(sectionContent, "time"))
				{
					if (cJSON_String == temp1->type)
					{
						if (pOCPP->lcCfg.lcType == LC_TYPE_1st)
						{
							time_t tm = Parse_ESiOCPPTime(temp1->valuestring);

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

				if (temp1 = cJSON_GetObjectItem(sectionContent, "accept"))
				{
					if (cJSON_Number == temp1->type) // required to Avoid Segmetation Fault 
					{
						if (temp1->valueint == 1)
						{
							pOCPP->bBootNotifyDone = TRUE;
						}
						else
						{
							pOCPP->bBootNotifyDone = FALSE;
							pOCPP->emOCPPConnStatus = BootNotify_Rejected;
						}
						TRACE("Info: temp1->valueint=%d bBootNotifyDone=%d\n", temp1->valueint, pOCPP->bBootNotifyDone);
					}
				}

				if (temp1 = cJSON_GetObjectItem(sectionContent, "hb_interv"))
				{
					if (cJSON_Number == temp1->type) // required to Avoid Segmetation Fault 
					{
						int nHearBeat = temp1->valueint;
						if ((nHearBeat >= 5) && (nHearBeat <= 300))
						{
							//pOCPP->blobBackendSet.nChargerStatusReportInterval = nHearBeat;
						}					
					}
				}
				
				// if(pOCPP->emOCPPConnStatus == Ready)
				// {
				// 	//now should update OCPP heartbeat interval
				// 	VAR_VALUE varV;
				// 	pOCPP->stOCPPSettings.emBlobType = BLOB_TYPE_OCPP_CONFIGURATIONS;
				// 	varV.blobValue = &(pOCPP->stOCPPSettings);
				// 	pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 16, 
				// 		&varV, sizeof(A_BLOB_OCPP_CONFIGS), 0);
				// }
			}
			 
			else if (msgType == C_101_StatusNotif)
			{
				// empty Response 
				TRACEX("Status Response Recv for conn[%d] \n", pOCPP->activeCallMsg.msgData.nBelongedConn);
		 
				cJSON *temp1 = NULL;
				//VAR_VALUE varV;
				if (temp1 = cJSON_GetObjectItem(sectionContent, "time"))
				{
					if (cJSON_String == temp1->type)
					{
						if (pOCPP->lcCfg.lcType == LC_TYPE_1st)
						{
							time_t tm = Parse_ESiOCPPTime(temp1->valuestring);

							if (ABS(tm - _NOW_) > 3) //sync RTC
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
			}
			else if (msgType == C_407_StatOfDiag)
			{
				// empty Response 
				if((pOCPP->emDiagnosUploadStatus == DIAGNOUPLOAD_Uploaded) || 
					(pOCPP->emDiagnosUploadStatus == DIAGNOUPLOAD_UploadFailed))
				{
					pOCPP->emDiagnosUploadStatus = DIAGNOUPLOAD_Idle;
				}
			}
			else if (msgType == C_405_StatOfFirmw)
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

			if((msgType == S_401_RmtRest) || (msgType == S_402_RmtChgEVESEAbi) || (msgType == S_404_RmtUpdtFirmw) ||
			(msgType == S_406_RmtCallDiag) ||(msgType == S_411_RmtGetConfig) || (msgType == S_412_RmtSetConfig) || 
			(msgType == S_503_RmtGetEMSPlan) ||(msgType == S_504_RmtSetEMSPlanProfile) || 
			(msgType == S_601_RmtGetFeeRateCfg) || (msgType == S_602_RmtSetFeeRateProfile))
			{
				TRACED("Kuangyue4 OCPP pReporter=%p, type=%d, data='%s'",
					pReporter,
					DUMP_TYPE_READ,
					pOCPP->szReadBuffer);
				LOG_INFO("IOCPP pReporter=%p, type=%d, data='%s'",
					pReporter,
					DUMP_TYPE_READ,
					pOCPP->szReadBuffer);//作为回复报文的回针数据
			}//edit by kuangyue 20250318
		}
		else //CallError
		{
			if(pOCPP->bHasActiveMsg && (nMsgId == pOCPP->stMsgInfoList[pOCPP->activeCallMsg.msgData.emMsgType].nMsgId))
			{
				if(FindMsg_ByUniqueid(pOCPP, sectionUniqKey->valuestring))
				{
					cJSON *temp1 = NULL;
					int nErrCode = 0;
					if (temp1 = cJSON_GetObjectItem(sectionContent, "code"))
					{
						nErrCode = temp1->valueint;
					}

					char *errDesp = "";
					if (temp1 = cJSON_GetObjectItem(sectionContent, "err_desp"))
					{
						if(cJSON_String == temp1->type)
						{
							errDesp = temp1->valuestring;
						}
					}

					LogOut(MAIN_MODULE_REPORTER, "OCPP", LOG_TYPE_ERROR,
						"Received MsgCallError from CS for [%d], errCode[%s], ErrDescription = [%s]", 
						pOCPP->stMsgInfoList[pOCPP->activeCallMsg.msgData.emMsgType].nMsgId, 
						nErrCode, errDesp);

					pOCPP->activeCallMsg.msgData.nMsgErrorRecievedCount++;

					//here we handle the Trans-Related msg error attempts
				 

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

		cJSON_Delete(json);		//do not forget to destroy the json
		//now continue parsing for another frame

		TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszPos - pszEnd));
		pOCPP->nReadBuffLen -= (pszEnd - pszPos);
		TRACEX("Len pOCPP->nReadBuffLen : %d \n  ", pOCPP->nReadBuffLen);
		pszPos = pszEnd;
	}

	return 0;
}

