/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CapInfoCalc.c
** @Founder      : ZQD808
** @Date         : 2026.03.16
** @Function     : SOC容量计算模块
** @Instructions :
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "ParaRead.h"
#include "ParaInterface.h"
#include "EnerInterface.h"
#include "EnerInfoCalc.h"
#include "CurrIntegral.h"
#include "CapInfoCalc.h"
#include "PublicInfo.h"
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>

extern u16 gGBmuGenPara_102[];		/*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	/*主控高级参数值*/
extern s16 *gSysMainInfo_51;        /*系统整体信息*/
extern u16 *gSysCapInfo_52;         /*系统电量信息*/
static t_CAPFORM *sCapForm = NULL;  // 指向动态分配的结构体数组
/*=============================================================================================
** @Name      : void CapInfoCalcDataInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 调用此函数时，参数配置还没初始化，先按2簇的内存分配
**===========================================================================================*/
void CapInfoCalcDataInit(void)
{
    static u8 FirstInitFlag = 0;
    if(FirstInitFlag == 1)
    {
        return;
    }
    // 分配 clusterNum 个 t_CAPFORM 结构体的连续内存
    sCapForm = ExtMalloc(2 * sizeof(t_CAPFORM));
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return;
    }
    else
    {
        printf("malloc sCapForm %d success!!!\n",2 * sizeof(t_CAPFORM));
        for (u8 i = 0; i < 2; i++)
        {
            #if (PRJ_CELL_CAP == 314)
                sCapForm[i].nowCap    = CAP_ZERO_POINT + 157000;
                sCapForm[i].remCap    = CAP_ZERO_POINT + 157000;
                sCapForm[i].baseCap = CAP_ZERO_POINT;
                sCapForm[i].topCap    = CAP_ZERO_POINT + 304000;
                sCapForm[i].standCap  = 314000;
            #elif (PRJ_CELL_CAP == 285)
                sCapForm[i].nowCap    = CAP_ZERO_POINT + 142500;
                sCapForm[i].remCap    = CAP_ZERO_POINT + 142500;
                sCapForm[i].baseCap = CAP_ZERO_POINT;
                sCapForm[i].topCap    = CAP_ZERO_POINT + 275000;
                sCapForm[i].standCap  = 285000;
            #else
                sCapForm[i].nowCap    = CAP_ZERO_POINT + 140000;
                sCapForm[i].remCap    = CAP_ZERO_POINT + 140000;
                sCapForm[i].baseCap = CAP_ZERO_POINT;
                sCapForm[i].topCap    = CAP_ZERO_POINT + 270000;
                sCapForm[i].standCap  = 280000;
            #endif
        }
    }
    FirstInitFlag = 1;
}
/*=============================================================================================
** @Name      : void GroupCapSocInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组当前容量SOC计算初始化
** @The notes :
**===========================================================================================*/
void GroupCapSocInit(void)
{
	u32 data[5] = {0};
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++) //按两簇初始化
    {
        if(clunum >= 2)
        {
            return;
        }
        //已初始化EEPROM有效值
        if(TRUE == ParaReadStoreCapFormInfo(clunum, data, 5))
        {
            sCapForm[clunum].nowCap = CAP_ZERO_POINT + data[1];
            sCapForm[clunum].remCap = CAP_ZERO_POINT + data[2];
            sCapForm[clunum].topCap = CAP_ZERO_POINT + data[3];
            sCapForm[clunum].baseCap = CAP_ZERO_POINT;
            sCapForm[clunum].standCap = data[4];
        }
        //首次上电未初始化
        else
        {
            if(clunum == 0)
            {
                sCapForm[clunum].nowCap = CAP_ZERO_POINT + ((u32)gGBmuGenPara_102[eBmuGenPara102_1NowSoc] * gGBmuGenPara_102[eBmuGenPara102_1ReaAllCap] / 10);
                sCapForm[clunum].topCap = CAP_ZERO_POINT + gGBmuGenPara_102[eBmuGenPara102_1ReaAllCap] * 100;
            }
            else
            {
                sCapForm[clunum].nowCap = CAP_ZERO_POINT + ((u32)gGBmuGenPara_102[eBmuGenPara102_2NowSoc] * gGBmuGenPara_102[eBmuGenPara102_2ReaAllCap] / 10);
                sCapForm[clunum].topCap = CAP_ZERO_POINT + gGBmuGenPara_102[eBmuGenPara102_2ReaAllCap] * 100;
            }
            sCapForm[clunum].remCap = sCapForm[clunum].nowCap;
            sCapForm[clunum].baseCap = CAP_ZERO_POINT;
            //sCapForm.standCap = gGBmuGenPara_102[eBmuGenPara102_RateCap] * 100;
            //额定容量无小数才为标称容量
            if(0 == (gGBmuGenPara_102[eBmuGenPara102_RateCap] % 10))
            {
                sCapForm[clunum].standCap = gGBmuGenPara_102[eBmuGenPara102_RateCap] * 100;
            }
            //额定容量有小数取实际总容量
            else
            {
                if(clunum == 0)
                {
                    sCapForm[clunum].standCap = gGBmuGenPara_102[eBmuGenPara102_1ReaAllCap] * 100;
                }
                else
                {
                    sCapForm[clunum].standCap = gGBmuGenPara_102[eBmuGenPara102_2ReaAllCap] * 100;
                }
                
            }
        }
    }
}
/*=============================================================================================
** @Name      : void GroupCapSocCalcTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组当前容量SOC计算任务
** @The notes : 100ms调用,测算总容量和SOC
**===========================================================================================*/
void GroupCapSocCalcTask(void)
{
    static u8 sState = 0;
    
    switch(sState)
    {
        case 0:
        	CalcGroupNowCapHandle();        /*计算当前容量*/
        	StoreGroupNowCapToEEP();        /*写当前实际容量到EEP*/
            sState = 1;
            break;
            
        case 1:
            CalcGroupRemainCapHandle();     /*计算剩余容量*/
        	StoreGroupRemainCapToEEP();     /*写当前剩余容量到EEP*/
            sState = 0;
            break;
            
        default:
            sState = 0;
            break;
    }
}
/*=============================================================================================
** @Name      : void CorrGNowCapBySocAPI(u8 clunum, u16 soc)
** @Input     : clunum:簇编号 soc:目标SOC(0.1%)
** @Output    : void
** @Function  : 根据电池组SOC修正当前容量
** @The notes :
**===========================================================================================*/
void CorrGNowCapBySocAPI(u8 clunum, u16 soc)
{
    u32 nowCap = 0;
    u32 lowCap = 0;
    u32 allCap = 0;

    if(clunum >= 2)
    {
        return;
    }
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return;
    }
    lowCap = sCapForm[clunum].baseCap;
    allCap = sCapForm[clunum].topCap;
    
    if(allCap <= lowCap)
    {
        return;
    }
    
    nowCap = (allCap - lowCap) * soc / 1000 + CAP_ZERO_POINT;
    
    if(nowCap > allCap)
    {
        nowCap = allCap;
    }
    else if(nowCap < lowCap)
    {
        nowCap = lowCap;
    }
    
    sCapForm[clunum].nowCap = nowCap;

    /*写当前实际容量到EEP*/
    StoreGroupNowCapToEEP();
}
/*=============================================================================================
** @Name      : void CorrGRemainCapBySocAPI(u8 clunum, u16 soc)
** @Input     : clunum:簇编号 soc:目标SOC(0.1%)
** @Output    : void
** @Function  : 根据电池组SOC修正剩余电量
** @The notes : 相对总容量剩余值
**===========================================================================================*/
void CorrGRemainCapBySocAPI(u8 clunum, u16 soc)
{
    u32 remCap = 0;
    u32 lowCap = 0;
    u32 allCap = 0;

    if(clunum >= 2)
    {
        return;
    }
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return;
    }
    lowCap = sCapForm[clunum].baseCap;
    allCap = sCapForm[clunum].topCap;

    if(allCap <= lowCap)
    {
        return;
    }

    remCap = (allCap - lowCap) * soc / 1000 + CAP_ZERO_POINT;

    if(remCap > allCap)
    {
    	remCap = allCap;
    }
    else if(remCap < lowCap)
    {
    	remCap = lowCap;
    }

    sCapForm[clunum].remCap = remCap;

    /*写剩余容量到EEP*/
    StoreGroupRemainCapToEEP();
}

/*=============================================================================================
** @Name      : void CorrGroupTotalCapAPI(u8 clunum, u32 cap)
** @Input     : clunum:簇编号 cap:电池组总容量(单位:1mAH)
** @Output    : void
** @Function  : 修正电池组总容量
** @The notes :
**===========================================================================================*/
void CorrGroupTotalCapAPI(u8 clunum, u32 cap)
{
    u32 soc = 0;
    u32 nowCap = 0;
    u32 remCap = 0;
    u32 lowCap = 0;
    u32 allCap = 0;
    u32 befCap = 0;

    if(clunum >= 2)
    {
        return;
    }
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return;
    }
    nowCap = sCapForm[clunum].nowCap;
    remCap = sCapForm[clunum].remCap;
    lowCap = sCapForm[clunum].baseCap;
    allCap = sCapForm[clunum].topCap;
    befCap = sCapForm[clunum].topCap - CAP_ZERO_POINT;
    
    if(allCap <= lowCap)                /*变量异常*/
    {
        cap = 0;
        nowCap = CAP_ZERO_POINT;
        remCap = CAP_ZERO_POINT;
        allCap = CAP_ZERO_POINT;
    }
    else
    {
    	/*求顶端容量*/
        allCap = cap + CAP_ZERO_POINT;  /*顶端容量*/

    	/*求当前容量*/
        if(nowCap >= allCap)            /*变量异常*/
        {
            soc = 10000;
        }
        else if(nowCap <= lowCap)       /*变量异常*/
        {
            soc = 0;
        }
        else
        {
            soc = (nowCap - lowCap) * 10000 / befCap;
        }

        nowCap = cap * soc / 10000 + CAP_ZERO_POINT;
        if(nowCap > allCap)             /*计算当前容量超过总容量*/
        {
            nowCap = allCap;
        }

    	/*求剩余容量*/
        if(remCap >= allCap)            /*变量异常*/
        {
            soc = 10000;
        }
        else if(remCap <= lowCap)       /*变量异常*/
        {
            soc = 0;
        }
        else
        {
            soc = (remCap - lowCap) * 10000 / befCap;
        }
        
        remCap = cap * soc / 10000 + CAP_ZERO_POINT;
        if(remCap > allCap)             /*计算当前容量超过总容量*/
        {
        	remCap = allCap;
        }
    }

    sCapForm[clunum].nowCap = nowCap;
    sCapForm[clunum].remCap = remCap;
    sCapForm[clunum].topCap = allCap;

    StoreGroupNowCapToEEP();                    /*写当前实际容量到EEP*/
    StoreGroupRemainCapToEEP();                 /*写当前剩余容量到EEP*/
    StoreGroupTotalCapToEEP(clunum);                  /*写电池组总容量到EEP*/

    EnerAftTotalCapCorrDealHook(clunum, befCap, cap);   /*总容量修正后事件*/
}

/*=============================================================================================
** @Name      : void CorrGroupStandCapAPI(u8 clunum, u32 cap)
** @Input     : clunum:簇编号 cap:电池组标称容量(单位:1mAH)
** @Output    : void
** @Function  : 修正电池组标称容量
** @The notes :
**===========================================================================================*/
void CorrGroupStandCapAPI(u8 clunum, u32 cap)
{
    if(clunum >= 2)
    {
        return;
    }
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return;
    }
    sCapForm[clunum].standCap = cap;

    StoreGroupStandCapToEEP(clunum);                  /*写电池组标称容量到EEP*/
}

/*=============================================================================================
** @Name      : u32 GetGroupNowCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前容量(单位:1mAH)
** @Function  : 获取电池组当前电量
** @The notes : 剩余能够使用的电量
**===========================================================================================*/
u32 GetGroupNowCapAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return (0);
    }
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return (0);
    }
    return(sCapForm[clunum].nowCap - CAP_ZERO_POINT);
}
/*=============================================================================================
** @Name      : u32 GetGroupRemainCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组剩余电量(单位:1mAH)
** @Function  : 获取电池组剩余电量
** @The notes : 包括放不出的电量
**===========================================================================================*/
u32 GetGroupRemainCapAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return (0);
    }
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return (0);
    }
    return(sCapForm[clunum].remCap - CAP_ZERO_POINT);
}
/*=============================================================================================
** @Name      : u32 GetGroupTotalCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组总容量(单位:1mAH)
** @Function  : 获取电池组总容量
** @The notes :
**===========================================================================================*/
u32 GetGroupTotalCapAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return (0);
    }
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return (0);
    }
    return(sCapForm[clunum].topCap - CAP_ZERO_POINT);
}
/*=============================================================================================
** @Name      : u32 GetGroupStandCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组标称容量(单位:1mAH)
** @Function  : 获取电池组标称容量
** @The notes :
**===========================================================================================*/
u32 GetGroupStandCapAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return (0);
    }
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return (0);
    }
    return(sCapForm[clunum].standCap);
}
/*=============================================================================================
** @Name      : u16 GetGRealSocTenThousAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组真实SOC值(单位:万分之一)
** @Function  : 获取电池组真实SOC值
** @The notes :
**===========================================================================================*/
u16 GetGRealSocTenThousAPI(u8 clunum)
{
    u32 soc = 0;
    u32 nowCap = 0;
    u32 lowCap = 0;
    u32 allCap = 0;
    
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return (0);
    }
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return (0);
    }
    nowCap = sCapForm[clunum].nowCap;
    lowCap = sCapForm[clunum].baseCap;
    allCap = sCapForm[clunum].topCap;
    
    if(nowCap >= allCap)
    {
        soc = 10000;
    }
    else if((nowCap <= lowCap) || ((allCap <= lowCap)))
    {
        soc = 0;
    }
    else
    {
        soc = (nowCap - lowCap) * 10000 / (allCap - lowCap);
        
        if(soc > 10000)
        {
            soc = 10000;
        }
    }
    
    return((u16)soc);
}
/*=============================================================================================
** @Name      : u16 GetGRealSocMilliAPI(u8 clunum)
** @Input     : void
** @Output    : 电池组真实SOC值(单位:千分之一)
** @Function  : 获取电池组真实SOC值
** @The notes :
**===========================================================================================*/
u16 GetGRealSocMilliAPI(u8 clunum)
{
    u32 soc = 0;
    u32 nowCap = 0;
    u32 lowCap = 0;
    u32 allCap = 0;
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return (0);
    }
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return (0);
    }
    nowCap = sCapForm[clunum].nowCap;
    lowCap = sCapForm[clunum].baseCap;
    allCap = sCapForm[clunum].topCap;
    
    if(nowCap >= allCap)
    {
        soc = 1000;
    }
    else if((nowCap <= lowCap) || ((allCap <= lowCap)))
    {
        soc = 0;
    }
    else
    {
        soc = (nowCap - lowCap) * 1000 / (allCap - lowCap);
        
        if(soc > 1000)
        {
            soc = 1000;
        }
    }
    
    return((u16)soc);
}
/*=============================================================================================
** @Name      : u16 GetGRemainSocMilliAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组剩余SOC值(单位:千分之一)
** @Function  : 获取电池组剩余SOC值
** @The notes :
**===========================================================================================*/
u16 GetGRemainSocMilliAPI(u8 clunum)
{
    u32 soc = 0;
    u32 remCap = 0;
    u32 lowCap = 0;
    u32 allCap = 0;
    if(clunum >= 2)
    {
        ssdk_printf(SSDK_WARNING, "%s: clunum >= 2\r\n", __func__);
        return (0);
    }
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return (0);
    }
    remCap = sCapForm[clunum].remCap;
    lowCap = sCapForm[clunum].baseCap;
    allCap = sCapForm[clunum].topCap;

    if(remCap >= allCap)
    {
        soc = 1000;
    }
    else if((remCap <= lowCap) || ((allCap <= lowCap)))
    {
        soc = 0;
    }
    else
    {
        soc = (remCap - lowCap) * 1000 / (allCap - lowCap);

        if(soc > 1000)
        {
            soc = 1000;
        }
    }

    return((u16)soc);
}
/*=============================================================================================
** @Name      : static void CalcGroupNowCapHandle(void)
** @Input     : void
** @Output    : void
** @Function  : 计算电池组当前容量处理
** @The notes : 实际可放电容量
**===========================================================================================*/
static void CalcGroupNowCapHandle(void)
{
    s32 nowChangCap = 0;            /*当前充放电电量变化值(单位:1mAH,正数为放电量,负数为充电量)*/
    s32 realChangCap = 0;           /*实际充放电电量变化值(单位:1mAH,正数为放电量,负数为充电量)*/
    
    static s32 sHisChangCap[2] = {0};    /*历史充放电电量变化值(单位:1mAH,正数为放电量,负数为充电量)*/
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        nowChangCap = 0;            
        realChangCap = 0;           
        if(clunum >= 2)
        {
            return;
        }
        nowChangCap = GetChgDhgChangCapAPI(clunum);
        
        realChangCap = nowChangCap - sHisChangCap[clunum];                  /*求间隔时间实际变化电量*/
        
        if((realChangCap > (s32)sCapForm[clunum].standCap)
            || (realChangCap < ((s32)0 - (s32)sCapForm[clunum].standCap)))  /*计算异常(变化量超过标称容量)*/
        {
            realChangCap = 0;                                       /*抛弃本次计算值*/
        }
        
        sCapForm[clunum].nowCap -= realChangCap;                            /*计算当前容量(即剩余容量,放电为正数容量减小)*/
        
        if(sCapForm[clunum].nowCap > sCapForm[clunum].topCap)                       /*当前容量高端越界*/
        {
            sCapForm[clunum].nowCap = sCapForm[clunum].topCap;
        }
        
        if(sCapForm[clunum].nowCap < sCapForm[clunum].baseCap)                      /*当前容量低端越界*/
        {
            sCapForm[clunum].nowCap = sCapForm[clunum].baseCap;
        }
        
        sHisChangCap[clunum] = nowChangCap;
    }
}
/*=============================================================================================
** @Name      : static void CalcGroupRemainCapHandle(void)
** @Input     : void
** @Output    : void
** @Function  : 计算电池组剩余容量
** @The notes : 包括不可用的容量
**===========================================================================================*/
static void CalcGroupRemainCapHandle(void)
{
    s32 nowChangCap = 0;            /*当前充放电电量变化值(单位:1mAH,正数为放电量,负数为充电量)*/
    s32 realChangCap = 0;           /*实际充放电电量变化值(单位:1mAH,正数为放电量,负数为充电量)*/

    static s32 sHisChangCap[2] = {0};    /*历史充放电电量变化值(单位:1mAH,正数为放电量,负数为充电量)*/
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        nowChangCap = 0;           
        realChangCap = 0;           

        if(clunum >= 2)
        {
            return;
        }

        nowChangCap = GetChgDhgChangCapAPI(clunum);

        realChangCap = nowChangCap - sHisChangCap[clunum];                  /*求间隔时间实际变化电量*/

        if((realChangCap > (s32)sCapForm[clunum].standCap)
            || (realChangCap < ((s32)0 - (s32)sCapForm[clunum].standCap)))  /*计算异常(变化量超过标称容量)*/
        {
            realChangCap = 0;                                       /*抛弃本次计算值*/
        }

        sCapForm[clunum].remCap -= realChangCap;                            /*计算当前容量(即剩余容量,放电为正数容量减小)*/

        if(sCapForm[clunum].remCap > sCapForm[clunum].topCap)                       /*当前容量高端越界*/
        {
            sCapForm[clunum].remCap = sCapForm[clunum].topCap;
        }

        if(sCapForm[clunum].remCap < sCapForm[clunum].baseCap)                      /*当前容量低端越界*/
        {
            sCapForm[clunum].remCap = sCapForm[clunum].baseCap;
        }

        sHisChangCap[clunum] = nowChangCap;
    }
}
/*=============================================================================================
** @Name      : static void StoreGroupNowCapToEEP(void)
** @Input     : void
** @Output    : void
** @Function  : 存储电池组当前可用容量到EEP
** @The notes : 判断存储周期
**===========================================================================================*/
static void StoreGroupNowCapToEEP(void)
{
    u32 nowCap = 0;
    u16 nowState = 0;
    u16 changeSoc = 0;
    static u32 sHisCap[2] = {0};                                 /*历史电量(单位:1mAH)*/
    static u32 sHisState[2] = {0};                               /*历史充放电状态*/
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return;
    }

    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        nowCap = 0;
        nowState = 0;
        changeSoc = 0;
        if(clunum >= 2)
        {
            return;
        }
        nowState = GetGChgDhgStateAPI(clunum);
        nowCap = sCapForm[clunum].nowCap - sCapForm[clunum].baseCap;
        changeSoc = (u16)(ABS(nowCap, sHisCap[clunum]) * 1000 / GetGroupTotalCapAPI(clunum));
        
        if((changeSoc >= SOC_CHANG_TO_WRITE_EEP)                /*达到写EEP变化量*/
            || ((ABS(nowCap, sHisCap[clunum]) >= 10)                    /*与EEP值不一致*/
            && (eCURR_IDLE == nowState)
            && (eCURR_IDLE != sHisState[clunum])))                      /*由充放电转为待机*/
        {
            EnerChangEepGNowCapHook(clunum, nowCap);                    /*写EEP*/
            sHisCap[clunum] = nowCap;
        }

        sHisState[clunum] = nowState;
    }

}
/*=============================================================================================
** @Name      : static void StoreGroupRemainCapToEEP(void)
** @Input     : void
** @Output    : void
** @Function  : 存储电池组剩余容量到EEP
** @The notes : 判断存储周期
**===========================================================================================*/
static void StoreGroupRemainCapToEEP(void)
{
    u32 nowCap = 0;
    u16 nowState = 0;
    u16 changeSoc = 0;
    static u32 sHisCap[2] = {0};                                 /*历史电量(单位:1mAH)*/
    static u32 sHisState[2] = {0};                               /*历史充放电状态*/
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        nowCap = 0;
        nowState = 0;
        changeSoc = 0;
        if(clunum >= 2)
        {
            return;
        }
        nowState = GetGChgDhgStateAPI(clunum);
        nowCap = sCapForm[clunum].remCap - sCapForm[clunum].baseCap;
        changeSoc = (u16)(ABS(nowCap, sHisCap[clunum]) * 1000 / GetGroupTotalCapAPI(clunum));

        if((changeSoc >= SOC_CHANG_TO_WRITE_EEP)                /*达到写EEP变化量*/
            || ((ABS(nowCap, sHisCap[clunum]) >= 10)                    /*与EEP值不一致*/
            && (eCURR_IDLE == nowState)
            && (eCURR_IDLE != sHisState[clunum])))                      /*由充放电转为待机*/
        {
            EnerChangEepGRemainCapHook(clunum, nowCap);                 /*写EEP*/
            sHisCap[clunum] = nowCap;
        }

        sHisState[clunum] = nowState;
    }

}
/*=============================================================================================
** @Name      : static void StoreGroupTotalCapToEEP(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 存储电池组总容量到EEP
** @The notes : 直接写EEPROM
**===========================================================================================*/
static void StoreGroupTotalCapToEEP(u8 clunum)
{
    u32 allCap = 0;
    static u32 sHisCap[2] = {0};                         /*历史总容量(单位:1mAH)*/
    if(clunum >= 2)
    {
        return;
    }
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return;
    }
    allCap = sCapForm[clunum].topCap - sCapForm[clunum].baseCap;
    
    if(ABS(allCap, sHisCap[clunum]) >= 10)                  /*总容量变化*/
    {
    	EnerChangEepGTotalCapHook(clunum, allCap);          /*写EEP*/
        sHisCap[clunum] = allCap;
    }
}

/*=============================================================================================
** @Name      : static void StoreGroupStandCapToEEP(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 存储电池组标称容量到EEP
** @The notes : 直接写EEPROM
**===========================================================================================*/
static void StoreGroupStandCapToEEP(u8 clunum)
{
    static u32 sHisCap[2] = {0};                         /*历史标称容量(单位:1mAH)*/
    if(clunum >= 2)
    {
        return;
    }
    if (sCapForm == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapForm null!!!\r\n", __func__);
        return;
    }
    if(ABS(sCapForm[clunum].standCap, sHisCap[clunum]) >= 100)      /*标称容量变化*/
    {
    	EnerChangEepGStandCapHook(clunum, sCapForm[clunum].standCap);/*写EEP*/
        sHisCap[clunum] = sCapForm[clunum].standCap;
    }
}

