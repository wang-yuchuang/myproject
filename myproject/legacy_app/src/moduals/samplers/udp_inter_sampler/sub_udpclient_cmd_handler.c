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
BOOL Client_SendData(STU_UDP_ROUGHDATA *pUdp, 
                    cJSON* pJson,
                    UDP_CLIENT_CMDTP emMsgType, 
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
		pUdp->stMsgList_Client[emMsgType].nMsgId,
		nMsgUniqID,
		szData);

	free(szData);

	pUdp->nSendBuffLen = strlen(pUdp->szSendBuffer);

	TRACE("EVERYMSG == %s  Length = %d \n", pUdp->szSendBuffer, pUdp->nSendBuffLen);

	if(pSampler->pUsedCommPort->Write(pSampler->hCommPort, (char*)pUdp->szSendBuffer, pUdp->nSendBuffLen) != pUdp->nSendBuffLen)
	{
		TRACE("Failed: UpdClient: WriteData Failed nSendLen=[%d]!!!\n", pUdp->nSendBuffLen);
		return FALSE;
	}

	TRACE("\n_________Now Succeffully write msg to UdpUdp_WriteData len[%d]_________\n", pUdp->nSendBuffLen);
	
    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Prepare C10X_MeterValue()
*  CREATOR       : John
*  CREATED DATE  : 2024-03-02 11:13:53
*  DESCRIPTION   : MeterValue Content should Include "IP Addr".
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int PrepareClient_S10X_MeterValue(void* pArg)
{
	STU_UDP_ROUGHDATA* pUdp = (STU_UDP_ROUGHDATA*)pArg;
	SAMPLER* pSampler = (SAMPLER*)pUdp->hSampler;
 	VAR_VALUE 				varV;
	ERRCDDE_DPR 			err;

	TRACE("--- PrepareClient_S10X_MeterValue --- emESSystemType=[%d]\n", pUdp->emESSystemType);

    cJSON *pRoot = cJSON_CreateObject();
	cJSON* pAllACMeter_Array = cJSON_CreateArray();

	ZERO_POBJS(&pUdp->blobMeter, 	1);
	varV.blobValue = (BYTE *)&pUdp->blobMeter;

	int nDevID = 790;
	err = pUdp->pDPR->Get(pUdp->hSampler, GETTYPE_SP_RAW_VAL, 
		nDevID, 430, &varV, sizeof(pUdp->blobMeter));
	if(err != ERR_DPR_OK)
	{	
		TRACE("Error: DAI Get Dev=%d, blob=430 Failed err=[%d]!\n", nDevID, err);
		return -1;
	}

	cJSON *pCabMeterObj = cJSON_CreateObject();

	char szIP[32]={0};
	//cJSON_AddStringToObject(pRoot, 		"TM", 			Make_UdpTimeString(pUdp, time(NULL), szTime));
	
	cJSON_AddStringToObject(pRoot, 		"IP", 				Make_IPV42String(pUdp, pUdp->blobNetWorkInfo.netInfo_eht0.ulIpV4_IP, szIP));
	cJSON_AddNumberToObject(pRoot, 		"TM", 				time(NULL));
	cJSON_AddNumberToObject(pRoot, 		"SysTp", 			pUdp->emESSystemType);  
	
	cJSON_AddNumberToObject(pCabMeterObj, 	"DevId", 		nDevID);
	cJSON_AddFloatToObject(pCabMeterObj, 	"VPhA", 		pUdp->blobMeter.fPhA_Volt,  			1);
	cJSON_AddFloatToObject(pCabMeterObj, 	"VPhB", 		pUdp->blobMeter.fPhB_Volt,  			1);
	cJSON_AddFloatToObject(pCabMeterObj, 	"VPhC", 		pUdp->blobMeter.fPhC_Volt,  			1);
	cJSON_AddFloatToObject(pCabMeterObj, 	"APhA", 		pUdp->blobMeter.fPhA_Curr,  			1);
	cJSON_AddFloatToObject(pCabMeterObj, 	"APhB", 		pUdp->blobMeter.fPhB_Curr,  			1);
	cJSON_AddFloatToObject(pCabMeterObj, 	"APhC", 		pUdp->blobMeter.fPhC_Curr,  			1);
 	cJSON_AddFloatToObject(pCabMeterObj, 	"TotkW",		pUdp->blobMeter.fRealTime_ActivePower,  1);
	cJSON_AddFloatToObject(pCabMeterObj,	"kWPhA",		pUdp->blobMeter.fPhA_ActivePower,  		1);
	cJSON_AddFloatToObject(pCabMeterObj,	"kWPhB",		pUdp->blobMeter.fPhB_ActivePower,  		1);
	cJSON_AddFloatToObject(pCabMeterObj,	"kWPhC",		pUdp->blobMeter.fPhC_ActivePower,  		1);
 	cJSON_AddFloatToObject(pCabMeterObj, 	"TotkVar",		pUdp->blobMeter.fReactivePower,  		1);
	cJSON_AddFloatToObject(pCabMeterObj,	"kVarPhA",		pUdp->blobMeter.fPhA_ReactivePower,  	1);
	cJSON_AddFloatToObject(pCabMeterObj,	"kVarPhB",		pUdp->blobMeter.fPhB_ReactivePower,  	1);
	cJSON_AddFloatToObject(pCabMeterObj,	"kVarPhC",		pUdp->blobMeter.fPhC_ReactivePower,  	1);
 	cJSON_AddFloatToObject(pCabMeterObj,	"kWDemand",		pUdp->blobMeter.fPmaxDemand,  			1);

 	cJSON_AddFloatToObject(pCabMeterObj, 	"TotPF"	 ,		pUdp->blobMeter.fPower_Factor,			2);
	cJSON_AddFloatToObject(pCabMeterObj,	"kVarPhA",		pUdp->blobMeter.fPhA_PowerFactor,  		2);
	cJSON_AddFloatToObject(pCabMeterObj,	"kVarPhB",		pUdp->blobMeter.fPhB_PowerFactor,  		2);
	cJSON_AddFloatToObject(pCabMeterObj,	"kVarPhC",		pUdp->blobMeter.fPhC_PowerFactor,  		2);
  	cJSON_AddFloatToObject(pCabMeterObj,	"TotHz",		pUdp->blobMeter.fAC_frequency,  		2);
  	cJSON_AddFloatToObject(pCabMeterObj,	"kWhPos",		pUdp->blobMeter.fRealTime_kWh,  		1);
 	cJSON_AddFloatToObject(pCabMeterObj,	"kWhNeg",		pUdp->blobMeter.fReverse_kWh,  			1);
 	cJSON_AddFloatToObject(pCabMeterObj,	"kVar1",		pUdp->blobMeter.fRealTime_kVarh1,  		1);
	cJSON_AddFloatToObject(pCabMeterObj,	"kVar2",		pUdp->blobMeter.fRealTime_kVarh2,  		1);

	cJSON_AddItemToArray(pAllACMeter_Array, pCabMeterObj);

	//----------------Grid AC MeterValue ---------------//
	//cJSON *pGridMeterObj = cJSON_CreateObject();

	//blobMeter.fPhA_Volt = 420;
	//blobMeter.fPhB_Volt = 421;
	//blobMeter.fPhC_Volt = 422;

	//nDevID = 791;
	//cJSON_AddNumberToObject(pGridMeterObj, 	"DevId", 		nDevID);  
	//cJSON_AddFloatToObject(pGridMeterObj, 	"VPhA", 		blobMeter.fPhA_Volt,  				1);
	//cJSON_AddFloatToObject(pGridMeterObj, 	"VPhB", 		blobMeter.fPhB_Volt,  				1);
	//cJSON_AddFloatToObject(pGridMeterObj, 	"VPhC", 		blobMeter.fPhC_Volt,  				1);
	//cJSON_AddItemToArray(pAllACMeter_Array, pGridMeterObj);

	cJSON_AddItemToObject(pRoot, "all_meter", pAllACMeter_Array);
 
	Client_SendData(pUdp, pRoot, S_10X_METERVALUE_CLIENT, TRUE);

	return 0;
}


void UdpClient_StuffMeterValue(STU_UDP_ROUGHDATA* pUdp, UINT nDevId, A_BLOB_METER_VALUES *pBlobMeter)
{	
	UINT 		nMapDevId = 0;
	VAR_VALUE 	varV;

	TRACE("UdpClient_StuffMeterValue: nDevId=[%d] fRealTime_ActivePower=[%.1f]\n", \
		nDevId, pBlobMeter->fRealTime_ActivePower);

	if(nDevId == DEVID_BLOBMETER_GRID)
	{
		nMapDevId = DEVID_SIGLMETER_GRID;
 	}
	else if(nDevId == DEVID_BLOBMETER_AUX1)
	{
		nMapDevId = DEVID_SIGLMETER_AUX1;
	}
	else if(nDevId == DEVID_BLOBMETER_AUX3)
	{
		nMapDevId = DEVID_SIGLMETER_AUX3;
	}
	else 
	{
		return;
	}

	if(nMapDevId == DEVID_SIGLMETER_GRID)
	{
		pUdp->fPgrid_Sum = pBlobMeter->fRealTime_ActivePower;
	}
	varV.fValue = pBlobMeter->fRealTime_ActivePower;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 400, varV);

	varV.fValue = pBlobMeter->fPhA_ActivePower;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 401, varV);

	varV.fValue = pBlobMeter->fPhB_ActivePower;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 402, varV);

	varV.fValue = pBlobMeter->fPhC_ActivePower;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 403, varV);

	varV.fValue = pBlobMeter->fReactivePower;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 404, varV);

	varV.fValue = pBlobMeter->fPhA_ReactivePower;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 405, varV);

	varV.fValue = pBlobMeter->fPhB_ReactivePower;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 406, varV);

	varV.fValue = pBlobMeter->fPhC_ReactivePower;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 407, varV);

	varV.fValue = pBlobMeter->fRealTime_kWh;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 411, varV);

	varV.fValue = pBlobMeter->fRealTime_kVarh1;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 412, varV);

	varV.fValue = pBlobMeter->fPhA_Volt;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 413, varV);

	varV.fValue = pBlobMeter->fPhA_Curr;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 414, varV);

	varV.fValue = pBlobMeter->fPhB_Volt;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 415, varV);

	varV.fValue = pBlobMeter->fPhB_Curr;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 416, varV);

	varV.fValue = pBlobMeter->fPhC_Volt;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 417, varV);

	varV.fValue = pBlobMeter->fPhC_Curr;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 418, varV);

	varV.fValue = pBlobMeter->fRealTime_kVarh2;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 419, varV);

 	varV.fValue = pBlobMeter->fReverse_kWh;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 422, varV);

 	varV.fValue = pBlobMeter->fAC_frequency;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 424, varV);

	varV.fValue = pBlobMeter->fPmaxDemand;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 425, varV);

	varV.fValue = pBlobMeter->fPower_Factor;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 426, varV);

	varV.fValue = pBlobMeter->fPhA_PowerFactor;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 427, varV);

	varV.fValue = pBlobMeter->fPhB_PowerFactor;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 428, varV);

	varV.fValue = pBlobMeter->fPhC_PowerFactor;
	Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, nMapDevId, 429, varV);

	return;
}



static void Handle_UpdSyncTime(STU_UDP_ROUGHDATA *pUdp, time_t tm)
{
	SAMPLER* 		pSampler = (SAMPLER*)pUdp->hSampler;
	static time_t	tm_Last;
	VAR_VALUE		varV;

	if(ABS(tm - _NOW_) > 30) //sync RTC
	{
		TRACE("Udp Client: tm=[%ld] _NOW_=[%ld]\n", tm, _NOW_);
	
		varV.dtValue = tm;
		if(ABS(_NOW_ - tm_Last) > 60) // Avoid Sync Time Too Frequently
		{
			tm_Last = _NOW_;
			Push_VarValue(SETTYPE_INTERN_SETT, pUdp->hSampler, 1, 1, varV);
		}
	}
	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Handle S100_MeterValue()
*  CREATOR       : John
*  CREATED DATE  : 2024-03-02 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int HandleClient_R100_MeterValue(void* pArg)
{
	STU_UDP_ROUGHDATA* 	pUdp = (STU_UDP_ROUGHDATA*)pArg;
	SAMPLER* 			pSampler = (SAMPLER*)pUdp->hSampler;
	time_t 				tmRefresh;
	ESSYS_TYPE			emDataFrom;
	BYTE 				szIP[32];
	cJSON* 				pJsCurrItem = NULL;
	cJSON* 				pItemName = NULL;

	//TRACEX("--- HandleClient_R100_MeterValue ---\n");

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
	}else{
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

			Handle_UpdSyncTime(pUdp, tmRefresh);
		}
	}else{
		TRACE("Error: TM is NULL !!!\n");
	}
	
	if (temp1 = cJSON_GetObjectItem(pJson, "SysTp"))
	{
		if(temp1->type == cJSON_Number)
		{
			TRACE("SysTp=[%d]\n", temp1->valueint);
			emDataFrom = (ESSYS_TYPE)temp1->valueint;
		}
	}else{
		TRACE("Error: SysTp is NULL !!!\n");
	}

	//Priority
	if(temp1 = cJSON_GetObjectItem(pJson, "Prio"))
	{
		int nSize = cJSON_GetArraySize(temp1);
		if(nSize > 1)
		{
			int i = 0;
			for(i = 0; i < nSize; i++)
			{
				pJsCurrItem = cJSON_GetArrayItem(temp1, i);
				if(pJsCurrItem->type == cJSON_Number)
				{
					UINT nPrio = (UINT)pJsCurrItem->valuedouble;
					TRACE("Warning: i=[%d] Value=[%d]\n", i , nPrio);
					if(i == pUdp->emESSystemType)
					{
						VAR_VALUE varV;
						varV.ulValue = nPrio;
						Push_VarValue(SETTYPE_RAW_SP, pUdp->hSampler, 1, 47, varV);
					}
				}
			}
		}else{
			TRACE("PrioArry nSize=[%d]\n", nSize);
		}
	}else{
		TRACE("Error: Prio is NULL!!!\n");
	}

	//-------- Begin Parse all_meter group --------//
	int nItemTotal = 0;
	if (temp1 = cJSON_GetObjectItem(pJson, "all_meter"))
	{
		nItemTotal = cJSON_GetArraySize(temp1);
		if(nItemTotal < 1)
		{
			TRACE("Warning: nItemTotal =%d\n", nItemTotal);
			//Send_CallError(pOCPP, S_504_RmtSetEMSPlanProfile, CallError_FormatError, "ems_plan_cfg Sector Number is INVALID!");
			return -1;
		}
	}

	int n = 0; 
	UINT nDevId = 0;
	for(n = 0; n < nItemTotal; n++)
	{
		A_BLOB_METER_VALUES 	blobMeter;
		ZERO_POBJS(&blobMeter, 	1);
		blobMeter.tmRefresh = tmRefresh;
		blobMeter.emDataFrom = emDataFrom;
		memcpy(blobMeter.szIP, 	szIP , 	32);

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
				TRACE("n=[%d] kVarPhA=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhA_PowerFactor = (float)pItemName->valuedouble;
			}
		}
	 
	 	if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "PfPhB"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] kVarPhB=[%f]\n", n , pItemName->valuedouble);
				blobMeter.fPhB_PowerFactor = (float)pItemName->valuedouble;
			}
		}

	 	if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "PfPhC"))
		{
			if(pItemName->type == cJSON_Number)
			{
				TRACE("n=[%d] kVarPhC=[%f]\n", n , pItemName->valuedouble);
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

		UdpClient_StuffMeterValue(pUdp, nDevId, &blobMeter);

		Push_BlobValue(SETTYPE_RAW_SP, pUdp->hSampler, 
			nDevId, 430, (void*)&blobMeter, sizeof(A_BLOB_METER_VALUES), BLOB_TYPE_METERVALUES);
	}

	return 0;
}




static BOOL Compare_IfNeedUpdate_EMSParam(STU_UDP_ROUGHDATA *pUdp, A_BLOB_EMS_CONFIG_PLAN *pEmsPlan)
{
    BOOL 	bRet = FALSE;
    int 	n = 0;

    if(pUdp->emsPlan.nTotal_EmsLmt_Sectors != pEmsPlan->nTotal_EmsLmt_Sectors)
    {
        TRACE("Old.nTotal_EmsLmt_Sectors=[%d] New.nTotal_EmsLmt_Sectors=[%d]\n", \
			pUdp->emsPlan.nTotal_EmsLmt_Sectors , pEmsPlan->nTotal_EmsLmt_Sectors);
        return TRUE;
    }

	if(pUdp->emsPlan.nEMSMgmt_Ver != pEmsPlan->nEMSMgmt_Ver)
	{
		TRACE("Old.nEMSMgmt_Ver=[%d] New.nEMSMgmt_Ver=[%d]\n", \
			pUdp->emsPlan.nEMSMgmt_Ver, pEmsPlan->nEMSMgmt_Ver);
		return TRUE;
	}

    for(n = 0; n < pUdp->emsPlan.nTotal_EmsLmt_Sectors; n++)
    {
        if(pUdp->emsPlan.emsLmtSectors[n].tmStart != pEmsPlan->emsLmtSectors[n].tmStart)
        {
            TRACE("pEmsPlan->tmStart=[%ld] emsPlan.tmStart=[%ld]\n", \
                pUdp->emsPlan.emsLmtSectors[n].tmStart , pEmsPlan->emsLmtSectors[n].tmStart);

            bRet = TRUE;
            break;
        }
 
        if(pUdp->emsPlan.emsLmtSectors[n].nStopSOC != pEmsPlan->emsLmtSectors[n].nStopSOC)
        {
            TRACE("pEmsPlan->nStopSOC=[%d] emsPlan.nStopSOC=[%d]\n", \
                pUdp->emsPlan.emsLmtSectors[n].nStopSOC , pEmsPlan->emsLmtSectors[n].nStopSOC);

            bRet = TRUE;
            break;
        }

        if(pUdp->emsPlan.emsLmtSectors[n].nBattGrpPwrLmt != pEmsPlan->emsLmtSectors[n].nBattGrpPwrLmt)
        {
            TRACE("pEmsPlan->nBattGrpPwrLmt=[%d] emsPlan.nBattGrpPwrLmt=[%d]\n", \
                pUdp->emsPlan.emsLmtSectors[n].nBattGrpPwrLmt , pEmsPlan->emsLmtSectors[n].nBattGrpPwrLmt);

            bRet = TRUE;
            break;
        }

        if(pUdp->emsPlan.emsLmtSectors[n].nActvPwrLmt != pEmsPlan->emsLmtSectors[n].nActvPwrLmt)
        {
            bRet = TRUE;
            break;
        }

        if(pUdp->emsPlan.emsLmtSectors[n].nReActvPwrLmt != pEmsPlan->emsLmtSectors[n].nReActvPwrLmt)
        {
            bRet = TRUE;
            break;
        } 

    }
    return bRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Handle Client_R400_EmsMgmt()
*  CREATOR       : John
*  CREATED DATE  : 2024-03-29 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int HandleClient_R400_EmsMgmt(void* pArg)
{
	TRACE("--- HandleClient_R400_EmsMgmt ---\n");
	STU_UDP_ROUGHDATA* 	pUdp = (STU_UDP_ROUGHDATA*)pArg;
	SAMPLER* 			pSampler = (SAMPLER*)pUdp->hSampler;
	time_t 				tmNow = _NOW_;

	if(ABS(tmNow - pUdp->tm_LastRecvEmsMgmt) < 5)
	{
		TRACE("Warning: Client Recv EmgMgmt Too Frequently. _NOW_=[%ld] tm_LastRecvEmsMgmt=[%ld]!!!\n",\
			tmNow, pUdp->tm_LastRecvEmsMgmt);
		return 0;
	}
	pUdp->tm_LastRecvEmsMgmt = tmNow;

	cJSON 				*pJson 		= pUdp->pCurrJson;
	cJSON 				*temp1 		= NULL;
	int 				nItemTotal 	= 0;
	BOOL 				bAccepted	= FALSE;
	VAR_VALUE 			varV;

 	A_BLOB_EMS_CONFIG_PLAN emsPlan;
 	ZERO_POBJS(&emsPlan, 1);
 
	if(temp1 = cJSON_GetObjectItem(pJson, "Ver"))
	{
		TRACE("ems_plan_ver =%d temp1->valueint=%d\n", emsPlan.nEMSMgmt_Ver, temp1->valueint);
		if(temp1->valueint != emsPlan.nEMSMgmt_Ver)
		{
			emsPlan.nEMSMgmt_Ver = (UINT)temp1->valuedouble;
		}
		else{
			return -1;
		}
	}else{
		return -1;
	}
	
	if (temp1 = cJSON_GetObjectItem(pJson, "Item"))
	{
		nItemTotal = cJSON_GetArraySize(temp1);
		if((nItemTotal < 1) || (nItemTotal > MAX_EMSMGMT_LMT_SETOR))
		{
			TRACE("nItemTotal =%d\n", nItemTotal);
			return -1;
		}

		emsPlan.nTotal_EmsLmt_Sectors = nItemTotal;

		TRACEX("Handle_S504_SetEMSPlanProfile nItemTotal= %d \n", nItemTotal);
		
		int n = 0;
		cJSON* pJsCurrItem = NULL;
		cJSON* pItemName = NULL;
		bAccepted = TRUE;
 		for(n = 0; n < nItemTotal; n++)
		{
			pJsCurrItem = cJSON_GetArrayItem(temp1, n);
			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "TM_Start"))
			{
				if(cJSON_Number == pItemName->type)
				{
					emsPlan.emsLmtSectors[n].tmStart = (time_t)pItemName->valuedouble;
				}else
				{
					bAccepted = FALSE;
					break;
				}
			}else{
				bAccepted = FALSE;
				break;
			}
			
			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "Interval"))
			{
				if(cJSON_Number != pItemName->type)
				{
					bAccepted = FALSE;
					break;
				}else{
					emsPlan.emsLmtSectors[n].nInterval =  pItemName->valueint;
				}
			}else{
				bAccepted = FALSE;
				break;
			}

			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "Mode"))
			{
				if(cJSON_Number != pItemName->type)
				{
					bAccepted = FALSE;
					break;
				}else{
					emsPlan.emsLmtSectors[n].emBatt_WorkMode =  pItemName->valueint;
				}
			}else{
				bAccepted = FALSE;
				break;
			}

			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "Stop_soc"))
			{
				if((pItemName->valueint < 0) || (pItemName->valueint > 100) || (cJSON_Number != pItemName->type))
				{
					bAccepted = FALSE;
					break;
				}else{
					emsPlan.emsLmtSectors[n].nStopSOC = pItemName->valueint;
				}
			}else{
				bAccepted = FALSE;
				break;
			}

			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "PsumLmt"))
 			{
				if((pItemName->valueint < 0) || (pItemName->valueint > 999999.0) || (cJSON_Number != pItemName->type))
				{
					bAccepted = FALSE;
					break;
				}else{
					emsPlan.emsLmtSectors[n].nActvPwrLmt = (UINT)(pItemName->valueint);
				}
			}else{
				bAccepted = FALSE;
				break;
			}

			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "QsumLmt"))
 			{
				if((pItemName->valueint < -999999) || (pItemName->valueint > 999999) || (cJSON_Number != pItemName->type))
				{
					bAccepted = FALSE;
					break;
				}else{
					emsPlan.emsLmtSectors[n].nReActvPwrLmt = (int)(pItemName->valueint);
				}
			}else{
				bAccepted = FALSE;
				break;
			}

			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "PbatLmt"))
 			{
				if((pItemName->valueint < 0) || (pItemName->valueint > 999999.0) || (cJSON_Number != pItemName->type))
				{
					bAccepted = FALSE;
					break;
				}else{
					emsPlan.emsLmtSectors[n].nBattGrpPwrLmt = (UINT)(pItemName->valueint);
				}
			}else{
				bAccepted = FALSE;
				break;
			}
	
			TRACE("Item[%d] start_time=%d interval=%d batt_Priworkmode=%d \
				nActvPwrLmt=%d nReActvPwrLmt=%d nStopSOC=%d emFeeRate=%d\n", \
				n,emsPlan.emsLmtSectors[n].tmStart, \
				emsPlan.emsLmtSectors[n].nInterval, \ 
				emsPlan.emsLmtSectors[n].emBatt_WorkMode,\
				emsPlan.emsLmtSectors[n].nActvPwrLmt, \
				emsPlan.emsLmtSectors[n].nReActvPwrLmt, \
				emsPlan.emsLmtSectors[n].nStopSOC);
		}
	}	

	if(!bAccepted)
	{
		printf("Error: EMS Mgmt!!!\n");
		return -1;
	}

	if(Compare_IfNeedUpdate_EMSParam(pUdp, &emsPlan))
	{
		Push_BlobValue(SETTYPE_RAW_ST, pUdp->hSampler, 200, 201, 
			(void*)&emsPlan, sizeof(A_BLOB_EMS_CONFIG_PLAN), BLOB_TYPE_EMS_CONFIG_PLAN);
		memcpy(&pUdp->emsPlan, &emsPlan, sizeof(A_BLOB_EMS_CONFIG_PLAN));
	}
	return 0;
}