/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CellFadeCalc.c
** @Founder      : ZQD808
** @Date         : 2026.03.16
** @Function     : 电池总容量和SOH计算模块
** @Instructions :
**===========================================================================================*/

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "ParaInterface.h"
#include "CapInfoCalc.h"
#include "EnerInterface.h"
#include "CellFadeCalc.h"
#include "PublicInfo.h"
#include "ParaRead.h"
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>
extern u16 gGBmuGenPara_102[];		    /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	    /*主控高级参数值*/
extern u16 *gSysCapInfo_52;             /*系统电量信息*/

static t_FADEFORM *sFadeForm = NULL;   // 指向动态分配的数组（2个元素）
/*=============================================================================================
** @Name      : void CellFadeCalcDataInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 调用此函数时，参数配置还没初始化，按2簇的内存分配
**===========================================================================================*/
void CellFadeCalcDataInit(void)
{
    static u8 FirstInitFlag = 0;
    if(FirstInitFlag == 1)
    {
        return;
    }
    // 分配 2 个 t_FADEFORM 结构体的内存
    sFadeForm = ExtMalloc(2 * sizeof(t_FADEFORM));
    if (sFadeForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sFadeForm null!!!\r\n", __func__);
        return;
    }
    else
    {
        printf("malloc sFadeForm %d success!!!\n",2 * sizeof(t_FADEFORM));
        memset(sFadeForm, 0, 2 * sizeof(t_FADEFORM));
        for (u8 i = 0; i < 2; i++)
        {
            sFadeForm[i].fadeCap     = 1000;   // 衰减总容量 (0.1AH)
            sFadeForm[i].ratedCap   = 1000;   // 额定总容量 (0.1AH)
            sFadeForm[i].fadeCycle   = 1;      // 实际循环次数
            sFadeForm[i].ratedCycle = 1000;   // 额定循环次数
        }
    }
    FirstInitFlag = 1;
}
/*=============================================================================================
** @Name      : void GroupFadeSohInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池衰减程度SOH计算初始化
** @The notes :
**===========================================================================================*/
void GroupFadeSohInit(void)
{
	u16 raCap = 0;
	u16 data[2] = {0};

    if (sFadeForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sFadeForm null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++) //按两簇初始化
    {
        raCap = 0;
        if(clunum >= 2)
        {
            return;
        }
        raCap = gGBmuGenPara_102[eBmuGenPara102_RateCap];

        //额定容量无小数则为标称容量,只取93%
        if(0 == (gGBmuGenPara_102[eBmuGenPara102_RateCap] % 10))
        {
            raCap = (u16)((u32)gGBmuGenPara_102[eBmuGenPara102_RateCap] * 93 / 100);
        }

        /*读取记录的SOH值*/
        if((TRUE == ParaReadStoreNowCapSoh(clunum ,data, 2))
            && (data[1] <= 1000))
        {
            //sFadeForm.fadeCap = gGBmuGenPara_102[eBmuGenPara102_RateCap] * data[1] / 1000;
            sFadeForm[clunum].fadeCap = raCap * data[1] / 1000;
        }

        sFadeForm[clunum].fadeCycle = 0;

        /*获取额定总容量和额定循环次数*/
        //sFadeForm.ratedCap = gGBmuGenPara_102[eBmuGenPara102_RateCap];
        sFadeForm[clunum].ratedCap = raCap;
        sFadeForm[clunum].ratedCycle = gGBmuGenPara_102[eBmuGenPara102_DampNum];
    }

}
/*=============================================================================================
** @Name      : void GroupFadeSohCalcTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池容量衰减程度SOH计算任务
** @The notes : 100ms调用,测算总容量和SOH
**===========================================================================================*/
void GroupFadeSohCalcTask(void)
{
	static u8 state = 0;

	switch(state)
	{
		case 0:
			CalcGroupCellFadeHandle();	/*计算电池SOH*/
			state = 0;
			break;

		default:
			state = 0;
			break;
	}
}
/*=============================================================================================
** @Name      : void CorrGroupRatedCapAPI(u8 clunum, u16 rateCap)
** @Input     : clunum:簇编号 rateCap:电池组额定容量(单位:0.1AH)
** @Output    : void
** @Function  : 修正电池组额定容量
** @The notes :
**===========================================================================================*/
void CorrGroupRatedCapAPI(u8 clunum, u16 rateCap)
{
    if(clunum >= 2)
    {
        return;
    }
    if(sFadeForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sFadeForm null!!!\r\n", __func__);
        return;
    }
	sFadeForm[clunum].ratedCap = (u32)rateCap * 100;

	/*写电池组总容量到EEP*/
	EnerChangEepGRatedCapHook(rateCap);
}
/*=============================================================================================
** @Name      : void CorrGroupRatedCycleAPI(u8 clunum, u16 rateCycle)
** @Input     : clunum:簇编号 rateCycle:电池组额定循环次数
** @Output    : void
** @Function  : 修正电池组额定循环次数
** @The notes :
**===========================================================================================*/
void CorrGroupRatedCycleAPI(u8 clunum, u16 rateCycle)
{
    if(clunum >= 2)
    {
        return;
    }
    if(sFadeForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sFadeForm null!!!\r\n", __func__);
        return;
    }
	sFadeForm[clunum].ratedCycle = rateCycle;

	/*写电池组额定循环次数到EEP*/
	EnerChangEepGDampNumHook(rateCycle);
}
/*=============================================================================================
** @Name      : u16 GetGFadeTotalCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组衰减后的总容量值(0.1AH)
** @Function  : 获取电池组衰减后的总容量值
** @The notes :
**===========================================================================================*/
u16 GetGFadeTotalCapAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return(0);
    }
    if(sFadeForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sFadeForm null!!!\r\n", __func__);
        return(0);
    }
    return(sFadeForm[clunum].fadeCap);
}
/*=============================================================================================
** @Name      : u16 GetGroupRatedCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组额定容量(单位:0.1AH)
** @Function  : 获取电池组额定容量
** @The notes :
**===========================================================================================*/
u16 GetGroupRatedCapAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return(0);
    }
    if(sFadeForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sFadeForm null!!!\r\n", __func__);
        return(0);
    }
    return(sFadeForm[clunum].ratedCap);
}
/*=============================================================================================
** @Name      : u16 GetGroupFadeCycleAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池实际衰减循环次数(1次)
** @Function  : 获取电池组实际衰减循环次数
** @The notes :
**===========================================================================================*/
u16 GetGroupFadeCycleAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return(0);
    }
    if(sFadeForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sFadeForm null!!!\r\n", __func__);
        return(0);
    }
    return(sFadeForm[clunum].fadeCycle);
}
/*=============================================================================================
** @Name      : u16 GetGroupRatedCycleAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池额定循环次数(1次)
** @Function  : 获取电池组额定循环次数
** @The notes :
**===========================================================================================*/
u16 GetGroupRatedCycleAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return(0);
    }
    if(sFadeForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sFadeForm null!!!\r\n", __func__);
        return(0);
    }
    return(sFadeForm[clunum].ratedCycle);
}
/*=============================================================================================
** @Name      : u16 GetGRealSohMilliAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前健康度SOH值(0.1%)
** @Function  : 获取电池组当前健康度SOH值
** @The notes : 分辨率千分之一
**===========================================================================================*/
u16 GetGRealSohMilliAPI(u8 clunum)
{
	u16 soh = 0;
    if(clunum >= 2)
    {
        return(0);
    }
    if(sFadeForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sFadeForm null!!!\r\n", __func__);
        return(0);
    }
	if(sFadeForm[clunum].ratedCap > 0)
	{
		//soh = (u32)sFadeForm.fadeCap * 1000 / sFadeForm.ratedCap;
		soh = ((u32)sFadeForm[clunum].fadeCap * 10000 / sFadeForm[clunum].ratedCap + 5) / 10;

		if(soh > 1000)
		{
			soh = 1000;
		}

		return(soh);
	}
	else
	{
		return(0);
	}
}
/*=============================================================================================
** @Name      : static void CalcGroupCellFadeHandle(void)
** @Input     : void
** @Output    : void
** @Function  : 计算电池组电池衰减状态
** @The notes :
**===========================================================================================*/
static void CalcGroupCellFadeHandle(void)
{
	u16 capSoh = 0;
	u16 timSoh = 0;
    if (sFadeForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sFadeForm null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++) //按两簇初始化
    {
        capSoh = 0;
	    timSoh = 0;
        if(clunum >= 2)
        {
            return;
        }
        /*充放电次数变化则计算一次SOH*/
        if((0 == sFadeForm[clunum].fadeCycle) || (GetGChgCycleTimesAPI(clunum) != sFadeForm[clunum].fadeCycle))
        {
            sFadeForm[clunum].fadeCycle = GetGChgCycleTimesAPI(clunum);

            /*通过实际总容量计算的SOH*/
            if(GetGroupRatedCapAPI(clunum) > 0)
            {
                /*使用出厂额定容量(0.1AH)*/
                capSoh = (u16)(GetGroupTotalCapAPI(clunum) * 10 / GetGroupRatedCapAPI(clunum));

                if(capSoh > 1000)
                {
                    capSoh = 1000;
                }
            }
            else
            {
                capSoh = 0;
            }

            /*额定充放电循环次数有效*/
            if(sFadeForm[clunum].ratedCycle > 2)
            {
                /*通过充放电次数计算的SOH*/
                if(sFadeForm[clunum].ratedCycle >= sFadeForm[clunum].fadeCycle)
                {
                    /*额定充放电次数*/
                    timSoh = 1000 - (sFadeForm[clunum].fadeCycle * 200 / sFadeForm[clunum].ratedCycle);
                }
                else
                {
                    timSoh = 800;
                }
            }
            /*额定充放电次数清零*/
            else
            {
                /*仅使用容量计算SOH*/
                timSoh = capSoh;
            }

            /*判断两个方式计算的SOH误差*/
            if(ABS(capSoh, timSoh) <= 50)
            {
                /*误差小则认为当前总容量可信*/
                //sFadeForm.fadeCap = GetGroupTotalCapAPI() / 100;
                /*误差小以充放电次数为准(防止跳变)*/
                capSoh = timSoh;
            }
            else
            {
                /*误差大则求平均值计算衰减容量*/
                capSoh = (capSoh + timSoh) / 2;
            }

            /*额定充放电循环次数有效时防止显示值跳变*/
            if(sFadeForm[clunum].ratedCycle > 1)
            {
                /*SOH不能增大*/
                if(capSoh > GetGBattSohAPI(clunum))
                {
                    capSoh = GetGBattSohAPI(clunum);
                }
                /*SOH变化超过0.5%*/
                else if((capSoh + 5) < GetGBattSohAPI(clunum))
                {
                    /*当前SOH在强制显示100%范围*/
                    if(GetGBattSohAPI(clunum) >= 994)
                    {
                        /*防止被强制显示100%*/
                        capSoh = 993;
                    }
                    else
                    {
                        /*SOH每次最大下降0.5%*/
                        capSoh = GetGBattSohAPI(clunum) - 5;
                    }
                }
            }

            /*防止上位机只显示1%刻度时新电池99.9%显示99%*/
            if(capSoh >= 994)
            {
                /*99.4%以上显示100%*/
                capSoh = 1000;
            }

            /*反算当前SOH对应衰减容量(四舍五入防止精度丢失显示SOH越来越小)*/
            sFadeForm[clunum].fadeCap = (GetGroupRatedCapAPI(clunum) * capSoh + 500) / 1000;

            /*SOH变化则记录*/
            if(capSoh != GetGBattSohAPI(clunum))
            {
                EnerChangEepGNowCapSohHook(clunum, capSoh);
            }
        }
    }
}
