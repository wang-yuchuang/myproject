// #define _TRACE_PRINT_ // SEMS
#include "ccu_sampler_main.h"

// #define TRACE_PESS
//#define TRACE_AvgPwrFirst // 同充同放

#if (defined TRACE_PESS) || (defined TRACE_AvgPwrFirst)
#define TIMEOUT_LOGOUT 5
    static float s_fTotal_PessChgAvaliable = 0;
    static float s_fTotal_PessDischgLmt = 0;

    static BOOL s_bEfficientChrg = TRUE;
    static BOOL s_bEfficientDisChrg = TRUE;
    static float fRemain_ACChrgingPwr_Lmt[MAX_SINGLE_STBATT_NUM];
    static float fRemain_ACLoad_DisChrgPwr_Req[MAX_SINGLE_STBATT_NUM];
#endif

/*********************************************************************************
*  
*  FUNCTION NAME :   
*  CREATOR       :  
*  CREATED DATE  : 2022-04-13 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static  BOOL IsBatt_Plan2_StartChrg(ROUGH_CCU_DATA* pCCU, UINT nBattSOC)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
 	if(pEMS->nTMSector == -1)
	{
		return FALSE;
	}

	// LJH250902 modified
    EMS_LIMIT_SECTOR* pPlanSectorNow = &pEMS->planSectorNow;
    TRACE("----- Now sector[%d]judgeSOC->[%d], nStopSOC->[%d]  -------\n", pEMS->nTMSector, nBattSOC, 
		pPlanSectorNow->nStopSOC);
    // LJH240911 AvgPwrFirst add emBatt_WorkMode
	if((pPlanSectorNow->emBatt_WorkMode == BATT_WORKMODE_FIRST_CHRGING) &&
        (nBattSOC < pPlanSectorNow->nStopSOC))
    {
		//EMS Mgmt Will Advice wether Need Start Charge
		TRACE("Plan Start Charge!!\n");
		return TRUE;
	}
	else
	{
		TRACE("Not Need Start Charge!\n");
	}

	return FALSE;
}


static BOOL IsBatt_Plan2_StopDischarge(ROUGH_CCU_DATA* pCCU, UINT nBattSOC)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    // LJH240911 AvgPwrFirst add bEnterUrgetDischrgMode
    if(pEMS->nTMSector == -1 || pEMS->bEnterUrgetDischrgMode)
	{
		return FALSE;
	}
    // LJH250902 modified
    EMS_LIMIT_SECTOR* pPlanSectorNow = &pEMS->planSectorNow;
    // LJH240911 AvgPwrFirst add emBatt_WorkMode
    if (pPlanSectorNow->emBatt_WorkMode != BATT_WORKMODE_FIRST_DISCHRG)
    {
        return TRUE;
    }
    //John-Added 20240413. ZYF Req Need Added.
 	if(pCCU->emDisChgByStopSOC == TRUE)
    {
        if(nBattSOC <= pPlanSectorNow->nStopSOC)
        {
            //TRACE("IsBatt_Plan2_StopDischarge: fJudgeSOC =%d pSTBatt_Rough->nProtect_SOC=%d\n",
            //	nBattSOC , pSTBatt_Rough->nProtectSOC);
            return TRUE;
        }
    }
 
	return FALSE;
}

/*********************************************************************************
*  
*  FUNCTION NAME :   
*  CREATOR       :  
*  CREATED DATE  : 2022-04-13 11:13:53
*  DESCRIPTION   :  //冒泡排序法
*				    
*  
*  CHANGE HISTORY:
*  1.LJH250110 增加延时 MAX_S_SORTED_NODE 防止频繁交换
***********************************************************************************/
void Sort_NodeOrderByChrgedEnergy(ROUGH_CCU_DATA *pCCU)
{
    EMS_MGMT_STU *pEMS	= 	&pCCU->emsMgmtData;
	int i = 0, j = 0;
	int 	nLen = pEMS->nNodeNum;
    A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Nodes = pEMS->calc_Node;
    static const UINT MAX_S_SORTED_NODE = 3600;
    TRACE(">>> Sort_NodeOrderByChrgedEnergy nNodeIdx:%d %d %d %d fChrgedEnergy:%.1f %.1f %.1f %.1f stmLastSorted:%lu sNeedSwap:%d",
        pCalc_Nodes[0].nNodeIdx, pCalc_Nodes[1].nNodeIdx, pCalc_Nodes[2].nNodeIdx, pCalc_Nodes[3].nNodeIdx,
        pCalc_Nodes[0].fChrgedEnergy, pCalc_Nodes[1].fChrgedEnergy, pCalc_Nodes[2].fChrgedEnergy, pCalc_Nodes[3].fChrgedEnergy, pEMS->tm_LastSort, pEMS->bSwapLastSort);
    if (ElapseS(pEMS->tm_LastSort) > MAX_S_SORTED_NODE)
    {
        BOOL bSwap = FALSE;//加判断条件  减少比较次数
        for (i = 0; i < nLen - 1; i++)
        {
            for (j = 0; j < nLen - 1 - i; j++)
            {
                //the Lower the ChargedEnergy, the first in queue
                if (pCalc_Nodes[j].fChrgedEnergy > (pCalc_Nodes[j + 1].fChrgedEnergy + 100))//John 20220413,Over 100KWH,Sort the Order
                {
                    A_BLOB_BRANCH_NODE_CALC_DATA tmpNode;
                    tmpNode =            pCalc_Nodes[j];
                    pCalc_Nodes[j] =     pCalc_Nodes[j + 1];
                    pCalc_Nodes[j + 1] = tmpNode;
                    bSwap = TRUE;
                    // TRACE("Sort_NodeOrderByChrgedEnergy j:%d bSwap:%d", j, bSwap);
                }
            }
            if (!bSwap)
            {
                break;
            }
        }

        if (bSwap)
        {
            for (i = 0; i < nLen; i++)
            {
                pEMS->idxsLastSort[i] = (BYTE)pCalc_Nodes[i].nNodeIdx;
            }
        }
        pEMS->tm_LastSort = _NOW_;
        pEMS->bSwapLastSort = bSwap;
        TRACE("Sort_NodeOrderByChrgedEnergy bSwap:%d sNeedSwap:%d %s", bSwap, pEMS->bSwapLastSort, printableBytes(pEMS->idxsLastSort, nLen));
    }
    else
    {
        // 按上次的排序
        TRACE("Sort_NodeOrderByChrgedEnergy By old sorted before %s nNodeIdx:%d %d %d %d fChrgedEnergy:%.1f %.1f %.1f %.1f",
            printableBytes(pEMS->idxsLastSort, nLen),
            pCalc_Nodes[0].nNodeIdx, pCalc_Nodes[1].nNodeIdx, pCalc_Nodes[2].nNodeIdx, pCalc_Nodes[3].nNodeIdx,
            pCalc_Nodes[0].fChrgedEnergy, pCalc_Nodes[1].fChrgedEnergy, pCalc_Nodes[2].fChrgedEnergy, pCalc_Nodes[3].fChrgedEnergy);

        if (pEMS->bSwapLastSort)
        {
            // LJH250210 fix
            // char szBuf[128] = "";
            static A_BLOB_BRANCH_NODE_CALC_DATA calc_Nodes[MAX_SEMS_CLIENTS_NUM];
            memcpy(calc_Nodes, pCalc_Nodes, sizeof(calc_Nodes));
            for (j = 0; j < nLen; j++)
            {
                pCalc_Nodes[j] = calc_Nodes[pEMS->idxsLastSort[j]];
                // snprintf(szBuf+strlen(szBuf), 1024, "%d ", pCalc_Nodes[j].nNodeIdx);
            }
            TRACE("Sort_NodeOrderByChrgedEnergy sNeedSwap NodeIdx %d %d %d %d ", // "szBuf:%s",
                pCalc_Nodes[0].nNodeIdx, pCalc_Nodes[1].nNodeIdx, pCalc_Nodes[2].nNodeIdx, pCalc_Nodes[3].nNodeIdx); //, szBuf);
        }
    }
	TRACE("Sort_NodeOrderByChrgedEnergy nNodeIdx:%d nNodeIdx:%d %d %d %d fChrgedEnergy:%.1f %.1f %.1f %.1f >>>",
        pEMS->nNodeNum, pCalc_Nodes[0].nNodeIdx, pCalc_Nodes[1].nNodeIdx, pCalc_Nodes[2].nNodeIdx, pCalc_Nodes[3].nNodeIdx,
        pCalc_Nodes[0].fChrgedEnergy, pCalc_Nodes[1].fChrgedEnergy, pCalc_Nodes[2].fChrgedEnergy, pCalc_Nodes[3].fChrgedEnergy);
}

// XSY251218 SocFirst
void Sort_NodeOrderBySoc(ROUGH_CCU_DATA *pCCU, WORKMODE_REQ workmodeReq)
{
    EMS_MGMT_STU *pEMS	= 	&pCCU->emsMgmtData;
	int i = 0, j = 0;
	int 	nLen = pEMS->nNodeNum;
    A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Nodes = pEMS->calc_Node;
    static const UINT DIFF_SOC = 4; // LJH260105 add
    if((ElapseS(pEMS->tm_LastSort) >= 20*60) || (workmodeReq != pEMS->emLastSocSortWm)) // wait 20 min
    {
        pEMS->emLastSocSortWm = workmodeReq;
        A_BLOB_BRANCH_NODE_CALC_DATA tmpNode;
        BOOL bSwap = FALSE;//加判断条件  减少比较次数

        if(workmodeReq == CHARGE)
        {
            for (i = 0; i < nLen - 1; i++)
            {
                for (j = 0; j < nLen - 1 - i; j++)
                {
                    //the Lower SOC, the first in queue
                    if(pEMS->calc_Node[j].nSOC > pEMS->calc_Node[j + 1].nSOC + DIFF_SOC)
                    {
                        tmpNode = pEMS->calc_Node[j];
                        pEMS->calc_Node[j] = pEMS->calc_Node[j + 1];
                        pEMS->calc_Node[j + 1] = tmpNode;
                        bSwap = TRUE;
                    }
                }
                if (!bSwap)
                {
                    break;
                }
            }
        }
        else if(workmodeReq == DISCHARGE)
        {
            for (i = 0; i < nLen - 1; i++)
            {
                for (j = 0; j < nLen - 1 - i; j++)
                {
                    //the Higher SOC, the first in queue
                    if(pEMS->calc_Node[j].nSOC + DIFF_SOC < pEMS->calc_Node[j + 1].nSOC)
                    {
                        tmpNode = pEMS->calc_Node[j];
                        pEMS->calc_Node[j] = pEMS->calc_Node[j + 1];
                        pEMS->calc_Node[j + 1] = tmpNode;
                        bSwap = TRUE;
                    }
                }
                if (!bSwap)
                {
                    break;
                }
            }
        }

        if(bSwap)
        {
            for (i = 0; i < nLen; i++)
            {
                pEMS->idxsLastSort[i] = (BYTE)pCalc_Nodes[i].nNodeIdx;
            }
        }
        pEMS->tm_LastSort = _NOW_;
        pEMS->bSwapLastSort = bSwap;
    }
    else
    {
        // 按上次的排序
        if (pEMS->bSwapLastSort)
        {
            static A_BLOB_BRANCH_NODE_CALC_DATA calc_Nodes[MAX_SEMS_CLIENTS_NUM];
            memcpy(calc_Nodes, pCalc_Nodes, sizeof(calc_Nodes));
            for (j = 0; j < nLen; j++)
            {
                pCalc_Nodes[j] = calc_Nodes[pEMS->idxsLastSort[j]];
            }
        }
    }
}

// LJH240911 AvgPwrFirst
void Sort_NodeOrderByAbility_Chrg(ROUGH_CCU_DATA *pCCU)
{
    EMS_MGMT_STU *pEMS	= 	&pCCU->emsMgmtData;
    BOOL bSwap = FALSE;//加判断条件  减少比较次数
    int i = 0, j = 0;
    int nLen = pEMS->nNodeNum;
    for (i = 0; i < nLen - 1; i++)
    {
        for (j = 0; j < nLen - 1 - i; j++)
        {
            //the Lower the Ability, the first in queue
            if (pEMS->calc_Node[j].fMax_Chrg_PwrAbility > (pEMS->calc_Node[j + 1].fMax_Chrg_PwrAbility))
            {
                A_BLOB_BRANCH_NODE_CALC_DATA  tmpNode;
                memcpy(&tmpNode, &pEMS->calc_Node[j], sizeof(A_BLOB_BRANCH_NODE_CALC_DATA));
                memcpy(&pEMS->calc_Node[j], &pEMS->calc_Node[j + 1], sizeof(A_BLOB_BRANCH_NODE_CALC_DATA));
                memcpy(&pEMS->calc_Node[j + 1] , &tmpNode, sizeof(A_BLOB_BRANCH_NODE_CALC_DATA));
                bSwap = TRUE;
            }
        }
        if (!bSwap)
        {
            break;
        }
    }
}

void Sort_NodeOrderByAbility_DisChrg(ROUGH_CCU_DATA *pCCU)
{
    EMS_MGMT_STU *pEMS	= 	&pCCU->emsMgmtData;
    BOOL 	bSwap = FALSE;//加判断条件  减少比较次数
    int i = 0, j = 0;
    int nLen = pEMS->nNodeNum;
    for (i = 0; i < nLen - 1; i++)
    {
        for (j = 0; j < nLen - 1 - i; j++)
        {
            //the Lower the Ability, the first in queue
            if (pEMS->calc_Node[j].fMax_DisChrg_PwrAbility > (pEMS->calc_Node[j + 1].fMax_DisChrg_PwrAbility))
            {
                A_BLOB_BRANCH_NODE_CALC_DATA  tmpNode;
                memcpy(&tmpNode, &pEMS->calc_Node[j], sizeof(A_BLOB_BRANCH_NODE_CALC_DATA));
                memcpy(&pEMS->calc_Node[j], &pEMS->calc_Node[j + 1], sizeof(A_BLOB_BRANCH_NODE_CALC_DATA));
                memcpy(&pEMS->calc_Node[j + 1] , &tmpNode, sizeof(A_BLOB_BRANCH_NODE_CALC_DATA));
                bSwap = TRUE;
            }
        }
        if (!bSwap)
        {
            break;
        }
    }
}

/*********************************************************************************
*  
*  FUNCTION NAME :   
*  CREATOR       :  
*  CREATED DATE  : 2022-04-13 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Sort_NodeOrderByNodeIdx(ROUGH_CCU_DATA *pCCU)
{
	EMS_MGMT_STU *pEMS	= 	&pCCU->emsMgmtData;

    BOOL bSwap = FALSE;//加判断条件  减少比较次数
	int i = 0, j = 0;
	int nLen = pEMS->nNodeNum;

    for (i = 0; i < nLen - 1; i++)
    {
        for (j = 0; j < nLen - 1 - i; j++)
        {
			//the Lower the ChargedEnergy, the first in queue
            if (pEMS->calc_Node[j].nNodeIdx > pEMS->calc_Node[j + 1].nNodeIdx )
            {
				A_BLOB_BRANCH_NODE_CALC_DATA  tmpNode;

                //tmpNode = arr[j];
				memcpy(&tmpNode, &pEMS->calc_Node[j], sizeof(A_BLOB_BRANCH_NODE_CALC_DATA));
                //arr[j] = arr[j + 1];
				memcpy(&pEMS->calc_Node[j], &pEMS->calc_Node[j + 1], sizeof(A_BLOB_BRANCH_NODE_CALC_DATA));
                //arr[j + 1] = tmpNode;//交换
				memcpy(&pEMS->calc_Node[j + 1] , &tmpNode, sizeof(A_BLOB_BRANCH_NODE_CALC_DATA));
                bSwap = TRUE;
            }
        }
        if (!bSwap)
        {
            return;
        }
    }

	TRACEX("Sort_NodeOrderByChrgedEnergy: nLen =%d\n", pEMS->nNodeNum);
	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :   
*  CREATOR       :  
*  CREATED DATE  : 2022-04-13 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void MPET_Calc4DispatchChrgMode(ROUGH_CCU_DATA *pCCU)
{
	EMS_MGMT_STU *pEMS	= 	&pCCU->emsMgmtData;
	UINT m = 0;
	UINT nNodeIdx = 0;
    BOOL bBreak = FALSE;
	
	pEMS->fTotalPM_Chrg_PwrAbility =    0.01f;
    pEMS->fTotalPM_Chrg_PwrAbilityAll = 0.01f;
	for(m = 0; m < (UINT)pEMS->nNodeNum; m++)
	{
        //added by John To Solve Exp: Battery Current SOC = End SOC = 20%, fTotalPM_Chrg_PwrAbility will Added this Branch Value.
        if(IsBatt_Plan2_StartChrg(pCCU, pEMS->calc_Node[m].nSOC) == FALSE)
        {
            TRACEW("MPET_Calc4DispatchChrgMode continue; stop charge m:%d nSOC:%d",
                m, pEMS->calc_Node[m].nSOC);
            continue;
        }
        // LJH240911 AvgPwrFirst 增加:fMax_Chrg_PwrAbility fTotalPM_Chrg_PwrAbilityAll
        pEMS->fTotalPM_Chrg_PwrAbilityAll += pEMS->calc_Node[m].fMax_Chrg_PwrAbility;

        if (!bBreak)
        {
            pEMS->fTotalPM_Chrg_PwrAbility = pEMS->fTotalPM_Chrg_PwrAbilityAll;
            pEMS->fChrg_PwrRate = pEMS->fRemain_ACChrgingPwr_Lmt/pEMS->fTotalPM_Chrg_PwrAbility;
            pEMS->fChrg_PwrRate = MAX(pEMS->fChrg_PwrRate, 0);
            if(pEMS->nLast_Chrg_NodeNum == m) //If Last Calc Node is some to Current, So Adjust the Max Lmt Point To 1.0
            {
                if(pEMS->fChrg_PwrRate <= 1.0) //[0.7,1.0] is Adjust Range,Avoid Shake.
                {
                    pEMS->nChrg_NodeNum = m;
                    bBreak = TRUE;
                    //break;
                }
                pEMS->nChrg_NodeNum = m;
            }
            else
            {
                if(pEMS->fChrg_PwrRate < 0.7) //Find Single Group Max Efficient Range
                {
                    pEMS->nChrg_NodeNum = m;
                    bBreak = TRUE;
                    //break;
                }
                pEMS->nChrg_NodeNum = m;
            }
        }
	}
    pEMS->nLast_Chrg_NodeNum = pEMS->nChrg_NodeNum;

    // old
    // for(m = 0; m < MAX_SINGLE_STBATT_NUM; m++)
	// {
    //     //added by John To Solve Exp: Battery Current SOC = End SOC = 20%, fTotalPM_Chrg_PwrAbility will Added this Branch Value.
    //     if(IsBatt_Plan2_StartChrg(pCCU, pEMS->calc_Node[m].nSOC) == FALSE)
    //     {
    //         TRACE("If Current Branch Not Need StartChg, Skip.Idx=[%d]\n", m);
    //         continue;
    //     }

 	// 	pEMS->fTotalPM_Chrg_PwrAbility += MIN(pEMS->calc_Node[m].fACDC_Max_Ability_Pwr,pEMS->calc_Node[m].fRemain_Batt_Chrg_Pwr_Req);
	// 	pEMS->fChrg_PwrRate = pEMS->fRemain_ACChrgingPwr_Lmt/pEMS->fTotalPM_Chrg_PwrAbility;
    //     pEMS->fChrg_PwrRate = MAX(pEMS->fChrg_PwrRate, 0);

	// 	if(pEMS->nLast_Chrg_NodeNum == m) //If Last Calc Node is some to Current, So Adjust the Max Lmt Point To 1.0
	// 	{
	// 		if(pEMS->fChrg_PwrRate <= 1.0) //[0.7,1.0] is Adjust Range,Avoid Shake.
	// 		{
	// 			pEMS->nChrg_NodeNum = m;
	// 			break;
	// 		}

	// 		pEMS->nChrg_NodeNum = m;
	// 	}
	// 	else
	// 	{
	// 		if(pEMS->fChrg_PwrRate < 0.7) //Find Single Group Max Efficient Range
	// 		{
	// 			pEMS->nChrg_NodeNum = m;
	// 			break;
	// 		}

	// 		pEMS->nChrg_NodeNum = m;
	// 	}	
	// }
	// pEMS->nLast_Chrg_NodeNum = pEMS->nChrg_NodeNum;

	TRACE("n=[%d]nNodeIdx=[%d] nChrg_NodeNum=[%d] fRemain_ACChrgingPwr_Lmt=[%.1f] fTotalPM_Chrg_PwrAbility=[%.1f] fChrg_PwrRate=[%.2f] \n",\
	 	m, nNodeIdx, pEMS->nChrg_NodeNum, pEMS->fRemain_ACChrgingPwr_Lmt, pEMS->fTotalPM_Chrg_PwrAbility, pEMS->fChrg_PwrRate);

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :   
*  CREATOR       :  
*  CREATED DATE  : 2022-04-13 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void MPET_Calc4DispatchDisChrgMode(ROUGH_CCU_DATA *pCCU)
{
	EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
	UINT n = 0;
	UINT nNodeIdx = 0;
    BOOL bBreak = FALSE;

	pEMS->fTotalPM_DisChrging_PwrAbility =    0.01f;
    pEMS->fTotalPM_DisChrging_PwrAbilityAll = 0.01f;
	for(n = 0; n < (UINT)pEMS->nNodeNum; n++)
	{
        // LJH240911 AvgPwrFirst 增加:1.停止放电不计入fTotalPM_DisChrging_PwrAbility 2.fMax_DisChrg_PwrAbility
        if(IsBatt_Plan2_StopDischarge(pCCU, pEMS->calc_Node[n].nSOC) == TRUE)
        {
            TRACEW("MPET_Calc4DispatchDisChrgMode continue; stop discharge n:%d nSOC:%d",
                n, pEMS->calc_Node[n].nSOC);
 		    continue;
        }
        pEMS->fTotalPM_DisChrging_PwrAbilityAll += pEMS->calc_Node[n].fMax_DisChrg_PwrAbility;

        if (!bBreak)
        {
            pEMS->fTotalPM_DisChrging_PwrAbility = pEMS->fTotalPM_DisChrging_PwrAbilityAll;
            pEMS->fDisChrging_PwrRate = pEMS->fRemain_ACLoad_DisChrgPwr_Req/pEMS->fTotalPM_DisChrging_PwrAbility;
            pEMS->fDisChrging_PwrRate = MAX(pEMS->fDisChrging_PwrRate, 0);

            if(pEMS->nLast_DisChrging_NodeNum == n) //If Last Calc Node is some to Current, So Adjust the Max Lmt Point To 1.0
            {
                if(pEMS->fDisChrging_PwrRate <= 1.0) //[0.7,1.0] is Adjust Range,Avoid Shake.
                {
                    pEMS->nDisChrging_NodeNum = n;
                    bBreak = TRUE;
                    //break;
                }
                pEMS->nDisChrging_NodeNum = n;
            }
            else
            {
                if(pEMS->fDisChrging_PwrRate < 0.7) //Find Single Group Max Efficient Range
                {
                    pEMS->nDisChrging_NodeNum = n;
                    bBreak = TRUE;
                    //break;
                }
                pEMS->nDisChrging_NodeNum = n;
            }	
        }
	}
    pEMS->nLast_DisChrging_NodeNum = pEMS->nDisChrging_NodeNum;

    // old
    // for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	// {
	// 	pEMS->fTotalPM_DisChrging_PwrAbility += MIN(pEMS->calc_Node[n].fACDC_Max_Ability_Pwr, pEMS->calc_Node[n].fRemain_Batt_DisChrg_Pwr);
	// 	pEMS->fDisChrging_PwrRate = pEMS->fRemain_ACLoad_DisChrgPwr_Req/pEMS->fTotalPM_DisChrging_PwrAbility;
	// 	pEMS->fDisChrging_PwrRate = MAX(pEMS->fDisChrging_PwrRate, 0);

	// 	if(pEMS->nLast_DisChrging_NodeNum == n) //If Last Calc Node is some to Current, So Adjust the Max Lmt Point To 1.0
	// 	{
	// 		if(pEMS->fDisChrging_PwrRate <= 1.0) //[0.7,1.0] is Adjust Range,Avoid Shake.
	// 		{
	// 			pEMS->nDisChrging_NodeNum = n;
	// 			break;
	// 		}

	// 		pEMS->nDisChrging_NodeNum = n;
	// 	}
	// 	else
	// 	{
	// 		if(pEMS->fDisChrging_PwrRate < 0.7) //Find Single Group Max Efficient Range
	// 		{
	// 			pEMS->nDisChrging_NodeNum = n;
	// 			break;
	// 		}

	// 		pEMS->nDisChrging_NodeNum = n;
	// 	}	
	// }
	// pEMS->nLast_DisChrging_NodeNum = pEMS->nDisChrging_NodeNum;

	TRACEX("n=[%d] nNodeIdx=[%d]fRemain_ACLoad_DisChrgPwr_Req=[%.1f] fTotalPM_DisChrging_PwrAbility=[%.1f] fDisChrging_PwrRate=[%.2f]\n",\
		n,nNodeIdx,pEMS->fRemain_ACLoad_DisChrgPwr_Req,pEMS->fTotalPM_DisChrging_PwrAbility,pEMS->fDisChrging_PwrRate);

    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  Refresh ESSRemainPower
*  CREATOR       :  
*  CREATED DATE  : 2022-04-13 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Refresh_ESSRemainPower(ROUGH_CCU_DATA* pCCU)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    //John 20220401,Add Avg Filter,To Solve Power Freq Up->Down->Up. 
    int nPointNum = (int)(MIN_FILTER_POINT + pEMS->nPrio*10);
    if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_CHRGING)
    {
        if ((pCCU->lcCfg.lcType == LC_TYPE_1st) && (!pEMS->bCtrlByREMS))
        {
            pEMS->fAvg_Remain_ChrgingPwr_Lmt = 
                    Samp_Avg_Filter(FILTERTYPE_REMAIN_CHRGING_LMT, pEMS->fTotal_PessChg_PidOut, nPointNum);
            if (pCCU->bQuickChannle || pEMS->emHighSensitivityEnable)
            {
                // printf("Refresh_ESSRemainPower Charge: %d\n", pEMS->emHighSensitivityEnable);
                pEMS->fAvg_Remain_ChrgingPwr_Lmt = pEMS->fTotal_PessChg_PidOut;
            }
            // else
            // {
            //     pEMS->fAvg_Remain_ChrgingPwr_Lmt = 
            //         Samp_Avg_Filter(FILTERTYPE_REMAIN_CHRGING_LMT, pEMS->fTotal_PessChg_PidOut, nPointNum);
            // }
            //Remain Charg Limit
            pEMS->fRemain_ACChrgingPwr_Lmt = 
                MIN(pEMS->fTotal_PessChg_PidOut, pEMS->fAvg_Remain_ChrgingPwr_Lmt);
            
            pEMS->fRemain_ACChrgingPwr_Lmt = 
                MAX(pEMS->fRemain_ACChrgingPwr_Lmt, 0);
        }
        else
        {
            if (pCCU->bQuickChannle || pEMS->emHighSensitivityEnable)
            {
                // printf("Refresh_ESSRemainPower Charge: %d\n", pEMS->emHighSensitivityEnable);
                pEMS->fAvg_Remain_ChrgingPwr_Lmt = pEMS->fTotal_PessChgAvaliable;
            }
            else
            {
                pEMS->fAvg_Remain_ChrgingPwr_Lmt = 
                    Samp_Avg_Filter(FILTERTYPE_REMAIN_CHRGING_LMT, pEMS->fTotal_PessChgAvaliable, nPointNum);
            }
            //Remain Charg Limit
            pEMS->fRemain_ACChrgingPwr_Lmt = 
                MIN(pEMS->fTotal_PessChgAvaliable, pEMS->fAvg_Remain_ChrgingPwr_Lmt);
            
            pEMS->fRemain_ACChrgingPwr_Lmt = 
                MAX(pEMS->fRemain_ACChrgingPwr_Lmt, 0);
        }
    }
    else if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_DISCHRG)
    {
        if ((pCCU->lcCfg.lcType == LC_TYPE_1st) && (!pEMS->bCtrlByREMS))
        {
            pEMS->fAvg_Remain_DisChrgPwr_Req = Samp_Avg_Filter(FILTERTYPE_REMAIN_DISCHRG_PWR, pEMS->fTotal_PessDischg_PidOut, (int)pCCU->nDisChrgFilterPoint); //nPointNum
            if (pCCU->bQuickChannle || pEMS->emHighSensitivityEnable)
            {
                // printf("Refresh_ESSRemainPower Discharge: %d\n", pEMS->emHighSensitivityEnable);
                pEMS->fAvg_Remain_DisChrgPwr_Req = pEMS->fTotal_PessDischg_PidOut;
            }
            // else
            // {
            //     pEMS->fAvg_Remain_DisChrgPwr_Req = Samp_Avg_Filter(FILTERTYPE_REMAIN_DISCHRG_PWR, pEMS->fTotal_PessDischg_PidOut, (int)pCCU->nDisChrgFilterPoint); //nPointNum
            // }
            ///Remain Discharge Power Limit
            pEMS->fRemain_ACLoad_DisChrgPwr_Req = 
                MIN(pEMS->fTotal_PessDischg_PidOut, pEMS->fAvg_Remain_DisChrgPwr_Req); 

            pEMS->fRemain_ACLoad_DisChrgPwr_Req = 
                MAX(pEMS->fRemain_ACLoad_DisChrgPwr_Req, 0); 
        }
        else
        {
            if (pCCU->bQuickChannle || pEMS->emHighSensitivityEnable)
            {
                // printf("Refresh_ESSRemainPower Discharge: %d\n", pEMS->emHighSensitivityEnable);
                pEMS->fAvg_Remain_DisChrgPwr_Req = pEMS->fTotal_PessDischgLmt;
            }
            else
            {
                pEMS->fAvg_Remain_DisChrgPwr_Req = Samp_Avg_Filter(FILTERTYPE_REMAIN_DISCHRG_PWR, pEMS->fTotal_PessDischgLmt, (int)pCCU->nDisChrgFilterPoint); //nPointNum
            }

            ///Remain Discharge Power Limit
            pEMS->fRemain_ACLoad_DisChrgPwr_Req = 
                MIN(pEMS->fTotal_PessDischgLmt, pEMS->fAvg_Remain_DisChrgPwr_Req); 

            pEMS->fRemain_ACLoad_DisChrgPwr_Req = 
                MAX(pEMS->fRemain_ACLoad_DisChrgPwr_Req, 0); 
        }
    }
    //--- Added For Q ReactivePower ---//
    if (pCCU->bQuickChannle || pEMS->emHighSensitivityEnable)
    {
        // printf("Refresh_ESSRemainPower Q: %d\n", pEMS->emHighSensitivityEnable);
        pEMS->fAvg_Remain_QoutputLmt = pEMS->fTotal_Qess_OutputLmt;
    }
    else
    {
        pEMS->fAvg_Remain_QoutputLmt = Samp_Avg_Filter(FILTERTYPE_REMAIN_REACTVPWR_LMT, pEMS->fTotal_Qess_OutputLmt, MIN_FILTER_POINT);    
    }
    pEMS->fRemain_QoutputLmt = MIN(pEMS->fQacload4Org, pEMS->fAvg_Remain_QoutputLmt); 
    pEMS->fRemain_QoutputLmt = MAX(pEMS->fRemain_QoutputLmt, 0);
    TRACED("Refresh_ESSRemainPower emBatt_WorkModeReq:%d 5:idle "
        "fAvg_Remain_ChrgingPwr_Lmt:%.1f fRemain_ACChrgingPwr_Lmt:%.1f fTotal_PessChgAvaliable:%.1f "
        "fAvg_Remain_DisChrgPwr_Req:%.1f fRemain_ACLoad_DisChrgPwr_Req:%.1f fTotal_PessDischgLmt:%.1f nPointNum:%d",
        pEMS->emBatt_WorkModeReq, pEMS->fAvg_Remain_ChrgingPwr_Lmt, pEMS->fRemain_ACChrgingPwr_Lmt, pEMS->fTotal_PessChgAvaliable,
        pEMS->fAvg_Remain_DisChrgPwr_Req, pEMS->fRemain_ACLoad_DisChrgPwr_Req, pEMS->fTotal_PessDischgLmt, nPointNum);
    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-04-13 11:13:53
*  DESCRIPTION   : 放电功率分配
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void DisPatch_ACLoadReq_DisChrg_Power(ROUGH_CCU_DATA* pCCU)
{
    int m = 0;
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
	float fACLoad_DisChrgPwr_Req = pEMS->fRemain_ACLoad_DisChrgPwr_Req;

    // LJH240911 AvgPwrFirst
    BOOL bEfficient = (pCCU->blobBackendCfg.emPowCtrlMode == POW_CTRL_MODE_EFFICIENCY || 
                        (fACLoad_DisChrgPwr_Req>1.0 && pEMS->fTotalPM_DisChrging_PwrAbilityAll>1.0 &&
                            fACLoad_DisChrgPwr_Req >= pEMS->fTotalPM_DisChrging_PwrAbilityAll));
    if (!bEfficient)
    {
        Sort_NodeOrderByAbility_DisChrg(pCCU);
    }

#ifdef TRACE_AvgPwrFirst
    memset(fRemain_ACLoad_DisChrgPwr_Req, 0, sizeof(fRemain_ACLoad_DisChrgPwr_Req));
    s_bEfficientDisChrg = bEfficient;
#endif

    //Get Current Sector
    for(m = 0; m < pEMS->nNodeNum; m++)
    {
#ifdef TRACE_AvgPwrFirst
        fRemain_ACLoad_DisChrgPwr_Req[m] = pEMS->fRemain_ACLoad_DisChrgPwr_Req;
#endif
        // LJH0303 Modify pEMS->nTMSector == -1 !BATT_WORKMODE_FIRST_DISCHRG
        if(pEMS->nTMSector < 0)
 		{
 		    //John 20220413. If Not Config, Not Allocate Power.
 		    pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr = 0;
            TRACEW("DisPatch_ACLoadReq_DisChrg_Power fBatt_Distri_4_ACDC_Pwr:0 pEMS->nTMSector == -1");
 		    continue;
 		}
        if(pEMS->calc_Node[m].emBatt_WorkMode != BATT_WORKMODE_FIRST_DISCHRG)
        {
            pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr = 0.0;
            TRACEW("DisPatch_ACLoadReq_DisChrg_Power m:%d fBatt_Distri_4_ACDC_Pwr:0 !BATT_WORKMODE_FIRST_DISCHRG", m);
            continue;
        }
        // LJH240911 AvgPwrFirst add fMax_DisChrg_PwrAbility <= 1.0
        if (pEMS->calc_Node[m].fMax_DisChrg_PwrAbility <= 1.0)
        {
            pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr = 0;
            TRACEW("DisPatch_ACLoadReq_DisChrg_Power m:%d fBatt_Distri_4_ACDC_Pwr:0 fMax_DisChrg_PwrAbility <= 1.0", m);
 		    continue;
        }
        if(pEMS->fTotalPM_DisChrging_PwrAbilityAll <= 1.0)
        {
 		    pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr = 0;
            TRACEW("DisPatch_ACLoadReq_DisChrg_Power m:%d fBatt_Distri_4_ACDC_Pwr:0 fTotalPM_DisChrging_PwrAbilityAll <= 1.0", m);
 		    continue;
        }
        if(bEfficient && pEMS->fTotalPM_DisChrging_PwrAbility<= 1.0)
        {
 		    pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr = 0;
            TRACEW("DisPatch_ACLoadReq_DisChrg_Power m:%d fBatt_Distri_4_ACDC_Pwr:0 fTotalPM_DisChrging_PwrAbility <= 1.0", m);
 		    continue;
        }
        // LJH241202 SEMS 由Ems_init初始化
 		// if(pEMS->bEnterUrgetDischrgMode == TRUE)
 		// {
 		//     pEMS->calc_Node[m].emBatt_WorkMode = BATT_WORKMODE_FIRST_DISCHRG;
 		// }
 		// else
 		// {
 		//     pEMS->calc_Node[m].emBatt_WorkMode = pEMS->planSectorNow.emBatt_WorkMode;
 		// }
        //Added By John. 2024-04-16, Move By Gorege 2024-07-26
        if(IsBatt_Plan2_StopDischarge(pCCU, pEMS->calc_Node[m].nSOC) == TRUE)
        {
            pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr = 0;
            TRACEW("DisPatch_ACLoadReq_DisChrg_Power m:%d fBatt_Distri_4_ACDC_Pwr:0 IsBatt_Plan2_StopDischarge", m);
 		    continue;
        }

        // old
        // float fDischgAbility = MIN(pEMS->calc_Node[m].fACDC_Max_Ability_Pwr, pEMS->calc_Node[m].fRemain_Batt_DisChrg_Pwr);
 		// pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr = 
		//  (fDischgAbility/pEMS->fTotalPM_DisChrging_PwrAbility) * fACLoad_DisChrgPwr_Req;
        // LJH240911 AvgPwrFirst
        if (bEfficient)
        {
            pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr = 
                (pEMS->calc_Node[m].fMax_DisChrg_PwrAbility/pEMS->fTotalPM_DisChrging_PwrAbility) * 
                fACLoad_DisChrgPwr_Req;
        }
        else
        {
            // LJH240911 AvgPwrFirst
            float fAvgRemain_ACLoad_DisChrgPwr_Req = (float)(pEMS->fRemain_ACLoad_DisChrgPwr_Req/(float)(pEMS->nNodeNum-m));
            // Judge_IfNeedDisChg_ON 限制了最小功率
            float fPwrLmtMin = MAX(pCCU->blobBackendCfg.fPwrLmtMin_AvgPwrMode, pEMS->calc_Node[m].fACDC_Max_Ability_Pwr*0.1f);
            if (fAvgRemain_ACLoad_DisChrgPwr_Req < pEMS->calc_Node[m].fMax_DisChrg_PwrAbility &&
                fAvgRemain_ACLoad_DisChrgPwr_Req > fPwrLmtMin-0.1)
            {
                // 同放
                pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr = fAvgRemain_ACLoad_DisChrgPwr_Req;
            }
            else
            {
                pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr = MIN(pEMS->fRemain_ACLoad_DisChrgPwr_Req, pEMS->calc_Node[m].fMax_DisChrg_PwrAbility);
            }
        }
        TRACED("DisPatch_ACLoadReq_DisChrg_Power m:%d fBatt_Distri_4_ACDC_Pwr:%.1f", m, pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr);
        // LJH250515 // LJH250222 Added 出现-D上报fACDC_Max_Ability_Pwr错误问题
        pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr =
            MIN(pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr, pEMS->calc_Node[m].fMax_DisChrg_PwrAbility);
        // LJH250612 planSector
        if (pCCU->lcCfg.emSystemType < SYSTYPE_EMS_STATION && pCCU->lcCfg.lcType == LC_TYPE_1st)
        {
            pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr =
                MIN(pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr, (float)pEMS->planSectorNow.nBattGrpPwrLmt);
        }
        // old pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr =
        //     MIN(pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr, (float)pEMS->planSectorNow.nBattGrpPwrLmt);
		// old pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr = 
		// 	MIN(pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr, pEMS->calc_Node[m].fACDC_Max_Ability_Pwr);

		pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr = 
			MIN(pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr, pEMS->calc_Node[m].fRemain_Batt_DisChrg_Pwr); 
		
		pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr = 
			MIN(pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr, pEMS->fRemain_ACLoad_DisChrgPwr_Req);

        pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr = 
			MAX(pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr, 0.0f);
        TRACED("DisPatch_ACLoadReq_DisChrg_Power m:%d fTotalPM_DisChrging_PwrAbility:%.1f "
			"fRemain_Batt_DisChrg_Pwr:%.1f fMax_DisChrg_PwrAbility:%.1f nBattGrpPwrLmt:%u fRemain_ACLoad_DisChrgPwr_Req:%.1f \n"
            ">>> fBatt_Distri_4_ACDC_Pwr=%.1f bEfficient:%d (%.1f/%.1f)*%.1f %.1f/%d",
			m, pEMS->fTotalPM_DisChrging_PwrAbility,
			pEMS->calc_Node[m].fRemain_Batt_DisChrg_Pwr, pEMS->calc_Node[m].fMax_DisChrg_PwrAbility, pEMS->planSectorNow.nBattGrpPwrLmt, pEMS->fRemain_ACLoad_DisChrgPwr_Req,
            
            pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr, bEfficient,
            pEMS->calc_Node[m].fMax_DisChrg_PwrAbility, pEMS->fTotalPM_DisChrging_PwrAbility, fACLoad_DisChrgPwr_Req,
            pEMS->fRemain_ACLoad_DisChrgPwr_Req, pEMS->nNodeNum-m);

        //Step2. Calc Remain ACLoad_Power_Req & Remain Battery Power
        pEMS->calc_Node[m].fRemain_Batt_DisChrg_Pwr = pEMS->calc_Node[m].fRemain_Batt_DisChrg_Pwr - pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr;
        pEMS->fRemain_ACLoad_DisChrgPwr_Req = pEMS->fRemain_ACLoad_DisChrgPwr_Req - pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr;

        if(pEMS->fRemain_ACLoad_DisChrgPwr_Req < 0.01)
        {
            //break; //John Modify it.
            pEMS->fRemain_ACLoad_DisChrgPwr_Req = 0;
            //pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr = 0; //John 2023-05-11,Avoid Cause This Group Not Dischg.
        }
    }

    TRACE("------ End DisPatch_ACLoadReq_DisChrg_Power ------\n");
    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  DisPatch_ReactivePower_Output
*  CREATOR       :  
*  CREATED DATE  : 2023-12-08 11:13:53
*  DESCRIPTION   : When EMS Plan On Chging Section, Calc ReactivePower Allocation.
*				    
*  
*  CHANGE HISTORY: if Grp#1 Pallocate = 110KW, Grp#2 Pallocate=80KW;
*  Reverse Allocate ReactivePower. Qgrp#2 = 35KW  Qgrp#1 = 10KW.
*  
***********************************************************************************/
void DisPatch_ReactivePower_Output(ROUGH_CCU_DATA* pCCU)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    int m = 0;

 
    for(m = 0; m < pEMS->nNodeNum; m++) // Allocate Reactive Power.
    {   
 		if(pEMS->nTMSector == -1)
 		{
 		    pEMS->calc_Node[m].fQoutput_Distri4ACDC = 0;
 		    continue;
 		}
        // LJH241202 SEMS 由Ems_init初始化
 		// if(pEMS->bEnterUrgetDischrgMode == TRUE)
 		// {
 		//     pEMS->calc_Node[m].emBatt_WorkMode = BATT_WORKMODE_FIRST_DISCHRG;
 		// }
 		// else
 		// {
 		//     pEMS->calc_Node[m].emBatt_WorkMode = pEMS->planSectorNow.emBatt_WorkMode;
 		// }
        
        //Added By John. 2024-04-16
        if(IsBatt_Plan2_StopDischarge(pCCU, pEMS->calc_Node[m].nSOC) == TRUE)
        {
 		    pEMS->calc_Node[m].fQoutput_Distri4ACDC = 0;
 		    continue;
        }

        float fPCS_Used = 0;
 		if(pEMS->calc_Node[m].emBatt_WorkMode == BATT_WORKMODE_FIRST_DISCHRG)
        {
            fPCS_Used = pEMS->calc_Node[m].fBatt_Distri_4_ACDC_Pwr;
            if(pEMS->calc_Node[m].fBatt_Max_DisChrg_Pwr <= 1.0)
            {
                pEMS->calc_Node[m].fQoutput_Distri4ACDC = 0;
                continue;
            }
        }
        else
        {
            fPCS_Used = pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr;
            if(pEMS->calc_Node[m].fBatt_Max_Chrg_Pwr_Req <= 1.0)
            {
                pEMS->calc_Node[m].fQoutput_Distri4ACDC = 0;
                continue;
            }
        }
        
        if(fPCS_Used <= 0) //John Added 2024-01-05. if Not Disttribute ActivePower, So Not Disttribute ReactivePower.
        {
            pEMS->calc_Node[m].fQoutput_Distri4ACDC = 0;
            continue; 
        }

        pEMS->calc_Node[m].fQoutput_Distri4ACDC = 
               pCCU->strLSYSInfo.branchNodeData[m].fACDC_Max_Output_Pwr_Ability - fPCS_Used;
           
        pEMS->calc_Node[m].fQoutput_Distri4ACDC = 
               MIN(pEMS->calc_Node[m].fQoutput_Distri4ACDC, pEMS->fRemain_QoutputLmt);

        pEMS->calc_Node[m].fQoutput_Distri4ACDC = 
               MAX(pEMS->calc_Node[m].fQoutput_Distri4ACDC, 0);

        pEMS->fRemain_QoutputLmt = pEMS->fRemain_QoutputLmt -  pEMS->calc_Node[m].fQoutput_Distri4ACDC;
        if(pEMS->fRemain_QoutputLmt <= 0)
        {
            pEMS->fRemain_QoutputLmt = 0;
        }

        TRACE("Idx=[%d] fQoutput_Distri4ACDC=[%.f] fPCS_Used=[%.f] fRemain_QoutputLmt=[%.f] emBatt_WorkMode=[%s]\n", \
            m, pEMS->calc_Node[m].fQoutput_Distri4ACDC, fPCS_Used, pEMS->fRemain_QoutputLmt,\
            (pEMS->calc_Node[m].emBatt_WorkMode==0)?"Dischg":"Chg");
    }
    
    return;
}

 

/*********************************************************************************
*  
*  FUNCTION NAME :  Dispatch_ESSLmtBattChrg_Power
*  CREATOR       :  
*  CREATED DATE  : 2022-04-13 11:13:53
*  DESCRIPTION   : 电池组充电功率分配
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Dispatch_ESSLmtBattChrg_Power(ROUGH_CCU_DATA* pCCU)
{
    int m = 0;
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
  	float fACChrgingPwr_Lmt = pEMS->fRemain_ACChrgingPwr_Lmt;

    // LJH240911 AvgPwrFirst
    BOOL bEfficient = (pCCU->blobBackendCfg.emPowCtrlMode == POW_CTRL_MODE_EFFICIENCY || 
                        (fACChrgingPwr_Lmt>1.0 && pEMS->fTotalPM_Chrg_PwrAbilityAll>1.0 &&
                            fACChrgingPwr_Lmt >= pEMS->fTotalPM_Chrg_PwrAbilityAll));
    if (!bEfficient)
    {
        Sort_NodeOrderByAbility_Chrg(pCCU);
    }
#ifdef TRACE_AvgPwrFirst
    memset(fRemain_ACChrgingPwr_Lmt, 0, sizeof(fRemain_ACChrgingPwr_Lmt));
    s_bEfficientChrg = bEfficient;
#endif

    for(m = 0; m < pEMS->nNodeNum; m++)
    {
#ifdef TRACE_AvgPwrFirst
        fRemain_ACChrgingPwr_Lmt[m] = pEMS->fRemain_ACChrgingPwr_Lmt;
#endif
        // LJH0303 Modify Add emBatt_WorkMode
        if(pEMS->nTMSector == -1) //John 20220413. If Not Config, Allocate 0.
        {
            pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = 0.0;
            TRACEW("Dispatch_ESSLmtBattChrg_Power fACDC_Distri_4_BattChrg_Pwr:0 pEMS->nTMSector == -1");
            continue;
        }
        if(pEMS->calc_Node[m].emBatt_WorkMode != BATT_WORKMODE_FIRST_CHRGING)
        {
            pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = 0.0;
            TRACEW("Dispatch_ESSLmtBattChrg_Power m:%d fACDC_Distri_4_BattChrg_Pwr:0 !BATT_WORKMODE_FIRST_CHRGING", m);
            continue;  
        }
        // LJH240911 AvgPwrFirst add fMax_Chrg_PwrAbility <= 1.0 nNodeNum=4,电池柜不在线分配0kW
        if(pEMS->calc_Node[m].fMax_Chrg_PwrAbility <= 1.0)
        {
            pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = 0.0f;
            TRACEW("Dispatch_ESSLmtBattChrg_Power m:%d fACDC_Distri_4_BattChrg_Pwr:0 fMax_Chrg_PwrAbility <= 1.0", m);
            continue;  
        }
        if(pEMS->fTotalPM_Chrg_PwrAbilityAll <= 1.0)
        {
 		    pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = 0.0f;
            TRACEW("Dispatch_ESSLmtBattChrg_Power m:%d fACDC_Distri_4_BattChrg_Pwr:0 fTotalPM_Chrg_PwrAbilityAll <= 1.0", m);
 		    continue;
        }
        if(bEfficient && pEMS->fTotalPM_Chrg_PwrAbility <= 1.0)
        {
            pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = 0.0f;
            TRACEW("Dispatch_ESSLmtBattChrg_Power m:%d fACDC_Distri_4_BattChrg_Pwr:0 fTotalPM_Chrg_PwrAbility <= 1.0", m);
            continue;  
        }
        if((pEMS->bEnterUrgetDischrgMode == FALSE) &&
            // (pEMS->planSectorNow.emBatt_WorkMode == BATT_WORKMODE_FIRST_CHRGING) && // LJH240911 AvgPwrFirst
            (IsBatt_Plan2_StartChrg(pCCU, pEMS->calc_Node[m].nSOC)))
        {
            // old
            // pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = 
		    //     (pEMS->calc_Node[m].fACDC_Max_Ability_Pwr/pEMS->fTotalPM_Chrg_PwrAbility) * fACChrgingPwr_Lmt;
            // LJH240911 AvgPwrFirst
            if (bEfficient)
            {
                pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = 
                    (pEMS->calc_Node[m].fMax_Chrg_PwrAbility/pEMS->fTotalPM_Chrg_PwrAbility) * 
                    fACChrgingPwr_Lmt;
            }
            else
            {
                // LJH240911 AvgPwrFirst
                float fAvgRemain_ACChrgingPwr_Lmt = (float)(pEMS->fRemain_ACChrgingPwr_Lmt/(float)(pEMS->nNodeNum-m));
                // Judge_IfNeedChg_Start 限制了最小功率
                float fPwrLmtMin = MAX(pCCU->blobBackendCfg.fPwrLmtMin_AvgPwrMode, pEMS->calc_Node[m].fACDC_Max_Ability_Pwr*0.1f);
                if (fAvgRemain_ACChrgingPwr_Lmt < pEMS->calc_Node[m].fMax_Chrg_PwrAbility &&
                    fAvgRemain_ACChrgingPwr_Lmt > fPwrLmtMin-0.1)
                {
                    // 同充
                    pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = fAvgRemain_ACChrgingPwr_Lmt;
                }
                else
                {
                    pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = MIN(pEMS->fRemain_ACChrgingPwr_Lmt, pEMS->calc_Node[m].fMax_Chrg_PwrAbility);
                }
            }
            TRACED("Dispatch_ESSLmtBattChrg_Power m:%d fACDC_Distri_4_BattChrg_Pwr:%.1f", m, pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr);
            pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = 
		        MIN(pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr,pEMS->calc_Node[m].fRemain_Batt_Chrg_Pwr_Req);
            // LJH250515 // LJH250222 Added 出现-D上报fACDC_Max_Ability_Pwr错误问题
            pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = 
                MIN(pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr, pEMS->calc_Node[m].fMax_Chrg_PwrAbility);
            // LJH250612 planSector
            if (pCCU->lcCfg.emSystemType < SYSTYPE_EMS_STATION && pCCU->lcCfg.lcType == LC_TYPE_1st)
            {
                pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = 
                    MIN(pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr, (float)pEMS->planSectorNow.nBattGrpPwrLmt);
            }
            // pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = 
            //     MIN(pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr, (float)pEMS->planSectorNow.nBattGrpPwrLmt);
            // old pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = 
		    //     MIN(pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr, pEMS->calc_Node[m].fACDC_Max_Ability_Pwr);
        
		    pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = 
		        MIN(pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr, pEMS->fRemain_ACChrgingPwr_Lmt);

            pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = 
		        MAX(pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr, 0.0f);
            TRACED("Dispatch_ESSLmtBattChrg_Power m:%d fACDC_Max_Ability_Pwr: %.1f fTotalPM_Chrg_PwrAbility:%.1f "
                "fRemain_Batt_Chrg_Pwr_Req:%.1f fMax_Chrg_PwrAbility:%.1f nBattGrpPwrLmt:%u fRemain_ACChrgingPwr_Lmt:%.1f \n"
                ">>> fACDC_Distri_4_BattChrg_Pwr=%.1f bEfficient:%d (%.1f/%.1f)*%.1f %.1f/%d",
                m, pEMS->calc_Node[m].fACDC_Max_Ability_Pwr, pEMS->fTotalPM_Chrg_PwrAbility,
                pEMS->calc_Node[m].fRemain_Batt_Chrg_Pwr_Req, pEMS->calc_Node[m].fMax_Chrg_PwrAbility, pEMS->planSectorNow.nBattGrpPwrLmt, pEMS->fRemain_ACChrgingPwr_Lmt,

                pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr, bEfficient,
                pEMS->calc_Node[m].fMax_Chrg_PwrAbility, pEMS->fTotalPM_Chrg_PwrAbility, fACChrgingPwr_Lmt,
                pEMS->fRemain_ACChrgingPwr_Lmt, pEMS->nNodeNum-m);

            //Step2. Calc Reamin Total AC Power & Remain Battery Charging Power Requirement
            pEMS->calc_Node[m].fRemain_Batt_Chrg_Pwr_Req = 
			    pEMS->calc_Node[m].fRemain_Batt_Chrg_Pwr_Req - pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr;
            pEMS->fRemain_ACChrgingPwr_Lmt = 
			    pEMS->fRemain_ACChrgingPwr_Lmt - pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr;
            
            if(pEMS->fRemain_ACChrgingPwr_Lmt < 0.01)
            {
                pEMS->fRemain_ACChrgingPwr_Lmt = 0; //John 2023-12-06 Modify it.Need Calc All Node each Times.
            }
        
		    TRACE("Dispatch_ESSLmtBattChrg_Power Charge Pwr Dispatch Finished:Node[%d] fRemain_ACChrgingPwr_Lmt=[%.1f]fRemain_Batt_Chrg_Pwr_Req=[%.1f]\n", \
                m,pEMS->fRemain_ACChrgingPwr_Lmt,pEMS->calc_Node[m].fRemain_Batt_Chrg_Pwr_Req);
        }
        else
        {
            pEMS->calc_Node[m].fACDC_Distri_4_BattChrg_Pwr = 0.0;
        }
    }

    // Set fSysACInput_PwrLmt through EMS Management Plan.
    // Added by Jeffery 20250224 for Electra. // 同步到站级EMS 20250524
    if (pEMS->planSectorNow.emBatt_WorkMode == BATT_WORKMODE_FIRST_CHRGING
        && pCCU->blobBackendCfg.emDynamicGridInputLimit_Mode != MODE_DISABLE)
    {
        SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
        float fSysACInput_PwrLmt = 0.0; // New Setting using Grid Power Limit.

        if (pCCU->blobBackendCfg.emDynamicGridInputLimit_Mode == MODE_DYNAMIC_LIMIT)
        {
            fSysACInput_PwrLmt = (float)pCCU->blobEmsCfgPlan.emsLmtSectors_ext[pEMS->nTMSector].nGridInputPwrLmt;
        }
        else if (pCCU->blobBackendCfg.emDynamicGridInputLimit_Mode == MODE_FIXED_LIMIT)
        {
            fSysACInput_PwrLmt = pCCU->blobBackendCfg.fFixed_GridInput_Limit;
        }

        //if (pCCU->blobBackendCfg.fSysACInput_PwrLmt != fSysACInput_PwrLmt)
        if (FLOAT_NOT_EQUAL(pCCU->blobBackendCfg.fSysACInput_PwrLmt, fSysACInput_PwrLmt))
        {
            pCCU->blobBackendCfg.fSysACInput_PwrLmt = fSysACInput_PwrLmt;
            Push_BlobValue(SETTYPE_INTERN_SETT, (HANDLE)pSampler, 1, 26, 
                (void*)&pCCU->blobBackendCfg, sizeof(A_BLOB_ES_BACKEND_CFG), BLOB_TYPE_ES_BACKEND_CFG);
            
            // Auto discharge if overload in charge mode.
            VAR_VALUE varV;
            varV.fValue = fSysACInput_PwrLmt;
            Push_VarValue(SETTYPE_INTERN_SETT, pCCU->hSampler, 1001, 3308, varV);
        }

        float fRealtime_StopSOC = 0;
        fRealtime_StopSOC = (float)pCCU->blobEmsCfgPlan.emsLmtSectors[pEMS->nTMSector].nStopSOC;
        VAR_VALUE varV1;
        varV1.fValue = fRealtime_StopSOC;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3905, varV1);
    }
   
    TRACE("------ End Dispatch_ESSLmtBattChrg_Power ------\n");
    return;
}

static void Refresh_RealTime_MeterValue(ROUGH_CCU_DATA* pCCU)
{
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

    VAR_VALUE varV;
    ERRCDDE_DPR err;

    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;

    //--- Get Grid AC Meter And Cab Meter Value And Aux Meter Value---//
    // LJH241202 SEMS cabMeter
    if (pCCU->lcCfg.emSystemType == SYSTYPE_EMS_STATION)
    {
        if(!pEMS->bCabMeter_SEMS)
        {
            pEMS->f_PcabMeter = pEMS->clientInfoSum.f_PcabMeterSum;
        }
        else
        {
            pEMS->f_PcabMeter = CalcMeterByApplyScenery(pSampler,METER_TYPES_CAB);
        }
    }
    else
    {
        err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 407, 400, &varV, 4); //Cab ACMeter
        pEMS->f_PcabMeter = (err == ERR_DPR_OK) ? varV.fValue : 0; //KW
    }

    // // unit test meter
    // {
    //     /*
    //     fSysACInput_PwrLmt: 1000 PreProt:100 EMSPlan:Idle
    //     emCCUStateSum:0 emPMCtrlModeSum:0
    //     逆流转充电 emACDC_WorkStatuSum:1 fSOCSum:50 f_PcabMeterSum:100 fActvPwr_PCSGrpSum:50 fQ_PCSGrpSum:10 fMax_Chrg_PwrAbilitySum:125 fMax_DisChrg_PwrAbilitySum:120 fChrgedEnergySum:300 fRealTime_MaxCurrSum:136 fACDC_RealTime_VoltSum:860 fACDC_RealTime_CurrSum:100
    //     1.f_PgridMeter:-500 f_PcabMeter:200 OnCharging
    //     2.f_PgridMeter:500  f_PcabMeter:200 OnIdle
    //     过载转放电 emACDC_WorkStatuSum:2 fSOCSum:50 f_PcabMeterSum:-100.0 fActvPwr_PCSGrpSum:-50.0 fQ_PCSGrpSum:10.0 fMax_Chrg_PwrAbilitySum:125.0 fMax_DisChrg_PwrAbilitySum:120.0 fChrgedEnergySum:300.0 fRealTime_MaxCurrSum:100.0 fACDC_RealTime_VoltSum:860.0 fACDC_RealTime_CurrSum:100
    //     1.f_PgridMeter:1500 f_PcabMeter:200 OnDischarging
    //     2.f_PgridMeter:900  f_PcabMeter:200 OnIdle
    //     */

    //     err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 200, 204, &varV, 4); // MPPT_1 Generate KWH
    //     Push_VarValue(SETTYPE_RAW_SP, (HANDLE)pSampler, 408, 400, varV); // f_PgridMeter

    //     err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 200, 205, &varV, 4); // MPPT_2 Generate KWH
    //     Push_VarValue(SETTYPE_RAW_SP, (HANDLE)pSampler, 407, 400, varV); // f_PcabMeter

    //     err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 200, 206, &varV, 4); // MPPT_3 Generate KWH
    //     Push_VarValue(SETTYPE_RAW_SP, (HANDLE)pSampler, 408, 425, varV); // f_PmaxGridDemand

    //     err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 200, 207, &varV, 4); // MPPT_4 Generate KWH
    //     Push_VarValue(SETTYPE_RAW_SP, (HANDLE)pSampler, 409, 400, varV); // f_PauxMeter

    //     // if (pCCU->emPhaseFrom>PHASE_DISABLE && pCCU->emPhaseFrom<=PHASE_C)
    //     // {// LJH250815 added:佳霖其他相缺负载
    //     //     err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 200, 206, &varV, 4); // MPPT_3 Generate KWH
    //     //     Push_VarValue(SETTYPE_RAW_SP, (HANDLE)pSampler, 408, 400+pCCU->emPhaseFrom, varV); // fPhasekW
    //     //     /* err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 200, 207, &varV, 4); // MPPT_4 Generate KWH
    //     //     Push_VarValue(SETTYPE_RAW_SP, (HANDLE)pSampler, 408, 426+pCCU->emPhaseFrom, varV); // fPf */
    //     // }

    //     err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 407, 400, &varV, 4); //Cab ACMeter
    //     pEMS->f_PcabMeter = (err == ERR_DPR_OK) ? varV.fValue : 0; //KW
    // }
    
    // LJH250401 topology#7 一级主控
    if (pCCU->lcCfg.emSystemType == SYSTYPE_EMS_STATION && pCCU->emAuxMeterPos == MULTI_BANCH_ESS_WITHOUT_HVMETER &&
        pCCU->lcCfg.lcType == LC_TYPE_1st && !pEMS->bGMeter_SEMS)
    {
        pEMS->f_PgridMeter = pEMS->clientInfoSum.f_PgridMeterSum;
    }
    else
    {
        if(pCCU->tagGridApplyScenery == SCENERY_ACTIVE_STANDBY)
        {
            pEMS->f_PgridMeter = CalcMeterByApplyScenery(pSampler,METER_TYPES_GRID);
            TRACED("GRID : PriMeterId = [%d], f_PgridMeter == [%.1f]\n", pCCU->nSetPriGridMeter, pEMS->f_PgridMeter);
        }
        else
        {
            err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 408, 400, &varV, 4); //Grid AC Meter
            pEMS->f_PgridMeter = (err == ERR_DPR_OK) ? varV.fValue : 0.0f; //KW
        }
    }

 	err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 408, 425, &varV, 4); //Grid AC Meter Pmax Demand Power
    pEMS->f_PmaxGridDemand = (err == ERR_DPR_OK) ? varV.fValue : 0; //KW

    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 409, 400, &varV, 4); //Aux1 AC Meter Power
    pEMS->f_PauxMeter = (err == ERR_DPR_OK) ? varV.fValue : 0; //KW

    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 410, 400, &varV, 4); //Aux3 AC Meter Power
    pEMS->f_PauxMeter_3 = (err == ERR_DPR_OK) ? varV.fValue : 0; //KW
    
    //--- Added By John 2023-12-06 ---//
    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 408, 404, &varV, 4); //Grid Qac_Meter
    pEMS->f_QgridMeter = (err == ERR_DPR_OK) ? varV.fValue : 0; //KW
    
    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 407, 404, &varV, 4); //Cab Qac_Meter
    pEMS->f_QcabMeter = (err == ERR_DPR_OK) ? varV.fValue : 0; //KW

    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 409, 404, &varV, 4); //Aux1 Qac_Meter
	pEMS->f_QauxMeter = (err == ERR_DPR_OK) ? varV.fValue : 0; //KW

    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 410, 404, &varV, 4); //Aux3 Qac_Meter
	pEMS->f_QauxMeter_3 = (err == ERR_DPR_OK) ? varV.fValue : 0; //KW

    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 1, 47, &varV, 4); //Priority
    pEMS->nPrio = MIN(1, varV.ulValue);

    TRACED("Refresh_RealTime_MeterValue f_PgridMeter=%.1f bCabMeter_SEMS:%d f_PcabMeter=%.1f f_PauxMeter=%.1f f_PauxMeter_3=[%.1f] f_PmaxGridDemand=%.1f "
        "f_QgridMeter=[%.1f] f_QcabMeter=[%.1f], f_QauxMeter=[%.1f], f_QauxMeter_3=[%.1f] nPrio=[%d]\n",
        pEMS->f_PgridMeter, pEMS->bCabMeter_SEMS, pEMS->f_PcabMeter, pEMS->f_PauxMeter, pEMS->f_PauxMeter_3, pEMS->f_PmaxGridDemand,
        pEMS->f_QgridMeter, pEMS->f_QcabMeter, pEMS->f_QauxMeter, pEMS->f_QauxMeter_3 , pEMS->nPrio);
    return;
}

// LJH240705 AitiOverLoad HVMeter 众树
static BOOL Need_AntiOverloadHV(ROUGH_CCU_DATA* pCCU)
{
    return (AUXMETER_POS_BRANCH_TRANSFER == pCCU->emAuxMeterPos && pCCU->blobBackendCfg.fSysACInput_PwrLmtHV > 0.1);
}

static void JudgeIfEnterAnti_RefluxOrOverload(ROUGH_CCU_DATA* pCCU, float fAnti_RefluxMeter, float fAnti_OverLoadMeter)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    time_t tm_Now = _NOW_;
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
    static BOOL bLastAntiReflux = FALSE;

    VAR_VALUE varV;
    ERRCDDE_DPR err;
    err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 200, 225, &varV, 4);
    pEMS->fAllowedReverseValue = (err == ERR_DPR_OK) ? varV.fValue : 0.0f; //KW 奥地利认证 逆流值可设置
    pEMS->fAtRflx_kW = fAnti_RefluxMeter;
	pEMS->fAtOvl_kW =  fAnti_OverLoadMeter;
    BOOL bAntiReflux =   (fAnti_RefluxMeter - pCCU->blobBackendCfg.fPreProt4RefluxAndOverLoad < (pEMS->fAllowedReverseValue * -1.0f));
    BOOL bAntiOverLoad = ((pCCU->blobBackendCfg.fSysACInput_PwrLmt - fAnti_OverLoadMeter) <
                            pCCU->blobBackendCfg.fPreProt_Chrg);
    // //奥地利认证：进入防逆流下发到模块进入硬件降载模式
    // if (bAntiReflux != bLastAntiReflux)
    // {
    //     if(bAntiReflux) // 从0变为1：进入硬件降载模式
    //     {
    //         varV.emValue = 1;
    //         Push_VarValue(SETTYPE_INTERN_SETT, (HANDLE)pSampler, 701, 91011, varV);
    //         TRACED("进入硬件降载模式,bAntiReflux%d变为%d", bLastAntiReflux, bAntiReflux);
    //     }
    //     else // 从1变为0：退出硬件降载模式
    //     {
    //         varV.emValue = 0;
    //         Push_VarValue(SETTYPE_INTERN_SETT, (HANDLE)pSampler, 701,  91011, varV);
    //         TRACED("退出硬件降载模式,bAntiReflux%d变为%d", bLastAntiReflux, bAntiReflux);
    //     }
    //     // 更新上一次状态为当前状态
    //     bLastAntiReflux = bAntiReflux;
    // }

    TRACED("JudgeIfEnterAnti_RefluxOrOverload fPacload4Chg:%.1f fPacload4Dischg:%.1f bAntiReflux:%d fAtRflx_kW:%.1f "
            "bAntiOverLoad:%d fAtOvl_kW:%.1f fSysACInput_PwrLmt:%.1f fPreProt_Chrg:%.1f fPreProt4RefluxAndOverLoad:%.1f 
            fAllowedReverseValue:%.1f",
            pEMS->fPacload4Chg, pEMS->fPacload4Dischg, bAntiReflux, pEMS->fAtRflx_kW,
            bAntiOverLoad, pEMS->fAtOvl_kW, pCCU->blobBackendCfg.fSysACInput_PwrLmt,
            pCCU->blobBackendCfg.fPreProt_Chrg, pCCU->blobBackendCfg.fPreProt4RefluxAndOverLoad, pEMS->fAllowedReverseValue);
    // LJH250222 fTotal_PessChgAvaliable fTotal_PessDischgLmt不再判断emBatt_WorkModeReq
    // 防逆流
    if(pEMS->bEnterAnti_Reflux == TRUE)
    {
        if (((ABS(tm_Now - pEMS->tm_LastEnterReflux) > MAX_RETDIFF_PERIOD) && !bAntiReflux) ||
            bAntiOverLoad)
        {
            pEMS->bEnterAnti_Reflux = FALSE;
            TRACE("111 tm_Now=[%ld] pEMS->bEnterAnti_Reflux=[%d] fAnti_RefluxMeter=[%.1f]\n", \
                tm_Now, pEMS->bEnterAnti_Reflux, fAnti_RefluxMeter);
        }
    }
    else
    {
        if(bAntiReflux) //Pre-Sett Value,Due To EMS Adjust Power is Too Slow.
        {
            pEMS->bEnterAnti_Reflux = TRUE;
            pEMS->tm_LastEnterReflux = tm_Now;
            TRACEE("JudgeIfEnterAnti_RefluxOrOverload bEnterAnti_Reflux fAnti_RefluxMeter:%.1f<Prot:%.1f",
                fAnti_RefluxMeter, pCCU->blobBackendCfg.fPreProt4RefluxAndOverLoad);
        }
    }
        
    // LJH241007 AntiPhase 单相防逆流
    if (pCCU->needAntiReflux_Phase)
    {
        if(pEMS->bEnterAnti_Reflux_Phase)
        {
            if(((ABS(tm_Now - pEMS->tm_LastEnterReflux_Phase)) > MAX_RETDIFF_PERIOD) && 
                (pEMS->fAtRflxMtr_kWPhMin >= pCCU->blobBackendCfg.fPreProt4Reflux_Phase))  
            {
                pEMS->bEnterAnti_Reflux_Phase = FALSE;
                static time_t tmLogOutAitiRefluxPhase_Exit = 0;
                if (ElapseS(tmLogOutAitiRefluxPhase_Exit) > MAX_RETDIFF_PERIOD)
                {
                    tmLogOutAitiRefluxPhase_Exit = _NOW_;
                    LogOut(MAIN_MODULE_SAMPLER, "CCU_SAMP", LOG_TYPE_INFO, "Exit Reflux Phase:%.1f %.1f %.1f min:%.1f Prot:%.1f",
                        pEMS->fAtRflxMtr_kWPhs[0], pEMS->fAtRflxMtr_kWPhs[1], pEMS->fAtRflxMtr_kWPhs[2],
                        pEMS->fAtRflxMtr_kWPhMin, pCCU->blobBackendCfg.fPreProt4Reflux_Phase);
                }
            }
        }
        else
        {
            if(pEMS->fAtRflxMtr_kWPhMin < pCCU->blobBackendCfg.fPreProt4Reflux_Phase)
            {
                pEMS->bEnterAnti_Reflux_Phase = TRUE;
                pEMS->tm_LastEnterReflux_Phase = tm_Now;
                static time_t tmLogOutAitiRefluxPhase = 0;
                if (ElapseS(tmLogOutAitiRefluxPhase) > MAX_RETDIFF_PERIOD)
                {
                    tmLogOutAitiRefluxPhase = _NOW_;
                    LogOut(MAIN_MODULE_SAMPLER, "CCU_SAMP", LOG_TYPE_WARING, "Enter Reflux Phase:%.1f %.1f %.1f min:%.1f Prot:%.1f",
                        pEMS->fAtRflxMtr_kWPhs[0], pEMS->fAtRflxMtr_kWPhs[1], pEMS->fAtRflxMtr_kWPhs[2],
                        pEMS->fAtRflxMtr_kWPhMin, pCCU->blobBackendCfg.fPreProt4Reflux_Phase);
                }
            }
        }
    }
    else
    {
        pEMS->bEnterAnti_Reflux_Phase = FALSE;
    }

    // 防过载 
    if(pEMS->bEnter_OverLoad)
    {
        // LJH0303 Add || bAntiReflux
        if (((ABS(tm_Now - pEMS->tm_LastEnterOverLoad) > MAX_RETDIFF_PERIOD) && !bAntiOverLoad) ||
            bAntiReflux)
        {
            pEMS->bEnter_OverLoad = FALSE;
            TRACE("333 tm_Now=[%ld] pEMS->bEnter_OverLoad=[%d] fAnti_OverLoadMeter=[%.1f]\n", \
                tm_Now, pEMS->bEnter_OverLoad, fAnti_OverLoadMeter);
        }
    }
    else
    {
        if (bAntiOverLoad)
        {
            pEMS->bEnter_OverLoad = TRUE;
            pEMS->tm_LastEnterOverLoad = tm_Now;
            TRACEE("JudgeIfEnterAnti_RefluxOrOverload bEnter_OverLoad fAnti_OverLoadMeter:%.1f", fAnti_OverLoadMeter);
        }
    }
    // LJH241007 AntiPhase 单相防过载
    if (pCCU->needAntiOverLoad_Phase &&
        pCCU->blobBackendCfg.fMaxChrgCurLmt_Phase>CHRGCURLMT_PHASE_VALID)
    {
        if(pEMS->bEnter_OverLoad_Phase)
        {
            if((ABS(tm_Now - pEMS->tm_LastEnterOverLoad_Phase) > MAX_RETDIFF_PERIOD) && 
                (pEMS->fAtOvlMtr_APhMax < pCCU->blobBackendCfg.fMaxChrgCurLmt_Phase))
            {
                pEMS->bEnter_OverLoad_Phase = FALSE;
                static time_t tmLogOutAitiOverLoadPhase_Exit = 0;
                if (ElapseS(tmLogOutAitiOverLoadPhase_Exit) > MAX_RETDIFF_PERIOD)
                {
                    tmLogOutAitiOverLoadPhase_Exit = _NOW_;
                    LogOut(MAIN_MODULE_SAMPLER, "CCU_SAMP", LOG_TYPE_INFO, "Exit OverLoad Phase:%.1f %.1f %.1f max:%.1f MaxCur:%.1f",
                        pEMS->fAtOvlMtr_APhs[0], pEMS->fAtOvlMtr_APhs[1], pEMS->fAtOvlMtr_APhs[2],
                        pEMS->fAtOvlMtr_APhMax, pCCU->blobBackendCfg.fMaxChrgCurLmt_Phase);
                }
            }
        }
        else
        {
            if(pEMS->fAtOvlMtr_APhMax >= pCCU->blobBackendCfg.fMaxChrgCurLmt_Phase)
            {
                pEMS->bEnter_OverLoad_Phase = TRUE;
                pEMS->tm_LastEnterOverLoad_Phase = tm_Now;
                static time_t tmLogOutAitiOverLoadPhase = 0;
                if (ElapseS(tmLogOutAitiOverLoadPhase) > MAX_RETDIFF_PERIOD)
                {
                    tmLogOutAitiOverLoadPhase = _NOW_;
                    LogOut(MAIN_MODULE_SAMPLER, "CCU_SAMP", LOG_TYPE_WARING, "Enter OverLoad Phase:%.1f %.1f %.1f max:%.1f MaxCur:%.1f",
                        pEMS->fAtOvlMtr_APhs[0], pEMS->fAtOvlMtr_APhs[1], pEMS->fAtOvlMtr_APhs[2],
                        pEMS->fAtOvlMtr_APhMax, pCCU->blobBackendCfg.fMaxChrgCurLmt_Phase);
                }
            }
        }
    }
    else
    {
        pEMS->bEnter_OverLoad_Phase = FALSE;
    }

    // LJH240705 AitiOverLoad HVMeter 240822:跟随高压表最大需量+OA(防超需量模式) LJH250710 added正向有功总最大需量使能
    if ( Need_AntiOverloadHV(pCCU) || pCCU->emPmaxDemandMode == 1)
    {
        float fPdemandHVMax = MAX(pCCU->blobBackendCfg.fSysACInput_PwrLmtHV, pEMS->f_PmaxGridDemand);
        fPdemandHVMax =       MAX(pCCU->blobBackendCfg.fPfactory_SettDemand, fPdemandHVMax);
        if(pEMS->bEnter_OverLoadHV)
        {
            if ((ABS(tm_Now - pEMS->tm_LastEnterOverLoadHV) > MAX_RETDIFF_PERIOD) &&
                ((fPdemandHVMax - pEMS->f_PgridMeter) >= pCCU->blobBackendCfg.fPreProt_Chrg))
            {
                // LJH240705 AitiOverLoad HVMeter
                pEMS->bEnter_OverLoadHV = FALSE;
                TRACE("Need_AntiOverloadHV 333 tm_Now=[%ld] pEMS->bEnter_OverLoadHV=[%d] f_PgridMeter=[%.1f]\n", \
                    tm_Now, pEMS->bEnter_OverLoadHV, pEMS->f_PgridMeter);
            }
        }
        else
        {
            // LJH240705 AitiOverLoad HVMeter
            if ((fPdemandHVMax - pEMS->f_PgridMeter) < pCCU->blobBackendCfg.fPreProt_Chrg)
            {
                pEMS->bEnter_OverLoadHV = TRUE;
                pEMS->tm_LastEnterOverLoadHV = tm_Now;
                static time_t tmLogOutAitiOverLoadHV = 0;
                if (ElapseS(tmLogOutAitiOverLoadHV) > MAX_RETDIFF_PERIOD)
                {
                    tmLogOutAitiOverLoadHV = _NOW_;
                    LogOut(MAIN_MODULE_SAMPLER, "CCU_SAMP", LOG_TYPE_WARING, "OverDemand GkW:%.1f Gdemand:%.1f InitDemand:%.1f",
                        pEMS->f_PgridMeter, pEMS->f_PmaxGridDemand, pCCU->blobBackendCfg.fSysACInput_PwrLmtHV);
                }
            }
        }
    }
    else
    {
        pEMS->bEnter_OverLoadHV = FALSE;
    }
}

// LJH241007 AntiPhase
static void Refresh_RealTime_MeterValue_Phase(ROUGH_CCU_DATA* pCCU, PHASE_TYPE type, int nDevId)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
    VAR_VALUE varV;
    ERRCDDE_DPR err;
    int n = 0;
    switch (type)
    {
    case PHASE_TYPE_POWER:
        {
            pEMS->fAtRflxMtr_kWPhMin = MAX_INVALID_FLOAT;
            err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, nDevId, 426, &varV, 4);
            pEMS->fAtRflxMtr_Factor = (err == ERR_DPR_OK) ? varV.fValue : 1.0f;
            for (n=0; n<MAX_PHASE_NUM; n++)
            {
                if (pEMS->isAtRflxMtrHV && nDevId == 408)
                {
                    if (n == 1)
                    {
                        // 高压表B相不接
                        pEMS->fAtRflxMtr_kWPhs[n] = 0.0f;
                        continue;
                    }
                }
                err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, nDevId, 401+n, &varV, 4);
                pEMS->fAtRflxMtr_kWPhs[n] = (err == ERR_DPR_OK) ? varV.fValue : 0.0f;
                pEMS->fAtRflxMtr_kWPhMin = MIN(pEMS->fAtRflxMtr_kWPhMin, pEMS->fAtRflxMtr_kWPhs[n]);
            }
        }
        break;
        
    case PHASE_TYPE_VA:
        {
            pEMS->fAtOvlMtr_VPhMin = MAX_INVALID_FLOAT;
            err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, nDevId, 426, &varV, 4);
            pEMS->fAtOvlMtr_Factor = (err == ERR_DPR_OK) ? varV.fValue : 1.0f;

            pEMS->fAtOvlMtr_APhMax = MIN_INVALID_FLOAT;
            for (n=0; n<MAX_PHASE_NUM; n++)
            {
                err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, nDevId, 413+2*n, &varV, 4);
                pEMS->fAtOvlMtr_VPhs[n] = (err == ERR_DPR_OK) ? varV.fValue : 0.0f;
                pEMS->fAtOvlMtr_VPhMin = MIN(pEMS->fAtOvlMtr_VPhMin, pEMS->fAtOvlMtr_VPhs[n]);

                err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, nDevId, 414+2*n, &varV, 4);
                pEMS->fAtOvlMtr_APhs[n] = (err == ERR_DPR_OK) ? varV.fValue : 0.0f;
                
                pEMS->fAtOvlMtr_APhMax = MAX(pEMS->fAtOvlMtr_APhMax, pEMS->fAtOvlMtr_APhs[n]);
            }
            pEMS->fAtOvlMtr_VPhMin = (pEMS->fAtOvlMtr_VPhMin > 999999 ? 0 : pEMS->fAtOvlMtr_VPhMin);
            pEMS->fAtOvlMtr_APhMax = (pEMS->fAtOvlMtr_APhMax < 0 ?     0 : pEMS->fAtOvlMtr_APhMax);
        }
        break;

    default:
        break;
    }
    // // unit test meter phase
    // {
    //     /*
    //     f_PgridMeter:80 f_PcabMeter:75
    //     fAtOvlMtr_VPhMin: 220 fAtOvlMtr_APhMax:2
    //     */
    //     err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 200, 206, &varV, 4); // MPPT_3 Generate KWH
    //     pEMS->fAtOvlMtr_VPhMin = (err == ERR_DPR_OK) ? varV.fValue : 0;

    //     err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 200, 207, &varV, 4); // MPPT_4 Generate KWH
    //     pEMS->fAtOvlMtr_APhMax = (err == ERR_DPR_OK) ? varV.fValue : 0;

    //     pEMS->fAtOvlMtr_Factor = 1;
    // }
}

/**      |Grid  #1 未安装 新巨能
*       | |电网侧关口表 Grid Ac Meter (防逆流+防过载，单相防逆流+单相防过载)
*        |
*——————————————————————————
*    |            | | 储能系统关口表 Cab AC Meter
*    |             |
*    AC Load      ESS
*/ 
static void calcOverLoadAndReflux_By_Grid2CabMeter(ROUGH_CCU_DATA* pCCU)
{
    TRACE("---calcOverLoadAndReflux_By_Grid2CabMeter---\n");
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;

    pEMS->fPacload4Dischg   = 
        pEMS->f_PgridMeter - pEMS->f_PcabMeter;

    pEMS->fPacload4Chg      =  
        pEMS->f_PgridMeter - pEMS->f_PcabMeter;

    pEMS->fQacload4Org      = 
        pEMS->f_QgridMeter - pEMS->f_QcabMeter;
    if (pCCU->needAntiReflux_Phase)
    {
        Refresh_RealTime_MeterValue_Phase(pCCU, PHASE_TYPE_POWER, 408);
    }
    if (pCCU->needAntiOverLoad_Phase &&
        pCCU->blobBackendCfg.fMaxChrgCurLmt_Phase>CHRGCURLMT_PHASE_VALID)
    {
        Refresh_RealTime_MeterValue_Phase(pCCU, PHASE_TYPE_VA, 408);
    }

    JudgeIfEnterAnti_RefluxOrOverload(pCCU, pEMS->f_PgridMeter, pEMS->f_PgridMeter);

    return;
}


/**      |Grid  #2 低压变压器关口表 众树 松下
*       | |电网侧关口表 Grid Ac Meter(用于防逆流, 单相防逆流) 高压表 // LJH240705 防超需量
*        |
*————————————————————————————————
*                      | |电表2-支路变压器关口表(用于防过载，单相防过载) 辅助表
*       ________________|____________
*           |             |
*           |            | | 储能系统关口表 Cab AC Meter
*           |             |
*       AC Load         ESS
*/ 
static void calcOverLoadAndReflux_By_Grid2Cab_BranchMeter(ROUGH_CCU_DATA* pCCU)
{
    TRACE("---calcOverLoadAndReflux_By_Grid2Cab_BranchMeter---\n");
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;

    pEMS->fPacload4Dischg   = 
        pEMS->f_PgridMeter - pEMS->f_PcabMeter; //Used For Anti-ReFlux

    pEMS->fPacload4Chg      = 
        pEMS->f_PauxMeter - pEMS->f_PcabMeter; //Used For Overload

    pEMS->fQacload4Org      = 
            pEMS->f_QauxMeter - pEMS->f_QcabMeter; //Used For Overload

    if (pCCU->needAntiReflux_Phase)
    {
        Refresh_RealTime_MeterValue_Phase(pCCU, PHASE_TYPE_POWER, 408);
    }
    if (pCCU->needAntiOverLoad_Phase &&
        pCCU->blobBackendCfg.fMaxChrgCurLmt_Phase>CHRGCURLMT_PHASE_VALID)
    {
        Refresh_RealTime_MeterValue_Phase(pCCU, PHASE_TYPE_VA, 409);
    }
    
    JudgeIfEnterAnti_RefluxOrOverload(pCCU, pEMS->f_PgridMeter, pEMS->f_PauxMeter);

    return;
}

/**      |Grid  #3 民用电关口表
*       | |电网侧关口表 Grid Ac Meter(用于防逆流) 
*        |
*——————————————————————————————————————————————————————————   
*           |                    |                       |
*          | | AuxMeter         | | 储能系统关口表       |
*           |                    |  Cab AC Meter         |
*       Civil AC Load           ESS                     Industry ACLoad
*/ 
static void calcOverLoadAndReflux_By_Grid2Cab_CivilMeter(ROUGH_CCU_DATA* pCCU)
{
    TRACE("---calcOverLoadAndReflux_By_Grid2Cab_CivilMeter---\n");
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;

    pEMS->fPacload4Dischg   = 
        pEMS->f_PgridMeter - pEMS->f_PcabMeter - pEMS->f_PauxMeter; //Used For Anti-ReFlux

    pEMS->fPacload4Chg      = 
        pEMS->f_PgridMeter - pEMS->f_PcabMeter; //Used For Overload

    pEMS->fQacload4Org      = 
        pEMS->f_QgridMeter - pEMS->f_QcabMeter; //Used For Q_Chg Distrribute to Avoid OverLoad.
    pCCU->needAntiReflux_Phase = FALSE;
    pCCU->needAntiOverLoad_Phase = FALSE;
    
    JudgeIfEnterAnti_RefluxOrOverload(pCCU, pEMS->f_PgridMeter, pEMS->f_PgridMeter);

    return;
}


/**      |Grid  #4 工业用电关口表
*       | |电网侧关口表 Grid AC Meter(用于防逆流) 
*        |
*————————————————————————————————————————————————————— 
*           |                          |                       
*          | | ACLoad                 | | 储能系统关口表        
*           |  关口表                  |  Cab AC Meter         
*        AC Load                      ESS                     
*/ 
static void calcOverLoadAndReflux_By_Grid2Cab_ACLoadMeter(ROUGH_CCU_DATA* pCCU)
{
    TRACE("---calcOverLoadAndReflux_By_Grid2Cab_ACLoadMeter---\n");
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;

    pEMS->fPacload4Dischg   = pEMS->f_PauxMeter; //Used For Dischg
    pEMS->fPacload4Chg      = pEMS->f_PauxMeter; //Used For Overload

    pEMS->fQacload4Org      = pEMS->f_QauxMeter; //Used For Q_Chg Distrribute to Avoid OverLoad.
    pCCU->needAntiReflux_Phase = FALSE;
    pCCU->needAntiOverLoad_Phase = FALSE;
    
    JudgeIfEnterAnti_RefluxOrOverload(pCCU, pEMS->f_PgridMeter, pEMS->f_PgridMeter);

    return;
}

/**             #5 多负载支路关口表 惠州-深新-华亿泰-储能 浙江杭州-朗新-储能
*——————————————————————————————————————————————————————————————10KV
*    |                   | |电网侧关口表 Grid AC Meter(用于防过载，单相防过载)|
*    |        ____________|_____________                       | |电表3
*   | |电表1    |             |                                 |  
*    |          |            | | 储能系统关口表 Cab AC Meter     |
*    |          |             |                                 |
*    ACLoad     AC Load        ESS                            ACLoad
*/ 
static void calcOverLoadAndReflux_By_MultiBranch_ACLoadMeter(ROUGH_CCU_DATA* pCCU)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;

    pEMS->fPacload4Dischg   = 
        pEMS->f_PgridMeter - pEMS->f_PcabMeter + pEMS->f_PauxMeter + pEMS->f_PauxMeter_3;  

    pEMS->fPacload4Chg      = 
        pEMS->f_PgridMeter - pEMS->f_PcabMeter; //Used For Overload
    
    pEMS->fQacload4Org      = 
        pEMS->f_QgridMeter - pEMS->f_QcabMeter; //Used For Q_Chg Distrribute to Avoid OverLoad.
    pCCU->needAntiReflux_Phase = FALSE;
    if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_CHRGING && pCCU->needAntiOverLoad_Phase &&
        pCCU->blobBackendCfg.fMaxChrgCurLmt_Phase>CHRGCURLMT_PHASE_VALID)
    {
        Refresh_RealTime_MeterValue_Phase(pCCU, PHASE_TYPE_VA, 408);
    }
 
    JudgeIfEnterAnti_RefluxOrOverload(pCCU, pEMS->fPacload4Dischg, pEMS->f_PgridMeter);

    return;
}

static void Calc_ACLoadPwrByDiffMeterPosition(ROUGH_CCU_DATA* pCCU)
{
    //POS_AUXMETER   emPosAux = AUXMETER_NOT_INSTALLED;//Need Get From System config
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    pEMS->fAtRflx_kW = 0;
	pEMS->fAtOvl_kW =  0;
    switch (pCCU->emAuxMeterPos)
    {
    // LJH250401 added top#6
    case AUXMETER_NOT_INSTALLED: case MULTI_BANCH_ESS_WITH_HVMETER:
        calcOverLoadAndReflux_By_Grid2CabMeter(pCCU); 
        break;

    case AUXMETER_POS_BRANCH_TRANSFER:
        calcOverLoadAndReflux_By_Grid2Cab_BranchMeter(pCCU); 
        break;

    case AUXMETER_POS_CIVIL_ELECTRIC:
        calcOverLoadAndReflux_By_Grid2Cab_CivilMeter(pCCU); 
        break;

    case AUXMETER_POS_ACLOAD:
        calcOverLoadAndReflux_By_Grid2Cab_ACLoadMeter(pCCU); 
        break;
    // LJH250401 added top#7
    case AUXMETER_MULTI_BANCH: case MULTI_BANCH_ESS_WITHOUT_HVMETER:
        calcOverLoadAndReflux_By_MultiBranch_ACLoadMeter(pCCU); 
        break;
    default:
        calcOverLoadAndReflux_By_Grid2CabMeter(pCCU); 
        break;
    }

    // LJH250222 delete 因为光伏逆流时是负数
    // pEMS->fPacload4Dischg = MAX(0.0f, pEMS->fPacload4Dischg); //Used For Dischg
    // pEMS->fPacload4Chg = MAX(0.0f, pEMS->fPacload4Chg); //Used For Overload
    
    pEMS->fDirect   = (pEMS->fQacload4Org >= 0) ? -1.0 : 1.0; 
    pEMS->fQacload4Org  = (float)fabs(pEMS->fQacload4Org);//??? Need Judge Output Q_ReactivePower Type
 
    TRACE("Calc_ACLoadPwrByDiffMeterPosition fPacload4Dischg=[%f] fPacload4Chg=[%f] emAuxMeterPos=[%d]",
        pEMS->fPacload4Dischg, pEMS->fPacload4Chg, pCCU->emAuxMeterPos);

    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Calc_ESS_ChgLmtPower
*  CREATOR       :  
*  CREATED DATE  : 2023-02-26 11:13:53
*  DESCRIPTION   : 计算储能系统的充电功率限制值
*					
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Calc_ESS_ChgLmtPower(ROUGH_CCU_DATA* pCCU)
{   
    // SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    float fProt = pCCU->blobBackendCfg.fPreProt_Chrg; // LJH250703 分开充放电保护/跟随比(充电)
    // LJH250205 REMS 没有管理计划也要先计算，REMS后面需要防过载
    // if(pEMS->nTMSector == -1)
    // {
    //     pEMS->fTotal_PessChgAvaliable = 0;
    //     return;
    // }
    //Max Limit Power
    pEMS->fGridACInputPwr_Lmt = MIN(pCCU->blobBackendCfg.fEnter_UrgetDischrgPwr, pCCU->blobBackendCfg.fSysACInput_PwrLmt);

    //Added By John For PmaxDemand Mgmt.20230425
    // ->Start->Read Psett_Demand->Read Pgrid->Calc Pchg_Avaliable
    if(pCCU->emPmaxDemandMode == 1) 
    {
        float fPdemand = MAX(pEMS->f_PmaxGridDemand, pCCU->blobBackendCfg.fPfactory_SettDemand);
        pEMS->fGridACInputPwr_Lmt = MIN(pEMS->fGridACInputPwr_Lmt, fPdemand);
        TRACED("Calc_ESS_ChgLmtPower fPdemand=[%.1f] f_PmaxGridDemand=[%.1f] fPfactory_SettDemand=[%.1f] "
            "fGridACInputPwr_Lmt:%.1f fSysACInput_PwrLmt:%.1f",
            fPdemand, pEMS->f_PmaxGridDemand, pCCU->blobBackendCfg.fPfactory_SettDemand,
            pEMS->fGridACInputPwr_Lmt, pCCU->blobBackendCfg.fSysACInput_PwrLmt);
    }
    // LJH241007 AntiPhase
#ifdef UNIT_TEST_AntiPhase
    s_fTotal_PessChgAvaliable = pEMS->fTotal_PessChgAvaliable;
#endif
    if (pCCU->needAntiOverLoad_Phase && pCCU->blobBackendCfg.fMaxChrgCurLmt_Phase>CHRGCURLMT_PHASE_VALID)
    {
        pEMS->fMaxPwr4Chrg_SumPhs = 
                ABS((((pEMS->fAtOvlMtr_VPhMin) * (pCCU->blobBackendCfg.fMaxChrgCurLmt_Phase) * (pEMS->fAtOvlMtr_Factor)) * 3.0f) / 1000.0f);
        pEMS->fGridACInputPwr_Lmt = MIN(pEMS->fGridACInputPwr_Lmt, pEMS->fMaxPwr4Chrg_SumPhs);
        TRACEI("Calc_ESS_ChgLmtPower needAntiOverLoad_Phase fGridACInputPwr_Lmt:%.1f fMaxPwr4Chrg_SumPhs:%.1f V:%.4f * I:%.4f * pf:%.4f*3/1000",
            pEMS->fGridACInputPwr_Lmt, pEMS->fMaxPwr4Chrg_SumPhs,
            pEMS->fAtOvlMtr_VPhMin, pCCU->blobBackendCfg.fMaxChrgCurLmt_Phase, pEMS->fAtOvlMtr_Factor);
    }
    
    // LJH240705 AitiOverLoad HVMeter 240822:跟随高压表最大需量+OA(防超需量模式)
    pEMS->fRemainPwr4Chrg = pEMS->fGridACInputPwr_Lmt - pEMS->fPacload4Chg; // 剩余充电功率
    pEMS->fRemainDemand4Chrg = 0.0f;
    // BOOL bFromHV = FALSE; // LJH250710 fix:众树超需量
    if (Need_AntiOverloadHV(pCCU))
    {
        // 当前电表最大需量 vs 设置的需量初始值
        float fPdemandHV = MAX(pEMS->f_PmaxGridDemand, pCCU->blobBackendCfg.fSysACInput_PwrLmtHV);
        // LJH250401 fix old pEMS->fRemainDemand4Chrg = fPdemandHV - pEMS->f_PgridMeter;
        pEMS->fRemainDemand4Chrg = fPdemandHV - pEMS->f_PgridMeter + pEMS->f_PcabMeter; // 剩余需量
        // if (pEMS->fRemainDemand4Chrg > 0 && pEMS->fRemainDemand4Chrg < pEMS->fRemainPwr4Chrg)
        // {
        //     bFromHV = TRUE;
        // }
        pEMS->fGridRemainACPwr2Chrg = MIN(pEMS->fRemainDemand4Chrg, pEMS->fRemainPwr4Chrg);
        TRACEI("Calc_ESS_ChgLmtPower Need_AntiOverloadHV f_PmaxGridDemand=[%.1f] fRemainDemand4Chrg=[%.1f] %.1f-%.1f+%.1f "
            "fGridRemainACPwr2Chrg:%.1f fRemainPwr4Chrg:%.1f",
            pEMS->f_PmaxGridDemand, pEMS->fRemainDemand4Chrg, fPdemandHV, pEMS->f_PgridMeter, pEMS->f_PcabMeter,
            pEMS->fGridRemainACPwr2Chrg, pEMS->fRemainPwr4Chrg);
    }
    else
    {
        pEMS->fGridRemainACPwr2Chrg = pEMS->fRemainPwr4Chrg;
    }
    // old pEMS->fGridRemainACPwr2Chrg = MAX(0.0, pEMS->fGridRemainACPwr2Chrg*0.9);
    // LJH250303 Modify // LJH250703 分开充放电保护/跟随比
    pEMS->fGridRemainACPwr2Chrg =
        MAX(0.0f, (pEMS->fGridRemainACPwr2Chrg - fProt) * pCCU->blobBackendCfg.PowerRate_Chrg);

    //old added By John. 2023/07/21 PCS is DC Adjust Consider 0.02 Efficient Lost; // LJH250205 REMS
    // float fActvPwrChgLmt;
    // if(pEMS->nTMSector == -1)
    // {
    //     fActvPwrChgLmt = MAX_PWR_LMT;
    // }
    // else
    // {
    //     pEMS->planSectorNow.nActvPwrLmt
    //     fActvPwrChgLmt = (float)pEMS->planSectorNow.nActvPwrLmt;
    // }

    // // LJH240705 AitiOverLoad HVMeter 240822:跟随高压表最大需量+OA(防超需量模式)
    // pEMS->fTotal_PessChgAvaliable = MIN(fActvPwrChgLmt, pEMS->fGridRemainACPwr2Chrg);

    // LJH250910 modified
    float fActvPwrChgLmt = (float)pEMS->planSectorNow.nActvPwrLmt;
    pEMS->fTotal_PessChgAvaliable = MIN(fActvPwrChgLmt, pEMS->fGridRemainACPwr2Chrg);
    TRACED("Calc_ESS_ChgLmtPower fTotal_PessChgAvaliable:%.1f MIN(fActvPwrChgLmt:%.1f, fGridRemainACPwr2Chrg:%.1f) "
        "fGridRemainACPwr2Chrg=(fRemainPwr4Chrg:%.1f-fProt:%.1f)*rate:%.1f fRemainPwr4Chrg=fGridACInputPwr_Lmt:%.1f-fPacload4Chg:%.1f",
        pEMS->fTotal_PessChgAvaliable, fActvPwrChgLmt, pEMS->fGridRemainACPwr2Chrg, 
        pEMS->fRemainPwr4Chrg, fProt, pCCU->blobBackendCfg.PowerRate_Chrg, pEMS->fGridACInputPwr_Lmt, pEMS->fPacload4Chg);
    // LJH241007 AntiPhase add bEnter_OverLoad_Phase
    if (pEMS->bEnter_OverLoad)
    {
        // old pEMS->fTotal_PessChgAvaliable *= 0.8f;
        // LJH250303 Added 分级处理
        float fRemain = pCCU->blobBackendCfg.fSysACInput_PwrLmt - pEMS->fAtOvl_kW;
        float fHalfProt = fProt / 2.0f;
        TRACED("Calc_ESS_ChgLmtPower fTotal_PessChgAvaliableOld:%.1f fRemain:%.1f fHalfProt:%.1f", pEMS->fTotal_PessChgAvaliable, fRemain, fHalfProt);
        if (fRemain <= 0)
        {
            pEMS->fTotal_PessChgAvaliable *= 0.8f;
            TRACEE("Calc_ESS_ChgLmtPower *= 0.8 fTotal_PessChgAvaliable:%.1f fRemain:%.1f fProt:%.1f", pEMS->fTotal_PessChgAvaliable, fRemain, fProt);
        }
        else if (fRemain < fHalfProt)
        {
            pEMS->fTotal_PessChgAvaliable -= fProt;
            TRACEW("Calc_ESS_ChgLmtPower -= fProt fTotal_PessChgAvaliable:%.1f fRemain:%.1f fProt:%.1f", pEMS->fTotal_PessChgAvaliable, fRemain, fProt);
        }
        else
        {
            fRemain = MAX((fProt-fRemain), 0); // LJH250726 Added
            pEMS->fTotal_PessChgAvaliable -= fRemain;
            TRACEI("Calc_ESS_ChgLmtPower -= (%.1f) fTotal_PessChgAvaliable:%.1f fProt:%.1f", fRemain, pEMS->fTotal_PessChgAvaliable, fProt);
        }
    }
    if (pEMS->bEnter_OverLoadHV || pEMS->bEnter_OverLoad_Phase)
    {
        // LJH250710 fix:众树超需量 // LJH250812 added:单相防过载
        pEMS->fTotal_PessChgAvaliable *= 0.8f;
        TRACEW("Calc_ESS_ChgLmtPower bEnter_OverLoadHV:%d bEnter_OverLoad_Phase:%d fTotal_PessChgAvaliable:%.1f",
            pEMS->bEnter_OverLoadHV, pEMS->bEnter_OverLoad_Phase, pEMS->fTotal_PessChgAvaliable);
    }
    if(pCCU->emESSDevType == LIQUID_COOLING_BACKUP) //WLD01206 2419备电柜特有的柜内防过载
    {
        int n = 0;
        if((_NOW_ - pCCU->tPowerUpdataTime) > 8)
        {//防止系统损耗波动 8秒计算一次系统消耗电量能力
            pCCU->fcabPowerConsump = 0;
            float fSumPwr = 0;
            pCCU->tPowerUpdataTime = _NOW_;
            for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
            {
                fSumPwr += (float)(pCCU->esRunData.fACDC_OutVolt[n]*pCCU->esRunData.fACDC_OutCurr[n]*0.001);
            }//系统损耗 = 柜内表 - 离网负载表 - 系统正在充电能力
            pCCU->fcabPowerConsump = pCCU->esRunData.f_PcabMeter - pCCU->esRunData.fPoffgrid_LoadMeter - fSumPwr; 
        }
        if(pCCU->fcabPowerConsump < 0 || pCCU->fcabPowerConsump > 10)
        {//系统消耗量不会超过10KW
            pCCU->fcabPowerConsump = 0;
        }
        float remain_ChgAvaliable = (float)fabs(pCCU->fTotalPowerLimit - pCCU->esRunData.fPoffgrid_LoadMeter - pCCU->fcabPowerConsump);
        if(pCCU->esRunData.f_PcabMeter > pCCU->fTotalPowerLimit || ((pCCU->esRunData.fPoffgrid_LoadMeter + fActvPwrChgLmt + pCCU->fcabPowerConsump) > pCCU->fTotalPowerLimit))
        {//减去离网负载和系统消耗后，系统剩余充电能力， 取所有的最小值，防止干扰原因防过载能力
            pEMS->fTotal_PessChgAvaliable = MIN(MIN(remain_ChgAvaliable, fActvPwrChgLmt), pEMS->fTotal_PessChgAvaliable);
            //TRACED("fActvPwrChgLmt = [%f], fTotal_PessChgAvaliable = [%f]\n", fActvPwrChgLmt, pEMS->fTotal_PessChgAvaliable);          
        }
    }
    pEMS->fTotal_PessChgAvaliable = MAX(pEMS->fTotal_PessChgAvaliable, 0);

    TRACED("Calc_ESS_ChgLmtPower bEnter_OverLoad=[%d] bEnter_OverLoadHV:%d bEnter_OverLoad_Phase:%d Need_AntiOverloadHV:%d fGridACInputPwr_Lmt=%.1f fPacload4Chg=%.1f fTotal_PessChgAvaliable=%.1f fSysACInput_PwrLmt=%.1f\n",
        pEMS->bEnter_OverLoad, pEMS->bEnter_OverLoadHV, pEMS->bEnter_OverLoad_Phase, Need_AntiOverloadHV(pCCU),
        pEMS->fGridACInputPwr_Lmt, pEMS->fPacload4Chg, pEMS->fTotal_PessChgAvaliable,pCCU->blobBackendCfg.fSysACInput_PwrLmt);
    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  Calc_ESS_DischgLmtPower
*  CREATOR       :  
*  CREATED DATE  : 2023-02-26 11:13:53
*  DESCRIPTION   :  计算储能系统的放电功率限制值
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Calc_ESS_DischgLmtPower(ROUGH_CCU_DATA *pCCU)
{
    // SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    float fProt = pCCU->blobBackendCfg.fPreProt4RefluxAndOverLoad;
    if(pEMS->nTMSector == -1)
    {
       pEMS->fTotal_PessDischgLmt = 0.0f; 
       return;
    }
    //--Added By John, IF ACLoad is Lower Then fMin_ACLoadPwr_Start_Dischg, ESS Not Start Dischg.---//
    if (pCCU->bQuickChannle || pEMS->emHighSensitivityEnable)
    {
        // printf("Calc_ESS_DischgLmtPower: %d\n", pEMS->emHighSensitivityEnable);
        pEMS->fAvg_Pacload4Dischg = pEMS->fPacload4Dischg;
    }
    else
    {
        pEMS->fAvg_Pacload4Dischg = Samp_Avg_Filter(FILTERTYPE_ACLOAD4DISCHGING, pEMS->fPacload4Dischg, (int)pCCU->nDisChrgFilterPoint);//MIN_FILTER_POINT
    }

    // LJH251129 deleted AEMS需要fPacload4Dischg的负值
    // if(pEMS->fAvg_Pacload4Dischg < pCCU->blobBackendCfg.fMin_ACLoadPwr_Start_Dischg)
    // {
    //     pEMS->fPacload4Dischg = 0.0f;
    // }

    TRACE("fAvg_Pacload4Dischg=[%.1f] fPacload4Dischg=[%.1f] fMin_ACLoadPwr_Start_Dischg=[%.1f]\n",\
        pEMS->fAvg_Pacload4Dischg, pEMS->fPacload4Dischg, pCCU->blobBackendCfg.fMin_ACLoadPwr_Start_Dischg);
    // LJH241007 AntiPhase
#ifdef UNIT_TEST_AntiPhase
    s_fTotal_PessDischgLmt = pEMS->fTotal_PessDischgLmt;
#endif

    //Step3.Calc OuputToACPower * Rate.
    // old pEMS->fTotal_PessDischgLmt = pEMS->fPacload4Dischg*pCCU->blobBackendCfg.fOutput2AC_PwrRate*0.95;
    // LJH250303 Modify
    pEMS->fTotal_PessDischgLmt = 
        (pEMS->fPacload4Dischg - fProt) * pCCU->blobBackendCfg.fOutput2AC_PwrRate;
    
    //Step4. Comp to Min Output To AC Power. 输出到AC侧最小功率限制(慎)
    // LJH250408 移到最后，不防逆流 pEMS->fTotal_PessDischgLmt = MAX(pCCU->blobBackendCfg.fMin_Output2AC_PwrLmt, pEMS->fTotal_PessDischgLmt);

    //Step5. Comp to Max Output To AC Power.
    
    //Step6. Dynamic Grid Input Limit
    // 使能动态网侧输入限值功能时，最小启动放电阈值设为0，放电比例设为100%
    // Added by Jeffery 241204 for Electra // 同步到站级EMS 250523
    
    if(pCCU->blobBackendCfg.emDynamicGridInputLimit_Mode != MODE_DISABLE)
    {   
        float fGridInputLimit = 0.0f;

        switch(pCCU->blobBackendCfg.emDynamicGridInputLimit_Mode)
        {
            case MODE_FIXED_LIMIT:
                fGridInputLimit = pCCU->blobBackendCfg.fFixed_GridInput_Limit;
                break;
            case MODE_DYNAMIC_LIMIT:
                fGridInputLimit = (float)(pCCU->blobEmsCfgPlan.emsLmtSectors_ext[pEMS->nTMSector].nGridInputPwrLmt);
                break;
            default:
                TRACE("Wrong Type from MQTT!\n"); // Prevent wrong value from MQTT setting.
                break;
        }
        // LJH251129 modify AEMS需要fPacload4Dischg的负值
        if(pEMS->fAvg_Pacload4Dischg < fGridInputLimit || pEMS->fAvg_Pacload4Dischg < pCCU->blobBackendCfg.fMin_ACLoadPwr_Start_Dischg)
        {
            pEMS->fTotal_PessDischgLmt = 0.0f;
        }
        else
        {
            pEMS->fTotal_PessDischgLmt = (float)((pEMS->fTotal_PessDischgLmt - fGridInputLimit) * 1.02);
        }
    }
   
    // //old added By John. 2023/07/21 PCS is DC Adjust Consider 0.02 Efficient Lost;
    // float fActvPwrDischgLmt = (float)((float)pEMS->planSectorNow.nActvPwrLmt * 1.02f);
    // //printf("pEMS->nTMSector=[%d] fActvPwrDischgLmt=[%.1f]\n", pEMS->nTMSector, fActvPwrDischgLmt);

    // pEMS->fTotal_PessDischgLmt = MIN(fActvPwrDischgLmt, pEMS->fTotal_PessDischgLmt);

    //奥地利认证:增加可设置逆流值
    pEMS->fTotal_PessDischgLmt += pEMS->fAllowedReverseValue;
    // LJH250910 modified
    pEMS->fTotal_PessDischgLmt = MIN((float)pEMS->planSectorNow.nActvPwrLmt, pEMS->fTotal_PessDischgLmt);
    if(pEMS->bEnterUrgetDischrgMode == TRUE) //Priority Process Urget Dischg Mode
    {
        //Enter Urget Power/Exit Power > 2, Use 0.5, if < 2, Use 0.2.
	    float fUrgetModeRate = 0.2f;
        if((pCCU->blobBackendCfg.fEnter_UrgetDischrgPwr/(pCCU->blobBackendCfg.fExit_UrgetDischrgPwr+1)) >= 3)
        {
            fUrgetModeRate = 0.5f;
        }  
        pEMS->fTotal_PessDischgLmt = pEMS->fTotal_PessDischgLmt*fUrgetModeRate;
    }
    // LJH241007 AntiPhase
    if (pCCU->needAntiReflux_Phase)
    {
        float fPhaseNum = (pEMS->isAtRflxMtrHV ? 2.0f:3.0f);
        pEMS->fAtRflxMtr_Factor = ((pEMS->fAtRflxMtr_Factor>-0.01f && pEMS->fAtRflxMtr_Factor<0.01f) ? 1.0f:pEMS->fAtRflxMtr_Factor);
        pEMS->fDiffPwr4Dischrg_SumPhs =
                (((pCCU->blobBackendCfg.fPreProt4Reflux_Phase - pEMS->fAtRflxMtr_kWPhMin) * fPhaseNum) /
                    pEMS->fAtRflxMtr_Factor);
        pEMS->fDiffPwr4Dischrg_SumPhs = MAX(0.0f, pEMS->fDiffPwr4Dischrg_SumPhs);
        pEMS->fTotal_PessDischgLmt = pEMS->fTotal_PessDischgLmt - pEMS->fDiffPwr4Dischrg_SumPhs;
    }
    // LJH241007 AntiPhase add bEnterAnti_Reflux_Phase
    if(pEMS->bEnterAnti_Reflux || pEMS->bEnterAnti_Reflux_Phase)
    {
        // old pEMS->fTotal_PessDischgLmt = pEMS->fTotal_PessDischgLmt*0.9f;
        // LJH250303 Added 分级处理
        TRACED("Calc_ESS_DischgLmtPower fTotal_PessDischgLmt:%.1f", pEMS->fTotal_PessDischgLmt);
        float fRemain = pEMS->fAtRflx_kW;
        float fHalfProt = fProt / 2.0f;
        if (pCCU->bQuickChannle == FALSE) //快速通道不需要分级处理
        {
            if (fRemain <= 0)
            {
                pEMS->fTotal_PessDischgLmt *= 0.8f;
                TRACEE("Calc_ESS_DischgLmtPower *= 0.8 fTotal_PessDischgLmt:%.1f fRemain:%.1f fProt:%.1f", pEMS->fTotal_PessDischgLmt, fRemain, fProt);
            }
            else if (fRemain < fHalfProt)
            {
                pEMS->fTotal_PessDischgLmt -= fProt;
                TRACEW("Calc_ESS_DischgLmtPower -= fProt fTotal_PessDischgLmt:%.1f fRemain:%.1f fProt:%.1f", pEMS->fTotal_PessDischgLmt, fRemain, fProt);
            }
            else
            {
                fRemain = MAX((fProt-fRemain), 0); // LJH250726 Added
                pEMS->fTotal_PessDischgLmt -= fRemain;
                TRACEI("Calc_ESS_DischgLmtPower -= (%.1f) fTotal_PessDischgLmt:%.1f fProt:%.1f", fRemain, pEMS->fTotal_PessDischgLmt, fProt);
            }
        }
    }
    // LJH250408 输出到AC侧最小功率限制(慎), 移到这里，不用防逆流
    pEMS->fTotal_PessDischgLmt = MAX(pCCU->blobBackendCfg.fMin_Output2AC_PwrLmt, pEMS->fTotal_PessDischgLmt);
    pEMS->fTotal_PessDischgLmt = MAX(pEMS->fTotal_PessDischgLmt, 0);
    TRACED("Calc_ESS_DischgLmtPower bEnterAnti_Reflux=[%d] fPacload4Dischg=%.1f fTotal_PessDischgLmt=%.1f\n",
        pEMS->bEnterAnti_Reflux, pEMS->fPacload4Dischg, pEMS->fTotal_PessDischgLmt);
    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  Calc ESS DischgLmt_ReactivePower
*  CREATOR       :  
*  CREATED DATE  : 2023-12-06 11:13:53
*  DESCRIPTION   :  Calc ESS Total Need Output Reactive Power when Dischg.
*		PCS : Positive + is L.   Negtive - is C.		    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Calc_ESS_Q_OutputLmt(ROUGH_CCU_DATA* pCCU)
{
    // SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;

    if(pEMS->nTMSector == -1)
    {
       pEMS->fTotal_Qess_OutputLmt = 0.0; 
       return;
    }

    float fQessLmt_Sett = (float)fabs((float)pEMS->planSectorNow.nReActvPwrLmt); 
 
    pEMS->fTotal_Qess_OutputLmt = pEMS->fQacload4Org*pCCU->blobBackendCfg.fOutput2AC_PwrRate*0.95f;
    
    pEMS->fTotal_Qess_OutputLmt = MIN(pCCU->blobBackendCfg.fQmaxOutput2ACLmt, pEMS->fTotal_Qess_OutputLmt);

    pEMS->fTotal_Qess_OutputLmt = MIN(fQessLmt_Sett, pEMS->fTotal_Qess_OutputLmt);

    pEMS->fTotal_Qess_OutputLmt = MAX(0.0f, pEMS->fTotal_Qess_OutputLmt);

    TRACE("tm=[%ld] fQessLmt_Sett=[%.f] fQacload4Org=[%.f] fTotal_Qess_OutputLmt=[%.1f] fQmaxOutput2ACLmt=[%.f] fOutput2AC_PwrRate=[%.1f]\n",\
        _NOW_, fQessLmt_Sett , pEMS->fQacload4Org, pEMS->fTotal_Qess_OutputLmt , \
        pCCU->blobBackendCfg.fQmaxOutput2ACLmt, pCCU->blobBackendCfg.fOutput2AC_PwrRate);

    return;
}

static void JudgeUrgetDisChrgMode(ROUGH_CCU_DATA* pCCU)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    time_t tm_Now = _NOW_;

    if(pEMS->bEnterUrgetDischrgMode == TRUE)
    {
        if(((ABS(tm_Now - pEMS->tm_LastEnterUrgetMode)) > MAX_RETDIFF_PERIOD) && 
            (pEMS->f_PgridMeter < pCCU->blobBackendCfg.fExit_UrgetDischrgPwr))  
        {
            pEMS->bEnterUrgetDischrgMode = FALSE;
            TRACE("111 tm_Now=[%ld] pEMS->bEnterUrgetDischrgMode=[%d] f_PgridMeter=[%.1f]\n", \
                tm_Now, pEMS->bEnterUrgetDischrgMode, pEMS->f_PgridMeter);
        }
    }
    else
    {
        if(pEMS->f_PgridMeter > pCCU->blobBackendCfg.fEnter_UrgetDischrgPwr) //Pre-Sett Value,Due To EMS Adjust Power is Too Slow.
        {
            pEMS->bEnterUrgetDischrgMode = TRUE;
            pEMS->tm_LastEnterUrgetMode = tm_Now;
            TRACE("222 tm_Now=[%ld] pEMS->bEnterUrgetDischrgMode=[%d] f_PgridMeter=[%.1f]\n", \
                tm_Now, pEMS->bEnterUrgetDischrgMode, pEMS->f_PgridMeter);
        }
    }

    return;
}

static void Calc_AllMeterPower(ROUGH_CCU_DATA* pCCU)
{
    Calc_ACLoadPwrByDiffMeterPosition(pCCU);
    
    Calc_ESS_ChgLmtPower(pCCU);

    Calc_ESS_DischgLmtPower(pCCU);

#ifdef _TRACE_PRINT_
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    TRACEI("Calc_AllMeterPower f_PgridMeter:%.1f f_PcabMeter:%.1f bCabMeter_SEMS:%d  f_PauxMeter=%.1f f_PauxMeter_3=[%.1f] f_PmaxGridDemand=%.1f\n"
        "fPacload4Chg:%.1f fPacload4Dischg:%.1f fAtOvl_kW:%.1f fAtRflx_kW:%.1f fSysACInput_PwrLmt:%.1f fPreProt4RefluxAndOverLoad:%.1f\n"
        "fTotal_PessChgAvaliable:%.1f fTotal_PessDischgLmt:%.1f",
        pEMS->f_PgridMeter, pEMS->f_PcabMeter, pEMS->bCabMeter_SEMS, pEMS->f_PauxMeter, pEMS->f_PauxMeter_3, pEMS->f_PmaxGridDemand,
        pEMS->fPacload4Chg, pEMS->fPacload4Dischg, pEMS->fAtOvl_kW, pEMS->fAtRflx_kW, pCCU->blobBackendCfg.fSysACInput_PwrLmt, pCCU->blobBackendCfg.fPreProt4RefluxAndOverLoad,
        pEMS->fTotal_PessChgAvaliable, pEMS->fTotal_PessDischgLmt
        );
#endif
    Calc_ESS_Q_OutputLmt(pCCU);

    // EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    // printf("f_PgridMeter:[%.1f] f_PcabMeter:[%.1f] fPacload4Dischg:[%.1f] fTotal_PessDischgLmt:[%.1f] fAvg_Pacload4Dischg:[%.1f]\n",
    //     pEMS->f_PgridMeter, pEMS->f_PcabMeter,pEMS->fPacload4Dischg,pEMS->fTotal_PessDischgLmt,pEMS->fAvg_Pacload4Dischg);
    return;
}
// LJH250808 从控只取管理计划第一段
BOOL GetCurrentTimeSector(ROUGH_CCU_DATA* pCCU, BOOL bFromOrg)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    pEMS->nTMSector = Get_EmsCfgPlan_Setctor(&pCCU->blobEmsCfgPlan, bFromOrg);

    EMS_LIMIT_SECTOR* pPlanSectorOrg = &pEMS->planSectorOrg;
    EMS_LIMIT_SECTOR* pPlanSectorNow = &pEMS->planSectorNow;
    if(pEMS->nTMSector < 0)
    {
        pPlanSectorNow->tmStart =        pEMS->tmStart0sec; // 946684800
        pPlanSectorNow->nInterval =      86400;
        // pPlanSectorNow->emFeeRate = FEERATE_VALLEY;
        pPlanSectorNow->emBatt_WorkMode = BATT_WORKMODE_IDLE;
        pPlanSectorNow->nStopSOC =       0;
        pPlanSectorNow->nActvPwrLmt =    0;
        pPlanSectorNow->nReActvPwrLmt =  0;
        pPlanSectorNow->nBattGrpPwrLmt = 0;
        *pPlanSectorOrg = *pPlanSectorNow;
        TRACEE("GetCurrentTimeSector nTMSector < 0 :%d nBattGrpPwrLmt:%u", pEMS->nTMSector, pPlanSectorNow->nBattGrpPwrLmt);
    }
    else
    {
        // LJH241202 SEMS
        *pPlanSectorOrg = pCCU->blobEmsCfgPlan.emsLmtSectors[pEMS->nTMSector];
        if (bFromOrg)
        {
            *pPlanSectorNow = *pPlanSectorOrg;
        }
        // 朗新放电的SOC下发100%
        // pPlanSectorNow->nStopSOC=5;
        // if(pCCU->emDisChgByStopSOC == 0)
        // {

        // }
        // pPlanSectorNow->nStopSOC = pCCU->nProtectSOC;

        // LJH250625 Added
        A_BLOB_SEMS_CLIENTINFO_SUM* pClientSum = &pEMS->clientInfoSum;
        BOOL bStopChrgBySoc_Plan = (pClientSum->nSOCMinSum >= (BYTE)pPlanSectorNow->nStopSOC);
        BOOL bStopDiscBySoc_Plan = (pClientSum->nSOCMaxSum <= (BYTE)pPlanSectorNow->nStopSOC);
        // LJH250930 加快自动充放响应,ChgDisOnly
        pPlanSectorNow->emBatt_WorkMode = (pPlanSectorNow->emBatt_WorkMode==BATT_WORKMODE_DISCHRG_ONLY ?
            BATT_WORKMODE_FIRST_DISCHRG : pPlanSectorNow->emBatt_WorkMode);
        pPlanSectorNow->emBatt_WorkMode = (pPlanSectorNow->emBatt_WorkMode==BATT_WORKMODE_CHRGING_ONLY ?
            BATT_WORKMODE_FIRST_CHRGING : pPlanSectorNow->emBatt_WorkMode);
        if (pPlanSectorNow->nActvPwrLmt < 1 || pPlanSectorNow->nBattGrpPwrLmt < 1 ||
            pClientSum->emCCUStateSum!=CCU_STATE_IDLE || pClientSum->emPMCtrlModeSum!=AUTO_CTRL_PM_MODE ||
            (pPlanSectorNow->emBatt_WorkMode==BATT_WORKMODE_FIRST_CHRGING && bStopChrgBySoc_Plan) ||
            (pPlanSectorNow->emBatt_WorkMode==BATT_WORKMODE_FIRST_DISCHRG && bStopDiscBySoc_Plan)
        )
        {
            TRACEI("GetCurrentTimeSector set emBatt_WorkMode = BATT_WORKMODE_IDLE \n"
                "nActvPwrLmt:%d nBattGrpPwrLmt:%u emCCUStateSum:%d emPMCtrlModeSum:%d "
                "(emBatt_WorkMode:%d && bStopChrgBySoc_Plan:%d) || (emBatt_WorkMode:%d && bStopDiscBySoc_Plan:%d",
                pPlanSectorNow->nActvPwrLmt, pPlanSectorNow->nBattGrpPwrLmt, pClientSum->emCCUStateSum, pClientSum->emPMCtrlModeSum,
                pPlanSectorNow->emBatt_WorkMode, bStopChrgBySoc_Plan, pPlanSectorNow->emBatt_WorkMode, bStopDiscBySoc_Plan);

            pPlanSectorNow->emBatt_WorkMode = BATT_WORKMODE_IDLE;
        }
    }
    pEMS->emBatt_WorkModeReq = pPlanSectorNow->emBatt_WorkMode;

    // else
    // {
    //     // LJH241202 SEMS
    //     *pPlanSectorOrg = pCCU->blobEmsCfgPlan.emsLmtSectors[pEMS->nTMSector];
    //     if (bFromOrg)
    //     {
    //         *pPlanSectorNow = *pPlanSectorOrg;
    //     }
    //     // LJH250625 Added
    //     A_BLOB_SEMS_CLIENTINFO_SUM* pClientSum = &pEMS->clientInfoSum;
    //     BOOL bStopChrgBySoc_Plan = (pClientSum->nSOCMinSum >= (BYTE)pPlanSectorNow->nStopSOC);
    //     BOOL bStopDiscBySoc_Plan = (pClientSum->nSOCMaxSum <= (BYTE)pPlanSectorNow->nStopSOC);
    //     // LJH250930 加快自动充放响应,ChgDisOnly
    //     if (pPlanSectorNow->nActvPwrLmt < 1 || pPlanSectorNow->nBattGrpPwrLmt < 1 ||
    //         pClientSum->emCCUStateSum!=CCU_STATE_IDLE || pClientSum->emPMCtrlModeSum!=AUTO_CTRL_PM_MODE ||
    //         ((  pPlanSectorNow->emBatt_WorkMode==BATT_WORKMODE_FIRST_CHRGING ||
    //             pPlanSectorNow->emBatt_WorkMode==BATT_WORKMODE_CHRGING_ONLY) && bStopChrgBySoc_Plan) ||
    //         (( pPlanSectorNow->emBatt_WorkMode==BATT_WORKMODE_FIRST_DISCHRG ||
    //             pPlanSectorNow->emBatt_WorkMode==BATT_WORKMODE_DISCHRG_ONLY) && bStopDiscBySoc_Plan)
    //     )
    //     {
    //         TRACEI("GetCurrentTimeSector set emBatt_WorkMode = BATT_WORKMODE_IDLE \n"
    //             "nActvPwrLmt:%d nBattGrpPwrLmt:%u emCCUStateSum:%d emPMCtrlModeSum:%d "
    //             "(emBatt_WorkMode:%d && bStopChrgBySoc_Plan:%d) || (emBatt_WorkMode:%d && bStopDiscBySoc_Plan:%d",
    //             pPlanSectorNow->nActvPwrLmt, pPlanSectorNow->nBattGrpPwrLmt, pClientSum->emCCUStateSum, pClientSum->emPMCtrlModeSum,
    //             pPlanSectorNow->emBatt_WorkMode, bStopChrgBySoc_Plan, pPlanSectorNow->emBatt_WorkMode, bStopDiscBySoc_Plan);

    //         pPlanSectorNow->emBatt_WorkMode = BATT_WORKMODE_IDLE;
    //     }
    // }
    // pEMS->emBatt_WorkModeReq = pPlanSectorNow->emBatt_WorkMode;
    // pEMS->emBatt_WorkModeReq = (pEMS->emBatt_WorkModeReq==BATT_WORKMODE_DISCHRG_ONLY ?
    //     BATT_WORKMODE_FIRST_DISCHRG:pEMS->emBatt_WorkModeReq);
    // pEMS->emBatt_WorkModeReq = (pEMS->emBatt_WorkModeReq==BATT_WORKMODE_CHRGING_ONLY ?
    //     BATT_WORKMODE_FIRST_CHRGING:pEMS->emBatt_WorkModeReq);

    TRACED("GetCurrentTimeSector nTotal_EmsLmt_Sectors=%d nTMSector=%d bFromOrg:%d emBatt_WorkModeReq:%d 5:idle \n"
        "Org:emBatt_WorkMode:%d nStopSOC:%u, nActvPwrLmt:%u nBattGrpPwrLmt:%u \n"
        "Now:emBatt_WorkMode:%d nStopSOC:%u, nActvPwrLmt:%u nBattGrpPwrLmt:%u",
        pCCU->blobEmsCfgPlan.nTotal_EmsLmt_Sectors, pEMS->nTMSector, bFromOrg, pEMS->emBatt_WorkModeReq,
        pPlanSectorOrg->emBatt_WorkMode, pPlanSectorOrg->nStopSOC, pPlanSectorOrg->nActvPwrLmt, pPlanSectorOrg->nBattGrpPwrLmt,
        pPlanSectorNow->emBatt_WorkMode, pPlanSectorNow->nStopSOC, pPlanSectorNow->nActvPwrLmt, pPlanSectorNow->nBattGrpPwrLmt);
    return TRUE;
}

void Ems_init(ROUGH_CCU_DATA *pCCU)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    if (pEMS->bEnterUrgetDischrgMode)
    {
        pEMS->emBatt_WorkModeReq = BATT_WORKMODE_FIRST_DISCHRG;
    }
    
    // LJH241120 fixChrgAndDisTogether 初始化
    // int n = 0;
    // charge
    pEMS->fTotal_PessChgAvaliable = 0;
    pEMS->fTotalPM_Chrg_PwrAbility = 0;
    pEMS->fTotalPM_Chrg_PwrAbilityAll = 0;
    pEMS->fRemain_ACChrgingPwr_Lmt = 0;
    pEMS->fAvg_Remain_ChrgingPwr_Lmt = 0;
    pEMS->fTotal_PessChg_PidOut = 0;
    // discharge
    pEMS->fTotal_PessDischgLmt = 0;
    pEMS->fTotalPM_DisChrging_PwrAbility = 0;
    pEMS->fTotalPM_DisChrging_PwrAbilityAll = 0;
    pEMS->fRemain_ACLoad_DisChrgPwr_Req = 0;
    pEMS->fAvg_Remain_DisChrgPwr_Req = 0;
    pEMS->fTotal_PessDischg_PidOut = 0;
}

void Ems_Reset_Calc_Node(A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Node)
{
    pCalc_Node->emBatt_WorkMode = BATT_WORKMODE_IDLE;
    pCalc_Node->fRemain_Batt_Chrg_Pwr_Req = 0;
    pCalc_Node->fMax_Chrg_PwrAbility = 0;
    pCalc_Node->fACDC_Distri_4_BattChrg_Pwr = 0;
    pCalc_Node->fAvg_ChrgLmtPower = 0;

    pCalc_Node->fRemain_Batt_DisChrg_Pwr = 0;
    pCalc_Node->fMax_DisChrg_PwrAbility = 0;
    pCalc_Node->fBatt_Distri_4_ACDC_Pwr = 0;
    pCalc_Node->fAvg_DisChrgingLmtPower = 0;
    pCalc_Node->nStopSOCReq = 50;
}

// LJH250205 REMS 当从机不执行管理计划，使用服务端下发的充放电功率，临时修改第一段00:00:00-24:00:00的管理计划
// LJH250612 add:nBattGrpPwrLmt 单柜最大功率限制 pPlanSectorOrg:主控A6_USYS或者数据库原始数据 pPlanSectorNow:当前业务数据
void SetPlanSector(PlanSector* pPlanSector,
    BYTE emBatt_WorkMode, BYTE nStopSOC, UINT nActvPwrLmt, int nReActvPwrLmt, UINT nBattGrpPwrLmt)
{
    pPlanSector->emBatt_WorkModePlan = emBatt_WorkMode;
    pPlanSector->nStopSOCPlan =        nStopSOC;
    pPlanSector->nActvPwrLmtPlan =     nActvPwrLmt;
    pPlanSector->nReActvPwrLmtPlan =   nReActvPwrLmt;
    pPlanSector->nBattGrpPwrLmtPlan =  nBattGrpPwrLmt;
}

void ModifyBlobEmsCfgPlan(ROUGH_CCU_DATA* pCCU, BOOL bRestored, PlanSector* pPlanNow, PlanSector* pPlanOrg)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    if (bRestored)
    {
        VAR_VALUE varV;
        varV.blobValue = (BYTE *)&pCCU->blobEmsCfgPlan;
	    pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 200, 201, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN));
        pEMS->changedBlobEmsCfgPlan = FALSE;
        TRACED("ModifyBlobEmsCfgPlan restored");
    }
    else
    {
        pEMS->changedBlobEmsCfgPlan = TRUE;
        // 修改第0段EMSPlan
        pCCU->blobEmsCfgPlan.nTotal_EmsLmt_Sectors = 1;
        pEMS->nTMSector = 0;
        EMS_LIMIT_SECTOR* pPlanSector0 =   &pCCU->blobEmsCfgPlan.emsLmtSectors[0];
        pPlanSector0->tmStart =        pEMS->tmStart0sec; // 946684800
        pPlanSector0->nInterval =      86400;
        // pPlanSector0->emFeeRate = FEERATE_VALLEY;
        if (pPlanOrg != NULL)
        {
            // TRACEI("ModifyBlobEmsCfgPlan !bRestored set pPlanSectorOrg");
            EMS_LIMIT_SECTOR* pPlanSectorOrg = &pEMS->planSectorOrg;
            pPlanSector0->emBatt_WorkMode = pPlanOrg->emBatt_WorkModePlan;
            pPlanSector0->nStopSOC =        pPlanOrg->nStopSOCPlan;
            pPlanSector0->nActvPwrLmt =     pPlanOrg->nActvPwrLmtPlan;
            pPlanSector0->nReActvPwrLmt =   pPlanOrg->nReActvPwrLmtPlan;
            pPlanSector0->nBattGrpPwrLmt =  pPlanOrg->nBattGrpPwrLmtPlan;
            *pPlanSectorOrg = *pPlanSector0;
        }
        if (pPlanNow != NULL)
        {
            // TRACEI("ModifyBlobEmsCfgPlan !bRestored set pPlanSectorNow");
            EMS_LIMIT_SECTOR* pPlanSectorNow = &pEMS->planSectorNow;
            pPlanSectorNow->tmStart =        pEMS->tmStart0sec; // 946684800
            pPlanSectorNow->nInterval =      86400;
            pPlanSectorNow->emBatt_WorkMode = pPlanNow->emBatt_WorkModePlan;
            pPlanSectorNow->nStopSOC =        pPlanNow->nStopSOCPlan;
            pPlanSectorNow->nActvPwrLmt =     pPlanNow->nActvPwrLmtPlan;
            pPlanSectorNow->nReActvPwrLmt =   pPlanNow->nReActvPwrLmtPlan;
            pPlanSectorNow->nBattGrpPwrLmt =  pPlanNow->nBattGrpPwrLmtPlan;
        }
        // TRACED("ModifyBlobEmsCfgPlan !bRestored tmStart:%lu \n"
        //     "Org:emBatt_WorkMode:%d 5:idle nStopSOC:%d nActvPwrLmt:%u nBattGrpPwrLmt:%u \n"
        //     "Now:emBatt_WorkMode:%d 5:idle nStopSOC:%d nActvPwrLmt:%u nBattGrpPwrLmt:%u",
        //     pPlanSector0->tmStart, 
        //     pEMS->planSectorOrg.emBatt_WorkMode, pEMS->planSectorOrg.nStopSOC,
        //     pEMS->planSectorOrg.nActvPwrLmt, pEMS->planSectorOrg.nBattGrpPwrLmt,
        //     pEMS->planSectorNow.emBatt_WorkMode, pEMS->planSectorNow.nStopSOC,
        //     pEMS->planSectorNow.nActvPwrLmt, pEMS->planSectorNow.nBattGrpPwrLmt);
    }
}

// LJH250902 top#6+REMS 获取上级EMS遥调参数 104-6191H
void GetCtrlParam_REMS(ROUGH_CCU_DATA* pCCU)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
    DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
    A_BLOB_SEMS_CLIENTINFO_SUM* pClientSum = &pCCU->emsMgmtData.clientInfoSum;
    VAR_VALUE varV;
    ERRCDDE_DPR err;
    
    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 1001, 3902, &varV, 4);
    pClientSum->emEMSCtrlMode_REMS = (err == ERR_DPR_OK) ? varV.emValue : EMSCtrlMode_Local;
    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 1001, 3907, &varV, 4);
    pClientSum->emACDC_WorkStatu_REMS = (err == ERR_DPR_OK) ? varV.emValue : STATU_ACDC_IDLE;
    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 1001, 3901, &varV, 4);
    pClientSum->fActvPwrReq_REMS = (err == ERR_DPR_OK) ? varV.fValue : 0.0f;
    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 1001, 3908, &varV, 4);
    pClientSum->bREMS_CommStatus = (err == ERR_DPR_OK) ? varV.emValue : 1;

    if(pClientSum->bREMS_CommStatus == 1)
    {
        pClientSum->emEMSCtrlMode_REMS = EMSCtrlMode_Local;
        pClientSum->emACDC_WorkStatu_REMS = STATU_ACDC_IDLE;
        pClientSum->fActvPwrReq_REMS = 0;
    }

    // // unit test REMS
    // {
    //     err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 200, 204, &varV, 4); // MPPT_1 Generate KWH
    //     pClientSum->emEMSCtrlMode_REMS = (varV.fValue < 1.0f ? EMSCtrlMode_Local : EMSCtrlMode_Remote);
    //     err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 200, 205, &varV, 4); // MPPT_2 Generate KWH
    //     pClientSum->emACDC_WorkStatu_REMS = (ACDC_WORKSTATU)varV.fValue;
    //     err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 200, 206, &varV, 4); // MPPT_3 Generate KWH
    //     pClientSum->fActvPwrReq_REMS = varV.fValue;
    // }
    
    if (pClientSum->emACDC_WorkStatu_REMS == STATU_ACDC_IDLE && pEMS->bStandby_BLL)
    {
        pEMS->bStandby_BLL = FALSE;
        varV.emValue = (_ENUM)pEMS->bStandby_BLL;
        Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1001, 3800, varV);
    }
    else if (pClientSum->emACDC_WorkStatu_REMS == STATU_ACDC_STANDBY && !pEMS->bStandby_BLL)
    {
        pEMS->bStandby_BLL = TRUE;
        varV.emValue = (_ENUM)pEMS->bStandby_BLL;
        Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1001, 3800, varV);
    }

    TRACED("GetCtrlParam_REMS emEMSCtrlMode_REMS:%d 1:Remote emACDC_WorkStatu_REMS:%d fActvPwrReq_REMS:%.1f",
        pClientSum->emEMSCtrlMode_REMS, pClientSum->emACDC_WorkStatu_REMS, pClientSum->fActvPwrReq_REMS);
}

// LJH250808 从控只取管理计划第一段
static void S_Ems_Mgmt_Total_Pess(ROUGH_CCU_DATA* pCCU)
{
    TRACE("-------------------Start Solar Backup EMS Calculate Node------------------\n");
    //RefreshLoadReq(pCCU);
    Refresh_RealTime_MeterValue(pCCU);
    // LJH241007 先判断是否要紧急放电
    JudgeUrgetDisChrgMode(pCCU);//Need Updete InFuture.John 20230226
    Ems_init(pCCU);
    // fTotal_PessChgAvaliable fTotal_PessDischgLmt
    Calc_AllMeterPower(pCCU);
    
    TRACEI("S_Ems_Mgmt_Total_Pess fTotal_PessChgAvaliable:%.1f fTotal_PessDischgLmt:%.1f nTMSector:%d emBatt_WorkModeReq:%d",
        pCCU->emsMgmtData.fTotal_PessChgAvaliable, pCCU->emsMgmtData.fTotal_PessDischgLmt,
        pCCU->emsMgmtData.nTMSector, pCCU->emsMgmtData.emBatt_WorkModeReq
    );
#ifdef TRACE_PESS
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    // A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Nodes = &(pEMS->calc_Node[0]);
    int nActvPwrLmt = (int)pEMS->planSectorNow.nActvPwrLmt;
    // if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_CHRGING)
    {
        float Pess1 = pEMS->fGridACInputPwr_Lmt - pEMS->fPacload4Chg - pCCU->blobBackendCfg.fPreProt4RefluxAndOverLoad;
        TRACEW(
            "S_Ems_Mgmt_Total_Pess chrg fTotal_PessChgAvaliable:%.1f %.1f Pess1:%.1f "
            "MIN(EmsPlan'nActvPwrLmt:%.1f %d*0.98 fGridRemainACPwr2Chrg:%.1f) "

            "MIN-fGridRemainACPwr2Chrg:%.1f(Pess1:%.1f=fGridACInputPwr_Lmt:%.1f-%.1f-%.1f "
            "HV:%d?:%.1f=%.1f-%.1f-%.1f) "
            "MIN-fGridACInputPwr_Lmt:%.1f(fSysACInput_PwrLmt:%.1f fMaxPwr4Chrg_SumPhs:%.1f) "
            "bEnter_OverLoad:%d|%d? %.1f*0.8=%.1f fSysACInput_PwrLmt:%.1f bEnter_OverLoadHV:%d?%.1f Pess1-fPreProt4RefluxAndOverLoad"
            "fMaxPwr4Chrg_SumPhs:%.1f= %.3f*%.3f*%.3f/1000.0*3.0 "
            "fAtOvl_kW:%.1f fAtOvlMtr_VPhMin:%.1f|%.1f %.1f %.1f fAtOvlMtr_APhMax:%.1f|%.1f %.1f %.1f "
            "needAntiOverLoad_Phase:%d",
            pEMS->fTotal_PessChgAvaliable, s_fTotal_PessChgAvaliable, Pess1,
            (float)nActvPwrLmt*0.98f, nActvPwrLmt, pEMS->fGridRemainACPwr2Chrg,

            pEMS->fGridRemainACPwr2Chrg, Pess1, pEMS->fGridACInputPwr_Lmt, pEMS->fPacload4Chg, pCCU->blobBackendCfg.fPreProt4RefluxAndOverLoad,
            Need_AntiOverloadHV(pCCU), pEMS->f_PmaxGridDemand-pEMS->f_PgridMeter-pCCU->blobBackendCfg.fPreProt4RefluxAndOverLoad, pEMS->f_PmaxGridDemand, pEMS->f_PgridMeter, pCCU->blobBackendCfg.fPreProt4RefluxAndOverLoad,
            pEMS->fGridACInputPwr_Lmt, pCCU->blobBackendCfg.fSysACInput_PwrLmt, pEMS->fMaxPwr4Chrg_SumPhs,
            pEMS->bEnter_OverLoad, pEMS->bEnter_OverLoad_Phase, Pess1, Pess1*0.8, pCCU->blobBackendCfg.fSysACInput_PwrLmt, pEMS->bEnter_OverLoadHV, Pess1-pCCU->blobBackendCfg.fPreProt4RefluxAndOverLoad,
            pEMS->fMaxPwr4Chrg_SumPhs, pEMS->fAtOvl_kW, pEMS->fAtOvlMtr_VPhMin, pCCU->blobBackendCfg.fMaxChrgCurLmt_Phase, pEMS->fAtOvlMtr_Factor,
            pEMS->fAtOvlMtr_VPhMin, pEMS->fAtOvlMtr_VPhs[0], pEMS->fAtOvlMtr_VPhs[1], pEMS->fAtOvlMtr_VPhs[2],
            pEMS->fAtOvlMtr_APhMax, pEMS->fAtOvlMtr_APhs[0], pEMS->fAtOvlMtr_APhs[1], pEMS->fAtOvlMtr_APhs[2],
            pCCU->needAntiOverLoad_Phase
        );
    }
    // else if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_DISCHRG)
    {
        float Pess1 = MIN(
            (pEMS->fPacload4Dischg - pCCU->blobBackendCfg.fPreProt4RefluxAndOverLoad) * pCCU->blobBackendCfg.fOutput2AC_PwrRate,
                (float)nActvPwrLmt*1.02f);
        float PessDiffPhs = (Pess1 - pEMS->fDiffPwr4Dischrg_SumPhs);
        TRACEI(
            "S_Ems_Mgmt_Total_Pess dischrg fTotal_PessDischgLmt:%.1f %.1f "
            "MIN(Pess1:%.1f fPacload4Dischg:(%.1f-%.1f)*%.3f EmsPlan'nActvPwrLmt:%.1f=%d*1.02) "
            "bEnterAnti_Reflux:%d?%.1f Pess1:%.1f*0.9 "
            "bEnterAnti_Reflux_Phase:%d?%.1f %.1f*0.9 "
            "needAntiReflux_Phase:%d?%.1f=%.1f-%.1f fDiffPwr4Dischrg_SumPhs:%.1f=%.3f/%.3f*3 |isAtRflxMtrHV?%d 2 "
            "fPreProt4Reflux_Phase:%.1f fAtRflx_kW:%.1f "
            "fAtRflxMtr_kWPhMin:%.1f|%.1f %.1f %.1f",
            pEMS->fTotal_PessDischgLmt, s_fTotal_PessDischgLmt,
            Pess1, pEMS->fPacload4Dischg, pCCU->blobBackendCfg.fPreProt4RefluxAndOverLoad, pCCU->blobBackendCfg.fOutput2AC_PwrRate,
            nActvPwrLmt*1.02, nActvPwrLmt,
            pEMS->bEnterAnti_Reflux, Pess1, Pess1*0.9,
            pEMS->bEnterAnti_Reflux_Phase, PessDiffPhs*0.9, PessDiffPhs,
            pCCU->needAntiReflux_Phase, PessDiffPhs, Pess1, pEMS->fDiffPwr4Dischrg_SumPhs,
            pEMS->fDiffPwr4Dischrg_SumPhs, pCCU->blobBackendCfg.fPreProt4Reflux_Phase - pEMS->fAtRflxMtr_kWPhMin, pEMS->fAtRflxMtr_Factor, pEMS->isAtRflxMtrHV,
            pCCU->blobBackendCfg.fPreProt4Reflux_Phase,  pEMS->fAtRflx_kW,
            pEMS->fAtRflxMtr_kWPhMin, pEMS->fAtRflxMtr_kWPhs[0], pEMS->fAtRflxMtr_kWPhs[1], pEMS->fAtRflxMtr_kWPhs[2]
        );
    }
#endif
}

// void Ems_Mgmt_Total_Pess_REMS(ROUGH_CCU_DATA* pCCU)
// {
//     SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
//     DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
//     EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
//     A_BLOB_SEMS_CLIENTINFO_SUM* pClientSum = &pEMS->clientInfoSum;

//     VAR_VALUE varV;
//     ERRCDDE_DPR err;

//     err = pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1, 47, &varV, 4); //Priority
//     pEMS->nPrio = (err == ERR_DPR_OK) ? varV.ulValue : 1;
//     pEMS->bEnterUrgetDischrgMode = FALSE;

//     // LJH251028 REMS,增加ACDCWorkStatu
//     STBATT_PRI_WORKMODE emBatt_WorkModeReq = MapACDC_WorkStatu2BattWorkMode(pClientSum->emACDC_WorkStatu_REMS);
//     BYTE nStopSoc;
//     switch (pClientSum->emACDC_WorkStatu_REMS)
//     {
//     case STATU_ACDC_RECTIFY:
//         nStopSoc = pEMS->nStopSOC_ChrgSet_SEMS;
//         break;
//     case STATU_ACDC_INVERTER:
//         nStopSoc = pEMS->nStopSOC_DiscSet_SEMS;
//         break;
//     default:
//         nStopSoc = 0;
//         break;
//     }
    
//     // old STBATT_PRI_WORKMODE emBatt_WorkModeReq = BATT_WORKMODE_IDLE;
//     // BYTE nStopSoc;
//     // if (pClientSum->fActvPwrReq_REMS > MIN_PWR_PCS_OUT_REQ)
//     // {
//     //     emBatt_WorkModeReq = BATT_WORKMODE_FIRST_CHRGING;
//     //     nStopSoc = pEMS->nStopSOC_ChrgSet_SEMS;
//     //     // nStopSoc = MAX_STOPSOC_CHRG;
//     // }
//     // else if (pClientSum->fActvPwrReq_REMS < MIN_PWR_PCS_OUT_REQ*-1.0f)
//     // {
//     //     emBatt_WorkModeReq = BATT_WORKMODE_FIRST_DISCHRG;
//     //     nStopSoc = pEMS->nStopSOC_DiscSet_SEMS;
//     //     // nStopSoc = MIN_STOPSOC_DISC;
//     // }
//     // else
//     // {
//     //     TRACEW("Ems_Mgmt_Total_Pess_REMS set BATT_WORKMODE_IDLE fActvPwrReq_REMS:%.1f < %.1f", pClientSum->fActvPwrReq_REMS, MIN_PWR_PCS_OUT_REQ);
//     //     emBatt_WorkModeReq = BATT_WORKMODE_IDLE;
//     //     nStopSoc = pEMS->nStopSOC_DiscSet_SEMS;
//     //     // nStopSoc = MIN_STOPSOC_DISC;
//     // }
//     // TRACED("-------XSY:nStopSOC_ChrgSet_SEMS = %d, nStopSOC_DiscSet_SEMS=%d\n", 
//     //     pEMS->nStopSOC_ChrgSet_SEMS, pEMS->nStopSOC_DiscSet_SEMS);

//     // 修改第0段EMSPlan
//     PlanSector planSectorNow;
//     SetPlanSector(&planSectorNow, emBatt_WorkModeReq, (BYTE)nStopSoc, (UINT)(ABS(pClientSum->fActvPwrReq_REMS)), 0, MAX_PWR_LMT);
//     TRACED("Ems_Mgmt_Total_Pess_REMS ModifyBlobEmsCfgPlan planSectorNow emBatt_WorkMode:%d 5:idle nActvPwrLmtPlan:%u nBattGrpPwrLmtPlan:%u fActvPwrReq_REMS:%.1f nStopSoc:%d",
//         planSectorNow.emBatt_WorkModePlan, planSectorNow.nActvPwrLmtPlan, planSectorNow.nBattGrpPwrLmtPlan, pClientSum->fActvPwrReq_REMS, nStopSoc);
//     ModifyBlobEmsCfgPlan(pCCU, FALSE, &planSectorNow, NULL);
//     GetCurrentTimeSector(pCCU, FALSE);
//     Ems_Mgmt_Total_Pess(pCCU);
//     if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_CHRGING)
//     {
//         if (pEMS->enableAntiOvld_REMS)
//         {
//             // 防过载
//             pEMS->fTotal_PessChgAvaliable = MIN(pEMS->fTotal_PessChgAvaliable, pClientSum->fActvPwrReq_REMS);
//         }
//         else
//         {
//             pEMS->fTotal_PessChgAvaliable = pClientSum->fActvPwrReq_REMS;
//         }
//     }
//     else if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_DISCHRG)
//     {
//         float fActvPwrReqPositive = ABS(pClientSum->fActvPwrReq_REMS);
//         if (pEMS->enableAntiRflx_REMS)
//         {
//             // 防逆流
//             pEMS->fTotal_PessDischgLmt = MIN(pEMS->fTotal_PessDischgLmt, fActvPwrReqPositive);
//         }
//         else
//         {
//             pEMS->fTotal_PessDischgLmt = fActvPwrReqPositive;
//         }
//     }
//     TRACEI("Ems_Mgmt_Total_Pess_REMS emBatt_WorkModeReq:%d fActvPwrReq_REMS:%.1f fTotal_PessChgAvaliable:%.1f fTotal_PessDischgLmt:%.1f "
//         "enableAntiOvld_REMS:%d enableAntiRflx_REMS:%d",
//         pEMS->emBatt_WorkModeReq, pClientSum->fActvPwrReq_REMS, pEMS->fTotal_PessChgAvaliable, pEMS->fTotal_PessDischgLmt,
//         pEMS->enableAntiOvld_REMS, pEMS->enableAntiRflx_REMS);
// }
static void S_Ems_Mgmt_Total_Pess_EMSClient(ROUGH_CCU_DATA* pCCU)
{
    // LJH241202 SEMS从机/ESC
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    VAR_VALUE varV;
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
    DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

    ERRCDDE_DPR err = pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1, 47, &varV, 4); //Priority
    pEMS->nPrio = (err == ERR_DPR_OK) ? varV.ulValue : 1;
    pEMS->bEnterUrgetDischrgMode = FALSE;
    
    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 407, 400, &varV, 4);
    pEMS->f_PcabMeter = (err == ERR_DPR_OK) ? varV.fValue : 0; //KW

    // LJH250930 加快自动充放响应,standbySetReq
    err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1001, 3800, &varV, 4);
    BOOL bStandby_Rx = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
    if (pCCU->lcCfg.emSystemType < SYSTYPE_EMS_STATION)
    {
        pEMS->bStandby_BLL = bStandby_Rx;
    }
    else
    {
        pEMS->bStandby_BLL = (bStandby_Rx || pEMS->bStandby_Set);
    }

    ZERO_POBJS(&pEMS->blobRev6330, 1);
    varV.blobValue = (BYTE*)&pEMS->blobRev6330;
    err = pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 
        200, 206, &varV, sizeof(A_BLOB_SEMS2SLAVE_6330));
    S63Uctl_30WorkPwr* pRev6330 = &pEMS->blobRev6330.stRev6330;
    U6330WorkAsap* pU6330WorkAsap = (U6330WorkAsap*)&pRev6330->byWorkAsap;
    pEMS->byWorkAsap = pU6330WorkAsap->byWorkAsapData;
    pEMS->bChrgAsap =  pU6330WorkAsap->bChrgAsap;
    pEMS->bDiscAsap =  pU6330WorkAsap->bDiscAsap;

    // LJH250612 planSector
    A_BLOB_SEMS_A6_USYS blobA6_USYS;
    ZERO_POBJS(&blobA6_USYS, 1);
    varV.blobValue = (BYTE*)&blobA6_USYS;
    err = pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 
        200, 207, &varV, sizeof(A_BLOB_SEMS_A6_USYS));
    if (err == ERR_DPR_OK)
    {
        pEMS->blobA6_USYS = blobA6_USYS;
    }
    SEMS_A6_USYS_DATA_INFO* pA6_USYS = &pEMS->blobA6_USYS.A6_USYS;
    ACDC_WORKSTATU emACDC_WorkStatuCtrl = pRev6330->emACDC_WorkStatuCtrl;
    if (pRev6330->fActv_PwrCtrl < MIN_PWR_PCS_OUT_REQ)
    {
        TRACEW("S_Ems_Mgmt_Total_Pess_EMSClient set emACDC_WorkStatuCtrl=STATU_ACDC_IDLE fActv_PwrCtrl:%.1f < 1.0f", pRev6330->fActv_PwrCtrl);
        emACDC_WorkStatuCtrl = STATU_ACDC_IDLE;
    }
    STBATT_PRI_WORKMODE emBatt_WorkModeCtrl = MapACDC_WorkStatu2BattWorkMode(emACDC_WorkStatuCtrl);
    // LJH241202 SEMS // 修改第0段EMSPlan // LJH250612 planSector
    PlanSector planSectorNow, planSectorOrg;
    SetPlanSector(&planSectorOrg, pA6_USYS->emBatt_WorkModePlan,
        pA6_USYS->nStopSOCPlan, pA6_USYS->nActvPwrLmtPlan, pA6_USYS->nReActvPwrLmtPlan, pA6_USYS->nBattGrpPwrLmtPlan);
    if (pCCU->lcCfg.emSystemType < SYSTYPE_EMS_STATION)
    {// ESC
        BOOL bModifiedByAsap = FALSE;
        // LJH250930 加快自动充放响应,WorkASAP
        if (pEMS->bChrgAsap && emACDC_WorkStatuCtrl==STATU_ACDC_RECTIFY)
        {
            TRACEW("S_Ems_Mgmt_Total_Pess_EMSClient bChrgAsap");
            SetPlanSector(&planSectorNow, emBatt_WorkModeCtrl, pEMS->nStopSOC_ChrgSet_SEMS, MAX_PWR_LMT, 0, MAX_PWR_LMT);
            bModifiedByAsap = TRUE;
        }
        if (pEMS->bChrgAsap && emACDC_WorkStatuCtrl==STATU_ACDC_INVERTER)
        {
            TRACEI("S_Ems_Mgmt_Total_Pess_EMSClient bChrgAsap");
            SetPlanSector(&planSectorNow, emBatt_WorkModeCtrl, pEMS->nStopSOC_DiscSet_SEMS, MAX_PWR_LMT, 0, MAX_PWR_LMT);
            bModifiedByAsap = TRUE;
        }
        if ( !bModifiedByAsap )
        {
            SetPlanSector(&planSectorNow, emBatt_WorkModeCtrl, pA6_USYS->nStopSOCPlan,
                pA6_USYS->nActvPwrLmtPlan, pA6_USYS->nReActvPwrLmtPlan, pA6_USYS->nBattGrpPwrLmtPlan);
        }
    }
    else
    {
        SetPlanSector(&planSectorNow, emBatt_WorkModeCtrl, pA6_USYS->nStopSOCPlan,
            MAX_PWR_LMT, 0, MAX_PWR_LMT);
    }
    // TRACED("S_Ems_Mgmt_Total_Pess_EMSClient ModifyBlobEmsCfgPlan before \n"
    //     "Org:emBatt_WorkMode:%d nStopSOC:%u, nActvPwrLmt:%u nBattGrpPwrLmt:%u \n"
    //     "Now:emBatt_WorkMode:%d nStopSOC:%u, nActvPwrLmt:%u nBattGrpPwrLmt:%u",
    //     planSectorOrg.emBatt_WorkModePlan, planSectorOrg.nStopSOCPlan, planSectorOrg.nActvPwrLmtPlan, planSectorOrg.nBattGrpPwrLmtPlan,
    //     planSectorNow.emBatt_WorkModePlan, planSectorNow.nStopSOCPlan, planSectorNow.nActvPwrLmtPlan, planSectorNow.nBattGrpPwrLmtPlan);
    ModifyBlobEmsCfgPlan(pCCU, FALSE, &planSectorNow, &planSectorOrg);
    GetCurrentTimeSector(pCCU, FALSE);
    // LJH250703 added:ESC最大功率限制
    if (emACDC_WorkStatuCtrl == STATU_ACDC_RECTIFY)
    {
        pEMS->fTotal_PessChgAvaliable = MIN(pRev6330->fActv_PwrCtrl, pCCU->fTotalPowerLimit);
        TRACEW("S_Ems_Mgmt_Total_Pess_EMSClient 6330:charge:%.1f", pEMS->fTotal_PessChgAvaliable);
    }
    else if (emACDC_WorkStatuCtrl == STATU_ACDC_INVERTER)
    {
        pEMS->fTotal_PessDischgLmt    = MIN(pRev6330->fActv_PwrCtrl, pCCU->fTotalPowerLimit);
        TRACEI("S_Ems_Mgmt_Total_Pess_EMSClient 6330:discharge:%.1f", pEMS->fTotal_PessDischgLmt);
    }
    TRACEI("S_Ems_Mgmt_Total_Pess_EMSClient 6330:emACDC_WorkStatuCtrl:%d %d 0:IDLE fActv_PwrCtrl:%.1f emBatt_WorkModeCtrl:%d "
        "byWorkAsap:%d byWorkAsap:%d bChrgAsap:%d bDiscAsap:%d bStandby_BLL:%d bStandby_Rx:%d||bStandby_Set:%d\n"
        "USYS: emBatt_WorkModePlan:%d nStopSOCPlan:%d nActvPwrLmtPlan:%u nBattGrpPwrLmtPlan:%u pEMS->emBatt_WorkModeReq:%d",
        pRev6330->emACDC_WorkStatuCtrl, emACDC_WorkStatuCtrl, pRev6330->fActv_PwrCtrl, emBatt_WorkModeCtrl,
        pRev6330->byWorkAsap, pEMS->byWorkAsap, pEMS->bChrgAsap, pEMS->bDiscAsap, pEMS->bStandby_BLL, bStandby_Rx, pEMS->bStandby_Set,
        pA6_USYS->emBatt_WorkModePlan, pA6_USYS->nStopSOCPlan, pA6_USYS->nActvPwrLmtPlan, pA6_USYS->nBattGrpPwrLmtPlan, pEMS->emBatt_WorkModeReq);
}

// LJH260131 增加top#6/top#7的ESC防逆流/防过载, 计算SEMS/ESC的Pess合在一起
void Ems_Mgmt_Total_Pess_All(ROUGH_CCU_DATA* pCCU)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    VAR_VALUE varV;
    ERRCDDE_DPR err;

    if (pCCU->lcCfg.lcType == LC_TYPE_1st)
    {
        // 一级主控 SEMS/ESC
        if (pEMS->enableREMSCtrl)
        {
            GetCtrlParam_REMS(pCCU);
            if (pEMS->clientInfoSum.emEMSCtrlMode_REMS == EMSCtrlMode_Remote)
            {
                pEMS->bCtrlByREMS = TRUE;
                // old S_Ems_Mgmt_Total_Pess_REMS(pCCU);
                DATA_PROCESSOR* pDPR = pCCU->pDPR;
                A_BLOB_SEMS_CLIENTINFO_SUM* pClientSum = &pEMS->clientInfoSum;

                err = pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1, 47, &varV, 4); //Priority
                pEMS->nPrio = (err == ERR_DPR_OK) ? varV.ulValue : 1;
                pEMS->bEnterUrgetDischrgMode = FALSE;

                // LJH251028 REMS,增加ACDCWorkStatu
                STBATT_PRI_WORKMODE emBatt_WorkModeReq = MapACDC_WorkStatu2BattWorkMode(pClientSum->emACDC_WorkStatu_REMS);
                BYTE nStopSoc;
                switch (pClientSum->emACDC_WorkStatu_REMS)
                {
                case STATU_ACDC_RECTIFY:
                    nStopSoc = pEMS->nStopSOC_ChrgSet_SEMS;
                    break;
                case STATU_ACDC_INVERTER:
                    nStopSoc = pEMS->nStopSOC_DiscSet_SEMS;
                    break;
                default:
                    nStopSoc = 0;
                    break;
                }
                
                // 修改第0段EMSPlan
                PlanSector planSectorNow;
                SetPlanSector(&planSectorNow, emBatt_WorkModeReq, (BYTE)nStopSoc, (UINT)(ABS(pClientSum->fActvPwrReq_REMS)), 0, MAX_PWR_LMT);
                TRACED("Ems_Mgmt_Total_Pess_All REMS ModifyBlobEmsCfgPlan planSectorNow emBatt_WorkMode:%d 5:idle nActvPwrLmtPlan:%u nBattGrpPwrLmtPlan:%u fActvPwrReq_REMS:%.1f nStopSoc:%d",
                    planSectorNow.emBatt_WorkModePlan, planSectorNow.nActvPwrLmtPlan, planSectorNow.nBattGrpPwrLmtPlan, pClientSum->fActvPwrReq_REMS, nStopSoc);
                ModifyBlobEmsCfgPlan(pCCU, FALSE, &planSectorNow, NULL);
                GetCurrentTimeSector(pCCU, FALSE);
                S_Ems_Mgmt_Total_Pess(pCCU);
                if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_CHRGING)
                {
                    if (pEMS->enableAntiOvld_REMS)
                    {
                        // 防过载
                        pEMS->fTotal_PessChgAvaliable = MIN(pEMS->fTotal_PessChgAvaliable, pClientSum->fActvPwrReq_REMS);
                    }
                    else
                    {
                        pEMS->fTotal_PessChgAvaliable = pClientSum->fActvPwrReq_REMS;
                    }
                }
                else if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_DISCHRG)
                {
                    float fActvPwrReqPositive = ABS(pClientSum->fActvPwrReq_REMS);
                    if (pEMS->enableAntiRflx_REMS)
                    {
                        // 防逆流
                        pEMS->fTotal_PessDischgLmt = MIN(pEMS->fTotal_PessDischgLmt, fActvPwrReqPositive);
                    }
                    else
                    {
                        pEMS->fTotal_PessDischgLmt = fActvPwrReqPositive;
                    }
                }
                TRACEI("Ems_Mgmt_Total_Pess_All REMS emBatt_WorkModeReq:%d fActvPwrReq_REMS:%.1f fTotal_PessChgAvaliable:%.1f fTotal_PessDischgLmt:%.1f "
                    "enableAntiOvld_REMS:%d enableAntiRflx_REMS:%d",
                    pEMS->emBatt_WorkModeReq, pClientSum->fActvPwrReq_REMS, pEMS->fTotal_PessChgAvaliable, pEMS->fTotal_PessDischgLmt,
                    pEMS->enableAntiOvld_REMS, pEMS->enableAntiRflx_REMS);
            }
            else
            {
                pEMS->bCtrlByREMS = FALSE;
            }
        }
        else
        {
            pEMS->bCtrlByREMS = FALSE;
        }
        if (!pEMS->bCtrlByREMS)
        {
            TRACED("Ems_Mgmt_Main_SEMS bCtrlByREMS==FALSE changedBlobEmsCfgPlan:%d bAutoDischrgEnable:%d bAutoChrgEnable:%d bEnter_AutoDisc_SEMS:%d bEnter_AutoChrg_SEMS:%d",
                pEMS->changedBlobEmsCfgPlan, pEMS->bAutoDischrgEnable, pEMS->bAutoChrgEnable, pEMS->bEnter_AutoDisc_SEMS, pEMS->bEnter_AutoChrg_SEMS);
            if (pEMS->changedBlobEmsCfgPlan)
            {
                ModifyBlobEmsCfgPlan(pCCU, TRUE, NULL, NULL); // restored
            }
            GetCurrentTimeSector(pCCU, TRUE);
            S_Ems_Mgmt_Total_Pess(pCCU);
            // LJH250612 增加EMS_LIMIT_SECTOR的参数，主控限制总功率nActvPwrLmt，ESC限制 nBattGrpPwrLmt
            if (pCCU->lcCfg.emSystemType == SYSTYPE_EMS_STATION)
            {
                pEMS->nBattGrpPwrLmt = pEMS->planSectorNow.nBattGrpPwrLmt; // LZY PID
                pEMS->planSectorNow.nActvPwrLmt =    MAX_PWR_LMT;
                pEMS->planSectorNow.nBattGrpPwrLmt = MAX_PWR_LMT; // Dispatch_ESSLmtBattChrg_Power DisPatch_ACLoadReq_DisChrg_Power
            }
            // LJH250930 加快自动充放响应,ChgDisOnly
            if (!NEED_AUTODISC() && !NEED_AUTOCHRG())
            {
                pEMS->bStandby_Auto = FALSE;
                pEMS->bEnter_AutoChrg_SEMS = FALSE;
                pEMS->bEnter_AutoDisc_SEMS = FALSE;
            }
            else
            {
                if (NEED_AUTODISC())
                {
                    AutoDischarging_IfOverload(pCCU);
                    pEMS->planSector4USYSIfAutoChDis = pEMS->planSectorNow;
                    // standby
                    if (ElapseS(pEMS->tmStandby_4Auto) < pEMS->uDelayStandby_4Disc)
                    {
                        pEMS->bStandby_Auto = TRUE;
                    }
                    else
                    {
                        pEMS->bStandby_Auto = FALSE;
                    }
                }
                else
                {
                    pEMS->bEnter_AutoDisc_SEMS = FALSE;
                }
                if (NEED_AUTOCHRG() && !pEMS->bEnter_AutoDisc_SEMS)
                {
                    AutoCharging_IfReflux(pCCU);
                    pEMS->planSector4USYSIfAutoChDis = pEMS->planSectorNow;
                    // standby
                    if (ElapseS(pEMS->tmStandby_4Auto) < pEMS->uDelayStandby_4Chrg)
                    {
                        pEMS->bStandby_Auto = TRUE;
                    }
                    else
                    {
                        pEMS->bStandby_Auto = FALSE;
                    }
                }
                else
                {
                    pEMS->bEnter_AutoChrg_SEMS = FALSE;
                }
            }
            if (pCCU->lcCfg.emSystemType == SYSTYPE_EMS_STATION)
            {
                pEMS->bStandby_BLL = (pEMS->planSectorOrg.emBatt_WorkMode==BATT_WORKMODE_STANDBY ||
                    pEMS->bStandby_Auto || pEMS->bStandby_Set || pEMS->bStandby_AEMS);
            }
            else
            {
                BOOL bStandby_BLL = (pEMS->planSectorOrg.emBatt_WorkMode==BATT_WORKMODE_STANDBY ||
                    pEMS->bStandby_Auto || pEMS->bStandby_Set || pEMS->bStandby_AEMS);
                if (pEMS->bStandby_BLL != bStandby_BLL)
                {
                    pEMS->bStandby_BLL = bStandby_BLL;
                    varV.emValue = (_ENUM)pEMS->bStandby_BLL;
                    Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1001, 3800, varV);
                }

                // LJH251127 单柜自动充放电
                varV.emValue = (!pEMS->bEnter_AutoChrg_SEMS && !pEMS->bEnter_AutoDisc_SEMS ? 0:1);
                Push_VarValue(SETTYPE_INTERN_CTRL,  pCCU->hSampler, 501, 527, varV);
            }
        }
        // LJH250930 加快自动充放响应
        pEMS->bChrgAsap = (pEMS->bCtrlByREMS);
        pEMS->bDiscAsap = (pEMS->bCtrlByREMS);
        if (pCCU->lcCfg.emSystemType == SYSTYPE_EMS_STATION)
        {
            pEMS->bChrgAsap = (pEMS->bChrgAsap || pEMS->bEnter_AutoChrg_SEMS);
            pEMS->bDiscAsap = (pEMS->bDiscAsap || pEMS->bEnter_AutoDisc_SEMS);
        }
        pEMS->byWorkAsap = Make_WorkAsap(pEMS->bChrgAsap, pEMS->bDiscAsap).byWorkAsapData;
        
        TRACED("Ems_Mgmt_Main_SEMS bCtrlByREMS:%d changedBlobEmsCfgPlan:%d bChrgAsap:%d bDiscAsap:%d \n"
            "bStandby_BLL:%d bStandby_Auto:%d bStandby_Set:%d bEnter_AutoChrg_SEMS:%d bEnter_AutoDisc_SEMS:%d",
            pEMS->bCtrlByREMS, pEMS->changedBlobEmsCfgPlan, pEMS->bChrgAsap, pEMS->bDiscAsap,
            pEMS->bStandby_BLL, pEMS->bStandby_Auto, pEMS->bStandby_Set, pEMS->bEnter_AutoChrg_SEMS, pEMS->bEnter_AutoDisc_SEMS);
    }
    else
    {
        // 从机 LJH0325 AntiOvldRflxByLC
        if (pEMS->enableAntiOvld_REMS || pEMS->enableAntiRflx_REMS)
        {
            // LJH250812 added:for Calc_ESS_ChgLmtPower Calc_ESS_DischgLmtPower计算pess
            pEMS->nTMSector = 0;
            pEMS->planSectorNow.nActvPwrLmt = MAX_PWR_LMT;
            S_Ems_Mgmt_Total_Pess(pCCU);
            pEMS->fTotal_PessDischgLmt_Org =    pEMS->fTotal_PessDischgLmt;
            pEMS->fTotal_PessChgAvaliable_Org = pEMS->fTotal_PessChgAvaliable;
            S_Ems_Mgmt_Total_Pess_EMSClient(pCCU);
            if (pEMS->enableAntiOvld_REMS)
            {
                pEMS->fTotal_PessChgAvaliable = MIN(pEMS->fTotal_PessChgAvaliable, pEMS->fTotal_PessChgAvaliable_Org);
                TRACEW("Ems_Mgmt_Main_SEMS enableAntiOvld_REMS fTotal_PessChgAvaliable_Org:%.1f fTotal_PessChgAvaliable:%.1f",
                    pEMS->fTotal_PessChgAvaliable_Org, pEMS->fTotal_PessChgAvaliable);
            }
            if (pEMS->enableAntiRflx_REMS)
            {
                pEMS->fTotal_PessDischgLmt = MIN(pEMS->fTotal_PessDischgLmt, pEMS->fTotal_PessDischgLmt_Org);
                TRACEI("Ems_Mgmt_Main_SEMS enableAntiRflx_REMS fTotal_PessDischgLmt_Org:%.1f fTotal_PessDischgLmt:%.1f",
                    pEMS->fTotal_PessDischgLmt_Org, pEMS->fTotal_PessDischgLmt);
            }
        }
        else
        {
            Refresh_RealTime_MeterValue(pCCU);
            Ems_init(pCCU);
            S_Ems_Mgmt_Total_Pess_EMSClient(pCCU);
        }
    }
}

U6330WorkAsap Make_WorkAsap(BOOL bChrgAsap, BOOL bDiscAsap)
{
    U6330WorkAsap workASAP;
	workASAP.byWorkAsapData = 0;
	workASAP.bChrgAsap = bChrgAsap ? 1:0;
	workASAP.bDiscAsap = bDiscAsap ? 1:0;
	return workASAP;
}

STBATT_PRI_WORKMODE MapACDC_WorkStatu2BattWorkMode(ACDC_WORKSTATU emACDC_WorkStatu)
{
    static STBATT_PRI_WORKMODE mapBatt_WorkModes[STATU_ACDC_MAX] = {
            BATT_WORKMODE_IDLE,          // STATU_ACDC_IDLE
            BATT_WORKMODE_FIRST_CHRGING, // STATU_ACDC_RECTIFY
            BATT_WORKMODE_FIRST_DISCHRG, // STATU_ACDC_INVERTER
            BATT_WORKMODE_STANDBY        // STATU_ACDC_STANDBY
            };
    emACDC_WorkStatu = (emACDC_WorkStatu<STATU_ACDC_MAX ? emACDC_WorkStatu : STATU_ACDC_IDLE);
    return mapBatt_WorkModes[emACDC_WorkStatu];
}

ACDC_WORKSTATU MapBattWorkMode2ACDC_WorkStatu(STBATT_PRI_WORKMODE emBatt_WorkMode)
{
    static ACDC_WORKSTATU mapACDC_WorkStatus[BATT_WORKMODE_MAX] = {
		STATU_ACDC_INVERTER, // BATT_WORKMODE_FIRST_DISCHRG
		STATU_ACDC_RECTIFY,  // BATT_WORKMODE_FIRST_CHRGING
        STATU_ACDC_INVERTER, // BATT_WORKMODE_DISCHRG_ONLY
		STATU_ACDC_RECTIFY,  // BATT_WORKMODE_CHRGING_ONLY
        STATU_ACDC_STANDBY,  // BATT_WORKMODE_STANDBY
        STATU_ACDC_IDLE      // BATT_WORKMODE_IDLE
		};
    emBatt_WorkMode = (emBatt_WorkMode<BATT_WORKMODE_MAX ? emBatt_WorkMode : BATT_WORKMODE_IDLE);
    return mapACDC_WorkStatus[emBatt_WorkMode];
}

GRP_RECT_CTRL_CMDTYPE MapBattWorkMode2GrpRectCtrlCmdType(STBATT_PRI_WORKMODE emBatt_WorkMode)
{
    static GRP_RECT_CTRL_CMDTYPE mapGrpRectCtrlCmdType[BATT_WORKMODE_MAX] = {
		GRP_ACDC_INVERT_START,   // BATT_WORKMODE_FIRST_DISCHRG
		GRP_ACDC_RECTIFY_START,  // BATT_WORKMODE_FIRST_CHRGING
        GRP_ACDC_INVERT_START,   // BATT_WORKMODE_DISCHRG_ONLY
		GRP_ACDC_RECTIFY_START,  // BATT_WORKMODE_CHRGING_ONLY
        GRP_ACDC_INVERT_PENDING, // BATT_WORKMODE_STANDBY
        GRP_ACDC_STOP            // BATT_WORKMODE_IDLE
		};
    emBatt_WorkMode = (emBatt_WorkMode<BATT_WORKMODE_MAX ? emBatt_WorkMode : BATT_WORKMODE_IDLE);
    return mapGrpRectCtrlCmdType[emBatt_WorkMode];
}

void AutoCharging_IfReflux(ROUGH_CCU_DATA* pCCU)
{
    // 逆流自动启动充电
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    // LJH241202 SEMS AutoChgDis
    float fPreProt = pCCU->blobBackendCfg.fPreProt_Chrg;
    A_BLOB_SEMS_CLIENTINFO_SUM* pClientSum = &pEMS->clientInfoSum;
    pEMS->fTargetPwr_4Chrg_SEMS = MIN(pEMS->fTargetPwr_4Chrg_SEMS, pCCU->blobBackendCfg.fSysACInput_PwrLmt);
    // float fCalPessTrgt = pEMS->fTargetPwr_4Chrg_SEMS - pEMS->fAtRflx_kW + pEMS->f_PcabMeter;
    float fCalPessTrgt = pEMS->fTargetPwr_4Chrg_SEMS - pEMS->fPacload4Dischg;
    float fCalPessOffset = fCalPessTrgt + pEMS->fTargetPwr_4ChrgOffset_SEMS;
    fCalPessOffset = MAX(fCalPessOffset, MIN_PWR_KEEPWORKING);
    fCalPessOffset = MIN(fCalPessOffset, pClientSum->fMax_Chrg_PwrAbilitySum0);
    static time_t s_tmLastRflx_Enter, s_tmLastRflx_Exit;
    if (fCalPessTrgt <= MIN_PWR_KEEPWORKING)
    {
        s_tmLastRflx_Exit = _NOW_;
    }
    if (fCalPessTrgt > 0)
    {
        s_tmLastRflx_Enter = _NOW_;
    }
    if (ElapseS(s_tmLastRflx_Exit) > MAX_PERIOD_ENTER_WORKING)
    {
        pEMS->bEnter_AutoChrg_SEMS = TRUE;
    }
    // if (ElapseS(s_tmLastRflx_Enter) > MAX_PERIOD_DELAY_WORKING ||
    //     (pEMS->bEnter_OverLoad && (pEMS->f_PcabMeter - fPreProt<=0)))
    if (ElapseS(s_tmLastRflx_Enter) > MAX_PERIOD_DELAY_WORKING)
    {
        TRACEW("AutoCharging_IfReflux set bEnter_AutoChrg_SEMS:FALSE %d ||(%d && %d)",
            ElapseS(s_tmLastRflx_Enter) > MAX_PERIOD_DELAY_WORKING, pEMS->bEnter_OverLoad, pEMS->f_PcabMeter - fPreProt<=0);
        pEMS->bEnter_AutoChrg_SEMS = FALSE;
    }
    // LJH250625 fix:光伏逆流值小于管理计划的功率值不启动自动充电
    EMS_LIMIT_SECTOR* pPlanSectorOrg = &pEMS->planSectorOrg;
    float fTotalPessPlanOrg_Chrg = 0;
    // if (pPlanSectorOrg->emBatt_WorkMode == BATT_WORKMODE_FIRST_CHRGING && pClientSum->nSOCMinSum<=pPlanSectorOrg->nStopSOC)
    if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_CHRGING)
    {
        fTotalPessPlanOrg_Chrg = (float)MIN(pPlanSectorOrg->nActvPwrLmt, pPlanSectorOrg->nBattGrpPwrLmt*pCCU->lcCfg.nESCNum);
        fTotalPessPlanOrg_Chrg = MIN(pEMS->fTotal_PessChgAvaliable, fTotalPessPlanOrg_Chrg);
    }
    if (pEMS->bEnter_AutoChrg_SEMS && fCalPessTrgt < fTotalPessPlanOrg_Chrg+1.0f)
    {
        TRACEW("AutoCharging_IfReflux bAutoChrg=FALSE fCalPessTrgt:%.1f fTotalPessPlanOrg_Chrg:%.1f",
            fCalPessTrgt, fTotalPessPlanOrg_Chrg);
        pEMS->bEnter_AutoChrg_SEMS = FALSE;
    }
    // LJH250930 加快自动充放响应
        float fPess2Standby = pEMS->fPacload4Dischg;
        static time_t s_tmLastStandby_Exit;
        if (fPess2Standby > pEMS->fLtPwr2Standby_4Chrg)
        {
            s_tmLastStandby_Exit = _NOW_;
        }
        if (ElapseS(s_tmLastStandby_Exit) > MAX_PERIOD_ENTER_WORKING && !pEMS->hasPCSDischrgLast_SEMS && !pEMS->bEnterUrgetDischrgMode)
        {
            pEMS->tmStandby_4Auto = _NOW_;
        }
    TRACED("AutoCharging_IfReflux bEnter_AutoChrg_SEMS:%d fTotal_PessChgAvaliable:%.1f fTotalPessPlanOrg_Chrg:%.1f "
        "fCalPessTrgt:%.1f=fTargetPwr_4Chrg_SEMS:%.1f-fPacload4Dischg:%.1f "
        "fCalPessOffset:%.1f=MAX(%.1f+%.1f:%.1f) MIN fMax_Chrg_PwrAbilitySum:%.1f "
        "fTargetPwr_4Chrg_SEMS:%.1f hasPCSDischrgLast_SEMS:%d "
        "ElapseS Exit:%u >%d:TRUE Enter:%u >%d:FALSE bEnter_OverLoad:%d %.1f"
        "emBatt_WorkModeReq:%d 5:idle bEnterUrgetDischrgMode:%d OutPwr:cab:%.1f PCS:%.1f "
        "fPess2Standby:%.1f ElapseSStandby:%u bStandby_Auto:%d", 
        pEMS->bEnter_AutoChrg_SEMS, pEMS->fTotal_PessChgAvaliable, fTotalPessPlanOrg_Chrg,
        fCalPessTrgt, pEMS->fTargetPwr_4Chrg_SEMS, pEMS->fPacload4Dischg,
        fCalPessOffset, fCalPessTrgt, pEMS->fTargetPwr_4ChrgOffset_SEMS, MIN_PWR_KEEPWORKING, pClientSum->fMax_Chrg_PwrAbilitySum,
        pEMS->fTargetPwr_4Chrg_SEMS, pEMS->hasPCSDischrgLast_SEMS,
        ElapseS(s_tmLastRflx_Exit), MAX_PERIOD_ENTER_WORKING, ElapseS(s_tmLastRflx_Enter), MAX_PERIOD_DELAY_WORKING, pEMS->bEnter_OverLoad, pEMS->f_PcabMeter-fPreProt,
        pEMS->emBatt_WorkModeReq, pEMS->bEnterUrgetDischrgMode, pEMS->f_PcabMeter, pClientSum->fActvPwr_PCSGrpSum,
        fPess2Standby, ElapseS(pEMS->tmStandby_4Auto), pEMS->bStandby_Auto);
#ifdef LOG_FILEDEBUG
        static BOOL bEnter_AutoChrg_SEMS = FALSE;
        static time_t stmLogFile = 0;
        if (ElapseS(stmLogFile) > INTV_S_LOG_DEBUGFILE || bEnter_AutoChrg_SEMS != pEMS->bEnter_AutoChrg_SEMS)
        {
            stmLogFile = _NOW_;
            bEnter_AutoChrg_SEMS = pEMS->bEnter_AutoChrg_SEMS;
            LOGFD("AutoCharging_IfReflux bEnter_AutoChrg_SEMS:%d bEnterAnti_Reflux:%d fTotal_PessChgAvaliable:%.1f fTotalPessPlanOrg_Chrg:%.1f "
                "fCalPessTrgt:%.1f=fTargetPwr_4Chrg_SEMS:%.1f-fPacload4Dischg:%.1f "
                "fCalPessOffset:%.1f=MAX(%.1f+%.1f:%.1f) MIN fMax_Chrg_PwrAbilitySum:%.1f "
                "fTargetPwr_4Chrg_SEMS:%.1f hasPCSDischrgLast_SEMS:%d "
                "ElapseS Exit:%u >%d:TRUE Enter:%u >%d:FALSE bEnter_OverLoad:%d %.1f"
                "emBatt_WorkModeReq:%d 5:idle bEnterUrgetDischrgMode:%d OutPwr:cab:%.1f PCS:%.1f "
                "fPess2Standby:%.1f ElapseSStandby:%u bStandby_Auto%d", 
                pEMS->bEnter_AutoChrg_SEMS, pEMS->bEnterAnti_Reflux, pEMS->fTotal_PessChgAvaliable, fTotalPessPlanOrg_Chrg,
                fCalPessTrgt, pEMS->fTargetPwr_4Chrg_SEMS, pEMS->fPacload4Dischg,
                fCalPessOffset, fCalPessTrgt, pEMS->fTargetPwr_4ChrgOffset_SEMS, MIN_PWR_KEEPWORKING, pClientSum->fMax_Chrg_PwrAbilitySum,
                pEMS->fTargetPwr_4Chrg_SEMS, pEMS->hasPCSDischrgLast_SEMS,
                ElapseS(s_tmLastRflx_Exit), MAX_PERIOD_ENTER_WORKING, ElapseS(s_tmLastRflx_Enter), MAX_PERIOD_DELAY_WORKING, pEMS->bEnter_OverLoad, pEMS->f_PcabMeter-fPreProt,
                pEMS->emBatt_WorkModeReq, pEMS->bEnterUrgetDischrgMode, pEMS->f_PcabMeter, pClientSum->fActvPwr_PCSGrpSum,
                fPess2Standby, ElapseS(pEMS->tmStandby_4Auto), pEMS->bStandby_Auto);
        }
#endif
    // LJH251122 added:|| pEMS->bEnterAnti_Reflux LJH251208 deleted 会造成放电逆流后就停止放电问题
    // LJH251208 modified old pEMS->bEnter_AutoChrg_SEMS = ((pEMS->bEnter_AutoChrg_SEMS || pEMS->bEnterAnti_Reflux) && !pEMS->hasPCSDischrgLast_SEMS && !pEMS->bEnterUrgetDischrgMode); // Send_A6USYS
    pEMS->bEnter_AutoChrg_SEMS = (pEMS->bEnter_AutoChrg_SEMS && !pEMS->bEnterUrgetDischrgMode);
    if (pEMS->bEnter_AutoChrg_SEMS)
    {
        pEMS->fTotal_PessChgAvaliable = fCalPessOffset; // LJH250507 修改LJH250407版本逆流时要设置管理计划充电才能充电
        // LJH250407 old pEMS->fTotal_PessChgAvaliable = MIN(fCalPessOffset, pEMS->fTotal_PessChgAvaliable);
        // 修改第0段EMSPlan
        PlanSector planSectorNow;
        // SetPlanSector(&planSectorNow, BATT_WORKMODE_FIRST_CHRGING, MAX_STOPSOC_CHRG, MAX_PWR_LMT, 0, MAX_PWR_LMT);
        SetPlanSector(&planSectorNow, BATT_WORKMODE_FIRST_CHRGING, pEMS->nStopSOC_ChrgSet_SEMS, MAX_PWR_LMT, 0, MAX_PWR_LMT);
        ModifyBlobEmsCfgPlan(pCCU, FALSE, &planSectorNow, NULL);
        GetCurrentTimeSector(pCCU, FALSE);
        pEMS->tmStandby_4Auto = _NOW_;
        TRACEW("AutoCharging_IfReflux Charging fTotal_PessChgAvaliable:%.1f fPess2Standby:%.1f ElapseSStandby:%u",
            pEMS->fTotal_PessChgAvaliable, fPess2Standby, ElapseS(pEMS->tmStandby_4Auto));
    }
}

void AutoDischarging_IfOverload(ROUGH_CCU_DATA* pCCU)
{
    //   过载自动启动放电使能
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    // LJH241202 SEMS AutoChgDis
    // float fPreProt = pCCU->blobBackendCfg.fPreProt4RefluxAndOverLoad;
    A_BLOB_SEMS_CLIENTINFO_SUM* pClientSum = &pEMS->clientInfoSum;
    // float fCalPessTrgt = pEMS->fAtOvl_kW - pEMS->fTargetPwr_4Dischrg_SEMS - pEMS->f_PcabMeter;
    float fCalPessTrgt = pEMS->fPacload4Chg - pEMS->fTargetPwr_4Dischrg_SEMS;
    float fCalPessOffset = fCalPessTrgt + pEMS->fTargetPwr_4DischrgOffset_SEMS;
    fCalPessOffset = MAX(fCalPessOffset, MIN_PWR_KEEPWORKING);
    fCalPessOffset = MIN(fCalPessOffset, pClientSum->fMax_DisChrg_PwrAbilitySum0);
    static time_t s_tmLastOvld_Enter, s_tmLastOvld_Exit;
    if (fCalPessTrgt <= MIN_PWR_KEEPWORKING)
    {
        s_tmLastOvld_Exit = _NOW_;
    }
    if (fCalPessTrgt > 0)
    {
        s_tmLastOvld_Enter = _NOW_;
    }
    if (ElapseS(s_tmLastOvld_Exit) > MAX_PERIOD_ENTER_WORKING)
    {
        pEMS->bEnter_AutoDisc_SEMS = TRUE;
    }
    // if (ElapseS(s_tmLastOvld_Enter) > MAX_PERIOD_DELAY_WORKING ||
    //     (pEMS->bEnterAnti_Reflux && (pEMS->fAtRflx_kW - pEMS->f_PcabMeter<=0)))
    if (ElapseS(s_tmLastOvld_Enter) > MAX_PERIOD_DELAY_WORKING)
    {
        TRACEW("AutoDischarging_IfOverload set FALSE %d ||(%d && %d)",
            ElapseS(s_tmLastOvld_Enter) > MAX_PERIOD_DELAY_WORKING, pEMS->bEnterAnti_Reflux, pEMS->fAtRflx_kW - pEMS->f_PcabMeter<=0);
        pEMS->bEnter_AutoDisc_SEMS = FALSE;
    }
    // LJH250625 fix
    EMS_LIMIT_SECTOR* pPlanSectorOrg = &pEMS->planSectorOrg;
    float fTotalPessPlanOrg_Dischrg = 0;
    // if (pPlanSectorOrg->emBatt_WorkMode == BATT_WORKMODE_FIRST_DISCHRG)
    if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_DISCHRG)
    {
        fTotalPessPlanOrg_Dischrg = (float)MIN(pPlanSectorOrg->nActvPwrLmt, pPlanSectorOrg->nBattGrpPwrLmt*pCCU->lcCfg.nESCNum);
        fTotalPessPlanOrg_Dischrg = MIN(pEMS->fTotal_PessDischgLmt, fTotalPessPlanOrg_Dischrg);
    }
    if (pEMS->bEnter_AutoDisc_SEMS && fCalPessTrgt < fTotalPessPlanOrg_Dischrg+1.0f)
    {
        pEMS->bEnter_AutoDisc_SEMS = FALSE;
        TRACEW("AutoDischarging_IfOverload bAutoDischrg=FALSE fCalPessTrgt:%.1f fTotalPessPlanOrg_Dischrg:%.1f",
            fCalPessTrgt, fTotalPessPlanOrg_Dischrg);
    }
    // LJH250930 加快自动充放响应
        float fPess2Standby = pEMS->fPacload4Chg;
        static time_t s_tmLastStandby_Exit;
        if (fPess2Standby < pEMS->fGtPwr2Standby_4Disc)
        {
            s_tmLastStandby_Exit = _NOW_;
        }
        if (ElapseS(s_tmLastStandby_Exit) > MAX_PERIOD_ENTER_WORKING && !pEMS->hasPCSChrgLast_SEMS)
        {
            pEMS->tmStandby_4Auto = _NOW_;
        }
    TRACED("AutoDischarging_IfOverload bEnter_AutoDisc_SEMS:%d fTotal_PessDischgLmt:%.1f fTotalPessPlan_DischrgReq:%.1f "
        "fCalPessTrgt:%.1f=fPacload4Chg:%.1f-fTargetPwr_4Dischrg_SEMS:%.1f "
        "fCalPessOffset:%.1f=MAX(%.1f+%.1f:%.1f) MIN fMax_DisChrg_PwrAbilitySum:%.1f "
        "fTargetPwr_4Dischrg_SEMS:%.1f hasPCSDischrgLast_SEMS:%d "
        "ElapseS Exit:%u >%d:TRUE Enter:%u >%d:FALSE "
        "emBatt_WorkModeReq:%d 5:idle OutPwr:cab:%.1f PCS:%.1f "
        "fPess2Standby:%.1f ElapseSStandby:%u bStandby_Auto:%d",
        pEMS->bEnter_AutoDisc_SEMS, pEMS->fTotal_PessDischgLmt, fTotalPessPlanOrg_Dischrg,
        fCalPessTrgt, pEMS->fPacload4Chg, pEMS->fTargetPwr_4Dischrg_SEMS,
        fCalPessOffset, fCalPessTrgt, pEMS->fTargetPwr_4DischrgOffset_SEMS, MIN_PWR_KEEPWORKING, pClientSum->fMax_DisChrg_PwrAbilitySum,
        pEMS->fTargetPwr_4Dischrg_SEMS, pEMS->hasPCSDischrgLast_SEMS,
        ElapseS(s_tmLastOvld_Exit), MAX_PERIOD_ENTER_WORKING, ElapseS(s_tmLastOvld_Enter), MAX_PERIOD_DELAY_WORKING,
        pEMS->emBatt_WorkModeReq, pEMS->f_PcabMeter, pClientSum->fActvPwr_PCSGrpSum,
        fPess2Standby, ElapseS(pEMS->tmStandby_4Auto), pEMS->bStandby_Auto);
#ifdef LOG_FILEDEBUG
        static BOOL bEnter_AutoDisc_SEMS = FALSE;
        static time_t stmLogFile = 0;
        if (ElapseS(stmLogFile) > INTV_S_LOG_DEBUGFILE || bEnter_AutoDisc_SEMS != pEMS->bEnter_AutoDisc_SEMS)
        {
            stmLogFile = _NOW_;
            bEnter_AutoDisc_SEMS = pEMS->bEnter_AutoDisc_SEMS;
            LOGFD("AutoDischarging_IfOverload bEnter_AutoDisc_SEMS:%d bEnter_OverLoad:%d fTotal_PessDischgLmt:%.1f fTotalPessPlan_DischrgReq:%.1f "
                "fCalPessTrgt:%.1f=fPacload4Chg:%.1f-fTargetPwr_4Dischrg_SEMS:%.1f "
                "fCalPessOffset:%.1f=MAX(%.1f+%.1f:%.1f) MIN fMax_DisChrg_PwrAbilitySum:%.1f "
                "fTargetPwr_4Dischrg_SEMS:%.1f hasPCSDischrgLast_SEMS:%d "
                "ElapseS Exit:%u >%d:TRUE Enter:%u >%d:FALSE "
                "emBatt_WorkModeReq:%d 5:idle OutPwr:cab:%.1f PCS:%.1f "
                "fPess2Standby:%.1f ElapseSStandby:%u bStandby_Auto:%d",
                pEMS->bEnter_AutoDisc_SEMS, pEMS->bEnter_OverLoad, pEMS->fTotal_PessDischgLmt, fTotalPessPlanOrg_Dischrg,
                fCalPessTrgt, pEMS->fPacload4Chg, pEMS->fTargetPwr_4Dischrg_SEMS,
                fCalPessOffset, fCalPessTrgt, pEMS->fTargetPwr_4DischrgOffset_SEMS, MIN_PWR_KEEPWORKING, pClientSum->fMax_DisChrg_PwrAbilitySum,
                pEMS->fTargetPwr_4Dischrg_SEMS, pEMS->hasPCSDischrgLast_SEMS,
                ElapseS(s_tmLastOvld_Exit), MAX_PERIOD_ENTER_WORKING, ElapseS(s_tmLastOvld_Enter), MAX_PERIOD_DELAY_WORKING,
                pEMS->emBatt_WorkModeReq, pEMS->f_PcabMeter, pClientSum->fActvPwr_PCSGrpSum,
                fPess2Standby, ElapseS(pEMS->tmStandby_4Auto), pEMS->bStandby_Auto);
        }
#endif
    // LJH251122 added:|| pEMS->bEnter_OverLoad
    // old pEMS->bEnter_AutoDisc_SEMS = ((pEMS->bEnter_AutoDisc_SEMS || pEMS->bEnter_OverLoad) && !pEMS->hasPCSChrgLast_SEMS); // Send_A6USYS
    if (pEMS->bEnter_AutoDisc_SEMS)
    {
        pEMS->fTotal_PessDischgLmt = fCalPessOffset; // LJH250507 修改
        // LJH250407 old pEMS->fTotal_PessDischgLmt = MIN(fCalPessOffset, pEMS->fTotal_PessDischgLmt);
        // 修改第0段EMSPlan
        PlanSector planSectorNow;
        // SetPlanSector(&planSectorNow, BATT_WORKMODE_FIRST_DISCHRG, MIN_STOPSOC_DISC, MAX_PWR_LMT, 0, MAX_PWR_LMT);
        SetPlanSector(&planSectorNow, BATT_WORKMODE_FIRST_DISCHRG, pEMS->nStopSOC_DiscSet_SEMS, MAX_PWR_LMT, 0, MAX_PWR_LMT);
        ModifyBlobEmsCfgPlan(pCCU, FALSE, &planSectorNow, NULL);
        GetCurrentTimeSector(pCCU, FALSE);
        pEMS->tmStandby_4Auto = _NOW_;
        TRACEI("AutoDischarging_IfOverload Discharging fTotal_PessDischgLmt:%.1f fPess2Standby:%.1f ElapseSStandby:%u",
            pEMS->fTotal_PessDischgLmt, fPess2Standby, ElapseS(pEMS->tmStandby_4Auto));
    }
}

// LZY260115 PID Initial
void PID_Init(ROUGH_CCU_DATA *pCCU) 
{
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    PID_Ctx *pPidCtx = &pEMS->stPidCtx;
    PID_Params *pPidChgParams = &pPidCtx->stPIDChgParams;
    PID_Params *pPidDischgParams = &pPidCtx->stPIDDischgParams;
    DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

    VAR_VALUE varV;
    ERRCDDE_DPR err;
    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 300, &varV, 4);
	pEMS->emPIDEnableState = (err==ERR_DPR_OK) ? varV.emValue : POWER_CTRL_DISABLE;
    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 301, &varV, 4);
	pEMS->emHighSensitivityEnable = (err==ERR_DPR_OK) ? varV.emValue : POWER_CTRL_DISABLE;

    // printf("PID_Init emPIDEnableState:[%d],emHighSensitivityEnable:[%d]", pEMS->emPIDEnableState, pEMS->emHighSensitivityEnable);

    if (pEMS->emPIDEnableState>POWER_CTRL_DISABLE)
    {
        pEMS->emHighSensitivityEnable = pEMS->emPIDEnableState | pEMS->emHighSensitivityEnable;
    }
    
    // 1. 充电参数
    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 302, &varV, 4);
    pPidChgParams->Kp = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;

    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 303, &varV, 4);
    pPidChgParams->Ki = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;

    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 304, &varV, 4);
    pPidChgParams->fDead_Band = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;

    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 305, &varV, 4);
    pPidChgParams->fProportional_Term_Sepa_Thres = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;

    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 306, &varV, 4);
    pPidChgParams->fIntegral_Term_Sepa_Thres = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;

    // 2. 放电参数
    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 307, &varV, 4);
    pPidDischgParams->Kp = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;

    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 308, &varV, 4);
    pPidDischgParams->Ki = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;

    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 309, &varV, 4);
    pPidDischgParams->fDead_Band = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
    
    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 310, &varV, 4);
    pPidDischgParams->fProportional_Term_Sepa_Thres = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
    
    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 311, &varV, 4);
    pPidDischgParams->fIntegral_Term_Sepa_Thres = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;

    // 3. 更新其他上下文信息
    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 312, &varV, 4);   // 补偿清除阈值
    pPidCtx->fClearCompensationThres = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;

    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 313, &varV, 4);   // 绝对最大输出限幅
    pPidCtx->fMaxOutput_Lmt = (err==ERR_DPR_OK) ? varV.fValue : 200.0f;

    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 314, &varV, 4);   // 绝对最小输出限幅
    pPidCtx->fMinOutput_Lmt = (err==ERR_DPR_OK) ? varV.fValue : -200.0f;

    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 315, &varV, 4);   // 补偿值限幅
    pPidCtx->fCompensation_Lmt = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;

    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 316, &varV, 4);   // 初始补偿系数
    pPidCtx->fInitCompCoef = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;

    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 317, &varV, 4);   // 目标值稳定计数阈值
    pPidCtx->nTargetStableCount_Thres = (UINT)(err==ERR_DPR_OK) ? varV.ulValue : 5;

    err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 318, &varV, 4);   // 目标值稳定阈值
    pPidCtx->fTargetStableThres = (err==ERR_DPR_OK) ? varV.fValue : 1.0f;

    pPidCtx->fCompensation = 0.0f;
    pPidCtx->fPrevError = 0.0f;
    pPidCtx->fPrevPrevError = 0.0f;
    pPidCtx->fErrorAvgFilter = 0.0f;
    pPidCtx->fPrevTarget = 0.0f;
}

void Power_Control_Loop(ROUGH_CCU_DATA *pCCU)
{
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    PID_Ctx *pPID_Ctx = &pEMS->stPidCtx;
    DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
    PID_Params *pPidParams = NULL;
    
    BOOL bEnablePID = FALSE;
    float target_power = 0.0f;
    float meter_power = pEMS->f_PcabMeter;
    float error = 0.0f;

    VAR_VALUE varV;
    ERRCDDE_DPR err;

    // 只对充放电、PID使能工况条件下生效
    if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_CHRGING)
    {
        pPidParams = &pPID_Ctx->stPIDChgParams;
        target_power = pEMS->fTotal_PessChgAvaliable;
        if (pEMS->emPIDEnableState == POWER_CTRL_CHARGE || pEMS->emPIDEnableState == POWER_CTRL_CAHARGE_DISCHARGE) bEnablePID=TRUE;
    }
    else if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_DISCHRG)
    {
        pPidParams = &pPID_Ctx->stPIDDischgParams;
        target_power = -pEMS->fTotal_PessDischgLmt;
        if (pEMS->emPIDEnableState>POWER_CTRL_CHARGE) bEnablePID=TRUE;
    }
    // 恒定负载时启用
#if 0
    else if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_STANDBY || pEMS->emBatt_WorkModeReq == BATT_WORKMODE_IDLE)
    {
        error = 0.0f - meter_power;
        pPID_Ctx->fErrorAvgFilter = Samp_Avg_Filter(FILTERTYPE_PID_ERROR, error, 25);  // TODO: 滤波点数需要优化(optinal)
    }
#endif

    // double tmNow = GetCurrentTime();

    if (!bEnablePID)
    {
        pEMS->fTotal_PessChg_PidOut = pEMS->fTotal_PessChgAvaliable;
        pEMS->fTotal_PessDischg_PidOut = pEMS->fTotal_PessDischgLmt;

        pPID_Ctx->fPrevError = 0.0f;
        pPID_Ctx->fPrevPrevError = 0.0f;
        pPID_Ctx->fCompensation = 0.0f;
        pPID_Ctx->fPrevTarget = 0.0f;

        pEMS->nTargetStableCount = 0;

        // printf("PID_DATA,%.3f,%.3f,%.3f,%.3f\n", 
        //     tmNow,
        //     target_power, 
        //     meter_power,
        //     target_power);
        
        return;
    }

    // 目标值处于稳态时才启用PID调节作用
    if (fabsf(target_power - pPID_Ctx->fPrevTarget)>pPID_Ctx->fTargetStableThres)
    {
        pEMS->nTargetStableCount = 0;
    }
    else
    {
        if (pEMS->nTargetStableCount < (pPID_Ctx->nTargetStableCount_Thres + 5))
        {
            pEMS->nTargetStableCount++;
        }
    }

    float fTotalPwoer_Lmt = (float)pEMS->nBattGrpPwrLmt * pEMS->nNodeNum;
    // printf("target_power:[%.3f], nBattGrpPwrLmt:[%.3f], nNodeNum:[%d]", target_power, (float)pEMS->nBattGrpPwrLmt, pEMS->nNodeNum);
    if ((pEMS->nTargetStableCount < pPID_Ctx->nTargetStableCount_Thres) || 
    (fabsf(target_power)>=fTotalPwoer_Lmt) ||
    ((fabsf(meter_power)+fabsf(pPID_Ctx->fCompensation))>=fTotalPwoer_Lmt))
    // if (pEMS->nTargetStableCount < pPID_Ctx->nTargetStableCount_Thres)
    {
        error = 0.0f;
        pPID_Ctx->fPrevError = 0.0f;     
        pPID_Ctx->fPrevPrevError = 0.0f;
        pEMS->emHighSensitivityEnable = POWER_CTRL_DISABLE;
    }
    else
    {
        error = target_power - meter_power;

        if (pEMS->emHighSensitivityEnable == POWER_CTRL_DISABLE)
        {
            err = pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 301, &varV, 4);
            pEMS->emHighSensitivityEnable = (err==ERR_DPR_OK) ? varV.emValue : POWER_CTRL_DISABLE;
            pEMS->emHighSensitivityEnable = pEMS->emPIDEnableState | pEMS->emHighSensitivityEnable;
        }   
    }
    
    if (fabsf(error) < pPidParams->fDead_Band)
    {
        error = 0.0f;
    }

    float p_term_delta = 0.0f;
    if (fabsf(error) > pPidParams->fProportional_Term_Sepa_Thres)
    {
        p_term_delta = 0.0f; 
        error = 0.0f;
    }
    else
    {
        p_term_delta = pPidParams->Kp * (error - pPID_Ctx->fPrevError);
    }

    float i_term_delta = 0.0f;
    if (fabsf(error) > pPidParams->fIntegral_Term_Sepa_Thres)
    {
        i_term_delta = 0.0f;
    }
    else
    {
        i_term_delta = pPidParams->Ki * error;
    }

    float output_delta = p_term_delta + i_term_delta;

    pPID_Ctx->fCompensation += output_delta;

    if (pPID_Ctx->fCompensation > pPID_Ctx->fCompensation_Lmt) 
        pPID_Ctx->fCompensation = pPID_Ctx->fCompensation_Lmt;
    else if (pPID_Ctx->fCompensation < -pPID_Ctx->fCompensation_Lmt) 
        pPID_Ctx->fCompensation = -pPID_Ctx->fCompensation_Lmt;

    if (fabsf(target_power) < pPID_Ctx->fClearCompensationThres)
    {
        pPID_Ctx->fCompensation = 0.0f;
    }
    
#if 0
    // 恒定负载时 启用
    else if (fabsf(pPID_Ctx->fPrevTarget) <  pPID_Ctx->fClearCompensationThres && 
            fabsf(target_power) >=  pPID_Ctx->fClearCompensationThres && 
            pPID_Ctx->fInitCompCoef >= 0.0f)
    {
        if (pPID_Ctx->fInitCompCoef > 1.0f) pPID_Ctx->fInitCompCoef = 1.0f;
        pPID_Ctx->fCompensation = pPID_Ctx->fErrorAvgFilter * pPID_Ctx->fInitCompCoef;
    }
# endif

    float raw_output = target_power + pPID_Ctx->fCompensation;

    if (raw_output > pPID_Ctx->fMaxOutput_Lmt) raw_output = pPID_Ctx->fMaxOutput_Lmt;
    else if (raw_output < pPID_Ctx->fMinOutput_Lmt) raw_output = pPID_Ctx->fMinOutput_Lmt;

    pPID_Ctx->fPrevPrevError = pPID_Ctx->fPrevError;   // E(k-2)
    pPID_Ctx->fPrevError = error;                      // E(k-1)
    pPID_Ctx->fPrevTarget = target_power;              // P(k-1)
    
    if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_CHRGING)
    {
        pEMS->fTotal_PessChg_PidOut = raw_output; 
        pEMS->fTotal_PessDischg_PidOut = pEMS->fTotal_PessDischgLmt; 
    }
    else if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_DISCHRG)
    {
        pEMS->fTotal_PessDischg_PidOut = -raw_output; 
        pEMS->fTotal_PessChg_PidOut = pEMS->fTotal_PessChgAvaliable;
    }

    TRACED("Power_Control_Loop stPIDChgParams Kp:[%0.3f], Ki:[%0.3f], Dead_Band:[%0.3f], Proportional_Term_Sepa_Thres:[%0.3f], Integral_Term_Sepa_Thres:[%0.3f]\n"
        "stPIDDischgParams Kp:[%0.3f], Ki:[%0.3f], Dead_Band:[%0.3f], Proportional_Term_Sepa_Thres:[%0.3f], Integral_Term_Sepa_Thres:[%0.3f]\n"
        "fPrevError:[%0.3f], fPrevPrevError:[%0.3f], fCompensation:[%0.3f], fPrevTarget:[%0.3f]\n"
        "fMaxOutput_Lmt:[%0.3f], fMinOutput_Lmt:[%0.3f], fCompensation_Lmt:[%0.3f], fInitCompCoef:[%0.3f]\n"
        "fErrorAvgFilter:[%0.3f], emPIDEnableState:[%d], emHighSensitivityEnable:[%d]\n", 
        pPID_Ctx->stPIDChgParams.Kp, pPID_Ctx->stPIDChgParams.Ki, pPID_Ctx->stPIDChgParams.fDead_Band, pPID_Ctx->stPIDChgParams.fProportional_Term_Sepa_Thres, pPID_Ctx->stPIDChgParams.fIntegral_Term_Sepa_Thres,
        pPID_Ctx->stPIDDischgParams.Kp, pPID_Ctx->stPIDDischgParams.Ki, pPID_Ctx->stPIDDischgParams.fDead_Band, pPID_Ctx->stPIDDischgParams.fProportional_Term_Sepa_Thres, pPID_Ctx->stPIDDischgParams.fIntegral_Term_Sepa_Thres,
        pPID_Ctx->fPrevError, pPID_Ctx->fPrevPrevError, pPID_Ctx->fCompensation, pPID_Ctx->fPrevTarget,
        pPID_Ctx->fMaxOutput_Lmt, pPID_Ctx->fMinOutput_Lmt, pPID_Ctx->fCompensation_Lmt, pPID_Ctx->fInitCompCoef,
        pPID_Ctx->fErrorAvgFilter, pEMS->emPIDEnableState, pEMS->emHighSensitivityEnable
    );

    return;
}
