//#define TRACE_2DBLOG
#include "ccu_sampler_main.h"

/**************************************************************
*Function:
*Creator:
*Time:
*ChangeLog:
*Remarks:
        struct tm {
        int tm_sec;         // 秒，范围从 0 到 59
        int tm_min;         // 分，范围从 0 到 59
        int tm_hour;        // 小时，范围从 0 到 23
        int tm_mday;        // 一月中的第几天，范围从 1 到 31
        int tm_mon;         // 月份，范围从 0 到 11
        int tm_year;        // 自 1900 起的年数
        int tm_wday;        // 一周中的第几天，范围从 0 到 6
        int tm_yday;        // 一年中的第几天，范围从 0 到 365
        int tm_isdst;       // 夏令时
        };
****************************************************************/

static void CalcACMeter_TMSectorKWH(ROUGH_CCU_DATA *pCCU, int nCurr_Hour)
{
    float fRemoteAC_PosInCreaseKWh = 0.0,   fRemoteAC_NegInCreaseKWh = 0.0;
    float fMstAC_PosInCreaseKWh = 0.0,      fMstAC_NegInCreaseKWh = 0.0;
    float fSlv1AC_PosInCreaseKWh = 0.0,     fSlv1AC_NegInCreaseKWh = 0.0;
    float fOrgACLoadIncreaseKWh = 0.0;

    if((nCurr_Hour < 0) || (nCurr_Hour > MAX_HOURS_SECTOR) )
    {
        TRACE("nCurr_Hour = %d Error nCurr_Hour, Go Back\n", nCurr_Hour);
        return;
    }

    //Step1. Refresh Remote ACMeter Realtime KWH Reading   电表H 实时电量
    if(pCCU->emsOrgCalcData.fRmtAC_RealTime_ReadKWH > 1.0)
    {
        pCCU->hisESData.es_Data.hours_Data.fAC_CurrKWH_Reading[nCurr_Hour] = pCCU->emsOrgCalcData.fRmtAC_RealTime_ReadKWH;
    }

    //Step2. Calc Remote AC meter Positive Increase KWH  电表H实时电量相减  增量
    if(pCCU->emsOrgCalcData.fRmtAC_LastkWhReading > 0.001)
    {
        fRemoteAC_PosInCreaseKWh = pCCU->emsOrgCalcData.fRmtAC_RealTime_ReadKWH - pCCU->emsOrgCalcData.fRmtAC_LastkWhReading;
        // LJH240809 修改电网侧高压表修改变比后关口表正向消耗一直为0
        //if((fRemoteAC_PosInCreaseKWh > 0.0001) && (fRemoteAC_PosInCreaseKWh < 299.0))
        if(fRemoteAC_PosInCreaseKWh > 0.0001)
        {
            pCCU->hisESData.es_Data.hours_Data.fAC_CostKWH_Sector[nCurr_Hour] += fRemoteAC_PosInCreaseKWh;
        }
        else
        {
            fRemoteAC_PosInCreaseKWh = 0.0;
        } 
    }
    pCCU->emsOrgCalcData.fRmtAC_LastkWhReading = pCCU->emsOrgCalcData.fRmtAC_RealTime_ReadKWH;
    
    //Step3. Calc Remote AC Meter Negtive Increase KWH  电表H 反向有功总电能相减  增量
    if(pCCU->emsOrgCalcData.fRmtAC_Last_Revs_ReadKWH > 0.001)
    {
        fRemoteAC_NegInCreaseKWh = pCCU->emsOrgCalcData.fRmtAC_Revs_ReadKWH - pCCU->emsOrgCalcData.fRmtAC_Last_Revs_ReadKWH;
        // if((fRemoteAC_NegInCreaseKWh > 0.0001) && (fRemoteAC_NegInCreaseKWh < 10.0))
        if(fRemoteAC_NegInCreaseKWh > 0.0001)
        {
            pCCU->hisESData.es_Data.hours_Data.fAC_RevrtOutPutKWH_Sector[nCurr_Hour] += fRemoteAC_NegInCreaseKWh;    
        }
        else
        {
            fRemoteAC_NegInCreaseKWh = 0.0;
        } 
    }
    pCCU->emsOrgCalcData.fRmtAC_Last_Revs_ReadKWH = pCCU->emsOrgCalcData.fRmtAC_Revs_ReadKWH;

    //Step6.Calc Slave Local AC Meter Value  主从模式下，计算从-柜内表正向消耗电量  增量
    if(pCCU->emsOrgCalcData.fLast_Slv1_LocalAC_RTKWH_Reading > 0.001)  
    {
        fSlv1AC_PosInCreaseKWh = pCCU->emsOrgCalcData.fSlv1_LocalAC_RTKWH_Reading - pCCU->emsOrgCalcData.fLast_Slv1_LocalAC_RTKWH_Reading;
        // if((fSlv1AC_PosInCreaseKWh > 0.0001) && (fSlv1AC_PosInCreaseKWh < 10.0))
        if(fSlv1AC_PosInCreaseKWh > 0.0001)
        {
            pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[nCurr_Hour] += fSlv1AC_PosInCreaseKWh;    
        }
        else
        {
            fSlv1AC_PosInCreaseKWh = 0.0;
        }   
    }
    pCCU->emsOrgCalcData.fLast_Slv1_LocalAC_RTKWH_Reading = pCCU->emsOrgCalcData.fSlv1_LocalAC_RTKWH_Reading;

    //Step7.主从模式下，计算从-柜内表反向消耗电量  增量
    if(pCCU->emsOrgCalcData.fLast_Slv1_LocalAC_RevsKWH > 0.001)
    {
        fSlv1AC_NegInCreaseKWh = pCCU->emsOrgCalcData.fSlv1_LocalAC_RevsKWH - pCCU->emsOrgCalcData.fLast_Slv1_LocalAC_RevsKWH;
        // if((fSlv1AC_NegInCreaseKWh > 0.0001) && (fSlv1AC_NegInCreaseKWh < 10.0))
        if(fSlv1AC_NegInCreaseKWh > 0.0001)
        {
            pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[nCurr_Hour] += fSlv1AC_NegInCreaseKWh;    
        }
        else
        {
            fSlv1AC_NegInCreaseKWh = 0.0;
        } 
    }
    pCCU->emsOrgCalcData.fLast_Slv1_LocalAC_RevsKWH = pCCU->emsOrgCalcData.fSlv1_LocalAC_RevsKWH;

    //Step4. Calc Master Local AC Meter Value  主从模式下，计算主-柜内表正向消耗电量  增量
    if(pCCU->emsOrgCalcData.fLast_Master_LocalAC_RTKWH_Reading > 0.001)
    {
        fMstAC_PosInCreaseKWh = pCCU->emsOrgCalcData.fMaster_LocalAC_RTKWH_Reading - pCCU->emsOrgCalcData.fLast_Master_LocalAC_RTKWH_Reading;
        // if((fMstAC_PosInCreaseKWh > 0.0001) && (fMstAC_PosInCreaseKWh < 10.0))
        if(fMstAC_PosInCreaseKWh > 0.0001)
        {
            pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[nCurr_Hour] += fMstAC_PosInCreaseKWh;    
        }
        else
        {
            fMstAC_PosInCreaseKWh = 0.0;
        } 
    }
    pCCU->emsOrgCalcData.fLast_Master_LocalAC_RTKWH_Reading = pCCU->emsOrgCalcData.fMaster_LocalAC_RTKWH_Reading;

    //Step5.calc Master Local AC Revs KWH  主从模式下，计算主-柜内表反向消耗电量  增量
    if(pCCU->emsOrgCalcData.fLast_Master_LocalAC_RevsKWH > 0.001)
    {
        fMstAC_NegInCreaseKWh = pCCU->emsOrgCalcData.fMaster_LocalAC_RevsKWH - pCCU->emsOrgCalcData.fLast_Master_LocalAC_RevsKWH;
        // if((fMstAC_NegInCreaseKWh > 0.0001) && (fMstAC_NegInCreaseKWh < 10.0))
        if(fMstAC_NegInCreaseKWh > 0.0001)
        {
            pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[nCurr_Hour] += fMstAC_NegInCreaseKWh;    
        }
        else
        {
            fMstAC_NegInCreaseKWh = 0.0;
        }
    }
    pCCU->emsOrgCalcData.fLast_Master_LocalAC_RevsKWH = pCCU->emsOrgCalcData.fMaster_LocalAC_RevsKWH;

    //Step8: Calc Remote ACLoad Org Data-should Consider Slave Cabinet Comm Fail Statu

    /*
        charge:     fRemoteAC_PosInCreaseKWh = Pacload + fMstAC_PosInCreaseKWh + fSlv1AC_PosInCreaseKWh;  
        discharge:  fMstAC_NegInCreaseKWh + fSlv1AC_NegInCreaseKWh = Pacload + fRemoteAC_NegInCreaseKWh;  
    */  
    fOrgACLoadIncreaseKWh = fRemoteAC_PosInCreaseKWh - fMstAC_PosInCreaseKWh - fSlv1AC_PosInCreaseKWh + fMstAC_NegInCreaseKWh + fSlv1AC_NegInCreaseKWh - fRemoteAC_NegInCreaseKWh;
    //2020-01-08 REmote AC and Local AC Sample Speed is diff ,cause the increase value become negative
    //fOrgACLoadIncreaseKWh = MAX(fOrgACLoadIncreaseKWh, 0.0);
    // if((fOrgACLoadIncreaseKWh > -10.0) && (fOrgACLoadIncreaseKWh < 10.0))
    if(fOrgACLoadIncreaseKWh > -10.0)
    {
        pCCU->hisESData.es_Data.hours_Data.fOrgACLoad_CostKWH_Sector[nCurr_Hour] += fOrgACLoadIncreaseKWh;
    }

    TRACE("fOrgACLoadIncreaseKWh=%.1f,fRemoteAC_PosInCreaseKWh=%.1f, fMstAC_PosInCreaseKWh=%.1f fSlv1AC_PosInCreaseKWh=%.1f,\
        fMstAC_NegInCreaseKWh=%.1f, fSlv1AC_NegInCreaseKWh=%.1f\n",
        fOrgACLoadIncreaseKWh,fRemoteAC_PosInCreaseKWh , fMstAC_PosInCreaseKWh,fSlv1AC_PosInCreaseKWh,\
        fMstAC_NegInCreaseKWh , fSlv1AC_NegInCreaseKWh);

    /*int n = 0;
    for(n= 0; n < MAX_HOURS_SECTOR; n++)
    {
        TRACE("hours_Data.fAC_CostKWH_Sector[%d]=%f fOrgACLoad_CostKWH_Sector=%f\n",\
            n, pCCU->hisESData.es_Data.hours_Data.fAC_CostKWH_Sector[n],
            pCCU->hisESData.es_Data.hours_Data.fOrgACLoad_CostKWH_Sector[n]);
    }*/
    return;
}

static void CalcDCLoad_TMSectorKWH(ROUGH_CCU_DATA *pCCU, int nCurr_Hour)
{
    float fDCLoad_InCreasekWh = 0.0;
    //DC Load 24 Hour Sector KWH, Process it Later
    if(pCCU->emsOrgCalcData.fLast_DCLoad_TotalReadKWH > 0.001)
    {
        fDCLoad_InCreasekWh = pCCU->emsOrgCalcData.fDCLoad_TotalReadKWH - pCCU->emsOrgCalcData.fLast_DCLoad_TotalReadKWH;
        if((fDCLoad_InCreasekWh > 0.0001) && (fDCLoad_InCreasekWh < 10.0))
        {
            pCCU->hisESData.es_Data.hours_Data.fDCLoad_CostKWH_Sector[nCurr_Hour] += fDCLoad_InCreasekWh;
        }
    }
    pCCU->emsOrgCalcData.fLast_DCLoad_TotalReadKWH = pCCU->emsOrgCalcData.fDCLoad_TotalReadKWH;
    
    return;
}

static void CalcMPPT_TMSectorKWH(ROUGH_CCU_DATA *pCCU, int nCurr_Hour)
{
    float   fMPPT_InCreasekWh = 0.0;
    int     nGrp = 0;

    if((nCurr_Hour < 0) || (nCurr_Hour > MAX_HOURS_SECTOR) )
    {
        TRACE("nCurr_Hour = %d Error nCurr_Hour, Go Back\n", nCurr_Hour);
        return;
    }

    for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
        if(pCCU->emsOrgCalcData.fMPPT_LastkWhReading[nGrp] > 0.001)
        {
            fMPPT_InCreasekWh = pCCU->emsOrgCalcData.fMPPT_RealTime_ReadKWH[nGrp] - pCCU->emsOrgCalcData.fMPPT_LastkWhReading[nGrp];
            if((fMPPT_InCreasekWh > 0.0001) && (fMPPT_InCreasekWh < 10.0))
            {
                pCCU->hisESData.es_Data.hours_Data.fMppt_GKWH_Sector[nGrp][nCurr_Hour] += fMPPT_InCreasekWh;
            }

        }
        pCCU->emsOrgCalcData.fMPPT_LastkWhReading[nGrp] = pCCU->emsOrgCalcData.fMPPT_RealTime_ReadKWH[nGrp];
    }
    
    /*int n = 0;
    for(n = 0; n < MAX_HOURS_SECTOR; n++)
    {
        TRACE("hours_Data.fMppt_GKWH_Sector[0][%d]=%f | hours_Data.fMppt_GKWH_Sector[1][%d]=%f\n",\
            n, pCCU->hisESData.es_Data.hours_Data.fMppt_GKWH_Sector[0][n],\
            n, pCCU->hisESData.es_Data.hours_Data.fMppt_GKWH_Sector[1][n]);
    }*/
    return;
}

static void CalcBatt_TMSectorKWH(ROUGH_CCU_DATA *pCCU, int nCurr_Hour)
{
    float   fBattChrg_InCreaseKWh = 0.0, fBattDisChrg_InCreaseKWh = 0.0;
    int     nGrp = 0;

    if((nCurr_Hour < 0) || (nCurr_Hour > MAX_HOURS_SECTOR) )
    {
        TRACE("nCurr_Hour = %d Error nCurr_Hour, Go Back\n", nCurr_Hour);
        return;
    }

    for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
        TRACE("nGrp=[%d] fBattChrg_RealTime_KWH=[%.1f] fBattChrg_LastkWhReading=[%.1f]\n",\
            nGrp,pCCU->emsOrgCalcData.fBattChrg_RealTime_KWH[nGrp],\
            pCCU->emsOrgCalcData.fBattChrg_LastkWhReading[nGrp]);
        //1. Calc Battery Charging KWH
        if(pCCU->emsOrgCalcData.fBattChrg_RealTime_KWH[nGrp] > 0.001)
        {
            fBattChrg_InCreaseKWh = pCCU->emsOrgCalcData.fBattChrg_RealTime_KWH[nGrp] - pCCU->emsOrgCalcData.fBattChrg_LastkWhReading[nGrp];
            if((fBattChrg_InCreaseKWh > 0.0001) && (fBattChrg_InCreaseKWh < 10.0))
            {
                pCCU->hisESData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[nGrp][nCurr_Hour] += fBattChrg_InCreaseKWh;
            }
        }
        pCCU->emsOrgCalcData.fBattChrg_LastkWhReading[nGrp] = pCCU->emsOrgCalcData.fBattChrg_RealTime_KWH[nGrp];

        //2. Calc Battery DisCharging KWH
        if(pCCU->emsOrgCalcData.fBattDisChrg_RealTime_KWH[nGrp] > 0.001)
        {
            fBattDisChrg_InCreaseKWh = pCCU->emsOrgCalcData.fBattDisChrg_RealTime_KWH[nGrp] - pCCU->emsOrgCalcData.fBattDisChrg_LastkWhReading[nGrp];
            if((fBattDisChrg_InCreaseKWh > 0.0001) && (fBattDisChrg_InCreaseKWh < 10.0))
            {
                pCCU->hisESData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[nGrp][nCurr_Hour] += fBattDisChrg_InCreaseKWh;
            }
        }
        pCCU->emsOrgCalcData.fBattDisChrg_LastkWhReading[nGrp] = pCCU->emsOrgCalcData.fBattDisChrg_RealTime_KWH[nGrp];
    }

    TRACE("\n\thours_Data.fBatt_ChrgKWH_Sector[0][%d]=%f | hours_Data.fBatt_DisChrgKWH_Sector[0][%d]=%f \
        \n\thours_Data.fBatt_ChrgKWH_Sector[1][%d]=%f | hours_Data.fBatt_DisChrgKWH_Sector[1][%d]=%f\n",
        nCurr_Hour, pCCU->hisESData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[0][nCurr_Hour],\
        nCurr_Hour, pCCU->hisESData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[0][nCurr_Hour],\
        nCurr_Hour, pCCU->hisESData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[1][nCurr_Hour],\
        nCurr_Hour, pCCU->hisESData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[1][nCurr_Hour]);
    return;
}

static void CalcOff_GridLoad_TMSectorKWH(ROUGH_CCU_DATA *pCCU, int nCurr_Hour)
{
    float fRemoteLoad_PosInCreaseKWh = 0.0, fRemoteLoad_NegInCreaseKWh = 0.0;
    if((nCurr_Hour < 0) || (nCurr_Hour > MAX_HOURS_SECTOR) )
    {
        TRACE("nCurr_Hour = %d Error nCurr_Hour, Go Back\n", nCurr_Hour);
        return;
    }
    if(pCCU->emsOrgCalcData.fRmtLoad_LastkWhReading > 0.001)
    {
        fRemoteLoad_PosInCreaseKWh = pCCU->emsOrgCalcData.fRmtLoad_RealTime_ReadKWH - pCCU->emsOrgCalcData.fRmtLoad_LastkWhReading;
        if((fRemoteLoad_PosInCreaseKWh > 0.001) && (fRemoteLoad_PosInCreaseKWh < 10))
        {
            pCCU->hisESData.es_Data.hours_Data.fOffGridLoad_CostKWH_Sector[nCurr_Hour] += fRemoteLoad_PosInCreaseKWh;
        }
    }
    pCCU->emsOrgCalcData.fRmtLoad_LastkWhReading = pCCU->emsOrgCalcData.fRmtLoad_RealTime_ReadKWH;

    if(pCCU->emsOrgCalcData.fRmtLoad_Last_Revs_ReadKWH > 0.001)
    {
        fRemoteLoad_NegInCreaseKWh = pCCU->emsOrgCalcData.fRmtLoad_Revs_ReadKWH - pCCU->emsOrgCalcData.fRmtLoad_Last_Revs_ReadKWH;
        if((fRemoteLoad_NegInCreaseKWh > 0.001) && (fRemoteLoad_NegInCreaseKWh < 10))
        {
            pCCU->hisESData.es_Data.hours_Data.fOffGridLoad_CostKWH_Sector[nCurr_Hour] -= fRemoteLoad_NegInCreaseKWh;
        }
    }
    pCCU->emsOrgCalcData.fRmtLoad_Last_Revs_ReadKWH = pCCU->emsOrgCalcData.fRmtLoad_Revs_ReadKWH;

    
    return;
}

void Init_CurrDayKWH(ROUGH_CCU_DATA *pCCU, int nDayIdx)
{
    int nGrp = 0;
    if((nDayIdx < 0) || (nDayIdx > MAX_DAYS_SECTOR) )
    {
        TRACE("nDayIdx = %d Error DayIdx, Go Back\n", nDayIdx);
        return;
    }
    
    pCCU->hisESData.es_Data.days_Data.tmTimeStamp_D[nDayIdx] = _NOW_;

    pCCU->hisESData.es_Data.days_Data.fAC_CurrKWH_Reading[nDayIdx] = pCCU->emsOrgCalcData.fRmtAC_RealTime_ReadKWH;

    pCCU->hisESData.es_Data.days_Data.fACTotalCostKWH_D[nDayIdx] = 0.0;

     //365 Days Org AC Load Cost KWH
    pCCU->hisESData.es_Data.days_Data.fOrgACLoad_CostKWH_D[nDayIdx] = 0.0;

	//365 Days Remote AC Reverse Output KWH
	pCCU->hisESData.es_Data.days_Data.fAC_RevrtOutPutKWH_D[nDayIdx] = 0.0;

	//365 Days Local AC Meter Input KWH 
	pCCU->hisESData.es_Data.days_Data.fLocalAC_INPUTKWH_D[nDayIdx] = 0.0;

	//365 Days Local AC Meter Output KWH 
	pCCU->hisESData.es_Data.days_Data.fLocalAC_OUTPUTKWH_D[nDayIdx] = 0.0;

	//365 Days DC Load Cost KWH
	pCCU->hisESData.es_Data.days_Data.fDCLoad_CostKWH_D[nDayIdx] = 0.0;

    for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
        pCCU->hisESData.es_Data.days_Data.fMpptTotalGKWH_D[nGrp][nDayIdx] = 0.0;
    }

    for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
        pCCU->hisESData.es_Data.days_Data.fBattTotalChrgKWH_D[nGrp][nDayIdx] = 0.0;
        pCCU->hisESData.es_Data.days_Data.fBattTotalDisChrgKWH_D[nGrp][nDayIdx] = 0.0;
    
        pCCU->hisESData.es_Data.days_Data.fBattChrgTimes_D[nGrp][nDayIdx] = 0.0;
        pCCU->hisESData.es_Data.days_Data.fBattDisChrgTimes_D[nGrp][nDayIdx] = 0.0;
    } 

    //Sharp KWH
	pCCU->hisESData.es_Data.days_Data.fLocalAC_Sharp_InputKWH_D[nDayIdx]  = 0.0;
	pCCU->hisESData.es_Data.days_Data.fLocalAC_Sharp_OutPutKWH_D[nDayIdx] = 0.0;

    pCCU->hisESData.es_Data.days_Data.fOrgACLoad_Sharp_CostKWH_D[nDayIdx] = 0.0;
    
    for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
        pCCU->hisESData.es_Data.days_Data.fMppt_Sharp_GKWH_D[nGrp][nDayIdx] = 0.0;
    }

    //Peak KWH
    pCCU->hisESData.es_Data.days_Data.fLocalAC_Peak_InputKWH_D[nDayIdx]  = 0.0;
	pCCU->hisESData.es_Data.days_Data.fLocalAC_Peak_OutPutKWH_D[nDayIdx] = 0.0;
    pCCU->hisESData.es_Data.days_Data.fOrgACLoad_Peak_CostKWH_D[nDayIdx] = 0.0;
    
    for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
        pCCU->hisESData.es_Data.days_Data.fMppt_Peak_GKWH_D[nGrp][nDayIdx] = 0.0;   
    }
   
    //Flat KWH
	pCCU->hisESData.es_Data.days_Data.fLocalAC_Flat_InputKWH_D[nDayIdx] = 0.0;
	pCCU->hisESData.es_Data.days_Data.fLocalAC_Flat_OutputKWH_D[nDayIdx] = 0.0;
    pCCU->hisESData.es_Data.days_Data.fOrgACLoad_Flat_CostKWH_D[nDayIdx] = 0.0;
    
    for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
        pCCU->hisESData.es_Data.days_Data.fMppt_Flat_GKWH_D[nGrp][nDayIdx] = 0.0;    
    }

    //Valley KWH
    pCCU->hisESData.es_Data.days_Data.fLocalAC_Valley_InputKWH_D[nDayIdx] = 0.0;
	pCCU->hisESData.es_Data.days_Data.fLocalAC_Valley_OutputKWH_D[nDayIdx] = 0.0;
    pCCU->hisESData.es_Data.days_Data.fOrgACLoad_Valley_CostKWH_D[nDayIdx] = 0.0;

    for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
        pCCU->hisESData.es_Data.days_Data.fMppt_Valley_GKWH_D[nGrp][nDayIdx] = 0.0;   
    }
 
    return;
}

void AddHourSector4DaysKWH(ROUGH_CCU_DATA *pCCU, int nDayIdx, int nLast_Hour, FEERATE_ATTRI emLast_FeeRate)
{
    int nGrp = 0;
    if((nDayIdx < 0) || (nDayIdx > MAX_DAYS_SECTOR) )
    {
        TRACE("nDayIdx = %d Error DayIdx, Go Back\n", nDayIdx);
        return;
    }

    pCCU->hisESData.es_Data.days_Data.tmTimeStamp_D[nDayIdx] = _NOW_ - 60; //should -60S, otherwise will log the next day timestamp

    if(pCCU->emsOrgCalcData.fRmtAC_RealTime_ReadKWH > 1.0)//This should Be a Valid Value
    {
        pCCU->hisESData.es_Data.days_Data.fAC_CurrKWH_Reading[nDayIdx] = pCCU->emsOrgCalcData.fRmtAC_RealTime_ReadKWH;
    }

    pCCU->hisESData.es_Data.days_Data.fACTotalCostKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fAC_CostKWH_Sector[nLast_Hour];
    
    //365 Days Org AC Load Cost KWH
    pCCU->hisESData.es_Data.days_Data.fOrgACLoad_CostKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fOrgACLoad_CostKWH_Sector[nLast_Hour];

	//365 Days Remote AC Reverse Output KWH
	pCCU->hisESData.es_Data.days_Data.fAC_RevrtOutPutKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fAC_RevrtOutPutKWH_Sector[nLast_Hour];

	//365 Days Local AC Meter Input KWH 
	pCCU->hisESData.es_Data.days_Data.fLocalAC_INPUTKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[nLast_Hour];

	//365 Days Local AC Meter Output KWH 
	pCCU->hisESData.es_Data.days_Data.fLocalAC_OUTPUTKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[nLast_Hour];

	//365 Days DC Load Cost KWH
	pCCU->hisESData.es_Data.days_Data.fDCLoad_CostKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fDCLoad_CostKWH_Sector[nLast_Hour];

    //365 Days Off-Grid Load Meter KWH
    pCCU->hisESData.es_Data.days_Data.fOffGridLoad_CostKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fOffGridLoad_CostKWH_Sector[nLast_Hour];

    for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
         pCCU->hisESData.es_Data.days_Data.fMpptTotalGKWH_D[nGrp][nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fMppt_GKWH_Sector[nGrp][nLast_Hour];
    }

    for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
        pCCU->hisESData.es_Data.days_Data.fBattTotalChrgKWH_D[nGrp][nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[nGrp][nLast_Hour];
        pCCU->hisESData.es_Data.days_Data.fBattTotalDisChrgKWH_D[nGrp][nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[nGrp][nLast_Hour];
    
        if((pCCU->emsOrgCalcData.fBatt_Rate_KWH[nGrp] > 0) && (pCCU->emsOrgCalcData.fBatt_Rate_KWH[nGrp] < 9999.0))
        {
            pCCU->hisESData.es_Data.days_Data.fBattChrgTimes_D[nGrp][nDayIdx] = pCCU->hisESData.es_Data.days_Data.fBattTotalChrgKWH_D[nGrp][nDayIdx]/pCCU->emsOrgCalcData.fBatt_Rate_KWH[nGrp];
            pCCU->hisESData.es_Data.days_Data.fBattDisChrgTimes_D[nGrp][nDayIdx] = pCCU->hisESData.es_Data.days_Data.fBattTotalDisChrgKWH_D[nGrp][nDayIdx]/pCCU->emsOrgCalcData.fBatt_Rate_KWH[nGrp];
        }
    }
    
    if(emLast_FeeRate == FEERATE_SHARP)
    {
	    pCCU->hisESData.es_Data.days_Data.fLocalAC_Sharp_InputKWH_D[nDayIdx]  += pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[nLast_Hour];
		pCCU->hisESData.es_Data.days_Data.fLocalAC_Sharp_OutPutKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[nLast_Hour];

        pCCU->hisESData.es_Data.days_Data.fOrgACLoad_Sharp_CostKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fOrgACLoad_CostKWH_Sector[nLast_Hour];
        
        for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
        {
            pCCU->hisESData.es_Data.days_Data.fMppt_Sharp_GKWH_D[nGrp][nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fMppt_GKWH_Sector[nGrp][nLast_Hour];

        }
    }
    else if(emLast_FeeRate == FEERATE_PEAK)
    {
    	pCCU->hisESData.es_Data.days_Data.fLocalAC_Peak_InputKWH_D[nDayIdx]  += pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[nLast_Hour];
		pCCU->hisESData.es_Data.days_Data.fLocalAC_Peak_OutPutKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[nLast_Hour];
        
        pCCU->hisESData.es_Data.days_Data.fOrgACLoad_Peak_CostKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fOrgACLoad_CostKWH_Sector[nLast_Hour];
        
        for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
        {
            pCCU->hisESData.es_Data.days_Data.fMppt_Peak_GKWH_D[nGrp][nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fMppt_GKWH_Sector[nGrp][nLast_Hour];
            
        }
    }    
    else if(emLast_FeeRate == FEERATE_FLAT)
    {
		pCCU->hisESData.es_Data.days_Data.fLocalAC_Flat_InputKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[nLast_Hour];
		pCCU->hisESData.es_Data.days_Data.fLocalAC_Flat_OutputKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[nLast_Hour];

        pCCU->hisESData.es_Data.days_Data.fOrgACLoad_Flat_CostKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fOrgACLoad_CostKWH_Sector[nLast_Hour];
        
        for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
        {
            pCCU->hisESData.es_Data.days_Data.fMppt_Flat_GKWH_D[nGrp][nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fMppt_GKWH_Sector[nGrp][nLast_Hour];
            
        }
    }
    else if(emLast_FeeRate == FEERATE_VALLEY)
    {
        pCCU->hisESData.es_Data.days_Data.fLocalAC_Valley_InputKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[nLast_Hour];
		pCCU->hisESData.es_Data.days_Data.fLocalAC_Valley_OutputKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[nLast_Hour];

        pCCU->hisESData.es_Data.days_Data.fOrgACLoad_Valley_CostKWH_D[nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fOrgACLoad_CostKWH_Sector[nLast_Hour];
        
        for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
        {
            pCCU->hisESData.es_Data.days_Data.fMppt_Valley_GKWH_D[nGrp][nDayIdx] += pCCU->hisESData.es_Data.hours_Data.fMppt_GKWH_Sector[nGrp][nLast_Hour];
            
        }
    }

    TRACE("days_Data.fACTotalCostKWH_D[%d].fValue=%f\
    \n\tdays_Data.fMpptTotalGKWH_D[0][%d].fValue=%f | days_Data.fMpptTotalGKWH_D[1][%d].fValue=%f\
    \n\tdays_Data.fBattTotalChrgKWH_D[0][%d].fValue=%f | days_Data.fBattTotalChrgKWH_D[1][%d].fValue=%f\
    \n\tdays_Data.fBattTotalDisChrgKWH_D[0][%d].fValue=%f | days_Data.fBattTotalDisChrgKWH_D[1][%d].fValue=%f \n", \
    nDayIdx, pCCU->hisESData.es_Data.days_Data.fACTotalCostKWH_D[nDayIdx],\
    nDayIdx, pCCU->hisESData.es_Data.days_Data.fMpptTotalGKWH_D[0][nDayIdx],\
    nDayIdx, pCCU->hisESData.es_Data.days_Data.fMpptTotalGKWH_D[1][nDayIdx],\
    nDayIdx, pCCU->hisESData.es_Data.days_Data.fBattTotalChrgKWH_D[0][nDayIdx],\
    nDayIdx, pCCU->hisESData.es_Data.days_Data.fBattTotalChrgKWH_D[1][nDayIdx],\
    nDayIdx, pCCU->hisESData.es_Data.days_Data.fBattTotalDisChrgKWH_D[0][nDayIdx],\
    nDayIdx, pCCU->hisESData.es_Data.days_Data.fBattTotalDisChrgKWH_D[1][nDayIdx]);

    return;
}


// void RefreshWeekly24HSectorKWH(ROUGH_CCU_DATA *pCCU, int nCurr_Wday, int nCurr_Hour)
// {
//     int nGrp = 0;
//     if((nCurr_Wday < 0 ) || (nCurr_Wday > MAX_WEEKLY_SECTOR))
//     {
//         return;
//     }
// 	if((nCurr_Hour < 0) || (nCurr_Hour > MAX_HOURS_SECTOR) )
//     {
//         TRACE("nCurr_Hour = %d Error nCurr_Hour, Go Back\n", nCurr_Hour);
//         return;
//     }

//     pCCU->hisESData.es_Data.week_Data.tmTimeStamp_W[nCurr_Wday][nCurr_Hour] = _NOW_;

//     //Calc RealTime Ac Meter Reading
//     if(pCCU->emsOrgCalcData.fRmtAC_RealTime_ReadKWH > 1.0)//if >1.0 as a Valid Value
//     {
//         pCCU->hisESData.es_Data.week_Data.fACRealTimeKWH_W[nCurr_Wday][nCurr_Hour] = pCCU->emsOrgCalcData.fRmtAC_RealTime_ReadKWH;
//     }
//     pCCU->hisESData.es_Data.week_Data.fACTotalCostKWH_W[nCurr_Wday][nCurr_Hour] = pCCU->hisESData.es_Data.hours_Data.fAC_CostKWH_Sector[nCurr_Hour];
 
//     //Weekly Org AC Load Cost KWH
//     pCCU->hisESData.es_Data.week_Data.fOrgACLoad_CostKWH_W[nCurr_Wday][nCurr_Hour] = pCCU->hisESData.es_Data.hours_Data.fOrgACLoad_CostKWH_Sector[nCurr_Hour];

// 	//Weekly Remote AC Reverse Output KWH
// 	pCCU->hisESData.es_Data.week_Data.fAC_RevrtOutPutKWH_W[nCurr_Wday][nCurr_Hour] = pCCU->hisESData.es_Data.hours_Data.fAC_RevrtOutPutKWH_Sector[nCurr_Hour];

// 	//Weekly Local AC Meter Input KWH 
// 	pCCU->hisESData.es_Data.week_Data.fLocalAC_INPUTKWH_W[nCurr_Wday][nCurr_Hour] = pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[nCurr_Hour];

// 	//Weekly Local AC Meter Output KWH 
// 	pCCU->hisESData.es_Data.week_Data.fLocalAC_OUTPUTKWH_W[nCurr_Wday][nCurr_Hour] = pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[nCurr_Hour];

// 	//Weekly DC Load Cost KWH
// 	pCCU->hisESData.es_Data.week_Data.fDCLoad_CostKWH_W[nCurr_Wday][nCurr_Hour] = pCCU->hisESData.es_Data.hours_Data.fDCLoad_CostKWH_Sector[nCurr_Hour];


//     for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
//     {
//         pCCU->hisESData.es_Data.week_Data.fMpptTotalGKWH_W[nGrp][nCurr_Wday][nCurr_Hour] = pCCU->hisESData.es_Data.hours_Data.fMppt_GKWH_Sector[nGrp][nCurr_Hour];
//     }

//     for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
//     {
//         pCCU->hisESData.es_Data.week_Data.fBattTotalChrgKWH_W[nGrp][nCurr_Wday][nCurr_Hour] = pCCU->hisESData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[nGrp][nCurr_Hour];

//         pCCU->hisESData.es_Data.week_Data.fBattTotalDisChrgKWH_W[nGrp][nCurr_Wday][nCurr_Hour] = pCCU->hisESData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[nGrp][nCurr_Hour];
//     }

//     TRACE("week_Data.fACTotalCostKWH_W[%d][%d].fValue=%f\
//     \n\tweek_Data.fMpptTotalGKWH_W[0][%d][%d].fValue=%f | week_Data.fMpptTotalGKWH_W[1][%d][%d].fValue=%f\
//     \n\tweek_Data.fBattTotalChrgKWH_W[0][%d][%d].fValue=%f | week_Data.fBattTotalChrgKWH_W[1][%d][%d].fValue=%f\
//     \n\tweek_Data.fBattTotalDisChrgKWH_W[0][%d][%d].fValue=%f | week_Data.fBattTotalDisChrgKWH_W[1][%d][%d].fValue=%f\n", \
//     nCurr_Wday, nCurr_Hour, pCCU->hisESData.es_Data.week_Data.fACTotalCostKWH_W[nCurr_Wday][nCurr_Hour],\
//     nCurr_Wday, nCurr_Hour, pCCU->hisESData.es_Data.week_Data.fMpptTotalGKWH_W[0][nCurr_Wday][nCurr_Hour],\
//     nCurr_Wday, nCurr_Hour, pCCU->hisESData.es_Data.week_Data.fMpptTotalGKWH_W[1][nCurr_Wday][nCurr_Hour],\
//     nCurr_Wday, nCurr_Hour, pCCU->hisESData.es_Data.week_Data.fBattTotalChrgKWH_W[0][nCurr_Wday][nCurr_Hour],\
//     nCurr_Wday, nCurr_Hour, pCCU->hisESData.es_Data.week_Data.fBattTotalChrgKWH_W[1][nCurr_Wday][nCurr_Hour],\
//     nCurr_Wday, nCurr_Hour, pCCU->hisESData.es_Data.week_Data.fBattTotalDisChrgKWH_W[0][nCurr_Wday][nCurr_Hour] ,\
//     nCurr_Wday, nCurr_Hour, pCCU->hisESData.es_Data.week_Data.fBattTotalDisChrgKWH_W[1][nCurr_Wday][nCurr_Hour]);

//     return;
// }


void RefreshDailySectorTemp(ROUGH_CCU_DATA *pCCU, int nCurr_Hour)
{
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

    int nGrp = 0;
    VAR_VALUE varV;
    ERRCODE_DAI err;

	if((nCurr_Hour < 0) || (nCurr_Hour > MAX_HOURS_SECTOR) )
    {
        TRACE("nCurr_Hour = %d Error nCurr_Hour, Go Back\n", nCurr_Hour);
        return;
    }

    for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
        //Daily Temp Data
        pCCU->hisESData.es_Data.hours_Data.fBattMaxTemp[nGrp][nCurr_Hour] = pCCU->blobSingleBattData[nGrp].fMax_CellTemp;

        pCCU->hisESData.es_Data.hours_Data.fBattMinTemp[nGrp][nCurr_Hour] = pCCU->blobSingleBattData[nGrp].fMin_CellTemp;

        pCCU->hisESData.es_Data.hours_Data.fBattAvgTemp[nGrp][nCurr_Hour] = pCCU->blobSingleBattData[nGrp].fAvg_CellTemp;

        err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 951 + nGrp, 1541, &varV, 4);
        if(err == ERR_DPR_OK)
        {
            pCCU->hisESData.es_Data.hours_Data.fBattInWaterTemp[nGrp][nCurr_Hour] = varV.fValue;
        }
        else
        {
            pCCU->hisESData.es_Data.hours_Data.fBattInWaterTemp[nGrp][nCurr_Hour] = 0.0;
        }

        err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 951 + nGrp, 1542, &varV, 4);
        if(err == ERR_DPR_OK)
        {
            pCCU->hisESData.es_Data.hours_Data.fBattOutWaterTemp[nGrp][nCurr_Hour] = varV.fValue;
        }
        else
        {
            pCCU->hisESData.es_Data.hours_Data.fBattOutWaterTemp[nGrp][nCurr_Hour] = 0.0;
        };
    }

    return;
}


void RefreshDailyEfficiencyData(ROUGH_CCU_DATA *pCCU, int nCurr_Hour, int nCurr_MDay)
{
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

    VAR_VALUE varV;
    ERRCODE_DAI err;

    EFFICIENCY_DATA_DAILY efficiencyData;
    char szStringTime[32] = "\0";

    struct tm gmTimeLast;
    struct tm gmTimeNow;

    if((nCurr_MDay < 0) || (nCurr_MDay > MAX_DAYS_SECTOR) )
    {
        TRACE("nCurr_MDay = %d Error nCurr_MDay, Go Back\n", nCurr_MDay);
        return;
    }

    if((nCurr_Hour < 0) || (nCurr_Hour > MAX_HOURS_SECTOR) )
    {
        TRACE("nCurr_Hour = %d Error nCurr_Hour, Go Back\n", nCurr_Hour);
        return;
    }

    if(nCurr_Hour != pCCU->nLast_Hour || nCurr_MDay != pCCU->nLast_MDay)
    {
        err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 200, 1897, &varV, 4);
        
        if(err == ERR_DPR_OK)
        {
            if(varV.ulValue == 0)
            {
                efficiencyData.tmTimeSaved = _NOW_;
            }
            else
            {
                efficiencyData.tmTimeSaved = varV.ulValue;
            } 
        }
        else
        {
            efficiencyData.tmTimeSaved = -1;
        }
        strcpy(szStringTime, "");

        err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 200, 1898, &varV, 4);
        if(err == ERR_DPR_OK)
        {
            efficiencyData.fTotalACInputKWH_Daily = varV.fValue;
        }
        else
        {
            efficiencyData.fTotalACInputKWH_Daily = 0.0;
        }

        err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 200, 1899, &varV, 4);
        if(err == ERR_DPR_OK)
        {
            efficiencyData.fTotalACOutputKWH_Daily = varV.fValue;
        }
        else
        {
            efficiencyData.fTotalACOutputKWH_Daily = 0.0;
        }

        err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 200, 1900, &varV, 4);
        if(err == ERR_DPR_OK)
        {
            efficiencyData.fTotalACInputKWH_Monthly = varV.fValue;
        }
        else
        {
            efficiencyData.fTotalACInputKWH_Monthly = 0.0;
        }

        err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 200, 1901, &varV, 4);
        if(err == ERR_DPR_OK)
        {
            efficiencyData.fTotalACOutputKWH_Monthly = varV.fValue;
        }
        else
        {
            efficiencyData.fTotalACOutputKWH_Monthly = 0.0;
        }

        err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 200, 1902, &varV, 4);
        if(err == ERR_DPR_OK)
        {
            efficiencyData.fTotalACInputKWH_Yearly = varV.fValue;
        }
        else
        {
            efficiencyData.fTotalACInputKWH_Yearly = 0.0;
        }

        err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 200, 1903, &varV, 4);
        if(err == ERR_DPR_OK)
        {
            efficiencyData.fTotalACOutputKWH_Yearly = varV.fValue;
        }
        else
        {
            efficiencyData.fTotalACOutputKWH_Yearly = 0.0;
        }

        efficiencyData.nFlagIsMonthDataRecordNeeded = 0;
        efficiencyData.nFlagIsYearDataRecordNeeded = 0;
    }

    if(nCurr_MDay != pCCU->nLast_MDay) //在这里把上一次记录写入数据库
    {
        TRACE("Now send msg for saving the efficiency data into DB!\n");
        EVENT_ACCUMULATED_DATA evAccumulatedData;
        time_t tmNow = _NOW_;
        time_t tmLast = efficiencyData.tmTimeSaved;
        gmtime_r(&tmNow, &gmTimeNow);
        gmtime_r(&tmLast, &gmTimeLast);

        efficiencyData.fTotalACInputKWH_Daily += pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[pCCU->nLast_Hour];
        varV.fValue = efficiencyData.fTotalACInputKWH_Daily;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1898, varV);

        efficiencyData.fTotalACOutputKWH_Daily += pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[pCCU->nLast_Hour];
        varV.fValue = efficiencyData.fTotalACOutputKWH_Daily;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1899, varV);

        efficiencyData.fTotalACOutputKWH_Daily += pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[pCCU->nLast_Hour];
        varV.fValue = efficiencyData.fTotalACInputKWH_Monthly;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1900, varV);

        efficiencyData.fTotalACOutputKWH_Daily += pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[pCCU->nLast_Hour];
        varV.fValue = efficiencyData.fTotalACOutputKWH_Monthly;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1901, varV);

        efficiencyData.fTotalACOutputKWH_Daily += pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[pCCU->nLast_Hour];
        varV.fValue = efficiencyData.fTotalACInputKWH_Yearly;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1902, varV);

        efficiencyData.fTotalACOutputKWH_Daily += pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[pCCU->nLast_Hour];
        varV.fValue = efficiencyData.fTotalACOutputKWH_Yearly;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1903, varV);

        // printf("4 time now is:%d\n", efficiencyData.tmTimeSaved);
        // printf("4 input kwh D is: %f\n", efficiencyData.fTotalACInputKWH_Daily);
        // printf("4 output kwh D is: %f\n", efficiencyData.fTotalACOutputKWH_Daily);
        // printf("4 input kwh M is: %f\n", efficiencyData.fTotalACInputKWH_Monthly);
        // printf("4 output kwh M is: %f\n", efficiencyData.fTotalACOutputKWH_Monthly);
        // printf("4 input kwh Y is: %f\n", efficiencyData.fTotalACInputKWH_Yearly);
        // printf("4 output kwh Y is: %f\n", efficiencyData.fTotalACOutputKWH_Yearly);

        if(gmTimeLast.tm_mon != gmTimeNow.tm_mon) //decide that should monthly data be saved to the database or not
        {
            efficiencyData.nFlagIsMonthDataRecordNeeded = 1;
        }

        if(gmTimeLast.tm_year != gmTimeNow.tm_year) //decide that should yearly data be saved to the database or not
        {
            efficiencyData.nFlagIsYearDataRecordNeeded = 1;
        }

        evAccumulatedData.pEfficiency_data = &efficiencyData;

        SendMsg(pCCU->hSampler, EVENT_TYPE_SAVE_ACCUMULATED_DATA, EVENT_MSG_STATUS_CHANGED,
                &evAccumulatedData, sizeof(EVENT_ACCUMULATED_DATA));

        Sleep(1000);

        if(gmTimeLast.tm_mon != gmTimeNow.tm_mon) //reset monthly data
        {
            efficiencyData.fTotalACInputKWH_Monthly = 0.0;
            varV.fValue = efficiencyData.fTotalACInputKWH_Monthly;
            Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1900, varV);

            efficiencyData.fTotalACOutputKWH_Monthly = 0.0;
            varV.fValue = efficiencyData.fTotalACOutputKWH_Monthly;
            Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1901, varV);
        }

        if(gmTimeLast.tm_year != gmTimeNow.tm_year) //reset yearly data
        {
            efficiencyData.fTotalACInputKWH_Yearly = 0.0;
            varV.fValue = efficiencyData.fTotalACInputKWH_Yearly;
            Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1902, varV);

            efficiencyData.fTotalACOutputKWH_Yearly = 0.0;
            varV.fValue = efficiencyData.fTotalACOutputKWH_Yearly;
            Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1903, varV);
        }

        //reset daily data
        varV.ulValue = _NOW_;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1897, varV);

        efficiencyData.fTotalACInputKWH_Daily = 0.0;
        varV.fValue = efficiencyData.fTotalACInputKWH_Daily;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1898, varV);

        efficiencyData.fTotalACOutputKWH_Daily = 0.0;
        varV.fValue = efficiencyData.fTotalACOutputKWH_Daily;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1899, varV);
    }

    else if(nCurr_Hour != pCCU->nLast_Hour)
    {
        if((nCurr_Hour - pCCU->nLast_Hour) >= 1) //Only Right Hour Time Changed, then Do KWH add Calc
        {
            varV.ulValue = _NOW_;
            Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1897, varV);

            efficiencyData.fTotalACInputKWH_Daily += pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[pCCU->nLast_Hour];
            varV.fValue = efficiencyData.fTotalACInputKWH_Daily;
            Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1898, varV);

            efficiencyData.fTotalACOutputKWH_Daily += pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[pCCU->nLast_Hour];
            varV.fValue = efficiencyData.fTotalACOutputKWH_Daily;
            Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1899, varV);

            efficiencyData.fTotalACInputKWH_Monthly += pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[pCCU->nLast_Hour];
            varV.fValue = efficiencyData.fTotalACInputKWH_Monthly;
            Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1900, varV);

            efficiencyData.fTotalACOutputKWH_Monthly += pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[pCCU->nLast_Hour];
            varV.fValue = efficiencyData.fTotalACOutputKWH_Monthly;
            Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1901, varV);

            efficiencyData.fTotalACInputKWH_Yearly += pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[pCCU->nLast_Hour];
            varV.fValue = efficiencyData.fTotalACInputKWH_Yearly;
            Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1902, varV);

            efficiencyData.fTotalACOutputKWH_Yearly += pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[pCCU->nLast_Hour];
            varV.fValue = efficiencyData.fTotalACOutputKWH_Yearly;
            Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1903, varV);
        }
    }
}


static void AddDaysSector4YearKWH(ROUGH_CCU_DATA *pCCU, int nYearIdx, int nLast_Hour, FEERATE_ATTRI    emLast_FeeRate)
{
    int nGrp = 0;
    //int nDaySector = 0;

    if((nYearIdx < 0) || (nYearIdx > MAX_YEARS_SECTOR) )
    {
        TRACE("nYearIdx = %d Error, Go Back\n", nYearIdx);
        return;
    }
    
    if((nLast_Hour < 0) || (nLast_Hour > MAX_HOURS_SECTOR) )
    {
        TRACE("nCurr_Hour = %d Error nCurr_Hour, Go Back\n", nLast_Hour);
        return;
    }

    pCCU->hisESData.es_Data.years_Data.tmTimeStamp_Y[nYearIdx] = _NOW_ - 60;

    if(pCCU->emsOrgCalcData.fRmtAC_RealTime_ReadKWH > 1.0)//This should Be a Valid Value
    {
        pCCU->hisESData.es_Data.years_Data.fAC_CurrKWH_Reading[nYearIdx] = pCCU->emsOrgCalcData.fRmtAC_RealTime_ReadKWH;
    }

    pCCU->hisESData.es_Data.years_Data.fACTotalCostKWH_Y[nYearIdx] += pCCU->hisESData.es_Data.hours_Data.fAC_CostKWH_Sector[nLast_Hour];

    //Weekly Org AC Load Cost KWH
    pCCU->hisESData.es_Data.years_Data.fOrgACLoad_CostKWH_Y[nYearIdx] += pCCU->hisESData.es_Data.hours_Data.fOrgACLoad_CostKWH_Sector[nLast_Hour];

	//Weekly Remote AC Reverse Output KWH
	pCCU->hisESData.es_Data.years_Data.fAC_RevrtOutPutKWH_Y[nYearIdx] += pCCU->hisESData.es_Data.hours_Data.fAC_RevrtOutPutKWH_Sector[nLast_Hour];

	//Weekly Local AC Meter Input KWH 
	pCCU->hisESData.es_Data.years_Data.fLocalAC_INPUTKWH_Y[nYearIdx] += pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[nLast_Hour];

	//Weekly Local AC Meter Output KWH 
	pCCU->hisESData.es_Data.years_Data.fLocalAC_OUTPUTKWH_Y[nYearIdx] += pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[nLast_Hour];

	//Weekly DC Load Cost KWH
	pCCU->hisESData.es_Data.years_Data.fDCLoad_CostKWH_Y[nYearIdx] += pCCU->hisESData.es_Data.hours_Data.fDCLoad_CostKWH_Sector[nLast_Hour];


    for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
        pCCU->hisESData.es_Data.years_Data.fMpptTotalGKWH_Y[nGrp][nYearIdx] += pCCU->hisESData.es_Data.hours_Data.fMppt_GKWH_Sector[nGrp][nLast_Hour];
    }

    for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
        pCCU->hisESData.es_Data.years_Data.fBattTotalChrgKWH_Y[nGrp][nYearIdx] += pCCU->hisESData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[nGrp][nLast_Hour];

        pCCU->hisESData.es_Data.years_Data.fBattTotalDisChrgKWH_Y[nGrp][nYearIdx] += pCCU->hisESData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[nGrp][nLast_Hour];

        if((pCCU->emsOrgCalcData.fBatt_Rate_KWH[nGrp] > 0) && (pCCU->emsOrgCalcData.fBatt_Rate_KWH[nGrp] < 9999.0))
        {
            pCCU->hisESData.es_Data.years_Data.fBattChrgTimes_Y[nGrp][nYearIdx] = 
                pCCU->hisESData.es_Data.years_Data.fBattTotalChrgKWH_Y[nGrp][nYearIdx]/pCCU->emsOrgCalcData.fBatt_Rate_KWH[nGrp];
            
            pCCU->hisESData.es_Data.years_Data.fBattDisChrgTimes_Y[nGrp][nYearIdx] = 
                pCCU->hisESData.es_Data.years_Data.fBattTotalDisChrgKWH_Y[nGrp][nYearIdx]/pCCU->emsOrgCalcData.fBatt_Rate_KWH[nGrp];
        }
    }

    if(emLast_FeeRate == FEERATE_PEAK)
    {
    	pCCU->hisESData.es_Data.years_Data.fLocalAC_Peak_InputKWH_Y[nYearIdx]  += pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[nLast_Hour];
		pCCU->hisESData.es_Data.years_Data.fLocalAC_Peak_OutPutKWH_Y[nYearIdx] += pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[nLast_Hour];

    }    
    else if(emLast_FeeRate == FEERATE_FLAT)
    {
		pCCU->hisESData.es_Data.years_Data.fLocalAC_Flat_InputKWH_Y[nYearIdx] += pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[nLast_Hour];
		pCCU->hisESData.es_Data.years_Data.fLocalAC_Flat_OutputKWH_Y[nYearIdx] += pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[nLast_Hour];

    }
    else if(emLast_FeeRate == FEERATE_VALLEY)
    {
        pCCU->hisESData.es_Data.years_Data.fLocalAC_Valley_InputKWH_Y[nYearIdx] += pCCU->hisESData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[nLast_Hour];
		pCCU->hisESData.es_Data.years_Data.fLocalAC_Valley_OutputKWH_Y[nYearIdx] += pCCU->hisESData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[nLast_Hour];
    }


    TRACEX("years_Data.fACTotalCostKWH_Y[%d].fValue=%f\
    \n\tyears_Data.fMpptTotalGKWH_Y[0][%d].fValue=%f | years_Data.fMpptTotalGKWH_Y[1][%d].fValue=%f\
    \n\tyears_Data.fBattTotalChrgKWH_Y[0][%d].fValue=%f | years_Data.fBattTotalChrgKWH_Y[1][%d].fValue=%f\
    \n\tyears_Data.fBattTotalDisChrgKWH_Y[0][%d].fValue=%f | years_Data.fBattTotalDisChrgKWH_Y[1][%d].fValue=%f \n", \
    nYearIdx, pCCU->hisESData.es_Data.years_Data.fACTotalCostKWH_Y[nYearIdx],\
    nYearIdx, pCCU->hisESData.es_Data.years_Data.fMpptTotalGKWH_Y[0][nYearIdx],\
    nYearIdx, pCCU->hisESData.es_Data.years_Data.fMpptTotalGKWH_Y[1][nYearIdx],\
    nYearIdx, pCCU->hisESData.es_Data.years_Data.fBattTotalChrgKWH_Y[0][nYearIdx],\
    nYearIdx, pCCU->hisESData.es_Data.years_Data.fBattTotalChrgKWH_Y[1][nYearIdx],\
    nYearIdx, pCCU->hisESData.es_Data.years_Data.fBattTotalDisChrgKWH_Y[0][nYearIdx],\
    nYearIdx, pCCU->hisESData.es_Data.years_Data.fBattTotalDisChrgKWH_Y[1][nYearIdx]);

    return;
}

void SaveES_StoreData2DB(ROUGH_CCU_DATA *pCCU)
{
    time_t tmNow = _NOW_;

    if((ABS(tmNow - pCCU->tmLastSaveESData2DB) > 120))
    {
        //now push the stat data to db
        static A_BLOB_ES_STATISTIC_DATA blobESData;
        memcpy(&blobESData.es_Data, &pCCU->hisESData.es_Data, sizeof(ES_DAILY_STAT_DATA));
        Push_BlobValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 200, 79, (void*)&blobESData,
                sizeof(A_BLOB_ES_STATISTIC_DATA), BLOB_TYPE_ES_STATISTICDATA);

        pCCU->tmLastSaveESData2DB = tmNow;
        
        pCCU->hisESData.es_Data.tmTimeSave = tmNow;

        TRACE("Now send msg for saving the data into DB!\n");
        EVENT_ES_SAVE_STATDATA evESSave;
        evESSave.pDailyData = &pCCU->hisESData;

        SendMsg(pCCU->hSampler, EVENT_TYPE_SAVE_ES_STATDATA, EVENT_MSG_STATUS_CHANGED,
                &evESSave, sizeof(EVENT_ES_SAVE_STATDATA));
    }
	
    return;
}

static void  Refresh_RealTimeReadingKWH(ROUGH_CCU_DATA *pCCU)
{ 
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

    int nGrp = 0;
    VAR_VALUE varV;
    ERRCODE_DAI err;
    
    //------------------Start Get Local Master & Slave AC Meter-----------------//
    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 407, 411, &varV, 4);
    if(err == ERR_DPR_OK)
    {
        pCCU->emsOrgCalcData.fMaster_LocalAC_RTKWH_Reading = varV.fValue;
        TRACE("fMaster_LocalAC_RTKWH_Reading = %f\n",varV.fValue);
    }
    else
    {
        pCCU->emsOrgCalcData.fMaster_LocalAC_RTKWH_Reading = 0.0;
    }

    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 407, 422, &varV, 4);
    if(err == ERR_DPR_OK)
    {
        pCCU->emsOrgCalcData.fMaster_LocalAC_RevsKWH = varV.fValue;
        TRACE("fMaster_LocalAC_RevsKWH = %f\n",varV.fValue);
    }
    else
    {
        pCCU->emsOrgCalcData.fMaster_LocalAC_RevsKWH = 0.0;
    }

    //err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 1, 33, &varV, 4);
    //if(err == ERR_DPR_OK)
    //{
    //    pCCU->emsOrgCalcData.fSlv1_LocalAC_RTKWH_Reading = varV.fValue;
    //    TRACE("fSlv1_LocalAC_ActivePwr = %f\n",varV.fValue);
    //}
    //else
    //{
        pCCU->emsOrgCalcData.fSlv1_LocalAC_RTKWH_Reading = 0.0;
    //}

    //err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 1, 34, &varV, 4);
    //if(err == ERR_DPR_OK)
    //{
    //    pCCU->emsOrgCalcData.fSlv1_LocalAC_RevsKWH = varV.fValue;
    //    TRACE("fSlv1_LocalAC_ActivePwr = %f\n",varV.fValue);
    //}
    //else
    //{
        pCCU->emsOrgCalcData.fSlv1_LocalAC_RevsKWH = 0.0;
    //}
    //------------------ End Master & Slave Local AC Meter-------------//

 	err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 408, 411, &varV, 4);
    if(err == ERR_DPR_OK)
    {
        pCCU->emsOrgCalcData.fRmtAC_RealTime_ReadKWH = varV.fValue;
        TRACE("Remote AC Total Value = %f\n",varV.fValue);
    }
    else
    {
        pCCU->emsOrgCalcData.fRmtAC_RealTime_ReadKWH = 0.0;
    }
    
    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 408, 422, &varV, 4);
    if(err == ERR_DPR_OK)
    {
        pCCU->emsOrgCalcData.fRmtAC_Revs_ReadKWH = varV.fValue;
        TRACE("Remote AC Total Value = %f\n",varV.fValue);
    }
    else
    {
        pCCU->emsOrgCalcData.fRmtAC_Revs_ReadKWH = 0.0;
    }

    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 411, 411, &varV, 4);
    if(err == ERR_DPR_OK)
    {
        pCCU->emsOrgCalcData.fRmtLoad_RealTime_ReadKWH = varV.fValue;
    }
    else
    {
        pCCU->emsOrgCalcData.fRmtLoad_RealTime_ReadKWH = 0.0;
    }
    
    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 411, 422, &varV, 4);
    if(err == ERR_DPR_OK)
    {
        pCCU->emsOrgCalcData.fRmtLoad_Revs_ReadKWH = varV.fValue;
    }
    else
    {
        pCCU->emsOrgCalcData.fRmtLoad_Revs_ReadKWH = 0.0;
    }

    for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
        err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 200, 153 + nGrp, &varV, 4);
        if(err == ERR_DPR_OK)
        {
            pCCU->emsOrgCalcData.fMPPT_RealTime_ReadKWH[nGrp] = varV.fValue;
        }
        else
        {
            pCCU->emsOrgCalcData.fMPPT_RealTime_ReadKWH[nGrp] = 0.0;
        }
    }

    err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 200, 155, &varV, 4);
    if(err == ERR_DPR_OK)
    {
        pCCU->emsOrgCalcData.fDCLoad_TotalReadKWH = varV.fValue;
    }
    else
    {
        pCCU->emsOrgCalcData.fDCLoad_TotalReadKWH = 0.0;
    }

    for(nGrp = 0 ; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
        err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 501 + nGrp, 554, &varV, 4);
        if(err == ERR_DPR_OK)
        {
            pCCU->emsOrgCalcData.fBattChrg_RealTime_KWH[nGrp] = varV.fValue;
        }
        else
        {
            pCCU->emsOrgCalcData.fBattChrg_RealTime_KWH[nGrp] = 0.0;
        }
        

        err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 501 + nGrp, 555, &varV, 4);
        if(err == ERR_DPR_OK)
        {
            pCCU->emsOrgCalcData.fBattDisChrg_RealTime_KWH[nGrp] = varV.fValue;
        }
        else
        {
            pCCU->emsOrgCalcData.fBattDisChrg_RealTime_KWH[nGrp] = 0.0;
        }

        err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 501 + nGrp, 506, &varV, 4);
        if(err == ERR_DPR_OK)
        {
            pCCU->emsOrgCalcData.fBatt_Rate_KWH[nGrp] = varV.fValue;
        }
        else
        {
            pCCU->emsOrgCalcData.fBatt_Rate_KWH[nGrp] = 0.0;
        }   
    }    
    
    /*
    //Just or Test
    pCCU->emsOrgCalcData.fMaster_LocalAC_RTKWH_Reading += 0.2;
	pCCU->emsOrgCalcData.fMaster_LocalAC_RevsKWH += 0.4;

 	pCCU->emsOrgCalcData.fSlv1_LocalAC_RTKWH_Reading += 0.6;
	pCCU->emsOrgCalcData.fSlv1_LocalAC_RevsKWH += 0.8;

    pCCU->emsOrgCalcData.fRmtAC_Revs_ReadKWH += 1.2;
 
	pCCU->emsOrgCalcData.fDCLoad_TotalReadKWH += 1.5;

    pCCU->emsOrgCalcData.fRmtAC_RealTime_ReadKWH += 0.1;

    for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
        pCCU->emsOrgCalcData.fMPPT_RealTime_ReadKWH[nGrp] +=  (0.2*(nGrp+1));
    }

    for(nGrp = 0; nGrp < MAX_SINGLE_STBATT_NUM; nGrp++)
    {
        pCCU->emsOrgCalcData.fBattChrg_RealTime_KWH[nGrp] += (0.3*(nGrp+1));
        pCCU->emsOrgCalcData.fBattDisChrg_RealTime_KWH[nGrp] += (0.4*(nGrp+1));
    }
    

    TRACE("fRmtAC_RealTime_ReadKWH=%f \
    \n\tfMPPT_RealTime_ReadKWH[0]=%f, fMPPT_RealTime_ReadKWH[1]=%f,\
    \n\tfBatt_RealTime_KWH_Chrg[0]=%f,fBatt_RealTime_KWH_DisChrg[0]=%f,\
    \n\tfBatt_RealTime_KWH_Chrg[1]=%f,fBatt_RealTime_KWH_DisChrg[1]=%f\n",\
        pCCU->emsOrgCalcData.fRmtAC_RealTime_ReadKWH,\
        pCCU->emsOrgCalcData.fMPPT_RealTime_ReadKWH[0],\
        pCCU->emsOrgCalcData.fMPPT_RealTime_ReadKWH[1],\
        pCCU->emsOrgCalcData.fBattChrg_RealTime_KWH[0],\
        pCCU->emsOrgCalcData.fBattDisChrg_RealTime_KWH[0],\
        pCCU->emsOrgCalcData.fBattChrg_RealTime_KWH[1],\
        pCCU->emsOrgCalcData.fBattDisChrg_RealTime_KWH[1]);
    */
    return;
}

static void Refresh_BattGrpSOC(ROUGH_CCU_DATA *pCCU)
{
    int     n = 0, nIdx = 0;
    float   fSOC = 0.0;
    time_t  tmNow = _NOW_;
    for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
    {
        nIdx = pCCU->hisESData.es_Data.hours_Data.nSOC_STBatt_TotalPoint[n];
	    fSOC = (float)pCCU->blobSingleBattData[n].nSOC;
	    if((fabs(fSOC - pCCU->emsOrgCalcData.fSTBatt_LastSOC[n]) > 1.0) && (nIdx < MAX_TIME_VARIANT_VAL_ITEMS))
	    {
            pCCU->hisESData.es_Data.hours_Data.soc_RT_STBatt[n][nIdx].tmTimeStamp = tmNow;
            pCCU->hisESData.es_Data.hours_Data.soc_RT_STBatt[n][nIdx].fValue = fSOC;
            pCCU->emsOrgCalcData.fSTBatt_LastSOC[n] = fSOC;
            pCCU->hisESData.es_Data.hours_Data.nSOC_STBatt_TotalPoint[n]++;
	    }
    }

    return;
}

static void InitLastReadingKWH(ROUGH_CCU_DATA *pCCU)
{
    int n = 0;
    time_t tmNow = _NOW_;

    //Event Handler ,Restore BlobHisStrData
    pCCU->tmLastCalcESStoreData = tmNow;

    pCCU->tmLastSaveESData2DB = tmNow;

    //Init Last Reading Data
    pCCU->emsOrgCalcData.fRmtAC_LastkWhReading = pCCU->emsOrgCalcData.fRmtAC_RealTime_ReadKWH;
    
    pCCU->emsOrgCalcData.fRmtAC_Last_Revs_ReadKWH = pCCU->emsOrgCalcData.fRmtAC_Revs_ReadKWH;

    pCCU->emsOrgCalcData.fRmtLoad_LastkWhReading = pCCU->emsOrgCalcData.fRmtLoad_RealTime_ReadKWH;

    pCCU->emsOrgCalcData.fRmtLoad_Last_Revs_ReadKWH = pCCU->emsOrgCalcData.fRmtLoad_Revs_ReadKWH;

    pCCU->emsOrgCalcData.fLast_Master_LocalAC_RTKWH_Reading = pCCU->emsOrgCalcData.fMaster_LocalAC_RTKWH_Reading;
	pCCU->emsOrgCalcData.fLast_Master_LocalAC_RevsKWH = pCCU->emsOrgCalcData.fMaster_LocalAC_RevsKWH;

    pCCU->emsOrgCalcData.fLast_Slv1_LocalAC_RTKWH_Reading = pCCU->emsOrgCalcData.fSlv1_LocalAC_RTKWH_Reading;
	pCCU->emsOrgCalcData.fLast_Slv1_LocalAC_RevsKWH = pCCU->emsOrgCalcData.fSlv1_LocalAC_RevsKWH;

    pCCU->emsOrgCalcData.fLast_DCLoad_TotalReadKWH = pCCU->emsOrgCalcData.fDCLoad_TotalReadKWH;
     
    for(n = 0 ; n < MAX_SINGLE_STBATT_NUM; n++)
    {
        pCCU->emsOrgCalcData.fMPPT_LastkWhReading[n] = pCCU->emsOrgCalcData.fMPPT_RealTime_ReadKWH[n];
    }

    for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
    {
        pCCU->emsOrgCalcData.fBattChrg_LastkWhReading[n] = pCCU->emsOrgCalcData.fBattChrg_RealTime_KWH[n];
        pCCU->emsOrgCalcData.fBattDisChrg_LastkWhReading[n] = pCCU->emsOrgCalcData.fBattDisChrg_RealTime_KWH[n];
    }
    return;
}


void Manage_ESStoreData(ROUGH_CCU_DATA *pCCU)
{

    time_t tmNow = _NOW_;
    struct tm gmTime;
    gmtime_r(&tmNow, &gmTime);
    
    int             n = 0;
    int             nCurr_Hour = 2*gmTime.tm_hour;
    if((gmTime.tm_min >= 30))
	{
		nCurr_Hour += 1;
	}

    int                     nFeeRate_Sector = 0;
    //int                     nCurr_Wday = gmTime.tm_wday;
    int                     nCurr_MDay = gmTime.tm_mday - 1;
    int                     nCurr_tmYear = gmTime.tm_year - 120; //2020-1900=120, offset From 120
    FEERATE_ATTRI           emFeeRate = FEERATE_SHARP;

    static BOOL             bFirstRun = TRUE;
 
    TRACE("---Start--tmNow=[%d]-tm_yday=%d: %d-%d-%d %d:%d:%d------\n", \
        tmNow, gmTime.tm_year , gmTime.tm_yday , gmTime.tm_mon + 1, gmTime.tm_mday, gmTime.tm_hour, gmTime.tm_min, gmTime.tm_sec);
    
    //calculate it every 10 sec.
    UINT nWait_Sec = (bFirstRun == TRUE) ? 30 : 10;

    if(ABS(tmNow - pCCU->tmLastCalcESStoreData) < nWait_Sec)
    {
        TRACE("GoBack:tmNow=[%ld],tmLastCalcESStoreData=[%ld],nWait_Sec=[%d]\n",tmNow , pCCU->tmLastCalcESStoreData, nWait_Sec);
        return;
    }

    if(bFirstRun == TRUE)
    {
        bFirstRun = FALSE;

        if((pCCU->nLast_Hour == 0) && 
            (pCCU->nLast_MDay == 0) && 
            (pCCU->nLast_tmYear == 0))
        {
            pCCU->nLast_Hour    = nCurr_Hour;
            pCCU->nLast_MDay    = nCurr_MDay;
            pCCU->nLast_tmYear  = nCurr_tmYear;
            nFeeRate_Sector = Get_FeeRate_Setctor_Accroding2Time(&pCCU->blobFeeRate, tmNow);
            if(nFeeRate_Sector != -1)
            {
                pCCU->emLast_FeeRate = pCCU->blobFeeRate.feeRateSectors[nFeeRate_Sector].emFeeRate;
            }

            LogOut(MAIN_MODULE_SAMPLER, "CCU_SAMP_HIS", LOG_TYPE_ERROR, "Restore LastTime FAIL.tmYear[%d]-MDay[%d] Hour[%d]:tm_min[%d]Last_FeeRate[%d]",
                pCCU->nLast_tmYear,pCCU->nLast_MDay,pCCU->nLast_Hour, gmTime.tm_min, pCCU->emLast_FeeRate);
        }
        
        // printf("First Enter:Last Hour[%d]MDay[%d]tmYear[%d]Last_FeeRate[%d]\n", \
        //     pCCU->nLast_Hour,pCCU->nLast_MDay, pCCU->nLast_tmYear, pCCU->emLast_FeeRate);
        
        Refresh_RealTimeReadingKWH(pCCU);

        InitLastReadingKWH(pCCU);
       
        pCCU->tmLastSaveESData2DB = 0; //First Run,Need Sync to DB
        return;
    }

    TRACE("tmNow=%ld , pCCU->tmLastCalcESStoreData=%ld nCurr_Hour=[%d]\n",\
        tmNow , pCCU->tmLastCalcESStoreData, nCurr_Hour);

	pCCU->tmLastCalcESStoreData = tmNow;

	Refresh_RealTimeReadingKWH(pCCU);

    Refresh_BattGrpSOC(pCCU);

    CalcACMeter_TMSectorKWH(pCCU,  nCurr_Hour);

    CalcDCLoad_TMSectorKWH(pCCU,  nCurr_Hour);
 
    CalcMPPT_TMSectorKWH(pCCU,  nCurr_Hour);

    CalcBatt_TMSectorKWH(pCCU,  nCurr_Hour);

    CalcOff_GridLoad_TMSectorKWH(pCCU,  nCurr_Hour);

    //RefreshWeekly24HSectorKWH(pCCU, nCurr_Wday, nCurr_Hour);

    RefreshDailySectorTemp(pCCU, nCurr_Hour);

    RefreshDailyEfficiencyData(pCCU, nCurr_Hour, nCurr_MDay);

    // Step1. If Hour Sector Changed, calc Day Statistic And Year Statistic KWH
    nFeeRate_Sector = Get_FeeRate_Setctor_Accroding2Time(&pCCU->blobFeeRate, tmNow);
    if(nFeeRate_Sector != -1)
    {
        emFeeRate = pCCU->blobFeeRate.feeRateSectors[nFeeRate_Sector].emFeeRate;
    }

    TRACE("nFeeRate_Sector=[%d] emFeeRate=[%d]\n", nFeeRate_Sector , emFeeRate);

    if(nCurr_Hour != pCCU->nLast_Hour)
    {
        if(((nCurr_Hour - pCCU->nLast_Hour) == 1) && (nCurr_MDay == pCCU->nLast_MDay)) //Only Right Hour Time Changed, then Do KWH add Calc
        {
            AddHourSector4DaysKWH(pCCU, nCurr_MDay, pCCU->nLast_Hour, pCCU->emLast_FeeRate);
            AddDaysSector4YearKWH(pCCU, nCurr_tmYear, pCCU->nLast_Hour, pCCU->emLast_FeeRate);
            pCCU->tmLastSaveESData2DB = 0; //Time Sector Changed,Need Sync to DB ASAP
            TRACE("### nCurr_Hour=%d,s_nLast_Hour=%d,nCurr_MDay=%d,s_nLast_MDay=%d\n", \
                nCurr_Hour , pCCU->nLast_Hour, nCurr_MDay , pCCU->nLast_MDay);
        }
        else if((nCurr_Hour == 0) && (nCurr_MDay != pCCU->nLast_MDay)) //23:00~00:00 Curr_Hour = 00:00
        {
            AddHourSector4DaysKWH(pCCU, pCCU->nLast_MDay, pCCU->nLast_Hour, pCCU->emLast_FeeRate);
            AddDaysSector4YearKWH(pCCU, pCCU->nLast_tmYear, pCCU->nLast_Hour, pCCU->emLast_FeeRate);
            pCCU->tmLastSaveESData2DB = 0;//Time Sector Changed,Need Sync to DB ASAP
            TRACE("*** nCurr_Hour=[%d], nLast_Hour=[%d], nCurr_MDay=[%d], nLast_MDay=[%d]\n", \
                nCurr_Hour , pCCU->nLast_Hour, nCurr_MDay , pCCU->nLast_MDay);
        }
        pCCU->nLast_Hour = nCurr_Hour;
        pCCU->emLast_FeeRate = emFeeRate;
    }

    //Step2. If Day Changed, Clear Hour Sector KWH 
    if(nCurr_MDay != pCCU->nLast_MDay)
    {
        pCCU->nLast_MDay = nCurr_MDay;
        Init_CurrDayKWH(pCCU, nCurr_MDay); //Added By John 2021-09-24,To Solve KWH Dubble Added Bug.
        ZERO_POBJS(&pCCU->hisESData.es_Data.hours_Data, 1);
        TRACE("pCCU->nLast_MDay=%d, nCurr_MDay=%d\n",  pCCU->nLast_MDay , nCurr_MDay);
        //Logout
        return;
    }

    //Step3. If Year Changed, Clear Hour Sector KWH 		
	if(nCurr_tmYear != pCCU->nLast_tmYear)
    {
        pCCU->nLast_tmYear = nCurr_tmYear;
        ZERO_POBJS(&pCCU->hisESData.es_Data.hours_Data, 1);
        TRACE("s_nLast_tmYear=%d -- nCurr_tmYear=%d\n",pCCU->nLast_tmYear , nCurr_tmYear);
        //Logout
        return;
    }

    SaveES_StoreData2DB(pCCU);
    
	TRACE("\n\t--------- End ---------\n");

    return;
}