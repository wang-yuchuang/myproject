#include "udp_public_main.h"
 

UINT Make_UniqID_UDP(void)
{
	#define UDP_UNIQID_START	25689566

	//UINT nUniqID = tmNow * 100 + (UINT)diffMsec;
	static UINT s_nOldUniqID = 0;
	if(s_nOldUniqID == 0)
	{
		s_nOldUniqID = (UINT)_NOW_ + UDP_UNIQID_START; //just to mix the value
	}
	UINT nUniqID = s_nOldUniqID;

	s_nOldUniqID++;
	if(s_nOldUniqID == 0xFFFFFFFF)
	{
		s_nOldUniqID = UDP_UNIQID_START;
	}

	TRACE("---The uniqID = %lu----\n", nUniqID);

	return nUniqID;
}

char* Make_UdpTimeString(STU_UDP_ROUGHDATA* pUdp, time_t tmInout, char* pszOut)
{
	struct tm currenttime;

	//tmInout = _NOW_; // test purpose  nned to be Comment in Final Version 
	
	gmtime_r(&tmInout,&currenttime);
	currenttime.tm_year = 2000 + (currenttime.tm_year - 100);
	char timeinstring[32] = {'\0'};

	snprintf(timeinstring, 32, "%4d-%02d-%02d %02d:%02d:%02d",currenttime.tm_year, currenttime.tm_mon + 1, 
		currenttime.tm_mday, currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec);
	
	memcpy(pszOut, timeinstring, 32);
	return pszOut;
}

char* Make_IPV42String(STU_UDP_ROUGHDATA* pUdp, ULONG ulIpV4_IP, char* pszOut)
{
	memcpy(pszOut, inet_ntoa(*((struct in_addr*)(&ulIpV4_IP))), 32);

	return pszOut;
}

/*********************************************************************************
*  
*  FUNCTION NAME : UdpPublic Initialize()
*  CREATOR       : John
*  CREATED DATE  : 2024-02-25 17:13:53
*  DESCRIPTION   : UPD Public
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL UdpPublic_Initialize(HANDLE hSamplerSelf)
{
	TRACE("--- UpdPublic_Initialize ---\n");
	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
	STU_UDP_ROUGHDATA* pUdp = (STU_UDP_ROUGHDATA*)pSampler->pDataHeap;
	VAR_VALUE 			varV;

	UINT n = 0;
	for(n = 0; n < ESSTYPE_MAX; n++)
	{
		pUdp->tmLastSuccessExchange[n] 	= 0;
		pUdp->fKWh_Pos[n] 				= 0.0;
		pUdp->bRegisted[n]				= FALSE;	
	}
	
	pUdp->tm_LastSyncEmsMgmt = 0; //Need Sync At First Time.
	pUdp->tm_LastRecvEmsMgmt = 0;

	//Get EMS Plan.
  	ZERO_POBJS(&pUdp->emsPlan, 1);
 	varV.blobValue = (BYTE *)&pUdp->emsPlan;
	pUdp->pDPR->Get(pUdp->hSampler, GETTYPE_ST_VAR, 
		200, 201, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN));

	return TRUE; 
}

BOOL isIPAddrMatchSysTp(STU_UDP_ROUGHDATA* pUdp)
{	
	VAR_VALUE varV;
	int a1, a2, a3, a4;
	char szIP[32]={0};
	int res=sscanf(Make_IPV42String(pUdp, pUdp->blobNetWorkInfo.netInfo_eht0.ulIpV4_IP, szIP),"%d.%d.%d.%d",&a1,&a2,&a3,&a4);
	static int arrIP[ESSTYPE_MAX] =   {100, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109};

	
	TRACE("isIPAddrMatchSysTp:res=[%d] %d | %d | %d | %d emESSystemType=[%d]\n", \
		res, a1, a2, a3, a4, pUdp->emESSystemType);

	if(res == 4)
	{
		if(a4 != arrIP[pUdp->emESSystemType])
		{
			varV.emValue = 1;
			Push_VarValue(SETTYPE_LOGIC_SP, pUdp->hSampler, 1, 46, varV);
			TRACE("Warning: SysId is Not Match eth0 IP!!!\n");
			return FALSE;
		}
		else
		{
			varV.emValue = 0;
			Push_VarValue(SETTYPE_LOGIC_SP, pUdp->hSampler, 1, 46, varV);
		}
	}
	else
	{
		varV.emValue = 1;
		Push_VarValue(SETTYPE_LOGIC_SP, pUdp->hSampler, 1, 46, varV);
	}

	return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Event_Handler()
*  CREATOR       : John
*  CREATED DATE  : 2024-02-25 17:13:53
*  DESCRIPTION   : UPD Public
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE		emEvType,
	_EVENT_MSG		emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int				nMaxBlockMSec)
{	
	SAMPLER* pSampler = (SAMPLER*)hModuleSeif;
	STU_UDP_ROUGHDATA* pUdp = (STU_UDP_ROUGHDATA*)pSampler->pDataHeap;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;
	BYTE bySendBuff[TCP_BUFF_LEN];
	memset(bySendBuff, 0, TCP_BUFF_LEN);
	int nDataSize = 0, nDataLen = 0;

	TRACE("Event_Handler:emEvType=%d emMsgType=%d nEvDataSize=%d\n",\
		emEvType,emMsgType,nEvDataSize);
	
	//Mutex_Lock(pUdp->hSamplerLock, WAIT_INFINITE); //Added Lock

	if(emEvType == EVENT_TYPE_CFG_CHANGED)
	{
		if(nEvDataSize != sizeof(EVENT_PARAM_CHG))
		{
			return EVENT_HANDED_FAILED;
		}

		EVENT_PARAM_CHG *pCfgChange = (EVENT_PARAM_CHG*)pEvData;
		if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 3))
		{
			TRACE("Network config Changed, Do Update!\n");
 			varV.blobValue = (BYTE *)&pUdp->blobNetWorkInfo;
			ERRCDDE_DPR err = pUdp->pDPR->Get(pSampler, GETTYPE_ST_VAR,
				1, 3, &varV, sizeof(A_BLOB_NETWORKINFO));
			if(err != ERR_DPR_OK)
			{
				TRACE("!!! Network Changed Failed !!!\n");
				return EVENT_HANDED_FAILED;
			}
			TRACE("!!! Network Changed success !!!\n");
		}
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 8))
		{
			ESSYS_TYPE  emESSysTp = GetESSysType(pUdp->hSampler);
			if(emESSysTp != pUdp->emESSystemType)
			{
				if((emESSysTp <= ESSTYPE_UDP_SERVER) ||
					((pUdp->emESSystemType == ESSTYPE_UDP_SERVER) && (emESSysTp > ESSTYPE_UDP_SERVER)))
				{
					LogOut(MAIN_MODULE_SAMPLER, "Udp", LOG_TYPE_ERROR, 
						"SysTp=[%d] Changed to [%d]", pUdp->emESSystemType, emESSysTp);
					//gracefully reboot system
					EVENT_SYSTEM_EXIT evExit;
					evExit.nExitType = SYS_INTERNAL_ERROR;
					SendMsg(pUdp->hSampler, EVENT_TYPE_SYSTEM_EXIT, EVENT_MSG_BEGIN, 
						&evExit, sizeof(EVENT_SYSTEM_EXIT));
				}
				else
				{
					pUdp->emESSystemType = emESSysTp;
				}
				isIPAddrMatchSysTp(pUdp);
			}
		}
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 27))
		{
			ERRCDDE_DPR err = pUdp->pDPR->Get(pUdp->hSampler, GETTYPE_ST_VAR, 1, 27, &varV, 4);
			if(varV.ulValue != pUdp->nConfigSlvNumb) //Reset All Slv Node CommStatus.
			{
				UINT n = 0, m = 0;
				for(n = ESSTYPE_UDP_CLIENT1; n < ESSTYPE_MAX; n++)
				{
 					varV.emValue = 0; //Comm-OK
					Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, 801 + m, 431, varV);
					A_BLOB_METER_VALUES blobMeter;
					ZERO_POBJS(&blobMeter, 1);
					
					Push_BlobValue(SETTYPE_RAW_SP, pUdp->hSampler, 
						801 + m, 430, (void*)&blobMeter, sizeof(A_BLOB_METER_VALUES), BLOB_TYPE_METERVALUES);
			

					m++;
				}
			}
			pUdp->nConfigSlvNumb = varV.ulValue;
			TRACE("pUdp->nConfigSlvNumb =[%d]\n", pUdp->nConfigSlvNumb );
		}
		else if((pCfgChange->parmPTR.nId == 200) && (pCfgChange->parmPTR.nSubId == 201))//EMS Mgmt Plan
		{
			pUdp->tm_LastSyncEmsMgmt = 0;
			
			VAR_VALUE 			varV;
			varV.blobValue = (BYTE *)&pUdp->emsPlan;
			pUdp->pDPR->Get(pUdp->hSampler, GETTYPE_ST_VAR,
				200, 201, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN));
		}
	}
	//Mutex_Unlock(pUdp->hSamplerLock);
	return EVENT_HANDED_OK;
}
 
/*********************************************************************************
*  
*  FUNCTION NAME : Initialize()
*  CREATOR       : John
*  CREATED DATE  : 2024-02-25 17:13:53
*  DESCRIPTION   : UPD Public
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 
DLLExport BOOL Initialize(HANDLE hSamplerSelf)
{	
	TRACE("Initialize Udp Public Sampler !!!\n");
	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
	pSampler->pDataHeap = (void*)NEW(STU_UDP_ROUGHDATA, 1);
	if(pSampler->pDataHeap == NULL)
	{
		return FALSE;
	}
	
	STU_UDP_ROUGHDATA* pUdp = (STU_UDP_ROUGHDATA*)pSampler->pDataHeap;
	ZERO_POBJS(pUdp, 1);

	pUdp->hSamplerLock = pSampler->hSamplerLock; //Init lock
	pUdp->hSampler = hSamplerSelf;
	pUdp->pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	
	VAR_VALUE varV;
	pUdp->pDPR->Get(hSamplerSelf, GETTYPE_ST_VAR, 1, 8, &varV, 4);
	pUdp->emESSystemType = (ESSYS_TYPE )varV.emValue;  //??? Need Refresh
	
	TRACE("UDP Main: emESSystemType=[%d] emValue=[%d]\n", \
		pUdp->emESSystemType, varV.emValue);

	//Initialize network info. 
	//now get the saved APN
	varV.blobValue = (BYTE *)&pUdp->blobNetWorkInfo;
	ERRCDDE_DPR err = pUdp->pDPR->Get(hSamplerSelf, GETTYPE_ST_VAR,
			1, 3, &varV, sizeof(A_BLOB_NETWORKINFO));

 	err = pUdp->pDPR->Get(hSamplerSelf, GETTYPE_ST_VAR,
			1, 27, &varV, 4);
	pUdp->nConfigSlvNumb = varV.ulValue;

	BOOL bRet = TRUE;
	if(pUdp->emESSystemType == ESSTYPE_UNINSTALL)
	{
		return TRUE;
	}
	else if(pUdp->emESSystemType == ESSTYPE_UDP_SERVER)
	{
		bRet = UdpServer_Initialize( hSamplerSelf );
	}
	else
	{
		bRet = UdpClient_Initialize( hSamplerSelf );
	}

	UdpPublic_Initialize( hSamplerSelf );

	isIPAddrMatchSysTp(pUdp);

	EVHAND_TAGS tags[] = 
	{
		{EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN},
		{EVENT_TYPE_SAVE_ES_STATDATA,	EVENT_MSG_STATUS_CHANGED},
	};

	int n;
	for(n = 0; n < ITEM_OF(tags); n++)
	{
		pUdp->pDPR->Register_EvHandler("Udp_EvHandler",
			hSamplerSelf,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}

	return bRet;
}
 
/*********************************************************************************
*  
*  FUNCTION NAME : Sample()
*  CREATOR       : John
*  CREATED DATE  : 2024-02-25 17:13:53
*  DESCRIPTION   : UPD Public
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 
DLLExport int Sample(HANDLE hSamplerThis)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	STU_UDP_ROUGHDATA* pUdp = (STU_UDP_ROUGHDATA*)pSampler->pDataHeap;
	ERRCODE_SAMPLER err = ERR_SAMPLE_OK;

	if(pUdp->emESSystemType == ESSTYPE_UNINSTALL)
	{
		Sleep_Ex(1000);
	}
	else if(pUdp->emESSystemType == ESSTYPE_UDP_SERVER)
	{
		err = UdpServer_Sample( hSamplerThis );
	}
	else
	{
		err = UdpClient_Sample( hSamplerThis );
	}

	Sleep_Ex(100);

	TRACE("--- Udp Public tm=[%ld] ---\n", _NOW_);

	return err;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Control()
*  CREATOR       : John
*  CREATED DATE  : 2024-02-25 17:13:53
*  DESCRIPTION   : UPD Public
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 
DLLExport int Control(HANDLE hSamplerThis, PARAM_TYPE	emParaType,
	int			nDevId,
	int			nParamId,
	VAR_VALUE	*pVarVal,
	int			nValSize,
	int			nTimeOut)
{
	TRACE("emParaType =%d nDevId=%d nParamId=%d nValSize=%d \n",\
		emParaType, nDevId,nParamId,nValSize);

	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	STU_UDP_ROUGHDATA* pUdp = (STU_UDP_ROUGHDATA *)pSampler->pDataHeap;
	//BYTE byDataBuff[TCP_BUFF_LEN];
	//memset(byDataBuff, 0, TCP_BUFF_LEN);
	
	VAR_VALUE 	varV;
	int nRet = ERR_CONTROL_OK;
	int n = 0, nDataSize = 0;
 	
	if(Mutex_Lock(pUdp->hSamplerLock, nTimeOut) != ERR_MUTEX_OK)
	{
		return ERR_CONTROL_TIMEOUT;
	}

	if(emParaType == PARAM_TYPE_SETTING)
	{
		TRACE("emParaType =%d\n",emParaType);
	}
	else //Control
	{

	} 
	Mutex_Unlock(pUdp->hSamplerLock);

	return ERR_CONTROL_OK;
	
}

/*********************************************************************************
*  
*  FUNCTION NAME : JigTest()
*  CREATOR       : John
*  CREATED DATE  : 2024-02-25 17:13:53
*  DESCRIPTION   : UPD Public
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/ 
DLLExport int JigTest(HANDLE hSamplerThis, void* pJigData)
{
	return 0;
}

	

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