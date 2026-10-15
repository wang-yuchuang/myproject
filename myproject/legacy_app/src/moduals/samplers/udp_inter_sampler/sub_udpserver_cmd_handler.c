#include "udp_public_main.h"


/*********************************************************************************
*  
*  FUNCTION NAME : Client SendData()
*  CREATOR       : John
*  CREATED DATE  : 2024-03-02 11:13:53
*  DESCRIPTION   :
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL Server_SendData(STU_UDP_ROUGHDATA *pUdp, 
                    cJSON* pJson,
                    UDP_SERVER_CMDTP emMsgType, 
                    BOOL bUrget)
{
    SAMPLER* pSampler = (SAMPLER*)pUdp->hSampler;
    char *szData = 0;
	szData = cJSON_Print(pJson);

	//TRACEX("Send_CallResult Json_Print = %s\n",szData);

	cJSON_Delete(pJson);

	UINT nMsgUniqID = Make_UniqID_UDP(); 

	memset(pUdp->szSendBuffer, 0 , UDP_MAX_SENTMSG_SIZE);
	snprintf(pUdp->szSendBuffer, UDP_MAX_SENTMSG_SIZE, "{\"udpId\":\"Q%d\", \"uniqKey\":\"%d\",	\"content\":%s}",
		pUdp->stMsgList_Server[emMsgType].nMsgId,
		nMsgUniqID,
		szData);

	free(szData);

	pUdp->nSendBuffLen = strlen(pUdp->szSendBuffer);

	TRACE("EVERYMSG == %s  Length = %d emMsgType=[%d] nMsgId=[%d]\n",\
		 pUdp->szSendBuffer, pUdp->nSendBuffLen, emMsgType, pUdp->stMsgList_Server[emMsgType].nMsgId);

	if(pSampler->pUsedCommPort->Write(pSampler->hCommPort, (char*)pUdp->szSendBuffer, pUdp->nSendBuffLen) != pUdp->nSendBuffLen)
	{
		TRACE("Failed: UpdServer: WriteData Failed nSendLen=[%d]!!!\n", pUdp->nSendBuffLen);
		return FALSE;
	}

	TRACE("\n_________Now Succeffully write msg to UdpServer_WriteData len[%d]_________\n", pUdp->nSendBuffLen);
	
    return;
}


static void Prio_AddItemToArray(STU_UDP_ROUGHDATA *pUdp, cJSON *pPrio_Array, ESSYS_TYPE emType)
{
	float 		fComp 	=  pUdp->fKWh_Pos[emType];
	float 		fMax 	= 0.0;
	ESSYS_TYPE 	emInitType = ESSTYPE_UNINSTALL;
	BOOL 		bBigger = FALSE;
	int 		nPrio = 0 ;

	for(emInitType = ESSTYPE_UNINSTALL; emInitType < ESSTYPE_MAX; emInitType++)
	{
		if(emInitType == emType)
		{
			continue;
		}

		if(fMax < pUdp->fKWh_Pos[emInitType]) 
		{
			fMax = pUdp->fKWh_Pos[emInitType];
		}
	}
	 
	nPrio = (fComp > fMax) ? 1 : 0;

	cJSON_AddItemToArray(pPrio_Array, cJSON_CreateNumber(nPrio));

	//TRACE("emType=[%d] fComp=[%f] fMax=[%.1f]\n", emType, fComp, fMax);
	return;
}
 
/*********************************************************************************
*  
*  FUNCTION NAME : PrepareServer S100_MeterValue()
*  CREATOR       : John
*  CREATED DATE  : 2024-03-02 16:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int PrepareServer_S100_MeterValue(void* pArg)
{
	STU_UDP_ROUGHDATA* pUdp = (STU_UDP_ROUGHDATA*)pArg;
	SAMPLER* pSampler = (SAMPLER*)pUdp->hSampler;
	VAR_VALUE 				varV;
	ERRCDDE_DPR 			err;

    cJSON *pRoot = cJSON_CreateObject();
	cJSON* pAllACMeter_Array = cJSON_CreateArray();

	A_BLOB_METER_VALUES 	blobMeter;
	ZERO_POBJS(&blobMeter, 	1);
	varV.blobValue = (BYTE *)&blobMeter;

	char szIP[32]={0};
	//cJSON_AddStringToObject(pRoot, 		"TM", 			Make_UdpTimeString(pUdp, time(NULL), szTime));
	cJSON_AddStringToObject(pRoot, 			"IP", 			Make_IPV42String(pUdp, pUdp->blobNetWorkInfo.netInfo_eht0.ulIpV4_IP, szIP));
	cJSON_AddNumberToObject(pRoot, 			"TM", 			time(NULL));
	cJSON_AddNumberToObject(pRoot, 			"SysTp", 		pUdp->emESSystemType);  

	cJSON* pPrio_Array = cJSON_CreateArray();
	ESSYS_TYPE emType = ESSTYPE_UNINSTALL;
	for(emType = ESSTYPE_UNINSTALL; emType < ESSTYPE_MAX; emType++)
	{
		Prio_AddItemToArray(pUdp, pPrio_Array, emType);
	}
	cJSON_AddItemToObject(pRoot, "Prio", pPrio_Array);

	UINT nDevID = DEVID_BLOBMETER_GRID;
 	for(nDevID = DEVID_BLOBMETER_GRID; nDevID <= DEVID_BLOBMETER_AUX3; nDevID++)
	{
		err = pUdp->pDPR->Get(pUdp->hSampler, GETTYPE_SP_RAW_VAL, 
			nDevID, 430, &varV, sizeof(blobMeter));
		if(err != ERR_DPR_OK)
		{	
			TRACE("Error: DAI Get Dev=DEVID_BLOBMETER_GRID, blob=430 Failed err=[%d]!\n", err);
			ZERO_POBJS(&blobMeter, 1);
		}

		cJSON *pCabMeterObj = cJSON_CreateObject();

		//blobMeter.fPhA_Volt = (float)nDevID;

		cJSON_AddNumberToObject(pCabMeterObj, 	"DevId", 		nDevID);
		cJSON_AddFloatToObject(pCabMeterObj, 	"VPhA", 		blobMeter.fPhA_Volt,  				1);
		cJSON_AddFloatToObject(pCabMeterObj, 	"VPhB", 		blobMeter.fPhB_Volt,  				1);
		cJSON_AddFloatToObject(pCabMeterObj, 	"VPhC", 		blobMeter.fPhC_Volt,  				1);

		cJSON_AddFloatToObject(pCabMeterObj, 	"APhA", 		blobMeter.fPhA_Curr,  				1);
		cJSON_AddFloatToObject(pCabMeterObj, 	"APhB", 		blobMeter.fPhB_Curr,  				1);
		cJSON_AddFloatToObject(pCabMeterObj, 	"APhC", 		blobMeter.fPhC_Curr,  				1);

		cJSON_AddFloatToObject(pCabMeterObj, 	"TotkW",		blobMeter.fRealTime_ActivePower,  	1);
		cJSON_AddFloatToObject(pCabMeterObj,	"kWPhA",		blobMeter.fPhA_ActivePower,  		1);
		cJSON_AddFloatToObject(pCabMeterObj,	"kWPhB",		blobMeter.fPhB_ActivePower,  		1);
		cJSON_AddFloatToObject(pCabMeterObj,	"kWPhC",		blobMeter.fPhC_ActivePower,  		1);

		cJSON_AddFloatToObject(pCabMeterObj, 	"TotkVar",		blobMeter.fReactivePower,  			1);
		cJSON_AddFloatToObject(pCabMeterObj,	"kVarPhA",		blobMeter.fPhA_ReactivePower,  		1);
		cJSON_AddFloatToObject(pCabMeterObj,	"kVarPhB",		blobMeter.fPhB_ReactivePower,  		1);
		cJSON_AddFloatToObject(pCabMeterObj,	"kVarPhC",		blobMeter.fPhC_ReactivePower,  		1);
		
		cJSON_AddFloatToObject(pCabMeterObj,	"kWDemand",		blobMeter.fPmaxDemand,  			1);

		cJSON_AddFloatToObject(pCabMeterObj, 	"TotPF"	 ,		blobMeter.fPower_Factor,			2);
		cJSON_AddFloatToObject(pCabMeterObj,	"kVarPhA",		blobMeter.fPhA_PowerFactor,  		2);
		cJSON_AddFloatToObject(pCabMeterObj,	"kVarPhB",		blobMeter.fPhB_PowerFactor,  		2);
		cJSON_AddFloatToObject(pCabMeterObj,	"kVarPhC",		blobMeter.fPhC_PowerFactor,  		2);

		cJSON_AddFloatToObject(pCabMeterObj,	"TotHz",		blobMeter.fAC_frequency,  			2);
		
		cJSON_AddFloatToObject(pCabMeterObj,	"kWhPos",		blobMeter.fRealTime_kWh,  			1);
		cJSON_AddFloatToObject(pCabMeterObj,	"kWhNeg",		blobMeter.fReverse_kWh,  			1);

		cJSON_AddFloatToObject(pCabMeterObj,	"kVar1",		blobMeter.fRealTime_kVarh1,  		1);
		cJSON_AddFloatToObject(pCabMeterObj,	"kVar2",		blobMeter.fRealTime_kVarh2,  		1);

		cJSON_AddItemToArray(pAllACMeter_Array, pCabMeterObj);

		if(nDevID == DEVID_BLOBMETER_GRID)
		{
			pUdp->fKWh_Pos[ESSTYPE_UDP_SERVER] = blobMeter.fRealTime_kWh;
		}
	}

	cJSON_AddItemToObject(pRoot, "all_meter", pAllACMeter_Array);
 
	Server_SendData(pUdp, pRoot, S_100_METERVALUE_SERVER, TRUE);

	TRACEX("--- PrepareServer_S100_MeterValue ---\n");

    return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : HandleServer R101_MeterValue()
*  CREATOR       : John
*  CREATED DATE  : 2024-03-02 17:13:53
*  DESCRIPTION   :  
*  {
    "content": {
        "TM": "2024-03-0614: 52: 47",
        "SysTp": 2,
        "all_meter": [
            {
				"DevId": 407,
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
            },
            {
                "DevId": 408,
                "VPhA": 0,
                "VPhB": 0,
                "VPhC": 0
            }
        ]
    }
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int HandleServer_R10X_MeterValue(void* pArg)
{
    STU_UDP_ROUGHDATA* pUdp = (STU_UDP_ROUGHDATA*)pArg;
	SAMPLER* pSampler = (SAMPLER*)pUdp->hSampler;
	static UINT arrDevId[ESSTYPE_MAX]={0, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809};
	
	int 		n = 0;
	UINT 		nMapDevId = 0;
	time_t 		tmRefresh;
	ESSYS_TYPE	emDataFrom;
	BYTE 		szIP[32]={0};
	time_t 		tmNow = _NOW_;

	TRACEX("--- HandleServer_R10X_MeterValue ---\n");

	cJSON *pJson = pUdp->pCurrJson;
	cJSON *temp1 = NULL;

	if (temp1 = cJSON_GetObjectItem(pJson, "IP"))
	{
		if(temp1->type == cJSON_String)
		{
			TRACE("IP=[%s]\n", temp1->valuestring);
			memcpy(szIP, temp1->valuestring, 32);
		}
		else if(temp1->type == cJSON_Number)
		{
			TRACE("cJSON_Number:IP=[%ld]\n", temp1->valueint);
		}
	}
	else
	{
		TRACE("Error: IP is NULL !!!\n");
	}

	if (temp1 = cJSON_GetObjectItem(pJson, "TM"))
	{
		if(temp1->type == cJSON_String)
		{
			TRACE("TM=[%s]\n", temp1->valuestring);
		}
		else if(temp1->type == cJSON_Number)
		{
			TRACE("TM=[%ld]\n", temp1->valueint);
			tmRefresh = (time_t)temp1->valueint;
		}
	}
	else
	{
		TRACE("Error: TM is NULL !!!\n");
	}
	
	if (temp1 = cJSON_GetObjectItem(pJson, "SysTp"))
	{
		if(temp1->type == cJSON_Number)
		{
			if((temp1->valueint >= ESSTYPE_MAX) || (temp1->valueint < ESSTYPE_UDP_SERVER))
			{
				return -1;
			}
			emDataFrom = (ESSYS_TYPE)temp1->valueint;
			if(emDataFrom < ESSTYPE_MAX)
			{
				if(!pUdp->bRegisted[emDataFrom])
				{
					pUdp->tm_LastSyncEmsMgmt 		= 0;
					pUdp->bRegisted[emDataFrom]		= TRUE;
					TRACE("Warning: Trigger EMS Mgmt emDataFrom=[%d] tmNow=[%ld]\n", emDataFrom, tmNow);
				}
				pUdp->tmLastSuccessExchange[emDataFrom] = tmNow;
			}
			nMapDevId = arrDevId[emDataFrom];
			TRACE("SysTp=[%d] nMapDevId=[%d]\n", temp1->valueint, nMapDevId);
		}
		else
		{
			TRACE("Error: SysTp is Not Number\n");
			return -1;
		}
	}
	else
	{
		TRACE("Error: SysTp is NULL !!!\n");
		return -1;
	}

	//-------- Begin Parse all_meter group --------//
	int nItemTotal = 0;
	if (temp1 = cJSON_GetObjectItem(pJson, "all_meter"))
	{
		nItemTotal = cJSON_GetArraySize(temp1);
		if(nItemTotal < 1)
		{
			TRACE("Warning: nItemTotal =%d\n", nItemTotal);
			//Send_CallError(pUdp, S_504_RmtSetEMSPlanProfile, CallError_FormatError, "ems_plan_cfg Sector Number is INVALID!");
			return -1;
		}
	}

 	cJSON* pJsCurrItem = NULL;
	cJSON* pItemName = NULL;
	for(n = 0; n < nItemTotal; n++)
	{
		UINT nDevId = 0;
		A_BLOB_METER_VALUES 	blobMeter;
		ZERO_POBJS(&blobMeter, 	1);

		blobMeter.emDataFrom 	= emDataFrom;
		blobMeter.tmRefresh 	= tmRefresh;
		memcpy(blobMeter.szIP, 	szIP , 	32);

		TRACE("Server Recv : blobMeter.szIP=[%s]\n", blobMeter.szIP);

		pJsCurrItem = cJSON_GetArrayItem(temp1, n);
		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "DevId"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] DevID=[%d]\n", n , pItemName->valueint);
				nDevId = pItemName->valueint;
			}
		}else{
			TRACE("Error: Not Found DevID!!!\n");
		}

		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "VPhA"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] VPhA=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhA_Volt = (float)pItemName->valuedouble;
			}
		}else{
			TRACE("Error: Not Found VPhA !!!\n");
		}

		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "VPhB"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] VPhB=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhB_Volt = (float)pItemName->valuedouble;
			}
		}else{
			TRACE("Error: Not Found VPhB !!!\n");
		}

		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "VPhC"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] VPhC=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhC_Volt = (float)pItemName->valuedouble;
			}
		}else{
			TRACE("Error: Not Found VPhC !!!\n");
		}

		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "APhA"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] APhA=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhA_Curr = (float)pItemName->valuedouble;
			}
		}

		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "APhB"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] APhB=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhB_Curr = (float)pItemName->valuedouble;
			}
		}
 
 		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "APhC"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] APhC=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhC_Curr = (float)pItemName->valuedouble;
			}
		}
 
  		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "TotkW"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] TotkW=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fRealTime_ActivePower = (float)pItemName->valuedouble;
			}
		}

		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "kWPhA"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] kWPhA=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhA_ActivePower = (float)pItemName->valuedouble;
			}
		}
	 
	 	if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "kWPhB"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] kWPhB=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhB_ActivePower = (float)pItemName->valuedouble;
			}
		}
	 
	 	if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "kWPhC"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] kWPhC=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhC_ActivePower = (float)pItemName->valuedouble;
			}
		}
 
 	 	if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "TotkVar"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] TotkVar=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fReactivePower = (float)pItemName->valuedouble;
			}
		}

		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "kVarPhA"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] kVarPhA=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhA_ReactivePower = (float)pItemName->valuedouble;
			}
		}
 
 		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "kVarPhB"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] kVarPhB=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhB_ReactivePower = (float)pItemName->valuedouble;
			}
		}

 		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "kVarPhC"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] kVarPhC=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhC_ReactivePower = (float)pItemName->valuedouble;
			}
		}

		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "kWDemand"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] kWDemand=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPmaxDemand = (float)pItemName->valuedouble;
			}
		}
		
		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "TotPF"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] TotPF=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPower_Factor = (float)pItemName->valuedouble;
			}
		}

		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "PfPhA"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] PfPhA=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhA_PowerFactor = (float)pItemName->valuedouble;
			}
		}
	 
	 	if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "PfPhB"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] PfPhB=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhB_PowerFactor = (float)pItemName->valuedouble;
			}
		}

	 	if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "PfPhC"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] PfPhC=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhC_PowerFactor = (float)pItemName->valuedouble;
			}
		}
 
 	 	if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "TotHz"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] TotHz=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fAC_frequency = (float)pItemName->valuedouble;
			}
		}

 	 	if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "kWhPos"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] kWhPos=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fRealTime_kWh = (float)pItemName->valuedouble;
			}
		}

		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "kWhNeg"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] kWhNeg=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fReverse_kWh = (float)pItemName->valuedouble;
			}
		}

		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "kVar1"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] kVar1=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fRealTime_kVarh1 = (float)pItemName->valuedouble;
			}
		}

		if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "kVar2"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] kVar2=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fRealTime_kVarh2 = (float)pItemName->valuedouble;
			}
		}

		pUdp->fKWh_Pos[emDataFrom] = blobMeter.fRealTime_kWh;

		Push_BlobValue(SETTYPE_RAW_SP, pUdp->hSampler, 
			nMapDevId, 430, (void*)&blobMeter, sizeof(A_BLOB_METER_VALUES), BLOB_TYPE_METERVALUES);
		
		break;//Only Parse One MeterValue Content Element From Client.
	}
    return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : PrepareServer S100_MeterValue()
*  CREATOR       : John
*  CREATED DATE  : 2024-03-02 16:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int PrepareServer_S400_EmsMgmt(void* pArg)
{
	TRACE("--- PrepareServer_S400_EmsMgmt ---\n");
	STU_UDP_ROUGHDATA* pUdp = (STU_UDP_ROUGHDATA*)pArg;
	SAMPLER* pSampler = (SAMPLER*)pUdp->hSampler;
	
	int 		n = 0;
	VAR_VALUE 	varV;

	//A_BLOB_EMS_CONFIG_PLAN emsPlan;
 	//ZERO_POBJS(&emsPlan, 1);
 	//varV.blobValue = (BYTE *)&emsPlan;
	//pUdp->pDPR->Get(pUdp->hSampler, GETTYPE_ST_VAR, 
	//	200, 201, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN));

	cJSON *pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "Ver", pUdp->emsPlan.nEMSMgmt_Ver);

	cJSON *pSector_Array = cJSON_CreateArray();
	for(n = 0; n < pUdp->emsPlan.nTotal_EmsLmt_Sectors; n++)
	{
		cJSON *pSector_Obj = cJSON_CreateObject();

		//char szTime[32];
		//cJSON_AddStringToObject(pSector_Obj, "start_time", Make_ESiOCPPTimeString(pUdp->emsPlan.emsLmtSectors[n].tmStart, szTime));
		cJSON_AddNumberToObject(pSector_Obj, 	"TM_Start", 		pUdp->emsPlan.emsLmtSectors[n].tmStart);
		cJSON_AddNumberToObject(pSector_Obj, 	"Interval", 		pUdp->emsPlan.emsLmtSectors[n].nInterval);
		cJSON_AddNumberToObject(pSector_Obj, 	"Stop_soc", 		pUdp->emsPlan.emsLmtSectors[n].nStopSOC);
		cJSON_AddNumberToObject(pSector_Obj, 	"PsumLmt", 			pUdp->emsPlan.emsLmtSectors[n].nActvPwrLmt);
		cJSON_AddNumberToObject(pSector_Obj, 	"QsumLmt", 			pUdp->emsPlan.emsLmtSectors[n].nReActvPwrLmt);
		cJSON_AddNumberToObject(pSector_Obj, 	"PbatLmt", 			pUdp->emsPlan.emsLmtSectors[n].nBattGrpPwrLmt);
		cJSON_AddNumberToObject(pSector_Obj, 	"Mode", 			pUdp->emsPlan.emsLmtSectors[n].emBatt_WorkMode); 		
		cJSON_AddItemToArray(pSector_Array, pSector_Obj);
	}

	cJSON_AddItemToObject(pRoot, "Item", pSector_Array);

	Server_SendData(pUdp, pRoot, S_400_EMSMGMT_SERVER, TRUE);

	return 0;
}