/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : SocSoeSync.c
** @Founder      : ZSY342
** @Date         : 2019.10.30
** @Function     : SOC/SOE组间同步模块
** @Instructions : 多组并机时有效,确保组间SOC/SOE差距不大
**===========================================================================================*/

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "PublicPara.h"
#include "GroupPara.h"
#include "GroupInfo.h"
#include "SocInterface.h"
#include "SocUserAPI.h"
#include "SocSoeSync.h"
#include "EnerInfoCalc.h"
#include "EnerInterface.h"
#include "CapInfoCalc.h"

#if(1 == SOCSOE_GROUP_SYNC_EN)
/*=============================================================================================
** @Global variable definition
**===========================================================================================*/
//static u8 sSocRunGId[PILE_MAX_G_NUM-1] = {0};/*SOC计算正在运行的电池组号(不包括自身)*/
//static u8 sSoeRunGId[PILE_MAX_G_NUM-1] = {0};/*SOE计算正在运行的电池组号(不包括自身)*/
//static u8 sGRunChgDhgFinState = 0;           /*运行电池组充放电完成状态(不包括自身)*/

/*接收其他组信息(不包括自身)*/
//static u16 sGInfoRcvState = 0;
//static u8 sGProState[PILE_MAX_G_NUM-1] = {0};
//static u8 sGWorkState[PILE_MAX_G_NUM-1] = {0};
//static u8 sGChgState[PILE_MAX_G_NUM-1] = {0};
//static u16 sGNowSoc[PILE_MAX_G_NUM-1] = {0};
//static u16 sGNowSoe[PILE_MAX_G_NUM-1] = {0};
//static u16 sGCellMaxV[PILE_MAX_G_NUM-1] = {0};
//static u16 sGCellMinV[PILE_MAX_G_NUM-1] = {0};

extern u16 gSysGenParaRO_100[];              /*系统通用参数设置*/
extern u16 gGBmuGenPara_102[];               /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	         /*主控高级参数值*/
/*=============================================================================================
** @Name      : u16 GetGRealSocMilliAPI(u8 clunum)
** @Input     : void
** @Output    : 电池组真实SOC值(单位:千分之一)
** @Function  : 获取电池组真实SOC值
** @The notes :
**===========================================================================================*/
/*=============================================================================================
** @Static function declaration
**===========================================================================================*/

/*=============================================================================================
** @Interface function definition
**===========================================================================================*/
/*=============================================================================================
** @Name      :  void SocSoeTogetherSyncTask(void)
** @Input     :  void
** @Output    :  void
** @Function  :  SOC/SOE相互同步修正
** @The notes :  500ms,确保SOE变化跟随SOC同步(SOE全程均不能大于SOC)
**===========================================================================================*/
void SocSoeTogetherSyncTask(void)
{
    u16 nowSoc = 0;
    u16 nowSoe = 0;

    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        /*单体信息就绪*/
        if(1 == EnerGetGroupBattReadyHook(clunum))
        {
            nowSoc = GetGRealSocMilliAPI(clunum);
            nowSoe = GetGRealSoeMilliAPI(clunum);

            /*充电状态*/
            if(eCURR_CHG == GetGChgDhgStateAPI(clunum))
            {
                /*任何情况SOE不能大于SOC+2.5%*/
                if(nowSoe >= (nowSoc + 25))
                {
                    //修正SOE只高于SOC+1.8%
                    CorrGNowEnerBySoeAPI(clunum, (nowSoc + 18));
                }
                /*任何情况SOE不能低于SOC-2.5%*/
                else if((nowSoe + 25) <= nowSoc)
                {
                    //修正SOE低于SOC-1.5%
                    CorrGNowEnerBySoeAPI(clunum, (nowSoc - 15));
                }
                /*SOC≤5%时SOE不能大于(SOC+2.5%)*/
                else if(nowSoc <= 50)
                {
                    if(nowSoe >= (nowSoc + 25))
                    {
                        //修正SOE低于SOC+2.2%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc + 22));
                    }
                }
                /*5%<SOC≤10%时SOE不能大于(SOC+2.2%)*/
                else if(nowSoc <= 100)
                {
                    if(nowSoe >= (nowSoc + 22))
                    {
                        //修正SOE低于SOC+1.9%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc + 19));
                    }
                }
                /*10%<SOC≤15%时SOE不能大于(SOC+1.6%)*/
                else if(nowSoc <= 150)
                {
                    if(nowSoe >= (nowSoc + 16))
                    {
                        //修正SOE低于SOC+1.3%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc + 13));
                    }
                }
                /*15%<SOC≤20%时SOE不能大于(SOC+1.3%)*/
                else if(nowSoc <= 200)
                {
                    if(nowSoe >= (nowSoc + 13))
                    {
                        //修正SOE低于SOC+1.0%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc + 10));
                    }
                }
                /*20%<SOC≤25%时SOE不能大于(SOC+1.0%)*/
                else if(nowSoc <= 250)
                {
                    if(nowSoe >= (nowSoc + 10))
                    {
                        //修正SOE低于SOC+0.7%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc + 7));
                    }
                }
                /*25%<SOC≤30%时SOE不能大于(SOC+0.7%)*/
                else if(nowSoc <= 300)
                {
                    if(nowSoe >= (nowSoc + 7))
                    {
                        //修正SOE低于SOC+0.4%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc + 4));
                    }
                }
                /*30%<SOC≤35%时SOE不能大于SOC+0.4%*/
                else if(nowSoc <= 350)
                {
                    if(nowSoe >= (nowSoc + 4))
                    {
                        //修正SOE低于SOC+0.1%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc + 1));
                    }
                }
                /*35%<SOC≤40%时SOE不能大于SOC+0.1%*/
                else if(nowSoc <= 400)
                {
                    if(nowSoe >= (nowSoc + 1))
                    {
                        //修正SOE低于SOC-0.2%
                        CorrGNowEnerBySoeAPI(clunum, nowSoc - 2);
                    }
                }
                /*40%<SOC≤45%时SOE不能大于SOC-0.2%*/
                else if(nowSoc <= 450)
                {
                    if(nowSoe >= (nowSoc - 2))
                    {
                        //修正SOE高于SOC-0.5%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc - 5));
                    }
                }
                /*45%<SOC≤50%时SOE不能大于SOC-0.5%*/
                else if(nowSoc <= 500)
                {
                    if(nowSoe >= (nowSoc - 5))
                    {
                        //修正SOE高于SOC-0.8%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc - 8));
                    }
                }
                /*50%<SOC≤55%时SOE不能大于SOC-0.8%*/
                else if(nowSoc <= 550)
                {
                    if(nowSoe >= (nowSoc - 8))
                    {
                        //修正SOE高于SOC-1.1%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc - 11));
                    }
                }
                /*55%<SOC≤60%时SOE不能大于SOC-1.1%*/
                else if(nowSoc <= 600)
                {
                    if(nowSoe >= (nowSoc - 11))
                    {
                        //修正SOE高于SOC-1.4%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc - 14));
                    }
                }
                /*60%<SOC≤65%时SOE不能大于SOC-1.1%*/
                else if(nowSoc <= 650)
                {
                    if(nowSoe >= (nowSoc - 11))
                    {
                        //修正SOE高于SOC-1.4%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc - 14));
                    }
                }
                /*65%<SOC≤70%时SOE不能大于SOC-0.8%*/
                else if(nowSoc <= 700)
                {
                    if(nowSoe >= (nowSoc - 8))
                    {
                        //修正SOE高于SOC-1.1%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc - 11));
                    }
                }
                /*70%<SOC≤75%时SOE不能大于SOC-0.5%*/
                else if(nowSoc <= 750)
                {
                    if(nowSoe >= (nowSoc - 5))
                    {
                        //修正SOE高于SOC-0.8%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc - 8));
                    }
                }
                /*75%<SOC≤80%时SOE不能大于SOC-0.2%*/
                else if(nowSoc <= 800)
                {
                    if(nowSoe >= (nowSoc - 2))
                    {
                        //修正SOE高于SOC-0.5%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc - 5));
                    }
                }
                /*80%<SOC≤85%时SOE不能大于SOC+0.1%*/
                else if(nowSoc <= 850)
                {
                    if(nowSoe >= (nowSoc + 1))
                    {
                        //修正SOE高于SOC-0.2%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc - 2));
                    }
                }
                /*85%<SOC≤90%时SOE不能大于SOC+0.4%*/
                else if(nowSoc <= 900)
                {
                    if(nowSoe >= (nowSoc + 4))
                    {
                        //修正SOE高于SOC+0.4%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc + 1));
                    }
                }
                /*90%<SOC≤95%时SOE不能大于SOC+0.7%*/
                else if(nowSoc <= 950)
                {
                    if(nowSoe >= (nowSoc + 7))
                    {
                        //修正SOE高于SOC+0.4%
                        CorrGNowEnerBySoeAPI(clunum, (nowSoc + 4));
                    }
                }
            }
    //        /*放电状态*/
    //        else if(eCURR_DHG == GetGChgDhgStateAPI())
    //        {
    //
    //        }
            else
            {
                /*其他情况SOE不能大于SOC+3%*/
                if(nowSoe >= (nowSoc + 30))
                {
                    //修正SOE只高于SOC+2%
                    CorrGNowEnerBySoeAPI(clunum, (nowSoc + 20));
                }
                /*任何情况SOE不能低于SOC-3%*/
                else if((nowSoe + 30) <= nowSoc)
                {
                    //修正SOE低于SOC-2%
                    CorrGNowEnerBySoeAPI(clunum, (nowSoc - 20));
                }

            }

    //        /*55%≤SOC≤60%时SOE不能大于(SOC-2.5%)*/
    //        if((nowSoc >= 550) && (nowSoc <= 600) && (nowSoe >= (nowSoc - 25)))
    //        {
    //            //修正SOE低于SOC2.8%
    //            CorrGNowEnerBySoeAPI((nowSoc - 28));
    //        }
    //        /*50%≤SOC≤65%时SOE不能大于(SOC-2.1%)*/
    //        else if((nowSoc >= 500) && (nowSoc <= 650) && (nowSoe >= (nowSoc - 21)))
    //        {
    //            //修正SOE低于SOC2.5%
    //            CorrGNowEnerBySoeAPI((nowSoc - 2.5));
    //        }
    //        /*45%≤SOC≤70%时SOE不能大于(SOC-1.8%)*/
    //        else if((nowSoc >= 450) && (nowSoc <= 700) && (nowSoe >= (nowSoc - 18)))
    //        {
    //            //修正SOE低于SOC2.1%
    //            CorrGNowEnerBySoeAPI((nowSoc - 21));
    //        }
    //        /*40%≤SOC≤75%时SOE不能大于(SOC-1.5%)*/
    //        else if((nowSoc >= 400) && (nowSoc <= 750) && (nowSoe >= (nowSoc - 15)))
    //        {
    //            //修正SOE低于SOC1.8%
    //            CorrGNowEnerBySoeAPI((nowSoc - 18));
    //        }
    //        /*35%≤SOC≤79%时SOE不能大于(SOC-1.0%)*/
    //        else if((nowSoc >= 350) && (nowSoc <= 790) && (nowSoe >= (nowSoc - 10)))
    //        {
    //            //修正SOE低于SOC1.5%
    //            CorrGNowEnerBySoeAPI((nowSoc - 15));
    //        }
    //        /*30%≤SOC≤83%时SOE不能大于(SOC-0.5%)*/
    //        else if((nowSoc >= 300) && (nowSoc <= 830) && (nowSoe >= (nowSoc - 5)))
    //        {
    //            //修正SOE低于SOC1.0%
    //            CorrGNowEnerBySoeAPI((nowSoc - 10));
    //        }
    //        /*25%≤SOC≤87%时SOE不能大于SOC*/
    //        else if((nowSoc >= 250) && (nowSoc <= 870) && (nowSoe >= nowSoc))
    //        {
    //            //修正SOE低于SOC0.5%
    //            CorrGNowEnerBySoeAPI((nowSoc - 5));
    //        }
    //        /*20%≤SOC≤90%时SOE不能大于SOC+0.5%*/
    //        else if((nowSoc >= 200) && (nowSoc <= 900) && (nowSoe >= (nowSoc + 5)))
    //        {
    //            //修正SOE与SOC一致
    //            CorrGNowEnerBySoeAPI(nowSoc);
    //        }
    //        /*15%≤SOC≤93%时SOE不能大于SOC+1%*/
    //        else if((nowSoc >= 150) && (nowSoc <= 930) && (nowSoe >= (nowSoc + 10)))
    //        {
    //            //修正SOE高于SOC0.5%
    //            CorrGNowEnerBySoeAPI((nowSoc + 5));
    //        }
    //        /*10%≤SOC≤96%时SOE不能大于SOC+1.5%*/
    //        else if((nowSoc >= 100) && (nowSoc <= 960) && (nowSoe >= (nowSoc + 15)))
    //        {
    //            //修正SOE高于SOC1.0%
    //            CorrGNowEnerBySoeAPI((nowSoc + 10));
    //        }
    //        /*5%≤SOC≤98%时SOE不能大于SOC+2%*/
    //        else if((nowSoc >= 50) && (nowSoc <= 980) && (nowSoe >= (nowSoc + 20)))
    //        {
    //            //修正SOE高于SOC1.5%
    //            CorrGNowEnerBySoeAPI((nowSoc + 15));
    //        }
    //        /*其他情况SOE不能大于SOC+3%*/
    //        else if(nowSoe >= (nowSoc + 30))
    //        {
    //            //修正SOE只高于SOC2%
    //            CorrGNowEnerBySoeAPI((nowSoc + 20));
    //        }
    //        /*任何情况SOE不能低于SOC-5%*/
    //        else if((nowSoe + 50) <= nowSoc)
    //        {
    //            //修正SOE低于SOC8%
    //            CorrGNowEnerBySoeAPI((nowSoc - 80));
    //        }
        }
    }

}
#if(0)  //组间同步功能禁用
/*=============================================================================================
** @Name      :  void SocSoeGroupSyncTask(void)
** @Input     :  void
** @Output    :  void
** @Function  :  SOC/SOE组间同步修正
** @The notes :  500ms,确保多组并机间误差3%内
**===========================================================================================*/
void SocSoeGroupSyncTask(void)
{
    u8 i = 0;
    u8 j = 0;
    u8 k = 0;
    u8 index = 0;
    u8 samNum = 0;
    u16 maxSoc = 0;
    u16 aimSoc = 0xffff;
    u16 maxSoe = 0;
    u16 aimSoe = 0xffff;
    u8 chgState = 0;
    static u8 sState = 0;
    static u8 sRunNum = 0;

    /*单体信息未就绪*/
    /*自身未运行*/
    /*只有1组*/
    if((0 == EnerGetGroupBattReadyHook())
        || (eWORK_RUN != GetGWorkStateAPI())
        || (gSysGenParaRO_100[eSGenPara100_AllGNum] <= 1))
    {
        sState = 0;
        return;
    }

    switch(sState)
    {
        //取运行的电池组号
        case 0:
            sRunNum = 0;
            for(i = 0; i < (gSysGenParaRO_100[eSGenPara100_AllGNum] - 1); i++)
            {
                //电池组通信正常且运行(不包括自身)
                if((eWORK_RUN == sGWorkState[i]) && (1 == BitGet(sGInfoRcvState, i)))
                {
                    //电池组空闲
                    if(eCURR_IDLE == sGChgState[i])
                    {
                        //电池组充满
                        if(ePRO_FULL == sGProState[i])
                        {
                            //记录电池组充满
                            chgState = 1;
                        }
                        //电池组放空
                        else if(ePRO_EMPTY == sGProState[i])
                        {
                            //记录电池组放空
                            chgState = 2;
                        }
                    }

                    //记录运行组号
                    sSocRunGId[sRunNum] = i;
                    sSoeRunGId[sRunNum] = i;
                    sRunNum++;
                }
            }

            //记录充放电截止状态
            sGRunChgDhgFinState = chgState;

            //无其他组运行
            if(0 == sRunNum)
            {
                //无法修正
                sState = 10;
            }
            //只有一组和自身运行
            else if(1 == sRunNum)
            {
                //直接判断SOC/SOE
                sState = 2;
            }
            //多组运行
            else
            {
                //SOC/SOE排序
                sState = 1;
            }
            break;

        //SOC/SOE排序
        case 1:
            maxSoc = sGNowSoc[sSocRunGId[0]];
            maxSoe = sGNowSoe[sSoeRunGId[0]];

            //降序排列运行组SOC/SOE
            for(i = 1; i < sRunNum; i++)
            {
                for(j = i; j < sRunNum; j++)
                {
                    //将最大的调换到前面(i)位置
                    if(sGNowSoc[sSocRunGId[j]] > maxSoc)
                    {
                        maxSoc = sGNowSoc[sSocRunGId[j]];
                        k = sSocRunGId[i];
                        sSocRunGId[i] = sSocRunGId[j];
                        sSocRunGId[j] = k;
                    }

                    //将最大的调换到前面(i)位置
                    if(sGNowSoe[sSoeRunGId[j]] > maxSoc)
                    {
                        maxSoc = sGNowSoe[sSoeRunGId[j]];
                        k = sSoeRunGId[i];
                        sSoeRunGId[i] = sSoeRunGId[j];
                        sSoeRunGId[j] = k;
                    }
                }
            }
            sState = 2;
            break;

        //SOC修正判断
        case 2:
            //无其他组运行
            if(0 == sRunNum)
            {
                //无需修正
                aimSoc = 0xffff;
            }
            //只有一组和自身运行
            else if(1 == sRunNum)
            {
                //两组SOC误差未超过3%
                if(ABS(GetGRealSocMilliAPI(), sGNowSoc[sSocRunGId[0]]) <= G_ALLOW_DIFF_SOC)
                {
                    //无需修正
                    aimSoc = 0xffff;
                }
                //正在放电(向其他最小SOC组看齐)
                else if(eCURR_DHG == GetGChgDhgStateAPI())
                {
                    //自身电压比其他组小而SOC比其他组大
                    //自身电压比其他组大而SOC比其他组小
                    if(((GetGCellMinVoltAPI() < sGCellMinV[sSocRunGId[0]])
                        && (GetGRealSocMilliAPI() > sGNowSoc[sSocRunGId[0]]))
                        || ((GetGCellMinVoltAPI() > sGCellMinV[sSocRunGId[0]])
                        && (GetGRealSocMilliAPI() < sGNowSoc[sSocRunGId[0]])))
                    {
                        //修正为两组平均值
                        aimSoc = (GetGRealSocMilliAPI() + sGNowSoc[sSocRunGId[0]]) / 2;
                    }
                    //自身电压比其他组大且SOC比其他组大
                    else if((GetGCellMinVoltAPI() >= sGCellMinV[sSocRunGId[0]])
                        && (GetGRealSocMilliAPI() > sGNowSoc[sSocRunGId[0]]))
                    {
                        //电压差异在允许范围内30mV
                        if(ABS(GetGCellMinVoltAPI(), sGCellMinV[sSocRunGId[0]]) <= G_ALLOW_DIFF_VOLT)
                        {
                            //修正为与其他组误差只有2.5%(SOC往低修正)
                            aimSoc = sGNowSoc[sSocRunGId[0]] + G_ALLOW_DIFF_SOC - 5;
                        }
                        //电压差异在允许范围内60mV
                        else if(ABS(GetGCellMinVoltAPI(), sGCellMinV[sSocRunGId[0]]) <= (G_ALLOW_DIFF_VOLT * 2))
                        {
                            //修正为与其他组误差只有5%(SOC往低修正)
                            aimSoc = sGNowSoc[sSocRunGId[0]] + (G_ALLOW_DIFF_SOC * 2) - 10;
                        }
                        //电压差异过大
                        else
                        {
                            //修正为与其他组误差只有7.5%(SOC往低修正)
                            aimSoc = sGNowSoc[sSocRunGId[0]] + (G_ALLOW_DIFF_SOC * 3) - 15;
                        }
                    }
                    //自身电压比其他组低
                    else
                    {
                        //无需修正
                        aimSoc = 0xffff;
                    }
                }
                //正在充电(向其他最大SOC组看齐)
                else if(eCURR_CHG == GetGChgDhgStateAPI())
                {
                    //自身电压比其他组小而SOC比其他组大
                    //自身电压比其他组大而SOC比其他组小
                    if(((GetGCellMaxVoltAPI() < sGCellMaxV[sSocRunGId[0]])
                        && (GetGRealSocMilliAPI() > sGNowSoc[sSocRunGId[0]]))
                        || ((GetGCellMaxVoltAPI() > sGCellMaxV[sSocRunGId[0]])
                        && (GetGRealSocMilliAPI() < sGNowSoc[sSocRunGId[0]])))
                    {
                        //修正为两组平均值
                        aimSoc = (GetGRealSocMilliAPI() + sGNowSoc[sSocRunGId[0]]) / 2;
                    }
                    //自身电压比其他组小且SOC比其他组小
                    else if((GetGCellMaxVoltAPI() <= sGCellMaxV[sSocRunGId[0]])
                        && (GetGRealSocMilliAPI() < sGNowSoc[sSocRunGId[0]]))
                    {
                        //电压差异在允许范围内30mV
                        if(ABS(GetGCellMaxVoltAPI(), sGCellMaxV[sSocRunGId[0]]) <= G_ALLOW_DIFF_VOLT)
                        {
                            //修正为与其他组误差只有2.5%(SOC往高修正)
                            aimSoc = sGNowSoc[sSocRunGId[0]] - G_ALLOW_DIFF_SOC + 5;
                        }
                        //电压差异在允许范围内60mV
                        else if(ABS(GetGCellMaxVoltAPI(), sGCellMaxV[sSocRunGId[0]]) <= (G_ALLOW_DIFF_VOLT * 2))
                        {
                            //修正为与其他组误差只有5%(SOC往高修正)
                            aimSoc = sGNowSoc[sSocRunGId[0]] - (G_ALLOW_DIFF_SOC * 2) + 10;
                        }
                        //电压差异过大
                        else
                        {
                            //修正为与其他组误差只有7.5%(SOC往高修正)
                            aimSoc = sGNowSoc[sSocRunGId[0]] - (G_ALLOW_DIFF_SOC * 3) + 15;
                        }
                    }
                    //自身电压比其他组低
                    else
                    {
                        //无需修正
                        aimSoc = 0xffff;
                    }
                }
                //系统待机
                else
                {
                    //自身电压比其他组小而SOC比其他组大
                    //自身电压比其他组大而SOC比其他组小
                    if(((GetGCellMaxVoltAPI() < sGCellMaxV[sSocRunGId[0]])
                        && (GetGCellMinVoltAPI() < sGCellMinV[sSocRunGId[0]])
                        && (GetGRealSocMilliAPI() > sGNowSoc[sSocRunGId[0]]))
                        || ((GetGCellMaxVoltAPI() > sGCellMaxV[sSocRunGId[0]])
                        && (GetGCellMinVoltAPI() > sGCellMinV[sSocRunGId[0]])
                        && (GetGRealSocMilliAPI() < sGNowSoc[sSocRunGId[0]])))
                    {
                        //修正为两组平均值
                        aimSoc = (GetGRealSocMilliAPI() + sGNowSoc[sSocRunGId[0]]) / 2;
                    }
                    //其他运行组充电截止自身SOC小
                    else if((1 == GetSocSoeGSyncChgDhgFinState())
                        && (GetGRealSocMilliAPI() < sGNowSoc[sSocRunGId[0]]))
                    {
                        //修正与另一组SOC一致
                        aimSoc = sGNowSoc[sSocRunGId[0]];
                    }
                    //其他运行组放电截止自身SOC大
                    else if((2 == GetSocSoeGSyncChgDhgFinState())
                        && (GetGRealSocMilliAPI() > sGNowSoc[sSocRunGId[0]]))
                    {
                        //修正与另一组SOC一致
                        aimSoc = sGNowSoc[sSocRunGId[0]];
                    }
                    //自身电压比其他组电压相当
                    else
                    {
                        //无需修正
                        aimSoc = 0xffff;
                    }
                }
            }
            //多组运行(sRunNum>=2)
            else
            {
                //其他组SOC误差未超过3%
                if(ABS(sGNowSoc[sSocRunGId[0]], sGNowSoc[sSocRunGId[sRunNum - 1]]) <= G_ALLOW_DIFF_SOC)
                {
                    //自身SOC比其他最大组还大
                    if(GetGRealSocMilliAPI() >= (sGNowSoc[sSocRunGId[0]] + 10))
                    {
                        //修正与最大组SOC一致
                        aimSoc = sGNowSoc[sSocRunGId[0]];
                    }
                    //自身SOC比其他最小组还小
                    else if((GetGRealSocMilliAPI() + 10) <= sGNowSoc[sSocRunGId[sRunNum - 1]])
                    {
                        //修正与最小组SOC一致
                        aimSoc = sGNowSoc[sSocRunGId[sRunNum - 1]];
                    }
                    //其他运行组充电截止自身SOC小
                    else if((1 == GetSocSoeGSyncChgDhgFinState())
                        && (GetGRealSocMilliAPI() < sGNowSoc[sSocRunGId[0]]))
                    {
                        //修正与最大组SOC一致
                        aimSoc = sGNowSoc[sSocRunGId[0]];
                    }
                    //其他运行组放电截止自身SOC大
                    else if((2 == GetSocSoeGSyncChgDhgFinState())
                        && (GetGRealSocMilliAPI() > sGNowSoc[sSocRunGId[sRunNum - 1]]))
                    {
                        //修正与最小组SOC一致
                        aimSoc = sGNowSoc[sSocRunGId[sRunNum - 1]];
                    }
                    //自身SOC处于其他组之间
                    else
                    {
                        //无需修正
                        aimSoc = 0xffff;
                    }
                }
                //其他组SOC误差超过3%
                else
                {
                    samNum = 0;
                    //查找最多SOC一致的组数
                    for(i = 0; i < sRunNum; i++)
                    {
                        k = 0;
                        maxSoc = sGNowSoc[sSocRunGId[i]];
                        //j越大SOC越小
                        for(j = i; j < sRunNum; j++)
                        {
                            //与起始(最大)值误差在3%内
                            if(maxSoc <= (sGNowSoc[sSocRunGId[j]] + G_ALLOW_DIFF_SOC))
                            {
                                k++;
                            }
                        }

                        //本次找到相同SOC的组数较大
                        if(k > samNum)
                        {
                            //记录起始索引和组数
                            index = i;
                            samNum = k;
                        }
                    }

                    //其他组无相同SOC
                    if(samNum <= 1)
                    {
                        //正在放电(向其他最小SOC组看齐)
                        if(eCURR_DHG == GetGChgDhgStateAPI())
                        {
                            //自身电压比其他组小而SOC比其他组大
                            //自身电压比其他组大而SOC比其他组小
                            if(((GetGCellMinVoltAPI() < sGCellMinV[sSocRunGId[sRunNum - 1]])
                                && (GetGRealSocMilliAPI() > sGNowSoc[sSocRunGId[sRunNum - 1]]))
                                || ((GetGCellMinVoltAPI() > sGCellMinV[sSocRunGId[sRunNum - 1]])
                                && (GetGRealSocMilliAPI() < sGNowSoc[sSocRunGId[sRunNum - 1]])))
                            {
                                //修正为两组平均值
                                aimSoc = (GetGRealSocMilliAPI() + sGNowSoc[sSocRunGId[sRunNum - 1]]) / 2;
                            }
                            //自身电压比其他组大且SOC比其他组大
                            else if((GetGCellMinVoltAPI() >= sGCellMinV[sSocRunGId[sRunNum - 1]])
                                && (GetGRealSocMilliAPI() > sGNowSoc[sSocRunGId[sRunNum - 1]]))
                            {
                                //电压差异在允许范围内30mV
                                if(ABS(GetGCellMinVoltAPI(), sGCellMinV[sSocRunGId[sRunNum - 1]]) <= G_ALLOW_DIFF_VOLT)
                                {
                                    //修正为与其他组误差只有2.5%(SOC往低修正)
                                    aimSoc = sGNowSoc[sSocRunGId[sRunNum - 1]] + G_ALLOW_DIFF_SOC - 5;
                                }
                                //电压差异在允许范围内60mV
                                else if(ABS(GetGCellMinVoltAPI(), sGCellMinV[sSocRunGId[sRunNum - 1]]) <= (G_ALLOW_DIFF_VOLT * 2))
                                {
                                    //修正为与其他组误差只有5%(SOC往低修正)
                                    aimSoc = sGNowSoc[sSocRunGId[sRunNum - 1]] + (G_ALLOW_DIFF_SOC * 2) - 10;
                                }
                                //电压差异过大
                                else
                                {
                                    //修正为与其他组误差只有7.5%(SOC往低修正)
                                    aimSoc = sGNowSoc[sSocRunGId[sRunNum - 1]] + (G_ALLOW_DIFF_SOC * 3) - 15;
                                }
                            }
                            //自身电压比其他组低
                            else
                            {
                                //无需修正
                                aimSoc = 0xffff;
                            }
                        }
                        //正在充电(向其他最大SOC组看齐)
                        else if(eCURR_CHG == GetGChgDhgStateAPI())
                        {
                            //自身电压比其他组小而SOC比其他组大
                            //自身电压比其他组大而SOC比其他组小
                            if(((GetGCellMaxVoltAPI() < sGCellMaxV[sSocRunGId[0]])
                                && (GetGRealSocMilliAPI() > sGNowSoc[sSocRunGId[0]]))
                                || ((GetGCellMaxVoltAPI() > sGCellMaxV[sSocRunGId[0]])
                                && (GetGRealSocMilliAPI() < sGNowSoc[sSocRunGId[0]])))
                            {
                                //修正为两组平均值
                                aimSoc = (GetGRealSocMilliAPI() + sGNowSoc[sSocRunGId[0]]) / 2;
                            }
                            //自身电压比其他组小且SOC比其他组小
                            else if((GetGCellMaxVoltAPI() <= sGCellMaxV[sSocRunGId[0]])
                                && (GetGRealSocMilliAPI() < sGNowSoc[sSocRunGId[0]]))
                            {
                                //电压差异在允许范围内30mV
                                if(ABS(GetGCellMaxVoltAPI(), sGCellMaxV[sSocRunGId[0]]) <= G_ALLOW_DIFF_VOLT)
                                {
                                    //修正为与其他组误差只有2.5%(SOC往高修正)
                                    aimSoc = sGNowSoc[sSocRunGId[0]] - G_ALLOW_DIFF_SOC + 5;
                                }
                                //电压差异在允许范围内60mV
                                else if(ABS(GetGCellMaxVoltAPI(), sGCellMaxV[sSocRunGId[0]]) <= (G_ALLOW_DIFF_VOLT * 2))
                                {
                                    //修正为与其他组误差只有5%(SOC往高修正)
                                    aimSoc = sGNowSoc[sSocRunGId[0]] - (G_ALLOW_DIFF_SOC * 2) + 10;
                                }
                                //电压差异过大
                                else
                                {
                                    //修正为与其他组误差只有7.5%(SOC往高修正)
                                    aimSoc = sGNowSoc[sSocRunGId[0]] - (G_ALLOW_DIFF_SOC * 3) + 15;
                                }
                            }
                            //自身电压比其他组低
                            else
                            {
                                //无需修正
                                aimSoc = 0xffff;
                            }
                        }
                        //系统待机
                        else
                        {
                            //自身电压比其他组小而SOC比其他组大
                            if((GetGCellMaxVoltAPI() < sGCellMaxV[sSocRunGId[sRunNum - 1]])
                                && (GetGCellMinVoltAPI() < sGCellMinV[sSocRunGId[sRunNum - 1]])
                                && (GetGRealSocMilliAPI() > sGNowSoc[sSocRunGId[sRunNum - 1]]))
                            {
                                //修正为两组平均值
                                aimSoc = (GetGRealSocMilliAPI() + sGNowSoc[sSocRunGId[sRunNum - 1]]) / 2;
                            }
                            //自身电压比其他组大而SOC比其他组小
                            else if((GetGCellMaxVoltAPI() > sGCellMaxV[sSocRunGId[0]])
                                && (GetGCellMinVoltAPI() > sGCellMinV[sSocRunGId[0]])
                                && (GetGRealSocMilliAPI() < sGNowSoc[sSocRunGId[0]]))
                            {
                                //修正为两组平均值
                                aimSoc = (GetGRealSocMilliAPI() + sGNowSoc[sSocRunGId[0]]) / 2;
                            }
                            //其他运行组充电截止自身SOC小
                            else if((1 == GetSocSoeGSyncChgDhgFinState())
                                && (GetGRealSocMilliAPI() < sGNowSoc[sSocRunGId[0]]))
                            {
                                //修正与最大组SOC一致
                                aimSoc = sGNowSoc[sSocRunGId[0]];
                            }
                            //其他运行组放电截止自身SOC大
                            else if((2 == GetSocSoeGSyncChgDhgFinState())
                                && (GetGRealSocMilliAPI() > sGNowSoc[sSocRunGId[sRunNum - 1]]))
                            {
                                //修正与最小组SOC一致
                                aimSoc = sGNowSoc[sSocRunGId[sRunNum - 1]];
                            }
                            //自身电压比其他组电压相当
                            else
                            {
                                //无需修正
                                aimSoc = 0xffff;
                            }
                        }
                    }
                    //其他组有相同SOC
                    else
                    {
                        //相同SOC的组不从头开始
                        if(index > 0)
                        {
                            //清除前面误差过大的电池组
                            for(i = 0; i < samNum; i++)
                            {
                                //组号前移
                                sSocRunGId[i] = sSocRunGId[index + i];
                            }
                        }

                        //数组中只剩下误差3%内的电池组
                        //自身SOC比其他最大组还大
                        if(GetGRealSocMilliAPI() >= (sGNowSoc[sSocRunGId[0]] + 10))
                        {
                            //修正与最大组SOC一致
                            aimSoc = sGNowSoc[sSocRunGId[0]];
                        }
                        //自身SOC比其他最小组还小
                        else if((GetGRealSocMilliAPI() + 10) <= sGNowSoc[sSocRunGId[samNum - 1]])
                        {
                            //修正与最小组SOC一致
                            aimSoc = sGNowSoc[sSocRunGId[samNum - 1]];
                        }
                        //其他运行组充电截止自身SOC小
                        else if((1 == GetSocSoeGSyncChgDhgFinState())
                            && (GetGRealSocMilliAPI() < sGNowSoc[sSocRunGId[0]]))
                        {
                            //修正与最大组SOC一致
                            aimSoc = sGNowSoc[sSocRunGId[0]];
                        }
                        //其他运行组放电截止自身SOC大
                        else if((2 == GetSocSoeGSyncChgDhgFinState())
                            && (GetGRealSocMilliAPI() > sGNowSoc[sSocRunGId[samNum - 1]]))
                        {
                            //修正与最小组SOC一致
                            aimSoc = sGNowSoc[sSocRunGId[samNum - 1]];
                        }
                        //自身SOC处于其他组之间
                        else
                        {
                            //无需修正
                            aimSoc = 0xffff;
                        }
                    }
                }
            }

            //计算修正目标SOC有效且修正值超过1%才执行修正
            if((0xffff != aimSoc) && (ABS(aimSoc, GetGRealSocMilliAPI()) > 10))
            {
                //修正自身SOC
                CorrGNowCapBySocAPI(clunum, aimSoc);
            }
            sState = 3;
            break;

        //SOE修正判断
        case 3:
            //无其他组运行
            if(0 == sRunNum)
            {
                //无需修正
                aimSoe = 0xffff;
            }
            //只有一组和自身运行
            else if(1 == sRunNum)
            {
                //两组SOE误差未超过3%
                if(ABS(GetGRealSoeMilliAPI(), sGNowSoe[sSoeRunGId[0]]) <= G_ALLOW_DIFF_SOE)
                {
                    //无需修正
                    aimSoe = 0xffff;
                }
                //正在放电(向其他最小SOE组看齐)
                else if(eCURR_DHG == GetGChgDhgStateAPI())
                {
                    //自身电压比其他组小而SOE比其他组大
                    //自身电压比其他组大而SOE比其他组小
                    if(((GetGCellMinVoltAPI() < sGCellMinV[sSoeRunGId[0]])
                        && (GetGRealSoeMilliAPI() > sGNowSoe[sSoeRunGId[0]]))
                        || ((GetGCellMinVoltAPI() > sGCellMinV[sSoeRunGId[0]])
                        && (GetGRealSoeMilliAPI() < sGNowSoe[sSoeRunGId[0]])))
                    {
                        //修正为两组平均值
                        aimSoe = (GetGRealSoeMilliAPI() + sGNowSoe[sSoeRunGId[0]]) / 2;
                    }
                    //自身电压比其他组大且SOE比其他组大
                    else if((GetGCellMinVoltAPI() >= sGCellMinV[sSoeRunGId[0]])
                        && (GetGRealSoeMilliAPI() > sGNowSoe[sSoeRunGId[0]]))
                    {
                        //电压差异在允许范围内30mV
                        if(ABS(GetGCellMinVoltAPI(), sGCellMinV[sSoeRunGId[0]]) <= G_ALLOW_DIFF_VOLT)
                        {
                            //修正为与其他组误差只有2.5%(SOE往低修正)
                            aimSoe = sGNowSoe[sSoeRunGId[0]] + G_ALLOW_DIFF_SOE - 5;
                        }
                        //电压差异在允许范围内60mV
                        else if(ABS(GetGCellMinVoltAPI(), sGCellMinV[sSoeRunGId[0]]) <= (G_ALLOW_DIFF_VOLT * 2))
                        {
                            //修正为与其他组误差只有5%(SOE往低修正)
                            aimSoe = sGNowSoe[sSoeRunGId[0]] + (G_ALLOW_DIFF_SOE * 2) - 10;
                        }
                        //电压差异过大
                        else
                        {
                            //修正为与其他组误差只有7.5%(SOE往低修正)
                            aimSoe = sGNowSoe[sSoeRunGId[0]] + (G_ALLOW_DIFF_SOE * 3) - 15;
                        }
                    }
                    //自身电压比其他组低
                    else
                    {
                        //无需修正
                        aimSoe = 0xffff;
                    }
                }
                //正在充电(向其他最大SOE组看齐)
                else if(eCURR_CHG == GetGChgDhgStateAPI())
                {
                    //自身电压比其他组小而SOE比其他组大
                    //自身电压比其他组大而SOE比其他组小
                    if(((GetGCellMaxVoltAPI() < sGCellMaxV[sSoeRunGId[0]])
                        && (GetGRealSoeMilliAPI() > sGNowSoe[sSoeRunGId[0]]))
                        || ((GetGCellMaxVoltAPI() > sGCellMaxV[sSoeRunGId[0]])
                        && (GetGRealSoeMilliAPI() < sGNowSoe[sSoeRunGId[0]])))
                    {
                        //修正为两组平均值
                        aimSoe = (GetGRealSoeMilliAPI() + sGNowSoe[sSoeRunGId[0]]) / 2;
                    }
                    //自身电压比其他组小且SOE比其他组小
                    else if((GetGCellMaxVoltAPI() <= sGCellMaxV[sSoeRunGId[0]])
                        && (GetGRealSoeMilliAPI() < sGNowSoe[sSoeRunGId[0]]))
                    {
                        //电压差异在允许范围内30mV
                        if(ABS(GetGCellMaxVoltAPI(), sGCellMaxV[sSoeRunGId[0]]) <= G_ALLOW_DIFF_VOLT)
                        {
                            //修正为与其他组误差只有2.5%(SOE往高修正)
                            aimSoe = sGNowSoe[sSoeRunGId[0]] - G_ALLOW_DIFF_SOE + 5;
                        }
                        //电压差异在允许范围内60mV
                        else if(ABS(GetGCellMaxVoltAPI(), sGCellMaxV[sSoeRunGId[0]]) <= (G_ALLOW_DIFF_VOLT * 2))
                        {
                            //修正为与其他组误差只有5%(SOE往高修正)
                            aimSoe = sGNowSoe[sSoeRunGId[0]] - (G_ALLOW_DIFF_SOE * 2) + 10;
                        }
                        //电压差异过大
                        else
                        {
                            //修正为与其他组误差只有7.5%(SOE往高修正)
                            aimSoe = sGNowSoe[sSoeRunGId[0]] - (G_ALLOW_DIFF_SOE * 3) + 15;
                        }
                    }
                    //自身电压比其他组低
                    else
                    {
                        //无需修正
                        aimSoe = 0xffff;
                    }
                }
                //系统待机
                else
                {
                    //自身电压比其他组小而SOE比其他组大
                    //自身电压比其他组大而SOE比其他组小
                    if(((GetGCellMaxVoltAPI() < sGCellMaxV[sSoeRunGId[0]])
                        && (GetGCellMinVoltAPI() < sGCellMinV[sSoeRunGId[0]])
                        && (GetGRealSoeMilliAPI() > sGNowSoe[sSoeRunGId[0]]))
                        || ((GetGCellMaxVoltAPI() > sGCellMaxV[sSoeRunGId[0]])
                        && (GetGCellMinVoltAPI() > sGCellMinV[sSoeRunGId[0]])
                        && (GetGRealSoeMilliAPI() < sGNowSoe[sSoeRunGId[0]])))
                    {
                        //修正为两组平均值
                        aimSoe = (GetGRealSoeMilliAPI() + sGNowSoe[sSoeRunGId[0]]) / 2;
                    }
                    //其他运行组充电截止自身SOE小
                    else if((1 == GetSocSoeGSyncChgDhgFinState())
                        && (GetGRealSoeMilliAPI() < sGNowSoe[sSoeRunGId[0]]))
                    {
                        //修正与另一组SOE一致
                        aimSoe = sGNowSoe[sSoeRunGId[0]];
                    }
                    //其他运行组放电截止自身SOE大
                    else if((2 == GetSocSoeGSyncChgDhgFinState())
                        && (GetGRealSoeMilliAPI() > sGNowSoe[sSoeRunGId[0]]))
                    {
                        //修正与另一组SOE一致
                        aimSoe = sGNowSoe[sSoeRunGId[0]];
                    }
                    //自身电压比其他组电压相当
                    else
                    {
                        //无需修正
                        aimSoe = 0xffff;
                    }
                }
            }
            //多组运行(sRunNum>=2)
            else
            {
                //其他组SOE误差未超过3%
                if(ABS(sGNowSoe[sSoeRunGId[0]], sGNowSoe[sSoeRunGId[sRunNum - 1]]) <= G_ALLOW_DIFF_SOE)
                {
                    //自身SOE比其他最大组还大
                    if(GetGRealSoeMilliAPI() >= (sGNowSoe[sSoeRunGId[0]] + 10))
                    {
                        //修正与最大组SOE一致
                        aimSoe = sGNowSoe[sSoeRunGId[0]];
                    }
                    //自身SOE比其他最小组还小
                    else if((GetGRealSoeMilliAPI() + 10) <= sGNowSoe[sSoeRunGId[sRunNum - 1]])
                    {
                        //修正与最小组SOE一致
                        aimSoe = sGNowSoe[sSoeRunGId[sRunNum - 1]];
                    }
                    //其他运行组充电截止自身SOE小
                    else if((1 == GetSocSoeGSyncChgDhgFinState())
                        && (GetGRealSoeMilliAPI() < sGNowSoe[sSoeRunGId[0]]))
                    {
                        //修正与最大组SOE一致
                        aimSoe = sGNowSoe[sSoeRunGId[0]];
                    }
                    //其他运行组放电截止自身SOE大
                    else if((2 == GetSocSoeGSyncChgDhgFinState())
                        && (GetGRealSoeMilliAPI() > sGNowSoe[sSoeRunGId[sRunNum - 1]]))
                    {
                        //修正与最小组SOE一致
                        aimSoe = sGNowSoe[sSoeRunGId[sRunNum - 1]];
                    }
                    //自身SOE处于其他组之间
                    else
                    {
                        //无需修正
                        aimSoe = 0xffff;
                    }
                }
                //其他组SOE误差超过3%
                else
                {
                    samNum = 0;
                    //查找最多SOE一致的组数
                    for(i = 0; i < sRunNum; i++)
                    {
                        k = 0;
                        maxSoe = sGNowSoe[sSoeRunGId[i]];
                        //j越大SOE越小
                        for(j = i; j < sRunNum; j++)
                        {
                            //与起始(最大)值误差在3%内
                            if(maxSoe <= (sGNowSoe[sSoeRunGId[j]] + G_ALLOW_DIFF_SOE))
                            {
                                k++;
                            }
                        }

                        //本次找到相同SOE的组数较大
                        if(k > samNum)
                        {
                            //记录起始索引和组数
                            index = i;
                            samNum = k;
                        }
                    }

                    //其他组无相同SOE
                    if(samNum <= 1)
                    {
                        //正在放电(向其他最小SOE组看齐)
                        if(eCURR_DHG == GetGChgDhgStateAPI())
                        {
                            //自身电压比其他组小而SOE比其他组大
                            //自身电压比其他组大而SOE比其他组小
                            if(((GetGCellMinVoltAPI() < sGCellMinV[sSoeRunGId[sRunNum - 1]])
                                && (GetGRealSoeMilliAPI() > sGNowSoe[sSoeRunGId[sRunNum - 1]]))
                                || ((GetGCellMinVoltAPI() > sGCellMinV[sSoeRunGId[sRunNum - 1]])
                                && (GetGRealSoeMilliAPI() < sGNowSoe[sSoeRunGId[sRunNum - 1]])))
                            {
                                //修正为两组平均值
                                aimSoe = (GetGRealSoeMilliAPI() + sGNowSoe[sSoeRunGId[sRunNum - 1]]) / 2;
                            }
                            //自身电压比其他组大且SOE比其他组大
                            else if((GetGCellMinVoltAPI() >= sGCellMinV[sSoeRunGId[sRunNum - 1]])
                                && (GetGRealSoeMilliAPI() > sGNowSoe[sSoeRunGId[sRunNum - 1]]))
                            {
                                //电压差异在允许范围内30mV
                                if(ABS(GetGCellMinVoltAPI(), sGCellMinV[sSoeRunGId[sRunNum - 1]]) <= G_ALLOW_DIFF_VOLT)
                                {
                                    //修正为与其他组误差只有2.5%(SOE往低修正)
                                    aimSoe = sGNowSoe[sSoeRunGId[sRunNum - 1]] + G_ALLOW_DIFF_SOE - 5;
                                }
                                //电压差异在允许范围内60mV
                                else if(ABS(GetGCellMinVoltAPI(), sGCellMinV[sSoeRunGId[sRunNum - 1]]) <= (G_ALLOW_DIFF_VOLT * 2))
                                {
                                    //修正为与其他组误差只有5%(SOE往低修正)
                                    aimSoe = sGNowSoe[sSoeRunGId[sRunNum - 1]] + (G_ALLOW_DIFF_SOE * 2) - 10;
                                }
                                //电压差异过大
                                else
                                {
                                    //修正为与其他组误差只有7.5%(SOE往低修正)
                                    aimSoe = sGNowSoe[sSoeRunGId[sRunNum - 1]] + (G_ALLOW_DIFF_SOE * 3) - 15;
                                }
                            }
                            //自身电压比其他组低
                            else
                            {
                                //无需修正
                                aimSoe = 0xffff;
                            }
                        }
                        //正在充电(向其他最大SOE组看齐)
                        else if(eCURR_CHG == GetGChgDhgStateAPI())
                        {
                            //自身电压比其他组小而SOE比其他组大
                            //自身电压比其他组大而SOE比其他组小
                            if(((GetGCellMaxVoltAPI() < sGCellMaxV[sSoeRunGId[0]])
                                && (GetGRealSoeMilliAPI() > sGNowSoe[sSoeRunGId[0]]))
                                || ((GetGCellMaxVoltAPI() > sGCellMaxV[sSoeRunGId[0]])
                                && (GetGRealSoeMilliAPI() < sGNowSoe[sSoeRunGId[0]])))
                            {
                                //修正为两组平均值
                                aimSoe = (GetGRealSoeMilliAPI() + sGNowSoe[sSoeRunGId[0]]) / 2;
                            }
                            //自身电压比其他组小且SOE比其他组小
                            else if((GetGCellMaxVoltAPI() <= sGCellMaxV[sSoeRunGId[0]])
                                && (GetGRealSoeMilliAPI() < sGNowSoe[sSoeRunGId[0]]))
                            {
                                //电压差异在允许范围内30mV
                                if(ABS(GetGCellMaxVoltAPI(), sGCellMaxV[sSoeRunGId[0]]) <= G_ALLOW_DIFF_VOLT)
                                {
                                    //修正为与其他组误差只有2.5%(SOE往高修正)
                                    aimSoe = sGNowSoe[sSoeRunGId[0]] - G_ALLOW_DIFF_SOE + 5;
                                }
                                //电压差异在允许范围内60mV
                                else if(ABS(GetGCellMaxVoltAPI(), sGCellMaxV[sSoeRunGId[0]]) <= (G_ALLOW_DIFF_VOLT * 2))
                                {
                                    //修正为与其他组误差只有5%(SOE往高修正)
                                    aimSoe = sGNowSoe[sSoeRunGId[0]] - (G_ALLOW_DIFF_SOE * 2) + 10;
                                }
                                //电压差异过大
                                else
                                {
                                    //修正为与其他组误差只有7.5%(SOE往高修正)
                                    aimSoe = sGNowSoe[sSoeRunGId[0]] - (G_ALLOW_DIFF_SOE * 3) + 15;
                                }
                            }
                            //自身电压比其他组低
                            else
                            {
                                //无需修正
                                aimSoe = 0xffff;
                            }
                        }
                        //系统待机
                        else
                        {
                            //自身电压比其他组小而SOE比其他组大
                            if((GetGCellMaxVoltAPI() < sGCellMaxV[sSoeRunGId[sRunNum - 1]])
                                && (GetGCellMinVoltAPI() < sGCellMinV[sSoeRunGId[sRunNum - 1]])
                                && (GetGRealSoeMilliAPI() > sGNowSoe[sSoeRunGId[sRunNum - 1]]))
                            {
                                //修正为两组平均值
                                aimSoe = (GetGRealSoeMilliAPI() + sGNowSoe[sSoeRunGId[sRunNum - 1]]) / 2;
                            }
                            //自身电压比其他组大而SOE比其他组小
                            else if((GetGCellMaxVoltAPI() > sGCellMaxV[sSoeRunGId[0]])
                                && (GetGCellMinVoltAPI() > sGCellMinV[sSoeRunGId[0]])
                                && (GetGRealSoeMilliAPI() < sGNowSoe[sSoeRunGId[0]]))
                            {
                                //修正为两组平均值
                                aimSoe = (GetGRealSoeMilliAPI() + sGNowSoe[sSoeRunGId[0]]) / 2;
                            }
                            //其他运行组充电截止自身SOE小
                            else if((1 == GetSocSoeGSyncChgDhgFinState())
                                && (GetGRealSoeMilliAPI() < sGNowSoe[sSoeRunGId[0]]))
                            {
                                //修正与最大组SOE一致
                                aimSoe = sGNowSoe[sSoeRunGId[0]];
                            }
                            //其他运行组放电截止自身SOE大
                            else if((2 == GetSocSoeGSyncChgDhgFinState())
                                && (GetGRealSoeMilliAPI() > sGNowSoe[sSoeRunGId[sRunNum - 1]]))
                            {
                                //修正与最小组SOE一致
                                aimSoe = sGNowSoe[sSoeRunGId[sRunNum - 1]];
                            }
                            //自身电压比其他组电压相当
                            else
                            {
                                //无需修正
                                aimSoe = 0xffff;
                            }
                        }
                    }
                    //其他组有相同SOE
                    else
                    {
                        //相同SOE的组不从头开始
                        if(index > 0)
                        {
                            //清除前面误差过大的电池组
                            for(i = 0; i < samNum; i++)
                            {
                                //组号前移
                                sSoeRunGId[i] = sSoeRunGId[index + i];
                            }
                        }

                        //数组中只剩下误差3%内的电池组
                        //自身SOE比其他最大组还大
                        if(GetGRealSoeMilliAPI() >= (sGNowSoe[sSoeRunGId[0]] + 10))
                        {
                            //修正与最大组SOE一致
                            aimSoe = sGNowSoe[sSoeRunGId[0]];
                        }
                        //自身SOE比其他最小组还小
                        else if((GetGRealSoeMilliAPI() + 10) <= sGNowSoe[sSoeRunGId[samNum - 1]])
                        {
                            //修正与最小组SOE一致
                            aimSoe = sGNowSoe[sSoeRunGId[samNum - 1]];
                        }
                        //其他运行组充电截止自身SOE小
                        else if((1 == GetSocSoeGSyncChgDhgFinState())
                            && (GetGRealSoeMilliAPI() < sGNowSoe[sSoeRunGId[0]]))
                        {
                            //修正与最大组SOE一致
                            aimSoe = sGNowSoe[sSoeRunGId[0]];
                        }
                        //其他运行组放电截止自身SOE大
                        else if((2 == GetSocSoeGSyncChgDhgFinState())
                            && (GetGRealSoeMilliAPI() > sGNowSoe[sSoeRunGId[samNum - 1]]))
                        {
                            //修正与最小组SOE一致
                            aimSoe = sGNowSoe[sSoeRunGId[samNum - 1]];
                        }
                        //自身SOE处于其他组之间
                        else
                        {
                            //无需修正
                            aimSoe = 0xffff;
                        }
                    }
                }
            }

            //计算修正目标SOE有效且修正值超过1%才执行修正
            if((0xffff != aimSoe) && (ABS(aimSoe, GetGRealSoeMilliAPI()) > 10))
            {
                //修正自身SOE
                CorrGNowEnerBySoeAPI(aimSoe);
            }
            sRunNum = 0;
            sState = 10;
            break;

        //修正周期延时
        case 10:
            //达到等待周期则返回
            if((sRunNum / 2) >= G_SOCSOE_SYNC_T)
            {
                sRunNum = 0;
                sState = 0;
            }
            //达到等待周期前2s
            else if((sRunNum / 2) == (G_SOCSOE_SYNC_T - 2))
            {
                //清除接收信息状态
                sRunNum++;
                sGInfoRcvState = 0;
            }
            else
            {
                sRunNum++;
            }
            break;

        default:
            sState = 0;
            break;
    }
}

/*=================================================================================================
** @Name      : void SocSoeSyncRcvGMsgHandle_API(u32 ID, u8 *pdata, u8 DLC, u8 canNum)
** @Input     : ID:CANID *pdata:CAN数据域  DLC:数据域长度 canNum:CAN通道号
** @Output    : void
** @Function  : SOCSOE同步接收处理任务
** @The notes : 用户放在CAN接收处理HOOK接口调用
**===============================================================================================*/
void SocSoeSyncRcvGMsgHandle_API(u32 ID, u8 *pdata, u8 DLC, u8 canNum)
{
     u8 saddr = 0;
     u8 daddr = 0;
     u8 index = 0;
     u8 funct = 0;
     u8 group = 0;

     /*解析CAN帧ID*/
     saddr = ID & 0xff;
     daddr = (ID >> 8) & 0xff;
     index = (ID >> 16) & 0xff;
     funct = (ID >> 25) & 0x0f;

    /*非自身报文/非广播报文*/
    if((gGBmuGenPara_102[eBmuGenPara102_BmuDev] != daddr) && (0xff != daddr))
    {
         return;
    }

    /*单帧上传电池组信息报文(同一堆下电池组)*/
    if((10 == funct) && (saddr >= BMU_BAS_DEVADDR) && (saddr < SMU_BAS_DEVADDR)
         && (((gGBmuGenPara_102[eBmuGenPara102_BmuDev] - BMU_BAS_DEVADDR) / gSysGenParaRO_100[eSGenPara100_AllGNum]) == ((saddr - BMU_BAS_DEVADDR) / gSysGenParaRO_100[eSGenPara100_AllGNum])))
    {
        /*组号*/
        group = (saddr - BMU_BAS_DEVADDR) % gSysGenParaRO_100[eSGenPara100_AllGNum];

        /*组地址在本组之后,索引前移1*/
        if(saddr > gGBmuGenPara_102[eBmuGenPara102_BmuDev])
        {
            group -= 1;
        }

        /*组号未越界*/
        if(group < (PILE_MAX_G_NUM-1))
        {
            /*记录通信接收正常*/
            BitSet(sGInfoRcvState, group);

            /*状态信息报文*/
            if((50 == index) && (DLC >= 6))
            {
                sGProState[group] = ((u16)pdata[1] << 8) | pdata[0];
                sGWorkState[group] = ((u16)pdata[3] << 8) | pdata[2];
                sGChgState[group] = ((u16)pdata[5] << 8) | pdata[4];
            }
            /*SOCSOE信息报文*/
            else if((61 == index) && (DLC >= 4))
            {
                sGNowSoc[group] = ((u16)pdata[1] << 8) | pdata[0];
                sGNowSoe[group] = ((u16)pdata[3] << 8) | pdata[2];
            }
            /*电压信息报文*/
            else if((63 == index) && (DLC >= 8))
            {
                sGCellMaxV[group] = ((u16)pdata[3] << 8) | pdata[2];
                sGCellMinV[group] = ((u16)pdata[7] << 8) | pdata[6];
            }
        }
    }
}

/*=================================================================================================
** @Name      : u8 GetSocSoeGSyncChgDhgFinState(void)
** @Input     : void
** @Output    : 0:非充放电截止 1:充电截止 2:放电截止
** @Function  : 组间SOCSOE同步充放电截止状态
** @The notes : 由于同网其他组充放电截止使本组不能继续充放电
**===============================================================================================*/
u8 GetSocSoeGSyncChgDhgFinState(void)
{
    //同网运行电池组运行且充电截止,自身也在运行空闲则需要同步SOC/SOE
    if((1 == sGRunChgDhgFinState)
        && (eWORK_RUN == GetGWorkStateAPI())
        && (eCURR_IDLE == GetGChgDhgStateAPI()))
    {
        return(1);
    }
    //同网运行电池组运行且放电截止,自身也在运行空闲则需要同步SOC/SOE
    else if((2 == sGRunChgDhgFinState)
        && (eWORK_RUN == GetGWorkStateAPI())
        && (eCURR_IDLE == GetGChgDhgStateAPI()))
    {
        return(2);
    }
    //非充放电截止
    else
    {
        return(0);
    }
}

/*=============================================================================================
** @Static function definition
**===========================================================================================*/
#endif
#endif
