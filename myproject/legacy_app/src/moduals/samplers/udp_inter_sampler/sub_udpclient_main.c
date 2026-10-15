#include "udp_public_main.h"


/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       : John
*  CREATED DATE  : 2024-03-05 17:13:53
*  DESCRIPTION   :  { id: , src:, DateTime: , UniqueKey: , content{}}
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 

#define INIT_UdpClient_MSG(_pMsg, _emMsgType, _nMsgId, _emMsgSrc, _pfnMsgHandler) \
	do{\
	(_pMsg)->emMsgType = (_emMsgType); \
	(_pMsg)->nMsgId = (_nMsgId); \
	(_pMsg)->emMsgSrc = (_emMsgSrc); \
	(_pMsg)->pfnMsgHandler = (_pfnMsgHandler); \
	(_pMsg)++; \
	}while(0)

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       : John
*  CREATED DATE  : 2024-03-05 17:13:53
*  DESCRIPTION   :  { id: , src:, DateTime: , UniqueKey: , content{}}
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 
static void Init_UdpClientMsgInfo(STU_UDP_ROUGHDATA *pUdp)
{
	CLIENT_MSG_INFO* p = &(pUdp->stMsgList_Client[0]);
	//!!!!!!!!!Notice Macro MUST be in the right order because we use it for INDEXING!!!!!!!!!/**/
	INIT_UdpClient_MSG(p,	S_10X_METERVALUE_CLIENT,	200,	UDP_MSG_C2S, 	PrepareClient_S10X_MeterValue);
	INIT_UdpClient_MSG(p,	R_100_METERVALUE_CLIENT,	100,	UDP_MSG_S2C,	HandleClient_R100_MeterValue);
 	INIT_UdpClient_MSG(p,	R_400_EMSMGMT_CLIENT,		400,	UDP_MSG_S2C,	HandleClient_R400_EmsMgmt);

	return;
}


/*********************************************************************************
*  
*  FUNCTION NAME : UdpClient Initialize()
*  CREATOR       : John
*  CREATED DATE  : 2024-02-25 17:13:53
*  DESCRIPTION   : UPD Client
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL UdpClient_Initialize(HANDLE hSamplerSelf)
{
	TRACE("--- UpdClient_Initialize ---\n");
	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
	STU_UDP_ROUGHDATA* pUdp = (STU_UDP_ROUGHDATA*)pSampler->pDataHeap;
	
	//Now open the port
	int nErrCode = 0;
	COMM_UDP_SET * sUdpServerInfo = NEW(COMM_UDP_SET,1);
    if (sUdpServerInfo == NULL) 
	{
        printf("Memory allocation failed\n");
        return FALSE;
    }
	ZERO_POBJS(sUdpServerInfo,1);

    sUdpServerInfo->nNonBlock = 1;	//非阻塞
    sUdpServerInfo->sPeerInfo.nPeerPort = 9999;
    strncpy(sUdpServerInfo->sPeerInfo.szAddr, "192.168.1.100", sizeof(sUdpServerInfo->sPeerInfo.szAddr));
	pSampler->hCommPort =  pSampler->pUsedCommPort->Open(
		//pSampler->pUsedCommPort->info.szPortAttri,
		//pSampler->info.szSamplerSetting,
		(char*)sUdpServerInfo,
		"0.0.0.0:10000",
		COMM_CLIENT_MODE, 100, &nErrCode);

	TRACEX("UdpUdp: Init nErrCode=%d szPortAttri=%s szSamplerSetting=%s\n",\
		nErrCode, pSampler->pUsedCommPort->info.szPortAttri, \
		pSampler->info.szSamplerSetting);
	if(ERR_COMM_OK != nErrCode)
	{
		return FALSE;
	}

	Init_UdpClientMsgInfo(pUdp);
	
	pUdp->nNoChange_Times = 0;

	return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       : John
*  CREATED DATE  : 2024-03-05 17:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 
static UDP_CLIENT_CMDTP FindClientMsg_ByMsgId(STU_UDP_ROUGHDATA* pUdp, int nMsgId, UDPMSG_CALL_TYPE callType)
{
	int n = 0;
	for(n = 0; n < Max_MSG_CLIENT; n++)
	{
		if(pUdp->stMsgList_Client[n].nMsgId == nMsgId)
		{
			TRACEX("callType = %d, pUdp->stMsgList_Client[%d].emMsgType\n", 
				callType, n,pUdp->stMsgList_Client[n].emMsgType);

			return pUdp->stMsgList_Client[n].emMsgType;
		}
	}

	return MSG_CLIENT_Unkown;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       : John
*  CREATED DATE  : 2024-03-05 17:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 
static BOOL ParseClient_DataFrames(STU_UDP_ROUGHDATA *pUdp)
{
	SAMPLER* pSampler = (SAMPLER*)pUdp->hSampler;
	UDP_CLIENT_CMDTP 	msgType;
	UDPMSG_CALL_TYPE  	emCallType = 0;

	char *pszPos = pUdp->szReadBuffer;
	char *pszEnd = NULL;

	while (pUdp->nReadBuffLen > 0)
	{
		cJSON *json = NULL;
		TRACE("IN Paser Function %s  %d \n", pUdp->szReadBuffer,pUdp->nReadBuffLen);
		json = cJSON_ParseWithOpts(pszPos,&pszEnd, FALSE);
		
		if (!json)
		{
			TRACEX("Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
			TRACEX(" Here IS the buffer %s \n", pszPos);
			return -1;
		}
		
		cJSON *sectionId = cJSON_GetObjectItem(json, "udpId");
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
		if(c0 == 'Q') {
			emCallType = Q_RECV;
		}
		else if(c0 == 'R') {
			emCallType = R_REPLY;
		}
		else if(c0 == 'E') {
			emCallType = E_ERROR;
		}
		else{
			TRACEX("4444Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
			TRACEX(" 4444Here IS the buffer %s \n", pszPos);
			cJSON_Delete(json);
			return -1;
		}

		int nMsgId = myAtoi(sectionId->valuestring + 1);
		TRACE("\nNow get nReadBuffLen=[%d] MsgId->%d, callType->%d, Recv_Packet: %s \n Next Object %s \n ", \
			pUdp->nReadBuffLen, nMsgId, emCallType, pszPos, pszEnd);

		if(emCallType == Q_RECV)
		{
			msgType = FindClientMsg_ByMsgId(pUdp, nMsgId, Q_RECV); 
			TRACE("msgType =%d nMsgId=%d\n", msgType, nMsgId);
			
			pUdp->pCurrJson = sectionContent;
			if(msgType != MSG_CLIENT_Unkown)
			{
				pUdp->stMsgList_Client[msgType].pfnMsgHandler(pUdp);
				//pUdp->tmLastSuccessExchange = _NOW_;
			}
			else
			{
				char szErrMsgName[256] = {0};
				//snprintf(szErrMsgName, 256, "Unkown Messege -> [%s]!", sectionId->valuestring);
				//Send_CallError(pUdp, Msg_Unkown, CallError_FormatError, szErrMsgName);
			}
		}
		else if((emCallType == R_REPLY))
		{
			TRACEX("Recv R_REPLY \n");
 
			pUdp->bHasActiveMsg = FALSE;
			pUdp->nConnErrTimes = 0;
			//pUdp->tmLastSuccessExchange = _NOW_;
		}
		else //CallError
		{
			 
		}

		cJSON_Delete(json);		//do not forget to destroy the json
		//now continue parsing for another frame

		TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszPos - pszEnd));
		pUdp->nReadBuffLen -= (pszEnd - pszPos);
		TRACEX("Len pUdp->nReadBuffLen : %d \n  ", pUdp->nReadBuffLen);
		pszPos = pszEnd;
	}

	return TRUE;
}

static void ReInitData_CommFailed(STU_UDP_ROUGHDATA *pUdp)
{
	VAR_VALUE varV;
	pUdp->bCommFail = TRUE;
	varV.emValue = pUdp->bCommFail;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, DEVID_BLOBMETER_GRID, 431, varV);
	
	A_BLOB_METER_VALUES blobMeter;
	ZERO_POBJS(&blobMeter, 1);

  	UdpClient_StuffMeterValue(pUdp, DEVID_BLOBMETER_GRID, &blobMeter);
  	UdpClient_StuffMeterValue(pUdp, DEVID_BLOBMETER_AUX1, &blobMeter);
  	UdpClient_StuffMeterValue(pUdp, DEVID_BLOBMETER_AUX3, &blobMeter);

	Push_BlobValue(SETTYPE_RAW_SP, pUdp->hSampler, 
		DEVID_BLOBMETER_GRID, 430, (void*)&blobMeter, sizeof(A_BLOB_METER_VALUES), BLOB_TYPE_METERVALUES);
	
	Push_BlobValue(SETTYPE_RAW_SP, pUdp->hSampler, 
		DEVID_BLOBMETER_AUX1, 430, (void*)&blobMeter, sizeof(A_BLOB_METER_VALUES), BLOB_TYPE_METERVALUES);
	
	Push_BlobValue(SETTYPE_RAW_SP, pUdp->hSampler, 
		DEVID_BLOBMETER_AUX3, 430, (void*)&blobMeter, sizeof(A_BLOB_METER_VALUES), BLOB_TYPE_METERVALUES);
			
	return;
}

static BOOL isRecvDataChanged(STU_UDP_ROUGHDATA *pUdp)
{
	#define MAX_RECVSOMEDATA_TIMES 100
	static float PLastgrid_Sum;
	if(pUdp->blobMeter.fRealTime_ActivePower < 10.0)
	{
		return TRUE;
	}

	if(FLOAT_EQUAL(pUdp->fPgrid_Sum, PLastgrid_Sum))
	{
		pUdp->nNoChange_Times++;
		if(pUdp->nNoChange_Times > MAX_RECVSOMEDATA_TIMES)
		{
			pUdp->nNoChange_Times = MAX_RECVSOMEDATA_TIMES;
			return FALSE;
		}
	}
	else
	{
		pUdp->nNoChange_Times = 0;
	}

	return TRUE;
}

static void OnHandle_CommFail(STU_UDP_ROUGHDATA *pUdp)
{
	pUdp->nCommFailTimes++;
	if(pUdp->nCommFailTimes > MAX_COMMFAIL_TIMES)
	{
		pUdp->nCommFailTimes = MAX_COMMFAIL_TIMES;
		if(!pUdp->bCommFail)
		{
			ReInitData_CommFailed(pUdp);
		}
	}
	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Udp ReadData()
*  CREATOR       : John
*  CREATED DATE  : 2024-02-25 17:13:53
*  DESCRIPTION   : ReadData through server sock_addr + sock_port = 192.168.1.100:9999.
*  				   Client SendData sock_addr + sock_port = 192.168.1.100:9999.
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 
static void UdpClient_ReadData(STU_UDP_ROUGHDATA *pUdp)
{
	SAMPLER* pSampler = (SAMPLER*)pUdp->hSampler;
	VAR_VALUE varV;

	memset(pUdp->szReadBuffer, 0, UDP_MAX_SENTMSG_SIZE);
	pUdp->nReadBuffLen = pSampler->pUsedCommPort->Read(pSampler->hCommPort, pUdp->szReadBuffer, TCP_BUFF_LEN);
	if(pUdp->nReadBuffLen > 0)
	{
		if(isRecvDataChanged(pUdp))
		{
			ParseClient_DataFrames(pUdp);
			pUdp->nCommFailTimes = 0;
			if(pUdp->bCommFail)
			{
				pUdp->bCommFail = FALSE;
				varV.emValue = pUdp->bCommFail;
				Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, DEVID_BLOBMETER_GRID, 431, varV);
			}
		}
		else
		{
			OnHandle_CommFail(pUdp);
		}

	}
	else  
	{
		OnHandle_CommFail(pUdp);
	}
	 
	TRACE("Warning: Client Read Nothing From Server:nReadBuffLen=[%d]!!!\n", pUdp->nReadBuffLen);
	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : UdpUdp WriteData()
*  CREATOR       : John
*  CREATED DATE  : 2024-02-25 17:13:53
*  DESCRIPTION   : WriteData to 192.168.1.255:10000, Sub BroadCast sock_addr + sock_port.
*  				   Client Should Use 0.0.0.0:10000 sock_addr + sock_port to Recv.
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 
static BOOL UdpClient_WriteData(STU_UDP_ROUGHDATA *pUdp)
{
	SAMPLER* pSampler = (SAMPLER*)pUdp->hSampler;

	pUdp->stMsgList_Client[S_10X_METERVALUE_CLIENT].pfnMsgHandler((void*)pUdp);

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Sample()
*  CREATOR       : John
*  CREATED DATE  : 2024-02-25 17:13:53
*  DESCRIPTION   : UPD Client
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 
int UdpClient_Sample(HANDLE hSamplerThis)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	STU_UDP_ROUGHDATA* pUdp = (STU_UDP_ROUGHDATA*)pSampler->pDataHeap;
	time_t 		tm_Now = _NOW_;
	int 		n = 0, nDataSize=0, nDataLen = 0;

	//double tm = GetCurrentTime();
 
	UdpClient_WriteData(pUdp);

	UdpClient_ReadData(pUdp);

	TRACE("Udp Client Running tm=[%ld]\n", _NOW_);
	
	Sleep_Ex(500);

	return ERR_SAMPLE_OK;
}
 

	

