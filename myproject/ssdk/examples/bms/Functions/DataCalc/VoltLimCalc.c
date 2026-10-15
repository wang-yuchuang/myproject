/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : VoltLimCalc.c
** @Founder      : ZQD808
** @Date         : 2026.03.12
** @Function     : 总压限值计算模块
** @Instructions : 对可调压充放电设备有效
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "CalcUtil.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "PublicInfo.h"
#include "CalcInterface.h"
//#include "TacInterface.h"
#include "GroupState.h"
#include "CurrLimCalc.h"
#include "VoltLimCalc.h"
#include "CurrLimFall.h"
#include <debug.h>
#include <string.h>
#include <stdio.h>


extern u16 *gSysLimInfo_53;             /*系统限值信息*/
extern u16 gGBmuHigLevPara_103[];  		/*主控高级参数值*/

/*=============================================================================================
** @Name      : void VoltLimCalcTask(void)
** @Input     : void
** @Output    : void
** @Function  : 充放电电压限值计算任务
** @The notes : 1S
**===========================================================================================*/
void VoltLimCalcTask(void)
{
    if(gSysLimInfo_53 == NULL)
    {
        printf("gSysLimInfo_53 is null\r\n");
        return;
    }
    /*总电压限值调节使能*/
    #if(1 == VOLT_LIM_AJUST_EN)
    static u8 sChgTime[2] = {0};
    static u8 sDhgTime[2] = {0};
    u8 clunum = 0;
    for(clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        if((GetGWorkStateAPI(clunum) == eWORK_INIT)           /*系统初始化*/
            || (GetGWorkStateAPI(clunum) == eWORK_OFF)        /*系统关机*/
            //|| (1 == CalcGetGroupCellVExpStateHook(clunum))   /*单体异常*/
            //|| (GetGCellSumVoltAPI(clunum) < gGBmuHigLevPara_103[eBmuHigLevPara103_HigVOffV])/*总电压异常*/
        )
        {
            if(clunum == 0)
            {
                /*恒压0V代表需PCS停机*/
                gSysLimInfo_53[eSysLimInfo53_1ChgV] = 0;
                gSysLimInfo_53[eSysLimInfo53_1DhgV] = 0;        /*电压限值为0*/
            }
            else
            {
                /*恒压0V代表需PCS停机*/
                gSysLimInfo_53[eSysLimInfo53_2ChgV] = 0;
                gSysLimInfo_53[eSysLimInfo53_2DhgV] = 0;        /*电压限值为0*/
            }

        }
        else if(eWORK_RUN != GetGWorkStateAPI(clunum))        /*系统未运行*/
        {
            /*按总电压±1V恒压*/
            //gGLimVInfo_52[eLimVInfo52_ChgV] = GetGCellSumVoltAPI() + 10;
            //gGLimVInfo_52[eLimVInfo52_DhgV] = GetGCellSumVoltAPI() - 10;
            if(clunum == 0)
            {
                gSysLimInfo_53[eSysLimInfo53_1ChgV]  = GetGCellSumVoltAPI(clunum) + 10;
                gSysLimInfo_53[eSysLimInfo53_1DhgV]  = GetGCellSumVoltAPI(clunum) - 10;
            }
            else
            {
                gSysLimInfo_53[eSysLimInfo53_2ChgV]  = GetGCellSumVoltAPI(clunum) + 10;
                gSysLimInfo_53[eSysLimInfo53_2DhgV]  = GetGCellSumVoltAPI(clunum) - 10;
            }
        }
        else                                           /*系统运行则正常计算调节*/
        {
            /*允许充电*/
            if(GetGChgLimCurrAPI(clunum) > 0)
            {
                /*未开始降电流*/
                if((0xff == GetFallChgCIndexStepAPI(clunum)) || (GetGChgLimCurrAPI(clunum) >= 100))
                {
                    if(clunum == 0)
                    {
                        /*执行均充最大电压*/
                        //gGLimVInfo_52[eLimVInfo52_ChgV] = CalcGetChgLimMaxSumVHook();
                        gSysLimInfo_53[eSysLimInfo53_1ChgV] = CalcGetChgLimMaxSumVHook();
                    }
                    else
                    {
                        gSysLimInfo_53[eSysLimInfo53_2ChgV] = CalcGetChgLimMaxSumVHook();
                    }

                }
                /*到达高压开始降电流*/
                else
                {
                    /*总压比均充电压大*/
                    if((GetGCellSumVoltAPI(clunum) + 10) > CalcGetChgLimMaxSumVHook())
                    {
                        if(clunum == 0)
                        {
                            /*按均充电压恒压*/
                            //gGLimVInfo_52[eLimVInfo52_ChgV] = CalcGetChgLimMaxSumVHook();
                             gSysLimInfo_53[eSysLimInfo53_1ChgV] = CalcGetChgLimMaxSumVHook();
                        }
                        else
                        {
                             gSysLimInfo_53[eSysLimInfo53_2ChgV] = CalcGetChgLimMaxSumVHook();
                        }

                    }
                    /*总电压未到达均衡最大电压*/
                    else
                    {
                        /*间隔60s检测一次是否上提恒压目标值*/
                        if(sChgTime[clunum] >= 60)
                        {
                            /*充电电流降到2A则上提1V恒压目标值*/
                            if(GetGSampOutCurrAPI(clunum) >= -20)
                            {
                                if(clunum == 0)
                                {
                                    //gGLimVInfo_52[eLimVInfo52_ChgV] = GetGCellSumVoltAPI() + 10;
                                    gSysLimInfo_53[eSysLimInfo53_1ChgV] = GetGCellSumVoltAPI(clunum) + 10;
                                }
                                else
                                {
                                    gSysLimInfo_53[eSysLimInfo53_2ChgV] = GetGCellSumVoltAPI(clunum) + 10;
                                }
                            }
                            sChgTime[clunum] = 0;
                        }
                        else
                        {
                            sChgTime[clunum]++;
                        }
                    }
                }
            }
            /*已不允许充电*/
            else
            {
                /*电池未充满*/
                if(1 != TacGetGroupBattStateAPI(clunum)) 
                {
                    /*按照当前总电压恒压*/
                    if(clunum == 0)
                    {
                        gSysLimInfo_53[eSysLimInfo53_1ChgV] = GetGCellSumVoltAPI(clunum);
                    }
                    else
                    {
                        gSysLimInfo_53[eSysLimInfo53_2ChgV] = GetGCellSumVoltAPI(clunum);
                    }
                }
                /*电池已经充满*/
                else
                {
                    /*电池电压过高或还在充电*/
                    if((GetGCellMaxVoltAPI(clunum) >= CalcGetChgLimCellVoltHook())
                        || (GetGSampOutCurrAPI(clunum) <= -5))
                    {
                        /*间隔60s下降0.1V恒压目标值*/
                        if(sChgTime[clunum] >= 60)
                        {
                            /*充电电流大于5A则下降0.5V*/
                            if(GetGSampOutCurrAPI(clunum) <= -50)
                            {
                                if(clunum == 0)
                                {
                                    if(gSysLimInfo_53[eSysLimInfo53_1ChgV] > 5)
                                    {
                                        gSysLimInfo_53[eSysLimInfo53_1ChgV] = gSysLimInfo_53[eSysLimInfo53_1ChgV] - 5;
                                    }
                                }
                                else
                                {
                                    if(gSysLimInfo_53[eSysLimInfo53_2ChgV] > 5)
                                    {
                                        gSysLimInfo_53[eSysLimInfo53_2ChgV] = gSysLimInfo_53[eSysLimInfo53_2ChgV] - 5;
                                    }
                                }
                            }
                            /*充电电流较小则下降0.1V*/
                            else
                            {
                                if(clunum == 0)
                                {
                                    if(gSysLimInfo_53[eSysLimInfo53_1ChgV] > 1)
                                    {
                                        gSysLimInfo_53[eSysLimInfo53_1ChgV] = gSysLimInfo_53[eSysLimInfo53_1ChgV] - 1;
                                    }
                                }
                                else
                                {
                                    if(gSysLimInfo_53[eSysLimInfo53_2ChgV] > 1)
                                    {
                                        gSysLimInfo_53[eSysLimInfo53_2ChgV] = gSysLimInfo_53[eSysLimInfo53_2ChgV] - 1;
                                    }
                                }
                            }
                            sChgTime[clunum] = 0;
                        }
                        else
                        {
                            sChgTime[clunum]++;
                        }
                    }
                }
            }
                
            /*允许放电*/
            if(GetGDhgLimCurrAPI(clunum) > 0)
            {
                /*未开始降电流*/
                if((0xff == GetFallDhgCIndexStepAPI(clunum)) || (GetGDhgLimCurrAPI(clunum) >= 100))
                {
                    if(clunum == 0)
                    {
                        /*执行放电最小电压限值*/
                        gSysLimInfo_53[eSysLimInfo53_1DhgV] = CalcGetDhgLimMinSumVHook();
                    }
                    else
                    {
                        /*执行放电最小电压限值*/
                        gSysLimInfo_53[eSysLimInfo53_2DhgV] = CalcGetDhgLimMinSumVHook();
                    }

                }
                /*已低压开始降电流*/
                else
                {
                    /*总压比放电目标电压小*/
                    if((GetGCellSumVoltAPI(clunum) - 10) < CalcGetDhgLimMinSumVHook())
                    {
                        if(clunum == 0)
                        {
                            /*按放电目标电压恒压*/
                            gSysLimInfo_53[eSysLimInfo53_1DhgV] = CalcGetDhgLimMinSumVHook();
                        }
                        else
                        {
                            /*按放电目标电压恒压*/
                            gSysLimInfo_53[eSysLimInfo53_2DhgV] = CalcGetDhgLimMinSumVHook();
                        }

                    }
                    /*总电压未到达放电最低电压*/
                    else
                    {
                        /*间隔60s检测一次是否下降恒压目标值*/
                        if(sDhgTime[clunum] >= 60)
                        {
                            /*放电电流降到2A则下降1V恒压目标值*/
                            if(GetGSampOutCurrAPI(clunum) <= 20)
                            {
                                if(clunum == 0)
                                {
                                    gSysLimInfo_53[eSysLimInfo53_1DhgV] = GetGCellSumVoltAPI(clunum) - 10;
                                }
                                else
                                {
                                    gSysLimInfo_53[eSysLimInfo53_2DhgV] = GetGCellSumVoltAPI(clunum) - 10;
                                }
                                
                            }
                            sDhgTime[clunum] = 0;
                        }
                        else
                        {
                            sDhgTime[clunum]++;
                        }
                    }
                }
            }
            /*已不允许放电*/
            else
            {
                /*电池未放空*/
                if(2 != TacGetGroupBattStateAPI(clunum)) 
                {
                    if(clunum == 0)
                    {
                        /*按照当前总电压恒压*/
                        gSysLimInfo_53[eSysLimInfo53_1DhgV] = GetGCellSumVoltAPI(clunum);
                    }
                    else
                    {
                        /*按照当前总电压恒压*/
                        gSysLimInfo_53[eSysLimInfo53_2DhgV] = GetGCellSumVoltAPI(clunum);
                    }

                }
                /*电池已经放空*/
                else
                {
                    /*电池电压低高或还在放电*/
                    if((GetGCellMinVoltAPI(clunum) <= CalcGetDhgLimCellVoltHook())
                        || (GetGSampOutCurrAPI(clunum) >= 5))
                    {
                        /*间隔60上升0.1V恒压目标值*/
                        if(sDhgTime[clunum] >= 60)
                        {
                            /*放电电流大于5A则下降0.5V*/
                            if(GetGSampOutCurrAPI(clunum) >= 50)
                            {
                                if(clunum == 0)
                                {
                                    if(gSysLimInfo_53[eSysLimInfo53_1DhgV] > 5)
                                    {
                                        gSysLimInfo_53[eSysLimInfo53_1DhgV] = gSysLimInfo_53[eSysLimInfo53_1DhgV] - 5;
                                    }
                                }
                                else
                                {
                                    if(gSysLimInfo_53[eSysLimInfo53_2DhgV] > 5)
                                    {
                                        gSysLimInfo_53[eSysLimInfo53_2DhgV] = gSysLimInfo_53[eSysLimInfo53_2DhgV] - 5;
                                    }
                                }
                            }
                            /*放电电流较小则下降0.1V*/
                            else
                            {
                                if(clunum == 0)
                                {
                                    if(gSysLimInfo_53[eSysLimInfo53_1DhgV] > 1)
                                    {
                                        gSysLimInfo_53[eSysLimInfo53_1DhgV] = gSysLimInfo_53[eSysLimInfo53_1DhgV] - 1;
                                    }
                                }
                                else
                                {
                                    if(gSysLimInfo_53[eSysLimInfo53_2DhgV] > 1)
                                    {
                                        gSysLimInfo_53[eSysLimInfo53_2DhgV] = gSysLimInfo_53[eSysLimInfo53_2DhgV] - 1;
                                    }
                                }

                            }
                            sDhgTime[clunum] = 0;
                        }
                        else
                        {
                            sDhgTime[clunum]++;
                        }
                    }
                }
            }
        }
    }
    /*总电压限值调节禁止*/
    #else
    /*计算电压限值*/
    if((GetSysWorkStateAPI() == eWORK_INIT)         /*系统初始化*/
        || (GetSysWorkStateAPI() == eWORK_ERR))     /*系统故障*/
    {
    	gGLimVInfo_52[eLimVInfo52_ChgV] = 0;
        gGLimVInfo_52[eLimVInfo52_DhgV] = 0;        /*电压限值为0*/
    }
    else                                            /*系统正常则正常计算*/
    {
        /*取电池组总压高低压限值*/
    	gGLimVInfo_52[eLimVInfo52_ChgV] = CalcGetChgLimVH2SumVolt();
        gGLimVInfo_52[eLimVInfo52_DhgV] = CalcGetDhgLimVL2SumVolt();
    }
    #endif
}

