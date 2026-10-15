#include "maitain_be.h"


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

static MAINTAINBE_MSG_TYPE FindMsg_ByMsgId(MAINTAINBE_DATA_STU* pOCPP, int nMsgId)
{
	int n;
	for(n = 0; n < Max_Maintain_Be_Msgs; n++)
	{
		//TRACEX("szName :%s \t pOCPP->stMsgInfoList[n].szMsgName : %s \n", szName, pOCPP->stMsgInfoList[n].szMsgName);
		//if(strcmp(pOCPP->stMsgInfoList[n].szMsgName, szName) == 0)
		if(pOCPP->stMsgInfoList[n].nMsgId == nMsgId)
		{
			TRACEX("pOCPP->stMsgInfoList[n].szMsgName : [%c%d]  %d \n", 
				 pOCPP->stMsgInfoList[n].nMsgId);

			return pOCPP->stMsgInfoList[n].emMsgType;
		}
	}

	return Msg_Unkown;
}

// static BOOL FindMsg_ByUniqueid(MAINTAINBE_DATA_STU* pOCPP, unsigned char* szuniqueId)
// {
// 	int n;
// 	UINT nUniqID = (UINT) myAtoi(szuniqueId); // don't use atoi() its issue
	
// 	for(n = 0; n < MIN(MAX_ES_iOCPP_SEND_RETRIES, pOCPP->activeCallMsg.nRetryTimes); n++)
// 	{
// 		//TRACEX("FindMsg_ByUniqueid Active messsage %lu  nUniqID %lu \n", pOCPP->activeCallMsg.nUniqMsgId[n], nUniqID);
// 		if(pOCPP->activeCallMsg.nUniqMsgId[n] == nUniqID)
// 		{
// 			TRACEX("FindMsg_ByUniqueid %d %d %d \n",n, pOCPP->activeCallMsg.nUniqMsgId[n], nUniqID);
// 			return TRUE;
// 		}
// 	}
// 	return FALSE;
// }

static void Sync_SysTime_FromMBESever(MAINTAINBE_DATA_STU* pOCPP, time_t tm)
{
	//!!!!!!!!!!!!!!!!!!!!Jimmy note 20220314: can NOT do sync time as it will affect OCPP !!!
	return;
	//!!!!!!!!!!!!!!!!!!!!

	//do not sync if OCPP backend is ON
	BOOL bNeedSyncTm = FALSE;
	if(pOCPP->stCurr_102Info.emVPNConnStatus < 1)
	{
		int s;
		MAINT_BE_ALARM* pMBEAlm = pOCPP->stCurr_102Info.actAlmTable;
		for(s = 0; s < pOCPP->stCurr_102Info.nActiveAlmTotal; s++, pMBEAlm++)
		{
			if(pMBEAlm->nAlarmId == 4)
			{
				bNeedSyncTm = TRUE;
				break;
			}
		}
	}
	if(!bNeedSyncTm)
	{
		return;
	}
	else if(ABS(tm - _NOW_) <= 10) //do NOT sync it too closely
	{
		return;
	}

	//2021-11-17. John Dsiable Sync Time,Due To Maintain is UTC, Cause EMSMgmt Need TimeZone
	//VAR_VALUE varV;
	//varV.dtValue = tm;
	//REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	//pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 1, &varV, 4, 0);
	
}


int Parse_MBE_Frames(MAINTAINBE_DATA_STU* pOCPP)
{
	char *pszPos = pOCPP->szReadBuffer;
	char *pszEnd = NULL;

	MAINTAINBE_MSG_TYPE msgType;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	MAINTAINBE_MSG_CALLTYPE  emCallType = 0;
	while (pOCPP->nReadBuffLen > 0)
	{
		cJSON *json = NULL;
		//printf("MaintBE: IN Paser Function %s  %d \n", pOCPP->szReadBuffer,pOCPP->nReadBuffLen);
		json = cJSON_ParseWithOpts(pszPos,&pszEnd, FALSE);
		
		if (!json)
		{
			TRACEX("MaintBE: Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
			TRACEX("MaintBE: Here IS the buffer %s \n", pszPos);
			return -1;
		}
		
		cJSON *sectionId = cJSON_GetObjectItem(json, "nm");
		cJSON *sectionUniqKey = cJSON_GetObjectItem(json, "uId");
		cJSON *sectionContent = cJSON_GetObjectItem(json, "msg");
		if((sectionId == NULL) || (sectionUniqKey == NULL) || (sectionContent == NULL))
		{
			TRACEX("11111Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
			TRACEX(" 11111118Here IS the buffer %s \n", pszPos);
			cJSON_Delete(json);
			return -1;
		}
		else if((cJSON_String != sectionId->type) || (cJSON_String != sectionUniqKey->type)/* ||
			(cJSON_Object != sectionContent->type)*/)
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
		if(c0 == 'A')
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
		msgType = FindMsg_ByMsgId(pOCPP, nMsgId); 
		if(msgType == Msg_Unkown)
		{
			//can NOT find the msg, so we skip it
			//cJSON_Delete(json);
			//return -1;
			cJSON_Delete(json);		//do not forget to destroy the json
			TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszPos - pszEnd));
			pOCPP->nReadBuffLen -= (pszEnd - pszPos);
			TRACEX("Len pOCPP->nReadBuffLen : %d \n  ", pOCPP->nReadBuffLen);
			pszPos = pszEnd;
			continue;
		}

		if((emCallType == Call))
		{			
			if(pOCPP->emOCPPConnStatus != Ready)
			{
				Send_CallError(pOCPP, emCallType, CallError_GenericError, "Only support TCP type yet!");
			}
			else
			{
				//now handling cmd
				pOCPP->pCurrJson = sectionContent;
				snprintf(pOCPP->szHandlingUniqID, 36, "%s", sectionUniqKey->valuestring);
				pOCPP->stMsgInfoList[msgType].pfnMsgHandler(pOCPP);
				pOCPP->tmLastSuccessExchange = _NOW_;
			}
		}
		else if((emCallType == CallResult))
		{
			TRACEX("Recv CallResult \n");
			cJSON *temp1 = NULL;
			if (msgType == C_100_Register)
			{
				TRACEX("Recv Pkt C_100_Register %s \n",pszPos);

				if (temp1 = cJSON_GetObjectItem(sectionContent, "accept"))
				{
					if (cJSON_Number == temp1->type) // required to Avoid Segmetation Fault 
					{
						//if (temp1->valueint == 1)
						if (temp1->valueint >= 1) //for test purpose
						{
							TRACEX("ACCCCCCCept for Pkt C_100_Register");
							pOCPP->bVerInfo_Changed = TRUE;
							pOCPP->bBootNotifyDone = TRUE;
						}
						else
						{
							TRACEX("REEEEEEEEEject for Pkt C_100_Register");
							pOCPP->bBootNotifyDone = FALSE;
							pOCPP->emOCPPConnStatus = BootNotify_Rejected;
						}
					}
				}

				//added by Jimmy 20210824
				if (temp1 = cJSON_GetObjectItem(sectionContent, "serv_ver"))
				{
					if (cJSON_String == temp1->type)
					{
						snprintf(pOCPP->szMBE_SWVer, 32,  "%s", temp1->valuestring);
					}
				}

				if(pOCPP->bBootNotifyDone)
				{					
					if (temp1 = cJSON_GetObjectItem(sectionContent, "time"))
					{
						if (cJSON_String == temp1->type)
						{
							time_t tm = Parse_MaintainBeTime(temp1->valuestring);
							Sync_SysTime_FromMBESever(pOCPP, tm);
						}
					}
				}
			}
			else if ((msgType == C_101_VersionInfo) || (msgType == C_102_StatusNotif)) 
			{
				TRACE("Recv Pkt %s: [%s] \n", (msgType == C_101_VersionInfo) ? "C_101_VersionInfo" : "C_102_StatusNotif", pszPos);
				cJSON *temp1 = NULL;
				//if OCPP server is ON, do not sync
				BOOL bNeedSyncTm = FALSE;
				if(pOCPP->stCurr_102Info.emVPNConnStatus < 1)
				{
					int s;
					MAINT_BE_ALARM* pMBEAlm = pOCPP->stCurr_102Info.actAlmTable;
					for(s = 0; s < pOCPP->stCurr_102Info.nActiveAlmTotal; s++, pMBEAlm++)
					{
						if(pMBEAlm->nAlarmId == 4)
						{
							bNeedSyncTm = TRUE;
							break;
						}
					}
				}
				if(bNeedSyncTm)
				{
					if (temp1 = cJSON_GetObjectItem(sectionContent, "time"))
					{
						if (cJSON_String == temp1->type)
						{
							time_t tm = Parse_MaintainBeTime(temp1->valuestring);
							Sync_SysTime_FromMBESever(pOCPP, tm);
						}
					}
				}
			}
			
			if(msgType == C_102_StatusNotif)
			{
				//update the last status so that we can update it in case anything upgrade
				pOCPP->bCPStatusChanged = FALSE;
				pOCPP->tmLast_SuccessExChange_102 = _NOW_;
				//memcpy(&pOCPP->stLast_102Info, &pOCPP->stCurr_102Info, sizeof(MAINT_BE_STATUS_INFO));
			}
			else if(msgType == C_101_VersionInfo)
			{
				pOCPP->bVerInfo_Changed = FALSE;
				pOCPP->tmLast_SuccessExChange_101 = _NOW_;
			}

			pOCPP->tmLastSuccessExchange = _NOW_;
			pOCPP->bHasActiveMsg = FALSE;
		}
		else //CallError
		{
			// if(pOCPP->bHasActiveMsg && (nMsgId == pOCPP->stMsgInfoList[pOCPP->activeCallMsg.msgData.emMsgType].nMsgId))
			// {
			// 	if(FindMsg_ByUniqueid(pOCPP, sectionUniqKey->valuestring))
			// 	{
			// 		cJSON *temp1 = NULL;
			// 		int nErrCode = 0;
			// 		if (temp1 = cJSON_GetObjectItem(sectionContent, "code"))
			// 		{
			// 			nErrCode = temp1->valueint;
			// 		}

			// 		char *errDesp = "";
			// 		if (temp1 = cJSON_GetObjectItem(sectionContent, "err_desp"))
			// 		{
			// 			if(cJSON_String == temp1->type)
			// 			{
			// 				errDesp = temp1->valuestring;
			// 			}
			// 		}

			// 		LogOut(MAIN_MODULE_REPORTER, "OCPP", LOG_TYPE_ERROR,
			// 			"Received MsgCallError from CS for [%d], errCode[%s], ErrDescription = [%s]", 
			// 			pOCPP->stMsgInfoList[pOCPP->activeCallMsg.msgData.emMsgType].nMsgId, 
			// 			nErrCode, errDesp);

			// 		pOCPP->activeCallMsg.msgData.nMsgErrorRecievedCount++;

			// 		//here we handle the Trans-Related msg error attempts
			// 		if(pOCPP->activeCallMsg.msgData.emMsgType == C_203_Summary)
			// 		{
			// 			pOCPP->activeCallMsg.msgData.tmLastErrorRespArrived = _NOW_;
			// 			if(pOCPP->activeCallMsg.msgData.nMsgErrorRecievedCount < 3)
			// 			{
			// 				//should put the messge back into the queue and continue trying
			// 				if(pOCPP->activeCallMsg.msgData.emSrc == CALL_SRC_HisTrans)
			// 				{
			// 					Queue_Put(pOCPP->hqHistoryTrans, &pOCPP->activeCallMsg.msgData, TRUE);
			// 				}
			// 				else if(pOCPP->activeCallMsg.msgData.emSrc == CALL_SRC_InProcessTrans)
			// 				{
			// 					int nGunIdx = pOCPP->activeCallMsg.msgData.nBelongedConn;
			// 					Queue_Put(pOCPP->stConnectorData[nGunIdx].hqInProcessTrans, &pOCPP->activeCallMsg.msgData, TRUE);
			// 				}
			// 			}
			// 			else //only try specified times, or else just omit the msg
			// 			{
			// 				//update to CCU sampler
			// 				VAR_VALUE varV;
			// 				A_BLOB_UPDATE_CHARGEMETER_STAT update;
			// 				int nGunIdx = pOCPP->activeCallMsg.msgData.nBelongedConn;
			// 				update.emBlobType = BLOB_TYPE_UPDATE_CHARG_STAT_METER;
			// 				update.emSenderType = NOTIFY_BY_OTHER_REPORTER;
			// 				update.emUpdateType = CMUPDATE_Omited; //should be omitted
			// 				update.nGunIdx = nGunIdx + 1;
			// 				update.tmChargeStart = pOCPP->activeCallMsg.msgData.tmRelatedTransStart;
			// 				varV.blobValue = &update;
			// 				if(pOCPP->activeCallMsg.msgData.emSrc == CALL_SRC_HisTrans)
			// 				{
			// 					TRACE("----Received error trans-related-conf, so send back to sysSampler-----\n");
			// 					pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 1, 7, &varV,
			// 						sizeof(A_BLOB_UPDATE_CHARGEMETER_STAT), 0);
			// 				}
			// 				else
			// 				{
			// 					TRACE("----Received error trans-related-conf, so send back to sysSampler-----\n");
			// 					pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 201 + nGunIdx, 105, &varV,
			// 						sizeof(A_BLOB_UPDATE_CHARGEMETER_STAT), 0);
			// 				}
			// 			}
			// 		}
			// 		else if (pOCPP->activeCallMsg.msgData.emMsgType == C_201_Authorize)
			// 		{
			// 			//Jimmy added 20190702 for OCTT test: when received CallError, shall set this to FALSE
			// 			//or else another Auth shall not be started any longer
			// 			pOCPP->bAuthorizeInProcess = FALSE;
			// 		}

			// 		//no need to send it again in case error happen again;
			// 		pOCPP->bHasActiveMsg = FALSE;
			// 		pOCPP->nConnErrTimes = 0;
			// 	}
			// }
			// else
			// {
			// 	//just discard it?
			// }

			pOCPP->bHasActiveMsg = FALSE;
		}

		cJSON_Delete(json);		//do not forget to destroy the json
		//now continue parsing for another frame

		TRACEX("MaintBE: pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszPos - pszEnd));
		pOCPP->nReadBuffLen -= (pszEnd - pszPos);
		TRACEX("MaintBE: Len pOCPP->nReadBuffLen : %d \n  ", pOCPP->nReadBuffLen);
		pszPos = pszEnd;
	}

	return 0;
}

