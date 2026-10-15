/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CurrLimCalc.c
** @Founder      : ZQD808
** @Date         : 2026.03.12
** @Function     : 充放电电流限值计算模块
** @Instructions :
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "CalcUtil.h"
#include "GroupPara.h"
#include "GroupInfo.h"
#include "PublicInfo.h"
//#include "EnerInterface.h"
#include "CalcInterface.h"
#include "CharactCalc.h"
#include "CapInfoCalc.h"
#include "CurrLimFall.h"
#include "CurrLimCalc.h"
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>
#include <stdio.h>
#include <debug.h>



extern s16 *gSysMainInfo_51;            /*系统整体信息*/
extern u16 *gSysLimInfo_53;             /*系统限值信息*/
extern s16 *gSysCharInfo_54;            /*系统特征信息*/
extern u16 gGBmuGenPara_102[];          /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];       /*主控高级参数值*/

extern u8 gGroupParaRO_115[];           /*不同温度充电倍率表*/
extern u8 gGroupParaRO_116[];           /*不同温度放电倍率表*/

/*=============================================================================================
** @Global variable definition
**===========================================================================================*/
const s16 gTempCurLimitTable[TEMP_CURRLIMIT_NUM] =      //充放电倍率温度表
{
    -10, -5, 0, 5, 10, 15, 20, 25, 35, 40, 45, 50, 55, 60, 70
};

/*=============================================================================================
** @Name      : void CurrLimCalcTask(void)
** @Input     : void
** @Output    : void
** @Function  : 充放电电流限值计算任务
** @The notes : 100ms
**===========================================================================================*/
void CurrLimCalcTask(void)
{
    u16 chgHCurr   = 0;         /*充电过流值*/
    u16 dhgHCurr   = 0;         /*放电过流值*/
    u16 tempMulLim = 0;         /*当前温度充电倍率限值*/
    u16 chUpLim    = 0;         /*充电电流限值*/
    u16 dhUpLim    = 0;         /*放电电流限值*/
    
    s16 nowTemp  = 0;           /*当前温度最大值*/
    s16 fstTemp  = 0;           /*降限值起始温度*/
    s16 lstTemp  = 0;           /*降限值结束温度*/
    if(gSysLimInfo_53 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysLimInfo_53 null\r\n", __func__);
        return;
    }
    /*平滑电流限值任务*/
    CurrLimFallTask();                             /*计算平滑的电流限值*/


    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        chgHCurr   = 0;        
        dhgHCurr   = 0;        
        tempMulLim = 0;        
        chUpLim    = 0;         
        dhUpLim    = 0;         
        nowTemp  = 0;          
        fstTemp  = 0;           
        lstTemp  = 0;          
        /*系统正常才进行充放电电流限值计算*/
        if((GetGWorkStateAPI(clunum) != eWORK_RUN)
            && (GetGWorkStateAPI(clunum) != eWORK_IDLE)
            && (GetGWorkStateAPI(clunum) != eWORK_START)
            && (GetGWorkStateAPI(clunum) != eWORK_STOP))
        {
            if(clunum == 0)
            {
                gSysLimInfo_53[eSysLimInfo53_1ChgC] = 0;
                gSysLimInfo_53[eSysLimInfo53_1DhgC] = 0;
                gSysLimInfo_53[eSysLimInfo53_1ChgP] = 0;
                gSysLimInfo_53[eSysLimInfo53_1DhgP] = 0;
            }
            else
            {
                gSysLimInfo_53[eSysLimInfo53_2ChgC] = 0;
                gSysLimInfo_53[eSysLimInfo53_2DhgC] = 0;
                gSysLimInfo_53[eSysLimInfo53_2ChgP] = 0;
                gSysLimInfo_53[eSysLimInfo53_2DhgP] = 0;
            }
            continue;
        }
        
        if(1 == CalcGetGropuNeedSysLimStateHook(clunum))      /*产生需充放电限制状态(系统告警或按钮停机)*/
        {
            if(clunum == 0)
            {
                gSysLimInfo_53[eSysLimInfo53_1ChgC] = 0;
                gSysLimInfo_53[eSysLimInfo53_1DhgC] = 0;
                gSysLimInfo_53[eSysLimInfo53_1ChgP] = 0;
                gSysLimInfo_53[eSysLimInfo53_1DhgP] = 0;
            }
            else
            {
                gSysLimInfo_53[eSysLimInfo53_2ChgC] = 0;
                gSysLimInfo_53[eSysLimInfo53_2DhgC] = 0;
                gSysLimInfo_53[eSysLimInfo53_2ChgP] = 0;
                gSysLimInfo_53[eSysLimInfo53_2DhgP] = 0;
            }
            continue;
        }
        
        /*获取过流限值可用比率下最大限流值*/
        chgHCurr = GetChgHReqMaxCurr();                 /*充电过流限值*/
        dhgHCurr = GetDhgHReqMaxCurr();                 /*放电过流限值*/
        
        /*取当前温度下充放电倍率和过流值两者中较小值*/
        tempMulLim = CalcTempChgMulCurr(clunum);              /*获得当前温度充电倍率下电流限值*/
        
        if(tempMulLim > chgHCurr)                       /*充电倍率限值大于充电过流值*/
        {
            chUpLim = chgHCurr;                         /*取较小的过流限值*/
        }
        else
        {
            chUpLim = tempMulLim;                       /*取充电倍率限值和过流值中较小值*/
        }
        
        tempMulLim = CalcTempDhgMulCurr(clunum);              /*获得当前温度放电倍率下电流限值*/
        
        if(tempMulLim > dhgHCurr)                       /*放电倍率限值大于放电过流值*/
        {
            dhUpLim = dhgHCurr;                         /*取较小的过流限值*/
        }
        else
        {
            dhUpLim = tempMulLim;                       /*取放电倍率限值和过流值中较小值*/
        }
        
        /*充满或充电限制故障时限制充电*/
        if(1 == CalcGetGroupNeedChgLimStateHook(clunum))
        {
            chUpLim = 0;                                /*充电限值为0,不允许充电*/
        }
        /*计算温度平滑后充电限值*/
        else
        {
            /*高温降功率*/
            if(((GetGCellMaxTempAPI(clunum) + 5) / 10 >= (CalcGetChgCDnFstHTempHook() - 3))  /*到达充电高温降限值起始点温度(回差3℃)*/
                && (1 == gGBmuHigLevPara_103[eBmuHigLevPara103_ChgTHLimEn]))/*充电高温降功率使能*/
            {
                nowTemp = (GetGCellMaxTempAPI(clunum) + 5) / 10;
                fstTemp = CalcGetChgCDnFstHTempHook();
                lstTemp = CalcGetChgCDnLstHTempHook();
                
                /*通过高温降充电电流限值*/
                chUpLim = ChgLimDownByTempH(clunum, fstTemp, lstTemp, nowTemp, chUpLim);

                /*充电限流仍大于最低电流*/
                if(chUpLim > gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnMinC])
                {
                    /*通过温差降充电电流限值*/
                    chUpLim = ChgLimDownBySubTemp(clunum, chUpLim);
                }

                //温度低时电流不能低于最小大电流
                if((chUpLim < gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnMinC])
                    && (nowTemp < ((s16)gGBmuGenPara_102[eBmuGenPara102_ChgTH2Lim] - 5)))
                {
                    chUpLim = gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnMinC];
                }
            }
            #if(0)
            /*低温降功率*/
            else if((GetGCellMinTempAPI() <= (CalcGetChgCDnFstLTempHook() + 3))/*到达充电低温降限值起始点温度(回差3℃)*/
                && (1 == gGBmuHigLevPara_103[eBmuHigLevPara103_ChgTLLimEn]))/*充电低温降功率使能*/
            {
                nowTemp = GetGCellMinTempAPI();
                fstTemp = CalcGetChgCDnFstLTempHook();
                lstTemp = CalcGetChgCDnLstLTempHook();

                chUpLim = CurrLimDownByTempL(fstTemp, lstTemp, nowTemp, chUpLim);
            }
            #endif

            chUpLim = ChgLimCurrByPowerLim(clunum, chUpLim);    /*通过功率限制后获得充电电流限值*/
        }
        
        /*计算电压/SOC平滑后的充电电流限值*/
        tempMulLim = GetFallChgLimCurr(clunum);               /*获得平滑计算的充电电流限值*/
        
        if(tempMulLim < chUpLim)                        /*平滑计算的电流小于限值*/
        {
            chUpLim = tempMulLim;                       /*取较小的过流限值*/
        }
        
        /*放空或放电限制故障时限制放电*/
        if(1 == CalcGetGroupNeedDhgLimStateHook(clunum))
        {
            dhUpLim = 0;                                /*放电限值为0,不允许放电*/
        }
        /*计算温度平滑后放电限值*/
        else
        {
            /*高温降功率*/
            if((((GetGCellMaxTempAPI(clunum) + 5) / 10) >= (CalcGetDhgCDnFstHTempHook() - 3))  /*到达放电高温降限值起始点温度(回差3℃)*/
                && (1 == gGBmuHigLevPara_103[eBmuHigLevPara103_DhgTHLimEn]))/*放电高温降功率使能*/
            {
                nowTemp = (GetGCellMaxTempAPI(clunum) + 5) / 10;
                fstTemp = CalcGetDhgCDnFstHTempHook();
                lstTemp = CalcGetDhgCDnLstHTempHook();
                
                dhUpLim = DhgLimDownByTempH(clunum, fstTemp, lstTemp, nowTemp, dhUpLim);

                /*放电限流仍大于最低电流*/
                if(dhUpLim > gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnMinC])
                {
                    /*通过温差降放电电流限值*/
                    dhUpLim = DhgLimDownBySubTemp(clunum, dhUpLim);
                }

                //温度低时电流不能低于最小电流
                if((dhUpLim < gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnMinC])
                    && (nowTemp < ((s16)gGBmuGenPara_102[eBmuGenPara102_DhgTH2Lim] - 10)))
                {
                    dhUpLim = gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnMinC];
                }
            }
            #if(0)
            /*低温降功率*/
            else if((GetGCellMinTempAPI() <= (CalcGetDhgCDnFstLTempHook() + 3))/*到达放电低温降限值起始点温度(回差3℃)*/
                && (1 == gGBmuHigLevPara_103[eBmuHigLevPara103_DhgTLLimEn]))/*放电低温降功率使能*/
            {
                nowTemp = GetGCellMinTempAPI();
                fstTemp = CalcGetDhgCDnFstLTempHook();
                lstTemp = CalcGetDhgCDnLstLTempHook();

                dhUpLim = CurrLimDownByTempL(fstTemp, lstTemp, nowTemp, dhUpLim);
            }
            #endif
            
            dhUpLim = DhgLimCurrByPowerLim(clunum, dhUpLim);    /*通过功率限制后获得放电电流限值*/
        }
        
        /*计算电压/SOC平滑后的放电电流限值*/
        tempMulLim = GetFallDhgLimCurr(clunum);               /*获得平滑计算的放电电流限值*/
        
        if(tempMulLim < dhUpLim)                        /*平滑计算的电流小于限值*/
        {
            dhUpLim = tempMulLim;                       /*取较小的过流限值*/
        }
        if(clunum == 0)
        {
            gSysLimInfo_53[eSysLimInfo53_1ChgC] = chUpLim;
            gSysLimInfo_53[eSysLimInfo53_1DhgC] = dhUpLim;
        }
        else
        {
            gSysLimInfo_53[eSysLimInfo53_2ChgC] = chUpLim;
            gSysLimInfo_53[eSysLimInfo53_2DhgC] = dhUpLim;
        }
        

        if(clunum == 0)
        {
            /*非运行状态限制功率为0*/
            if(eWORK_RUN == GetGWorkStateAPI(clunum))
            {
                gSysLimInfo_53[eSysLimInfo53_1ChgP] = (u16)(((u32)chUpLim * GetGSampSumVoltAPI(clunum) + 5000) / 10000);
                gSysLimInfo_53[eSysLimInfo53_1DhgP] = (u16)(((u32)dhUpLim * GetGSampSumVoltAPI(clunum) + 5000) / 10000);
            }
            else
            {
                gSysLimInfo_53[eSysLimInfo53_1ChgP] = 0;
                gSysLimInfo_53[eSysLimInfo53_1DhgP] = 0;
            }
        }
        else
        {
            /*非运行状态限制功率为0*/
            if(eWORK_RUN == GetGWorkStateAPI(clunum))
            {
                gSysLimInfo_53[eSysLimInfo53_2ChgP] = (u16)(((u32)chUpLim * GetGSampSumVoltAPI(clunum) + 5000) / 10000);
                gSysLimInfo_53[eSysLimInfo53_2DhgP] = (u16)(((u32)dhUpLim * GetGSampSumVoltAPI(clunum) + 5000) / 10000);
            }
            else
            {
                gSysLimInfo_53[eSysLimInfo53_2ChgP] = 0;
                gSysLimInfo_53[eSysLimInfo53_2DhgP] = 0;
            }
        }
    }
}

/*=============================================================================================
** @Name      : u16 GetChgHReqMaxCurr(void)
** @Input     : void
** @Output    : 充电过流值下允许的最大电流限值 0.1A
** @Function  : 返回充电过流值下允许的最大电流限值
** @The notes : 配置值
**===========================================================================================*/
u16 GetChgHReqMaxCurr(void)
{
    u16 curr = 0;
    
    curr = gGBmuGenPara_102[eBmuGenPara102_ChgCMaxLim];
    
    /*最大允许电流不能超过过流告警值*/
    if(curr >= gGBmuGenPara_102[eBmuGenPara102_ChgCH2Lim])
    {
        if(gGBmuGenPara_102[eBmuGenPara102_ChgCH2Lim] > 20)
        {
            /*允许值比告警值小1A*/
            curr = gGBmuGenPara_102[eBmuGenPara102_ChgCH2Lim] - 10;
            //curr = gGBmuGenPara_102[eBmuGenPara102_ChgCH2Lim];
        }
        else
        {
            /*允许值为限值*/
            curr = gGBmuGenPara_102[eBmuGenPara102_ChgCH2Lim];
        }
    }
    
    return(curr);
}
/*=============================================================================================
** @Name      : u16 GetDhgHReqMaxCurr(void)
** @Input     : void
** @Output    : 放电过流值下允许的最大电流限值 0.1A
** @Function  : 返回放电过流值下允许的最大电流限值
** @The notes : 配置值
**===========================================================================================*/
u16 GetDhgHReqMaxCurr(void)
{
    u16 curr = 0;
    
    curr = gGBmuGenPara_102[eBmuGenPara102_DhgCMaxLim];
    
    /*最大允许电流不能超过过流告警值*/
    if(curr >= gGBmuGenPara_102[eBmuGenPara102_DhgCH2Lim])
    {
        if(gGBmuGenPara_102[eBmuGenPara102_DhgCH2Lim] > 20)
        {
            /*允许值比告警值小1A*/
            curr = gGBmuGenPara_102[eBmuGenPara102_DhgCH2Lim] - 10;
        }
        else
        {
            /*允许值为限值*/
            curr = gGBmuGenPara_102[eBmuGenPara102_DhgCH2Lim];
        }
    }
    
    return(curr);
}
/*=============================================================================================
** @Name      : static u16 CalcTempChgMulCurr(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 当前温度充电倍率下的电流限值 0.1A
** @Function  : 计算当前温度充电倍率下的电流限值
** @The notes : 查表
**===========================================================================================*/
static u16 CalcTempChgMulCurr(u8 clunum)
{
    u8 index = 0;
    s16 temp = 0;
    u32 result = 0;
    static u8 sHisIndex[2] = {0};
    if(clunum >= 2)
    {
        return(0);
    }
    
    temp   = (GetGCellAvgTempAPI(clunum) + 5) / 10;
    index  = SearchHigFromUpTabs16(temp, gTempCurLimitTable, TEMP_CURRLIMIT_NUM);//取高端(高温度)
    if(index >= TEMP_CURRLIMIT_NUM)
    {
        index = TEMP_CURRLIMIT_NUM - 1;
    }

    //索引变化1则防止温度变化1℃进入另一温度区间导致电流限值反复跳变
    if(((index == (sHisIndex[clunum] + 1)) && (temp == (gTempCurLimitTable[sHisIndex[clunum]] + 1)))//温度上升1℃
        /*|| (((index + 1) == sHisIndex) && (temp == gTempCurLimitTable[index]))*/) //温度下降1℃
    {
        //保持上次限值(直到温度变化2℃)
        index = sHisIndex[clunum];
    }
    //无变化或变化区间大
    else
    {
        sHisIndex[clunum] = index;
    }

    result = (u32)gGroupParaRO_115[index] * GetGroupStandCapAPI(clunum) / 10000; 
    return((u16)result);
}

/*=============================================================================================
** @Name      : static u16 CalcTempDhgMulCurr(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 当前温度放电倍率下的电流限值 0.1A
** @Function  : 计算当前温度放电倍率下的电流限值
** @The notes : 查表
**===========================================================================================*/
static u16 CalcTempDhgMulCurr(u8 clunum)
{
    u8 index = 0;
    s16 temp = 0;
    u32 result = 0;
    static u8 sHisIndex[2] = {0};
    if(clunum >= 2)
    {
        return(0);
    }
    
    temp  = (GetGCellAvgTempAPI(clunum) + 5) / 10;
    index = SearchHigFromUpTabs16(temp, gTempCurLimitTable, TEMP_CURRLIMIT_NUM);//取高端(高温度)
    if(index >= TEMP_CURRLIMIT_NUM)
    {
        index = TEMP_CURRLIMIT_NUM - 1;
    }

    //索引变化1则防止温度变化1℃进入另一温度区间导致电流限值反复跳变
    if(((index == (sHisIndex[clunum] + 1)) && (temp == (gTempCurLimitTable[sHisIndex[clunum]] + 1)))//温度上升1℃
        /*|| (((index + 1) == sHisIndex) && (temp == gTempCurLimitTable[index]))*/) //温度下降1℃
    {
        //保持上次限值(直到温度变化2℃)
        index = sHisIndex[clunum];
    }
    //无变化或变化区间大
    else
    {
        sHisIndex[clunum] = index;
    }

    /*
    if((temp < -20) && (SocSlowlyValueHook() < 50))                             //-20℃以下SOC小于5%
    {
        result = 0;                                                             //禁止放电
    }
    else if((temp < 0) && (SocSlowlyValueHook() < 50))                          //0℃以下SOC小于5%
    {
        result = (u32)gEepCfg_105[TEMP_CURRLIMIT_NUM + 2 + index] * GetGroupStandCapAPI() / 1000;
        
        if(result > 300)                                                        //限最大30A放电
        {
            result = 300;
        }
    }
    else
    {
        result = (u32)gEepCfg_105[TEMP_CURRLIMIT_NUM + 2 + index] * GetGroupStandCapAPI() / 1000;
    }
    */

    result = (u32)gGroupParaRO_116[index] * GetGroupStandCapAPI(clunum) / 10000;
    return((u16)result);
}

/*=============================================================================================
** @Name      : static u16 ChgLimDownByTempH(u8 clunum, s16 startTemp, s16 endTemp, s16 nowTemp, u16 currLim)
** @Input     : clunum:簇编号 startTemp:开始降限值起始温度 endTemp:降限值为0的温度 nowTemp:当前温度  currLim:当前限值
** @Output    : 经高温降后的电流限值 0.1A
** @Function  : 根据高温降充电电流限值
** @The notes : 查表
**===========================================================================================*/
static u16 ChgLimDownByTempH(u8 clunum, s16 startTemp, s16 endTemp, s16 nowTemp, u16 currLim)
{
    u16 dowmCur = 0;                                        /*经高温降后的电流限值*/
    static s16 sHisTemp[2] = {0};                                /*上次温度值*/
    
    if(clunum >= 2)
    {
        return (0);
    }
    if((startTemp >= endTemp)                               /*起止温度错误*/
        || ((nowTemp < startTemp)                           /*当前最大温度未达到降限值温度*/
        && (sHisTemp[clunum] < startTemp)))                         /*之前温度未达到过降限值温度*/
    {
        sHisTemp[clunum] = nowTemp;
        return(currLim);                                    /*不降限值*/
    }
    
    if(nowTemp > endTemp)                                   /*当前温度值超过电流限值为零时温度值*/
    {
        nowTemp = endTemp;
    }

    if((nowTemp < sHisTemp[clunum]) && ((nowTemp + 2) >= sHisTemp[clunum])) /*温度跳动在2℃内(回差3℃)*/
    {
        nowTemp = sHisTemp[clunum];                                 /*当做温度未变化处理*/
    }
    else if((nowTemp < sHisTemp[clunum]) && ((nowTemp + 3) >= sHisTemp[clunum])) /*温度跳动在3℃内(回差4℃)*/
    {
        nowTemp += 1;
        sHisTemp[clunum] = nowTemp;                                 /*记录温度值高1℃(防止恢复后温度马上升1度跳变)*/
    }
    else
    {
        sHisTemp[clunum] = nowTemp;                                 /*记录当次计算温度值*/
    }

    dowmCur = ((u16)(endTemp - nowTemp) * currLim) / (u16)(endTemp - startTemp + 1);  /*按当前温度超过一般过温限值比率降电流限值*/

    return(dowmCur);
}
/*=============================================================================================
** @Name      : static u16 DhgLimDownByTempH(u8 clunum, s16 startTemp, s16 endTemp, s16 nowTemp, u16 currLim)
** @Input     : clunum:簇编号 startTemp:开始降限值起始温度 endTemp:降限值为0的温度 nowTemp:当前温度  currLim:当前限值
** @Output    : 经高温降后的电流限值 0.1A
** @Function  : 根据高温降放电电流限值
** @The notes : 查表
**===========================================================================================*/
static u16 DhgLimDownByTempH(u8 clunum, s16 startTemp, s16 endTemp, s16 nowTemp, u16 currLim)
{
    u16 dowmCur = 0;                                        /*经高温降后的电流限值*/
    static s16 sHisTemp[2] = {0};                                /*上次温度值*/
    if(clunum >= 2)
    {
        return (0);
    }
    if((startTemp >= endTemp)                               /*起止温度错误*/
        || ((nowTemp < startTemp)                           /*当前最大温度未达到降限值温度*/
        && (sHisTemp[clunum] < startTemp)))                         /*之前温度未达到过降限值温度*/
    {
        sHisTemp[clunum] = nowTemp;
        return(currLim);                                    /*不降限值*/
    }

    if(nowTemp > endTemp)                                   /*当前温度值超过电流限值为零时温度值*/
    {
        nowTemp = endTemp;
    }

    if((nowTemp < sHisTemp[clunum]) && ((nowTemp + 2) >= sHisTemp[clunum])) /*温度跳动在2℃内(回差3℃)*/
    {
        nowTemp = sHisTemp[clunum];                                 /*当做温度未变化处理*/
    }
    else if((nowTemp < sHisTemp[clunum]) && ((nowTemp + 3) >= sHisTemp[clunum])) /*温度跳动在3℃内(回差4℃)*/
    {
        nowTemp += 1;
        sHisTemp[clunum] = nowTemp;                                 /*记录温度值高1℃(防止恢复后温度马上升1度跳变)*/
    }
    else
    {
        sHisTemp[clunum] = nowTemp;                                 /*记录当次计算温度值*/
    }

    dowmCur = ((u16)(endTemp - nowTemp) * currLim) / (u16)(endTemp - startTemp + 1);  /*按当前温度超过一般过温限值比率降电流限值*/

    return(dowmCur);
}

/*=============================================================================================
** @Name      : static u16 ChgLimDownBySubTemp(u8 clunum, u16 currLim)
** @Input     : clunum:簇编号 currLim:当前限值 0.1A
** @Output    : 经温差降后的电流限值 0.1A
** @Function  : 根据温差降充电电流限值
** @The notes : 高温预警前只降1梯度,预警后降2梯度
**===========================================================================================*/
static u16 ChgLimDownBySubTemp(u8 clunum, u16 currLim)
{
    u16 dowmCur = 0;                                        /*经温差降后的电流限值*/
    s8 nowTemp = 0;                                         /*当前温度值(最高温)*/
    u8 subTemp = 0;                                         /*当前温差值*/
    static s16 sHisTemp[2] = {0};                                /*上次温度值*/
    static u8 sHisSubTemp[2] = {0};                              /*上次温差值*/

    if(clunum >= 2)
    {
        return (0);
    }

    nowTemp = (GetGCellMaxTempAPI(clunum) + 5) / 10;
    if((nowTemp < sHisTemp[clunum]) && ((nowTemp + 2) >= sHisTemp[clunum])) /*温度跳动在2℃内(回差3℃)*/
    {
        nowTemp = sHisTemp[clunum];                                 /*当做温度未变化处理*/
    }
    else if((nowTemp < sHisTemp[clunum]) && ((nowTemp + 3) >= sHisTemp[clunum])) /*温度跳动在3℃内(回差4℃)*/
    {
        nowTemp += 1;
        sHisTemp[clunum] = nowTemp;                                 /*记录温度值高1℃(防止恢复后温度马上升1度跳变)*/
    }
    else
    {
        sHisTemp[clunum] = nowTemp;                                 /*记录当次计算温度值*/
    }

    subTemp = ((GetGCellMaxTempAPI(clunum) + 5) / 10) - ((GetGCellMinTempAPI(clunum) + 5) / 10);

    //温差变小
    if((subTemp < sHisSubTemp[clunum]) && ((subTemp + 2) >= sHisSubTemp[clunum])) /*温差跳动在2℃内(回差3℃)*/
    {
        subTemp = sHisSubTemp[clunum];                              /*当做温差未变化处理*/
    }
    else if((subTemp < sHisSubTemp[clunum]) && ((subTemp + 3) >= sHisSubTemp[clunum])) /*温差跳动在3℃内(回差4℃)*/
    {
        subTemp += 1;
        sHisSubTemp[clunum] = subTemp;                              /*记录回差值大1℃(防止恢复后温差马上升1度跳变)*/
    }
    else
    {
        sHisSubTemp[clunum] = subTemp;                              /*记录当次计算温差值*/
    }

    //达到降限值温差
    if(subTemp >= gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnDifT])
    {
        //超过降限值温度
        if(nowTemp >= (gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnFstT] + 3))
        {
            //降2个梯度
            dowmCur = currLim - ((u32)currLim * gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnRate] * 2 / 1000);
        }
        //达到降限值温度
        else if(nowTemp >= (CalcGetChgCDnFstHTempHook() - 1))
        {
            //只降1个梯度
            dowmCur = currLim - ((u32)currLim * gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnRate] / 1000);
        }
        //未达到降限值温度
        else
        {
            //记录当前温差
            sHisSubTemp[clunum] = subTemp;
            //无需降限值
            dowmCur = currLim;
        }

        //限流不能低于最低电流
        if(dowmCur < gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnMinC])
        {
            dowmCur = gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnMinC];
        }
    }
    else
    {
        //无需降限值
        dowmCur = currLim;
    }

    return(dowmCur);
}
/*=============================================================================================
** @Name      : static u16 DhgLimDownBySubTemp(u8 clunum, u16 currLim)
** @Input     : clunum:簇编号 currLim:当前限值 0.1A
** @Output    : 经温差降后的电流限值 0.1A
** @Function  : 根据温差降放电电流限值
** @The notes : 高温预警前只降1梯度,预警后降2梯度
**===========================================================================================*/
static u16 DhgLimDownBySubTemp(u8 clunum, u16 currLim)
{
    u16 dowmCur = 0;                                        /*经温差降后的电流限值*/
    s8 nowTemp = 0;                                         /*当前温度值(最高温)*/
    u8 subTemp = 0;                                         /*当前温差值*/
    static s16 sHisTemp[2] = {0};                                /*上次温度值*/
    static u8 sHisSubTemp[2] = {0};                              /*上次温差值*/

    if(clunum >= 2)
    {
        return (0);
    }

    nowTemp = (GetGCellMaxTempAPI(clunum) + 5) / 10;
    if((nowTemp < sHisTemp[clunum]) && ((nowTemp + 2) >= sHisTemp[clunum])) /*温度跳动在2℃内(回差3℃)*/
    {
        nowTemp = sHisTemp[clunum];                                 /*当做温度未变化处理*/
    }
    else if((nowTemp < sHisTemp[clunum]) && ((nowTemp + 3) >= sHisTemp[clunum])) /*温度跳动在3℃内(回差4℃)*/
    {
        nowTemp += 1;
        sHisTemp[clunum] = nowTemp;                                 /*记录温度值高1℃(防止恢复后温度马上升1度跳变)*/
    }
    else
    {
        sHisTemp[clunum] = nowTemp;                                 /*记录当次计算温度值*/
    }

    subTemp = ((GetGCellMaxTempAPI(clunum) + 5) / 10) - ((GetGCellMinTempAPI(clunum) + 5) / 10);

    //温差变小
    if((subTemp < sHisSubTemp[clunum]) && ((subTemp + 2) >= sHisSubTemp[clunum])) /*温差跳动在2℃内(回差3℃)*/
    {
        subTemp = sHisSubTemp[clunum];                              /*当做温差未变化处理*/
    }
    else if((subTemp < sHisSubTemp[clunum]) && ((subTemp + 3) >= sHisSubTemp[clunum])) /*温差跳动在3℃内(回差4℃)*/
    {
        subTemp += 1;
        sHisSubTemp[clunum] = subTemp;                              /*记录回差值大1℃(防止恢复后温差马上升1度跳变)*/
    }
    else
    {
        sHisSubTemp[clunum] = subTemp;                              /*记录当次计算温差值*/
    }

    //达到降限值温差
    if(subTemp >= gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnDifT])
    {
        //超过降限值温度
        if(nowTemp >= (gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnFstT] + 3))
        {
            //2℃降1个梯度
            dowmCur = currLim - ((u32)currLim * gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnRate] * 2 / 1000);
        }
        //达到降限值温度
        else if(nowTemp >= (CalcGetDhgCDnFstHTempHook() - 1))
        {
            //只降1个梯度
            dowmCur = currLim - ((u32)currLim * gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnRate] / 1000);
        }
        //未达到降限值温度
        else
        {
            //记录当前温差
            sHisSubTemp[clunum] = subTemp;
            //无需降限值
            dowmCur = currLim;
        }

        //限流不能低于最低电流
        if(dowmCur < gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnMinC])
        {
            dowmCur = gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnMinC];
        }
    }
    else
    {
        //无需降限值
        dowmCur = currLim;
    }

    return(dowmCur);
}

/*=============================================================================================
** @Name      : static u16 ChgLimCurrByPowerLim(u8 clunum, u16 chgUpCur)
** @Input     : clunum:簇编号 chgUpCur 当前充电电流限值上限:0.1A
** @Output    : 经过充电功率限制后的充电电流限值:0.1A
** @Function  : 计算通过充电功率限制后的充电电流限值
** @The notes :
**===========================================================================================*/
static u16 ChgLimCurrByPowerLim(u8 clunum, u16 chgUpCur)
{
    u16 chgLimit = 0;                           /*充电电流限值*/
    u32 chgPowerCur = 0;                        /*按功率限值计算的电流限值*/
    u32 sumVolt = 0;                            /*组总电压*/
    
    if(clunum >= 2)
    {
        return (0);
    }
    sumVolt = GetGCellSumVoltAPI(clunum);
    
    if(sumVolt > 0)                             /*总电压不为0*/
    {
        chgPowerCur = (u32)gGBmuGenPara_102[eBmuGenPara102_ChgPMaxLim] * 10000 / sumVolt;
    }
    else                                        /*总电压为0V*/
    {
        chgPowerCur = 0;                        /*功率限制电流限值为0*/
    }
    
    if(chgUpCur <= chgPowerCur)
    {
        chgLimit = chgUpCur;                    /*取限值较小者*/
    }
    else
    {
        chgLimit = (u16)chgPowerCur;
    }
    
    return(chgLimit);
}

/*=============================================================================================
** @Name      : static u16 DhgLimCurrByPowerLim(u8 clunum, u16 dhgUpCur)
** @Input     : clunum:簇编号 dhgUpCur 当前放电电流限值上限0.1A
** @Output    : 经过放电功率限制后的放电电流限值0.1A
** @Function  : 计算通过放电功率限制后的放电电流限值
** @The notes :
**===========================================================================================*/
static u16 DhgLimCurrByPowerLim(u8 clunum, u16 dhgUpCur)
{
    u16 dhgLimit = 0;                       /*放电电流限值*/
    u32 dhgPowerCur = 0;                    /*按功率限值计算的电流限值*/
    u32 sumVolt = 0;                        /*组总电压*/
    
    if(clunum >= 2)
    {
        return (0);
    }

    sumVolt = GetGCellSumVoltAPI(clunum);
    
    if(sumVolt > 0)                         /*总电压不为0*/
    {
        dhgPowerCur = (u32)gGBmuGenPara_102[eBmuGenPara102_DhgPMaxLim] * 10000 / sumVolt;
    }
    else                                    /*总电压为0V*/
    {
        dhgPowerCur = 0;                    /*功率限制电流限值为0*/
    }
    
    if(dhgUpCur <= dhgPowerCur)
    {
        dhgLimit = dhgUpCur;                /*取限值较小者*/
    }
    else
    {
        dhgLimit = (u16)dhgPowerCur;
    }
    
    return(dhgLimit);
}



