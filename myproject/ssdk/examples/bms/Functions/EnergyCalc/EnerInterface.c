/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : EnerInterface.c
** @Founder      : ZQD808
** @Date         : 2019.10.30
** @Function     : 电能量计算模块接口函数
** @Instructions :
**===========================================================================================*/

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "GroupInfo.h"
#include "PublicInfo.h"
#include "GroupPara.h"
#include "SocUserAPI.h"
#include "SocInterface.h"
#include "ParaInterface.h"
#include "DiagInterface.h"
#include "CalcInterface.h"
#include "EnerInterface.h"
#include "DiagFilter.h"
#include "ParaWrite.h"
#include "DiagCheck.h"

extern u32 DiagGetCellVOffCellBitAPI(u8 clunum);
extern u16 gGBmuGenPara_102[];  			/*主控通用参数值*/
/*=============================================================================================
** @Name      : u8 EnerGetGroupBattReadyHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 0未就绪 1就绪
** @Function  : 获取电池信息就绪状态
** @The notes :
**===========================================================================================*/
u8 EnerGetGroupBattReadyHook(u8 clunum);
/*=============================================================================================
** @Name      : u8 EnerGetGroupBattReadyHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 0未就绪 1就绪
** @Function  : 获取电池信息就绪状态
** @The notes :
**===========================================================================================*/
u8 EnerGetGroupBattReadyHook(u8 clunum)
{
	static u8 FirstPowerFlag = 0;
	if(BSPGetOSMillSecTimeAPI() >= 5000)
	{
		FirstPowerFlag = 1;
	}
    if(clunum >= 2)
    {
        return 0;
    }
    if((BSPGetOSMillSecTimeAPI() < 5000 && FirstPowerFlag == 0)     //上电前5s不能执行防止出错
        || (GetGWorkStateAPI(clunum) == eWORK_INIT)                       //系统初始化
        || (GetGWorkStateAPI(clunum) == eWORK_ERR)                        //系统故障
        || (GetGWorkStateAPI(clunum) == eWORK_OFF)                        //系统关机
		|| (2 != DiagGetRcvAllSlaveFlagAPI(clunum))                       //未接收到所有从控信息
		|| (2 != DiagGetRcvAllCellFlagAPI(clunum))                        //未接收到所有单体信息
		|| (DiagGetCellSampErrFlagAPI(clunum) > 0)                        //单体采样失效
		|| (DiagGetCellVOffCellBitAPI(clunum) > 0)                        //单体电压断线
		|| (DiagGetCellTOffCellBitAPI(clunum) > 0)                        //单体温度断线
        || (ABS(GetGCellMaxVoltAPI(clunum), GetGCellMinVoltAPI(clunum)) >= 400) //单体压差过大
        || (eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_SAMP_EXP))     //采样芯片异常
        || (eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_SAMP_VOFF))    //采样电压断线
        || (eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_SAMP_TOFF))    //采样温度断线
        || (eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_INT_COFF)))    //主从CAN断线故障
    {
        return(0);
    }

    return(1);
}

/*=============================================================================================
** @Name      : u8 EnerGetGroupVHProStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组充电高压保护状态:0正常 1充电高压保护
** @Function  : 获取电池组充电高压保护状态
** @The notes :
**===========================================================================================*/
u8 EnerGetGroupVHProStateHook(u8 clunum)
{
    u8 state = 0;
    if(clunum >= 2)
    {
        return 0;
    }
    /*严重过压且电压扔超限*/
    if((eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_CEL_VH1))
        && (GetGCellMaxVoltAPI(clunum) >= gGBmuGenPara_102[eBmuGenPara102_VH2Lim]))
    {
        state = 1;
    }
    else
    {
        state = 0;
    }

    return(state);
}

/*=============================================================================================
** @Name      : u8 EnerGetGroupVLProStateHook(void)
** @Input     : clunum:簇编号
** @Output    : 电池组放电低压保护状态:0正常 1充电高压保护
** @Function  : 获取电池组放电低压保护状态
** @The notes :
**===========================================================================================*/
u8 EnerGetGroupVLProStateHook(u8 clunum)
{
    u8 state = 0;
    if(clunum >= 2)
    {
        return 0;
    }
    /*严重欠压且电压扔超限*/
    if((eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_CEL_VL1))
        && (GetGCellMinVoltAPI(clunum) <= gGBmuGenPara_102[eBmuGenPara102_VL2Lim]))
    {
        state = 1;
    }
    else
    {
        state = 0;
    }

    return(state);
}

/*=============================================================================================
** @Name      : u8 EnerGetGroupVHLimStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组充电高压限制状态:0正常 1充电高压限制
** @Function  : 获取电池组充电高压限制状态
** @The notes :
**===========================================================================================*/
u8 EnerGetGroupVHLimStateHook(u8 clunum)
{
    u8 state = 0;
    if(clunum >= 2)
    {
        return 0;
    }
    /*一般过压*/
    if(eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_CEL_VH2))
    {
        state = 1;
    }
    else
    {
        state = 0;
    }

    return(state);
}

/*=============================================================================================
** @Name      : u8 EnerGetGroupVLLimStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组放电欠压限制状态:0正常 1放电欠压限制
** @Function  : 获取电池组放电欠压限制状态
** @The notes :
**===========================================================================================*/
u8 EnerGetGroupVLLimStateHook(u8 clunum)
{
    u8 state = 0;
    if(clunum >= 2)
    {
        return 0;
    }
    /*一般欠压*/
    if(eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_CEL_VL2))
    {
        state = 1;
    }
    else
    {
        state = 0;
    }

    return(state);
}

/*=============================================================================================
** @Name      : u8 EnerGetUserSocStopReqHook(void)
** @Input     : void
** @Output    : TRUE:请求停止SOC FALSE:无请求
** @Function  : 获取用户SOC停止显示请求
** @The notes : 容量自动标定时停止SOC显示,停止在50%
**===========================================================================================*/
u8 EnerGetUserSocStopReqHook(void)
{
	/*非初始化时需要容量标定(长时间未标定)*/
//	if((eWORK_INIT != GetGWorkStateAPI()) && ((GetCapEnerDropExpAPI() >> 4) > 0))
//	{
//		//空闲时SOC保持不变
//		//充电时SOC超过50%不能继续上涨
//		//放电时SOC低于50%不能继续下降
//		if(((0 == GetGSampOutCurrAPI()) && (ePRO_FULL != GetGProtectStateAPI()) && (ePRO_EMPTY != GetGProtectStateAPI()))
//			|| ((GetGSampOutCurrAPI() < 0) && (GetGBattSocAPI() >= 500) && (ePRO_FULL != GetGProtectStateAPI()))
//			|| ((GetGSampOutCurrAPI() > 0) && (GetGBattSocAPI() <= 500) && (ePRO_EMPTY != GetGProtectStateAPI())))
//		{
//			/*需要停止SOC*/
//			return(TRUE);
//		}
//	}

	/*不需要停止SOC*/
	return(FALSE);
}
/*=============================================================================================
** @Name      : u8 EnerGetUserSoeStopReqHook(void)
** @Input     : void
** @Output    : TRUE:请求停止SOE FALSE:无请求
** @Function  : 获取用户SOE停止显示请求
** @The notes : 容量自动标定时停止SOE显示,停止在50%
**===========================================================================================*/
u8 EnerGetUserSoeStopReqHook(void)
{
	/*非初始化时需要容量标定*/
//	if((eWORK_INIT != GetGWorkStateAPI()) && (GetCapEnerDropExpAPI() > 0))
//	{
//		//空闲时SOE保持不变
//		//充电时SOE超过50%不能继续上涨
//		//放电时SOE低于50%不能继续下降
//		if(((0 == GetGSampOutCurrAPI()) && (ePRO_FULL != GetGProtectStateAPI()) && (ePRO_EMPTY != GetGProtectStateAPI()))
//			|| ((GetGSampOutCurrAPI() < 0) && (GetGBattSoeAPI() >= 500) && (ePRO_FULL != GetGProtectStateAPI()))
//			|| ((GetGSampOutCurrAPI() > 0) && (GetGBattSoeAPI() <= 500) && (ePRO_EMPTY != GetGProtectStateAPI())))
//		{
//			/*需要停止SOE*/
//			return(TRUE);
//		}
//	}

	/*不需要停止SOE*/
	return(FALSE);
}
/*=============================================================================================
** @Name      : void EnerChangEepGNowCapHook(u8 clunum, u32 cap)
** @Input     : clunum:簇编号 cap:当前容量 1mAH
** @Output    : void
** @Function  : 修改EEP当前容量值
** @The notes : 可用容量
**===========================================================================================*/
void EnerChangEepGNowCapHook(u8 clunum, u32 cap)
{
    if(clunum >= 2)
    {
        return;
    }
	/*写EEP存储*/
	(void)ParaWriteStoreCapFormInfo(clunum, 1, &cap, 1);
}
/*=============================================================================================
** @Name      : void EnerChangEepGRemainCapHook(u8 clunum, u32 cap)
** @Input     : clunum:簇编号 cap:目标剩余容量 1mAH
** @Output    : void
** @Function  : 修改EEP电池组剩余容量值
** @The notes : 包括不能放出来的容量
**===========================================================================================*/
void EnerChangEepGRemainCapHook(u8 clunum, u32 cap)
{
    if(clunum >= 2)
    {
        return;
    }
	/*写EEP存储*/
	(void)ParaWriteStoreCapFormInfo(clunum, 2, &cap, 1);
}
/*=============================================================================================
** @Name      : void EnerChangEepGTotalCapHook(u8 clunum, u32 cap)
** @Input     : clunum:簇编号 cap:目标总容量 1mAH
** @Output    : void
** @Function  : 修改EEP电池组总容量值
** @The notes : 实测容量
**===========================================================================================*/
void EnerChangEepGTotalCapHook(u8 clunum, u32 cap)
{
	u16 temp = 0;
    if(clunum >= 2)
    {
        return;
    }
	/*写EEP存储*/
	(void)ParaWriteStoreCapFormInfo(clunum, 3, &cap, 1);

	/*转化为0.1AH*/
	temp = cap / 100;
    if(clunum == 0)
    {
        /*参数有变化*/
        if(temp != gGBmuGenPara_102[eBmuGenPara102_1ReaAllCap])
        {
            /*写入配置区成功*/
            if(TRUE == ParaWriteStoreGroupGenParaInfo(eBmuGenPara102_1ReaAllCap, &temp, 1))
            {
                /*显示区索引*/
                gGBmuGenPara_102[eBmuGenPara102_1ReaAllCap] = temp;
            }
        }
    }
    else
    {
        /*参数有变化*/
        if(temp != gGBmuGenPara_102[eBmuGenPara102_2ReaAllCap])
        {
            /*写入配置区成功*/
            if(TRUE == ParaWriteStoreGroupGenParaInfo(eBmuGenPara102_2ReaAllCap, &temp, 1))
            {
                /*显示区索引*/
                gGBmuGenPara_102[eBmuGenPara102_2ReaAllCap] = temp;
            }
        }
    }
}
/*=============================================================================================
** @Name      : void EnerChangEepGStandCapHook(u8 clunum, u32 cap)
** @Input     : clunum:簇编号 cap:目标标称容量 1mAH
** @Output    : void
** @Function  : 修改EEP电池组标称容量值
** @The notes : 实测容量
**===========================================================================================*/
void EnerChangEepGStandCapHook(u8 clunum, u32 cap)
{
    if(clunum >= 2)
    {
        return;
    }
	/*写EEP存储*/
	(void)ParaWriteStoreCapFormInfo(clunum, 4, &cap, 1);
}
/*=============================================================================================
** @Name      : void EnerChangEepGNowSocHook(u8 clunum, u16 soc)
** @Input     : clunum:簇编号 soc:当前显示SOC 0.1%
** @Output    : void
** @Function  : 修改EEP当前显示SOC值
** @The notes : 显示SOC
**===========================================================================================*/
void EnerChangEepGNowSocHook(u8 clunum, u16 soc)
{
    if(clunum >= 2)
    {
        return;
    }
    if(clunum == 0)
    {
        /*参数有变化*/
        if(soc != gGBmuGenPara_102[eBmuGenPara102_1NowSoc])
        {
            /*写入配置区成功*/
            if(TRUE == ParaWriteStoreGroupGenParaInfo(eBmuGenPara102_1NowSoc, &soc, 1))
            {
                /*显示区索引*/
                gGBmuGenPara_102[eBmuGenPara102_1NowSoc] = soc;
            }
        }
    }
    else
    {    
        /*参数有变化*/
        if(soc != gGBmuGenPara_102[eBmuGenPara102_2NowSoc])
        {
            /*写入配置区成功*/
            if(TRUE == ParaWriteStoreGroupGenParaInfo(eBmuGenPara102_2NowSoc, &soc, 1))
            {
                /*显示区索引*/
                gGBmuGenPara_102[eBmuGenPara102_2NowSoc] = soc;
            }
        }

    }

}
/*=============================================================================================
** @Name      : void EnerChangEepGNowEner(u8 clunum, u32 ener)
** @Input     : clunum:簇编号 ener:当前能量 1Wh
** @Output    : void
** @Function  : 修改EEP电池组当前能量值
** @The notes : 可用电能
**===========================================================================================*/
void EnerChangEepGNowEner(u8 clunum, u32 ener)
{
    if(clunum >= 2)
    {
        return;
    }
	/*写EEP存储*/
	(void)ParaWriteStoreEnerFormInfo(clunum, 1, &ener, 1);
}
/*=============================================================================================
** @Name      : void EnerChangEepGRemainEner(u8 clunum, u32 ener)
** @Input     : clunum:簇编号 ener:剩余能量 1Wh
** @Output    : void
** @Function  : 修改EEP电池组当前剩余能量值
** @The notes : 包括不能放出来的电能
**===========================================================================================*/
void EnerChangEepGRemainEner(u8 clunum, u32 ener)
{
    if(clunum >= 2)
    {
        return;
    }
	/*写EEP存储*/
	(void)ParaWriteStoreEnerFormInfo(clunum, 2, &ener, 1);
}
/*=============================================================================================
** @Name      : void EnerChangEepTotalEner(u8 clunum, u32 ener)
** @Input     : clunum:簇编号 ener:总能量 1Wh
** @Output    : void
** @Function  : 修改EEP总能量值
** @The notes : 实测总电能
**===========================================================================================*/
void EnerChangEepTotalEner(u8 clunum, u32 ener)
{
	u16 temp = 0;
    if(clunum >= 2)
    {
        return;
    }
	/*写EEP存储*/
	(void)ParaWriteStoreEnerFormInfo(clunum, 3, &ener, 1);

	/*转化为0.1kWh*/
	temp = ener / 100;
    if(clunum == 0)
    {
        /*参数有变化*/
        if(temp != gGBmuGenPara_102[eBmuGenPara102_1ReaAllEner])
        {
            /*写入配置区成功*/
            if(TRUE == ParaWriteStoreGroupGenParaInfo(eBmuGenPara102_1ReaAllEner, &temp, 1))
            {
                /*显示区索引*/
                gGBmuGenPara_102[eBmuGenPara102_1ReaAllEner] = temp;
            }
        }
    }
    else
    {
        /*参数有变化*/
        if(temp != gGBmuGenPara_102[eBmuGenPara102_2ReaAllEner])
        {
            /*写入配置区成功*/
            if(TRUE == ParaWriteStoreGroupGenParaInfo(eBmuGenPara102_2ReaAllEner, &temp, 1))
            {
                /*显示区索引*/
                gGBmuGenPara_102[eBmuGenPara102_2ReaAllEner] = temp;
            }
        }
    }
}
/*=============================================================================================
** @Name      : void EnerChangEepStandEner(u8 clunum, u32 ener)
** @Input     : clunum:簇编号 ener:标称能量 1Wh
** @Output    : void
** @Function  : 修改EEP标称能量值
** @The notes : 标准电能值
**===========================================================================================*/
void EnerChangEepStandEner(u8 clunum, u32 ener)
{
    if(clunum >= 2)
    {
        return;
    }
	/*写EEP存储*/
	(void)ParaWriteStoreEnerFormInfo(clunum, 4, &ener, 1);
}
/*=============================================================================================
** @Name      : void EnerChangEepGNowSoeHook(u8 clunum, u16 soe)
** @Input     : clunum:簇编号 soe:当前显示SOE 0.1%
** @Output    : void
** @Function  : 修改EEP当前显示SOE值
** @The notes : 显示SOE
**===========================================================================================*/
void EnerChangEepGNowSoeHook(u8 clunum, u16 soe)
{
    if(clunum >= 2)
    {
        return;
    }
    if(clunum == 0)
    {
        /*参数有变化*/
        if(soe != gGBmuGenPara_102[eBmuGenPara102_1NowSoe])
        {
            /*写入配置区成功*/
            if(TRUE == ParaWriteStoreGroupGenParaInfo(eBmuGenPara102_1NowSoe, &soe, 1))
            {
                /*显示区索引*/
                gGBmuGenPara_102[eBmuGenPara102_1NowSoe] = soe;
            }
        }
    }
    else
    {
        /*参数有变化*/
        if(soe != gGBmuGenPara_102[eBmuGenPara102_2NowSoe])
        {
            /*写入配置区成功*/
            if(TRUE == ParaWriteStoreGroupGenParaInfo(eBmuGenPara102_2NowSoe, &soe, 1))
            {
                /*显示区索引*/
                gGBmuGenPara_102[eBmuGenPara102_2NowSoe] = soe;
            }
        }
    }

}

/*=============================================================================================
** @Name      : void EnerChangEepGRatedCapHook(u16 rateCap)
** @Input     : rateCap:电池组额定容量(0.1AH)
** @Output    : void
** @Function  : 修改电池组额定容量
** @The notes : 出厂有效容量
**===========================================================================================*/
void EnerChangEepGRatedCapHook(u16 rateCap)
{
	/*参数有变化*/
	if(rateCap != gGBmuGenPara_102[eBmuGenPara102_RateCap])
	{
		/*写入配置区成功*/
		if(TRUE == ParaWriteStoreGroupGenParaInfo(eBmuGenPara102_RateCap, &rateCap, 1))
		{
			/*显示区索引*/
			gGBmuGenPara_102[eBmuGenPara102_RateCap] = rateCap;
		}
	}
}

/*=============================================================================================
** @Name      : void EnerChangEepGDampNumHook(u16 times)
** @Input     : times:电池额定循环次数
** @Output    : void
** @Function  : 修改电池额定循环次数
** @The notes : 有效衰减次数
**===========================================================================================*/
void EnerChangEepGDampNumHook(u16 times)
{
	/*参数有变化*/
	if(times != gGBmuGenPara_102[eBmuGenPara102_DampNum])
	{
		/*写入配置区成功*/
		if(TRUE == ParaWriteStoreGroupGenParaInfo(eBmuGenPara102_DampNum, &times, 1))
		{
			/*显示区索引*/
			gGBmuGenPara_102[eBmuGenPara102_DampNum] = times;
		}
	}
}
/*=============================================================================================
** @Name      : void EnerChangEepGNowCapSohHook(u8 clunum, u16 soh)
** @Input     : clunum:簇编号 soh:电池SOH(0.1%)
** @Output    : void
** @Function  : 修改电池当前总容量SOH值
** @The notes : 有效衰减次数
**===========================================================================================*/
void EnerChangEepGNowCapSohHook(u8 clunum, u16 soh)
{
    if(clunum >= 2)
    {
        return;
    }
	/*写EEP存储*/
	(void)ParaWriteStoreNowCapSoh(clunum, 1, &soh, 1);
}
/*=============================================================================================
** @Name      : void EnerAftTotalCapCorrDealHook(u8 clunum, u16 befcap, u16 aftcap)
** @Input     : clunum:簇编号 befcap:前置总容量 aftcap:修正后总容量
** @Output    : void
** @Function  : 总容量修正后处理
** @The notes :
**===========================================================================================*/
void EnerAftTotalCapCorrDealHook(u8 clunum, u16 befcap, u16 aftcap)
{
	/*事件记录*/
	;
}

/*=============================================================================================
** @Name      : void EnerAftTotalEnerCorrDealHook(u8 clunum, u16 befener, u16 aftener)
** @Input     : clunum：簇编号 befener:前置总能量 aftener:修正后总能量
** @Output    : void
** @Function  : 总能量修正后处理
** @The notes :
**===========================================================================================*/
void EnerAftTotalEnerCorrDealHook(u8 clunum, u16 befener, u16 aftener)
{
	/*事件记录*/
	;
}