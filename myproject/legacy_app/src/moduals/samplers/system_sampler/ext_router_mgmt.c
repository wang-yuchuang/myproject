//#define _TRACE_PRINT_
#include "sampler_system.h"
#include "../../cJSON.h"
static COMMPORT	s_commTCP; 
static HANDLE 	s_hTCPPorInuse = NULL;
typedef struct
{
	char 	szId[64];
	char 	szSWVer[64];
	BOOL 	bIsSim_Ready; //SIM card ready or not
	BOOL	bIsConnOK; //connction to WLAN OK or not
	int		nRSSI; //0~31
	char	szIMSI[32];
	char	szICCID[32];
	char 	szNetMode[32];
}EXT_ROUTER_STATUS_INFO;

static BOOL s_bIsFirstTimeBoot = TRUE;
static BOOL s_bIsCommFail = FALSE;
static EXT_ROUTER_STATUS_INFO s_OldExtRouterStatusInfo;

// static char* GetRouter_HostIPAndPort_4AdminAPI(SYSTEM_SAMPLER_DATA *pSys_Rough, OUT char* szIPPort)
// {

// 	//fix => 192.168.1.1
// 	//sprintf(szIPPort, "192.168.1.1:6688");
// 	return szIPPort;
// }

BOOL NeedReconnectToRouteServer(void)
{
	s_bIsFirstTimeBoot = TRUE;
}

// 返回是否成功，true/false ;使用网关做路由器服务器地址，兼容多路由器方案
static int GetRouter_HostIPAndPort_4AdminAPI(SYSTEM_SAMPLER_DATA *pSys_Rough, OUT char* szIPPort, int nBufLen)
{
	if (szIPPort == NULL || pSys_Rough == NULL || nBufLen <= 0)
		return 0;

	struct in_addr gw_addr;
	gw_addr.s_addr = pSys_Rough->currNetWorkInfo.netInfo_eht0.ulIpV4_Gateway;

	if (gw_addr.s_addr == 0)
	{
		szIPPort[0] = '\0';
		return 0;  // 无网关
	}

	snprintf(szIPPort, nBufLen, "%s:6688", inet_ntoa(gw_addr));
	return 1;  // 成功
}


static BOOL ReOpen_TCPPort_4Comm(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
	static BOOL s_bGetCommPort = FALSE;
	SAMPLER* pSampler = (SAMPLER*)pSys_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	if(!s_bGetCommPort)
	{
		int err = pDPR->Get(pSys_Rough->hSampler, GETTYPE_COMMPORT,
			7, NULL, &s_commTCP, sizeof(COMMPORT));
		if(err != ERR_DPR_OK)
		{
			//printf("----ExtRouter: GEEET TCP port ERRRRoR----\n");
			return FALSE;
		}
		s_bGetCommPort = TRUE;
	}

	if(s_hTCPPorInuse != NULL)
	{
		s_commTCP.Close(s_hTCPPorInuse);
		s_hTCPPorInuse = NULL;
	}
	
	int nErrCode = 0;
	char szRouterServerPort[64];
	if (!GetRouter_HostIPAndPort_4AdminAPI(pSys_Rough, szRouterServerPort, sizeof(szRouterServerPort)))
	{
		//printf("Because gw is NULL, can't connect to route server\n");
		return FALSE;
	}
	//printf("Ready to connect to router server at: %s\n", szRouterServerPort);

	s_hTCPPorInuse = s_commTCP.Open("ROUTER_TCP",szRouterServerPort,
		COMM_CLIENT_MODE, 100, &nErrCode); //100 msec timeout   
	if(s_hTCPPorInuse != NULL)
	{
		//printf("---------ExtRouter: now Re-Open the serverTCPPort OOOOOOOOOOOK--------------\n");
		//set read/write timeout
		COMM_TIMEOUTS tmOut;
		tmOut.nReadTimeout = 100;
		tmOut.nWriteTimeout = 100;
	 	tmOut.nIntervalTimeout = 50;
		s_commTCP.Control(s_hTCPPorInuse, 
			COMM_SET_TIMEOUTS, &tmOut, sizeof(COMM_TIMEOUTS));
	}
	else
	{
		//printf("----ExtRouter: OPPPPPEN TCP port ERRRRoR----\n");
		return FALSE;
	}

	return TRUE;
}

static int Make_SendFrame_4Cmd(OUT BYTE* pbyOut, BYTE byCmd, USHORT usBodyLen, BYTE* pbyBody)
{
	USHORT usTotalLen = 7 + usBodyLen;
	pbyOut[0] = (BYTE)((usTotalLen >> 8) & 0x00FF);
	pbyOut[1] = (BYTE)(usTotalLen & 0x00FF);
	pbyOut[2] = byCmd;
	pbyOut[3] = 0x00; pbyOut[4] = 0x01;
	static USHORT s_usSN = 1;
	pbyOut[5] = (BYTE)((s_usSN >> 8) & 0x00FF);
	pbyOut[6] = (BYTE)(s_usSN & 0x00FF);
	memcpy(pbyOut +  7, pbyBody, usBodyLen);
	s_usSN++;
	if(s_usSN >= 0xFFFF)
	{
		s_usSN = 1;
	}

	return (int)usTotalLen;
}

static BOOL Try_GetRouterNetStatus_1B(SYSTEM_SAMPLER_DATA *pSys_Rough, EXT_ROUTER_STATUS_INFO* pExtRouterInfo)
{
	BYTE bySendCmd_1B[64];
	BYTE byBody_1B = 0x00;
	int nSendLen = Make_SendFrame_4Cmd(bySendCmd_1B, 0x1B, 1, &byBody_1B);
	ZERO_POBJS(pExtRouterInfo, 1);
	int nWriteLen = s_commTCP.Write(s_hTCPPorInuse, bySendCmd_1B, nSendLen);
	if(nWriteLen < nSendLen)
	{
		return FALSE;
	}

	BYTE byReceiveBuff[4096];
	memset(byReceiveBuff, 0, 4096);
	int nReadBytes = s_commTCP.Read(s_hTCPPorInuse, byReceiveBuff, 4096);
	if(nReadBytes > 2)
	{
		//printf("-----the Recived-Cont=>[[[%s]]]-----\n", byReceiveBuff);
		//now parse the JSON obj
		cJSON* pJCont = cJSON_Parse(byReceiveBuff);
		if(pJCont == NULL)
		{
			//printf("-------ExtRouter: ERROR 11111111---------\n");
			return FALSE;
		}
		//id
		cJSON* json_Tmp = cJSON_GetObjectItem(pJCont, "id");
		if(json_Tmp)
		{
			snprintf(pExtRouterInfo->szId, 64, "%s", json_Tmp->valuestring);
		}
		json_Tmp = cJSON_GetObjectItem(pJCont, "sw_ver");
		//sw Ver
		if(json_Tmp)
		{
			snprintf(pExtRouterInfo->szSWVer, 64, "%s", json_Tmp->valuestring);
		}
		//wan3g
		cJSON* json_wan3g = cJSON_GetObjectItem(pJCont, "wan3g");
		if(json_wan3g == NULL)
		{
			cJSON_Delete(pJCont);
			//printf("-------ExtRouter: ERROR 222222222---------\n");
			return FALSE;
		}
		else if(cJSON_Array != json_wan3g->type)
		{
			cJSON_Delete(pJCont);
			return FALSE;
		}
		cJSON* pJSeq0 = cJSON_GetArrayItem(json_wan3g, 0);
		if(pJSeq0 == NULL)
		{
			cJSON_Delete(pJCont);
			//printf("-------ExtRouter: ERROR 333333333---------\n");
			return FALSE;
		}
		//sim status
		json_Tmp = cJSON_GetObjectItem(pJSeq0, "simStatus");
		pExtRouterInfo->bIsSim_Ready = FALSE;
		if(json_Tmp)
		{
			if(json_Tmp->valuestring)
			{
				if(strstr(json_Tmp->valuestring, "simready") != NULL)
				{
					pExtRouterInfo->bIsSim_Ready = TRUE;
				}
			}			
		}
		//conn status
		json_Tmp = cJSON_GetObjectItem(pJSeq0, "connStatus");
		pExtRouterInfo->bIsConnOK = FALSE;
		if(json_Tmp)
		{
			if(json_Tmp->valuestring)
			{
				if(strcmp(json_Tmp->valuestring, "connected") == 0)
				{
					pExtRouterInfo->bIsConnOK = TRUE;
				}
			}			
		}
		//rssi
		json_Tmp = cJSON_GetObjectItem(pJSeq0, "rssi");
		if(json_Tmp)
		{
			//pExtRouterInfo->nRSSI = json_Tmp->valueint;	
			if(json_Tmp->valuestring)
			{
				pExtRouterInfo->nRSSI = atoi(json_Tmp->valuestring);	
			}				
		}
		//imsi
		json_Tmp = cJSON_GetObjectItem(pJSeq0, "imsi");
		if(json_Tmp)
		{
			if(json_Tmp->valuestring)
			{
				snprintf(pExtRouterInfo->szIMSI, 32, "%s", json_Tmp->valuestring);
			}			
		}
		//iccid
		json_Tmp = cJSON_GetObjectItem(pJSeq0, "iccid");
		if(json_Tmp)
		{
			if(json_Tmp->valuestring)
			{
				snprintf(pExtRouterInfo->szICCID, 32, "%s", json_Tmp->valuestring);
			}			
		}
		//netMode
		json_Tmp = cJSON_GetObjectItem(pJSeq0, "netMode");
		if(json_Tmp)
		{
			if(json_Tmp->valuestring)
			{
				snprintf(pExtRouterInfo->szNetMode, 32, "%s", json_Tmp->valuestring);
			}			
		}

		cJSON_Delete(pJCont);
	}
	else
	{
		//printf("-------ExtRouter: ERROR 4444444444---------\n");
		return FALSE;
	}
	
	return TRUE;
}

static void Manage_ZLWL_ZR2000_ExtRouter(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
	//refresh in every 300*15 ~= 5s
	static int s_nCounter = 0;
	s_nCounter++;
	if(s_nCounter > 15)
	{
		s_nCounter = 0;		
	}
	else
	{
		return;
	}
	//printf("s_bIsFirstTimeBoot = [%d] s_bIsCommFail = [%d]\n",s_bIsFirstTimeBoot,s_bIsCommFail);

	if(s_bIsFirstTimeBoot || s_bIsCommFail)
	{
		if(s_bIsFirstTimeBoot)
		{
			ZERO_POBJS(&s_OldExtRouterStatusInfo, 1);
		}		
		if(!ReOpen_TCPPort_4Comm(pSys_Rough))
		{
			return;
		}
		s_bIsFirstTimeBoot = FALSE;
	}

	EXT_ROUTER_STATUS_INFO runRouterInfo;
	static int s_nCommFailCounter = 0;
	if(!Try_GetRouterNetStatus_1B(pSys_Rough, &runRouterInfo))
	{
		s_nCommFailCounter++;
		if(s_nCommFailCounter >= 5)
		{
			s_nCommFailCounter = 0;
			s_bIsCommFail = TRUE;
			if(pSys_Rough->currNetWorkInfo.netInfo_usb0.emNetPort_Status != NETPORT_STATUS_NOTINST)
			{
				LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING, "The Ext-Router was LOST TCP-COMM with controller!!!");
				pSys_Rough->currNetWorkInfo.netInfo_usb0.emNetPort_Status = NETPORT_STATUS_NOTINST;
				Push_BlobValue(SETTYPE_RAW_ST, pSys_Rough->hSampler, 1, 3, 
					(void*)&pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);				
			}
		}
		return;
	}
	
	s_nCommFailCounter = 0;
	s_bIsCommFail = FALSE;

	//judge if need update network cfg
	if((strncmp(s_OldExtRouterStatusInfo.szIMSI, runRouterInfo.szIMSI, 32) != 0) || 
		(strncmp(s_OldExtRouterStatusInfo.szICCID, runRouterInfo.szICCID, 32) != 0) ||
		(strncmp(s_OldExtRouterStatusInfo.szNetMode, runRouterInfo.szNetMode, 32) != 0) ||
		(s_OldExtRouterStatusInfo.bIsSim_Ready != runRouterInfo.bIsSim_Ready) ||
		(s_OldExtRouterStatusInfo.bIsConnOK != runRouterInfo.bIsConnOK) ||
		(s_OldExtRouterStatusInfo.nRSSI != runRouterInfo.nRSSI))
	{
		//now push the network seting
		//RSSI, 31~15 => 5, 14~12 => 4, 12~10 => 3, 9~7 => 2, 7~5 => 1, else => 0
		pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_dBm = runRouterInfo.nRSSI;
		pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 0;
		if((runRouterInfo.nRSSI >= 5) && (runRouterInfo.nRSSI < 7))
		{
			pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 1;
		}
		else if((runRouterInfo.nRSSI >= 7) && (runRouterInfo.nRSSI < 10))
		{
			pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 2;
		}
		else if((runRouterInfo.nRSSI >= 10) && (runRouterInfo.nRSSI < 12))
		{
			pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 3;
		}
		else if((runRouterInfo.nRSSI >= 12) && (runRouterInfo.nRSSI <= 14))
		{
			pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 4;
		}
		else if((runRouterInfo.nRSSI >= 15) && (runRouterInfo.nRSSI <= 31))
		{
			pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 5;
		}
		if(runRouterInfo.bIsConnOK)
		{
			pSys_Rough->currNetWorkInfo.netInfo_usb0.emNetPort_Status = NETPORT_STATUS_CONNECTED;
		}
		else if(!runRouterInfo.bIsSim_Ready)
		{
			pSys_Rough->currNetWorkInfo.netInfo_usb0.emNetPort_Status = NETPORT_STATUS_NOTINST;
		}
		else
		{
			pSys_Rough->currNetWorkInfo.netInfo_usb0.emNetPort_Status = NETPORT_STATUS_DISCONN;
		}
		snprintf(pSys_Rough->currNetWorkInfo.lteModemINfo.szIMSI, 20, "%s", runRouterInfo.szIMSI);
		snprintf(pSys_Rough->currNetWorkInfo.lteModemINfo.szICCID, 32, "%s", runRouterInfo.szICCID);
		snprintf(pSys_Rough->currNetWorkInfo.lteModemINfo.szNetwork_Type, 32, "%s", runRouterInfo.szNetMode);
		snprintf(pSys_Rough->currNetWorkInfo.lteModemINfo.szSN, 64, "%s", runRouterInfo.szId);

		Push_BlobValue(SETTYPE_RAW_ST, pSys_Rough->hSampler, 1, 3, 
			(void*)&pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
		//now update the bootInfo
		pSys_Rough->blobOCPP_BootInfo.emBootInfoOptions.imsi = 1;
		snprintf(pSys_Rough->blobOCPP_BootInfo.szIMSI, 20, "%s", runRouterInfo.szIMSI);
		pSys_Rough->blobOCPP_BootInfo.emBootInfoOptions.iccid = 1;
		snprintf(pSys_Rough->blobOCPP_BootInfo.szICCID, 32, "%s", runRouterInfo.szICCID);
		Push_BlobValue(SETTYPE_INTERN_SETT, pSys_Rough->hSampler, 1, 17, 
			(void*)&pSys_Rough->blobOCPP_BootInfo, sizeof(A_BLOB_CHARGER_BOOTINFO), BLOB_TYPE_CHARGER_BOOTINFO);
		
	}
	//judge if need log the version Info
	if((strncmp(s_OldExtRouterStatusInfo.szId, runRouterInfo.szId, 64) != 0) || 
		(strncmp(s_OldExtRouterStatusInfo.szSWVer, runRouterInfo.szSWVer, 16) != 0))
	{
		//now log the version
		LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_INFO,
			 "The Ext-Router id[%s] is running on VERSION[%s]", runRouterInfo.szId, runRouterInfo.szSWVer);
	}

	memcpy(&s_OldExtRouterStatusInfo, &runRouterInfo, sizeof(EXT_ROUTER_STATUS_INFO));

}

char* DoAParse_Sec2Line_4ExtRouter(char* szSrcLine, char* szSecName)
{
	char * pos = NULL;
	if(strlen(szSrcLine) < 4)
	{
		return NULL;
	}

	char* tmp = strstr(szSrcLine, "var");
	if(tmp != NULL)
	{
		tmp = strstr(szSrcLine, szSecName);
		if(tmp != NULL)
		{
			tmp += strlen(szSecName) + 1;
			int n = 0;
			char c;
			for(n = 0; n < 10; n++, tmp++)
			{
				c = *tmp;
				if(c == 0)
				{
					break;
				}
				else if((c != ' ') && (c != '=') && (c != '"'))
				{
					break;
				}
			}

			for(n = strlen(szSrcLine) - 1; n > 0; n--)
			{
				if(*(szSrcLine + n) == '"')
				{
					*(szSrcLine + n) = '\0';
					break;
				}
			}

			//if((*tmp != '"') && (*tmp != '\n') && (*tmp == '\r') && (*tmp != '\0'))
			{
				pos = tmp;
			}			
		}
	}

	return pos;
}

#define TMP_FILENAME_ROUTERINFO	"/opt/appdata/data/ext_rtinfo"
static BOOL Parse_XJ_WR301D_HTTPInfo(SYSTEM_SAMPLER_DATA *pSys_Rough, EXT_ROUTER_STATUS_INFO* pRouterInfo)
{
	FILE *fp = fopen(TMP_FILENAME_ROUTERINFO, "r");
	if (fp == NULL)
	{
		printf("-------CANNNOT OPEN extRouterInfo file-------\n");
		return FALSE;
	}

	//read the content line by line to get
	char line [256];
	char* tmp;	
	BOOL bGetWanType = FALSE;
	BOOL bGetWanIP = FALSE;
	BOOL bGetSignalStrength = FALSE;
	//BOOL bGetIMSE = FALSE;
	BOOL bIsRet = FALSE;
	while(fgets(line, sizeof(line), fp))
	{
		//printf("\n----BEGIN parse => [%s]----\n", line);
		tmp = DoAParse_Sec2Line_4ExtRouter(line, "wantype");
		if(tmp)
		{
			bGetWanType = TRUE;
			printf("----The wantpe => [%s]----\n", tmp);	
			if(strlen(tmp) > 1)		
			{
				bIsRet = TRUE;
				pRouterInfo->bIsSim_Ready = TRUE;
				snprintf(pRouterInfo->szNetMode, 32, "%s", tmp);
			}
		}
		tmp = DoAParse_Sec2Line_4ExtRouter(line, "wanIP");
		if(tmp)
		{
			bGetWanIP = TRUE;
			printf("----The wanIP => [%s]----\n", tmp);	
			if(strlen(tmp) > 1)		
			{
				if(strstr(tmp, "0.0.0.0") == NULL)
				{
					bIsRet = TRUE;
					pRouterInfo->bIsConnOK = TRUE;
				}				
			}
		}
		tmp = DoAParse_Sec2Line_4ExtRouter(line, "signalstrength");
		if(tmp)
		{
			bGetSignalStrength = TRUE;
			printf("----The signalstrength => [%s]----\n", tmp);	
			if(strlen(tmp) > 1)		
			{
				bIsRet = TRUE;
				pRouterInfo->nRSSI = atoi(tmp);			
			}
		}
		if((bGetWanType == TRUE) && (bGetWanIP == TRUE) && (bGetSignalStrength == TRUE))
		{
			break;
		}

		memset(line, 0, 256);
	}
	fclose(fp);

	return bIsRet;
}

static void Manage_XJ_WR301D_ExtRouter(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
#define HTTPGET_ROUTERINFO_STAT_IDLE		0
#define HTTPGET_ROUTERINFO_STAT_WAITRESP	1
#define HTTPGET_ROUTERINFO_STAT_GETERROR	2
	static time_t 	s_tmLastGetRouterInfo = 0;
	static int		s_emRouterGetHTTP_Status = HTTPGET_ROUTERINFO_STAT_IDLE;
	static HANDLE	s_hRouterUsePort = NULL;

	time_t 	tmNow = _NOW_;
	int nOpenERROR = ERR_COMM_OK;
	static EXT_ROUTER_STATUS_INFO s_OldExtRouterStatusInfo;
	static BOOL s_bIsFirstTimeBoot = TRUE;
	static BOOL s_bIsCommFail = FALSE;
	if(s_bIsFirstTimeBoot || s_bIsCommFail)
	{
		if(s_bIsFirstTimeBoot)
		{
			ZERO_POBJS(&s_OldExtRouterStatusInfo, 1);
		}		
		s_bIsFirstTimeBoot = FALSE;
	}

	EXT_ROUTER_STATUS_INFO runRouterInfo;
	ZERO_POBJS(&runRouterInfo, 1);

	if(s_emRouterGetHTTP_Status == HTTPGET_ROUTERINFO_STAT_IDLE)
	{
		//get it in every 5sec
		if(ABS(tmNow - s_tmLastGetRouterInfo) < 5)
		{
			return;
		}
		s_tmLastGetRouterInfo = tmNow;
		//open the port first
		unlink(TMP_FILENAME_ROUTERINFO);
		s_hRouterUsePort = pSys_Rough->commHttpFtp.Open("http://192.168.10.1/index.asp",
			TMP_FILENAME_ROUTERINFO, 0, 1, &nOpenERROR); //1s timeout SHALL be enough!
		if((nOpenERROR != ERR_COMM_OK) || (s_hRouterUsePort == NULL))
		{
			printf("\n----ERRRRRRRR11111111-----\n");
			s_hRouterUsePort = NULL;
			s_emRouterGetHTTP_Status = HTTPGET_ROUTERINFO_STAT_GETERROR;
			return;
		}
		//now GET the info
		if(pSys_Rough->commHttpFtp.Read(s_hRouterUsePort, "admin:admin", -999) != 0)
		{
			printf("\n----ERRRRRRRR2222222-----\n");
			pSys_Rough->commHttpFtp.Close(s_hRouterUsePort);
			s_hRouterUsePort = NULL;
			s_emRouterGetHTTP_Status = HTTPGET_ROUTERINFO_STAT_GETERROR;
			return;
		}
		//now goto the WAITING...
		s_emRouterGetHTTP_Status = HTTPGET_ROUTERINFO_STAT_WAITRESP;
	}
	else if(s_emRouterGetHTTP_Status == HTTPGET_ROUTERINFO_STAT_WAITRESP)
	{
		int nStatus = pSys_Rough->commHttpFtp.Control(s_hRouterUsePort, COMM_GET_HTTPFTP_PROCESS, NULL, NULL);
		if(nStatus == ERR_COMM_OK)
		{			
			pSys_Rough->commHttpFtp.Close(s_hRouterUsePort);
			s_hRouterUsePort = NULL;
			//now parse the file
			printf("\n----DOING the parsing-----\n");
			if(!Parse_XJ_WR301D_HTTPInfo(pSys_Rough, &runRouterInfo))
			{
				printf("\n----ERRRRRRRR3333333333-----\n");
				s_emRouterGetHTTP_Status = HTTPGET_ROUTERINFO_STAT_GETERROR;
				return;
			}
			s_bIsCommFail = FALSE;
			s_emRouterGetHTTP_Status = HTTPGET_ROUTERINFO_STAT_IDLE;
		}
		else if(nStatus != ERR_COMM_HTTPFTP_PENDING)
		{
			pSys_Rough->commHttpFtp.Close(s_hRouterUsePort);
			s_hRouterUsePort = NULL;
			printf("\n----ERRRRRRRR444444-----\n");
			s_emRouterGetHTTP_Status = HTTPGET_ROUTERINFO_STAT_GETERROR;
			return;
		}
	}
	else //error
	{
		s_bIsCommFail = TRUE;
		if(ABS(tmNow - s_tmLastGetRouterInfo) > 30)
		{
			s_emRouterGetHTTP_Status = HTTPGET_ROUTERINFO_STAT_IDLE;
		}
	}
}

void Manage_ExternalRouter(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
	TRACEW("Manage_ExternalRouter");
	//-----Jimmy note20210914: do NOT consider other brands other than ZLWL as testing supports ZLWL!!!
	//-----Also the other brands do NOT support API and statbility is also no better than ZLWL
	//if(pSys_Rough->currNetWorkInfo.bIsEnable_LTEModem == 2)
	int nInstallType = pSys_Rough->currNetWorkInfo.bIsEnable_LTEModem;
	if (nInstallType == LTEMODE_INSTTYPE_NOTINST)
	{
		if (s_bIsFirstTimeBoot == FALSE) 		//Already initialized , then clear Data
		{
			//ZERO_POBJS(&s_OldExtRouterStatusInfo, 1); 
			s_bIsFirstTimeBoot = TRUE;

			pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_dBm = 0;
			pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 0;
			pSys_Rough->currNetWorkInfo.netInfo_usb0.emNetPort_Status = NETPORT_STATUS_NOTINST;

			memset(pSys_Rough->currNetWorkInfo.lteModemINfo.szIMSI, 0, 20);
			memset(pSys_Rough->currNetWorkInfo.lteModemINfo.szICCID, 0, 32);
			memset(pSys_Rough->currNetWorkInfo.lteModemINfo.szNetwork_Type, 0, 32);
			memset(pSys_Rough->currNetWorkInfo.lteModemINfo.szSN, 0, 64);

			Push_BlobValue(SETTYPE_RAW_ST, pSys_Rough->hSampler, 1, 3, 
				(void*)&pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);

			pSys_Rough->blobOCPP_BootInfo.emBootInfoOptions.imsi = 0;
			memset(pSys_Rough->blobOCPP_BootInfo.szIMSI, 0, 20);
			pSys_Rough->blobOCPP_BootInfo.emBootInfoOptions.iccid = 0;
			memset(pSys_Rough->blobOCPP_BootInfo.szICCID, 0, 32);
			
			Push_BlobValue(SETTYPE_INTERN_SETT, pSys_Rough->hSampler, 1, 17, 
				(void*)&pSys_Rough->blobOCPP_BootInfo, sizeof(A_BLOB_CHARGER_BOOTINFO), BLOB_TYPE_CHARGER_BOOTINFO);
		}
	}
	else if (nInstallType == LTEMODE_EXTTTYPE_ZR2000)
	{
		return Manage_ZLWL_ZR2000_ExtRouter(pSys_Rough);
	}
	// else if(pSys_Rough->currNetWorkInfo.bIsEnable_LTEModem == 3)
	// {
	// 	return Manage_XJ_WR301D_ExtRouter(pSys_Rough);
	// }
}