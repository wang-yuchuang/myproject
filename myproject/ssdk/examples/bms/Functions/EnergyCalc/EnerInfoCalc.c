/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : EnerInfoCalc.c
** @Founder      : ZQD808
** @Date         : 2026.03.17
** @Function     : 充放电电量电能计算模块
** @Instructions :
**===========================================================================================*/

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "ParaInterface.h"
#include "CurrIntegral.h"
#include "CapInfoCalc.h"
#include "EnerInterface.h"
#include "EnerInfoCalc.h"
#include "ParaRead.h"
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>

static t_ENERFORM *sEnerForm = NULL;
extern u16 gGBmuGenPara_102[];		/*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	/*主控高级参数值*/
extern s16 *gSysMainInfo_51;        /*系统整体信息*/
extern u16 *gSysCapInfo_52;         /*系统电量信息*/
extern u32 *gHisCapEner_56;         /*累计能量统计信息*/
/*=============================================================================================
** @Name      : void EnerInfoCalcDataInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 调用此函数时，参数配置还没初始化，先按2簇的内存分配
**===========================================================================================*/
void EnerInfoCalcDataInit(void)
{
    static u8 FirstInitFlag = 0;
    if(FirstInitFlag == 1)
    {
        return;
    }
     // 分配 2 个 t_CAPINT 结构体的连续内存
    sEnerForm = ExtMalloc(2 * sizeof(t_ENERFORM));
    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return;
    }
    // 清零整个数组（所有成员初始化为0）
    memset(sEnerForm, 0, 2 * sizeof(t_ENERFORM));
    for(u8 i = 0; i < 2; i++)
    {
        sEnerForm[i].nowEner = ENER_ZERO_POINT + 100000;
        sEnerForm[i].remEner = ENER_ZERO_POINT + 100000;
        sEnerForm[i].baseEner = ENER_ZERO_POINT;
        sEnerForm[i].topEner = ENER_ZERO_POINT + 205000;
        sEnerForm[i].standEner = 215000;
    }
    printf("sEnerForm malloc %d successfully\r\n",2 * sizeof(t_ENERFORM));
    FirstInitFlag = 1;
}

/*=============================================================================================
** @Name      : void GroupEnerSoeInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组当前电能SOE计算初始化
** @The notes :
**===========================================================================================*/
void GroupEnerSoeInit(void)
{
	u32 data[5] = {0};
    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return;
    }
    //直接初始化2簇的数据
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++) //按两簇初始化
    {
        if(clunum >= 2)
        {
            return;
        }
        if(TRUE == ParaReadStoreEnerFormInfo(clunum, data, 5))
        {
            sEnerForm[clunum].nowEner = ENER_ZERO_POINT + data[1];
            sEnerForm[clunum].remEner = ENER_ZERO_POINT + data[2];
            sEnerForm[clunum].topEner = ENER_ZERO_POINT + data[3];
            sEnerForm[clunum].baseEner = ENER_ZERO_POINT;
            sEnerForm[clunum].standEner = data[4];
        }
        else
        {
            if(clunum == 0)
            {
                sEnerForm[clunum].nowEner = ENER_ZERO_POINT + ((u32)gGBmuGenPara_102[eBmuGenPara102_1NowSoe] * gGBmuGenPara_102[eBmuGenPara102_1ReaAllEner] / 10);
                sEnerForm[clunum].remEner = sEnerForm[clunum].nowEner;
                //sEnerForm.topEner = ENER_ZERO_POINT + gGBmuGenPara_102[eBmuGenPara102_ReaAllEner] * 100;
                sEnerForm[clunum].baseEner = ENER_ZERO_POINT;
                //sEnerForm.standEner = (u32)gGBmuGenPara_102[eBmuGenPara102_ReaAllEner] * 100;
                //总电能无小数则为标称电能(总电能为97%)
                if(0 == (gGBmuGenPara_102[eBmuGenPara102_1ReaAllEner] % 10))
                {
                    sEnerForm[clunum].topEner = ENER_ZERO_POINT + (gGBmuGenPara_102[eBmuGenPara102_1ReaAllEner] * 9700 / 100);
                    sEnerForm[clunum].standEner = (u32)gGBmuGenPara_102[eBmuGenPara102_1ReaAllEner] * 100;
                }
                //有小数,未知标称电能认为总电能为97%
                else
                {
                    sEnerForm[clunum].topEner = ENER_ZERO_POINT + gGBmuGenPara_102[eBmuGenPara102_1ReaAllEner] * 100;
                    sEnerForm[clunum].standEner = (u32)gGBmuGenPara_102[eBmuGenPara102_1ReaAllEner] * 10000 / 97;
                }
            }
            else
            {
                sEnerForm[clunum].nowEner = ENER_ZERO_POINT + ((u32)gGBmuGenPara_102[eBmuGenPara102_2NowSoe] * gGBmuGenPara_102[eBmuGenPara102_2ReaAllEner] / 10);
                sEnerForm[clunum].remEner = sEnerForm[clunum].nowEner;
                //sEnerForm.topEner = ENER_ZERO_POINT + gGBmuGenPara_102[eBmuGenPara102_ReaAllEner] * 100;
                sEnerForm[clunum].baseEner = ENER_ZERO_POINT;
                //sEnerForm.standEner = (u32)gGBmuGenPara_102[eBmuGenPara102_ReaAllEner] * 100;
                //总电能无小数则为标称电能(总电能为97%)
                if(0 == (gGBmuGenPara_102[eBmuGenPara102_2ReaAllEner] % 10))
                {
                    sEnerForm[clunum].topEner = ENER_ZERO_POINT + (gGBmuGenPara_102[eBmuGenPara102_2ReaAllEner] * 9700 / 100);
                    sEnerForm[clunum].standEner = (u32)gGBmuGenPara_102[eBmuGenPara102_2ReaAllEner] * 100;
                }
                //有小数,未知标称电能认为总电能为97%
                else
                {
                    sEnerForm[clunum].topEner = ENER_ZERO_POINT + gGBmuGenPara_102[eBmuGenPara102_2ReaAllEner] * 100;
                    sEnerForm[clunum].standEner = (u32)gGBmuGenPara_102[eBmuGenPara102_2ReaAllEner] * 10000 / 97;
                }
            }
        }
    }
}
/*=============================================================================================
** @Name      : void GroupEnerSoeCalcTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组当前电能SOE计算任务
** @The notes : 100ms调用,测算当前能量和SOE
**===========================================================================================*/
void GroupEnerSoeCalcTask(void)
{
    static u8 sState = 0;
    u8 clunum = gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum];
    switch(sState)
    {
        case 0:
        	CalcGroupNowEnerHandle();   /*计算当前电能*/
        	StoreGroupNowEnerToEEP(0);   /*写当前可用电能到EEP*/
            if(clunum == 2)
            {
                StoreGroupNowEnerToEEP(1);   /*写当前可用电能到EEP*/
            }
            sState = 1;
            break;
            
        case 1:
        	CalcGroupRemainEnerHandle();/*计算剩余电能*/
        	StoreGroupRemainEnerToEEP(0);/*写剩余电能到EEP*/
            if(clunum == 2)
            {
                StoreGroupRemainEnerToEEP(1);/*写剩余电能到EEP*/
            }
            sState = 0;
            break;
            
        default:
            sState = 0;
            break;
    }
}
/*=============================================================================================
** @Name      : void CorrGNowEnerBySoeAPI(u8 clunum, u16 soe)
** @Input     : clunum:簇编号 soe:目标SOE(0.1%)
** @Output    : void
** @Function  : 根据电池组SOE修正当前电能
** @The notes : 用户需要修正电池组当前电能时调用
**===========================================================================================*/
void CorrGNowEnerBySoeAPI(u8 clunum, u16 soe)
{
	u32 nowEner = 0;
    u32 lowEner = 0;
    u32 allEner = 0;
    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return;
    }
    if(clunum >= 2)
    {
        return;
    }
    lowEner = sEnerForm[clunum].baseEner;
    allEner = sEnerForm[clunum].topEner;

    if(allEner <= lowEner)
    {
        return;
    }

    nowEner = (allEner - lowEner) * soe / 1000 + ENER_ZERO_POINT;

    if(nowEner > allEner)
    {
    	nowEner = allEner;
    }
    else if(nowEner < lowEner)
    {
    	nowEner = lowEner;
    }

    sEnerForm[clunum].nowEner = nowEner;

    /*写当前实际电能到EEP*/
    StoreGroupNowEnerToEEP(clunum);
}

/*=============================================================================================
** @Name      : void CorrGRemainEnerBySoeAPI(u8 clunum, u16 soe)
** @Input     : clunum:簇编号 soe:目标SOE(0.1%)
** @Output    : void
** @Function  : 根据电池组SOE修正剩余电能
** @The notes : 用户需要修正电池组剩余电能时调用
**===========================================================================================*/
void CorrGRemainEnerBySoeAPI(u8 clunum, u16 soe)
{
	u32 remEner = 0;
    u32 lowEner = 0;
    u32 allEner = 0;

    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return;
    }
    if(clunum >= 2)
    {
        return;
    }
    lowEner = sEnerForm[clunum].baseEner;
    allEner = sEnerForm[clunum].topEner;

    if(allEner <= lowEner)
    {
        return;
    }

    remEner = (allEner - lowEner) * soe / 1000 + ENER_ZERO_POINT;

    if(remEner > allEner)
    {
    	remEner = allEner;
    }
    else if(remEner < lowEner)
    {
    	remEner = lowEner;
    }

    sEnerForm[clunum].remEner = remEner;

    /*写剩余电能到EEP*/
    StoreGroupRemainEnerToEEP(clunum);
}
/*=============================================================================================
** @Name      : void CorrGroupTotalEnerAPI(u8 clunum, u32 ener)
** @Input     : clunum:簇编号 ener:电池组总能量(单位:1Wh)
** @Output    : void
** @Function  : 修正电池组总能量
** @The notes :
**===========================================================================================*/
void CorrGroupTotalEnerAPI(u8 clunum, u32 ener)
{
    u32 soe = 0;
    u32 nowEner = 0;
    u32 remEner = 0;
    u32 lowEner = 0;
    u32 allEner = 0;
    u32 befEner = 0;
    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return;
    }
    if(clunum >= 2)
    {
        return;
    }
    nowEner = sEnerForm[clunum].nowEner;
    remEner = sEnerForm[clunum].remEner;
    lowEner = sEnerForm[clunum].baseEner;
    allEner = sEnerForm[clunum].topEner;
    befEner = sEnerForm[clunum].topEner - ENER_ZERO_POINT;

    if(allEner <= lowEner)                      /*变量异常*/
    {
        ener = 0;
        nowEner = ENER_ZERO_POINT;
        remEner = ENER_ZERO_POINT;
        allEner = ENER_ZERO_POINT;
    }
    else
    {
    	/*求顶端容量*/
        allEner = ener + ENER_ZERO_POINT;       /*顶端能量*/

    	/*求当前电能*/
        if(nowEner >= allEner)                  /*变量异常*/
        {
            soe = 10000;
        }
        else if(nowEner <= lowEner)             /*变量异常*/
        {
            soe = 0;
        }
        else
        {
            soe = (nowEner - lowEner) * 10000 / befEner;
        }

        nowEner = ener * soe / 10000 + ENER_ZERO_POINT;
        if(nowEner > allEner)                   /*计算当前能量超过总能量*/
        {
            nowEner = allEner;
        }

    	/*求剩余电能*/
        if(remEner >= allEner)                  /*变量异常*/
        {
            soe = 10000;
        }
        else if(remEner <= lowEner)             /*变量异常*/
        {
            soe = 0;
        }
        else
        {
            soe = (remEner - lowEner) * 10000 / befEner;
        }

        remEner = ener * soe / 10000 + ENER_ZERO_POINT;
        if(remEner > allEner)                   /*计算当前能量超过总能量*/
        {
        	remEner = allEner;
        }
    }

    sEnerForm[clunum].nowEner = nowEner;
    sEnerForm[clunum].remEner = remEner;
    sEnerForm[clunum].topEner = allEner;

    StoreGroupNowEnerToEEP(clunum);                   /*写当前实际能量到EEP*/
    StoreGroupRemainEnerToEEP(clunum);                /*写当前剩余能量到EEP*/
    StoreGroupTotalEnerToEEP(clunum);                 /*写电池组总能量到EEP*/

    EnerAftTotalEnerCorrDealHook(clunum, befEner, ener);/*总能量修正后事件*/
}

/*=============================================================================================
** @Name      : void CorrGroupStandEnerAPI(u8 clunum, u32 ener)
** @Input     : clunum:簇编号 ener:电池组标称能量(单位:1Wh)
** @Output    : void
** @Function  : 修正电池组标称能量
** @The notes :
**===========================================================================================*/
void CorrGroupStandEnerAPI(u8 clunum, u32 ener)
{
    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return;
    }
    if(clunum >= 2)
    {
        return;
    }
    sEnerForm[clunum].standEner = ener;

    StoreGroupStandEnerToEEP(clunum);                 /*写电池组标称能量到EEP*/
}
/*=============================================================================================
** @Name      : u32 GetGroupNowEnerAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前电能,即可用电能(单位:1WH)
** @Function  : 获取电池组当前电能
** @The notes : 可以使用的电能
**===========================================================================================*/
u32 GetGroupNowEnerAPI(u8 clunum)
{
    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return(0);
    }
    if(clunum >= 2)
    {
        return(0);
    }
    return(sEnerForm[clunum].nowEner - ENER_ZERO_POINT);
}
/*=============================================================================================
** @Name      : u32 GetGroupRemainEnerAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组剩余电能 (单位:1WH)
** @Function  : 获取电池组剩余电能
** @The notes : 包括不能使用的电能
**===========================================================================================*/
u32 GetGroupRemainEnerAPI(u8 clunum)
{
    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return(0);
    }
    if(clunum >= 2)
    {
        return(0);
    }
    return(sEnerForm[clunum].remEner - ENER_ZERO_POINT);
}
/*=============================================================================================
** @Name      : u32 GetGroupTotalEnerAPI(u8 clunum)
** @Input     : clunum:簇编号 void
** @Output    : 电池组总电能(单位:1WH)
** @Function  : 获取电池组总电能
** @The notes :
**===========================================================================================*/
u32 GetGroupTotalEnerAPI(u8 clunum)
{
    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return(0);
    }
    if(clunum >= 2)
    {
        return(0);
    }
    return(sEnerForm[clunum].topEner - ENER_ZERO_POINT);
}
/*=============================================================================================
** @Name      : u32 GetGroupStandEnerAPI(void)
** @Input     : clunum:簇编号
** @Output    : 电池组标称电能(单位:1WH)
** @Function  : 获取电池组标称电能
** @The notes :
**===========================================================================================*/
u32 GetGroupStandEnerAPI(u8 clunum)
{
    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return(0);
    }
    if(clunum >= 2)
    {
        return(0);
    }
    return(sEnerForm[clunum].standEner);
}
/*=============================================================================================
** @Name      : u16 GetGRealSoeTenThousAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前真实SOE值(0.01%)
** @Function  : 获取电池组当前真实SOE值
** @The notes : 实际可用值，分辨率万分之一
**===========================================================================================*/
u16 GetGRealSoeTenThousAPI(u8 clunum)
{
    u32 ener = 0;
    u32 nowEner = 0;
    u32 lowEner = 0;
    u32 allEner = 0;
    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return(0);
    }
    if(clunum >= 2)
    {
        return(0);
    }
    nowEner = sEnerForm[clunum].nowEner;
    lowEner = sEnerForm[clunum].baseEner;
    allEner = sEnerForm[clunum].topEner;

    if(nowEner >= allEner)
    {
    	ener = 10000;
    }
    else if((nowEner <= lowEner) || ((allEner <= lowEner)))
    {
    	ener = 0;
    }
    else
    {
    	ener = (nowEner - lowEner) * 10000 / (allEner - lowEner);

        if(ener > 10000)
        {
        	ener = 10000;
        }
    }

    return((u16)ener);
}
/*=============================================================================================
** @Name      : u16 GetGRealSoeMilliAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前真实SOE值(0.1%)
** @Function  : 获取电池组当前真实SOE值
** @The notes : 实际可用值，分辨率千分之一
**===========================================================================================*/
u16 GetGRealSoeMilliAPI(u8 clunum)
{
    u32 ener = 0;
    u32 nowEner = 0;
    u32 lowEner = 0;
    u32 allEner = 0;

    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return(0);
    }
    if(clunum >= 2)
    {
        return(0);
    }

    nowEner = sEnerForm[clunum].nowEner;
    lowEner = sEnerForm[clunum].baseEner;
    allEner = sEnerForm[clunum].topEner;

    if(nowEner >= allEner)
    {
    	ener = 1000;
    }
    else if((nowEner <= lowEner) || ((allEner <= lowEner)))
    {
    	ener = 0;
    }
    else
    {
    	ener = (nowEner - lowEner) * 1000 / (allEner - lowEner);

        if(ener > 1000)
        {
        	ener = 1000;
        }
    }

    return((u16)ener);
}
/*=============================================================================================
** @Name      : u16 GetGRemainSoeMilliAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 剩余SOE值(0.1%)
** @Function  : 获取剩余SOE值
** @The notes : 相对总能量剩余值，分辨率千分之一
**===========================================================================================*/
u16 GetGRemainSoeMilliAPI(u8 clunum)
{
    u32 ener = 0;
    u32 remEner = 0;
    u32 lowEner = 0;
    u32 allEner = 0;

    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return(0);
    }
    if(clunum >= 2)
    {
        return(0);
    }

    remEner = sEnerForm[clunum].remEner;
    lowEner = sEnerForm[clunum].baseEner;
    allEner = sEnerForm[clunum].topEner;

    if(remEner >= allEner)
    {
    	ener = 1000;
    }
    else if((remEner <= lowEner) || ((allEner <= lowEner)))
    {
    	ener = 0;
    }
    else
    {
    	ener = (remEner - lowEner) * 1000 / (allEner - lowEner);

        if(ener > 1000)
        {
        	ener = 1000;
        }
    }

    return((u16)ener);
}
/*=============================================================================================
** @Static function definition
**===========================================================================================*/
/*=============================================================================================
** @Name      : static void CalcGroupNowEnerHandle(void)
** @Input     : void
** @Output    : void
** @Function  : 计算电池组当前电能处理
** @The notes : 实际可放电能量
**===========================================================================================*/
static void CalcGroupNowEnerHandle(void)
{
    s32 nowChangEner = 0;            /*当前充放电电能变化值(单位:1Wh,正数为放电量,负数为充电量)*/
    s32 realChangEner = 0;           /*实际充放电电能变化值(单位:1Wh,正数为放电量,负数为充电量)*/

    static s32 sHisChangEner[2] = {0};    /*历史充放电电能量变化值(单位:1mAH,正数为放电能量,负数为充电能量)*/
    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        nowChangEner = 0;            
        realChangEner = 0;          
        nowChangEner = GetChgDhgChangEnerAPI(clunum);

        realChangEner = nowChangEner - sHisChangEner[clunum];                   /*求间隔时间实际变化电能*/

        if((realChangEner > (s32)sEnerForm[clunum].standEner)
            || (realChangEner < ((s32)0 - (s32)sEnerForm[clunum].standEner)))   /*计算异常(变化量超过标称电能)*/
        {
            realChangEner = 0;                                          /*抛弃本次计算值*/
        }

        sEnerForm[clunum].nowEner -= realChangEner;                             /*计算当前电能(即剩余电能,放电为正数电能减小)*/

        if(sEnerForm[clunum].nowEner > sEnerForm[clunum].topEner)                       /*当前能量高端越界*/
        {
            sEnerForm[clunum].nowEner = sEnerForm[clunum].topEner;
        }

        if(sEnerForm[clunum].nowEner < sEnerForm[clunum].baseEner)                      /*当前能量低端越界*/
        {
            sEnerForm[clunum].nowEner = sEnerForm[clunum].baseEner;
        }

        sHisChangEner[clunum] = nowChangEner;
    }

}
/*=============================================================================================
** @Name      : static void CalcGroupRemainEnerHandle(void)
** @Input     : void
** @Output    : void
** @Function  : 计算电池组剩余电能
** @The notes : 包含不可用的电能
**===========================================================================================*/
static void CalcGroupRemainEnerHandle(void)
{
    s32 nowChangEner = 0;            /*当前充放电电能变化值(单位:1Wh,正数为放电量,负数为充电量)*/
    s32 realChangEner = 0;           /*实际充放电电能变化值(单位:1Wh,正数为放电量,负数为充电量)*/

    static s32 sHisChangEner[2] = {0};    /*历史充放电电能量变化值(单位:1mAH,正数为放电能量,负数为充电能量)*/

    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        nowChangEner = 0;           
        realChangEner = 0;          
        nowChangEner = GetChgDhgChangEnerAPI(clunum);

        realChangEner = nowChangEner - sHisChangEner[clunum];                   /*求间隔时间实际变化电能*/

        if((realChangEner > (s32)sEnerForm[clunum].standEner)
            || (realChangEner < ((s32)0 - (s32)sEnerForm[clunum].standEner)))   /*计算异常(变化量超过标称电能)*/
        {
            realChangEner = 0;                                          /*抛弃本次计算值*/
        }

        sEnerForm[clunum].remEner -= realChangEner;                             /*计算当前电能(即剩余电能,放电为正数电能减小)*/

        if(sEnerForm[clunum].remEner > sEnerForm[clunum].topEner)                       /*当前能量高端越界*/
        {
            sEnerForm[clunum].remEner = sEnerForm[clunum].topEner;
        }

        if(sEnerForm[clunum].remEner < sEnerForm[clunum].baseEner)                      /*当前能量低端越界*/
        {
            sEnerForm[clunum].remEner = sEnerForm[clunum].baseEner;
        }

        sHisChangEner[clunum] = nowChangEner;
    }

}
/*=============================================================================================
** @Name      : static void StoreGroupNowEnerToEEP(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 存储当前可用电能到EEP
** @The notes : 判断存储周期
**===========================================================================================*/
static void StoreGroupNowEnerToEEP(u8 clunum)
{
    u16 nowState = 0;
    u32 nowEner = 0;
    static u16 sHisState[2] = {0};
    static u32 sHisEner[2] = {0};
    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return;
    }
    if(clunum >= 2)
    {
        return;
    }
    nowState = GetGChgDhgStateAPI(clunum);
    
    nowEner = sEnerForm[clunum].nowEner - sEnerForm[clunum].baseEner;
    
    if((ABS(nowEner, sHisEner[clunum]) >= ENER_CHANG_TO_WRITE_EEP)  /*达到写EEP变化量*/
        || ((ABS(nowEner, sHisEner[clunum]) >=  10)                 /*与EEP值不一致*/
        && (eCURR_IDLE == nowState)
        && (eCURR_IDLE != sHisState[clunum])))                      /*由充放电转为待机*/
    {
    	EnerChangEepGNowEner(clunum, nowEner);                      /*写EEP*/
        sHisEner[clunum] = nowEner;
    }
    
    sHisState[clunum] = nowState;
}
/*=============================================================================================
** @Name      : static void StoreGroupRemainEnerToEEP(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 存储当前可用电能到EEP
** @The notes : 判断存储周期
**===========================================================================================*/
static void StoreGroupRemainEnerToEEP(u8 clunum)
{
    u16 nowState = 0;
    u32 nowEner = 0;
    static u16 sHisState[2] = {0};
    static u32 sHisEner[2] = {0};

    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return;
    }
    if(clunum >= 2)
    {
        return;
    }
    nowState = GetGChgDhgStateAPI(clunum);

    nowEner = sEnerForm[clunum].remEner - sEnerForm[clunum].baseEner;

    if((ABS(nowEner, sHisEner[clunum]) >= ENER_CHANG_TO_WRITE_EEP)  /*达到写EEP变化量*/
        || ((ABS(nowEner, sHisEner[clunum]) >=  10)                 /*与EEP值不一致*/
        && (eCURR_IDLE == nowState)
        && (eCURR_IDLE != sHisState[clunum])))                      /*由充放电转为待机*/
    {
    	EnerChangEepGRemainEner(clunum, nowEner);                   /*写EEP*/
        sHisEner[clunum] = nowEner;
    }

    sHisState[clunum] = nowState;
}

/*=============================================================================================
** @Name      : static void StoreGroupTotalEnerToEEP(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 存储实际总电能到EEP
** @The notes : 直接写存储
**===========================================================================================*/
static void StoreGroupTotalEnerToEEP(u8 clunum)
{
    u32 totalEner = 0;
    static u32 sHisTotal[2] = {0};
    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return;
    }
    if(clunum >= 2)
    {
        return;
    }

    totalEner = sEnerForm[clunum].topEner - sEnerForm[clunum].baseEner;

    if(ABS(totalEner, sHisTotal[clunum]) >= 10)                     /*总电能变化*/
    {
    	EnerChangEepTotalEner(clunum, totalEner);                   /*写EEP*/
        sHisTotal[clunum] = totalEner;
    }
}
/*=============================================================================================
** @Name      : static void StoreGroupStandEnerToEEP(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 存储实际标称电能到EEP
** @The notes : 直接写存储
**===========================================================================================*/
static void StoreGroupStandEnerToEEP(u8 clunum)
{
    static u32 sHisTotal[2] = {0};
    if (sEnerForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sEnerForm null!!!\r\n", __func__);
        return;
    }
    if(clunum >= 2)
    {
        return;
    }
    if(ABS(sEnerForm[clunum].standEner, sHisTotal[clunum]) >= 100)          /*标称电能变化*/
    {
    	EnerChangEepStandEner(clunum, sEnerForm[clunum].standEner);         /*写EEP*/
        sHisTotal[clunum] = sEnerForm[clunum].standEner;
    }
}





