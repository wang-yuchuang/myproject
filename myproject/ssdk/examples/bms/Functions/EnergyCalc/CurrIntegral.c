/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CurrIntegral.c
** @Founder      : ZQD808
** @Date         : 2026.03.17
** @Function     : 电流积分模块
** @Instructions :
**===========================================================================================*/

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "CurrIntegral.h"
#include <debug.h>
#include <cmsis_os2.h>
#include "stddef.h"
#include <stdio.h>
#include <string.h>
static t_CAPINT  *sCapInt  = NULL;   // 指向动态分配的容量积分数组（2个元素）
static t_ENERINT *sEnerInt = NULL;   // 指向动态分配的电能积分数组（2个元素）
/*=============================================================================================
** @Name      : void CurrIntegralDataInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 调用此函数时，参数配置还没初始化，按2簇的内存分配
**===========================================================================================*/
void CurrIntegralDataInit(void)
{
    static u8 FirstInitFlag = 0;
    if(FirstInitFlag == 1)
    {
        return;
    }
    // 分配 2 个 t_CAPINT 结构体的连续内存
    sCapInt = ExtMalloc(2 * sizeof(t_CAPINT));
    if (sCapInt == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapInt null!!!\r\n", __func__);
        return;
    }
    else
    {
        // 清零整个数组（所有成员初始化为0）
        memset(sCapInt, 0, 2 * sizeof(t_CAPINT));
        printf("sCapInt malloc %d successfully\r\n",2 * sizeof(t_CAPINT));
    }
     // 分配 2 个 t_ENERINT 结构体的连续内存
    sEnerInt = ExtMalloc(2 * sizeof(t_ENERINT));
    if (sEnerInt == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerInt null!!!\r\n", __func__);
        return;
    }
    else
    {
        // 清零整个数组（所有成员初始化为0）
        memset(sEnerInt, 0, 2 * sizeof(t_ENERINT));
        printf("sEnerInt malloc %d successfully\r\n",2 * sizeof(t_ENERINT));
    }

    FirstInitFlag = 1;
}
/*=============================================================================================
** @Name      : void ChgCapIntTask(u8 clunum, u32 curr, u8 cycle)
** @Input     : clunum:簇编号 curr:采样电流值(单位10mA) cycle:调用(积分)周期(1ms[1,10])
** @Output    : void
** @Function  : 充电电量积分任务
** @The notes : 最少10ms周期调用
**===========================================================================================*/
void ChgCapIntTask(u8 clunum, u32 curr, u8 cycle)
{
    if(sCapInt == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapInt null!!!\r\n", __func__);
        return;
    }
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return;
    }
    sCapInt[clunum].chgCap10ma1ms += (curr * cycle);
    /*计算充电电量变化量(负数)*/
    if(sCapInt[clunum].chgCap10ma1ms >= MULT_MAH_TO_10MAMS)                                 /*变化1mAH*/
    {
        sCapInt[clunum].changCap1ma1h -= (s32)(sCapInt[clunum].chgCap10ma1ms / MULT_MAH_TO_10MAMS); /*充电变化量递减*/
        
        sCapInt[clunum].chgCap1ma1h += (sCapInt[clunum].chgCap10ma1ms / MULT_MAH_TO_10MAMS);        /*转化为1mAH*/
        
        sCapInt[clunum].chgCap10ma1ms %= MULT_MAH_TO_10MAMS;
    }
    
    /*小储能直接用mAH为单位*/
    #if(0 == SMALL_ESS_EN)
    /*计算充电电量*/
    if(sCapInt[clunum].chgCap1ma1h >= MULT_AH_TO_MAH)                                       /*到达1AH*/
    {
        sCapInt[clunum].chgCap1a1h += (sCapInt[clunum].chgCap1ma1h / MULT_AH_TO_MAH);               /*转化为1mAH*/
        
        sCapInt[clunum].chgCap1ma1h %= MULT_AH_TO_MAH;
    }
    #endif
}
/*=============================================================================================
** @Name      : void DhgCapIntTask(u8 clunum, u32 curr, u8 cycle)
** @Input     : clunum:簇编号 curr:采样电流值(单位10mA) cycle:调用(积分)周期(1ms[1,10])
** @Output    : void
** @Function  : 放电电量积分任务
** @The notes : 最少10ms周期调用
**===========================================================================================*/
void DhgCapIntTask(u8 clunum, u32 curr, u8 cycle)
{
    if(sCapInt == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapInt null!!!\r\n", __func__);
        return;
    }
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return;
    }
    sCapInt[clunum].dhgCap10ma1ms += (curr * cycle);
    /*计算放电电量变化量(正数)*/
    if(sCapInt[clunum].dhgCap10ma1ms >= MULT_MAH_TO_10MAMS)                                 /*变化1mAH*/
    {
        sCapInt[clunum].changCap1ma1h += (s32)(sCapInt[clunum].dhgCap10ma1ms / MULT_MAH_TO_10MAMS); /*放电变化量递增*/
        
        sCapInt[clunum].dhgCap1ma1h += (sCapInt[clunum].dhgCap10ma1ms / MULT_MAH_TO_10MAMS);        /*转化为1mAH*/
        
        sCapInt[clunum].dhgCap10ma1ms %= MULT_MAH_TO_10MAMS;
    }
    
    /*小储能直接用mAH为单位*/
    #if(0 == SMALL_ESS_EN)
    /*计算放电电量*/
    if(sCapInt[clunum].dhgCap1ma1h >= MULT_AH_TO_MAH)                                       /*到达1AH*/
    {
        sCapInt[clunum].dhgCap1a1h += (sCapInt[clunum].dhgCap1ma1h / MULT_AH_TO_MAH);               /*转化为1AH*/
        
        sCapInt[clunum].dhgCap1ma1h %= MULT_AH_TO_MAH;
    }
    #endif
}
/*=============================================================================================
** @Name      : void ChgEnerIntTask(u8 clunum, u32 curr, u16 volt, u8 cycle)
** @Input     : clunum:簇编号 curr:采样电流值(单位10mA), volt采样电压值(单位:100mV) cycle:调用(积分)周期(1ms[1,10])
** @Output    : void
** @Function  : 充电电能积分任务
** @The notes : 最少10ms周期调用
**===========================================================================================*/
void ChgEnerIntTask(u8 clunum, u32 curr, u16 volt, u8 cycle)
{
    static u32 sChg1mw1ms[2] = {0};
    if(sEnerInt == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerInt null!!!\r\n", __func__);
        return;
    }
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return;
    }
    sChg1mw1ms[clunum] += (curr * (u32)volt * cycle);                               /*10mA*100mV*1ms=1mAVms=1mw1ms*/
    if(sChg1mw1ms[clunum] >= MULT_WMS_TO_MWMS)                                      /*达到1WMS*/
    {
        sEnerInt[clunum].chgEner1w1ms += (sChg1mw1ms[clunum] / MULT_WMS_TO_MWMS);           /*转化为1WMS*/
        
        sChg1mw1ms[clunum] %= MULT_WMS_TO_MWMS;
    }
    
    /*计算充电电能变化量(负数)*/
    if(sEnerInt[clunum].chgEner1w1ms >= MULT_WH_TO_WMS)                             /*变化1WH*/
    {
        sEnerInt[clunum].changEner1w1h -= (s32)(sEnerInt[clunum].chgEner1w1ms / MULT_WH_TO_WMS);
        
        sEnerInt[clunum].chgEner1w1h += (sEnerInt[clunum].chgEner1w1ms / MULT_WH_TO_WMS);   /*转化为1WH*/
        
        sEnerInt[clunum].chgEner1w1ms %= MULT_WH_TO_WMS;
    }
    
    /*小储能直接用WH为单位*/
    #if(0 == SMALL_ESS_EN)
    /*计算充电电能*/
    if(sEnerInt[clunum].chgEner1w1h >= MULT_100WH_TO_WH)                            /*到达0.1KWH*/
    {
        sEnerInt[clunum].chgEner100w1h += (sEnerInt[clunum].chgEner1w1h / MULT_100WH_TO_WH);/*转化为0.1KWH*/
        
        sEnerInt[clunum].chgEner1w1h %= MULT_100WH_TO_WH;
    }
    #endif
}
/*=============================================================================================
** @Name      : void DhgEnerIntTask(u8 clunum, u32 curr, u16 volt, u8 cycle)
** @Input     : clunum:簇编号 curr:采样电流值(单位10mA), volt采样电压值(单位:100mV) cycle:调用(积分)周期(1ms[1,10])
** @Output    : void
** @Function  : 放电电能积分任务
** @The notes : 最少10ms周期调用
**===========================================================================================*/
void DhgEnerIntTask(u8 clunum, u32 curr, u16 volt, u8 cycle)
{
    static u32 sDhg1mw1ms[2] = {0};
    if(sEnerInt == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerInt null!!!\r\n", __func__);
        return;
    }
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return;
    }
    sDhg1mw1ms[clunum] += (curr * (u32)volt * cycle);                                  /*10mA*100mV*1ms=1mAVms=1mw1ms*/
    if(sDhg1mw1ms[clunum] >= MULT_WMS_TO_MWMS)                                         /*达到1WMS*/
    {
        sEnerInt[clunum].dhgEner1w1ms += (sDhg1mw1ms[clunum] / MULT_WMS_TO_MWMS);              /*转化为1WMS*/
        
        sDhg1mw1ms[clunum] %= MULT_WMS_TO_MWMS;
    }
    
    /*计算放电电能变化量(正数)*/
    if(sEnerInt[clunum].dhgEner1w1ms >= MULT_WH_TO_WMS)                                 /*变化1WH*/
    {
        sEnerInt[clunum].changEner1w1h += (s32)(sEnerInt[clunum].dhgEner1w1ms / MULT_WH_TO_WMS);
        
        sEnerInt[clunum].dhgEner1w1h += (sEnerInt[clunum].dhgEner1w1ms / MULT_WH_TO_WMS);       /*转化为1WH*/
        
        sEnerInt[clunum].dhgEner1w1ms %= MULT_WH_TO_WMS;
    }
    
    /*小储能直接用WH为单位*/
    #if(0 == SMALL_ESS_EN)
    /*计算放电电能*/
    if(sEnerInt[clunum].dhgEner1w1h >= MULT_100WH_TO_WH)                                /*到达0.1KWH*/
    {
        sEnerInt[clunum].dhgEner100w1h += (sEnerInt[clunum].dhgEner1w1h / MULT_100WH_TO_WH);    /*转化为0.1KWH*/
        
        sEnerInt[clunum].dhgEner1w1h %= MULT_100WH_TO_WH;
    }
    #endif
}
/*=============================================================================================
** @Name      : u32 GetChgIntCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 充电积分电量值(单位:1AH)
** @Function  : 获取充电积分电量值
** @The notes :
**===========================================================================================*/
u32 GetChgIntCapAPI(u8 clunum)
{
    if(sCapInt == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapInt null!!!\r\n", __func__);
        return (0);
    }
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return (0);
    }
    /*小储能直接用mAH为单位*/
    #if(1 == SMALL_ESS_EN)
    return(sCapInt[clunum].chgCap1ma1h);
    #else
    return(sCapInt[clunum].chgCap1a1h);
    #endif
}
/*=============================================================================================
** @Name      : u32 GetDhgIntCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 放电积分电量值(单位:1AH)
** @Function  : 获取放电积分电量值
** @The notes :
**===========================================================================================*/
u32 GetDhgIntCapAPI(u8 clunum)
{
    if(sCapInt == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapInt null!!!\r\n", __func__);
        return (0);
    }
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return (0);
    }
    /*小储能直接用mAH为单位*/
    #if(1 == SMALL_ESS_EN)
    return(sCapInt[clunum].dhgCap1ma1h);
    #else
    return(sCapInt[clunum].dhgCap1a1h);
    #endif
}

/*=============================================================================================
** @Name      : s32 GetChgDhgChangCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 充放电电量变化值(单位:1mAH,正数为放电量,负数为充电量)
** @Function  : 获取充放电电量变化值
** @The notes :
**===========================================================================================*/
s32 GetChgDhgChangCapAPI(u8 clunum)
{
    if(sCapInt == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapInt null!!!\r\n", __func__);
        return (0);
    }
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return (0);
    }
    return(sCapInt[clunum].changCap1ma1h);
}
/*=============================================================================================
** @Name      : u32 GetChgIntEnerAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 充电积分电能值(单位:0.1KWH)
** @Function  : 获取充电积分电能值
** @The notes :
**===========================================================================================*/
u32 GetChgIntEnerAPI(u8 clunum)
{
    if(sEnerInt == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerInt null!!!\r\n", __func__);
        return (0);
    }
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return (0);
    }
    /*小储能直接用WH为单位*/
    #if(1 == SMALL_ESS_EN)
    return(sEnerInt[clunum].chgEner1w1h);
    #else
    return(sEnerInt[clunum].chgEner100w1h);
    #endif
}
/*=============================================================================================
** @Name      : u32 GetDhgIntEnerAPI(u8 clunum)
** @Input     : clunum
** @Output    : 放电积分电能值(单位:0.1KWH)
** @Function  : 获取放电积分电能值
** @The notes :
**===========================================================================================*/
u32 GetDhgIntEnerAPI(u8 clunum)
{
    if(sEnerInt == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerInt null!!!\r\n", __func__);
        return (0);
    }
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return (0);
    }
    /*小储能直接用WH为单位*/
    #if(1 == SMALL_ESS_EN)
    return(sEnerInt[clunum].dhgEner1w1h);
    #else
    return(sEnerInt[clunum].dhgEner100w1h);
    #endif
}
/*=============================================================================================
** @Name      : s32 GetChgDhgChangEnerAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 充放电电能变化值(单位:1WH,正数为放电能,负数为充电能)
** @Function  : 获取充放电电能变化值
** @The notes :
**===========================================================================================*/
s32 GetChgDhgChangEnerAPI(u8 clunum)
{
    if(sEnerInt == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerInt null!!!\r\n", __func__);
        return (0);
    }
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return (0);
    }
    return(sEnerInt[clunum].changEner1w1h);
}

