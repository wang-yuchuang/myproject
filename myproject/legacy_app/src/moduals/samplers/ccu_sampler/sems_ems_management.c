/********************************************************************************
* Copyright(c) Shenzhen Infypower Co., Ltd.
* @author LJH936
* @brief  站级EMS
* @history 2024-12-02 created
1.// LJH250930 加快自动充放响应,WorkASAP
 1)自动转充放电和REMS时，增加上报 fMax_Chrg_PwrAbilityA fMax_DisChrg_PwrAbilityA
 2)设置的截止SOC:bStopChrgBySoc_SetSEMS bStopDiscBySoc_SetSEMS
 3)pClientSum增加emACDC_WorkStatu_REMS
 4)pEMS增加待机:bStandby_BLL byWorkAsap
*******************************************************************************/

// #define _TRACE_PRINT_ // SEMS
#include "sems_sampler_main.h"

// calc_Node
static void Refresh_NodeParam_SEMS(ROUGH_CCU_DATA* pCCU)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    int n = 0, nErr;
    SEMS_B6_LSYS_DATA_INFO* pB6LSYSLast;
    A_BLOB_SEMS_CLIENTINFO *pClient;
    VAR_VALUE varV;
    BOOL bCommFailAll = TRUE, bAllLOCKED = TRUE, bAllFAULTED = TRUE,
        bSystemOK = TRUE, bAllStandby_Set = TRUE, bAllStandby_BLL = TRUE;
    BOOL hasAutoCtrlMode = FALSE;
    BOOL bHasCurrent = FALSE, hasPCSChrgNow = FALSE, hasPCSDischrgNow = FALSE, hasPCSStandby = FALSE;
    BOOL bStopChrgBySoc_Plan = FALSE, bStopDiscBySoc_Plan = FALSE; // 管理计划SOC
    BOOL bStopChrgBySoc_SetSEMS = FALSE, bStopDiscBySoc_SetSEMS = FALSE; // 设置的截止SOC
    BYTE byAllWorkAsap = 0xFF;
    UINT nClientNumCommOK = 0, nSOCMinSum = 100, nSOCMaxSum = 0;
    float fSOCSumClients =  0;
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

    A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Node;
    EMS_LIMIT_SECTOR* pPlanSectorOrg = &pEMS->planSectorOrg;
    EMS_LIMIT_SECTOR* pPlanSectorNow = &pEMS->planSectorNow;
    for(n = 0; n < pEMS->nNodeNum; n++)
    {
        pCalc_Node = &pEMS->calc_Node[n];
        // Ems_init中节点初始化移到这里
        Ems_Reset_Calc_Node(pCalc_Node);
        pCalc_Node->nNodeIdx = n;
        
        // Judge if comm fail
        pClient = &s_MMClients[n];
        if (ElapseS(pClient->tmLastRcvHeatbeat) >= MAX_S_COMMFAIL_SEMS) // Comm Fail (ElapseS(pCCU->tmSysBoot) > MAX_S_WAIT_FIRSTBOOT)
		{
			TRACEW("Refresh_NodeParam_SEMS comm fail single n:%d ElapseS LSYS:%d", n, ElapseS(pClient->tmLastRcvHeatbeat));
            pClient->bCommFailSemsClient = TRUE;
		}
        else
        {
            pClient->bCommFailSemsClient = FALSE;
            bCommFailAll = FALSE;
        }
        TRACE("Refresh_NodeParam_SEMS bCommFailSemsClient:%d n:%d", pClient->bCommFailSemsClient, n);
        varV.emValue = (_ENUM)pClient->bCommFailSemsClient;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3001 + n, varV);

        pB6LSYSLast = &pCCU->strLSYSInfo.branchNodeData[n].b6LSYSLast_SEMS;
        if(pCCU->blobBackendCfg.bEnableQuickResponseChannel == TRUE)
        {
            *pB6LSYSLast = pCCU->strLSYSInfo.branchNodeData[n].b6LSYS_SEMS;
        }
        else
        {
            nErr = Mutex_Lock(pClient->hLock, MAX_MS_VALIDDATA_SEMS);
            if (ERR_MUTEX_OK == nErr)
            {
                *pB6LSYSLast = pCCU->strLSYSInfo.branchNodeData[n].b6LSYS_SEMS;
                Mutex_Unlock(pClient->hLock);
            }
            else
            {
                TRACEE("Refresh_NodeParam_SEMS Mutex_Lock failed n:%d nErr:%d 1:TIMEOUT 2:INVALID_ARG", n, nErr);
            }
        }
        if (pClient->bCommFailSemsClient /*|| ElapseMSBcdTimeStmp(&pB6LSYSLast->tmStamp) > MAX_MS_VALIDDATA_SEMS*/)
        {
            ZERO_POBJS(pB6LSYSLast, 1);
            pB6LSYSLast->emCCUStateSum = CCU_STATE_FAULTED;
            TRACEW("Refresh_NodeParam_SEMS pB6LSYSLast invalid n:%d ElapseMSBcdTimeStmp:%u bCommFailSemsClient-Single:%d",
                n, ElapseMSBcdTimeStmp(&pB6LSYSLast->tmStamp), pClient->bCommFailSemsClient);
        }
        else
        {
            //0.1.fSOCSum 通信正常的客户端
            nClientNumCommOK++;
            nSOCMinSum = MIN(pB6LSYSLast->nSOCMinSum, nSOCMinSum);
            nSOCMaxSum = MAX(pB6LSYSLast->nSOCMaxSum, nSOCMaxSum);
            fSOCSumClients += pB6LSYSLast->fSOCSum;
            if (pB6LSYSLast->emCCUStateSum == CCU_STATE_IDLE)
            {
                bAllLOCKED = FALSE;
                bAllFAULTED = FALSE;
            }
            else if (pB6LSYSLast->emCCUStateSum == CCU_STATE_LOCKED)
            {
                bAllFAULTED = FALSE;
            }
            else if (pB6LSYSLast->emCCUStateSum == CCU_STATE_FAULTED)
            {
                bAllLOCKED = FALSE;
            }
            // Standby_SetReq WorkAsap
            bAllStandby_Set &= pB6LSYSLast->bStandby_SetSum;
            bAllStandby_BLL &= pB6LSYSLast->bStandby_BLLSum;
            byAllWorkAsap &=   pB6LSYSLast->byWorkAsapSum;
        }
        TRACE("Refresh_NodeParam_SEMS pB6LSYSLast n:%d ElapseMSBcdTimeStmp:%u LSYSLast->tmStamp:%s bAllStandby_Set:%d %d byAllWorkAsap:%d",
            n, ElapseMSBcdTimeStmp(&pB6LSYSLast->tmStamp), printableBcdTimeStmp(&pB6LSYSLast->tmStamp), bAllStandby_Set, bAllStandby_BLL, byAllWorkAsap);
        // 自动模式/手动模式 所有手动模式才是手动模式
        if (pB6LSYSLast->emPMCtrlModeSum == AUTO_CTRL_PM_MODE)
        {
            hasAutoCtrlMode = TRUE;
        }

        //0.6. ACDC_WorkStatuSum // LJH250703 added:emCCUState && emPMCtrlMode
        if (pCCU->emCCUState == CCU_STATE_IDLE && pCCU->emPMCtrlMode == AUTO_CTRL_PM_MODE)
        {
            bHasCurrent = (ABS(pB6LSYSLast->fActvPwr_PCSGrpSum) > MIN_PWR_CHRG && pB6LSYSLast->fACDC_RealTime_CurrSum > MIN_CURR_WORKING);
            if (pB6LSYSLast->emACDC_WorkStatuSum == STATU_ACDC_RECTIFY && bHasCurrent)
            {
                hasPCSChrgNow = TRUE;
            }
            if (pB6LSYSLast->emACDC_WorkStatuSum == STATU_ACDC_INVERTER && bHasCurrent)
            {
                hasPCSDischrgNow = TRUE;
            }
            if (pB6LSYSLast->emACDC_WorkStatuSum == STATU_ACDC_STANDBY)
            {
                hasPCSStandby = TRUE;
            }
        }
        //0.9.nTMSector
        if (pEMS->nTMSector == -1)
        {
            TRACEW("Refresh_NodeParam_SEMS continue; pEMS->nTMSector == -1 n:%d", n);
            continue;
        }

        //1. 储能电池最大放电功率&充电功率
        pCalc_Node->fBatt_Max_Chrg_Pwr_Req = pB6LSYSLast->fMax_Chrg_PwrAbilitySum0;
        pCalc_Node->fBatt_Max_DisChrg_Pwr =  pB6LSYSLast->fMax_DisChrg_PwrAbilitySum0;

        //5. M储能电池剩余充电/放电功率
        pCalc_Node->fRemain_Batt_Chrg_Pwr_Req = pCalc_Node->fBatt_Max_Chrg_Pwr_Req;
        pCalc_Node->fRemain_Batt_DisChrg_Pwr =  pCalc_Node->fBatt_Max_DisChrg_Pwr;

        //6.Calc Battery Group Charged Energy
        pCalc_Node->fChrgedEnergy =           pB6LSYSLast->fChrgedEnergySum;

        //7.fMax0 fMaxA充放电能力 // LJH250930 加快自动充放响应,StopSOC_SetSEMS
        bSystemOK = (pB6LSYSLast->emCCUStateSum == CCU_STATE_IDLE && pB6LSYSLast->emPMCtrlModeSum == AUTO_CTRL_PM_MODE &&
            pCCU->emCCUState == CCU_STATE_IDLE && pCCU->emPMCtrlMode == AUTO_CTRL_PM_MODE);
        bStopChrgBySoc_SetSEMS = (pB6LSYSLast->nSOCMinSum >= pEMS->nStopSOC_ChrgSet_SEMS);
        bStopDiscBySoc_SetSEMS = (pB6LSYSLast->nSOCMaxSum <= pEMS->nStopSOC_DiscSet_SEMS);
        
        pCalc_Node->fMax_Chrg_PwrAbility0 =    pB6LSYSLast->fMax_Chrg_PwrAbilitySum0;
        pCalc_Node->fMax_DisChrg_PwrAbility0 = pB6LSYSLast->fMax_DisChrg_PwrAbilitySum0;
        pCalc_Node->fMax_Chrg_PwrAbilityA =    (!bSystemOK || bStopChrgBySoc_SetSEMS) ? 0 : pB6LSYSLast->fMax_Chrg_PwrAbilitySumA;
        pCalc_Node->fMax_DisChrg_PwrAbilityA = (!bSystemOK || bStopDiscBySoc_SetSEMS) ? 0 : pB6LSYSLast->fMax_DisChrg_PwrAbilitySumA;
        
        //8.fMax充放电能力和emBatt_WorkMode // LJH250606 fix:从控故障主控还继续下发充放电-故障或手动状态上报Max功率能力为0
        pCalc_Node->emBatt_WorkMode = pEMS->emBatt_WorkModeReq;
        pCalc_Node->nStopSOCReq = pPlanSectorNow->nStopSOC;
        if ( !bSystemOK )
        {
            pCalc_Node->fMax_Chrg_PwrAbility =    0;
            pCalc_Node->fMax_DisChrg_PwrAbility = 0;
            TRACEW("Refresh_NodeParam_SEMS !bSystemOK n:%d set fMax_Chrg_PwrAbility=0 emCCUState:%d emPMCtrlMode:%d planSectorOrg.emBatt_WorkMode:%d",
                n, pCCU->emCCUState, pCCU->emPMCtrlMode, pPlanSectorOrg->emBatt_WorkMode);
        }
        else
        {
            if (!pEMS->bChrgAsap && !pEMS->bDiscAsap)
            {
                //8.1 work by plan
                pCalc_Node->fMax_Chrg_PwrAbility =    pB6LSYSLast->fMax_Chrg_PwrAbilitySum;
                pCalc_Node->fMax_DisChrg_PwrAbility = pB6LSYSLast->fMax_DisChrg_PwrAbilitySum;
                
                //8.2 fMax_Chrg_PwrAbility and emBatt_WorkMode by SOC LJH250620 added
                // set Calc_Node's emBatt_WorkMode by SOC // LJH250606 fix:主控SOC因无小数部分放电时过早停止 fSOCSum改为nSOCMinSum/nSOCMaxSum
                bStopChrgBySoc_Plan = (pB6LSYSLast->nSOCMinSum >= (BYTE)pCalc_Node->nStopSOCReq);
                bStopDiscBySoc_Plan = (pB6LSYSLast->nSOCMaxSum <= (BYTE)pCalc_Node->nStopSOCReq);
                if (bStopChrgBySoc_Plan || bStopChrgBySoc_SetSEMS)
                {
                    pCalc_Node->fMax_Chrg_PwrAbility = 0;
                    if (pCalc_Node->emBatt_WorkMode == BATT_WORKMODE_FIRST_CHRGING)
                    {
                        pCalc_Node->emBatt_WorkMode = BATT_WORKMODE_IDLE;
                    }
                    TRACEW("Refresh_NodeParam_SEMS bStopChrgBySoc:%d %d set fMax_Chrg_PwrAbility = 0 BATT_WORKMODE_IDLE n:%d nSOCMinSum:%d >= nStopSOCReq:%d ",
                        bStopChrgBySoc_Plan, bStopChrgBySoc_SetSEMS, n, pB6LSYSLast->nSOCMinSum, pCalc_Node->nStopSOCReq);
                }
                if (bStopDiscBySoc_Plan || bStopDiscBySoc_SetSEMS)
                {
                    pCalc_Node->fMax_DisChrg_PwrAbility = 0;
                    if (pCalc_Node->emBatt_WorkMode == BATT_WORKMODE_FIRST_DISCHRG)
                    {
                        pCalc_Node->emBatt_WorkMode = BATT_WORKMODE_IDLE;
                    }
                    TRACEW("Refresh_NodeParam_SEMS bStopDiscBySoc:%d %d set fMax_DisChrg_PwrAbility = 0 n:%d nSOCMaxSum:%d <= nStopSOCReq:%d",
                        bStopDiscBySoc_Plan, bStopDiscBySoc_SetSEMS, n, pB6LSYSLast->nSOCMaxSum, pCalc_Node->nStopSOCReq);
                }
            }
            else
            {
                //8.6 WorkASAP // LJH250930 加快自动充放响应,WorkASAP
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
        if (pCalc_Node->emBatt_WorkMode == BATT_WORKMODE_FIRST_CHRGING)
        {
            pCalc_Node->nSOC = pB6LSYSLast->nSOCMinSum;
        }
        else if (pCalc_Node->emBatt_WorkMode == BATT_WORKMODE_FIRST_DISCHRG)
        {
            pCalc_Node->nSOC = pB6LSYSLast->nSOCMaxSum;
        }

        //20.所有客户端累加数据
        pClientSum->fActvPwr_PCSGrpSum +=      pB6LSYSLast->fActvPwr_PCSGrpSum;
        pClientSum->fQ_PCSGrpSum +=            pB6LSYSLast->fQ_PCSGrpSum;
        
        pClientSum->fMax_Chrg_PwrAbilitySum +=     pCalc_Node->fMax_Chrg_PwrAbility;
        pClientSum->fMax_DisChrg_PwrAbilitySum +=  pCalc_Node->fMax_DisChrg_PwrAbility;
        pClientSum->fMax_Chrg_PwrAbilitySumA +=    pCalc_Node->fMax_Chrg_PwrAbilityA; // LJH250930 加快自动充放响应,WorkASAP
        pClientSum->fMax_DisChrg_PwrAbilitySumA += pCalc_Node->fMax_DisChrg_PwrAbilityA;
        pClientSum->fMax_Chrg_PwrAbilitySum0 +=    pCalc_Node->fMax_Chrg_PwrAbility0;
        pClientSum->fMax_DisChrg_PwrAbilitySum0 += pCalc_Node->fMax_DisChrg_PwrAbility0;

        pClientSum->fChrgedEnergySum +=           pB6LSYSLast->fChrgedEnergySum;
        pClientSum->fRealTime_MaxCurrSum +=       pB6LSYSLast->fRealTime_MaxCurrSum;
        pClientSum->fACDC_RealTime_VoltSum =      MAX(pClientSum->fACDC_RealTime_VoltSum, pB6LSYSLast->fACDC_RealTime_VoltSum);
        pClientSum->fACDC_RealTime_CurrSum +=     pB6LSYSLast->fACDC_RealTime_CurrSum;
        //61.LJH250401 topology#7 一级主控
        if (pCCU->emAuxMeterPos == MULTI_BANCH_ESS_WITHOUT_HVMETER &&
            pCCU->lcCfg.lcType == LC_TYPE_1st && !pEMS->bGMeter_SEMS)
        {
            pClientSum->f_PgridMeterSum +=    pB6LSYSLast->f_PgridMeterSum;
            // pClientSum->fPacload4ChgSum +=    pB6LSYSLast->fPacload4ChgSum;
            // pClientSum->fPacload4DischgSum += pB6LSYSLast->fPacload4DischgSum;
        }
        pClientSum->f_PcabMeterSum += pB6LSYSLast->f_PcabMeterSum;

        //62.待机 // LJH250930 加快自动充放响应,standbySetReq
        if (pB6LSYSLast->bStandby_BLLSum != pEMS->bStandby_BLL && ElapseMS(pCalc_Node->fTmLastSend6360Standby) > 800)
        {
            pCalc_Node->fTmLastSend6360Standby = GetCurrentTime();
            Send_6360UCTL_Standby(n, pEMS->bStandby_BLL);
        }
        TRACED("Refresh_NodeParam_SEMS n:%d Now:nBattGrpPwrLmt:%u nActvPwrLmt:%u "
            "LSYS:fMax_Chrg_PwrAbilitySum:%.1f fMax_DisChrg_PwrAbilitySum:%.1f "
            "bStandby_SetSum:%d bStandby_BLLSum:%d "
            "calc:fMax_Chrg_PwrAbility:%.1f dis:%.1f bStandby_BLL:%d",
            n, pPlanSectorNow->nBattGrpPwrLmt, pPlanSectorNow->nActvPwrLmt,
            pB6LSYSLast->fMax_Chrg_PwrAbilitySum, pB6LSYSLast->fMax_DisChrg_PwrAbilitySum,
            pB6LSYSLast->bStandby_SetSum, pB6LSYSLast->bStandby_BLLSum,
            pCalc_Node->fMax_Chrg_PwrAbility, pCalc_Node->fMax_DisChrg_PwrAbility, pEMS->bStandby_BLL);
    } // end for
    // sum1:状态
    pEMS->hasPCSChrgLast_SEMS =    hasPCSChrgNow;
    pEMS->hasPCSDischrgLast_SEMS = hasPCSDischrgNow;
    pCCU->bCommFail = bCommFailAll;
    // LJH250606 fix:从控故障主控还继续下发充放电
    if (bCommFailAll || bAllFAULTED || pCCU->emCCUState == CCU_STATE_FAULTED)
    {
        pClientSum->emCCUStateSum = CCU_STATE_FAULTED;
    }
    else if (bAllLOCKED || pCCU->emCCUState == CCU_STATE_LOCKED)
    {
        pClientSum->emCCUStateSum = CCU_STATE_LOCKED;
    }
    pClientSum->emPMCtrlModeSum = (BYTE)((hasAutoCtrlMode && pCCU->emPMCtrlMode == AUTO_CTRL_PM_MODE) ?
                                    AUTO_CTRL_PM_MODE : MANUAL_CTRL_PM_MODE);
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
    pClientSum->bStandby_SetSum = (BYTE)bAllStandby_Set;
    pClientSum->bStandby_BLLSum = (BYTE)bAllStandby_BLL;
    pClientSum->byWorkAsapSum =   byAllWorkAsap;

    // sum2:SOC
    if (nClientNumCommOK > 0)
    {
        pClientSum->nSOCMinSum = (BYTE)nSOCMinSum;
        pClientSum->nSOCMaxSum = (BYTE)nSOCMaxSum;
        pClientSum->fSOCSum =    (float)fSOCSumClients / (float)nClientNumCommOK;
    }
    else
    {
        pClientSum->nSOCMinSum = 0;
        pClientSum->nSOCMaxSum = 0;
        pClientSum->fSOCSum = 0;
    }
    // LJH250325 AntiOvldRflxByLC
    if (pEMS->enableAntiOvld_REMS)
    {
        pClientSum->fMax_Chrg_PwrAbilitySum =  MIN(pEMS->fTotal_PessChgAvaliable_Org, pClientSum->fMax_Chrg_PwrAbilitySum);
        pClientSum->fMax_Chrg_PwrAbilitySumA = MIN(pEMS->fTotal_PessChgAvaliable_Org, pClientSum->fMax_Chrg_PwrAbilitySumA);
        TRACEW("Refresh_NodeParam_SEMS enableAntiOvld_REMS ClientSum:fMax_Chrg_PwrAbilitySum:%.1f fTotal_PessChgAvaliable_Org:%.1f fTotal_PessChgAvaliable:%.1f",
            pClientSum->fMax_Chrg_PwrAbilitySum, pEMS->fTotal_PessChgAvaliable_Org, pEMS->fTotal_PessChgAvaliable);
    }
    if (pEMS->enableAntiRflx_REMS)
    {
        pClientSum->fMax_DisChrg_PwrAbilitySum =  MIN(pEMS->fTotal_PessDischgLmt_Org, pClientSum->fMax_DisChrg_PwrAbilitySum);
        pClientSum->fMax_DisChrg_PwrAbilitySumA = MIN(pEMS->fTotal_PessDischgLmt_Org, pClientSum->fMax_DisChrg_PwrAbilitySumA);
        TRACEI("Refresh_NodeParam_SEMS enableAntiRflx_REMS fMax_DisChrg_PwrAbilitySum:%.1f fTotal_PessDischgLmt_Org:%.1f fTotal_PessDischgLmt:%.1f",
            pClientSum->fMax_DisChrg_PwrAbilitySum, pEMS->fTotal_PessDischgLmt_Org, pEMS->fTotal_PessDischgLmt);
    }
    // sum3:meter
    // LJH250910 modified
    if (pEMS->bGMeter_SEMS)
    {
        pClientSum->f_PgridMeterSum = pEMS->f_PgridMeter;
    }
    // LJH251111 AEMS
    pClientSum->fPacload4ChgSum =    pEMS->fPacload4Chg;
	pClientSum->fPacload4DischgSum = pEMS->fPacload4Dischg;

    // LJH250910 modified old
    // if (pCCU->lcCfg.lcType == LC_TYPE_2nd)
    // {
    //     if (pCCU->emAuxMeterPos == MULTI_BANCH_ESS_WITHOUT_HVMETER && pEMS->bGMeter_SEMS)
    //     {// LJH250401 topology#7 二级主控（安装了H网侧表）
    //         pClientSum->f_PgridMeterSum =    pEMS->f_PgridMeter;
    //     }
    // }
    // sum4:maxAbility LJH250818 最大功率限制
    if (pCCU->lcCfg.lcType > LC_TYPE_1st)
    {
        pClientSum->fMax_Chrg_PwrAbilitySum =    MIN(pCCU->fTotalPowerLimit, pClientSum->fMax_Chrg_PwrAbilitySum);
        pClientSum->fMax_DisChrg_PwrAbilitySum = MIN(pCCU->fTotalPowerLimit, pClientSum->fMax_DisChrg_PwrAbilitySum);
        pClientSum->fMax_Chrg_PwrAbilitySumA =    MIN(pCCU->fTotalPowerLimit, pClientSum->fMax_Chrg_PwrAbilitySumA);
        pClientSum->fMax_DisChrg_PwrAbilitySumA = MIN(pCCU->fTotalPowerLimit, pClientSum->fMax_DisChrg_PwrAbilitySumA);
    }
#ifdef _TRACE_PRINT_
    {
        A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Nodes = pEMS->calc_Node;
        A_BLOB_BRANCH_GRP_RECTDATA* pBranchNodeData = pCCU->strLSYSInfo.branchNodeData;
        pB6LSYSLast = &pCCU->strLSYSInfo.branchNodeData[0].b6LSYSLast_SEMS;
        TRACEI("Refresh_NodeParam_SEMS Mutex_Lock:%d nNodeNum:%d "
            "emBatt_WorkModeReq:%d 5:idle "
            "nNodeIdx:%d %d %d %d fChrgedEnergy:%.1f %.1f %.1f %.1f "
            "fSOCSum:%.1f f_PcabMeterSum:%.1f fActvPwr_PCSGrpSum:%.1f fQ_PCSGrpSum:%.1f "
            "fChrgedEnergySum:%.1f %.1f %.1f %.1f fRealTime_MaxCurrSum:%.1f fACDC_RealTime_VoltSum:%.1f fACDC_RealTime_CurrSum:%.1f "
            "LSYS:fMax_Chrg_PwrAbilitySum:%.1f fMax_DisChrg_PwrAbilitySum:%.1f fTotalPowerLimit:%.1f \n"
            "emCCUStateSum:%d %d %d %d emPMCtrlModeSum:%d 0:AUTO_CTRL emACDC_WorkStatuSum:%d %d %d %d \n"
            "nSOC:%d %d %d %d nStopSOCReq:%d %d %d %d \n"
            ">>>fMax_Chrg_PwrAbility:%.1f %.1f %.1f %.1f fBatt_Max_Chrg_Pwr_Req:%.1f %.1f %.1f %.1f \n"
            "<<<fMax_DisChrg_PwrAbility:%.1f %.1f %.1f %.1f fBatt_Max_DisChrg_Pwr:%.1f %.1f %.1f %.1f\n"
            "bCommFailAll:%d bAllLOCKED:%d bAllFAULTED:%d hasPCSChrgNow:%d hasPCSDischrgNow:%d hasPCSStandby:%d \n"
            "ClientSum emEMSCtrlMode_REMS:%d emACDC_WorkStatu_REMS:%d fActvPwrReq_REMS:%.1f emCCUStateSum:%d emPMCtrlModeSum:%d emACDC_WorkStatuSum:%d bStandby_BLLSum:%d byWorkAsapSum:%d "
            "nSOCMinSum:%d nSOCMaxSum:%d fSOCSum:%.1f fActvPwr_PCSGrpSum:%.1f fQ_PCSGrpSum:%.1f "
            "\nfMax_Chrg_PwrAbilitySum:%.1f fMax_DisChrg_PwrAbilitySum:%.1f AbilitySum0:%.1f %.1f "
            "fChrgedEnergySum:%.1f fRealTime_MaxCurrSum:%.1f fACDC_RealTime_VoltSum:%.1f fACDC_RealTime_CurrSum:%.1f "
            "f_PgridMeterSum:%.1f f_PcabMeterSum:%.1f\n"
            "Org:emBatt_WorkMode:%d nStopSOC:%u, nActvPwrLmt:%u nBattGrpPwrLmt:%u \n"
            "Now:emBatt_WorkMode:%d nStopSOC:%u, nActvPwrLmt:%u nBattGrpPwrLmt:%u",
            nErr, pEMS->nNodeNum,
            pEMS->emBatt_WorkModeReq,
            pCalc_Nodes[0].nNodeIdx, pCalc_Nodes[1].nNodeIdx, pCalc_Nodes[2].nNodeIdx, pCalc_Nodes[3].nNodeIdx,
            pCalc_Nodes[0].fChrgedEnergy, pCalc_Nodes[1].fChrgedEnergy, pCalc_Nodes[2].fChrgedEnergy, pCalc_Nodes[3].fChrgedEnergy,
            pB6LSYSLast->fSOCSum, pB6LSYSLast->f_PcabMeterSum, pB6LSYSLast->fActvPwr_PCSGrpSum, pB6LSYSLast->fQ_PCSGrpSum,
            pBranchNodeData[0].b6LSYSLast_SEMS.fChrgedEnergySum, pBranchNodeData[1].b6LSYSLast_SEMS.fChrgedEnergySum, pBranchNodeData[2].b6LSYSLast_SEMS.fChrgedEnergySum, pBranchNodeData[3].b6LSYSLast_SEMS.fChrgedEnergySum,
            pB6LSYSLast->fRealTime_MaxCurrSum, pB6LSYSLast->fACDC_RealTime_VoltSum, pB6LSYSLast->fACDC_RealTime_CurrSum,
            pB6LSYSLast->fMax_Chrg_PwrAbilitySum, pB6LSYSLast->fMax_DisChrg_PwrAbilitySum, pCCU->fTotalPowerLimit,
            pBranchNodeData[0].b6LSYSLast_SEMS.emCCUStateSum, pBranchNodeData[1].b6LSYSLast_SEMS.emCCUStateSum, pBranchNodeData[2].b6LSYSLast_SEMS.emCCUStateSum, pBranchNodeData[3].b6LSYSLast_SEMS.emCCUStateSum,
            pB6LSYSLast->emPMCtrlModeSum,
            pBranchNodeData[0].b6LSYSLast_SEMS.emACDC_WorkStatuSum, pBranchNodeData[1].b6LSYSLast_SEMS.emACDC_WorkStatuSum, pBranchNodeData[2].b6LSYSLast_SEMS.emACDC_WorkStatuSum, pBranchNodeData[3].b6LSYSLast_SEMS.emACDC_WorkStatuSum,
            pCalc_Nodes[0].nSOC, pCalc_Nodes[1].nSOC, pCalc_Nodes[2].nSOC, pCalc_Nodes[3].nSOC,
            pCalc_Nodes[0].nStopSOCReq, pCalc_Nodes[1].nStopSOCReq, pCalc_Nodes[2].nStopSOCReq, pCalc_Nodes[3].nStopSOCReq,

            pCalc_Nodes[0].fMax_Chrg_PwrAbility, pCalc_Nodes[1].fMax_Chrg_PwrAbility, pCalc_Nodes[2].fMax_Chrg_PwrAbility, pCalc_Nodes[3].fMax_Chrg_PwrAbility,
            pCalc_Nodes[0].fBatt_Max_Chrg_Pwr_Req, pCalc_Nodes[1].fBatt_Max_Chrg_Pwr_Req, pCalc_Nodes[2].fBatt_Max_Chrg_Pwr_Req, pCalc_Nodes[3].fBatt_Max_Chrg_Pwr_Req,
            pCalc_Nodes[0].fMax_DisChrg_PwrAbility, pCalc_Nodes[1].fMax_DisChrg_PwrAbility, pCalc_Nodes[2].fMax_DisChrg_PwrAbility, pCalc_Nodes[3].fMax_DisChrg_PwrAbility,
            pCalc_Nodes[0].fBatt_Max_DisChrg_Pwr, pCalc_Nodes[1].fBatt_Max_DisChrg_Pwr, pCalc_Nodes[2].fBatt_Max_DisChrg_Pwr, pCalc_Nodes[3].fBatt_Max_DisChrg_Pwr,

            bCommFailAll, bAllLOCKED, bAllFAULTED, hasPCSChrgNow, hasPCSDischrgNow, hasPCSStandby,
            pClientSum->emEMSCtrlMode_REMS, pClientSum->emACDC_WorkStatu_REMS, pClientSum->fActvPwrReq_REMS, pClientSum->emCCUStateSum, pClientSum->emPMCtrlModeSum, pClientSum->emACDC_WorkStatuSum, pClientSum->bStandby_BLLSum, pClientSum->byWorkAsapSum,
            pClientSum->nSOCMinSum, pClientSum->nSOCMaxSum, pClientSum->fSOCSum, pClientSum->fActvPwr_PCSGrpSum, pClientSum->fQ_PCSGrpSum,
            pClientSum->fMax_Chrg_PwrAbilitySum, pClientSum->fMax_DisChrg_PwrAbilitySum, pClientSum->fMax_Chrg_PwrAbilitySum0, pClientSum->fMax_DisChrg_PwrAbilitySum0,
            pClientSum->fChrgedEnergySum, pClientSum->fRealTime_MaxCurrSum, pClientSum->fACDC_RealTime_VoltSum, pClientSum->fACDC_RealTime_CurrSum,
            pClientSum->f_PgridMeterSum, pClientSum->f_PcabMeterSum,
            
            pPlanSectorOrg->emBatt_WorkMode, pPlanSectorOrg->nStopSOC, pPlanSectorOrg->nActvPwrLmt, pPlanSectorOrg->nBattGrpPwrLmt,
            pPlanSectorNow->emBatt_WorkMode, pPlanSectorNow->nStopSOC, pPlanSectorNow->nActvPwrLmt, pPlanSectorNow->nBattGrpPwrLmt
        );
    }
#endif
}

static void Filter_Disttribute_Power_QAndClientSum_SEMS(ROUGH_CCU_DATA *pCCU, int n, BOOL bCalcQ)
{
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    if (bCalcQ)
    {   
        if (pCCU->bQuickChannle || pEMS->emHighSensitivityEnable)
        {
            // printf("Filter_Disttribute_Power_QAndClientSum_SEMS: %d\n", pEMS->emHighSensitivityEnable);
            pEMS->calc_Node[n].fAvg_QoutputLmt = pEMS->calc_Node[n].fQoutput_Distri4ACDC;
        }
        else
        {
            pEMS->calc_Node[n].fAvg_QoutputLmt = 
                Samp_Avg_Filter(n + FILTERTYPE_CALC_QLMT_GRP0, pEMS->calc_Node[n].fQoutput_Distri4ACDC, MIN_FILTER_POINT);
        }
        pEMS->calc_Node[n].fAvg_QoutputLmt = pEMS->fDirect*pEMS->calc_Node[n].fAvg_QoutputLmt;
    }
    
    Update_RuntimeData_2DB_SEMS(pCCU, n);

    s_MMClients[n].sems = *pEMS;
    Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3301 + n,
        (void*)&s_MMClients[n], sizeof(A_BLOB_SEMS_CLIENTINFO), BLOB_TYPE_SEMS_CLIENTNFO);
    // if (pEMS->enableREMSCtrl)
    {
        // to Upper
        Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3900,
            (void*)&pEMS->clientInfoSum, sizeof(A_BLOB_SEMS_CLIENTINFO_SUM), BLOB_TYPE_SEMS_CLIENTNFO_SUM);
        TRACE("Filter_Disttribute_Power_QAndClientSum_SEMS fSOCSum:%.1f", pEMS->clientInfoSum.fSOCSum);
    }
}

static void Filter_Disttribute_Power_SEMS(ROUGH_CCU_DATA *pCCU)
{
    EMS_MGMT_STU *pEMS	= 	&pCCU->emsMgmtData;
    int n = 0;
    if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_CHRGING)
    {
        for(n = 0; n < pEMS->nNodeNum; n++)
        {
            if (pCCU->bQuickChannle || pEMS->emHighSensitivityEnable)
            {
                // printf("Filter_Disttribute_Power_SEMS Charge: %d\n", pEMS->emHighSensitivityEnable);
                pEMS->calc_Node[n].fAvg_ChrgLmtPower = pEMS->calc_Node[n].fACDC_Distri_4_BattChrg_Pwr;
            }
            else
            {
                pEMS->calc_Node[n].fAvg_ChrgLmtPower = 
                    Samp_Avg_Filter(n + FILTERTYPE_CALC_CHRGPOWER_GRP0, pEMS->calc_Node[n].fACDC_Distri_4_BattChrg_Pwr, MIN_FILTER_POINT);
            }
    
            if (pEMS->calc_Node[n].fAvg_ChrgLmtPower < MIN_PWR_PCS_OUT_REQ)
            {// LJH241202 SEMS
                TRACEW("Filter_Disttribute_Power_SEMS set BATT_WORKMODE_IDLE fAvg_ChrgLmtPower:%.1f n:%d", pEMS->calc_Node[n].fAvg_ChrgLmtPower, n);
                pEMS->calc_Node[n].emBatt_WorkMode = BATT_WORKMODE_IDLE;
            }
            Filter_Disttribute_Power_QAndClientSum_SEMS(pCCU, n, TRUE);
        }
    }
    else if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_DISCHRG)
    {
        for(n = 0; n < pEMS->nNodeNum; n++)
        {
            if (pCCU->bQuickChannle || pEMS->emHighSensitivityEnable)
            {
                // printf("Filter_Disttribute_Power_SEMS DisCharge: %d\n", pEMS->emHighSensitivityEnable);
                pEMS->calc_Node[n].fAvg_DisChrgingLmtPower = pEMS->calc_Node[n].fBatt_Distri_4_ACDC_Pwr;
            }
            else
            {
                pEMS->calc_Node[n].fAvg_DisChrgingLmtPower = 
                    Samp_Avg_Filter(n + FILTERTYPE_CALC_DISCHRGPOWER_GRP0, pEMS->calc_Node[n].fBatt_Distri_4_ACDC_Pwr, (int)pCCU->nDisChrgFilterPoint);//MIN_FILTER_POINT
            }

            if (pEMS->calc_Node[n].fAvg_DisChrgingLmtPower < MIN_PWR_PCS_OUT_REQ)
            {// LJH241202 SEMS
                TRACEW("Filter_Disttribute_Power_SEMS set BATT_WORKMODE_IDLE fAvg_DisChrgingLmtPower:%.1f < %.1f n:%d",
                    pEMS->calc_Node[n].fAvg_DisChrgingLmtPower, MIN_PWR_PCS_OUT_REQ, n);
                pEMS->calc_Node[n].emBatt_WorkMode = BATT_WORKMODE_IDLE;
            }
            Filter_Disttribute_Power_QAndClientSum_SEMS(pCCU, n, TRUE);
        }
    }
    else
    {
        for(n = 0; n < pEMS->nNodeNum; n++)
        {
            Filter_Disttribute_Power_QAndClientSum_SEMS(pCCU, n, FALSE);
        }
    }
    
#ifdef _TRACE_PRINT_
    A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Nodes = pEMS->calc_Node;
    TRACED("Filter_Disttribute_Power_SEMS nNodeIdx:%d %d %d %d "
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

int Ems_Mgmt_Main_SEMS(ROUGH_CCU_DATA* pCCU)
{
    TRACED("Ems_Mgmt_Main_SEMS lcType:%d", pCCU->lcCfg.lcType);
    EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
    Ems_Mgmt_Total_Pess_All(pCCU);
    
    // old // LJH250902 top#6+REMS
    // if (pCCU->lcCfg.lcType == LC_TYPE_1st)
    // {
    //     // 一级主控
    //     // LJH250205 REMS
    //     if (pEMS->enableREMSCtrl)
    //     {
    //         GetCtrlParam_REMS(pCCU);
    //         if (pEMS->clientInfoSum.emEMSCtrlMode_REMS == EMSCtrlMode_Remote)
    //         {
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
    //         TRACED("Ems_Mgmt_Main_SEMS bCtrlByREMS==FALSE changedBlobEmsCfgPlan:%d bAutoDischrgEnable:%d bAutoChrgEnable:%d bEnter_AutoDisc_SEMS:%d bEnter_AutoChrg_SEMS:%d",
    //             pEMS->changedBlobEmsCfgPlan, pEMS->bAutoDischrgEnable, pEMS->bAutoChrgEnable, pEMS->bEnter_AutoDisc_SEMS, pEMS->bEnter_AutoChrg_SEMS);
    //         if (pEMS->changedBlobEmsCfgPlan)
    //         {
    //             ModifyBlobEmsCfgPlan(pCCU, TRUE, NULL, NULL); // restored
    //         }
    //         GetCurrentTimeSector(pCCU, TRUE);
    //         Ems_Mgmt_Total_Pess(pCCU);
    //         // LJH250612 增加EMS_LIMIT_SECTOR的参数，主控限制总功率nActvPwrLmt，ESC限制 nBattGrpPwrLmt
    //         // if (pCCU->lcCfg.emSystemType == SYSTYPE_EMS_STATION) // 一定是SYSTYPE_EMS_STATION
    //         {
    //             pEMS->nBattGrpPwrLmt = pEMS->planSectorNow.nBattGrpPwrLmt;
    //             pEMS->planSectorNow.nActvPwrLmt =    MAX_PWR_LMT;
    //             pEMS->planSectorNow.nBattGrpPwrLmt = MAX_PWR_LMT; // Dispatch_ESSLmtBattChrg_Power DisPatch_ACLoadReq_DisChrg_Power
    //         }
    //         // LJH250930 加快自动充放响应,ChgDisOnly
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
    //                 pEMS->planSector4USYSIfAutoChDis = pEMS->planSectorNow;
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
    //                 pEMS->planSector4USYSIfAutoChDis = pEMS->planSectorNow;
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
    //         pEMS->bStandby_BLL = (pEMS->planSectorOrg.emBatt_WorkMode==BATT_WORKMODE_STANDBY || pEMS->bStandby_Auto || pEMS->bStandby_Set || pEMS->bStandby_AEMS);
    //     }
    //     // LJH250930 加快自动充放响应
    //     pEMS->bChrgAsap = (pEMS->bEnter_AutoChrg_SEMS || pEMS->bCtrlByREMS);
    //     pEMS->bDiscAsap = (pEMS->bEnter_AutoDisc_SEMS || pEMS->bCtrlByREMS);
    //     pEMS->byWorkAsap = Make_WorkAsap(pEMS->bChrgAsap, pEMS->bDiscAsap).byWorkAsapData;
    //     TRACED("Ems_Mgmt_Main_SEMS bCtrlByREMS:%d changedBlobEmsCfgPlan:%d bChrgAsap:%d bDiscAsap:%d \n"
    //         "bStandby_BLL:%d bStandby_Auto:%d bStandby_Set:%d bEnter_AutoChrg_SEMS:%d bEnter_AutoDisc_SEMS:%d",
    //         pEMS->bCtrlByREMS, pEMS->changedBlobEmsCfgPlan, pEMS->bChrgAsap, pEMS->bDiscAsap,
    //         pEMS->bStandby_BLL, pEMS->bStandby_Auto, pEMS->bStandby_Set, pEMS->bEnter_AutoChrg_SEMS, pEMS->bEnter_AutoDisc_SEMS);
    // }
    // else
    // {
    //     // 从机 LJH0325 AntiOvldRflxByLC
    //     if (pEMS->enableAntiOvld_REMS || pEMS->enableAntiRflx_REMS)
    //     {
    //         // LJH250812 added:for Calc_ESS_ChgLmtPower Calc_ESS_DischgLmtPower计算pess
    //         pEMS->nTMSector = 0;
    //         pEMS->planSectorNow.nActvPwrLmt = MAX_PWR_LMT;
    //         Ems_Mgmt_Total_Pess(pCCU);
    //         pEMS->fTotal_PessDischgLmt_Org =    pEMS->fTotal_PessDischgLmt;
    //         pEMS->fTotal_PessChgAvaliable_Org = pEMS->fTotal_PessChgAvaliable;
    //         Ems_Mgmt_Total_Pess_EMSClient(pCCU);
    //         if (pEMS->enableAntiOvld_REMS)
    //         {
    //             pEMS->fTotal_PessChgAvaliable = MIN(pEMS->fTotal_PessChgAvaliable, pEMS->fTotal_PessChgAvaliable_Org);
    //             TRACEW("Ems_Mgmt_Main_SEMS enableAntiOvld_REMS fTotal_PessChgAvaliable_Org:%.1f fTotal_PessChgAvaliable:%.1f",
    //                 pEMS->fTotal_PessChgAvaliable_Org, pEMS->fTotal_PessChgAvaliable);
    //         }
    //         if (pEMS->enableAntiRflx_REMS)
    //         {
    //             pEMS->fTotal_PessDischgLmt = MIN(pEMS->fTotal_PessDischgLmt, pEMS->fTotal_PessDischgLmt_Org);
    //             TRACEI("Ems_Mgmt_Main_SEMS enableAntiRflx_REMS fTotal_PessDischgLmt_Org:%.1f fTotal_PessDischgLmt:%.1f",
    //                 pEMS->fTotal_PessDischgLmt_Org, pEMS->fTotal_PessDischgLmt);
    //         }
    //     }
    //     else
    //     {
    //         Ems_init(pCCU);
    //         Ems_Mgmt_Total_Pess_EMSClient(pCCU);
    //     }
    // }

    Power_Control_Loop(pCCU);
    
    Refresh_ESSRemainPower(pCCU);
    // calced pEMS->emBatt_WorkModeReq
    
    // 电池、模块最大充、放电功率
    Refresh_NodeParam_SEMS(pCCU);
    // calced pEMS->calc_Node[n].emBatt_WorkMode bySoc

    //!!! After Refresh ALL Node Param,Then Sort Node Order
    if(pEMS->emSortMethod == SORT_BY_CHGENG)
    {
        Sort_NodeOrderByChrgedEnergy(pCCU);
    }
    else // pEMS->emSortMethod == SORT_BY_SOC
    {
        if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_CHRGING)
        {
            Sort_NodeOrderBySoc(pCCU, CHARGE);
        }
        else if (pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_DISCHRG)
        {
            Sort_NodeOrderBySoc(pCCU, DISCHARGE);
        }
    }

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
    
    //Allocate Reactive Power When ESS is Chging/DisChging.
    DisPatch_ReactivePower_Output(pCCU);
    
    Sort_NodeOrderByNodeIdx(pCCU); // Resort The Position To Before Accoding To Chrged Energy.
    Filter_Disttribute_Power_SEMS(pCCU);

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

        SEMS_B6_LSYS_DATA_INFO* pB6LSYS0 = &pBranchNodeData[0].b6LSYSLast_SEMS;
        SEMS_B6_LSYS_DATA_INFO* pB6LSYS1 = &pBranchNodeData[1].b6LSYSLast_SEMS;
        SEMS_B6_LSYS_DATA_INFO* pB6LSYS2 = &pBranchNodeData[2].b6LSYSLast_SEMS;
        SEMS_B6_LSYS_DATA_INFO* pB6LSYS3 = &pBranchNodeData[3].b6LSYSLast_SEMS;
        LOGFD("Ems_Mgmt_Main_SEMS WorkModeReq:%s nNodeNum:%d emBatt_WorkModeReq:%d 5:idle "
            "bEnter_AutoChrg_SEMS:%d bEnter_AutoDisc_SEMS:%d "
            "emCCUState:%d emPMCtrlMode:%d 0:AUTO_CTRL nStopSOCReq:%d \n"

            "f_PgridMeter:%.1f f_PcabMeter:%.1f f_PmaxGridDemand:%.1f fPacload4Chg:%.1f fPacload4Dischg:%.1f "
            "fGridACInputPwr_Lmt:%.1f fGridRemainACPwr2Chrg:%.1f fRemainPwr4Chrg:%.1f fRemainDemand4Chrg:%.1f "
            "byWorkAsap:%d bChrgAsap:%d bDiscAsap:%d bStandby_BLL:%d bStandby_Set:%d bStandby_Auto:%d bStandby_AEMS:%d \n"

            "LSYS:emACDC_WorkStatuSum:%d %d %d %d bStandby_BLLSum:%d %d %d %d byWorkAsapSum:%d %d %d %d 0:IDLE fSOCSum:%.1f %.1f %.1f %.1f fActvPwr_PCSGrpSum:%.1f %.1f %.1f %.1f "
            "fMax_Chrg_PwrAbilitySum:%.1f %.1f %.1f %.1f fMax_Chrg_PwrAbilitySumA:%.1f %.1f %.1f %.1f fMax_Chrg_PwrAbilitySum0:%.1f %.1f %.1f %.1f "
            "fMax_DisChrg_PwrAbilitySum:%.1f %.1f %.1f %.1f fMax_DisChrg_PwrAbilitySumA:%.1f %.1f %.1f %.1f fMax_DisChrg_PwrAbilitySum0:%.1f %.1f %.1f %.1f fChrgedEnergySum:%.1f %.1f %.1f %.1f "
            "fRealTime_MaxCurrSum:%.1f %.1f %.1f %.1f f_PgridMeterSum:%.1f %.1f %.1f %.1f f_PcabMeterSum:%.1f %.1f %.1f %.1f \n"

            "Calc:emBatt_WorkMode:%d %d %d %d fChrgedEnergy:%.1f %.1f %.1f %.1f nNodeIdx:%d %d %d %d \n"
            
            "+++fTotalPM_Chrg_PwrAbility:%.1f All:%.1f "
            "fACDC_Distri_4_BattChrg_Pwr:%.1f %.1f %.1f %.1f "
            "fMax_Chrg_PwrAbility:%.1f %.1f %.1f %.1f fMax_Chrg_PwrAbilityA:%.1f %.1f %.1f %.1f fMax_Chrg_PwrAbility0:%.1f %.1f %.1f %.1f fBatt_Max_Chrg_Pwr_Req:%.1f %.1f %.1f %.1f \n"

            "---fTotalPM_DisChrging_PwrAbility:%.1f All:%.1f "
            "fBatt_Distri_4_ACDC_Pwr:%.1f %.1f %.1f %.1f "
            "fMax_DisChrg_PwrAbility:%.1f %.1f %.1f %.1f fMax_DisChrg_PwrAbilityA:%.1f %.1f %.1f %.1f fMax_DisChrg_PwrAbility0:%.1f %.1f %.1f %.1f fBatt_Max_DisChrg_Pwr:%.1f %.1f %.1f %.1f \n"

            "***ClientSum emEMSCtrlMode_REMS:%d emACDC_WorkStatu_REMS:%d fActvPwrReq_REMS:%.1f emCCUStateSum:%d emPMCtrlModeSum:%d emACDC_WorkStatuSum:%d bStandby_BLLSum:%d byWorkAsapSum:%d "
            "nSOCMinSum:%d nSOCMaxSum:%d fSOCSum:%.1f fActvPwr_PCSGrpSum:%.1f fQ_PCSGrpSum:%.1f "
            "fMax_Chrg_PwrAbilitySum:%.1f fMax_DisChrg_PwrAbilitySum:%.1f AbilitySumA:%.1f %.1f AbilitySum0:%.1f %.1f "
            "fChrgedEnergySum:%.1f fRealTime_MaxCurrSum:%.1f fACDC_RealTime_VoltSum:%.1f fACDC_RealTime_CurrSum:%.1f "
            "f_PgridMeterSum:%.1f f_PcabMeterSum:%.1f \n"
            "Org:emBatt_WorkMode:%d nStopSOC:%u, nActvPwrLmt:%u nBattGrpPwrLmt:%u \n"
            "Now:emBatt_WorkMode:%d nStopSOC:%u, nActvPwrLmt:%u nBattGrpPwrLmt:%u",

            pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_CHRGING ? "+chrg+":(pEMS->emBatt_WorkModeReq == BATT_WORKMODE_FIRST_DISCHRG?"-dischrg-":"IDLE"),
            pEMS->nNodeNum, pEMS->emBatt_WorkModeReq, pEMS->bEnter_AutoChrg_SEMS, pEMS->bEnter_AutoDisc_SEMS,
            pCCU->emCCUState, pCCU->emPMCtrlMode, pCalc_Nodes[0].nStopSOCReq,
            
            pEMS->f_PgridMeter, pEMS->f_PcabMeter, pEMS->f_PmaxGridDemand, pEMS->fPacload4Chg, pEMS->fPacload4Dischg,
            pEMS->fGridACInputPwr_Lmt, pEMS->fGridRemainACPwr2Chrg, pEMS->fRemainPwr4Chrg, pEMS->fRemainDemand4Chrg, 
            pEMS->byWorkAsap, pEMS->bChrgAsap, pEMS->bDiscAsap, pEMS->bStandby_BLL, pEMS->bStandby_Set, pEMS->bStandby_Auto, pEMS->bStandby_AEMS,

            pB6LSYS0->emACDC_WorkStatuSum, pB6LSYS1->emACDC_WorkStatuSum, pB6LSYS2->emACDC_WorkStatuSum, pB6LSYS3->emACDC_WorkStatuSum,
            pB6LSYS0->bStandby_BLLSum, pB6LSYS1->bStandby_BLLSum, pB6LSYS2->bStandby_BLLSum, pB6LSYS3->bStandby_BLLSum,
            pB6LSYS0->byWorkAsapSum, pB6LSYS1->byWorkAsapSum, pB6LSYS2->byWorkAsapSum, pB6LSYS3->byWorkAsapSum,
            pB6LSYS0->fSOCSum, pB6LSYS1->fSOCSum, pB6LSYS2->fSOCSum, pB6LSYS3->fSOCSum,
            pB6LSYS0->fActvPwr_PCSGrpSum, pB6LSYS1->fActvPwr_PCSGrpSum, pB6LSYS2->fActvPwr_PCSGrpSum, pB6LSYS3->fActvPwr_PCSGrpSum,
            pB6LSYS0->fMax_Chrg_PwrAbilitySum, pB6LSYS1->fMax_Chrg_PwrAbilitySum, pB6LSYS2->fMax_Chrg_PwrAbilitySum, pB6LSYS3->fMax_Chrg_PwrAbilitySum,
            pB6LSYS0->fMax_Chrg_PwrAbilitySumA, pB6LSYS1->fMax_Chrg_PwrAbilitySumA, pB6LSYS2->fMax_Chrg_PwrAbilitySumA, pB6LSYS3->fMax_Chrg_PwrAbilitySumA,
            pB6LSYS0->fMax_Chrg_PwrAbilitySum0, pB6LSYS1->fMax_Chrg_PwrAbilitySum0, pB6LSYS2->fMax_Chrg_PwrAbilitySum0, pB6LSYS3->fMax_Chrg_PwrAbilitySum0,

            pB6LSYS0->fMax_DisChrg_PwrAbilitySum, pB6LSYS1->fMax_DisChrg_PwrAbilitySum, pB6LSYS2->fMax_DisChrg_PwrAbilitySum, pB6LSYS3->fMax_DisChrg_PwrAbilitySum,
            pB6LSYS0->fMax_DisChrg_PwrAbilitySumA, pB6LSYS1->fMax_DisChrg_PwrAbilitySumA, pB6LSYS2->fMax_DisChrg_PwrAbilitySumA, pB6LSYS3->fMax_DisChrg_PwrAbilitySumA,
            pB6LSYS0->fMax_DisChrg_PwrAbilitySum0, pB6LSYS1->fMax_DisChrg_PwrAbilitySum0, pB6LSYS2->fMax_DisChrg_PwrAbilitySum0, pB6LSYS3->fMax_DisChrg_PwrAbilitySum0,
            pB6LSYS0->fChrgedEnergySum, pB6LSYS1->fChrgedEnergySum, pB6LSYS2->fChrgedEnergySum, pB6LSYS3->fChrgedEnergySum,
            pB6LSYS0->fRealTime_MaxCurrSum, pB6LSYS1->fRealTime_MaxCurrSum, pB6LSYS2->fRealTime_MaxCurrSum, pB6LSYS3->fRealTime_MaxCurrSum,
            pB6LSYS0->f_PgridMeterSum, pB6LSYS1->f_PgridMeterSum, pB6LSYS2->f_PgridMeterSum, pB6LSYS3->f_PgridMeterSum,
            pB6LSYS0->f_PcabMeterSum, pB6LSYS1->f_PcabMeterSum, pB6LSYS2->f_PcabMeterSum, pB6LSYS3->f_PcabMeterSum,

            pCalc_Nodes[0].emBatt_WorkMode, pCalc_Nodes[1].emBatt_WorkMode, pCalc_Nodes[2].emBatt_WorkMode, pCalc_Nodes[3].emBatt_WorkMode,
            pCalc_Nodes[0].fChrgedEnergy, pCalc_Nodes[1].fChrgedEnergy, pCalc_Nodes[2].fChrgedEnergy, pCalc_Nodes[3].fChrgedEnergy,
            pCalc_Nodes[0].nNodeIdx, pCalc_Nodes[1].nNodeIdx, pCalc_Nodes[2].nNodeIdx, pCalc_Nodes[3].nNodeIdx,

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