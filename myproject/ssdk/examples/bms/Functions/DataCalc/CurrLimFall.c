/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CurrLimFall.h
** @Founder      : ZQD808
** @Date         : 2026.03.12
** @Function     : 充放电电流限值平滑模块
** @Instructions :
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "CalcUtil.h"
//#include "CurrLimPID.h"
#include "CalcInterface.h"
#include "CurrLimCalc.h"
#include "CurrLimFall.h"
#include <debug.h>
#include <string.h>
#include "DiagCheck.h"
static u8  sFallChgCIndex[2] = {0xff,0xff};       /*平滑充电电流限值表索引*/
static u16 sFallChgMaxCurr[2] = {0};              /*平滑充电电流最大电流值(恒压开始时的实际电流, 0.1A)*/
static u16 sFallChgCurrLim[2] = {0};              /*平滑计算的充电电流限值*/

static u8  sFallDhgCIndex[2] = {0xff,0xff};       /*平滑放电电流限值表索引*/
static u16 sFallDhgMaxCurr[2] = {0};              /*平滑放电电流最大电流值(恒压开始时的实际电流, 0.1A)*/
static u16 sFallDhgCurrLim[2] = {0};              /*平滑计算的放电电流限值*/

extern u16 *gSysLimInfo_53;                       /*系统限值信息*/
extern u16 gGBmuGenPara_102[];                    /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];                 /*主控高级参数值*/

extern u8 gGroupParaRO_119[];                     /*EEP电压/SOC平滑电流限值参数表配置信息*/

/*=============================================================================================
** @Name      : void CurrLimFallTask(void)
** @Input     : void
** @Output    : void
** @Function  : 充放电电流限值平滑任务
** @The notes : 100ms
**===========================================================================================*/
void CurrLimFallTask(void)
{
    u16 currLim = 0;
    u16 chgHCurr = 0;         /*充电过流值*/
    u16 dhgHCurr = 0;         /*放电过流值*/
    static u16 sHisMaxChgCurr[2] = {0};
    static u16 sHisMaxDhgCurr[2] = {0};

    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        currLim = 0;
        chgHCurr = 0;         /*充电过流值*/
        dhgHCurr = 0;         /*放电过流值*/
        /*确保电压采样正常才进行充放电电流限值计算*/
        if((GetGWorkStateAPI(clunum) == eWORK_INIT)
            || (GetGWorkStateAPI(clunum) == eWORK_OFF)
            || (2 != DiagGetRcvAllSlaveFlagAPI(clunum))
            || (2 != DiagGetRcvAllCellFlagAPI(clunum)))
        {
            sHisMaxChgCurr[clunum] = 0;
            sHisMaxDhgCurr[clunum] = 0;

            sFallChgCIndex[clunum] = 0xff;
            sFallChgMaxCurr[clunum] = 0;
            sFallChgCurrLim[clunum] = GetChgHReqMaxCurr();

            sFallDhgCIndex[clunum] = 0xff;
            sFallDhgMaxCurr[clunum] = 0;
            sFallDhgCurrLim[clunum] = GetDhgHReqMaxCurr();
            continue;
        }

        /*获取过流限值可用比率下最大限流值*/
        chgHCurr = GetChgHReqMaxCurr();            /*充电过流限值*/
        dhgHCurr = GetDhgHReqMaxCurr();            /*放电过流限值*/
        
        /*使能充电末端PID恒压平滑电流限值*/
        if(1 == gGBmuHigLevPara_103[eBmuHigLevPara103_ChgFinCVEn])
        {
            if(sHisMaxChgCurr[clunum] == chgHCurr)                  /*最大限值未改变(防止参数配置)*/
            {
                CVPIDCtrlChgCLim(clunum);                         /*PID平滑计算充电电流限值*/
                
                /*未开始恒压*/
                if(0xff == sFallChgCIndex[clunum])
                {
                    sFallChgCurrLim[clunum] = chgHCurr;             /*取最大限流值*/
                }
                /*开始恒压*/
                else
                {
                    /*PID输出的电流合法 */
                    //if(1 == CVUtilPIDChgCLim(gChgCurrPiIO.piOut))
                    {

                        //不用PI调节直接按步长下降
                        currLim = CVCalcChgPIDAimCurr(clunum, sFallChgCIndex[clunum]);
                        
                        /*不能超过充电限值*/
                        if(currLim > chgHCurr)
                        {
                            currLim = chgHCurr;
                        }
                        
                        /*未到最后一个恒压点*/
                        if(sFallChgCIndex[clunum] < (SLOW_CURRLIMIT_NUM - 1))
                        {
                            /*不能小于下一个限流点限值*/
                            if(currLim < (CVCalcChgPIDAimCurr(clunum, sFallChgCIndex[clunum] + 1) - 10))
                            {
                                currLim = CVCalcChgPIDAimCurr(clunum, sFallChgCIndex[clunum] + 1) - 10;
                            }
                        }
                    }

                    /*不能小于截止电流 0.1A*/
                    if(currLim < gGBmuGenPara_102[eBmuGenPara102_ChgCFinLim])
                    {
                        currLim = gGBmuGenPara_102[eBmuGenPara102_ChgCFinLim];
                    }

                    sFallChgCurrLim[clunum] = currLim;
                }
            }
            else                                            /*充电电流限值配置值变化*/
            {
                /*不平滑充电电流限值*/
                CVChgCurrLimPIDReset(clunum);                     /*充电电流限值PID参数复位*/
                sFallChgCurrLim[clunum] = chgHCurr;
            }
        }
        /*未使能充电电流限值平滑*/
        else
        {
            sFallChgCurrLim[clunum] = chgHCurr;                     /*充电电流限值取过流值*/
        }
        
        /*使能放电末端PID恒压平滑电流限值*/
        if(1 == gGBmuHigLevPara_103[eBmuHigLevPara103_DhgFinCVEn])
        {
            if(sHisMaxDhgCurr[clunum] == dhgHCurr)                  /*放电最大限值未改变*/
            {
                CVCalcDhgCLimDnStep(clunum);                      /*计算末端降流步骤计算放电电流限值*/

                /*未开始恒压*/
                if(0xff == sFallDhgCIndex[clunum])
                {
                    sFallDhgCurrLim[clunum] = dhgHCurr;             /*取最大限流值*/
                }
                /*开始恒压*/
                else
                {
                    /*取当前步骤对应的电流限值*/
                    currLim  = CVCalcDhgStepDnAimCurr(clunum, sFallDhgCIndex[clunum]);

                    /*不能超过充电限值*/
                    if(currLim > dhgHCurr)
                    {
                        currLim = dhgHCurr;
                    }

                    /*未到最后一个恒压点*/
                    if(sFallDhgCIndex[clunum] < (SLOW_CURRLIMIT_NUM - 1))
                    {
                        /*不能小于下一个限流点限值*/
                        if(currLim < (CVCalcDhgStepDnAimCurr(clunum, sFallDhgCIndex[clunum] + 1) - 10))
                        {
                            currLim = (CVCalcDhgStepDnAimCurr(clunum, sFallDhgCIndex[clunum] + 1) - 10);
                        }
                    }

                    /*不能小于截止电流 0.1A*/
                    if(currLim < gGBmuGenPara_102[eBmuGenPara102_DhgCFinLim])
                    {
                        currLim = gGBmuGenPara_102[eBmuGenPara102_DhgCFinLim];
                    }

                    sFallDhgCurrLim[clunum] = currLim;
                }
            }
            else                                            /*放电电流限值配置值变化*/
            {
                /*不平滑放电电流限值*/
                CVDhgCurrLimStepDnReset(clunum);
                sFallDhgCurrLim[clunum] = dhgHCurr;                 /*放电电流限值取过流值*/
            }
        }
        /*未使能电流限值平滑*/
        else
        {
            sFallDhgCurrLim[clunum] = dhgHCurr;                     /*放电电流限值取过流值*/
        }
        
        sHisMaxChgCurr[clunum] = GetChgHReqMaxCurr();               /*充电过流限值*/
        sHisMaxDhgCurr[clunum] = GetDhgHReqMaxCurr();               /*放电过流限值*/
    }
}

/*=============================================================================================
** @Name      : void ResetCurrLimFallAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 复位充放电电流平滑限制
** @The notes : 平滑步骤清零
**===========================================================================================*/
void ResetCurrLimFallAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return;
    }
    sFallChgCIndex[clunum] = 0xff;
    sFallDhgCIndex[clunum] = 0xff;
}
/*=============================================================================================
** @Name      : u8 GetFallChgCIndexStepAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 放电电流限值
** @Function  : 返回平滑降充电电流限值当前进度步骤
** @The notes : 当前降到第几个点
**===========================================================================================*/
u8 GetFallChgCIndexStepAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    return(sFallChgCIndex[clunum]);
}
/*=============================================================================================
** @Name      : u8 GetFallDhgCIndexStepAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 放电电流限值
** @Function  : 返回平滑降放电电流限值当前进度步骤
** @The notes : 当前降到第几个点
**===========================================================================================*/
u8 GetFallDhgCIndexStepAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    return(sFallDhgCIndex[clunum]);
}
/*=============================================================================================
** @Name      : u16 GetFallChgLimCurr(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 充电电流限值
** @Function  : 返回平滑计算的充电电流限值
** @The notes :
**===========================================================================================*/
u16 GetFallChgLimCurr(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    return(sFallChgCurrLim[clunum]);
}

/*=============================================================================================
** @Name      : u16 GetFallDhgLimCurr(void)
** @Input     : clunum:簇编号
** @Output    : 放电电流限值
** @Function  : 返回平滑计算的放电电流限值
** @The notes :
**===========================================================================================*/
u16 GetFallDhgLimCurr(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    return(sFallDhgCurrLim[clunum]);
}
/*=============================================================================================
** @Name      : static void CVPIDCtrlChgCLim(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : PID调节充电电流控制电压稳定
** @The notes :
**===========================================================================================*/
static void CVPIDCtrlChgCLim(u8 clunum)
{
    //s32 aimVolt = 0;          /*目标电压*/
    u16 maxVolt = 0;            /*当前最大电压*/
    static u16 sHisMaxVolt[2] = {0}; /*上次最大电压*/
    if(clunum >= 2)
    {
        return;
    }
    /*当前最大电压*/
    maxVolt = GetGCellMaxVoltAPI(clunum);
    
    /*最大电压下降至严重小于起始压差或电压异常则重新开始恒压*/
    if(((maxVolt + CHG_FIN_CV_CLR_OFFV) < CVCalcChgPIDAimVolt(0))
        || (maxVolt >= (CalcGetChgLimCellVoltHook() + 500)))
    {
        sFallChgCIndex[clunum] = 0xff;
    }
    else
    {
        /*未开始降流恒压*/
        if(0xff == sFallChgCIndex[clunum])
        {
            /*电压到达*/
            if(maxVolt >= CVCalcChgPIDAimVolt(0))
            {
                /*开始降流恒压*/
                sFallChgCIndex[clunum] = 0;
                
                /*记录开始恒压时的电流值*/
                if(GetGSampOutCurrAPI(clunum) < 0)
                {
                    //当前实际电流小，降流起始点至少0.7C
                    if((0 - GetGSampOutCurrAPI(clunum)) <= (GetGBattAllCapAPI(clunum) * 7 / 10))
                    {
                    	//0.7C低于最大请求电流
                    	//if((GetGBattAllCapAPI() * 7 / 10) <= GetChgHReqMaxCurr())
                    	//{
                    		//降流点取0.7C
                    		sFallChgMaxCurr[clunum] = GetGBattAllCapAPI(clunum) * 7 / 10;
                    	//}
                    	//else
                    	//{
                    		//降流点取最大请求电流值
                    	//	sFallChgMaxCurr = GetChgHReqMaxCurr();
                    	//}
                    }
                    //实际电流超过0.7C
                    else
                    {
                    	//起始点取实际电流
                        sFallChgMaxCurr[clunum] = 0 - GetGSampOutCurrAPI(clunum);
                    }
                }
                else
                {
                    sFallChgMaxCurr[clunum] = 0;
                }
            }
        }
        /*已经开始恒压*/
        else
        {
            /*未调节到最小值*/
            if(sFallChgCIndex[clunum] < (SLOW_CURRLIMIT_NUM - 1))
            {
                /*恒压电流下降至目标电流且电压超过当前恒压点则抬升下一个恒压点*/
                /*当前电压超过下一个恒压点则抬升下一个恒压点*/
                if((maxVolt >= CVCalcChgPIDAimVolt(sFallChgCIndex[clunum] + 1))
                    && (sHisMaxVolt[clunum] >= CVCalcChgPIDAimVolt(sFallChgCIndex[clunum] + 1)))
                {
                    sFallChgCIndex[clunum]++;
                }
            }
            /*调节到最后一个恒压电*/
            else
            {
                /*保持*/
                sFallChgCIndex[clunum] = SLOW_CURRLIMIT_NUM - 1;
            }
        }
    }
    
    sHisMaxVolt[clunum] = maxVolt;

//    /*未开始恒压*/
//    if(0xff == sFallChgCIndex)
//    {
//        /*目标电压取第1个点*/
//        aimVolt = CVCalcChgPIDAimVolt(0);
//    }
//    /*已经开始恒压*/
//    else
//    {
//        /*计算目标电压*/
//        //aimVolt = CVCalcChgPIDAimVolt(sFallChgCIndex);/*恒压(根据电压PI调节电流)*/
//        aimVolt = CVCalcChgPIDAimVolt(sFallChgCIndex + 1);/*恒流(不调节电流)*/
//    }
//
//    /*检测电压差值值*/
//    gChgCurrPiIO.piIn = aimVolt - (s32)maxVolt;
//
//    /*根据电压差值进行PID调节电流限值*/
//    (void)UtilPIAdjust(&gChgCurrPiIO, &gChgCurrPICfg);
}
/*=============================================================================================
** @Name      : static void CVChgCurrLimPIDReset(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 恒压方式平滑充电电流限值PID参数复位
** @The notes :
**===========================================================================================*/
static void CVChgCurrLimPIDReset(u8 clunum)
{
    if(clunum >= 2)
    {
        return;
    }
    sFallChgCIndex[clunum] = 0xff;
    sFallChgMaxCurr[clunum] = 0;
    sFallChgCurrLim[clunum] = 0;
}

/*=============================================================================================
** @Name      : static u16 CVCalcChgPIDAimVolt(u8 index)
** @Input     : index:电流下标
** @Output    : PID目标电压:1mv
** @Function  : 计算恒压方式平滑充电电流限值PID目标电压
** @The notes : 从配置表查询
**===========================================================================================*/
static u16 CVCalcChgPIDAimVolt(u8 index)
{
    u8 tabNum = 0;
    u16 volt = 0;
    
    if(index >= SLOW_CURRLIMIT_NUM)
    {
        index = SLOW_CURRLIMIT_NUM - 1;
    }
    
    /*计算所在数组中对应下标*/
    tabNum = (SLOW_CURRLIMIT_NUM + 1) * 2 + 1 + index;
    
    /*根据压差计算目标电压*/
    volt = CalcGetChgLimCellVoltHook() - (gGroupParaRO_119[tabNum] * 10);
    
    return(volt);
}

/*=============================================================================================
** @Name      : static u16 CVCalcChgPIDAimCurr(u8 clunum, u8 index)
** @Input     : clunum:簇编号 index:电流下标
** @Output    : PID目标电流:0.1A
** @Function  : 计算恒压方式平滑充电电流限值PID目标电流
** @The notes : 从配置表查询
**===========================================================================================*/
static u16 CVCalcChgPIDAimCurr(u8 clunum, u8 index)
{
    u8 tabNum = 0;
    u16 curr = 0;
    if(clunum >= 2)
    {
        return (0);
    }
    if(index >= SLOW_CURRLIMIT_NUM)
    {
        index = SLOW_CURRLIMIT_NUM - 1;
    }
    
    /*计算所在数组中对应下标*/
    tabNum = 1 + index;
    
    /*记录恒压起始电流有效(必须大于最小限流值)*/
    if(sFallChgMaxCurr[clunum] >= ((u32)GetChgHReqMaxCurr() * gGroupParaRO_119[SLOW_CURRLIMIT_NUM] / 100))
    {
        /*根据恒压起始电流和当前限制倍率计算目标电流*/
        curr = (u16)((u32)sFallChgMaxCurr[clunum] * gGroupParaRO_119[tabNum] / 100);
    }
    /*未记录恒压起始电流或恒压起始点电流很小*/
    else
    {
        /*根据最大允许电流和当前限制倍率计算目标电流*/
        curr = (u16)((u32)GetChgHReqMaxCurr() * gGroupParaRO_119[tabNum] / 100);
    }
    
    /*最小限1A*/
    if(curr < 10)
    {
        curr = 10;
    }
    
    return(curr);
}
/*=============================================================================================
** @Name      : static void CVCalcDhgCLimDnStep(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 计算放电末端恒压放电电流限值下降步骤
** @The notes :
**===========================================================================================*/
static void CVCalcDhgCLimDnStep(u8 clunum)
{
    u16 minVolt = 0;           /*当前最小电压*/
    static u16 sHisMinVolt[2] = {0};/*上次最小电压*/

    if(clunum >= 2)
    {
        return;
    }

    /*当前最低电压*/
    minVolt = GetGCellMinVoltAPI(clunum);

    /*最小电压上升至严重大于起始压差或电压异常则重新开始恒压*/
    if((minVolt > (CVCalcDhgStepDnAimVolt(0) + DHG_FIN_CV_CLR_OFFV))
        || ((minVolt + 500) <= CalcGetDhgLimCellVoltHook()))
    {
        sFallDhgCIndex[clunum] = 0xff;
    }
    else
    {
        /*未开始降流恒压*/
        if(0xff == sFallDhgCIndex[clunum])
        {
            /*电压到达*/
            if(minVolt <= CVCalcDhgStepDnAimVolt(0))
            {
                /*开始降流恒压*/
                sFallDhgCIndex[clunum] = 0;

                /*记录开始恒压时的电流值*/
                if(GetGSampOutCurrAPI(clunum) > 0)
                {
                    //当前实际电流小，降流起始点至少0.7C
                    if(GetGSampOutCurrAPI(clunum) <= (GetGBattAllCapAPI(clunum) * 7 / 10))
                    {
                    	//0.7C低于最大请求电流
                    	//if((GetGBattAllCapAPI() * 7 / 10) <= GetDhgHReqMaxCurr())
                    	//{
                    		//降流点取0.7C
                    		sFallDhgMaxCurr[clunum] = GetGBattAllCapAPI(clunum) * 7 / 10;
                    	//}
                    	//else
                    	//{
                    		//降流点取最大请求电流值
                    	//	sFallDhgMaxCurr = GetDhgHReqMaxCurr();
                    	//}
                    }
                    //实际电流超过0.7C
                    else
                    {
                    	//起始点取实际电流
                    	sFallDhgMaxCurr[clunum] = GetGSampOutCurrAPI(clunum);
                    }
                }
                else
                {
                    sFallDhgMaxCurr[clunum] = 0;
                }
            }
        }
        /*已经开始恒压*/
        else
        {
            /*未调节到最小值*/
            if(sFallDhgCIndex[clunum] < (SLOW_CURRLIMIT_NUM - 1))
            {
                /*恒压电流下降至目标电流且电压下降至当前恒压点则抬升下一个恒压点*/
                /*当前电压下降至下一个恒压点则抬升下一个恒压点*/
                if((minVolt <= CVCalcDhgStepDnAimVolt(sFallDhgCIndex[clunum] + 1))
                    && (sHisMinVolt[clunum] <= CVCalcDhgStepDnAimVolt(sFallDhgCIndex[clunum] + 1)))
                {
                    sFallDhgCIndex[clunum]++;
                }
            }
            /*调节到最后一个恒压电*/
            else
            {
                /*保持*/
                sFallDhgCIndex[clunum] = SLOW_CURRLIMIT_NUM - 1;
            }
        }
    }

    sHisMinVolt[clunum] = minVolt;
}
/*=============================================================================================
** @Name      : static void CVDhgCurrLimStepDnReset(void)
** @Input     : void
** @Output    : void
** @Function  : 恒压方式平滑放电电流限值步长降流参数复位
** @The notes :
**===========================================================================================*/
static void CVDhgCurrLimStepDnReset(u8 clunum)
{
    if(clunum >= 2)
    {
        return;
    }
    sFallDhgCIndex[clunum] = 0xff;
    sFallDhgMaxCurr[clunum] = 0;
    sFallDhgCurrLim[clunum] = 0;
}
/*=============================================================================================
** @Name      : static u16 CVCalcDhgStepDnAimVolt(u8 index)
** @Input     : index:电流下标
** @Output    : 降流恒压目标电压:1mv
** @Function  : 计算恒压方式平滑放电电流限值降流恒压目标电压
** @The notes : 从配置表查询
**===========================================================================================*/
static u16 CVCalcDhgStepDnAimVolt(u8 index)
{
    u8 tabNum = 0;
    u16 volt = 0;

    if(index >= SLOW_CURRLIMIT_NUM)
    {
        index = SLOW_CURRLIMIT_NUM - 1;
    }

    /*计算所在数组中对应下标*/
    tabNum = (SLOW_CURRLIMIT_NUM + 1) * 3 + 1 + index;

    /*根据压差计算目标电压*/
    volt = CalcGetDhgLimCellVoltHook() + (gGroupParaRO_119[tabNum] * 10);

    return(volt);
}
/*=============================================================================================
** @Name      : static u16 CVCalcDhgStepDnAimCurr(u8 clunum, u8 index)
** @Input     : clunum:簇编号 index:电流下标
** @Output    : PID目标电流:0.1A
** @Function  : 计算恒压方式平滑放电电流限值PID目标电流
** @The notes : 从配置表查询
**===========================================================================================*/
static u16 CVCalcDhgStepDnAimCurr(u8 clunum, u8 index)
{
    u8 tabNum = 0;
    u16 curr = 0;

    if(clunum >= 2)
    {
        return (0);
    }
    if(index >= SLOW_CURRLIMIT_NUM)
    {
        index = SLOW_CURRLIMIT_NUM - 1;
    }

    /*计算所在数组中对应下标*/
    tabNum = SLOW_CURRLIMIT_NUM + 2 + index;

    /*记录恒压起始电流有效(必须大于最小限流值)*/
    if(sFallDhgMaxCurr[clunum] >= ((u32)GetDhgHReqMaxCurr() * gGroupParaRO_119[SLOW_CURRLIMIT_NUM * 2 + 1 ] / 100))
    {
        /*根据恒压起始电流和当前限制倍率计算目标电流*/
        curr = (u16)((u32)sFallDhgMaxCurr[clunum] * gGroupParaRO_119[tabNum] / 100);
    }
    /*未记录恒压起始电流或恒压起始点电流很小*/
    else
    {
        /*根据最大允许电流和当前限制倍率计算目标电流*/
        curr = (u16)((u32)GetDhgHReqMaxCurr() * gGroupParaRO_119[tabNum] / 100);
    }

    /*最小限1A*/
    if(curr < 10)
    {
        curr = 10;
    }

    return(curr);
}
