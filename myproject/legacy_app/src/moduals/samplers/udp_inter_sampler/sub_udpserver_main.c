#include "udp_public_main.h"

#define INIT_UdpServer_MSG(_pMsg, _emMsgType, _nMsgId, _emMsgSrc, _pfnMsgHandler) \
	do{\
	(_pMsg)->emMsgType = (_emMsgType); \
	(_pMsg)->nMsgId = (_nMsgId); \
	(_pMsg)->emMsgSrc = (_emMsgSrc); \
	(_pMsg)->pfnMsgHandler = (_pfnMsgHandler); \
	(_pMsg)++; \
	}while(0)


static void Init_UdpServerMsgInfo(STU_UDP_ROUGHDATA *pUdp)
{
	SERVER_MSG_INFO* p = &(pUdp->stMsgList_Server[0]);
	//!!!!!!!!!Notice Macro MUST be in the right order because we use it for INDEXING!!!!!!!!!/**/
	INIT_UdpServer_MSG(p,	S_100_METERVALUE_SERVER,	100,	UDP_MSG_S2C, 	PrepareServer_S100_MeterValue);
	INIT_UdpServer_MSG(p,	R_10X_METERVALUE_SERVER,	200,	UDP_MSG_C2S,	HandleServer_R10X_MeterValue);
	INIT_UdpServer_MSG(p,	S_400_EMSMGMT_SERVER,		400,	UDP_MSG_S2C, 	PrepareServer_S400_EmsMgmt);
	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : UPD Server Initialize()
*  CREATOR       : John
*  CREATED DATE  : 2024-02-25 17:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 
BOOL UdpServer_Initialize(HANDLE hSamplerSelf)
{	
	TRACE("Initialize Udp Server Sampler !!!\n");
	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
 	STU_UDP_ROUGHDATA* pUdp = (STU_UDP_ROUGHDATA*)pSampler->pDataHeap;
 
 	int nErrCode = 0;
	COMM_UDP_SET * sUdpClientInfo = NEW(COMM_UDP_SET,1);
    if (sUdpClientInfo == NULL) 
	{
        printf("Memory allocation failed\n");
        return FALSE;
    }
	ZERO_POBJS(sUdpClientInfo,1);
    sUdpClientInfo->nNonBlock = 1;	//非阻塞
    sUdpClientInfo->sPeerInfo.nPeerPort = 10000;
    strncpy(sUdpClientInfo->sPeerInfo.szAddr, "192.168.1.255", sizeof(sUdpClientInfo->sPeerInfo.szAddr));//�㲥
	pSampler->hCommPort = pSampler->pUsedCommPort->Open(
		//pSampler->pUsedCommPort->info.szPortAttri,
		(char*)sUdpClientInfo,
		pSampler->info.szSamplerSetting,
		COMM_SERVER_MODE, 100, &nErrCode);
	if(ERR_COMM_OK != nErrCode)
	{
		return FALSE;
	}
	
	TRACEX("UdpServer: Init nErrCode=%d szPortAttri=%s szSamplerSetting=%s\n",\
		nErrCode, pSampler->pUsedCommPort->info.szPortAttri, \
		pSampler->info.szSamplerSetting);
	
	Init_UdpServerMsgInfo(pUdp);

	return TRUE;
}


/*********************************************************************************
*  
*  FUNCTION NAME : FindServerMsg ByMsgId
*  CREATOR       : John
*  CREATED DATE  : 2024-03-05 17:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 
static UDP_CLIENT_CMDTP FindServerMsg_ByMsgId(STU_UDP_ROUGHDATA* pUdp, int nMsgId, UDPMSG_CALL_TYPE callType)
{
	int n = 0;
	for(n = 0; n < Max_MSG_SERVER; n++)
	{
		if(pUdp->stMsgList_Server[n].nMsgId == nMsgId)
		{
			TRACEX("callType = %d, pUdp->stMsgList_Server[%d].emMsgType\n", 
				callType, n,pUdp->stMsgList_Server[n].emMsgType);

			return pUdp->stMsgList_Server[n].emMsgType;
		}
	}

	return MSG_SERVER_Unkown;
}

/*********************************************************************************
*  
*  FUNCTION NAME : ParseClient DataFrames 
*  CREATOR       : John
*  CREATED DATE  : 2024-03-05 17:13:53
*  DESCRIPTION   :  
*  {
    "id": "Q200",
    "src": "2",
    "uniqKey": "0",
    "content": {
        "all_metervalue": [
            {
                "data_from": 407,
                "VPhA": 0,
                "VPhB": 0,
                "VPhC": 0,
                "APhA": 0,
                "APhB": 0,
                "APhC": 0,
                "TotkW": 0,
                "kWPhA": 0,
                "kWPhB": 0,
                "kWPhC": 0,
                "TotkVar": 0,
                "kVarPhA": 0,
                "kVarPhB": 0,
                "kVarPhC": 0,
                "TotPF": 0,
                "TotHz": 0,
                "kWhPos": 0,
                "kWhNeg": 0,
                "kVar1": 0,
                "kVar2": 0
            },
            {
                "data_from": 408,
                "VPhA": 0,
                "VPhB": 0,
                "VPhC": 0
            }
        ]
    }
}
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 
static BOOL ParseServer_DataFrames(STU_UDP_ROUGHDATA *pUdp)
{
	SAMPLER* pSampler = (SAMPLER*)pUdp->hSampler;
	UDP_SERVER_CMDTP 	msgType;
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
		
		cJSON *sectionId 		= cJSON_GetObjectItem(json, "udpId");
		cJSON *sectionUniqKey 	= cJSON_GetObjectItem(json, "uniqKey");
		cJSON *sectionContent 	= cJSON_GetObjectItem(json, "content");

		if((sectionId == NULL) 
			|| (sectionUniqKey == NULL) 
			|| (sectionContent == NULL))
		{
			TRACEX("11111Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
			TRACEX(" 11111118Here IS the buffer %s \n", pszPos);
			cJSON_Delete(json);
			return -1;
		}
		else if((cJSON_String != sectionId->type) 
			|| (cJSON_String != sectionUniqKey->type) 
			|| (cJSON_Object != sectionContent->type))
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
		if(c0 == 'Q'){
			emCallType = Q_RECV;
		}
		else if(c0 == 'R'){
			emCallType = R_REPLY;
		}
		else if(c0 == 'E'){
			emCallType = E_ERROR;
		}
		else{
			TRACEX("4444Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
			TRACEX(" 4444Here IS the buffer %s \n", pszPos);
			cJSON_Delete(json);
			return -1;
		}

		int nMsgId = myAtoi(sectionId->valuestring + 1);
		
		TRACE("Now get the nReadBuffLen=[%d] MsgId->%d, callType->%d, Recv_Packet: %s \n Next Object %s \n ", \
			pUdp->nReadBuffLen, nMsgId, emCallType, pszPos, pszEnd);

		if((emCallType == Q_RECV))
		{
			msgType = FindServerMsg_ByMsgId(pUdp, nMsgId, Q_RECV); 
			TRACE("msgType =%d nMsgId=%d\n", msgType, nMsgId);
			//now handling cmd
			pUdp->pCurrJson = sectionContent;
			//snprintf(pUdp->szHandlingUniqID, 36, "%s", sectionUniqKey->valuestring);
 			//TRACEX("!!!!!Server Call = Msg [%s], Cont = [%s]\n", pUdp->stMsgInfoList[msgType].szMsgName,
			//cJSON_Print(json));

			if(msgType != MSG_SERVER_Unkown)
			{
				pUdp->stMsgList_Server[msgType].pfnMsgHandler(pUdp);
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

static void Judge_SlvCommStatus(STU_UDP_ROUGHDATA *pUdp) 
{
	static UINT arrDevId[ESSTYPE_MAX]={0, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809};

	SAMPLER* pSampler = (SAMPLER*)pUdp->hSampler;

	UINT 		n 			= 0;
	UINT 		nConfigNum 	= 0;
	time_t 		tmNow 		= time(NULL);
	VAR_VALUE 	varV;
 
	for(n = ESSTYPE_UDP_CLIENT1; n < ESSTYPE_MAX; n++)
	{
 		TRACE("Idx=[%d] tmNow=[%ld]  tmLast=[%ld] CommFailedTimes=[%d]\n", \
			n, tmNow, pUdp->tmLastSuccessExchange[n], pUdp->nCommFailedTimes[n]);

		if(ABS(tmNow - pUdp->tmLastSuccessExchange[n]) > MAX_WAITSLV_SEC)
		{
			pUdp->nCommFailedTimes[n]++;
			if(pUdp->nCommFailedTimes[n] > MAX_COMMFAIL_TIMES)
			{
				pUdp->bRegisted[n]			= FALSE;
				pUdp->fKWh_Pos[n] 			= 0.0;
				pUdp->nCommFailedTimes[n] 	= MAX_COMMFAIL_TIMES;
				varV.emValue = 1; //1-CommFail
				Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, 801 + nConfigNum, 431, varV);

				UINT nMapDevId = arrDevId[n];
				A_BLOB_METER_VALUES blobMeter;
				ZERO_POBJS(&blobMeter, 1);
				Push_BlobValue(SETTYPE_RAW_SP, pUdp->hSampler, 
					nMapDevId, 430, (void*)&blobMeter, sizeof(A_BLOB_METER_VALUES), BLOB_TYPE_METERVALUES);
			}
		}
		else
		{

			pUdp->nCommFailedTimes[n] = 0;
			varV.emValue = 0; //0-CommOK
			Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, 801 + nConfigNum, 431, varV);
		}

		nConfigNum++;
		if(nConfigNum >= pUdp->nConfigSlvNumb)
		{
			TRACE("nConfigNum=[%d] pUdp->nConfigSlvNumb=[%d]\n",\
				nConfigNum, pUdp->nConfigSlvNumb);
			break;
		}
	}

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : UdpServer ReadData()
*  CREATOR       : John
*  CREATED DATE  : 2024-02-25 17:13:53
*  DESCRIPTION   : ReadData through server sock_addr + sock_port = 192.168.1.100:9999.
*  				   Client SendData sock_addr + sock_port = 192.168.1.100:9999.
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 

static void UdpServer_ReadData(STU_UDP_ROUGHDATA *pUdp)
{
	SAMPLER* pSampler = (SAMPLER*)pUdp->hSampler;

	memset(pUdp->szReadBuffer, 0, UDP_MAX_SENTMSG_SIZE);
	pUdp->nReadBuffLen = pSampler->pUsedCommPort->Read(pSampler->hCommPort, pUdp->szReadBuffer, TCP_BUFF_LEN);
	if(pUdp->nReadBuffLen > 0)
	{
 		ParseServer_DataFrames(pUdp);
	}
	else if(pUdp->nReadBuffLen == -1) //means server close session
	{

	}
	else
	{
		TRACE("Warning:SErver Read Nothing From Client!!!\n", pUdp->nReadBuffLen);
	}	 
	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : UdpServer WriteData()
*  CREATOR       : John
*  CREATED DATE  : 2024-02-25 17:13:53
*  DESCRIPTION   : WriteData to 192.168.1.255:10000, Sub BroadCast sock_addr + sock_port.
*  				   Client Should Use 0.0.0.0:10000 sock_addr + sock_port to Recv.
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 

static BOOL UdpServer_WriteData(STU_UDP_ROUGHDATA *pUdp)
{
	TRACE("--- UdpServer_WriteData ---\n");
	SAMPLER* pSampler = (SAMPLER*)pUdp->hSampler;
	time_t tmNow = _NOW_;

	pUdp->stMsgList_Server[S_100_METERVALUE_SERVER].pfnMsgHandler((void*)pUdp);

	if(ABS(tmNow - pUdp->tm_LastSyncEmsMgmt) >= TIMEOUT_60_SEC)
	{
		pUdp->tm_LastSyncEmsMgmt = tmNow;
		pUdp->stMsgList_Server[S_400_EMSMGMT_SERVER].pfnMsgHandler((void*)pUdp);
	}
	return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : UdpServer_Sample()
*  CREATOR       : John
*  CREATED DATE  : 2024-02-25 17:13:53
*  DESCRIPTION   : UPD ServerSample
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 
int UdpServer_Sample(HANDLE hSamplerThis)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	STU_UDP_ROUGHDATA* pUdp = (STU_UDP_ROUGHDATA*)pSampler->pDataHeap;
	time_t tm_Now = _NOW_;
 
	UdpServer_ReadData(pUdp);

	UdpServer_WriteData(pUdp);

	Judge_SlvCommStatus(pUdp);

	Sleep_Ex(500);
 
	TRACE("Udp Server Sampler Running tm=[%ld] !!!\n", _NOW_);

	return ERR_SAMPLE_OK;
}
 
