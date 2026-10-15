/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : TimesStatis.c
** @Founder      : ZQD808
** @Date         : 2026.03.10
** @Function     : 电池时间与次数统计模块
** @Instructions :
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "ParaInterface.h"
#include "CurrIntegral.h"
#include "CapInfoCalc.h"
#include "EnerInfoCalc.h"
#include "EnerInterface.h"
#include "CalcInterface.h"
#include "CharactCalc.h"
#include "StaInterface.h"
#include "TimesStatis.h"
#include "ParaRead.h"
#include "ParaWrite.h"
#include <stdio.h>
#include <debug.h>

extern u16 *gGStatistTime_59;                /*时间统计信息*/
extern u16 *gGStatistErr_60;                 /*故障统计信息*/
extern u16 gGBmuGenPara_102[];               /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	         /*主控高级参数值*/

static u32 sHisChgDhgState[2]  = {0};        /*上次充放电状态/上次充放电量*/
static u16 sHisMonErrTime[2]  = {0};         /*上次月故障统计时间(年月)*/
/*=============================================================================================
** @Name      : void GroupTimesStatisInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组时间次数统计初始化
** @The notes :
**===========================================================================================*/
void GroupTimesStatisInit(void)
{
    u32 info32[2] = {0};
    u16 info16[5] = {0};

    if(gGStatistTime_59 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGStatistTime_59 null!!!\r\n", __func__);
        return;
    }
    if(gGStatistErr_60 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGStatistErr_60 null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        /*计算充放电次数状态*/
        if(TRUE == ParaReadStoreChgTimesState(clunum, info32, 2))
        {
            /*通过时间计算充放电次数*/
            #if(1 == CALC_TIMES_BY_TIME)
            if((1 <= info32[1]) && (info32[1] <= 2))
            {
                sHisChgDhgState[clunum] = info32[1];
            }
            else
            {
                sHisChgDhgState[clunum] = 1;
            }
            /*通过历史电量计算充放电次数*/
            #else
            sHisChgDhgState[clunum] = info32[1];
            #endif
        }
        else
        {
            /*通过时间计算充放电次数*/
            #if(1 == CALC_TIMES_BY_TIME)
            sHisChgDhgState[clunum] = 1;
            /*通过历史电量计算充放电次数*/
            #else
            sHisChgDhgState[clunum] = 0;
            #endif
        }
        
        /*统计充放电次数当前次数*/
        if(TRUE == ParaReadStoreChgDhgTimes(clunum, info16, 2))
        {
            if(clunum == 0)
            {
                gGStatistTime_59[eTime59_1ChgCycle] = info16[1];
            }
            else
            {
                gGStatistTime_59[eTime59_2ChgCycle] = info16[1];
            }
            
        }
        /*未计算次数或清零*/
        else
        {
            if(clunum == 0)
            {
                gGStatistTime_59[eTime59_1ChgCycle] = 1;
            }
            else
            {
                 gGStatistTime_59[eTime59_2ChgCycle] = 1;
            }
        }

        /*月故障统计时间*/
        if(TRUE == ParaReadStoreMonErrTime(clunum, info16, 2))
        {
            sHisMonErrTime[clunum] = info16[1];
        }
        /*未计算次数或清零*/
        else
        {
            sHisMonErrTime[clunum] = 0;
        }

        /*读取故障告警数目统计*/
        if(TRUE == ParaReadStoreHisErrNum(clunum, info16, 5))
        {
            if(clunum == 0)
            {
                gGStatistErr_60[eErr60_1MonWarNum] = info16[1];
                gGStatistErr_60[eErr60_1MonErrNum] = info16[2];
                gGStatistErr_60[eErr60_1HisWarNum] = info16[3];
                gGStatistErr_60[eErr60_1HisErrNum] = info16[4];
            }
            else
            {
                gGStatistErr_60[eErr60_2MonWarNum] = info16[1];
                gGStatistErr_60[eErr60_2MonErrNum] = info16[2];
                gGStatistErr_60[eErr60_2HisWarNum] = info16[3];
                gGStatistErr_60[eErr60_2HisErrNum] = info16[4];
            }
        }
        else
        {
            if(clunum == 0)
            {
                gGStatistErr_60[eErr60_1MonWarNum] = 0;
                gGStatistErr_60[eErr60_1MonErrNum] = 0;
                gGStatistErr_60[eErr60_1HisWarNum] = 0;
                gGStatistErr_60[eErr60_1HisErrNum] = 0;
            }
            else
            {
                gGStatistErr_60[eErr60_2MonWarNum] = 0;
                gGStatistErr_60[eErr60_2MonErrNum] = 0;
                gGStatistErr_60[eErr60_2HisWarNum] = 0;
                gGStatistErr_60[eErr60_2HisErrNum] = 0;
            }
        }
    }
}
/*=============================================================================================
** @Name      : void GroupChgTimesStatisTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组充放电次数统计任务
** @The notes : 2s
**===========================================================================================*/
void GroupChgTimesStatisTask(void)
{
    /*通过时间计算充放电次数*/
    #if(1 == CALC_TIMES_BY_TIME)
    u8 chgState = 0;
    static u8 sBefState = 0;
    static u16 sChgTimers = 0;
    static u16 sDhgTimers = 0;
    
    chgState = (u8)GetGChgDhgStateAPI();            /*当前充放电状态*/
    
    if(chgState == 1)                               /*正在充电*/
    {
        if(chgState == sBefState)                   /*当前状态和上一状态相同(持续充电)*/
        {
            sChgTimers++;
            if(sChgTimers >= CHGDHG_TIMES_ADD_TIME) /*长时间充电10min(默认)*/
            {
                sChgTimers = CHGDHG_TIMES_ADD_TIME;
                
                if(2 == sHisChgDhgState)            /*历史状态是放电*/
                {
                	gGStatistTime_70[eTime70_ChgDchCycle]++;/*充放电增加一次*/
                    sHisChgDhgState = chgState;     /*更新历史值*/
                    ParaWriteStoreChgTimesState(1, &chgState, 1);
                    ParaWriteStoreChgDhgTimes(1, &gGStatistTime_70[eTime70_ChgCycle], 1);
                             
                }
                else if(sHisChgDhgState != 1)       /*历史状态不是充电也不是放电(参数值异常)*/
                {
                    sHisChgDhgState = chgState;     /*更新历史值*/
                    ParaWriteStoreChgTimesState(1, &chgState, 1);
                }       
            }
        }
        else                                        /*状态刚改变为充电*/
        {
            sBefState = chgState;                   /*开始计算充电时间*/
            sChgTimers = 0;
        }
    }
    else if(chgState == 2)                          /*正在放电*/
    {
        if(chgState == sBefState)                   /*当前状态和上一状态相同(持续)*/
        {
            sDhgTimers++;
            if(sDhgTimers >= CHGDHG_TIMES_ADD_TIME) /*长时间放电10min(默认)*/
            {
                sDhgTimers = CHGDHG_TIMES_ADD_TIME;
                
                if(1 == sHisChgDhgState)            /*历史状态是充电*/
                {
                	gGStatistTime_70[eTime70_ChgDchCycle]++;/*充放电增加一次*/
                    sHisChgDhgState = chgState;     /*更新历史值*/
                    ParaWriteStoreChgTimesState(1, &chgState, 1);
                    ParaWriteStoreChgDhgTimes(1, &gGStatistTime_70[eTime70_ChgCycle], 1);
                             
                }
                else if(sHisChgDhgState != 2)       /*历史状态不是充电也不是放电(参数值异常)*/
                {
                    sHisChgDhgState = chgState;     /*更新历史值*/
                    ParaWriteStoreChgTimesState(1, &chgState, 1);
                }
            }
        }
        else                                        /*状态刚改变为放电*/
        {
            sBefState = chgState;                   /*开始计算放电时间*/
            sDhgTimers = 0;
        }
    }
    /*通过历史电量计算充放电次数*/
    #else
    u32 avgCap = 0;
    u32 chgCap = 0;
    u32 dhgCap = 0;
    u32 totalCap = 0;
    if(gGStatistTime_59 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGStatistTime_59 null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        avgCap = 0;
        chgCap = 0;
        dhgCap = 0;
        totalCap = 0;
        chgCap = GetGHisChgCapAPI(clunum);
        dhgCap = GetGHisDhgCapAPI(clunum);
        
        /*计算当前历史充放电量平均值(防止越界)*/
        avgCap = ABS(chgCap, dhgCap) / 2;
        
        if(chgCap >= dhgCap)
        {
            avgCap = avgCap + dhgCap;
        }
        else
        {
            avgCap = avgCap + chgCap;
        }
        
        /*当前历史电量比上次记录值大(历史电量未清零)*/
        if(avgCap >= sHisChgDhgState[clunum])
        {
            /*小储能历史电量用mAH为单位*/
            #if(1 == SMALL_ESS_EN)
            totalCap = GetGroupTotalCapAPI();
            #else
            totalCap = GetGroupTotalCapAPI(clunum) / 1000;
            #endif
            
            /*充放电累加到容量值*/
            if(avgCap >= (sHisChgDhgState[clunum] + totalCap))
            {
                if(clunum == 0)
                {
                    gGStatistTime_59[eTime59_1ChgCycle]++;
                }
                else
                {
                    gGStatistTime_59[eTime59_2ChgCycle]++;
                }
                

                sHisChgDhgState[clunum] = avgCap;                /*更新历史值*/
                ParaWriteStoreChgTimesState(clunum, 1, &avgCap, 1);
                if(clunum == 0)
                {
                    ParaWriteStoreChgDhgTimes(clunum, 1, &gGStatistTime_59[eTime59_1ChgCycle], 1);
                }
                else
                {
                    ParaWriteStoreChgDhgTimes(clunum, 1, &gGStatistTime_59[eTime59_2ChgCycle], 1);
                }
            }
        }
        /*历史电量未清零*/
        else
        {
            /*充放电次数复位*/
            ClrGroupChgDhgTimesAPI(clunum);
            sHisChgDhgState[clunum] = avgCap;
            ParaWriteStoreChgTimesState(clunum, 1, &avgCap, 1);
        }
    }
    #endif
}
/*=============================================================================================
** @Name      : void GroupAllowTimeStatisTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组可充放电时间统计任务
** @The notes : 2s
**===========================================================================================*/
void GroupAllowTimeStatisTask(void)
{
    s16 power = 0;
    u16 sumVol = 0;
    u32 chgCap = 0;
    u32 dhgCap = 0;
    u32 calcTime = 0;
    //u16 chgPower = 0;
    //u16 dhgPower = 0;
    u16 chgCurr = 0;
    u16 dhgCurr = 0;

    if(gGStatistTime_59 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGStatistTime_59 null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        /*电池组初始化时不检测*/
        if(eWORK_INIT == GetGWorkStateAPI(clunum))
        {
            continue;
        }

        dhgCap = GetGroupNowCapAPI(clunum);
        chgCap = GetGroupTotalCapAPI(clunum) - dhgCap;

        sumVol = GetGCellSumVoltAPI(clunum) + 1;
        power = GetGSampOutPowerAPI(clunum);

        /*过压可充电量为0*/
        if((1 == StaGetVHLimStateHook(clunum))
            || (1 == StaGetBattVStateHook(clunum)))
        {
            chgCap = 0;
        }

        /*低压可放电量为0*/
        if((1 == StaGetVLLimStateHook(clunum))
            || (2 == StaGetBattVStateHook(clunum)))
        {
            dhgCap = 0;
        }

        /*通过功率计算
        //充电则放电取额定功率
        if(1 == GetChgDhgState())
        {
            dhgPower = gGBmuGenPara_102[eBmuGenPara102_DhgPRatLim];

            //充电功率大于0.1kW则转为正数
            if(power < -1)
            {
                chgPower = ABS(0, power);
            }
            //充电功率过小取额定功率
            else
            {
                chgPower = gGBmuGenPara_102[eBmuGenPara102_ChgPRatLim];
            }
        }
        //放电则充电取额定功率
        else if(2 == GetChgDhgState())
        {
            chgPower = gGBmuGenPara_102[eBmuGenPara102_ChgPRatLim];

            //放电功率取实际功率
            if(power > 1)
            {
                dhgPower = (u16)power;
            }
            //放电功率过小取额定功率
            else
            {
                dhgPower = gGBmuGenPara_102[eBmuGenPara102_DhgPRatLim];
            }
        }
        //待机则充放电功率取额定功率
        else
        {
            chgPower = gGBmuGenPara_102[eBmuGenPara102_ChgPRatLim];
            dhgPower = gGBmuGenPara_102[eBmuGenPara102_DhgPRatLim];
        }

        calcTime = (u32)sumVol * (chgCap / 100) * 6 / (u32)chgPower / 1000;
        if((calcTime <= 1) && (GetGChgLimCurrAPI() > 0))       //计算无剩余时间但仍可充电
        {
            gGStatistTime_70[eTime70_CanChg] = 1;                 //取剩余1min
        }
        else
        {
            gGStatistTime_70[eTime70_CanChg] = (gGStatistTime_70[eTime70_CanChg] + calcTime) / 2;
        }

        calcTime = (u32)sumVol * (dhgCap / 100) * 6 / (u32)dhgPower / 1000;
        if((calcTime <= 1) && (GetGDhgLimCurrAPI() > 0))       //计算无剩余时间但仍可充电
        {
            gGStatistTime_70[eTime70_CanChg] = 1;                 //取剩余1min
        }
        else
        {
            gGStatistTime_70[eTime70_CanChg] = (gGStatistTime_70[eTime70_CanChg] + calcTime) / 2;
        }
        */

        /*通过平均电流计算*/
        /*充电则放电取额定功率*/
        if(eCURR_CHG == GetGChgDhgStateAPI(clunum))
        {
            dhgCurr = (u16)((u32)gGBmuGenPara_102[eBmuGenPara102_DhgPRatLim] * 10000 / sumVol);

            /*充电功率大于0.1kW则转为正数*/
            if((power < -1) && (GetGroupAvgCurrAPI(clunum) < -1))
            {
                chgCurr = ABS(0, GetGroupAvgCurrAPI(clunum));
            }
            /*充电功率过小取额定功率*/
            else
            {
                chgCurr = (u16)((u32)gGBmuGenPara_102[eBmuGenPara102_ChgPRatLim] * 10000 / sumVol);
            }
        }
        /*放电则充电取额定功率*/
        else if(eCURR_DHG == GetGChgDhgStateAPI(clunum))
        {
            chgCurr = (u16)((u32)gGBmuGenPara_102[eBmuGenPara102_ChgPRatLim] * 10000 / sumVol);

            /*放电功率取实际功率*/
            if((power > 1) && (GetGroupAvgCurrAPI(clunum) > 1))
            {
                dhgCurr = (u16)GetGroupAvgCurrAPI(clunum);
            }
            /*放电功率过小取额定功率*/
            else
            {
                dhgCurr = (u16)((u32)gGBmuGenPara_102[eBmuGenPara102_DhgPRatLim] * 10000 / sumVol);
            }
        }
        /*待机则充放电功率取额定功率*/
        else
        {
            chgCurr = (u16)((u32)gGBmuGenPara_102[eBmuGenPara102_ChgPRatLim] * 10000 / sumVol);
            dhgCurr = (u16)((u32)gGBmuGenPara_102[eBmuGenPara102_DhgPRatLim] * 10000 / sumVol);
        }

        calcTime = (u32)chgCap * 6 / chgCurr / 10;
        if((calcTime <= 1) && (GetGChgLimCurrAPI(clunum) > 0))          /*计算无剩余时间但仍可充电*/
        {
            if(clunum == 0)
            {
                gGStatistTime_59[eTime59_1CanChg] = 1;                 /*取剩余1min*/
            }
            else
            {
                gGStatistTime_59[eTime59_2CanChg] = 1;                 /*取剩余1min*/
            }
            
        }
        else
        {
            if(clunum == 0)
            {
                gGStatistTime_59[eTime59_1CanChg] = (gGStatistTime_59[eTime59_1CanChg] + calcTime) / 2;
            }
            else
            {
                gGStatistTime_59[eTime59_2CanChg] = (gGStatistTime_59[eTime59_2CanChg] + calcTime) / 2;
            }
        }

        calcTime = (u32)dhgCap * 6 / dhgCurr / 10;
        if((calcTime <= 1) && (GetGDhgLimCurrAPI(clunum) > 0))         /*计算无剩余时间但仍可充电*/
        {
            if(clunum == 0)
            {
                gGStatistTime_59[eTime59_1CanDhg] = 1;                 /*取剩余1min*/
            }
            else
            {
                gGStatistTime_59[eTime59_2CanDhg] = 1;                 /*取剩余1min*/
            }
        }
        else
        {
            if(clunum == 0)
            {
                gGStatistTime_59[eTime59_1CanDhg] = (gGStatistTime_59[eTime59_1CanDhg] + calcTime) / 2;
            }
            else
            {
                gGStatistTime_59[eTime59_2CanDhg] = (gGStatistTime_59[eTime59_2CanDhg] + calcTime) / 2;
            }
        }
    }
}
/*=============================================================================================
** @Name      : void GroupErrNumStatisTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组故障告警次数统计任务
** @The notes : 2s
**===========================================================================================*/
void GroupErrNumStatisTask(void)
{
    u16 rcdTime = 0;
    static u16 sMonWarNum[2] = {0};
    static u16 sMonErrNum[2] = {0};
    static u16 sHisWarNum[2] = {0};
    static u16 sHisErrNum[2] = {0};
    static u16 sHisChgState[2] = {0};

    if(gGStatistErr_60 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGStatistErr_60 null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        /*获取当前年月份*/
        rcdTime = GetSysDateClockAPI().year - 2000;
        rcdTime = (rcdTime << 8) | GetSysDateClockAPI().month;

        /*月份切换*/
        if(sHisMonErrTime[clunum] != rcdTime)
        {
            if(clunum == 0)
            {
                gGStatistErr_60[eErr60_1MonWarNum] = 0;
                gGStatistErr_60[eErr60_1MonErrNum] = 0;
            }
            else
            {
                gGStatistErr_60[eErr60_2MonWarNum] = 0;
                gGStatistErr_60[eErr60_2MonErrNum] = 0;
            }


            /*重新记录时间*/
            ParaWriteStoreMonErrTime(clunum, 1, &rcdTime, 1);
            sHisMonErrTime[clunum] = rcdTime;
        }
        if(clunum == 0)
        {
            /*初始上电*/
            if(0 == sHisWarNum[clunum])
            {
                sMonWarNum[clunum] = gGStatistErr_60[eErr60_1MonWarNum];
                sHisWarNum[clunum] = gGStatistErr_60[eErr60_1HisWarNum];

                /*月数目大于累计数目*/
                if(gGStatistErr_60[eErr60_1MonWarNum] > gGStatistErr_60[eErr60_1HisWarNum])
                {
                    gGStatistErr_60[eErr60_1MonWarNum] = gGStatistErr_60[eErr60_1HisWarNum];
                }
            }
            /*当月新增10条或充放电停止或故障/关机记录一次*/
            else if((ABS(sMonWarNum[clunum], gGStatistErr_60[eErr60_1MonWarNum]) >= HISERR_TIMES_WR_TIMES)
                || ((sMonWarNum[clunum] != gGStatistErr_60[eErr60_1MonWarNum])
                && (((eCURR_IDLE == GetGChgDhgStateAPI(clunum))
                && (eCURR_IDLE != sHisChgState[clunum]))
                || (eWORK_ERR == GetGWorkStateAPI(clunum))
                || (eWORK_OFF == GetGWorkStateAPI(clunum)))))
            {
                /*记录当月告警数目*/
                ParaWriteStoreHisErrNum(clunum, 1, &gGStatistErr_60[eErr60_1MonWarNum], 1);
                ParaWriteStoreHisErrNum(clunum, 3, &gGStatistErr_60[eErr60_1HisWarNum], 1);
                sMonWarNum[clunum] = gGStatistErr_60[eErr60_1MonWarNum];
            }
            /*累计10条或充放电停止或故障/关机记录一次*/
            else if((ABS(sHisWarNum[clunum], gGStatistErr_60[eErr60_1HisWarNum]) >= HISERR_TIMES_WR_TIMES)
                || ((sHisWarNum[clunum] != gGStatistErr_60[eErr60_1HisWarNum])
                && (((eCURR_IDLE == GetGChgDhgStateAPI(clunum))
                && (eCURR_IDLE != sHisChgState[clunum]))
                || (eWORK_ERR == GetGWorkStateAPI(clunum))
                || (eWORK_OFF == GetGWorkStateAPI(clunum)))))
            {
                /*记录累计告警数目*/
                ParaWriteStoreHisErrNum(clunum, 1, &gGStatistErr_60[eErr60_1MonWarNum], 1);
                ParaWriteStoreHisErrNum(clunum, 3, &gGStatistErr_60[eErr60_1HisWarNum], 1);
                sHisWarNum[clunum] = gGStatistErr_60[eErr60_1HisWarNum];
            }

            /*初始上电*/
            if(0 == sHisErrNum[clunum])
            {
                sMonErrNum[clunum] = gGStatistErr_60[eErr60_1MonErrNum];
                sHisErrNum[clunum] = gGStatistErr_60[eErr60_1HisErrNum];

                /*月数目大于累计数目*/
                if(gGStatistErr_60[eErr60_1MonErrNum] > gGStatistErr_60[eErr60_1HisErrNum])
                {
                    gGStatistErr_60[eErr60_1MonErrNum] = gGStatistErr_60[eErr60_1HisErrNum];
                }
            }
            /*单月10条或充放电停止或故障记录一次*/
            else if((ABS(sMonErrNum[clunum], gGStatistErr_60[eErr60_1MonErrNum]) >= HISERR_TIMES_WR_TIMES)
                || ((sMonErrNum[clunum] != gGStatistErr_60[eErr60_1MonErrNum])
                && (((eCURR_IDLE == GetGChgDhgStateAPI(clunum))
                && (eCURR_IDLE != sHisChgState[clunum]))
                || (eWORK_ERR == GetGWorkStateAPI(clunum))
                || (eWORK_OFF == GetGWorkStateAPI(clunum)))))
            {
                /*记录累计故障数目*/
                ParaWriteStoreHisErrNum(clunum, 2, &gGStatistErr_60[eErr60_1MonErrNum], 1);
                ParaWriteStoreHisErrNum(clunum, 4, &gGStatistErr_60[eErr60_1HisErrNum], 1);
                sMonErrNum[clunum] = gGStatistErr_60[eErr60_1MonErrNum];
            }
            /*累计10条或充放电停止或故障记录一次*/
            else if((ABS(sHisErrNum[clunum], gGStatistErr_60[eErr60_1HisErrNum]) >= HISERR_TIMES_WR_TIMES)
                || ((sHisErrNum[clunum] != gGStatistErr_60[eErr60_1HisErrNum])
                && (((eCURR_IDLE == GetGChgDhgStateAPI(clunum))
                && (eCURR_IDLE != sHisChgState[clunum]))
                || (eWORK_ERR == GetGWorkStateAPI(clunum))
                || (eWORK_OFF == GetGWorkStateAPI(clunum)))))
            {
                /*记录累计故障数目*/
                ParaWriteStoreHisErrNum(clunum, 2, &gGStatistErr_60[eErr60_1MonErrNum], 1);
                ParaWriteStoreHisErrNum(clunum, 4, &gGStatistErr_60[eErr60_1HisErrNum], 1);
                sHisErrNum[clunum] = gGStatistErr_60[eErr60_1HisErrNum];
            }
        }
        else
        {
            /*初始上电*/
            if(0 == sHisWarNum[clunum])
            {
                sMonWarNum[clunum] = gGStatistErr_60[eErr60_2MonWarNum];
                sHisWarNum[clunum] = gGStatistErr_60[eErr60_2HisWarNum];

                /*月数目大于累计数目*/
                if(gGStatistErr_60[eErr60_2MonWarNum] > gGStatistErr_60[eErr60_2HisWarNum])
                {
                    gGStatistErr_60[eErr60_2MonWarNum] = gGStatistErr_60[eErr60_2HisWarNum];
                }
            }
            /*当月新增10条或充放电停止或故障/关机记录一次*/
            else if((ABS(sMonWarNum[clunum], gGStatistErr_60[eErr60_2MonWarNum]) >= HISERR_TIMES_WR_TIMES)
                || ((sMonWarNum[clunum] != gGStatistErr_60[eErr60_2MonWarNum])
                && (((eCURR_IDLE == GetGChgDhgStateAPI(clunum))
                && (eCURR_IDLE != sHisChgState[clunum]))
                || (eWORK_ERR == GetGWorkStateAPI(clunum))
                || (eWORK_OFF == GetGWorkStateAPI(clunum)))))
            {
                /*记录当月告警数目*/
                ParaWriteStoreHisErrNum(clunum, 1, &gGStatistErr_60[eErr60_2MonWarNum], 1);
                ParaWriteStoreHisErrNum(clunum, 3, &gGStatistErr_60[eErr60_2HisWarNum], 1);
                sMonWarNum[clunum] = gGStatistErr_60[eErr60_2MonWarNum];
            }
            /*累计10条或充放电停止或故障/关机记录一次*/
            else if((ABS(sHisWarNum[clunum], gGStatistErr_60[eErr60_2HisWarNum]) >= HISERR_TIMES_WR_TIMES)
                || ((sHisWarNum[clunum] != gGStatistErr_60[eErr60_2HisWarNum])
                && (((eCURR_IDLE == GetGChgDhgStateAPI(clunum))
                && (eCURR_IDLE != sHisChgState[clunum]))
                || (eWORK_ERR == GetGWorkStateAPI(clunum))
                || (eWORK_OFF == GetGWorkStateAPI(clunum)))))
            {
                /*记录累计告警数目*/
                ParaWriteStoreHisErrNum(clunum, 1, &gGStatistErr_60[eErr60_2MonWarNum], 1);
                ParaWriteStoreHisErrNum(clunum, 3, &gGStatistErr_60[eErr60_2HisWarNum], 1);
                sHisWarNum[clunum] = gGStatistErr_60[eErr60_2HisWarNum];
            }

            /*初始上电*/
            if(0 == sHisErrNum[clunum])
            {
                sMonErrNum[clunum] = gGStatistErr_60[eErr60_2MonErrNum];
                sHisErrNum[clunum] = gGStatistErr_60[eErr60_2HisErrNum];

                /*月数目大于累计数目*/
                if(gGStatistErr_60[eErr60_2MonErrNum] > gGStatistErr_60[eErr60_2HisErrNum])
                {
                    gGStatistErr_60[eErr60_2MonErrNum] = gGStatistErr_60[eErr60_2HisErrNum];
                }
            }
            /*单月10条或充放电停止或故障记录一次*/
            else if((ABS(sMonErrNum[clunum], gGStatistErr_60[eErr60_2MonErrNum]) >= HISERR_TIMES_WR_TIMES)
                || ((sMonErrNum[clunum] != gGStatistErr_60[eErr60_2MonErrNum])
                && (((eCURR_IDLE == GetGChgDhgStateAPI(clunum))
                && (eCURR_IDLE != sHisChgState[clunum]))
                || (eWORK_ERR == GetGWorkStateAPI(clunum))
                || (eWORK_OFF == GetGWorkStateAPI(clunum)))))
            {
                /*记录累计故障数目*/
                ParaWriteStoreHisErrNum(clunum, 2, &gGStatistErr_60[eErr60_2MonErrNum], 1);
                ParaWriteStoreHisErrNum(clunum, 4, &gGStatistErr_60[eErr60_2HisErrNum], 1);
                sMonErrNum[clunum] = gGStatistErr_60[eErr60_2MonErrNum];
            }
            /*累计10条或充放电停止或故障记录一次*/
            else if((ABS(sHisErrNum[clunum], gGStatistErr_60[eErr60_2HisErrNum]) >= HISERR_TIMES_WR_TIMES)
                || ((sHisErrNum[clunum] != gGStatistErr_60[eErr60_2HisErrNum])
                && (((eCURR_IDLE == GetGChgDhgStateAPI(clunum))
                && (eCURR_IDLE != sHisChgState[clunum]))
                || (eWORK_ERR == GetGWorkStateAPI(clunum))
                || (eWORK_OFF == GetGWorkStateAPI(clunum)))))
            {
                /*记录累计故障数目*/
                ParaWriteStoreHisErrNum(clunum, 2, &gGStatistErr_60[eErr60_2MonErrNum], 1);
                ParaWriteStoreHisErrNum(clunum, 4, &gGStatistErr_60[eErr60_2HisErrNum], 1);
                sHisErrNum[clunum] = gGStatistErr_60[eErr60_2HisErrNum];
            }
        }


        sHisChgState[clunum] = GetGChgDhgStateAPI(clunum);
    }
}

/*=============================================================================================
** @Name      : void GroupHisErrNumAddHandle(u8 clunum, u8 type)
** @Input     : clunum:簇编号 type:故障类型 0:告警 1:故障
** @Output    : void
** @Function  : 电池组故障告警累加处理
** @The notes : 确定是故障或告警后调用
**===========================================================================================*/
void GroupHisErrNumAddHandle(u8 clunum, u8 type)
{

    if(clunum >= 2)
    {
        return;
    }
    if(gGStatistErr_60 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGStatistErr_60 null!!!\r\n", __func__);
        return;
    }
    if(clunum == 0)
    {
        /*告警*/
        if(0 == type)
        {
            gGStatistErr_60[eErr60_1MonWarNum]++;
            gGStatistErr_60[eErr60_1HisWarNum]++;
        }
        /*故障*/
        else if(1 == type)
        {
            gGStatistErr_60[eErr60_1MonErrNum]++;
            gGStatistErr_60[eErr60_1HisErrNum]++;
        }
    }
    else
    {
        /*告警*/
        if(0 == type)
        {
            gGStatistErr_60[eErr60_2MonWarNum]++;
            gGStatistErr_60[eErr60_2HisWarNum]++;
        }
        /*故障*/
        else if(1 == type)
        {
            gGStatistErr_60[eErr60_2MonErrNum]++;
            gGStatistErr_60[eErr60_2HisErrNum]++;
        }
    }
}

/*=============================================================================================
** @Name      : void ClrGroupChgDhgTimesAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 清除电池组充放电次数
** @The notes :
**===========================================================================================*/
void ClrGroupChgDhgTimesAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return;
    }
    if(gGStatistTime_59 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGStatistTime_59 null!!!\r\n", __func__);
        return;
    }
    if(clunum == 0)
    {
        gGStatistTime_59[eTime59_1ChgCycle] = 1;
        ParaWriteStoreChgDhgTimes(clunum, 1, &gGStatistTime_59[eTime59_1ChgCycle], 1);
    }
    else
    {
        gGStatistTime_59[eTime59_2ChgCycle] = 1;
        ParaWriteStoreChgDhgTimes(clunum, 1, &gGStatistTime_59[eTime59_2ChgCycle], 1);
    }
}
/*=============================================================================================
** @Name      : void ClrGroupHisErrNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 清除电池组累计故障告警数目
** @The notes : 当月值和累计值
**===========================================================================================*/
void ClrGroupHisErrNumAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return;
    }
    if(gGStatistErr_60 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGStatistErr_60 null!!!\r\n", __func__);
        return;
    }

    if(clunum == 0)
    {
        gGStatistErr_60[eErr60_1MonWarNum] = 0;
        gGStatistErr_60[eErr60_1MonErrNum] = 0;
        gGStatistErr_60[eErr60_1HisWarNum] = 0;
        gGStatistErr_60[eErr60_1HisErrNum] = 0;
        ParaWriteStoreHisErrNum(clunum, 1, &gGStatistErr_60[eErr60_1MonWarNum], 4);
    }
    else
    {
        gGStatistErr_60[eErr60_2MonWarNum] = 0;
        gGStatistErr_60[eErr60_2MonErrNum] = 0;
        gGStatistErr_60[eErr60_2HisWarNum] = 0;
        gGStatistErr_60[eErr60_2HisErrNum] = 0;
        ParaWriteStoreHisErrNum(clunum, 1, &gGStatistErr_60[eErr60_2MonWarNum], 4);
    }
}
