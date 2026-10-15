// #define _TRACE_PRINT_ // SEMSClient
//#define   DataDebug
#include "slave_ems.h"

#define INIT_NL_MSGINFO(p, _msgType, _pfnParser, _byCmd, _usMinDataLen) ( \
	(p+_msgType)->msgTp = (_msgType), \
	(p+_msgType)->byCmd = (_byCmd), \
	(p+_msgType)->pfnParser = (_pfnParser), \
	(p+_msgType)->usMin_ExpectDataLen = (_usMinDataLen))

//static COMMPORT		s_CustomizeUdpClient;
static COMMPORT* 	s_pUDPCommClient = NULL;
static HANDLE 	 	s_hUDPPortClient = NULL;
typedef struct tagEvHandTags
{
	UINT nEvTag;
	UINT nMsgTag;
}EVHAND_TAGS;

char* Make_IPV42String(ROUGH_SLAVE_ROUGH* pSlave, ULONG ulIpV4_IP, char* pszOut)
{
	UNUSED(pSlave);
	memcpy(pszOut, inet_ntoa(*((struct in_addr*)(&ulIpV4_IP))), 32);

	return pszOut;
}

static int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE	emEvType,
	_EVENT_MSG	emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int			nMaxBlockMSec)
{
	UNUSED(bBlockMode);
	UNUSED(nMaxBlockMSec);
	REPORTER* pReporter = (REPORTER*)hModuleSeif;
	ROUGH_SLAVE_ROUGH* pSlave = (ROUGH_SLAVE_ROUGH*)pReporter->pDataHeap;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	RunThread_Heartbeat(pReporter->hThread); //in case the thread no response
	switch (emEvType)
	{
		case EVENT_TYPE_ALARM:
		{
			EVENT_ALARM * pAlmEv = (EVENT_ALARM*)pEvData;
			if (nEvDataSize != sizeof(EVENT_ALARM))
			{
				return EVENT_HANDED_FAILED;
			}

			pSlave->bNeedRefreshActvAlm = TRUE;
			TRACED("SLaveEMS_Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);

			Parse_AlarmLight_Status(pSlave, PROTOCOL_TCP);// edit by Kuangyue 20251112
		}
		break;
		case EVENT_TYPE_FIRMWARE_UPDATE:
		{
			TRACED("Slave  _EVENT_TYPE = EVENT_TYPE_FIRMUPDATE \t emMsgType = %d, size[%d,%d] \n", 
			emMsgType, nEvDataSize, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
			EVENT_FIRMWAREUPDATE_STATUS * pFirmUpSta = (EVENT_FIRMWAREUPDATE_STATUS*)pEvData;
			if (nEvDataSize != sizeof(EVENT_FIRMWAREUPDATE_STATUS))
			{
				return EVENT_HANDED_FAILED;
			}
			Parse_Upgrade_Status(pSlave, pFirmUpSta->emUpdateStatus, PROTOCOL_TCP);
		}
		break;
		case EVENT_TYPE_DIAGNOSTIC_UPLOAD:
		{
			TRACED("SLaveEMS_Event_Handler  _EVENT_TYPE = EVENT_TYPE_DIAGNOSTIC_UPLOAD, emMsgType = %d \n", emMsgType);
		}
		break;
		case EVENT_TYPE_ABILITY_CHANGE:
		{
			TRACED("SLaveEMS_Event_Handler  _EVENT_TYPE = EVENT_TYPE_ABILITY_CHANGE, emMsgType = %d \n", emMsgType);

		}
		break;
		case EVENT_TYPE_SYSTEM_EXIT:
		{
			EVENT_SYSTEM_EXIT* pEvReboot = (EVENT_SYSTEM_EXIT*)pEvData;
			if(nEvDataSize != sizeof(EVENT_SYSTEM_EXIT))
			{
				return EVENT_HANDED_FAILED;
			}
			TRACED("SLaveEMS_Event_Handler  _EVENT_TYPE = EVENT_TYPE_SYSTEM_EXIT, EXIT_TYPE = %d \n", pEvReboot->nExitType);
		}
		break;

		case EVENT_TYPE_CFG_CHANGED:
		{
			TRACED("SLaveEMS_Event_Handler  _EVENT_TYPE = EVENT_TYPE_CFG_CHANGED, emMsgType = %d \n", emMsgType);
			EVENT_PARAM_CHG * pParamChg = (EVENT_PARAM_CHG*)pEvData;
			if (nEvDataSize != sizeof(EVENT_PARAM_CHG))
			{
				return EVENT_HANDED_FAILED;
			}

			if ((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 26))
			{
				TRACED("SLaveEMS_Event_Handler Config Changed Evevt \n");
				PARAM_VAL_GET valGet;
				valGet.nGetSize = sizeof(A_BLOB_ES_BACKEND_CFG);
				valGet.val.blobValue = (BYTE*)&pSlave->blobBackendSet;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 26, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					TRACEE("SLaveEMS_Event_Handler ERROR: settings can not be retrieved\n");
					return FALSE;
				}
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 53))//站级EMS URL
			{
				valGet.nGetSize = 255;
				valGet.val.blobValue = (BYTE*)pSlave->szStationEMSUrl; //&pOCPP->szServerURL;
				pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 53, &valGet, sizeof(PARAM_VAL_GET));
				TRACEI("new Station EMS URL: [%s]", pSlave->szStationEMSUrl);
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 84))//SEMS_LSYS上报周期
			{
				valGet.nGetSize = 4;
				pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 84, &valGet, sizeof(PARAM_VAL_GET));
				pSlave->Sent_SEMS_LSYS_Period = valGet.val.emValue;
				TRACEI("new Sent_SEMS_LSYS_Period: [%d]", pSlave->Sent_SEMS_LSYS_Period);
			}
			else if ((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 3))//从机IP
			{

				valGet.nGetSize = sizeof(A_BLOB_NETWORKINFO);
				valGet.val.blobValue = (BYTE*)&pSlave->blobNetWorkInfo;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 3, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					TRACEE("ERROR: slave get A_BLOB_NETWORKINFO settings can not be retrieved !!");
					return FALSE;
				}		
			}
			else if((pParamChg->parmPTR.nId == 500) && (pParamChg->parmPTR.nSubId == 502))//nCfgBMSNum
			{
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					500, 502, &valGet, sizeof(PARAM_VAL_GET));
				pSlave->nCfgBMSNum = valGet.val.emValue;
				if(pSlave->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
				{
					pSlave->nCfgBMSNum = MAX_SINGLE_STBATT_NUM;
				}
				TRACEI("new nCfgBMSNum=[%d]\n", pSlave->nCfgBMSNum);
			}
			else if ((pParamChg->parmPTR.nSubId == 83))
			{
				// Read Config 
				valGet.nGetSize = 512;
				valGet.val.blobValue = (BYTE*)pSlave->szShieldAlarms;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 83, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("ERROR: IFY settings can not be retrieved\n");
					return FALSE;
				}
				TRACEI("Config Changed Evevt [%s]\n",pSlave->szShieldAlarms);
			}
			else if((pParamChg->parmPTR.nId == 1001) &&
				(pParamChg->parmPTR.nSubId == 3313 || pParamChg->parmPTR.nSubId == 3314 ||
				pParamChg->parmPTR.nSubId == 3315))
			{// LJH250428 cmd
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					pParamChg->parmPTR.nId, pParamChg->parmPTR.nSubId, &valGet, sizeof(PARAM_VAL_GET));
				TRACED("slave_ems_main::Event_Handler SEMS Changed err:%d nId:%d nSubId:%d ",
					err, pParamChg->parmPTR.nId, pParamChg->parmPTR.nSubId);
				if (err != ERRCODE_DAI_OK)
				{
					TRACEE("slave_ems_main pDAI->Get err:%d", err);
					return EVENT_HANDED_FAILED;
				}
				switch (pParamChg->parmPTR.nSubId)
				{
					case 3313: // Send CMD Info To LC Server 1001, 3313,
						pSlave->lcCfg.bSendCmdByLC1stOnly = (BOOL)valGet.val.emValue;
						break;
					case 3314: // Max ESC Num In Branch 1001, 3314,
						pSlave->lcCfg.nESCNum = valGet.val.ulValue;
						break;
					case 3315: // Branch Config Number 1001, 3315,
						pSlave->lcCfg.nBranchNum = valGet.val.ulValue;
						break;
					default:
						break;
				}
			}
		}
		break;
	default:
		break;
	}
	return EVENT_HANDED_OK;
}

static BOOL UDP_ParseOpenParams(char *pOpenParams, char *pdwIP, int *piPort)
{
	char	*pszPort = NULL;
	char	*p = pOpenParams;
	char	szIP[32];	// to save ascii IP: 255.255.255.255:65536 
	int		i;
	BOOL	bDomainName = FALSE;

	for (i = 0; (i < (int)(sizeof(szIP)-1)) && (*p != 0); i++)
	{
		/* only 0-9, '.' and ':' are allowed	*/
		if (((*p >= '0') && (*p <= '9')) || (*p == '.'))
		{
			szIP[i] = *p;		
		}
		else if (*p == ':')
		{
			szIP[i] = 0;	/* replace the colon with '\0', IP end	*/
			pszPort = &szIP[i+1];		/* the port string start	*/
		}
		else if(((*p >= 'a') && (*p <= 'z')) || ((*p >= 'A') && (*p <= 'Z')))
		{
			bDomainName = TRUE;
			szIP[i] = *p;
		}
		else		// error character			
		{
			return	FALSE;
		}
		p++;
	}

	szIP[i] = 0;		// end flag
	
	strncpy(pdwIP, szIP, sizeof(szIP));		

	TRACE("UDP_ParseOpenParams szIP=[%s] pszPort=[%s]\n", szIP, pszPort);
	if (pszPort == NULL)
	{
		TRACE("Err: pszPort == NULL!!!\n");
		return FALSE;	// error parameters		//
	}

	// convert port		//
	*piPort = atoi( pszPort );	// host format.

	TRACEX("222  szIP=[%s] pszPort=[%s]\n", szIP, pszPort);
 
	return TRUE;
}


static BOOL ReOpen_QuicklyCommPort_UDP_CLIENT(REPORTER* pReporter)
{
	ROUGH_SLAVE_ROUGH* pSlave = (ROUGH_SLAVE_ROUGH*)pReporter->pDataHeap;
	int nErrCode = 0;
	int portId = 11;	//udp
	int nSockPort;
	char	szIP[32];

	s_pUDPCommClient = NEW(COMMPORT,1);
    if (s_pUDPCommClient == NULL) 
	{
        TRACE("s_pUDPCommClient Memory allocation failed\n");
        return FALSE;
    }

	COMM_UDP_SET * sUdpServerInfo = NEW(COMM_UDP_SET,1);
    if (sUdpServerInfo == NULL) 
	{
        TRACE("sUdpServerInfo Memory allocation failed\n");
        return FALSE;
    }

	// 2. parse IP and socket port info .
	if (!UDP_ParseOpenParams(pSlave->szStationEMSUrl, szIP, &nSockPort))
	{
		return	FALSE;
	}

	ZERO_POBJS(sUdpServerInfo,1);
    sUdpServerInfo->nNonBlock = 0;	//阻塞
    sUdpServerInfo->sPeerInfo.nPeerPort = 9989;//nSockPort+1;//tcp:9988 udp:9989
    strncpy(sUdpServerInfo->sPeerInfo.szAddr, szIP, sizeof(sUdpServerInfo->sPeerInfo.szAddr));

	if (Reporter_GetCommPort_ById((HANDLE)pReporter, portId, s_pUDPCommClient) == NULL)
	{
		TRACEE("Slave ReOpen_QuicklyCommPort_UDP_CLIENT:GetCommPort_ById:%d failed", portId);
		return FALSE;
	}
	//strncpy(s_CustomizeUdpClient.info.szPortAttri, "sems_udp_client_BLOCK", sizeof(s_CustomizeUdpClient.info.szPortAttri));//block wait
	//s_pUDPCommClient = &s_CustomizeUdpClient;

	TRACE("ReOpen_QuicklyCommPort_UDP_CLIENT--szPortName=[%s]-szPortAttri=[%s]-szPortDriver=[%s]-!!! \n",\
		s_pUDPCommClient->info.szPortName,\
		s_pUDPCommClient->info.szPortAttri,\
		s_pUDPCommClient->info.szPortDriver);


	s_hUDPPortClient = s_pUDPCommClient->Open(
		//s_pUDPCommClient->info.szPortAttri,
		(char*)sUdpServerInfo,
		//pSlave->szStationEMSUrl,
		"0.0.0.0:10005",
		COMM_CLIENT_MODE, 500, &nErrCode);
	if(ERR_COMM_OK != nErrCode)
	{
		return FALSE;
	}

	return TRUE;
}
static BOOL JudgeisConnect(ROUGH_SLAVE_ROUGH* pSlave)
{
	VAR_VALUE varV;

	if (pSlave->nisFirstBoot == TRUE)//first Time no need to judge 
	{
		pSlave->nisFirstBoot = FALSE;
		return TRUE;
	}

	if (pSlave->bIsRegistered)//是否连接上服务端
	{
		if (ElapseS(pSlave->tmRegiseterd) > 30) //判断注册时长  		
		{
			// if (ElapseS(pSlave->tmLastRecv_USYS) > 10)//USYS周期；注册时长超过1分钟后，还没有收到USYS；
			if (ElapseS(pSlave->tmLastRecv) > 10) // LJH250625 收到任何server数据就认为通信成功
			{
				pSlave->nCommFailTimes++;
				if(pSlave->nCommFailTimes >= 5)
				{
					pSlave->nCommFailTimes = 5;
					varV.emValue = 1; 
					pSlave->bIsRegistered =FALSE ; //重连
				}
			}
			else
			{
				varV.emValue = 0;
				pSlave->nCommFailTimes = 0;
			}
		}
		else
		{
			varV.emValue = 0;
		}
	}
	else
	{
		varV.emValue = 1;
	}
	pSlave->pReporter->pDAI->Set((HANDLE)pSlave->pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
		1, 33, &varV, 4, 100);
	// LJH0222 通信中断后设置STATU_ACDC_IDLE fActv_Pwr = 0 从Ems_Mgmt_Main_ESS移到这里
	if (varV.emValue == 1)
	{
		A_BLOB_SEMS2SLAVE_6330 blobRev6330;
		S63Uctl_30WorkPwr* pStRev6330 = &blobRev6330.stRev6330;
		ERRCODE_DAI nErrCode;
		PARAM_VAL_GET valGet;
		valGet.nGetSize = sizeof(A_BLOB_SEMS2SLAVE_6330);
		valGet.val.blobValue = (BYTE*)&blobRev6330;
		nErrCode = pSlave->pReporter->pDAI->Get((HANDLE)pSlave->pReporter, DAITYPE_STAT_PARAMVALUE_R, 
			200, 206, &valGet, sizeof(PARAM_VAL_GET));
		TRACE("JudgeisConnect DisConnect Get nErrCode:%d emBlobType:%d emACDC_WorkStatuReq:%d fActv_Pwr:%.1f",
			nErrCode, blobRev6330.emBlobType, pStRev6330->emACDC_WorkStatuCtrl, pStRev6330->fActv_PwrCtrl);
		// set
		blobRev6330.emBlobType = BLOB_TYPE_SEMS2SlAVE_6330;
		varV.blobValue = (BYTE*)&blobRev6330;
		pStRev6330->emACDC_WorkStatuCtrl = STATU_ACDC_IDLE;
		pStRev6330->fActv_PwrCtrl = 0;
		nErrCode = pSlave->pReporter->pDAI->Set((HANDLE)pSlave->pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W,
			200, 206, &varV, sizeof(A_BLOB_SEMS2SLAVE_6330), 1000);
		TRACEW("JudgeisConnect DisConnect Set STATU_ACDC_IDLE nErrCode:%d emBlobType:%d emACDC_WorkStatuReq:%d fActv_Pwr:%.1f",
			nErrCode, blobRev6330.emBlobType, pStRev6330->emACDC_WorkStatuCtrl, pStRev6330->fActv_PwrCtrl);
	}

	return TRUE;
}

static BOOL TryConnect2Server(ROUGH_SLAVE_ROUGH* pSlave)
{
	COMM_TIMEOUTS timeOut;

	if (pSlave->bIsRegistered)
	{
		return TRUE;
	}
	REPORTER* pReporter = pSlave->pReporter;
	int  nErrCode;

	pReporter->pUsedCommPort->Close(pReporter->hCommPort);
	pReporter->hCommPort = pReporter->pUsedCommPort->Open("COM_SLAVE_TCP",
		pSlave->szStationEMSUrl, COMM_CLIENT_MODE, 1000, &nErrCode);
	if(nErrCode != ERR_COMM_OK)
	{
		//LogOut("OpenGUI Comm Port Failed-->%d");
		TRACEE("COM_SLAVE_TCP Open TCP Error:%d\n", nErrCode);
		return FALSE;
	}
			
	timeOut.nReadTimeout = 500;
	timeOut.nWriteTimeout = 500;
	timeOut.nIntervalTimeout = 50;
	pReporter->pUsedCommPort->Control(pReporter->hCommPort,COMM_SET_TIMEOUTS,&timeOut, sizeof(COMM_TIMEOUTS));

	TRACEI("COM_SLAVE_TCP TryConnect2Server successfully ! ! ! ");
 
	pSlave->bIsRegistered = TRUE;
	pSlave->tmRegiseterd = _NOW_;
	pSlave->bNeedRefreshActvAlm = TRUE;
	return TRUE;
}

int Slave_WriteData(ROUGH_SLAVE_ROUGH* pSlave, BYTE *szDataBuff, BYTE CMD, int nSendDataLen, PROTOCOL_TYPE bProtocol)
{
	REPORTER* pReporter = pSlave->pReporter;

	int nWrite = 0;
	BYTE sendBuff[TCP_BUFF_LEN];
	memset(sendBuff, 0, TCP_BUFF_LEN);

	//SOI
	sendBuff[0] = (BYTE)(NLMSG_SOI >> 16);
	sendBuff[1] = (BYTE)(NLMSG_SOI >> 8);
	sendBuff[2] = (BYTE)(NLMSG_SOI & 0xFF);

	//CMD
	sendBuff[3] = CMD;

	//DataLen
	sendBuff[4] = (BYTE)(nSendDataLen >> 8);
	sendBuff[5] = (BYTE)(nSendDataLen & 0xFF);

	//Data
	memcpy(sendBuff + 6, szDataBuff, (size_t)nSendDataLen);

	//Calc chksum
	USHORT ulCalcSum = MakeChkSum(CMD, (USHORT)nSendDataLen, sendBuff + 6);
	sendBuff[nSendDataLen + 6] = (BYTE)(ulCalcSum >> 8);
	sendBuff[nSendDataLen + 7] = (BYTE)(ulCalcSum & 0xFF);

	//EOI
	sendBuff[nSendDataLen + 8] = (BYTE)(NLMSG_EOI);

	//int nWrite = pReporter->pUsedCommPort->Write(pReporter->hCommPort, (char*)sendBuff, nSendDataLen + 9);
	if(bProtocol == PROTOCOL_TCP)
	{
		nWrite = pReporter->pUsedCommPort->Write(pReporter->hCommPort, (char*)sendBuff, nSendDataLen + 9);
	}
	else	//udp
	{
		nWrite = s_pUDPCommClient->Write(s_hUDPPortClient, (char*)sendBuff, nSendDataLen + 9);
	}

	//TRACE("Slave_WriteData nWrite:%d nSendLen:%d szBuff<%s> bProtocol[%d]\n", nWrite, nSendDataLen + 9, printableBytes(sendBuff, nSendDataLen + 9),bProtocol);
#ifdef _TRACE_PRINT_
	switch (CMD)
	{
	case 0x61: // CFGG
		TRACED("Slave_WriteData cmd:%02X CFGG byBuff<%s>", CMD, printableBytes(sendBuff, nWrite));
		break;
	case 0x62: // CFGS
		TRACED("Slave_WriteData cmd:%02X CFGS byBuff<%s>", CMD, printableBytes(sendBuff, nWrite));
		break;
	case 0x63: // UCTL
		TRACEI("Slave_WriteData cmd:%02X UCTL sub:%02X byBuff<%s>", CMD, sendBuff[6], printableBytes(sendBuff, nWrite));
		break;
	case 0x64: // RTIF
		TRACED("Slave_WriteData cmd:%02X RTIF byBuff<%s>", CMD, printableBytes(sendBuff, nWrite));
		break;
	case 0xB6: // LSYS
		TRACED("Slave_WriteData cmd:%02X LSYS byBuff<%s>", CMD, printableBytes(sendBuff, nWrite));
		break;
	case 0xC6: // LDEV
		TRACED("Slave_WriteData cmd:%02X LDEV byBuff<%s>", CMD, printableBytes(sendBuff, nWrite));
		break;

	default:
		TRACED("Slave_WriteData cmd:%02X byBuff<%s>", CMD, printableBytes(sendBuff, nWrite));
		break;
	}
#endif
	return nWrite;
}

int Slave_ReadData(ROUGH_SLAVE_ROUGH* pSlave, BYTE *rdBuff, int nReadLen)
{
	REPORTER* pReporter = pSlave->pReporter;

	int nReadBytes = pReporter->pUsedCommPort->Read(pReporter->hCommPort, (char*)rdBuff, nReadLen);
	if (nReadBytes != 0)
	{
		TRACED("Slave_ReadData nReadBytes:%d nReadLen:%d <%s>\n", nReadBytes, nReadLen, printableBytes(rdBuff, nReadBytes));
	}
	
	return nReadBytes;
}

USHORT MakeChkSum(BYTE byCmd, USHORT usDataLen, BYTE* byData)
{
	USHORT ulCalcSum = (USHORT)((USHORT)byCmd + (BYTE)(usDataLen >> 8) + (BYTE)usDataLen);
	//TRACE("ulCalcSum=%d byCmd=%02X =%d\n",ulCalcSum , byCmd , usDataLen);
	int n;
	for(n = 0; n < usDataLen; n++)
	{
		ulCalcSum += byData[n];
	}

	ulCalcSum &= 0xFFFF;
	//ulCalcSum = (~ulCalcSum) + 1;

	return ulCalcSum;
}

NLMSG_FRAME_INFO* Find_NLMsgFrame(IN ROUGH_SLAVE_ROUGH* pSlave, IN BYTE* pRaw, IN int nReadLen, 
	OUT BYTE** ppDataPos, OUT USHORT* pusDataLen)
{
	int n, m;
	BYTE* byPos = pRaw;
	USHORT nParsedDataLen = 0;
	NLMSG_FRAME_INFO* pFoundMsg = NULL;
	//Notice: only find the first frame, if have multi frames, ignore the followed others
	for(n = 0; n < nReadLen - NLMSG_MIN_LEN + 1; n++)
	{
		if((byPos[n] == (BYTE)(NLMSG_SOI >> 16)) && 
			(byPos[n + 1] == (BYTE)(NLMSG_SOI >> 8)) &&
			(byPos[n + 2] == (BYTE)(NLMSG_SOI & 0xFF)))
		{
			//CMD;
			for(m = 0; m < NL_MSG_Max; m++)
			{
				if(pSlave->nlFrameInfo[m].byCmd == byPos[n + 3])
				{
					pFoundMsg = &pSlave->nlFrameInfo[m];
					// TRACED("Had Found nlFrameInfo[%d].byCmd=[%02X]\n", n,pCCU->nlFrameInfo[m].byCmd);
					break;
				}
			}
			
			if(pFoundMsg == NULL)
			{
				// TRACED("Maybe CCU Transfer Error Cmd,Cause System reboot\n");
				continue;
			}	
			//now check data length;
			nParsedDataLen = ((USHORT)byPos[n + 4] << 8) + byPos[n + 5];

			if((nParsedDataLen > TCP_BUFF_LEN) || (nParsedDataLen < pFoundMsg->usMin_ExpectDataLen))
			{
				// TRACED("Found Msg[%02X] but the datalen[%d] is INVALID\n", byPos[n + 3], nParsedDataLen);
				//return NULL;
				pFoundMsg = NULL;
				continue;
			}

			//now check chksum
			USHORT ulCalcSum = MakeChkSum(pFoundMsg->byCmd, nParsedDataLen, byPos + n + 6);
			USHORT ulRealSum = ((USHORT)byPos[n + 6 + nParsedDataLen] << 8) + byPos[n + 7 + nParsedDataLen];
			if(ulCalcSum != ulRealSum)
			{
				TRACED("Found Msg[%02X] nParsedDataLen =%d but the chkSum[%02X, %02X] is DIFFERENT\n", byPos[n + 3], 
					nParsedDataLen, ulCalcSum, ulRealSum);
				//return NULL;
				pFoundMsg = NULL;
				continue;
			}
			//EOI
			if(byPos[n + 8 + nParsedDataLen] != NLMSG_EOI)
			{
				TRACED("Found Msg[%02X] but the EOI[%02X] is INVALID\n", byPos[n + 3], 
					byPos[n + 8 + nParsedDataLen]);
				//return NULL;
				pFoundMsg = NULL;
				continue;
			}
			//now succeccfully get the frame
			*ppDataPos = byPos + n + 6;
			*pusDataLen = nParsedDataLen;
			break;
		}
	}	
	return pFoundMsg;
}

static void Recv_Tcp_StationEms_Msg(ROUGH_SLAVE_ROUGH* pSlave)
{
	BYTE byBuff[TCP_BUFF_LEN];
	memset(byBuff, 0, TCP_BUFF_LEN);
	int nReadBytes = Slave_ReadData(pSlave, byBuff, sizeof(byBuff));
	if(nReadBytes >= NLMSG_MIN_LEN)
	{
		//now try to find the frame and parse it
		BYTE* pFoundPos = byBuff;
		do
		{
			BYTE *pDataPos;
			USHORT usDataLen;
			NLMSG_FRAME_INFO* pFrm = Find_NLMsgFrame(pSlave, pFoundPos, nReadBytes, &pDataPos, &usDataLen);
			if(pFrm == NULL)
			{
				TRACEE("---Get Frm Is NULL---\n");
				break;
			}
			else if((pFrm->byCmd == pSlave->nlFrameInfo[NL_MSG_CFGG].byCmd)||
					(pFrm->byCmd == pSlave->nlFrameInfo[NL_MSG_CFGS].byCmd)||
					(pFrm->byCmd == pSlave->nlFrameInfo[NL_MSG_UCTL].byCmd)||
					(pFrm->byCmd == pSlave->nlFrameInfo[NL_MSG_RTIF].byCmd)||
					(pFrm->byCmd == pSlave->nlFrameInfo[NL_MSG_USYS].byCmd)||
					(pFrm->byCmd == pSlave->nlFrameInfo[NL_MSG_PCSINFO].byCmd))
			{
				pSlave->tmLastRecv = _NOW_; // LJH250625 收到任何server数据就认为通信成功，现场经常报通信异常
				TRACEI("Recv_Tcp_StationEms_Msg byCmd=[%02X] NL_MSG_CFGG[%d] NL_MSG_CFGS[%d] NL_MSG_UCTL[%d] NL_MSG_RTIF[%d] NL_MSG_USYS[%d] ",
					pFrm->byCmd, pSlave->nlFrameInfo[NL_MSG_CFGG].byCmd,pSlave->nlFrameInfo[NL_MSG_CFGS].byCmd,pSlave->nlFrameInfo[NL_MSG_UCTL].byCmd,pSlave->nlFrameInfo[NL_MSG_RTIF].byCmd,pSlave->nlFrameInfo[NL_MSG_USYS].byCmd);
				pFrm->pfnParser(pSlave, pDataPos, usDataLen, PROTOCOL_TCP);

			}
			else
			{
				TRACE("Info: Maybe Other Msg byCmd=[%02X]\n", pFrm->byCmd);
			}
			nReadBytes -= usDataLen + NLMSG_MIN_LEN;
			pFoundPos += usDataLen + NLMSG_MIN_LEN;
		} while (nReadBytes >= NLMSG_MIN_LEN);
	}

	return;
}

BOOL ShieldAlarms_StringToUINT(ROUGH_SLAVE_ROUGH* pSlave)
{
	TRACEW("allow to shield alarms [%s]",pSlave->szShieldAlarms);
	UINT index = 0 ,ncount = 1;
	char szInput[512];

	ZERO_POBJS(pSlave->ShieldAlarms, pSlave->nAlarmInfosTotal);   //clear buff
    if (pSlave->szShieldAlarms[0] == '\0' || strcmp(pSlave->szShieldAlarms, "alm_id/alm_id/alm_id") == 0) 
	{
		TRACEE("NO alarm to be shield ...");
		pSlave->nShieldAlarmsNum = 0;
        return FALSE;
    }

    for (index = 0; pSlave->szShieldAlarms[index] != '\0'; index++) 
	{
        if (pSlave->szShieldAlarms[index] == '/') 
		{
            ncount++;
        }
    }
	pSlave->nShieldAlarmsNum = ncount;

	strncpy(szInput, pSlave->szShieldAlarms, 512);
	szInput[511] = '\0'; 

	char *token = strtok(szInput, "/");
    index = 0;

    while (token != NULL && index < ncount) 
	{
        pSlave->ShieldAlarms[index] = (UINT)strtoul(token, NULL, 10);
        token = strtok(NULL, "/");
		index++;
    }
    return TRUE;
}


void QueryActvAlarmInfo(ROUGH_SLAVE_ROUGH* pSlave)
{
	REPORTER* pReporter = (REPORTER*)pSlave->pReporter;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	ERRCODE_DAI errCode;
	int n = 0;

	EVENT_ALARM argActvAlm;
	QUERY_COND cond;
	cond.nArgSize = sizeof(EVENT_ALARM);
	cond.pCondArg = &argActvAlm;
	cond.nQueryCondType = 0;
	UINT nQueryCount = MAX_QUERY_ACTV_ALM;
 	ZERO_POBJS(pSlave->activeAlms, MAX_QUERY_ACTV_ALM);
 	errCode = pReporter->pDAI->Query((HANDLE)pReporter, QUERYTYPE_ACTV_ALARM, cond, (void*)pSlave->activeAlms , &nQueryCount);
	TRACE("-----slave ems Actv Alarm [%d],  err = %d-----\n", nQueryCount, errCode);
	TRACEI("-----slave ems Actv Alarm [%d],  err = %d-----\n", nQueryCount, errCode);

	if(errCode != ERRCODE_DAI_OK)
	{
		TRACE("Error: Query Actv Alarm Failed!\n");
 	}
	else
	{
		pSlave->nActiveAlmTotal = MIN(nQueryCount, MAX_QUERY_ACTV_ALM);
 		for(n = 0; n < pSlave->nActiveAlmTotal; n++)
		{
			TRACE("Info:Index[%d] nDevId=[%d] nAlarmId=[%d]\n",\
				n, pSlave->activeAlms[n].nDevId, pSlave->activeAlms[n].nAlarmId);
			TRACEI("Info:Index[%d] nDevId=[%d] nAlarmId=[%d]\n",\
				n, pSlave->activeAlms[n].nDevId, pSlave->activeAlms[n].nAlarmId);
		}
	}

	return;
}


void Pro_SysCmdDataInfo(ROUGH_SLAVE_ROUGH* pSlave)
{
	static const int DIVIDE_CMD_INTV = 3;
	time_t tmNow = _NOW_;
	// PARAM_VAL_GET 	valGet;
	// REPORTER* pReporter = pSlave->pReporter;
	
	// BOOL bSysStatuChanged = Check_IfStatusChanged(pSlave);
	// edit by KYA 20250520
	// if((ABS(tmNow - pSlave->tmLastSent_101) >= MAX(3, pSlave->blobBackendSet.nCmd_101_Interval)) || bSysStatuChanged)
	// {
	// 	pSlave->tmLastSent_101 = tmNow;
	// 	Send_SEMS_LDEV_C6_101(pSlave);
	// }

	// if(ABS(tmNow - pSlave->tmLastSent_102) >= MAX(3, pSlave->blobBackendSet.nCmd_102_Interval))
	// {
	// 	pSlave->tmLastSent_102 = tmNow;
	// 	Send_SEMS_LDEV_C6_102(pSlave);
	// }

 	 if(ABS(tmNow - pSlave->tmLastSent_103) >= MAX(3, pSlave->blobBackendSet.nCmd_103_Interval))
	 {
	 	Send_SEMS_LDEV_C6_103(pSlave,PROTOCOL_TCP);
		pSlave->tmLastSent_103 = tmNow;
	 }

	 if(ABS(tmNow - pSlave->tmLastSent_104) >= MAX(3, pSlave->blobBackendSet.nCmd_104_Interval))
	 {
	 	Send_SEMS_LDEV_C6_104(pSlave,PROTOCOL_TCP);
		 pSlave->tmLastSent_104 = tmNow;
	 }

	if(ABS(tmNow - pSlave->tmLastSent_105) >= MAX(3, pSlave->blobBackendSet.nCmd_105_Interval/DIVIDE_CMD_INTV))
	{
		pSlave->tmLastSent_105 = tmNow;
		Send_SEMS_LDEV_C6_155(pSlave,PROTOCOL_TCP);
	}

	// if(ABS(tmNow - pSlave->tmLastSent_106) >= MAX(3, pSlave->blobBackendSet.nCmd_106_Interval))
	// {
	// 	pSlave->tmLastSent_106 = tmNow;
	// 	Send_SEMS_LDEV_C6_106(pSlave);
	// }

	if(ABS(tmNow - pSlave->tmLastSent_107) >= MAX(3, pSlave->blobBackendSet.nCmd_107_Interval/DIVIDE_CMD_INTV))
	{// LJH250520 cmd157
		pSlave->tmLastSent_107 = tmNow;
		Send_SEMS_LDEV_C6_157(pSlave,PROTOCOL_TCP);
	}

	// if(ABS(tmNow - pSlave->tmLastSent_108) >= MAX(3, pSlave->blobBackendSet.nCmd_108_Interval))
	// {
	// 	pSlave->tmLastSent_108 = tmNow;
	// 	Send_SEMS_LDEV_C6_108(pSlave);
	// }

	 if((pSlave->bNeedRefreshActvAlm == TRUE)||
	 	(ABS(tmNow - pSlave->tmLastSent_109) >= MAX(3, pSlave->blobBackendSet.nCmd_109_Interval)))
	 {
		pSlave->bNeedRefreshActvAlm = FALSE;
		QueryActvAlarmInfo(pSlave);

		pSlave->tmLastSent_109 = tmNow;
	 	ShieldAlarms_StringToUINT(pSlave);
		Send_SEMS_LDEV_C6_109(pSlave,PROTOCOL_TCP);
	 }

	 if (pSlave->blobBackendSet.nCmd_110_Interval != 0 && // LJH250630 设置为0不上报到主控
		ABS(tmNow - pSlave->tmLastSent_110) >= MAX(3, pSlave->blobBackendSet.nCmd_110_Interval))
	{
		Send_SEMS_LDEV_C6_110(pSlave,PROTOCOL_TCP);

		pSlave->tmLastSent_110 = tmNow;
	}


	// if(ABS(tmNow - pSlave->tmLastSent_110) > (MAX(300, pSlave->blobBackendSet.nCmd_110_Interval)))
	// {
	// 	int n = 0;
	// 	for(n = 0; n < pSlave->nCfgBMSNum; n++)
	// 	{
	// 		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
	// 			501 + n, 550, &valGet, sizeof(PARAM_VAL_GET));
	// 		if(valGet.val.emValue <= STBATT_RUNSTAT_COMM_FAIL)
	// 		{
	// 			continue;
	// 		}
	// 		pSlave->nCurrBMS = n;
	// 		pSlave->stMsgInfoList[C_110_SingleBattryInfo].pfnMsgHandler((void*)pSlave);
	// 	}
	// 	pSlave->tmLastSent_110 = tmNow;
	// }
	

	// int nValue = tmNow - pSlave->tmLastSent_202;
	// TRACEX("tmNow:[%ld] - pSlave->tmLastSent_202[%d]:%ld = %d\n",
	// 	tmNow, pSlave->tmLastSent_202, nValue);
	// if(ABS(tmNow - pSlave->tmLastSent_202) >= MAX(3, pSlave->blobBackendSet.nCmd_202_Interval))
	// {
	// 	pSlave->tmLastSent_202 = tmNow;
	// 	pSlave->stMsgInfoList[C_202_HourSectorStaticKwh].pfnMsgHandler((void*)pSlave);
	// }

	// if(ABS(tmNow - pSlave->tmLastSent_204) >= MAX(3, pSlave->blobBackendSet.nCmd_204_Interval))
	// {
	// 	pSlave->tmLastSent_204 = tmNow;
	// 	pSlave->stMsgInfoList[C_204_DailyStaticKwh].pfnMsgHandler((void*)pSlave);
	// }

	// if(pSlave->blobBackendSet.nCmd_206_Interval > 0 && ABS(tmNow - pSlave->tmLastSent_206) >= MAX(3, pSlave->blobBackendSet.nCmd_206_Interval))
	// {
	// 	pSlave->tmLastSent_206 = tmNow;
	// 	pSlave->stMsgInfoList[C_206_FeeRateKwh].pfnMsgHandler((void*)pSlave);
	// }
}

static DWORD _ThreadEntry_SlaveUDPComm(void* pArg)
{
	REPORTER* pReporter = (REPORTER*)pArg;
	ROUGH_SLAVE_ROUGH* pSlave = (ROUGH_SLAVE_ROUGH*)pReporter->pDataHeap;
	BYTE byBuff[TCP_BUFF_LEN];
	int nReadBytes = 0;
	memset(byBuff, 0, TCP_BUFF_LEN);

	while(!pReporter->bExit)
	{
		//block wait receive 6330-UCTRL
		nReadBytes = s_pUDPCommClient->Read(s_hUDPPortClient, byBuff, TCP_BUFF_LEN);
		TRACE("_ThreadEntry_SlaveUDPComm Read...nReadBytes=[%d]\n",nReadBytes);

		if(nReadBytes >= NLMSG_MIN_LEN)
		{
			//now try to find the frame and parse it
			BYTE* pFoundPos = byBuff;
			do
			{
				BYTE *pDataPos;
				USHORT usDataLen;
				NLMSG_FRAME_INFO* pFrm = Find_NLMsgFrame(pSlave, pFoundPos, nReadBytes, &pDataPos, &usDataLen);
				if(pFrm == NULL)
				{
					TRACE("---Get Frm Is NULL---\n");
					break;
				}
				else if(/*(pFrm->byCmd == pSlave->nlFrameInfo[NL_MSG_CFGG].byCmd)||
						(pFrm->byCmd == pSlave->nlFrameInfo[NL_MSG_CFGS].byCmd)||
						(pFrm->byCmd == pSlave->nlFrameInfo[NL_MSG_USYS].byCmd)||
						(pFrm->byCmd == pSlave->nlFrameInfo[NL_MSG_RTIF].byCmd)||*/
						(pFrm->byCmd == pSlave->nlFrameInfo[NL_MSG_UCTL].byCmd))
				{
					TRACE("slave Do UDP Parsing byCmd=[%02X]\n",pFrm->byCmd);
					//TRACE("NL_MSG_CFGG[%d] NL_MSG_CFGS[%d] NL_MSG_UCTL[%d] NL_MSG_RTIF[%d] NL_MSG_USYS[%d] ",pSlave->nlFrameInfo[NL_MSG_CFGG].byCmd,pSlave->nlFrameInfo[NL_MSG_CFGS].byCmd,pSlave->nlFrameInfo[NL_MSG_UCTL].byCmd,pSlave->nlFrameInfo[NL_MSG_RTIF].byCmd,pSlave->nlFrameInfo[NL_MSG_USYS].byCmd);
					pFrm->pfnParser(pSlave, pDataPos, usDataLen, PROTOCOL_UDP);
					if (pSlave->nSemWithCCUAndSlave != -1)
					{
						// static time_t tmDebugOutSentSem = 0;
						// if (ElapseS(tmDebugOutSentSem) > 30)
						// {
						// 	tmDebugOutSentSem = _NOW_;
						//	TRACEI("============================slaveReporter sent sem to ccusampler time [%0.3f]============================\n",tmNow);
						// }
						sem_post_op(pSlave->nSemWithCCUAndSlave);//send sem to ccusampler
					}
				}
				else
				{
					TRACE("Info: Maybe Other Msg byCmd=[%02X]\n", pFrm->byCmd);
				}
			
				nReadBytes -= usDataLen + NLMSG_MIN_LEN;
				pFoundPos += usDataLen + NLMSG_MIN_LEN;
			} while (nReadBytes >= NLMSG_MIN_LEN);
		}
	}
}

static DWORD _ThreadEntry_SlaveTCPComm(void* pArg)
{
	REPORTER* pReporter = (REPORTER*)pArg;
	ROUGH_SLAVE_ROUGH* pSlave = (ROUGH_SLAVE_ROUGH*)pReporter->pDataHeap;

	time_t tmNow = _NOW_;
	pSlave->tmLastRecv = tmNow;

	while(!pReporter->bExit)
	{
		TRACEW(">>> slave_ems_main:_ThreadEntry_SlaveTCPComm >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
			
		RunThread_Heartbeat(pReporter->hThread);

		JudgeisConnect(pSlave);

		if (!TryConnect2Server(pSlave))
		{
			Sleep(3000);
			continue;
		}
		if(Mutex_Lock(pSlave->hMutex, 100) == ERR_MUTEX_OK)
		{
			Recv_Tcp_StationEms_Msg(pSlave);
			Mutex_Unlock(pSlave->hMutex);
		}

		if (pSlave->bHadRecvCFGS == TRUE)//start sent LSYS periodly After has registered with SEMS;
		{
			tmNow = _NOW_;
			//周期上报SEMS_LSYS信息；
			if(tmNow - pSlave->tmLast_Sent_LSYS >= (pSlave->Sent_SEMS_LSYS_Period / 1000))
			{
				if(Mutex_Lock(pSlave->hMutex, 100) == ERR_MUTEX_OK)
				{
					Send_SEMS_LSYS(pSlave,PROTOCOL_TCP);
					pSlave->tmLast_Sent_LSYS = tmNow;
					Mutex_Unlock(pSlave->hMutex);
				}
				else
				{
					TRACEE("slave_ems_main:_ThreadEntry_SlaveTCPComm Mutex_Lock failed");
				}
			}
			
			// LJH0625 默认先不上报到主控，数据量大的会影响到正常业务
			if (pSlave->lcCfg.bSendCmdByLC1stOnly)
			{
				Pro_SysCmdDataInfo(pSlave);
			}
		}
		Sleep(200);
	}

	//do clean job
	//THREAD_CANCEL_ALL means must system will reboot
	return THREAD_CANCEL_ALL;
}

void DebugTest(ROUGH_SLAVE_ROUGH* pSlave)
{
	REPORTER* pReporter = pSlave->pReporter;
	ERRCODE_DAI nErrCode;
	VAR_VALUE varV;
	// 6301 	并离网自动切换模式       		setting 218
	varV.emValue = 0;//(stRev6301->GridOnOffAutoSwitchMode == 0xA0) ? 0 : 1;
	nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,  //0-auto   1-manual
		200, 218, &varV, 4, 3000);   
	if (nErrCode == ERRCODE_DAI_OK)
	{
		TRACEI("pDAI->Set  DAITYPE_SETT_PARAMVALUE_RW  GridOnOffAutoSwitchMode successfully !!");
	}
	else
	{
		TRACEE("pDAI->Set  DAITYPE_SETT_PARAMVALUE_RW  GridOnOffAutoSwitchMode Fail errcode[%d]!!",nErrCode);
	}
	
	// 6302 	并离网模式						setting 219
	varV.emValue = 0;//(stRev6302->GridOnOffMode == 0xA1) ? 0 : 1;
	nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
		200, 219, &varV, 4, 3000);   
	if (nErrCode == ERRCODE_DAI_OK)
	{
		TRACEI("pDAI->Set  DAITYPE_SETT_PARAMVALUE_RW  GridOnOffMode successfully !!");
	}
	else
	{
		TRACEE("pDAI->Set  DAITYPE_SETT_PARAMVALUE_RW  GridOnOffMode Fail errcode[%d]!!",nErrCode);
	}

	// 6314  	手动控制充放电功率				CTRL 214
	A_BLOB_MANUALCTRL_RECT_CMD blobRev6314;
	ZERO_POBJS(&blobRev6314, 1);
	blobRev6314.emBlobType = BLOB_TYPE_MANUALCTRL_RECT;
	blobRev6314.emState = (STATE_MANUL_CTRL)0x01;//(STATE_MANUL_CTRL)stRev6314->emACDC_WorkStatu;   //枚举修改 pending 
	blobRev6314.nGrp = 0x00;//液冷一体柜  默认All Group； 
	blobRev6314.fGrpVolt = 35.4f;//stRev6314->fVolt;
	blobRev6314.fGrpCurr = 24.5f;//stRev6314->fCurrent;
	blobRev6314.nStopSOC = 80;//(UINT)stRev6314->nStopSoc;
	blobRev6314.nAdjMode = 0x01;//(UINT)stRev6314->nEnergySchedulingMode;
	blobRev6314.fActv_Pwr = 87.6f;//stRev6314->fActv_Pwr;
	blobRev6314.fReactv_Pwr = 23.4f; //(float)stRev6314->usReActv_Pwr;

	varV.blobValue = (BYTE *)&blobRev6314;
	nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 
			200, 214, &varV, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), 3000);
	if (nErrCode == ERRCODE_DAI_OK)
	{
		TRACEI("pDAI->Set A_BLOB_MANUALCTRL_RECT_CMD Set DAITYPE_CTRL_PARAMVALUE_RW Sucessfully\n");
	}
	else
	{
		TRACEE("pDAI->Set A_BLOB_MANUALCTRL_RECT_CMD Set DAITYPE_CTRL_PARAMVALUE_RW Fail err[%d]!\n",nErrCode);
	}

	// 6317	设置自动、手动模式				setting 217
	varV.emValue = 1;//(stRev6317->ManualMode == 0xA0) ? 0 : 1;
	nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
		200, 217, &varV, 4, 3000);   
	if (nErrCode == ERRCODE_DAI_OK)
	{
		TRACEI("pDAI->Set  DAITYPE_SETT_PARAMVALUE_RW  ManualMode successfully !!");
	}
	else
	{
		TRACEE("pDAI->Set  DAITYPE_SETT_PARAMVALUE_RW  ManualMode Fail errcode[%d]!!",nErrCode);
	}

	// 6330	自动控制充放电功率

	// 6390	启用/禁用系统					setting 198
	varV.emValue = 0;//(stRev6390->DisEnableSYS == 0x01) ? 0 : 1;
	nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
		200, 198, &varV, 4, 3000);   
	if (nErrCode == ERRCODE_DAI_OK)
	{
		TRACEI("pDAI->Set  DAITYPE_SETT_PARAMVALUE_RW  DisEnableSYS successfully !!");
	}
	else
	{
		TRACEE("pDAI->Set  DAITYPE_SETT_PARAMVALUE_RW  DisEnableSYS Fail errcode[%d]!!",nErrCode);
	}
}

static BOOL Init_GetSysAlarmDataInfo(ROUGH_SLAVE_ROUGH* pSlave)
{
	REPORTER* pReporter = (REPORTER*)pSlave->pReporter;
	// COMMPORT* pComm = pReporter->pUsedCommPort;
	ERRCODE_DAI err;
	BOOL bRet = TRUE;
	int n = 0;
	//now get alarm info
 	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARM_TOTAL_R, 0, 0, &pSlave->nAlarmInfosTotal, 4);
	TRACE("Info:nTotalCfgvAlmInfo = %d \n", pSlave->nAlarmInfosTotal);
	if(err != ERRCODE_DAI_OK)
	{
		TRACE("Error: Get Cfg Alarm Total, err[%d]!", err);
		return FALSE; //do not reply
	}
	if((pSlave->nAlarmInfosTotal < 0) || (pSlave->nAlarmInfosTotal >65535))
	{
		TRACE("Error: Get AlarmInfoTotla=[%d] Out Of Range!", pSlave->nAlarmInfosTotal);
		return FALSE;
	}

	TRACE("Info:---Now get alarm total->%d---\n", pSlave->nAlarmInfosTotal);
	pSlave->ShieldAlarms = NEW(UINT, pSlave->nAlarmInfosTotal);
	if( pSlave->ShieldAlarms == NULL)
    {
		TRACE("Error: Allocate to AlarmInfo Failed!\n");
        return FALSE;
    }
	ZERO_POBJS(pSlave->ShieldAlarms, pSlave->nAlarmInfosTotal);
    pSlave->pAllAlarmInfo = NEW(ALARM_INFO, (UINT)pSlave->nAlarmInfosTotal);
    if( pSlave->pAllAlarmInfo == NULL)
    {
		TRACE("Error: Allocate to AlarmInfo Failed!\n");
        return FALSE;
    }
	
	ALARM_INFO alarmInfo;
	for(n = 0; n < pSlave->nAlarmInfosTotal; n++)
	{
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARM_INFO_R,
			n, 0, &alarmInfo, sizeof(ALARM_INFO));
		if(err != ERRCODE_DAI_OK)
		{
			TRACE("Error:Get Alarm info, Idx[%d], err[%d]!\n", n, err);
			return FALSE; 
		} 
		memcpy(&pSlave->pAllAlarmInfo[n], &alarmInfo, sizeof(ALARM_INFO));	
	}
	
	return bRet;
}

DLLExport BOOL Initialize(HANDLE pReporterThis)
{
	TRACEI("---------------Initialize SLAVE EMS REPORTER Start !!!--------------- \n");
	int n = 0;
	REPORTER* pReporter = (REPORTER*)pReporterThis;
	pReporter->pDataHeap = (void*)NEW(ROUGH_SLAVE_ROUGH, 1);

	if(pReporter->pDataHeap == NULL)
	{
		return FALSE;
	}

	ROUGH_SLAVE_ROUGH* pSlave = (ROUGH_SLAVE_ROUGH*)pReporter->pDataHeap;
	ZERO_POBJS(pSlave, 1);

	pSlave->pReporter = pReporterThis;

	NLMSG_FRAME_INFO *pp = &pSlave->nlFrameInfo[0];
	INIT_NL_MSGINFO(pp, NL_MSG_CFGG,	(NLMSG_FRAME_PARSER)Handler_SEMS_CFGG,			0x61,	0);
	INIT_NL_MSGINFO(pp, NL_MSG_CFGS,	(NLMSG_FRAME_PARSER)Handler_SEMS_CFGS,			0x62,	2);
	INIT_NL_MSGINFO(pp, NL_MSG_UCTL,	(NLMSG_FRAME_PARSER)Handler_SEMS_UCTL,			0x63,	2);
	INIT_NL_MSGINFO(pp, NL_MSG_RTIF,	(NLMSG_FRAME_PARSER)Handler_SEMS_RTIF,			0x64,	18);
	INIT_NL_MSGINFO(pp, NL_MSG_USYS,	(NLMSG_FRAME_PARSER)Parse_SEMS_USYS,			0xA6,	0);
	INIT_NL_MSGINFO(pp, NL_MSG_PCSINFO,	(NLMSG_FRAME_PARSER)Handler_PCS_INFO,			0x65,	0);

	PARAM_VAL_GET valGet;
	valGet.nGetSize = 4;
	int nErrCode;
	nErrCode = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 84, &valGet, sizeof(PARAM_VAL_GET));
	if (nErrCode != ERRCODE_DAI_OK)
	{
		TRACE("ERROR: Sent_SEMS_LSYS_Period can not be retrieved\n");
		return FALSE;
	}
	pSlave->Sent_SEMS_LSYS_Period = valGet.val.emValue;			//SEMS_LSYS 上报周期

	nErrCode =pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		500, 502, &valGet, sizeof(PARAM_VAL_GET));
	if (nErrCode != ERRCODE_DAI_OK)
	{
		TRACE("ERROR: ES-SYSTEM-type can not be retrieved\n");
		return FALSE;
	}
	pSlave->nCfgBMSNum = valGet.val.emValue;						//nCfgBMSNum
	TRACEI("nCfgBMSNum: [%d]", pSlave->nCfgBMSNum);

	valGet.nGetSize = 255;
 	valGet.val.blobValue = (BYTE*)pSlave->szStationEMSUrl;		//Station EMS URL
	nErrCode = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 53, &valGet, sizeof(PARAM_VAL_GET));
	if (nErrCode != ERRCODE_DAI_OK)
	{
		TRACEE("ERROR: Station EMS URL can not be retrieved\n");
		return FALSE;
	}
	TRACEI("Station EMS URL: [%s]", pSlave->szStationEMSUrl);

	valGet.nGetSize = sizeof(A_BLOB_NETWORKINFO);
	valGet.val.blobValue = (BYTE*)&pSlave->blobNetWorkInfo;		//从机IP地址
	nErrCode = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 3, &valGet, sizeof(PARAM_VAL_GET));
	if (nErrCode != ERRCODE_DAI_OK)
	{
		TRACEE("ERROR: A_BLOB_NETWORKINFO can not be retrieved\n");
		return FALSE;
	}

	nErrCode = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_APP_VRESION_R,0, 0, pSlave->szIMMU2_SWVer, MAX_LEN_MU2SWVER);//IMMU2_SWVer
	if (nErrCode != ERRCODE_DAI_OK)
	{
		TRACEE("ERROR: szIMMU2_SWVer can not be retrieved\n");
		return FALSE;
	}
	TRACEI("szIMMU2_SWVer = [%s]",pSlave->szIMMU2_SWVer);

	valGet.nGetSize = sizeof(A_BLOB_ES_BACKEND_CFG);
	valGet.val.blobValue = (BYTE*)&pSlave->blobBackendSet;
	nErrCode = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 26, &valGet, sizeof(PARAM_VAL_GET));
	if (nErrCode != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: Backend settings Get Failed!\n");
		return FALSE;
	}

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, GETTYPE_ST_VAR, 1, 101, &valGet, sizeof(PARAM_VAL_GET));
	pSlave->emDevType = valGet.val.emValue;

	if (LIQUID_COOLING_BACKUP == pSlave->emDevType)
		pSlave->iBattCellNumPerGroup = 192;
	else 
		pSlave->iBattCellNumPerGroup = 240;
	

	if(!Init_GetSysAlarmDataInfo(pSlave))
	{
		TRACEX("ERROR: Get Sys Alarm DataInfo Failed!\n");
		return FALSE;
	}

	//--------------Get Shield Alarms id----------------//
	valGet.nGetSize = 512;
	valGet.val.blobValue = (BYTE*)pSlave->szShieldAlarms;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 83, &valGet, sizeof(PARAM_VAL_GET));


	pSlave->lcCfg = GetLCCfg_Reporter(pReporter); // LJH250428 cmd
	//--------------------------------End-------------------------//

	pSlave->hMutex = Mutex_Create(TRUE);
	if(pSlave->hMutex == NULL)
	{
		return FALSE;
	}
	pReporter->bExit = FALSE;
	pSlave->nisFirstBoot = TRUE;

	pSlave->bNeedRefreshActvAlm = TRUE;
	//DebugTest(pSlave);

	//init ccu and slave reporter sem
	pSlave->nSemWithCCUAndSlave = Creat_SemID(SEM_CCU_485_PATH, SEM_CCU_485_PROJ_ID, 1, IPC_CREAT | 0666);
	TRACE("slave reporter Init semid = [%d]\n",pSlave->nSemWithCCUAndSlave);
	LogOut(MAIN_MODULE_MAIN, "SLAVE_UDP_REPORTER", LOG_TYPE_INFO,
		"SLAVE_UDP_REPORTER semid Init --> [%d] !\n",pSlave->nSemWithCCUAndSlave);
	if (pSlave->nSemWithCCUAndSlave != -1)
	{
		Init_Sem(pSlave->nSemWithCCUAndSlave, 0, SETVAL, 0);
	}

	if (!ReOpen_QuicklyCommPort_UDP_CLIENT(pReporter))
	{
		TRACEE("Slave Initialize ReOpen_QuicklyCommPort_UDP_CLIENT failed");
		return FALSE;
	}
	pReporter->hThread = RunThread_Create("SlaveTCPComm",
		(RUN_THREAD_START_PROC)_ThreadEntry_SlaveTCPComm,
		(void *)(pReporter),
		(DWORD*)&pReporter->bExit);
	if(pReporter->hThread == NULL)
	{
		LogOut(MAIN_MODULE_REPORTER, "SlaveTCPComm", LOG_TYPE_ERROR, 
			"Init SlaveTCPComm FAILED!!!");	
		return FALSE;
	}

	//----- Create A Thread To reveive the UDP messages from SEMS -----//
	struct sched_param param;
	pthread_attr_t attr;
	pthread_t hThread;

	pthread_attr_init(&attr);
	param.sched_priority = 1;	//数值越大，优先级越高。
	pthread_attr_setschedpolicy(&attr,SCHED_RR);	//实时调度策略
	pthread_attr_setschedparam(&attr,&param);	//设置优先级
	pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);	//要使优先级其作用必须要有这句话

	if(pthread_create(&hThread, &attr, _ThreadEntry_SlaveUDPComm,
		(void *)pReporter) != 0) //0-Success
	{
		TRACE("----- Create _ThreadEntry_SlaveUDPComm Failed -----\n");
		LogOut(MAIN_MODULE_REPORTER, "SlaveUDPComm", LOG_TYPE_ERROR, 
			"Init _ThreadEntry_SlaveUDPComm FAILED!!!");
		pthread_attr_destroy(&attr);
		return FALSE;
	}

	//Init ev handler
	EVHAND_TAGS tags[] =
	{
		{ EVENT_TYPE_ALARM,				EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_RESTORE},
		{ EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN },
		{ EVENT_TYPE_FIRMWARE_UPDATE,	EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
		{ EVENT_TYPE_DIAGNOSTIC_UPLOAD,EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
		{ EVENT_TYPE_ABILITY_CHANGE,	EVENT_MSG_BEGIN | EVENT_MSG_END }, //do not treat this, use status change event instead
		{ EVENT_TYPE_SYSTEM_EXIT,		EVENT_MSG_END},
		//{EVENT_TYPE_METERVAL_UPDATE_IN_CHARGING,	EVENT_MSG_BEGIN},
	};

	for (n = 0; n < ITEM_OF(tags); n++)
	{
		pReporter->pDAI->Regsiter_EvHandler("SLAVE_CommEventHand",
			pReporterThis,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}

	return TRUE;
}



