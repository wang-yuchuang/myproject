#include "st_batt_sampler.h"

//#define TRACE	 printf("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), printf
//#define TRACEX	 printf("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), printf

//gcc -o multi_usb_write multi_usb_write.c -lpthread -lm

/*********************************************************************************
*  
*  FUNCTION NAME : Refresh_Relevant ItemData
*  CREATOR       :  
*  CREATED DATE  : 2022-05-15 21:07
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Refresh_RelevantItemData(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx, A_RELEVANT_HEALTH_ELEM *pRelvtElmt)
{
	A_BLOB_SINGLE_STBATT_RUNINFO*	pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

	pRelvtElmt->tmTimeStamp 			= _NOW_;

	pRelvtElmt->fMax_CellVolt 			= pSingleRunInfo->fMax_CellVolt;
    pRelvtElmt->fMin_CellVolt 			= pSingleRunInfo->fMin_CellVolt;
	pRelvtElmt->fAvg_CellVolt 			= pSingleRunInfo->fAvg_CellVolt;

	pRelvtElmt->uiBAT_Ucell_Max_CSC   	= pSingleRunInfo->uiBAT_Ucell_Max_CSC;
	pRelvtElmt->uiBAT_Ucell_Max_CELL  	= pSingleRunInfo->uiBAT_Ucell_Max_CELL;
	pRelvtElmt->uiBAT_Ucell_Min_CSC   	= pSingleRunInfo->uiBAT_Ucell_Min_CSC;
	pRelvtElmt->uiBAT_Ucell_Min_CELL  	= pSingleRunInfo->uiBAT_Ucell_Min_CELL;

	pRelvtElmt->fMax_CellTemp 			= pSingleRunInfo->fMax_CellTemp;
	pRelvtElmt->fMin_CellTemp 			= pSingleRunInfo->fMin_CellTemp;
	pRelvtElmt->fAvg_CellTemp 			= pSingleRunInfo->fAvg_CellTemp;

	pRelvtElmt->uiBAT_T_Max_CSC  		= pSingleRunInfo->nBAT_T_Max_CSC;
	pRelvtElmt->uiBAT_T_Max_CELL 		= pSingleRunInfo->uiBAT_T_Max_CELL;
	pRelvtElmt->uiBAT_T_Min_CSC  		= pSingleRunInfo->nBAT_T_Min_CSC;
	pRelvtElmt->uiBAT_T_Min_CELL 		= pSingleRunInfo->uiBAT_T_Min_CELL;

	pRelvtElmt->fChg_Start_MaxCellTemp 	= pSingleRunInfo->fChg_Start_MaxCellTemp;
	pRelvtElmt->fChg_Start_MinCellTemp 	=  pSingleRunInfo->fChg_Start_MinCellTemp;

	pRelvtElmt->fDischg_Start_MaxCellTemp = pSingleRunInfo->fDischg_Start_MaxCellTemp;
	pRelvtElmt->fDischg_Start_MinCellTemp = pSingleRunInfo->fDischg_Start_MinCellTemp;

	pRelvtElmt->fPackTotalCurr 			= pSingleRunInfo->fPackTotalCurr;

	pRelvtElmt->bySOC 					= (BYTE)pSingleRunInfo->nSOC;


	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Refresh_Health StaticList
*  CREATOR       :  
*  CREATED DATE  : 2022-05-15 21:07
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Refresh_HealthStaticList(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx)
{
	A_BLOB_SINGLE_STBATT_RUNINFO*	pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];
	HIS_HEALTH_STATIC_DAILY* 		pRec = &pSTBatt_Rough->hisHealthRec;
	A_BMS_HEALTH_ITEM* 				pItem = &pRec->bmsHealthData.healthItem[nBattIdx];
	
	pRec->bmsHealthData.nInstalledNum = pSTBatt_Rough->nSystemCfgBattNum;

	float fCurrt_Chg_TmsvDif_CellVolt = pSingleRunInfo->fMax_CellVolt - pSingleRunInfo->fMin_CellVolt;
	float fCurrt_Chg_TmsvDif_CellTemp = pSingleRunInfo->fMax_CellTemp - pSingleRunInfo->fMin_CellTemp;

	float fCurrt_Dischg_TmsvDif_CellVolt = pSingleRunInfo->fMax_CellVolt - pSingleRunInfo->fMin_CellVolt;
	float fCurrt_Dischg_TmsvDif_CellTemp = pSingleRunInfo->fMax_CellTemp - pSingleRunInfo->fMin_CellTemp;

	//TRACEX("Status=[%d] Chg_TmsvDif_CellVolt=[%.1f] TmsvDif_CellTemp=[%.1f] TmsvDif_CellVolt=[%.1f] TmsvDif_CellTemp=[%.1f]\n",\
	//	pSingleRunInfo->emBattStatus_Run,\
	//	fCurrt_Chg_TmsvDif_CellVolt,\
	//	fCurrt_Chg_TmsvDif_CellTemp,\
	//	fCurrt_Dischg_TmsvDif_CellVolt,\
	//	fCurrt_Dischg_TmsvDif_CellTemp);

	switch (pSingleRunInfo->emBattStatus_Run)
	{
		case STBATT_RUNSTAT_SLEEP:
		case STBATT_RUNSTAT_IDLE:
		{
			//--- 			Charging 			---//
			if(FLOAT_EQUAL0(pItem->fKey_Chg_TmsvDif_CellVolt))
			{
				pItem->fKey_Chg_TmsvDif_CellVolt = fCurrt_Chg_TmsvDif_CellVolt;
				Refresh_RelevantItemData(pSTBatt_Rough, nBattIdx, &pItem->chgTmsvDifCellVolt);
			}

			if(FLOAT_EQUAL0(pItem->fKey_Chg_TmsvDif_CellTemp))
			{
				pItem->fKey_Chg_TmsvDif_CellTemp = fCurrt_Chg_TmsvDif_CellTemp;
				Refresh_RelevantItemData(pSTBatt_Rough, nBattIdx, &pItem->chgTmsvDifCellTemp);
			}

			if(FLOAT_EQUAL0(pItem->fKey_Chg_Max_CellTemp))
			{
				pItem->fKey_Chg_Max_CellTemp = pSingleRunInfo->fMax_CellTemp;
				Refresh_RelevantItemData(pSTBatt_Rough, nBattIdx, &pItem->chgMaxCellTemp);
			}

			if(FLOAT_EQUAL0(pItem->fKey_Chg_Min_CellTemp))
			{
				pItem->fKey_Chg_Min_CellTemp = pSingleRunInfo->fMin_CellTemp;
				Refresh_RelevantItemData(pSTBatt_Rough, nBattIdx, &pItem->chgMinCellTemp);
			}

			//-----			Dischrging			-----//
			if(FLOAT_EQUAL0(pItem->fKey_Dischg_TmsvDif_CellVolt))
			{
				pItem->fKey_Dischg_TmsvDif_CellVolt = fCurrt_Dischg_TmsvDif_CellVolt;
				Refresh_RelevantItemData(pSTBatt_Rough, nBattIdx, &pItem->dischgTmsvDifCellVolt);
			}

			if(FLOAT_EQUAL0(pItem->fKey_Dischg_TmsvDif_CellTemp))
			{
				pItem->fKey_Dischg_TmsvDif_CellTemp = fCurrt_Dischg_TmsvDif_CellTemp;
				Refresh_RelevantItemData(pSTBatt_Rough, nBattIdx, &pItem->dischgTmsvDifCellTemp);
			}

			if(FLOAT_EQUAL0(pItem->fKey_Dischg_Max_CellTemp))
			{
				pItem->fKey_Dischg_Max_CellTemp = pSingleRunInfo->fMax_CellTemp;
				Refresh_RelevantItemData(pSTBatt_Rough, nBattIdx, &pItem->dischgMaxCellTemp);
			}

			if(FLOAT_EQUAL0(pItem->fKey_Dischg_Min_CellTemp))
			{
				pItem->fKey_Dischg_Min_CellTemp = pSingleRunInfo->fMin_CellTemp;
				Refresh_RelevantItemData(pSTBatt_Rough, nBattIdx, &pItem->dischgMinCellTemp);
			}

			break;
		}

		case STBATT_RUNSTAT_CHARGE:
		case STBATT_RUNSTAT_FULL_CHARGE:
		{
			if(fCurrt_Chg_TmsvDif_CellVolt > pItem->fKey_Chg_TmsvDif_CellVolt)
			{
				pItem->fKey_Chg_TmsvDif_CellVolt = fCurrt_Chg_TmsvDif_CellVolt;
				Refresh_RelevantItemData(pSTBatt_Rough, nBattIdx, &pItem->chgTmsvDifCellVolt);
			}

			if(fCurrt_Chg_TmsvDif_CellTemp > pItem->fKey_Chg_TmsvDif_CellTemp)
			{
				pItem->fKey_Chg_TmsvDif_CellTemp = fCurrt_Chg_TmsvDif_CellTemp;
				Refresh_RelevantItemData(pSTBatt_Rough, nBattIdx, &pItem->chgTmsvDifCellTemp);
			}

			if(pSingleRunInfo->fMax_CellTemp > pItem->fKey_Chg_Max_CellTemp)
			{
				pItem->fKey_Chg_Max_CellTemp = pSingleRunInfo->fMax_CellTemp;
				Refresh_RelevantItemData(pSTBatt_Rough, nBattIdx, &pItem->chgMaxCellTemp);
			}

			if(pSingleRunInfo->fMin_CellTemp < pItem->fKey_Chg_Min_CellTemp)
			{
				pItem->fKey_Chg_Min_CellTemp = pSingleRunInfo->fMin_CellTemp;
				Refresh_RelevantItemData(pSTBatt_Rough, nBattIdx, &pItem->chgMinCellTemp);
			}
			break;
		}

		case STBATT_RUNSTAT_DISCHARGE:
		case STBATT_RUNSTAT_FULL_DISCHARGE:
			{
				if(fCurrt_Dischg_TmsvDif_CellVolt > pItem->fKey_Dischg_TmsvDif_CellVolt)
				{
					pItem->fKey_Dischg_TmsvDif_CellVolt = fCurrt_Dischg_TmsvDif_CellVolt;
					Refresh_RelevantItemData(pSTBatt_Rough, nBattIdx, &pItem->dischgTmsvDifCellVolt);
				}

				if(fCurrt_Dischg_TmsvDif_CellTemp > pItem->fKey_Dischg_TmsvDif_CellTemp)
				{
					pItem->fKey_Dischg_TmsvDif_CellTemp = fCurrt_Dischg_TmsvDif_CellTemp;
					Refresh_RelevantItemData(pSTBatt_Rough, nBattIdx, &pItem->dischgTmsvDifCellTemp);
				}

				if(pSingleRunInfo->fMax_CellTemp > pItem->fKey_Dischg_Max_CellTemp)
				{
					pItem->fKey_Dischg_Max_CellTemp = pSingleRunInfo->fMax_CellTemp;
					Refresh_RelevantItemData(pSTBatt_Rough, nBattIdx, &pItem->dischgMaxCellTemp);
				}

				if(pSingleRunInfo->fMin_CellTemp < pItem->fKey_Dischg_Min_CellTemp)
				{
					pItem->fKey_Dischg_Min_CellTemp = pSingleRunInfo->fMin_CellTemp;
					Refresh_RelevantItemData(pSTBatt_Rough, nBattIdx, &pItem->dischgMinCellTemp);
				}
				break;
			}

		default:
			break;
	}

	pItem->fRack_TotalCapacity = pSingleRunInfo->fTotal_RateCapacity;


	/*TRACEX("Idx=[%d]\
		fKey_Chg_TmsvDif_CellVolt=[%.1f] fKey_Chg_TmsvDif_CellTemp=[%.1f]\
		fKey_Chg_Max_CellTemp=[%.1f] fKey_Chg_Min_CellTemp=[%.1f]\
		fKey_Dischg_TmsvDif_CellVolt=[%.1f] fKey_Dischg_TmsvDif_CellTemp=[%.1f]\
		fKey_Dischg_Max_CellTemp=[%.1f] fKey_Dischg_Min_CellTemp=[%.1f]\n", \
		nBattIdx,\
		pItem->fKey_Chg_TmsvDif_CellVolt,\
		pItem->fKey_Chg_TmsvDif_CellTemp,\
		pItem->fKey_Chg_Max_CellTemp,\
		pItem->fKey_Chg_Min_CellTemp,\
		pItem->fKey_Dischg_TmsvDif_CellVolt,\
		pItem->fKey_Dischg_TmsvDif_CellTemp,\
		pItem->fKey_Dischg_Max_CellTemp,\
		pItem->fKey_Dischg_Min_CellTemp);

	TRACEX("Idx=[%d]\
		pItem->chgMaxCellTemp.fMax_CellVolt=[%.1f]\
		pItem->dischgMaxCellTemp.fMax_CellVolt=[%.1f]\n", \
		nBattIdx,\
		pItem->chgMaxCellTemp.fMax_CellVolt,\
		pItem->dischgMaxCellTemp.fMax_CellVolt);
	*/
	return;
}


/*********************************************************************************
*  
*  FUNCTION NAME : Update_BMSHealthData2DB Entran
*  CREATOR       :  
*  CREATED DATE  : 2022-05-15 21:07
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Update_BMSHealthData2DB(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	static 	BOOL bFirstRun = TRUE;

	//John, After Reboot, Wait Blob Data Restore.
	//printf("nBattIdx=[%d] Update Health\n", nBattIdx);
	if(bFirstRun == TRUE)
	{
		if(ABS(_NOW_ - pSTBatt_Rough->t_FirstBattBoot) > 60)//should wait 60S 
		{
			//---Need Wait Event_Handler Restore Health StaticData---//
			bFirstRun = FALSE;
		}
		return;
	}

	A_BLOB_SINGLE_STBATT_RUNINFO *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];
	
	//printf("nBattIdx=[%d] emBattStatus_Run=[%d]\n", nBattIdx, pSingleRunInfo->emBattStatus_Run);
	
	if(pSingleRunInfo->emBattStatus_Run <= STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}


	HIS_HEALTH_STATIC_DAILY* pRec = &pSTBatt_Rough->hisHealthRec;

	Refresh_HealthStaticList(pSTBatt_Rough, nBattIdx);

	//now sendmsg => db

	//printf("nBattIdx=[%d] _NOW_=[%ld] tmLogTime=[%ld]\n", nBattIdx, _NOW_, pRec->tmLogTime);

	if(ABS(_NOW_ - pRec->tmLogTime) > 600) 
	{
		TRACEX("---SendMsg EVENT_TYPE_HEALTHATA_UPDATE---tmLogTime=[%ld]\n",pRec->tmLogTime);	

		pRec->tmLogTime = _NOW_;

		EVENT_BMSHEALTH_UPDATE evtbmsHealthUpdt;
		evtbmsHealthUpdt.pHisBmsHealthRec =  &pSTBatt_Rough->hisHealthRec;
		SendMsg(pSTBatt_Rough->hSampler, EVENT_TYPE_HEALTHDATA_UPDATE, EVENT_MSG_STATUS_CHANGED,
			&evtbmsHealthUpdt, sizeof(EVENT_BMSHEALTH_UPDATE));
	}

	return;

} 
