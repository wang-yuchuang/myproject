/********************************************************************************
* Copyright(c) Shenzhen Infypower Co., Ltd.
* @author LJH936
* @brief  储能EMS
* @history 2024-12-02 created
*******************************************************************************/
// #define _TRACE_PRINT_ // SEMSClient
#include "ess_sampler_main.h"

// calc_Node
static void Refresh_NodeParam_ESS(ROUGH_CCU_DATA* pCCU)
{
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
    DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
    VAR_VALUE varV;
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    int n = 0;
    // LJH250313 ClientSum
    BOOL bSystemOK = TRUE;
    BOOL hasPCSChrgNow = FALSE, hasPCSDischrgNow = FALSE, hasPCSStandby = FALSE;
    BOOL bStopChrgBySoc_Plan = FALSE, bStopDiscBySoc_Plan = FALSE; // 管理计划SOC
    BOOL bStopChrgBySoc_SetSEMS = FALSE, bStopDiscBySoc_SetSEMS = FALSE; // 设置的截止SOC
    UINT nSOC = 0, nClientNumCommOK = 0, nSOCMinSum = 100, nSOCMaxSum = 0, nSOCSumClients = 0;
    A_BLOB_SEMS_CLIENTINFO_SUM* pClientSum = &pEMS->clientInfoSum;
    EMSCtrlMode emEMSCtrlMode_REMS =       pClientSum->emEMSCtrlMode_REMS;
    ACDC_WORKSTATU emACDC_WorkStatu_REMS = pClientSum->emACDC_WorkStatu_REMS;
	float fActvPwrReq_REMS =               pClientSum->fActvPwrReq_REMS;
    ZERO_POBJS(pClientSum, 1);
    //0.ClientInfoSum RSEMS
    pClientSum->emEMSCtrlMode_REMS =    emEMSCtrlMode_REMS;
    pClientSum->emACDC_WorkStatu_REMS = emACDC_WorkStatu_REMS;
	pClientSum->fActvPwrReq_REMS =      fActvPwrReq_REMS;
    pClientSum->bStandby_BLL =     (BYTE)pEMS->bStandby_BLL;
    pClientSum->bStandby_AEMS =    (BYTE)pEMS->bStandby_AEMS;
    pClientSum->byWorkAsap_SEMS =  pEMS->byWorkAsap;

    if(pEMS->nNodeNum > MAX_SINGLE_STBATT_NUM)
    {
        TRACEX("Too Much Branch Node nNodeNum=[%d]\n", pEMS->nNodeNum);
        return ;
    }
    A_BLOB_BRANCH_GRP_RECTDATA* pBranchNodeData;
    A_BLOB_RECT_GRP_DATA* pGrpRectData;
    A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Node;
    // EMS_LIMIT_SECTOR* pPlanSectorOrg = &pEMS->planSectorOrg;
    EMS_LIMIT_SECTOR* pPlanSectorNow = &pEMS->planSectorNow;
    for(n = 0; n < pEMS->nNodeNum; n++)
    {
        pBranchNodeData = &pCCU->strLSYSInfo.branchNodeData[n];
        pGrpRectData =    &pCCU->strLSYSInfo.blobGrpRectData;
        pCalc_Node = &pEMS->calc_Node[n];
        // Ems_init中节点初始化移到这里
        Ems_Reset_Calc_Node(pCalc_Node);
        pCalc_Node->nNodeIdx = n;

        //0.1.fSOCSum
        if (pCCU->blobSingleBattData[n].emBattStatus_Run >= STBATT_RUNSTAT_SLEEP)
        {
            nClientNumCommOK++;
            nSOC = pBranchNodeData->nSOC;
            nSOCMinSum = MIN(nSOC, nSOCMinSum);
            nSOCMaxSum = MAX(nSOC, nSOCMaxSum);
            nSOCSumClients += nSOC;
        }

        //0.6. ACDC_WorkStatuSum // LJH250703 added:emCCUState && emPMCtrlMode
        if (pCCU->emCCUState == CCU_STATE_IDLE && pCCU->emPMCtrlMode == AUTO_CTRL_PM_MODE)
        {
            if (pBranchNodeData->emACDC_WorkStatu == STATU_ACDC_RECTIFY &&
                // pBranchNodeData->fActvPwr_PCSGrp > MIN_PWR_CHRG_ESS &&
                pBranchNodeData->fACDC_RealTime_Curr > MIN_CURR_WORKING_ESS)
            {
                hasPCSChrgNow = TRUE;
            }
            if (pBranchNodeData->emACDC_WorkStatu == STATU_ACDC_INVERTER &&
                // pBranchNodeData->fActvPwr_PCSGrp < MIN_PWR_DISCHRG_ESS &&
                pBranchNodeData->fACDC_RealTime_Curr > MIN_CURR_WORKING_ESS)
            {
                hasPCSDischrgNow = TRUE;
            }
            if (pBranchNodeData->emACDC_WorkStatu == STATU_ACDC_STANDBY)
            {
                if (!hasPCSStandby)
                {
                    hasPCSStandby = TRUE;
                }
            }
        }
        //0.9.nTMSector
        if (pEMS->nTMSector == -1)
        {
            TRACEW("Refresh_NodeParam_ESS continue; pEMS->nTMSector == -1 n:%d", n);
            continue;
        }

        // // unit test ess
        // {
        //     pBranchNodeData->nSOC = 55;
            
        //     nClientNumCommOK++;
        //     nSOC = pBranchNodeData->nSOC;
        //     nSOCMinSum = MIN(nSOC, nSOCMinSum);
        //     nSOCMaxSum = MAX(nSOC, nSOCMaxSum);
        //     nSOCSumClients += nSOC;

        //     pBranchNodeData->fACDC_Max_Output_Pwr_Ability = 125.0f;
        //     pBranchNodeData->fBatt_Max_Chrg_Pwr_Req = 130.0f;
        //     pBranchNodeData->fBatt_Max_DisChrg_Pwr = 140.0f;
        //     pGrpRectData->fRatedPower[n] = 125.0f;
        //     pCalc_Node->fChrgedEnergy = 500.0f;
        //     pCCU->emCCUState = CCU_STATE_IDLE;
        //     pCCU->emPMCtrlMode = AUTO_CTRL_PM_MODE;

        //     pGrpRectData->fRealTime_MaxCurr[n] = 160.0f;
        //     pBranchNodeData->fACDC_RealTime_Volt = 850.0f;
        //     pBranchNodeData->fACDC_RealTime_Curr = 150.0f;
        //     // hasPCSChrgNow = TRUE;
        //     // hasPCSDischrgNow = TRUE;
        // }

        //1. 储能电池最大放电功率&充电功率
        pCalc_Node->fBatt_Max_DisChrg_Pwr = pBranchNodeData->fBatt_Max_DisChrg_Pwr;
        pCalc_Node->fBatt_Max_Chrg_Pwr_Req = pBranchNodeData->fBatt_Max_Chrg_Pwr_Req;
        //1.1. 模块最大输出能力 LJH250610 取LSYS的原始值
        pCalc_Node->fACDC_Max_Ability_Pwr = pBranchNodeData->fACDC_Max_Output_Pwr_Ability;

        //5. M储能电池剩余充电/放电功率
        pCalc_Node->fRemain_Batt_Chrg_Pwr_Req = pCalc_Node->fBatt_Max_Chrg_Pwr_Req;
        pCalc_Node->fRemain_Batt_DisChrg_Pwr = pCalc_Node->fBatt_Max_DisChrg_Pwr;

        //6.Calc Battery Group Charged Energy [$,2,504]
 	    pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 501+n, 554, &varV, 4);  
        pCalc_Node->fChrgedEnergy = varV.fValue;

        //7.fMax0 fMaxA充放电能力 // LJH250930 加快自动充放响应,StopSOC_SetSEMS
        pCalc_Node->nSOC = pBranchNodeData->nSOC;
        bSystemOK = (pCCU->emCCUState == CCU_STATE_IDLE && pCCU->emPMCtrlMode == AUTO_CTRL_PM_MODE);
        bStopChrgBySoc_SetSEMS = (pCalc_Node->nSOC >= pEMS->nStopSOC_ChrgSet_SEMS);
        bStopDiscBySoc_SetSEMS = (pCalc_Node->nSOC <= pEMS->nStopSOC_DiscSet_SEMS);

        pCalc_Node->fMax_Chrg_PwrAbility0 =    MIN(pCalc_Node->fACDC_Max_Ability_Pwr, pCalc_Node->fBatt_Max_Chrg_Pwr_Req);
        pCalc_Node->fMax_DisChrg_PwrAbility0 = MIN(pCalc_Node->fACDC_Max_Ability_Pwr, pCalc_Node->fBatt_Max_DisChrg_Pwr);
        pCalc_Node->fMax_Chrg_PwrAbilityA =    (!bSystemOK || bStopChrgBySoc_SetSEMS) ? 0 : pCalc_Node->fMax_Chrg_PwrAbility0;
        pCalc_Node->fMax_DisChrg_PwrAbilityA = (!bSystemOK || bStopDiscBySoc_SetSEMS) ? 0 : pCalc_Node->fMax_DisChrg_PwrAbility0;
        
        //8.fMax充放电能力和emBatt_WorkMode // LJH250606 fix:从控故障主控还继续下发充放电-故障或手动状态上报Max功率能力为0
        pCalc_Node->emBatt_WorkMode = pEMS->emBatt_WorkModeReq;
        pCalc_Node->nStopSOCReq = pPlanSectorNow->nStopSOC;
        pCalc_Node->nSOC = pBranchNodeData->nSOC;
        if ( !bSystemOK )
        {
            pCalc_Node->fMax_Chrg_PwrAbility =    0;
            pCalc_Node->fMax_DisChrg_PwrAbility = 0;
            TRACEW("Refresh_NodeParam_ESS n:%d set fMax_Chrg_PwrAbility=0 emCCUState:%d emPMCtrlMode:%d",
                n, pCCU->emCCUState, pCCU->emPMCtrlMode);
        }
        else
        {
            if (!pEMS->bChrgAsap && !pEMS->bDiscAsap)
            {
                //8.1 work by plan
                // LJH250222 Added 出现-D上报fACDC_Max_Ability_Pwr错误问题
                pCalc_Node->fMax_Chrg_PwrAbility =    MAX(pBranchNodeData->fActvPwr_PCSGrp,      pCalc_Node->fMax_Chrg_PwrAbility0);
                pCalc_Node->fMax_DisChrg_PwrAbility = MAX(ABS(pBranchNodeData->fActvPwr_PCSGrp), pCalc_Node->fMax_DisChrg_PwrAbility0);
                // LJH250606 fix:现场出现pcs输出的功率会大于额定功率，用额定功率限制 LJH250917 不限制，可以1.1倍运行 LJH250930改回用额定功率限制
                pCalc_Node->fMax_Chrg_PwrAbility =    MIN(pGrpRectData->fRatedPower[n], pCalc_Node->fMax_Chrg_PwrAbility);
                pCalc_Node->fMax_DisChrg_PwrAbility = MIN(pGrpRectData->fRatedPower[n], pCalc_Node->fMax_DisChrg_PwrAbility);
                // LJH250612 planSector
                pCalc_Node->fMax_Chrg_PwrAbility =    MIN((float)pPlanSectorNow->nBattGrpPwrLmt, pCalc_Node->fMax_Chrg_PwrAbility);
                pCalc_Node->fMax_DisChrg_PwrAbility = MIN((float)pPlanSectorNow->nBattGrpPwrLmt, pCalc_Node->fMax_DisChrg_PwrAbility);
                
                //8.2 fMax_Chrg_PwrAbility and emBatt_WorkMode by SOC LJH250620 added
                bStopChrgBySoc_Plan = (pCalc_Node->nSOC >= pCalc_Node->nStopSOCReq);
                bStopDiscBySoc_Plan = (pCalc_Node->nSOC <= pCalc_Node->nStopSOCReq);
                if (bStopChrgBySoc_Plan || bStopChrgBySoc_SetSEMS)
                {
                    pCalc_Node->fMax_Chrg_PwrAbility = 0;
                    if (pCalc_Node->emBatt_WorkMode == BATT_WORKMODE_FIRST_CHRGING)
                    {
                        pCalc_Node->emBatt_WorkMode = BATT_WORKMODE_IDLE;
                    }
                    TRACEW("Refresh_NodeParam_ESS bStopChrgBySoc:%d %d set fMax_Chrg_PwrAbility = 0 BATT_WORKMODE_IDLE n:%d nSOC:%d >= nStopSOCReq:%d ",
                        bStopChrgBySoc_Plan, bStopChrgBySoc_SetSEMS, n, pCalc_Node->nSOC, pCalc_Node->nStopSOCReq);
                }
                if (bStopDiscBySoc_Plan || bStopDiscBySoc_SetSEMS)
                {
                    pCalc_Node->fMax_DisChrg_PwrAbility = 0;
                    if (pCalc_Node->emBatt_WorkMode == BATT_WORKMODE_FIRST_DISCHRG)
                    {
                        pCalc_Node->emBatt_WorkMode = BATT_WORKMODE_IDLE;
                    }
                    TRACEW("Refresh_NodeParam_ESS bStopDiscBySoc:%d %d set fMax_DisChrg_PwrAbility = 0 n:%d nSOC:%d <= nStopSOCReq:%d",
                        bStopDiscBySoc_Plan, bStopDiscBySoc_SetSEMS, n, pCalc_Node->nSOC, pCalc_Node->nStopSOCReq);
                }
            }
            else
            {
                //8.6 // LJH250930 加快自动充放响应,WorkASAP
                if (pEMS->bChrgAsap)
                {
                    pCalc_Node->fMax_Chrg_PwrAbility = pCalc_Node->fMax_Chrg_PwrAbilityA;
                }
                if (pEMS->bDiscAsap)
                {
                    pCalc_Node->fMax_DisChrg_PwrAbility = pCalc_Node->fMax_DisChrg_PwrAbilityA;
                }
            }
        }

        //20.所有客户端累加数据
        pClientSum->fActvPwr_PCSGrpSum +=      pBranchNodeData->fActvPwr_PCSGrp;
        pClientSum->fQ_PCSGrpSum +=            pBranchNodeData->fQ_PCSGrp;

        pClientSum->fMax_Chrg_PwrAbilitySum +=     pCalc_Node->fMax_Chrg_PwrAbility;
        pClientSum->fMax_DisChrg_PwrAbilitySum +=  pCalc_Node->fMax_DisChrg_PwrAbility;
        pClientSum->fMax_Chrg_PwrAbilitySumA +=    pCalc_Node->fMax_Chrg_PwrAbilityA; // LJH250930 加快自动充放响应,WorkASAP
        pClientSum->fMax_DisChrg_PwrAbilitySumA += pCalc_Node->fMax_DisChrg_PwrAbilityA;
        pClientSum->fMax_Chrg_PwrAbilitySum0 +=    pCalc_Node->fMax_Chrg_PwrAbility0; // LJH250610 fPwrAbility0
        pClientSum->fMax_DisChrg_PwrAbilitySum0 += pCalc_Node->fMax_DisChrg_PwrAbility0;

        pClientSum->fChrgedEnergySum +=           pCalc_Node->fChrgedEnergy;
        pClientSum->fRealTime_MaxCurrSum +=       pGrpRectData->fRealTime_MaxCurr[n];
        pClientSum->fACDC_RealTime_VoltSum =      MAX(pClientSum->fACDC_RealTime_VoltSum, pBranchNodeData->fACDC_RealTime_Volt);
        pClientSum->fACDC_RealTime_CurrSum +=     pBranchNodeData->fACDC_RealTime_Curr;
    } // end for
    // sum1:状态
    pEMS->hasPCSChrgLast_SEMS =      hasPCSChrgNow;
    pEMS->hasPCSDischrgLast_SEMS =   hasPCSDischrgNow;
    pClientSum->emCCUStateSum = pCCU->emCCUState;
    // LJH250625 SEMS使用设置的参数上报 -D上报的手动、自动模式不对
    pClientSum->emPMCtrlModeSum = pCCU->emPMCtrlMode;
    //old pClientSum->emPMCtrlModeSum = pCCU->strLSYSInfo.emRunningMode;
    if (hasPCSChrgNow)
    {
        pClientSum->emACDC_WorkStatuSum = STATU_ACDC_RECTIFY;
    }
    else if (hasPCSDischrgNow)
    {
        pClientSum->emACDC_WorkStatuSum = STATU_ACDC_INVERTER;
    }
    else if (hasPCSStandby)
    {
        pClientSum->emACDC_WorkStatuSum = STATU_ACDC_STANDBY;
    }
    pClientSum->bStandby_SetSum = (BYTE)pEMS->bStandby_Set;
    pClientSum->bStandby_BLLSum = (BYTE)pEMS->bStandby_BLL;
    pClientSum->byWorkAsapSum =   (BYTE)pEMS->byWorkAsap;

    // sum2:SOC
    if (nClientNumCommOK > 0)
    {
        pClientSum->nSOCMinSum = (BYTE)nSOCMinSum;
        pClientSum->nSOCMaxSum = (BYTE)nSOCMaxSum;
        pClientSum->fSOCSum = (float)nSOCSumClients / (float)nClientNumCommOK;
    }
    else
    {
        pClientSum->nSOCMinSum = 0;
        pClientSum->nSOCMaxSum = 0;
        pClientSum->fSOCSum = 0;
    }
    // LJH260131 增加top#6/top#7的ESC防逆流/防过载
    if (pEMS->enableAntiOvld_REMS)
    {
        pClientSum->fMax_Chrg_PwrAbilitySum =  MIN(pEMS->fTotal_PessChgAvaliable_Org, pClientSum->fMax_Chrg_PwrAbilitySum);
        pClientSum->fMax_Chrg_PwrAbilitySumA = MIN(pEMS->fTotal_PessChgAvaliable_Org, pClientSum->fMax_Chrg_PwrAbilitySumA);
        TRACEW("Refresh_NodeParam_ESS enableAntiOvld_REMS ClientSum:fMax_Chrg_PwrAbilitySum:%.1f fTotal_PessChgAvaliable_Org:%.1f fTotal_PessChgAvaliable:%.1f",
            pClientSum->fMax_Chrg_PwrAbilitySum, pEMS->fTotal_PessChgAvaliable_Org, pEMS->fTotal_PessChgAvaliable);
    }
    if (pEMS->enableAntiRflx_REMS)
    {
        pClientSum->fMax_DisChrg_PwrAbilitySum =  MIN(pEMS->fTotal_PessDischgLmt_Org, pClientSum->fMax_DisChrg_PwrAbilitySum);
        pClientSum->fMax_DisChrg_PwrAbilitySumA = MIN(pEMS->fTotal_PessDischgLmt_Org, pClientSum->fMax_DisChrg_PwrAbilitySumA);
        TRACEI("Refresh_NodeParam_ESS enableAntiRflx_REMS fMax_DisChrg_PwrAbilitySum:%.1f fTotal_PessDischgLmt_Org:%.1f fTotal_PessDischgLmt:%.1f",
            pClientSum->fMax_DisChrg_PwrAbilitySum, pEMS->fTotal_PessDischgLmt_Org, pEMS->fTotal_PessDischgLmt);
    }
    // sum3:meter
    if (pCCU->lcCfg.bGridMeter)
    {
        pClientSum->f_PgridMeterSum = pEMS->f_PgridMeter;
    }
    pClientSum->f_PcabMeterSum =     pEMS->f_PcabMeter;
    // LJH251111 AEMS
    pClientSum->fPacload4ChgSum =    pEMS->fPacload4Chg;
	pClientSum->fPacload4DischgSum = pEMS->fPacload4Dischg;
    // sum4:maxAbility LJH250703 ESC最大功率限制
    pClientSum->fMax_Chrg_PwrAbilitySum =    MIN(pCCU->fTotalPowerLimit, pClientSum->fMax_Chrg_PwrAbilitySum);
    pClientSum->fMax_DisChrg_PwrAbilitySum = MIN(pCCU->fTotalPowerLimit, pClientSum->fMax_DisChrg_PwrAbilitySum);
    pClientSum->fMax_Chrg_PwrAbilitySumA =    MIN(pCCU->fTotalPowerLimit, pClientSum->fMax_Chrg_PwrAbilitySumA);
    pClientSum->fMax_DisChrg_PwrAbilitySumA = MIN(pCCU->fTotalPowerLimit, pClientSum->fMax_DisChrg_PwrAbilitySumA);
#ifdef _TRACE_PRINT_
    pCalc_Node = &pEMS->calc_Node[0];
    TRACEI("Refresh_NodeParam_ESS emBatt_WorkMode:%d nSOC:%d nStopSOCReq:%d "
        "fMax_Chrg_PwrAbility:%.1f %.1f fMax_Chrg_PwrAbility0:%.1f %.1f fACDC_Max_Ability_Pwr=[%.1f] "
        "fBatt_Max_Chrg_Pwr_Req=[%.1f] fBatt_Max_DisChrg_Pwr=[%.1f] fChrgedEnergy=[%.1f] \n"
        "ClientSum emEMSCtrlMode_REMS:%d fActvPwrReq_REMS:%.1f emCCUStateSum:%d emPMCtrlModeSum:%d emACDC_WorkStatuSum:%d "
        "nSOCMinSum:%d nSOCMaxSum:%d fSOCSum:%.1f fActvPwr_PCSGrpSum:%.1f fQ_PCSGrpSum:%.1f "
        "\nfMax_Chrg_PwrAbilitySum:%.1f fMax_DisChrg_PwrAbilitySum:%.1f AbilitySum0:%.1f %.1f "
		"fChrgedEnergySum:%.1f fRealTime_MaxCurrSum:%.1f fACDC_RealTime_VoltSum:%.1f fACDC_RealTime_CurrSum:%.1f "
		"f_PgridMeterSum:%.1f f_PcabMeterSum:%.1f",
        pCalc_Node->emBatt_WorkMode, pCalc_Node->nSOC, pCalc_Node->nStopSOCReq, 
        pCalc_Node->fMax_Chrg_PwrAbility, pCalc_Node->fMax_DisChrg_PwrAbility, 
        pCalc_Node->fMax_Chrg_PwrAbility0, pCalc_Node->fMax_DisChrg_PwrAbility0, pCalc_Node->fACDC_Max_Ability_Pwr,
        pCalc_Node->fBatt_Max_Chrg_Pwr_Req, pCalc_Node->fBatt_Max_DisChrg_Pwr, pCalc_Node->fChrgedEnergy,
        pClientSum->emEMSCtrlMode_REMS, pClientSum->fActvPwrReq_REMS, pClientSum->emCCUStateSum, pClientSum->emPMCtrlModeSum, pClientSum->emACDC_WorkStatuSum, 
        pClientSum->nSOCMinSum, pClientSum->nSOCMaxSum, pClientSum->fSOCSum, pClientSum->fActvPwr_PCSGrpSum, pClientSum->fQ_PCSGrpSum,
        pClientSum->fMax_Chrg_PwrAbilitySum, pClientSum->fMax_DisChrg_PwrAbilitySum, pClientSum->fMax_Chrg_PwrAbilitySum0, pClientSum->fMax_DisChrg_PwrAbilitySum0,
		pClientSum->fChrgedEnergySum, pClientSum->fRealTime_MaxCurrSum, pClientSum->fACDC_RealTime_VoltSum, pClientSum->fACDC_RealTime_CurrSum,
		pClientSum->f_PgridMeterSum, pClientSum->f_PcabMeterSum);
#endif
    return ;
}

static void Filter_Disttribute_Power_QAndClientSum_ESS(ROUGH_CCU_DATA *pCCU, int n, BOOL bCalcQ)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    if (bCalcQ)
    {
        pEMS->calc_Node[n].fAvg_QoutputLmt = 
                Samp_Avg_Filter(n + FILTERTYPE_CALC_QLMT_GRP0, pEMS->calc_Node[n].fQoutput_Distri4ACDC, MIN_FILTER_POINT);
        pEMS->calc_Node[n].fAvg_QoutputLmt = pEMS->fDirect*pEMS->calc_Node[n].fAvg_QoutputLmt;
    }

    // if (pCCU->lcCfg.lcType == LC_TYPE_1st)
    {
        // to Upper
        Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3900,
            (void*)&pEMS->clientInfoSum, sizeof(A_BLOB_SEMS_CLIENTINFO_SUM), BLOB_TYPE_SEMS_CLIENTNFO_SUM);
    }
#ifdef _TRACE_PRINT_
    if (n == 0)
    {
        TRACE("Filter_Disttribute_Power_QAndClientSum_ESS n=[%d] nNodeIdx=[%d] emBatt_WorkMode:%d 5:idle fAvg_ChrgLmtPower=[%.f]fACDC_Distri_4_BattChrg_Pwr=[%.1f] "
            "fAvg_DisChrgingLmtPower=[%.f]fBatt_Distri_4_ACDC_Pwr=[%.1f] "
            "fAvg_QoutputLmt=[%.f] fQoutput_Distri4ACDC=[%.f] "
            "fSOCSum:%.1f ",
            n, pEMS->calc_Node[n].nNodeIdx, pEMS->calc_Node[n].emBatt_WorkMode,
            pEMS->calc_Node[n].fAvg_ChrgLmtPower,
            pEMS->calc_Node[n].fACDC_Distri_4_BattChrg_Pwr,
            pEMS->calc_Node[n].fAvg_DisChrgingLmtPower,
            pEMS->calc_Node[n].fBatt_Distri_4_ACDC_Pwr,
            pEMS->calc_Node[n].fAvg_QoutputLmt,
            pEMS->calc_Node[n].fQoutput_Distri4ACDC,
            pEMS->clientInfoSum.fSOCSum);
    }
#endif
}

static void Filter_Disttribute_Power_ESS(ROUGH_CCU_DATA *pCCU)
{
    EMS_MGMT_STU *pEMS	= 	&pCCU->emsMgmtData;
    int n = 0;
    if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_CHRGING)
    {
        for(n = 0; n < pEMS->nNodeNum; n++)
        {
            if (pCCU->bQuickChannle || pEMS->emHighSensitivityEnable)
            {
                // printf("Filter_Disttribute_Power_ESS Charge: %d\n", pEMS->emHighSensitivityEnable);
                pEMS->calc_Node[n].fAvg_ChrgLmtPower = pEMS->calc_Node[n].fACDC_Distri_4_BattChrg_Pwr;
            }
            else
            {
                pEMS->calc_Node[n].fAvg_ChrgLmtPower = 
                    Samp_Avg_Filter(n + FILTERTYPE_CALC_CHRGPOWER_GRP0, pEMS->calc_Node[n].fACDC_Distri_4_BattChrg_Pwr, MIN_FILTER_POINT);
            }
            
            if (pEMS->calc_Node[n].fAvg_ChrgLmtPower < MIN_PWR_PCS_OUT_REQ)
            {// LJH241202 SEMS
                TRACEW("Filter_Disttribute_Power_ESS set BATT_WORKMODE_IDLE fAvg_ChrgLmtPower:%.1f", pEMS->calc_Node[n].fAvg_ChrgLmtPower);
                pEMS->calc_Node[n].emBatt_WorkMode = BATT_WORKMODE_IDLE;
            }
            Filter_Disttribute_Power_QAndClientSum_ESS(pCCU, n, TRUE);
        }
    }
    else if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_DISCHRG)
    {
        for(n = 0; n < pEMS->nNodeNum; n++)
        {
            if (pCCU->bQuickChannle || pEMS->emHighSensitivityEnable)
            {
                // printf("Filter_Disttribute_Power_ESS Discharge: %d\n", pEMS->emHighSensitivityEnable);
                pEMS->calc_Node[n].fAvg_DisChrgingLmtPower = pEMS->calc_Node[n].fBatt_Distri_4_ACDC_Pwr;
            }
            else
            {
                pEMS->calc_Node[n].fAvg_DisChrgingLmtPower = 
                    Samp_Avg_Filter(n + FILTERTYPE_CALC_DISCHRGPOWER_GRP0, pEMS->calc_Node[n].fBatt_Distri_4_ACDC_Pwr, (int)pCCU->nDisChrgFilterPoint);//MIN_FILTER_POINT
            }
            
            if (pEMS->calc_Node[n].fAvg_DisChrgingLmtPower < MIN_PWR_PCS_OUT_REQ)
            {// LJH241202 SEMS
                TRACEW("Filter_Disttribute_Power_ESS set BATT_WORKMODE_IDLE fAvg_DisChrgingLmtPower:%.1f < %.1f", pEMS->calc_Node[n].fAvg_DisChrgingLmtPower, MIN_PWR_PCS_OUT_REQ);
                pEMS->calc_Node[n].emBatt_WorkMode = BATT_WORKMODE_IDLE;
            }
            Filter_Disttribute_Power_QAndClientSum_ESS(pCCU, n, TRUE);
        }
    }
    else
    {
        for(n = 0; n < pEMS->nNodeNum; n++)
        {
            Filter_Disttribute_Power_QAndClientSum_ESS(pCCU, n, FALSE);
        }
    }

#ifdef _TRACE_PRINT_
    A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Nodes = pEMS->calc_Node;
    TRACED("Filter_Disttribute_Power_ESS nNodeIdx:%d %d %d %d "
        "emBatt_WorkModeReq:%d emBatt_WorkMode:%d %d %d %d 5:idle fAvg_QoutputLmt=[%.f] fQoutput_Distri4ACDC=[%.f]"
        "\n>>> Charge fAvg_ChrgLmtPower:%.1f %.1f %.1f %.1f fACDC_Distri_4_BattChrg_Pwr:%.1f %.1f %.1f %.1f"
        "\n<<< Discharge fAvg_DisChrgingLmtPower:%.1f %.1f %.1f %.1f fBatt_Distri_4_ACDC_Pwr:%.1f %.1f %.1f %.1f",
        pCalc_Nodes[0].nNodeIdx, pCalc_Nodes[1].nNodeIdx, pCalc_Nodes[2].nNodeIdx, pCalc_Nodes[3].nNodeIdx,
        pEMS->emBatt_WorkModeReq,
        pCalc_Nodes[0].emBatt_WorkMode, pCalc_Nodes[1].emBatt_WorkMode, pCalc_Nodes[2].emBatt_WorkMode, pCalc_Nodes[3].emBatt_WorkMode,
        pEMS->calc_Node[0].fAvg_QoutputLmt, pEMS->calc_Node[0].fQoutput_Distri4ACDC,

        pCalc_Nodes[0].fAvg_ChrgLmtPower, pCalc_Nodes[1].fAvg_ChrgLmtPower, pCalc_Nodes[2].fAvg_ChrgLmtPower, pCalc_Nodes[3].fAvg_ChrgLmtPower,
        pCalc_Nodes[0].fACDC_Distri_4_BattChrg_Pwr, pCalc_Nodes[1].fACDC_Distri_4_BattChrg_Pwr, pCalc_Nodes[2].fACDC_Distri_4_BattChrg_Pwr, pCalc_Nodes[3].fACDC_Distri_4_BattChrg_Pwr,
        pCalc_Nodes[0].fAvg_DisChrgingLmtPower, pCalc_Nodes[1].fAvg_DisChrgingLmtPower, pCalc_Nodes[2].fAvg_DisChrgingLmtPower, pCalc_Nodes[3].fAvg_DisChrgingLmtPower,
        pCalc_Nodes[0].fBatt_Distri_4_ACDC_Pwr, pCalc_Nodes[1].fBatt_Distri_4_ACDC_Pwr, pCalc_Nodes[2].fBatt_Distri_4_ACDC_Pwr, pCalc_Nodes[3].fBatt_Distri_4_ACDC_Pwr
        );
#endif
}

int Ems_Mgmt_Main_ESS(ROUGH_CCU_DATA* pCCU)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    Ems_Mgmt_Total_Pess_All(pCCU);

    // // old
    // if (pCCU->lcCfg.lcType == LC_TYPE_1st)
    // {
    //     // LJH250408 CtrlByREMS 单ESC-主控 // LJH250902 top#6+REMS modified
    //     if (pEMS->enableREMSCtrl)
    //     {
    //         GetCtrlParam_REMS(pCCU);
    //         if (pEMS->clientInfoSum.emEMSCtrlMode_REMS == EMSCtrlMode_Remote)
    //         {
    //             // Send_A6USYS: (pEMS->enableREMSCtrl && pEMS->clientInfoSum.emEMSCtrlMode_REMS == EMSCtrlMode_Remote)
    //             pEMS->bCtrlByREMS = TRUE;
    //             Ems_Mgmt_Total_Pess_REMS(pCCU);
    //         }
    //         else
    //         {
    //             pEMS->bCtrlByREMS = FALSE;
    //         }
    //     }
    //     else
    //     {
    //         pEMS->bCtrlByREMS = FALSE;
    //     }
    //     if (!pEMS->bCtrlByREMS)
    //     {
    //         TRACED("Ems_Mgmt_Main_ESS bCtrlByREMS:FALSE changedBlobEmsCfgPlan:%d bAutoDischrgEnable:%d bAutoChrgEnable:%d bEnter_AutoDisc_SEMS:%d bEnter_AutoChrg_SEMS:%d",
    //             pEMS->changedBlobEmsCfgPlan, pEMS->bAutoDischrgEnable, pEMS->bAutoChrgEnable, pEMS->bEnter_AutoDisc_SEMS, pEMS->bEnter_AutoChrg_SEMS);
    //         // LJH250408 CtrlByREMS 单ESC-主控
    //         if (pEMS->changedBlobEmsCfgPlan)
    //         {
    //             ModifyBlobEmsCfgPlan(pCCU, TRUE, NULL, NULL); // restored
    //         }
    //         GetCurrentTimeSector(pCCU, TRUE);
    //         Ems_Mgmt_Total_Pess(pCCU);
    //         // new LJH250930 加快自动充放响应,ChgDisOnly
    //         if (!NEED_AUTODISC() && !NEED_AUTOCHRG())
    //         {
    //             pEMS->bStandby_Auto = FALSE;
    //             pEMS->bEnter_AutoChrg_SEMS = FALSE;
    //             pEMS->bEnter_AutoDisc_SEMS = FALSE;
    //         }
    //         else
    //         {
    //             if (NEED_AUTODISC())
    //             {
    //                 AutoDischarging_IfOverload(pCCU);
    //                 // standby
    //                 if (ElapseS(pEMS->tmStandby_4Auto) < pEMS->uDelayStandby_4Disc)
    //                 {
    //                     pEMS->bStandby_Auto = TRUE;
    //                 }
    //                 else
    //                 {
    //                     pEMS->bStandby_Auto = FALSE;
    //                 }
    //             }
    //             else
    //             {
    //                 pEMS->bEnter_AutoDisc_SEMS = FALSE;
    //             }
    //             if (NEED_AUTOCHRG() && !pEMS->bEnter_AutoDisc_SEMS)
    //             {
    //                 AutoCharging_IfReflux(pCCU);
    //                 // standby
    //                 if (ElapseS(pEMS->tmStandby_4Auto) < pEMS->uDelayStandby_4Chrg)
    //                 {
    //                     pEMS->bStandby_Auto = TRUE;
    //                 }
    //                 else
    //                 {
    //                     pEMS->bStandby_Auto = FALSE;
    //                 }
    //             }
    //             else
    //             {
    //                 pEMS->bEnter_AutoChrg_SEMS = FALSE;
    //             }
    //         }
    //         // LJH251127 单柜自动充放电
    //         VAR_VALUE varV;
    //         varV.emValue = (!pEMS->bEnter_AutoChrg_SEMS && !pEMS->bEnter_AutoDisc_SEMS ? 0:1);
    //         Push_VarValue(SETTYPE_INTERN_CTRL,  pCCU->hSampler, 501, 527, varV);

    //         // pEMS->emBatt_WorkMode_Standby = (!pEMS->bEnter_AutoChrg_SEMS && !pEMS->bEnter_AutoDisc_SEMS ? BATT_WORKMODE_IDLE:
    //         //     (pEMS->bEnter_AutoChrg_SEMS ? BATT_WORKMODE_FIRST_CHRGING:BATT_WORKMODE_FIRST_DISCHRG));
    //         BOOL bStandby = (pEMS->planSectorOrg.emBatt_WorkMode==BATT_WORKMODE_STANDBY || pEMS->bStandby_Auto || pEMS->bStandby_Set || pEMS->bStandby_AEMS);
    //         if (pEMS->bStandby_BLL != bStandby)
    //         {
    //             pEMS->bStandby_BLL = bStandby;
    //             varV.emValue = (_ENUM)pEMS->bStandby_BLL;
    //             Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1001, 3800, varV);
    //         }

    //         // // old
    //         // if (pEMS->bAutoChrgEnable && !pEMS->bEnter_AutoDisc_SEMS)
    //         // {
    //         //     AutoCharging_IfRefluxEss(pCCU);
    //         //     GetCurrentTimeSector(pCCU, FALSE);
    //         // }
    //         // BOOL bStandby_Plan = pEMS->planSectorOrg.emBatt_WorkMode==BATT_WORKMODE_STANDBY;
    //         // if (pEMS->bStandby_BLL != bStandby_Plan)
    //         // {
    //         //     pEMS->bStandby_BLL = bStandby_Plan;
    //         //     VAR_VALUE varV;
    //         //     varV.emValue = (_ENUM)pEMS->bStandby_BLL;
    //         //     Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1001, 3800, varV);
    //         // }
    //     }
    //     // LJH250930 加快自动充放响应
    //     pEMS->bChrgAsap = (pEMS->bCtrlByREMS);
    //     pEMS->bDiscAsap = (pEMS->bCtrlByREMS);
    //     pEMS->byWorkAsap = Make_WorkAsap(pEMS->bChrgAsap, pEMS->bDiscAsap).byWorkAsapData;
    //     TRACED("Ems_Mgmt_Main_ESS bCtrlByREMS:%d changedBlobEmsCfgPlan:%d bChrgAsap:%d bDiscAsap:%d \n"
    //         "bStandby_BLL:%d bStandby_Set:%d bEnter_AutoChrg_SEMS:%d bEnter_AutoDisc_SEMS:%d",
    //         pEMS->bCtrlByREMS, pEMS->changedBlobEmsCfgPlan, pEMS->bChrgAsap, pEMS->bDiscAsap,
    //         pEMS->bStandby_BLL, pEMS->bStandby_Set, pEMS->bEnter_AutoChrg_SEMS, pEMS->bEnter_AutoDisc_SEMS);
    // }
    // else
    // {
    //     // 从机
    //     TRACEI("Ems_Mgmt_Main_ESS Ems_Mgmt_Total_Pess_EMSClient");
    //     Ems_init(pCCU);
    //     Ems_Mgmt_Total_Pess_EMSClient(pCCU);
    // }
    Power_Control_Loop(pCCU);
    
    Refresh_ESSRemainPower(pCCU);
    // calced pEMS->emBatt_WorkModeReq
    
    // 电池、模块最大充、放电功率
    Refresh_NodeParam_ESS(pCCU);
    // calced pEMS->calc_Node[n].emBatt_WorkMode

    //!!! After Refresh ALL Node Param,Then Sort Node Order
    Sort_NodeOrderByChrgedEnergy(pCCU);
    
    if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_CHRGING)
    {
        MPET_Calc4DispatchChrgMode(pCCU);
        //分配给电池组的充电功率
        Dispatch_ESSLmtBattChrg_Power(pCCU);
    }
    else if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_DISCHRG)
    {
    	MPET_Calc4DispatchDisChrgMode(pCCU);
        //分配给AC侧负载的放电功率
        DisPatch_ACLoadReq_DisChrg_Power(pCCU);
    }

    // 分配给DCDC直流母线侧负载的功率 John 20220413 .Removed
    //Dispatch_Pwr4DCBusLoad(pCCU);
    
    //Allocate Reactive Power When ESS is Chging/DisChging.
    DisPatch_ReactivePower_Output(pCCU);

    Sort_NodeOrderByNodeIdx(pCCU); // Resort The Position To Before Accoding To Chrged Energy.

    Filter_Disttribute_Power_ESS(pCCU);

    TRACE("--------------------End- Solar Backup EMS Calculate Node------------------\n");
#ifdef LOG_FILEDEBUG
    static STBATT_PRI_WORKMODE emBatt_WorkModeReq = BATT_WORKMODE_IDLE;
    static time_t stmLogFile = 0;
    if (ElapseS(stmLogFile) > INTV_S_LOG_DEBUGFILE || emBatt_WorkModeReq != pEMS->emBatt_WorkModeReq)
    {
        stmLogFile = _NOW_;
        emBatt_WorkModeReq = pEMS->emBatt_WorkModeReq;
        A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Nodes = pEMS->calc_Node;
        A_BLOB_BRANCH_GRP_RECTDATA* pBranchNodeData = pCCU->strLSYSInfo.branchNodeData;
        A_BLOB_SEMS_CLIENTINFO_SUM* pClientSum = &pEMS->clientInfoSum;
        EMS_LIMIT_SECTOR* pPlanSectorOrg = &pEMS->planSectorOrg;
        EMS_LIMIT_SECTOR* pPlanSectorNow = &pEMS->planSectorNow;
        LOGFD("Ems_Mgmt_Main_ESS WorkModeReq:%s nNodeNum:%d emBatt_WorkModeReq:%d 5:idle "
            "bEnter_AutoChrg_SEMS:%d bEnter_AutoDisc_SEMS:%d "
            "emCCUState:%d emPMCtrlMode:%d 0:AUTO_CTRL nSOC:%d nStopSOCReq:%d \n"
            
            "f_PgridMeter:%.1f f_PcabMeter:%.1f f_PmaxGridDemand:%.1f fPacload4Chg:%.1f fPacload4Dischg:%.1f "
            "fGridACInputPwr_Lmt:%.1f fGridRemainACPwr2Chrg:%.1f fRemainPwr4Chrg:%.1f fRemainDemand4Chrg:%.1f "
            "byWorkAsap:%d bChrgAsap:%d bDiscAsap:%d bStandby_BLL:%d bStandby_Set:%d bStandby_Auto:%d bStandby_AEMS:%d \n"

            "LSYS:emACDC_WorkStatu:%d 0:IDLE nOnlineACDCNum:%d fACDC_RealTime_Volt:%.1f fACDC_RealTime_Curr:%.1f "
            "fBatt_RealTime_Volt:%.1f fBatt_RealTime_Curr:%.1f fACDCOutRelayVolt:%.1f fActvPwr_PCSGrp:%.1f \n"

            "Calc:emBatt_WorkMode:%d fChrgedEnergy:%.1f \n"

            "+++fTotalPM_Chrg_PwrAbility:%.1f All:%.1f "
            "fACDC_Distri_4_BattChrg_Pwr:%.1f %.1f %.1f %.1f "
            "fMax_Chrg_PwrAbility:%.1f %.1f %.1f %.1f fMax_Chrg_PwrAbilityA:%.1f %.1f %.1f %.1f fMax_Chrg_PwrAbility0:%.1f %.1f %.1f %.1f fBatt_Max_Chrg_Pwr_Req:%.1f %.1f %.1f %.1f \n"

            "---fTotalPM_DisChrging_PwrAbility:%.1f All:%.1f "
            "fBatt_Distri_4_ACDC_Pwr:%.1f %.1f %.1f %.1f "
            "fMax_DisChrg_PwrAbility:%.1f %.1f %.1f %.1f fMax_DisChrg_PwrAbilityA:%.1f %.1f %.1f %.1f fMax_DisChrg_PwrAbility0:%.1f %.1f %.1f %.1f fBatt_Max_DisChrg_Pwr:%.1f %.1f %.1f %.1f \n"
            
            "fACDC_Max_Ability_Pwr:%.1f %.1f %.1f %.1f fChrgedEnergy:%.1f %.1f %.1f %.1f nNodeIdx:%d %d %d %d \n"

            "***ClientSum emEMSCtrlMode_REMS:%d emACDC_WorkStatu_REMS:%d fActvPwrReq_REMS:%.1f emCCUStateSum:%d emPMCtrlModeSum:%d emACDC_WorkStatuSum:%d bStandby_BLLSum:%d byWorkAsapSum:%d "
            "nSOCMinSum:%d nSOCMaxSum:%d fSOCSum:%.1f fActvPwr_PCSGrpSum:%.1f fQ_PCSGrpSum:%.1f "
            "fMax_Chrg_PwrAbilitySum:%.1f fMax_DisChrg_PwrAbilitySum:%.1f AbilitySumA:%.1f %.1f AbilitySum0:%.1f %.1f "
            "fChrgedEnergySum:%.1f fRealTime_MaxCurrSum:%.1f fACDC_RealTime_VoltSum:%.1f fACDC_RealTime_CurrSum:%.1f "
            "f_PgridMeterSum:%.1f f_PcabMeterSum:%.1f\n"
            "Org:emBatt_WorkMode:%d nStopSOC:%u, nActvPwrLmt:%u nBattGrpPwrLmt:%u \n"
            "Now:emBatt_WorkMode:%d nStopSOC:%u, nActvPwrLmt:%u nBattGrpPwrLmt:%u",

            pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_CHRGING ? "+chrg+":(pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_DISCHRG?"-dischrg-":"IDLE"),
            pEMS->nNodeNum, pEMS->emBatt_WorkModeReq, pEMS->bEnter_AutoChrg_SEMS, pEMS->bEnter_AutoDisc_SEMS,
            pCCU->emCCUState, pCCU->emPMCtrlMode, pCalc_Nodes[0].nSOC, pCalc_Nodes[0].nStopSOCReq,

            pEMS->f_PgridMeter, pEMS->f_PcabMeter, pEMS->f_PmaxGridDemand, pEMS->fPacload4Chg, pEMS->fPacload4Dischg,
            pEMS->fGridACInputPwr_Lmt, pEMS->fGridRemainACPwr2Chrg, pEMS->fRemainPwr4Chrg, pEMS->fRemainDemand4Chrg,
            pEMS->byWorkAsap, pEMS->bChrgAsap, pEMS->bDiscAsap, pEMS->bStandby_BLL, pEMS->bStandby_Set, pEMS->bStandby_Auto, pEMS->bStandby_AEMS,

            pBranchNodeData[0].emACDC_WorkStatu, pBranchNodeData[0].nOnlineACDCNum, pBranchNodeData[0].fACDC_RealTime_Volt, pBranchNodeData[0].fACDC_RealTime_Curr, 
            pBranchNodeData[0].fBatt_RealTime_Volt, pBranchNodeData[0].fBatt_RealTime_Curr,
            pBranchNodeData[0].fACDCOutRelayVolt, pBranchNodeData[0].fActvPwr_PCSGrp,

            pCalc_Nodes[0].emBatt_WorkMode, pCalc_Nodes[0].fChrgedEnergy,
            
            pEMS->fTotalPM_Chrg_PwrAbility, pEMS->fTotalPM_Chrg_PwrAbilityAll,
            pCalc_Nodes[0].fACDC_Distri_4_BattChrg_Pwr, pCalc_Nodes[1].fACDC_Distri_4_BattChrg_Pwr, pCalc_Nodes[2].fACDC_Distri_4_BattChrg_Pwr, pCalc_Nodes[3].fACDC_Distri_4_BattChrg_Pwr,
            pCalc_Nodes[0].fMax_Chrg_PwrAbility, pCalc_Nodes[1].fMax_Chrg_PwrAbility, pCalc_Nodes[2].fMax_Chrg_PwrAbility, pCalc_Nodes[3].fMax_Chrg_PwrAbility,
            pCalc_Nodes[0].fMax_Chrg_PwrAbilityA, pCalc_Nodes[1].fMax_Chrg_PwrAbilityA, pCalc_Nodes[2].fMax_Chrg_PwrAbilityA, pCalc_Nodes[3].fMax_Chrg_PwrAbilityA,
            pCalc_Nodes[0].fMax_Chrg_PwrAbility0, pCalc_Nodes[1].fMax_Chrg_PwrAbility0, pCalc_Nodes[2].fMax_Chrg_PwrAbility0, pCalc_Nodes[3].fMax_Chrg_PwrAbility0,
            pCalc_Nodes[0].fBatt_Max_Chrg_Pwr_Req, pCalc_Nodes[1].fBatt_Max_Chrg_Pwr_Req, pCalc_Nodes[2].fBatt_Max_Chrg_Pwr_Req, pCalc_Nodes[3].fBatt_Max_Chrg_Pwr_Req,

            pEMS->fTotalPM_DisChrging_PwrAbility, pEMS->fTotalPM_DisChrging_PwrAbilityAll,
            pCalc_Nodes[0].fBatt_Distri_4_ACDC_Pwr, pCalc_Nodes[1].fBatt_Distri_4_ACDC_Pwr, pCalc_Nodes[2].fBatt_Distri_4_ACDC_Pwr, pCalc_Nodes[3].fBatt_Distri_4_ACDC_Pwr,
            pCalc_Nodes[0].fMax_DisChrg_PwrAbility, pCalc_Nodes[1].fMax_DisChrg_PwrAbility, pCalc_Nodes[2].fMax_DisChrg_PwrAbility, pCalc_Nodes[3].fMax_DisChrg_PwrAbility,
            pCalc_Nodes[0].fMax_DisChrg_PwrAbilityA, pCalc_Nodes[1].fMax_DisChrg_PwrAbilityA, pCalc_Nodes[2].fMax_DisChrg_PwrAbilityA, pCalc_Nodes[3].fMax_DisChrg_PwrAbilityA,
            pCalc_Nodes[0].fMax_DisChrg_PwrAbility0, pCalc_Nodes[1].fMax_DisChrg_PwrAbility0, pCalc_Nodes[2].fMax_DisChrg_PwrAbility0, pCalc_Nodes[3].fMax_DisChrg_PwrAbility0,
            pCalc_Nodes[0].fBatt_Max_DisChrg_Pwr, pCalc_Nodes[1].fBatt_Max_DisChrg_Pwr, pCalc_Nodes[2].fBatt_Max_DisChrg_Pwr, pCalc_Nodes[3].fBatt_Max_DisChrg_Pwr,

            pCalc_Nodes[0].fACDC_Max_Ability_Pwr, pCalc_Nodes[1].fACDC_Max_Ability_Pwr, pCalc_Nodes[2].fACDC_Max_Ability_Pwr, pCalc_Nodes[3].fACDC_Max_Ability_Pwr,
            pCalc_Nodes[0].fChrgedEnergy, pCalc_Nodes[1].fChrgedEnergy, pCalc_Nodes[2].fChrgedEnergy, pCalc_Nodes[3].fChrgedEnergy,
            pCalc_Nodes[0].nNodeIdx, pCalc_Nodes[1].nNodeIdx, pCalc_Nodes[2].nNodeIdx, pCalc_Nodes[3].nNodeIdx,

            pClientSum->emEMSCtrlMode_REMS, pClientSum->emACDC_WorkStatu_REMS, pClientSum->fActvPwrReq_REMS, pClientSum->emCCUStateSum, pClientSum->emPMCtrlModeSum, pClientSum->emACDC_WorkStatuSum, pClientSum->bStandby_BLLSum, pClientSum->byWorkAsapSum, 
            pClientSum->nSOCMinSum, pClientSum->nSOCMaxSum, pClientSum->fSOCSum, pClientSum->fActvPwr_PCSGrpSum, pClientSum->fQ_PCSGrpSum,
            pClientSum->fMax_Chrg_PwrAbilitySum, pClientSum->fMax_DisChrg_PwrAbilitySum, pClientSum->fMax_Chrg_PwrAbilitySumA, pClientSum->fMax_DisChrg_PwrAbilitySumA, pClientSum->fMax_Chrg_PwrAbilitySum0, pClientSum->fMax_DisChrg_PwrAbilitySum0,
            pClientSum->fChrgedEnergySum, pClientSum->fRealTime_MaxCurrSum, pClientSum->fACDC_RealTime_VoltSum, pClientSum->fACDC_RealTime_CurrSum,
            pClientSum->f_PgridMeterSum, pClientSum->f_PcabMeterSum,
            
            pPlanSectorOrg->emBatt_WorkMode, pPlanSectorOrg->nStopSOC, pPlanSectorOrg->nActvPwrLmt, pPlanSectorOrg->nBattGrpPwrLmt,
            pPlanSectorNow->emBatt_WorkMode, pPlanSectorNow->nStopSOC, pPlanSectorNow->nActvPwrLmt, pPlanSectorNow->nBattGrpPwrLmt
        );
    }
#endif

    return 0;
} 
