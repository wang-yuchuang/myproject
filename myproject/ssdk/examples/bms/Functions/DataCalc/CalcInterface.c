/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CalcInterface.c
** @Founder      : ZQD808
** @Date         : 2026.03.05
** @Function     : 计算模块用户接口函数
** @Instructions :
**===========================================================================================*/
/*=================================================================================================
** @Global variable definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "GroupPara.h"
#include "GroupInfo.h"
#include "PublicInfo.h"
#include "DiagInterface.h"
#include "DiagCheck.h"
#include "DiagFilter.h"
#include "GroupState.h"
#include "GroupProtect.h"
#include "UserTactic.h"
#include <stdio.h>
#include <debug.h>
#include "IMIU4Protocol.h"

extern u16 gGBmuGenPara_102[];          /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];       /*主控高级参数值*/

//IMIU4相关信息
extern gImiu4_data *gImiu4_Info[e_IMIU4_NUM]; 
/*=================================================================================================
** @Name      : u8 CalcGetGroupCellVExpStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体电压异常状态: 0:正常 1:异常
** @Function  : 获取电池组单体电压异常状态
** @The notes : 从控电压采样失效/断线/CAN断线时从控单节电压异常
**===============================================================================================*/
u8 CalcGetGroupCellVExpStateHook(u8 clunum)
{
	static u32 sErrTime[2] = {0};
    if(clunum >= 2)
    {
        return (0);
    }
	/*未接收到全部单体电压或采样断线*/
	if((2 != DiagGetRcvAllSlaveFlagAPI(clunum))
		|| 
		(2 != DiagGetRcvAllCellFlagAPI(clunum))
		|| (DiagGetCellSampErrFlagAPI(clunum) > 0)
		|| (DiagGetCellVOffCellBitAPI(clunum) > 0)
		|| (eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_SAMP_EXP))
		/*|| (eErr_AtoB == DiagGetErrFilStateAPI(eERR_ID_SAMP_EXP))*/
		/*|| (eErr_B == DiagGetErrLastStateAPI(eERR_ID_INT_LOFF))
		|| (eErr_AtoB == DiagGetErrFilStateAPI(eERR_ID_INT_LOFF))*/)
	{
		/*连续3s异常或系统已故障*/
		if((GetSysSecondTimeAPI() >= (sErrTime[clunum] + 3))
			|| (eWORK_ERR == GetGWorkStateAPI(clunum)))
		{
			return(1);
		}
		/*首次检测到异常*/
		else if(0 == sErrTime[clunum])
		{
			/*记录时间*/
			sErrTime[clunum] = GetSysSecondTimeAPI();
		}
	}
	else
	{
		sErrTime[clunum] = 0;
	}

	return(0);
}
/*=============================================================================================
** @Name      : u8 CalcGetGroupCellTExpStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体温度异常状态 0:正常 1:异常
** @Function  : 获取电池组单体温度异常状态
** @The notes : 从控温度计采样失效/断线/CAN断线时从控单节温度异常
**===========================================================================================*/
u8 CalcGetGroupCellTExpStateHook(u8 clunum)
{
	static u32 sErrTime[2] = {0};
    if(clunum >= 2)
    {
        return (0);
    }
	/*未接收到全部单体温度或采样断线*/
	if((2 != DiagGetRcvAllSlaveFlagAPI(clunum))
		|| 
		(2 != DiagGetRcvAllCellFlagAPI(clunum))
		|| (DiagGetCellSampErrFlagAPI(clunum) > 0)
		|| (DiagGetCellTOffCellBitAPI(clunum) > 0)
		|| (eErr_B == DiagGetErrLastStateAPI(clunum,eERR_ID_SAMP_EXP))
		/*|| (eErr_AtoB == DiagGetErrFilStateAPI(eERR_ID_SAMP_EXP))*/
		/*|| (eErr_B == DiagGetErrLastStateAPI(eERR_ID_INT_LOFF))
		|| (eErr_AtoB == DiagGetErrFilStateAPI(eERR_ID_INT_LOFF))*/)
	{
		/*首次检测到异常*/
		if(0 == sErrTime[clunum])
		{
			/*记录时间*/
			sErrTime[clunum] = GetSysSecondTimeAPI();
		}
		/*连续2s异常或系统已故障*/
		else if((GetSysSecondTimeAPI() >= (sErrTime[clunum] + 2))
			|| (eWORK_ERR == GetGWorkStateAPI(clunum)))
		{
			return(1);
		}
	}
	else
	{
		sErrTime[clunum] = 0;
	}

	return(0);
}
/*=================================================================================================
** @Name      : u8 CalcGetGropuNeedSysLimStateHook(void)
** @Input     : void
** @Output    : 充放电需限制状态:0正常 1需限制充放电
** @Function  : 获取电池组充放电需限制状态
** @The notes : 产生充放电限制故障或按下停机按钮
**===============================================================================================*/
u8 CalcGetGropuNeedSysLimStateHook(u8 clunum)
{
	e_ErrSort sort = 0;
    if(clunum >= 2)
    {
        return (0);
    }
    //获取当前全部故障级别
	sort = DiagGetGroupNowErrLevelAPI(clunum,0);
 	/*紧急模式不让故障导致限流为0*/
	if(GetGWorkModeAPI(clunum) != eMODE_EMERG)
	{
		//产生充放电限制/故障/或SOS异常或用户需求按钮停机
		if((eErr_SysLim == sort) || (eErr_SysPro == sort)
			|| (1 == GetButtonStopReqStateAPI(clunum))
			//|| (1 == GetFstChgStopReqFlagAPI())
        )
		{
			return(1);
		}
	}
	else
	{
		/*消防逻辑高于紧急逻辑:紧急模式下其余故障豁免限流,消防分级告警(57~60)仍强制限流;
		烟感197不参与(易误触发,且仅作L4判定输入,不足以单独压过应急供电)*/
		if((eErr_B == DiagGetErrNowStateAPI(clunum, eERR_ID_FIRE_EWAR))
			|| (eErr_B == DiagGetErrNowStateAPI(clunum, eERR_ID_FIRE1_WAR))
			|| (eErr_B == DiagGetErrNowStateAPI(clunum, eERR_ID_FIRE3_WAR))
			|| (eErr_B == DiagGetErrNowStateAPI(clunum, eERR_ID_FIRE4_WAR)))
		{
			return(1);
		}
	}
    /*紧急模式不让SOS导致限流为0*/
    if(GetGWorkModeAPI(clunum) != eMODE_EMERG)
    {
        //ZQD808 后续再实现
    	 //if(DiagGetGroupSosLevel() > 0)
    	 //{
    	//	 return(1);
    	 //}
    }
    return(0);
}
/*=============================================================================================
** @Name      : u8 CalcGetGroupNeedChgLimStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组需要充电限制状态: 0正常 1需充电限制
** @Function  : 获取电池组需要充电限制状态
** @The notes : 充满或充电限制
**===========================================================================================*/
u8 CalcGetGroupNeedChgLimStateHook(u8 clunum)
{
	e_ErrSort sort = 0;
    if(clunum >= 2)
    {
        return (0);
    }
    //获取当前充电故障级别
	sort = DiagGetGroupNowErrLevelAPI(clunum,1);

	//产生充电限制/故障或电池充满
	if((eErr_ChgLim == sort) || (eErr_ChgPro == sort)
		|| (1 == TacGetGroupBattStateAPI(clunum))
		|| (TacGetGAskChgProFlagAPI(clunum) > 0)
    )
	{
		return(1);
	}

	return(0);
}
/*=============================================================================================
** @Name      : u8 CalcGetGroupNeedDhgLimStateHook(void)
** @Input     : void
** @Output    : 电池组需要放电限制状态: 0正常 1需放电限制
** @Function  : 获取电池组需要放电限制状态
** @The notes : 放空或放电限制
**===========================================================================================*/
u8 CalcGetGroupNeedDhgLimStateHook(u8 clunum)
{
	e_ErrSort sort = 0;
    if(clunum >= 2)
    {
        return (0);
    }
    //获取当前放电故障级别
	sort = DiagGetGroupNowErrLevelAPI(clunum, 2);

	//产生放电限制/故障或电池放空
	if((eErr_DhgLim == sort) || (eErr_DhgPro == sort)
		|| (2 == TacGetGroupBattStateAPI(clunum))
		|| (TacGetGAskDhgProFlagAPI(clunum) > 0)
    )
	{
		return(1);
	}

	return(0);
}

/*=============================================================================================
** @Name      : u16 CalcGetChgLimMaxSumVHook(void)
** @Input     : void
** @Output    : 电池组总压充电截止最大电压值(单位:0.1V)
** @Function  : 获取电池组总压充电截止最大电压
** @The notes : 总压高压2级限值(充电限制)
**===========================================================================================*/
u16 CalcGetChgLimMaxSumVHook(void)
{
    return(gGBmuGenPara_102[eBmuGenPara102_SumVH2Lim]);
}
/*=============================================================================================
** @Name      : u16 CalcGetDhgLimMinSumVHook(void)
** @Input     : void
** @Output    : 电池组总压放电截止最小电压值(单位:0.1V)
** @Function  : 获取电池组总压放电截止最小电压
** @The notes : 总压低压2级限值(放电限制)
**===========================================================================================*/
u16 CalcGetDhgLimMinSumVHook(void)
{
    return(gGBmuGenPara_102[eBmuGenPara102_SumVL2Lim]);
}
/*=============================================================================================
** @Name      : u16 CalcGetChgLimCellVoltHook(void)
** @Input     : void
** @Output    : 电池组充电截止单体电压值(单位:1mV)
** @Function  : 获取电池组充电截止单体电压
** @The notes : 单体高压2级限值(充电限制)
**===========================================================================================*/
u16 CalcGetChgLimCellVoltHook(void)
{
    return(gGBmuGenPara_102[eBmuGenPara102_VH2Lim]);
}
/*=============================================================================================
** @Name      : u16 CalcGetDhgLimCellVoltHook(void)
** @Input     : void
** @Output    : 电池组放电截止单体电压值(单位:1mV)
** @Function  : 获取电池组放电截止单体电压
** @The notes : 单体低压2级限值(放电限制)
**===========================================================================================*/
u16 CalcGetDhgLimCellVoltHook(void)
{
    return(gGBmuGenPara_102[eBmuGenPara102_VL2Lim]);
}

/*=============================================================================================
** @Name      : s16 CalcGetChgCDnFstHTempHook(void)
** @Input     : void
** @Output    : 充电高温降电流限值的起始温度 1℃
** @Function  : 获取充电高温降电流限值的起始温度
** @The notes : 默认45℃开始限充电电流限值
**===========================================================================================*/
s16 CalcGetChgCDnFstHTempHook(void)
{
	s16 temp = 0;

	/*比充电截止提前5℃*/
	temp = (s16)gGBmuGenPara_102[eBmuGenPara102_ChgTH2Lim] - 5;

	/*不得低于40℃*/
	if(temp < 40)
	{
		temp = 40;
	}

	//需要提前高温预警降功率
	if(temp > (s16)gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnFstT])
	{
		//取高温预警降功率温度
		temp = (s16)gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnFstT];
	}

    return(temp);
}
/*=============================================================================================
** @Name      : s16 CalcGetChgCDnLstHTempHook(void)
** @Input     : void
** @Output    : 充电高温降电流限值结束温度 1℃
** @Function  : 获取充电高温降电流限值结束温度
** @The notes : 充电高温2级(充电截止)为降充电电流限值终点
**===========================================================================================*/
s16 CalcGetChgCDnLstHTempHook(void)
{
	s16 temp = 0;

	/*功率下降比率有效*/
	if(gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnRate] > 0)
	{
		/*计算按功率下降比率到0的温度*/
		temp = (s16)gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnFstT] - 1 + (1000 / gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnRate]);

		/*不能高于过温限值*/
		if(temp > (s16)gGBmuGenPara_102[eBmuGenPara102_ChgTH2Lim])
		{
			temp = (s16)gGBmuGenPara_102[eBmuGenPara102_ChgTH2Lim];
		}
	}
	else
	{
		temp = (s16)gGBmuGenPara_102[eBmuGenPara102_ChgTH2Lim];
	}

    return(temp);
}

/*=============================================================================================
** @Name      : s16 CalcGetDhgCDnFstHTempHook(void)
** @Input     : void
** @Output    : 放电高温降电流限值的起始温度 1℃
** @Function  : 获取放电高温降电流限值的起始温度
** @The notes : 默认45℃开始限放电电流限值
**===========================================================================================*/
s16 CalcGetDhgCDnFstHTempHook(void)
{
	s16 temp = 0;

	/*取过温末端保护降功率温度(比放电截止提前10℃)*/
	temp = (s16)gGBmuGenPara_102[eBmuGenPara102_DhgTH2Lim] - 10;

	/*不得低于40℃*/
	if(temp < 40)
	{
		temp = 40;
	}

	//需要提前高温预警降功率
	if(temp > (s16)gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnFstT])
	{
		//取高温预警降功率温度
		temp = (s16)gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnFstT];
	}

    return(temp);
}
/*=============================================================================================
** @Name      : s16 CalcGetDhgCDnLstHTempHook(void)
** @Input     : void
** @Output    : 放电高温降电流限值结束温度 1℃
** @Function  : 获取放电高温降电流限值结束温度
** @The notes : 放电高温2级(放电截止)为降放电电流限值终点
**===========================================================================================*/
s16 CalcGetDhgCDnLstHTempHook(void)
{
	s16 temp = 0;

	/*功率下降比率有效*/
	if(gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnRate] > 0)
	{
		/*计算按功率下降比率到0的温度*/
		temp = (s16)gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnFstT] - 1 + (1000 / gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnRate]);

		/*不能高于过温限值*/
		if(temp > (s16)gGBmuGenPara_102[eBmuGenPara102_DhgTH2Lim])
		{
			temp = (s16)gGBmuGenPara_102[eBmuGenPara102_DhgTH2Lim];
		}
	}
	else
	{
		temp = (s16)gGBmuGenPara_102[eBmuGenPara102_DhgTH2Lim];
	}
    return(temp);
}
/*=============================================================================================
** @Name      : u16 CalcGetDcSumSampVolt(u8 clunum)
** @Input     : clunum:
** @Output    : 电池组直流端总电压采样值(0.1V)
** @Function  : 获取电池组直流端总电压采样值
** @The notes : 根据采样方式
**===========================================================================================*/
u16 CalcGetDcSumSampVolt(u8 clunum)
{
    if(gImiu4_Info[e_IMIU4_1] == NULL)
    {
        printf("gImiu4_1SampInfo is null\r\n");
        return (0);
    }
    if(gImiu4_Info[e_IMIU4_2] == NULL)
    {
        printf("gImiu4_2SampInfo is null\r\n");
        return (0);
    }
    if(clunum >= 2)
    {
        return(0);
    }
        //通过通讯获取,D9上全部使用IMIU4数据，其他方式暂不实现
    #if(2 == BMU_DC_VH_TYPE)
    if(clunum == 0)
    {
        return(gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_BatVolt]);
    }
    else
    {
        return(gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_BatVolt]);
    }
        #else
                return (0);
    #endif

}
/*=============================================================================================
** @Name      :u16 CalcGetDcPreSampVolt(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组直流端预充电压采样值(0.1V)
** @Function  : 获取电池组直流端预充电压采样值
** @The notes : 根据采样方式
**===========================================================================================*/
u16 CalcGetDcPreSampVolt(u8 clunum)
{
    if(gImiu4_Info[e_IMIU4_1] == NULL)
    {
        printf("gImiu4_1SampInfo is null\r\n");
        return (0);
    }
    if(gImiu4_Info[e_IMIU4_2] == NULL)
    {
        printf("gImiu4_2SampInfo is null\r\n");
        return (0);
    }
    if(clunum >= 2)
    {
        return(0);
    }
    //通过通讯获取
    #if(2 == BMU_DC_VH_TYPE)
    if(clunum == 0)
    {
        return(gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_LinkVolt]);
    }
    else
    {
        return(gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_LinkVolt]);
    }
        #else
                return (0);
    #endif
}