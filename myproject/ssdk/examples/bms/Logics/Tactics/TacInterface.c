/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : TacInterface.c
** @Founder      : SBK326
** @Date         : 2019.11.1
** @Function     : 电池策略模块用户接口
** @Instructions : 
**===============================================================================================*/

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "PublicPara.h"
#include "GroupPara.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
#include "DiagFilter.h"
#include "DiagCheck.h"
#include "GroupProtect.h"
//#include "CurrSample.h"
//#include "HigAFESamp.h"
#include "PortCheck.h"
#include "InsuCheck.h"
#include "BalanTactic.h"
#include "DiagInterface.h"
#include "EnerInterface.h"
#include "DiagFilter.h"
#include "CurrIntegral.h"
#include "GroupTactic.h"
//#include "ChgTactics.h"
//#include "EventUser.h"
//#include "GroupMsg.h"
#include "DiagUser.h"
#include "TacUser.h"
#include "TacInterface.h"
#include "UserTactic.h"
#include <debug.h>
#include <cmsis_os2.h>

extern u16 gSysGenParaRO_100[];                             /*系统通用设置*/
extern u16 gGBmuGenPara_102[];  							/*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];							/*主控高级参数值*/
extern u8 gGHardPara_104[];                                 /*主控硬件参数配置信息*/

#define CAC_FIT_START_CACHE_MS 200
#define SUB_SLOT_SIZE 36
#define YIELD_EVERY_N_SUBSLOTS 1

static s8 sFitStartCache[2];
static u16 sFitStartRefreshTick[2];
static u8 sSubSlotProgress[2];

/*=================================================================================================
** @Name      : e_ErrSort TacGetGNowErrLevelHook(u8 clunum, u8 type)
** @Input     : clunum:簇编号 type:故障类别 0:全部故障 1:充电类故障 2:放电类故障
** @Output    : 电池组当前故障级别:0无故障 1仅提示 2充电限制 3放电限制 4充放电限制 5充电保护 6放电保护 7充放电保护 8系统故障保护
** @Function  : 获取电池组当前故障级别
** @The notes :
**===============================================================================================*/
e_ErrSort TacGetGNowErrLevelHook(u8 clunum, u8 type)
{
	u8 level = 0;
    if(clunum >= 2)
    {
        return (0);
    }
	level = DiagGetGroupNowErrLevelAPI(clunum, type);
	/*紧急模式屏蔽SOS相关保护*/
	if(GetGWorkModeAPI(clunum) != eMODE_EMERG)
	{
	    /*SOS二级以上需要下电故障保护,SOS相关后续实现*/
		//if(DiagGetGroupSosLevel() >= 2)
		//{
		    /*系统故障保护状态*/
		//    return(eErr_SysErr);
		//}
	    /*SOS一级需要充放电限制*/
		//else if((1 == DiagGetGroupSosLevel()) && (level < eErr_SysLim))
		//{
		//    /*系统充放电限制状态*/
		//    return(eErr_SysLim);
		//}
	}
    /*返回故障类型级别*/
    return(level);
}

/*=================================================================================================
** @Name      : u8 TacGetGAllSlaveReadyHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组所有从机就绪状态 TRUE:就绪 FALSE:未就绪
** @Function  : 获取电池组所有从机就绪状态
** @The notes : 包括从控通信状态和采样状态
**===============================================================================================*/
u8 TacGetGAllSlaveReadyHook(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    /*通信状态正常*/
	if((2 == DiagGetRcvAllSlaveFlagAPI(clunum))
	    && (2 == DiagGetRcvAllCellFlagAPI(clunum))
		//&& (0 == DiagGetCellSampErrFlagAPI())
		//&& (0 == DiagGetCellVOffCellBitAPI())
		//&& (0 == DiagGetCellTOffCellBitAPI())
		&& (eErr_A == DiagGetErrFilStateAPI(clunum, eERR_ID_SAMP_EXP))
		/*&& (eErr_A == DiagGetErrFilStateAPI(eERR_ID_INT_LOFF))*/
		&& (eErr_A == DiagGetErrFilStateAPI(clunum, eERR_ID_SAMP_VOFF))
		/*&& (eErr_A == DiagGetErrFilStateAPI(eERR_ID_SAMP_VOFF))*/
		&& (eErr_A == DiagGetErrFilStateAPI(clunum, eERR_ID_SAMP_TOFF))
		/*&& (eErr_A == DiagGetErrFilStateAPI(eERR_ID_SAMP_TOFF))*/
		&& (eErr_A == DiagGetErrFilStateAPI(clunum, eERR_ID_CELL_MISS))
		/*&& (eErr_A == DiagGetErrFilStateAPI(eERR_ID_CELL_MISS))*/)
	{
		return(TRUE);
	}

    return(FALSE);
}

/*=================================================================================================
** @Name      : u8 TacGetGAllSubdevReadyHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组所有子系统设备就绪状态 TRUE:就绪 FALSE:未就绪
** @Function  : 获取电池组所有子系统设备就绪状态
** @The notes : 包括IMLU1和IMEU2
**===============================================================================================*/
u8 TacGetGAllSubdevReadyHook(u8 clunum)
{
    /*通信状态正常*/
	if((2 == DiagGetRcvAllPeripFlagAPI()))
	{
		return(TRUE);
	}

    return(FALSE);
}

/*=================================================================================================
** @Name      : u8 TacGetGAllPeriphReadyHook(u8 clunum)
** @Input     : void
** @Output    : 电池组所有外设就绪状态 TRUE:就绪 FALSE:未就绪
** @Function  : 获取电池组所有外围设备就绪状态
** @The notes : 包括电流传感器和接触器、绝缘等
**===============================================================================================*/
u8 TacGetGAllPeriphReadyHook(u8 clunum)
{
	/*电流校零完成*/
	/*高压端粘连和绝缘检测完成,后续再实现*/
	//if((SampGetCurrZeroFinFlagAPI() > 0)
	//	&& (TRUE == GetInsuAdhesFinStateAPI()))
	{
		return(TRUE);
	}

    //return(FALSE);
}
/*=================================================================================================
** @Name      : u8 TacGetGNegCirReadyHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组所有从机就绪状态 TRUE:就绪 FALSE:未就绪
** @Function  : 获取电池组负极电路就绪状态
** @The notes : GND接外侧采预充电压正常
**===============================================================================================*/
u8 TacGetGNegCirReadyHook(u8 clunum)
{
	static u8 sHisState[2] = {0};
    if(clunum >= 2)
    {
        return (0);
    }
	/*有负极接触器*/
    #if(1 == BMU_NEG_SWH_EN)
    if(clunum == 0)
    {
        /*负极接触器断开*/
        if(ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK2_1Neg))
        {
            /*与HVCU通讯*/
            #if(2 == BMU_DC_VH_TYPE)
            /*GND接外侧总电压采样为0则就绪--负极接触器未粘连*/
            if(GetGSampPreVoltAPI(clunum) <= gGBmuHigLevPara_103[eBmuHigLevPara103_MSwhOffV])
            {
                return(TRUE);
            }
            else
            {
                return(FALSE);
            }
            /*自身采高压*/
            #elif(1 == BMU_DC_VH_TYPE)

            /*状态变化预充重新采样一次确认*/
            if(1 == sHisState[clunum])
            {
                /*清除预充采样完成标志(重新采样)*/
                SampSetOutHVSampFinStateAPI(0);
                sHisState[clunum] = 0;
            }

            /*预充重新采样完成,预充电压采样为0(多组并机不考虑)则就绪--负极接触器未粘连*/
            if((1 == SampGetOutHVSampFinStateAPI())
                && ((GetGDcSideSampRealInfoAPI(eReferSampInfo200_PChgHV) <= gGBmuHigLevPara_103[eBmuHigLevPara103_MSwhOffV])
                || (gSysGenParaRO_100[eSGenPara100_AllGNum] > 1)))
            {
                return(TRUE);
            }
            else
            {
                return(FALSE);
            }
            #else
            return(TRUE);
            #endif
        }
        /*负极接触器闭合*/
        else
        {
            /*与HVCU通讯*/
            #if(2 == BMU_DC_VH_TYPE)
            /*总电压采样正确外部电压采样为0(多组并机不考虑)则就绪(外部无虚电压)*/
            if((GetGSampSumVoltAPI(clunum) >= gGBmuHigLevPara_103[eBmuHigLevPara103_HigVOffV])
                && (GetGSampPreVoltAPI(clunum) <= gGBmuHigLevPara_103[eBmuHigLevPara103_MSwhOffV])
                && ((ABS(GetGSampPreVoltAPI(clunum), GetGCellSumVoltAPI(clunum)) >= gGBmuHigLevPara_103[eBmuHigLevPara103_SumDiffV])))
            {
                return(TRUE);
            }
            /*正极主接触器或预充接触器闭合*/
            else if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK1_1Mai))
            {
                return(TRUE);
            }
            else
            {
                return(FALSE);
            }
            #else

            /*状态变化预充重新采样一次确认*/
            if(0 == sHisState[clunum])
            {
                /*清除预充采样完成标志(重新采样)*/
                SampSetOutHVSampFinStateAPI(0);
                sHisState[clunum] = 1;
            }

            /*预充重新采样完成,总电压采样正确,外部电压采样为0(多组并机不考虑)或未使能电压判断接触器则就绪(外部无虚电压)*/
            if((1 == SampGetOutHVSampFinStateAPI())
                && (GetGSampSumVoltAPI(clunum) >= gGBmuHigLevPara_103[eBmuHigLevPara103_HigVOffV])
                && (((GetGDcSideSampRealInfoAPI(eReferSampInfo200_PChgHV) <= gGBmuHigLevPara_103[eBmuHigLevPara103_MSwhOffV])
                && (ABS(GetGDcSideSampRealInfoAPI(eReferSampInfo200_PChgHV), GetGSampSumVoltAPI(clunum)) >= gGBmuHigLevPara_103[eBmuHigLevPara103_SumDiffV]))
                || (0 == BitGet(gGBmuHigLevPara_103[eBmuHigLevPara103_AdhMonitType], 1))
                || (gSysGenParaRO_100[eSGenPara100_AllGNum] > 1)))
            {
                return(TRUE);
            }
            /*正极主接触器或预充接触器闭合*/
            else if((ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwh_Mai))
                || (ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwh_Pre)))
            {
                return(TRUE);
            }
            else
            {
                return(FALSE);
            }
            #endif
        }
    }
    else
    {
                /*负极接触器断开*/
        if(ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK4_2Neg))
        {
            /*与HVCU通讯*/
            #if(2 == BMU_DC_VH_TYPE)
            /*GND接外侧总电压采样为0则就绪--负极接触器未粘连*/
            if(GetGSampPreVoltAPI(clunum) <= gGBmuHigLevPara_103[eBmuHigLevPara103_MSwhOffV])
            {
                return(TRUE);
            }
            else
            {
                return(FALSE);
            }
            /*自身采高压*/
            #elif(1 == BMU_DC_VH_TYPE)

            /*状态变化预充重新采样一次确认*/
            if(1 == sHisState[clunum])
            {
                /*清除预充采样完成标志(重新采样)*/
                SampSetOutHVSampFinStateAPI(0);
                sHisState[clunum] = 0;
            }

            /*预充重新采样完成,预充电压采样为0(多组并机不考虑)则就绪--负极接触器未粘连*/
            if((1 == SampGetOutHVSampFinStateAPI())
                && ((GetGDcSideSampRealInfoAPI(eReferSampInfo200_PChgHV) <= gGBmuHigLevPara_103[eBmuHigLevPara103_MSwhOffV])
                || (gSysGenParaRO_100[eSGenPara100_AllGNum] > 1)))
            {
                return(TRUE);
            }
            else
            {
                return(FALSE);
            }
            #else
            return(TRUE);
            #endif
        }
        /*负极接触器闭合*/
        else
        {
            /*与HVCU通讯*/
            #if(2 == BMU_DC_VH_TYPE)
            /*总电压采样正确外部电压采样为0(多组并机不考虑)则就绪(外部无虚电压)*/
            if((GetGSampSumVoltAPI(clunum) >= gGBmuHigLevPara_103[eBmuHigLevPara103_HigVOffV])
                && (GetGSampPreVoltAPI(clunum) <= gGBmuHigLevPara_103[eBmuHigLevPara103_MSwhOffV])
                && ((ABS(GetGSampPreVoltAPI(clunum), GetGCellSumVoltAPI(clunum)) >= gGBmuHigLevPara_103[eBmuHigLevPara103_SumDiffV])))
            {
                return(TRUE);
            }
            /*正极主接触器或预充接触器闭合*/
            else if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK3_2Mai))
            {
                return(TRUE);
            }
            else
            {
                return(FALSE);
            }
            #else

            /*状态变化预充重新采样一次确认*/
            if(0 == sHisState[clunum])
            {
                /*清除预充采样完成标志(重新采样)*/
                SampSetOutHVSampFinStateAPI(0);
                sHisState[clunum] = 1;
            }

            /*预充重新采样完成,总电压采样正确,外部电压采样为0(多组并机不考虑)或未使能电压判断接触器则就绪(外部无虚电压)*/
            if((1 == SampGetOutHVSampFinStateAPI())
                && (GetGSampSumVoltAPI(clunum) >= gGBmuHigLevPara_103[eBmuHigLevPara103_HigVOffV])
                && (((GetGDcSideSampRealInfoAPI(eReferSampInfo200_PChgHV) <= gGBmuHigLevPara_103[eBmuHigLevPara103_MSwhOffV])
                && (ABS(GetGDcSideSampRealInfoAPI(eReferSampInfo200_PChgHV), GetGSampSumVoltAPI(clunum)) >= gGBmuHigLevPara_103[eBmuHigLevPara103_SumDiffV]))
                || (0 == BitGet(gGBmuHigLevPara_103[eBmuHigLevPara103_AdhMonitType], 1))
                || (gSysGenParaRO_100[eSGenPara100_AllGNum] > 1)))
            {
                return(TRUE);
            }
            /*正极主接触器或预充接触器闭合*/
            else if((ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwh_Mai))
                || (ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwh_Pre)))
            {
                return(TRUE);
            }
            else
            {
                return(FALSE);
            }
            #endif
        }
    }
    #else
    return(TRUE);
    #endif
}
/*=================================================================================================
** @Name      : u8 TacGetGPreChgFinStateHook(void)
** @Input     : void
** @Output    : 电池组当前直流侧预充状态 0:未预充 1:预充成功 2:预充过温 3:预充电流异常
** @Function  : 获取电池组直流侧预充状态
** @The notes :
**===============================================================================================*/
u8 TacGetGPreChgFinStateHook(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    //单体过压有充电电流则预充失败
    if((eErr_B == DiagGetErrFilStateAPI(clunum, eERR_ID_CEL_VH2))
        && (GetGSampOutCurrAPI(clunum) <= (0 - (s16)gGHardPara_104[eGHardPara104_ChgIdlePoint])))
    {
        return(3);
    }

    //单体欠压有放电电流则预充失败
    if((eErr_B == DiagGetErrFilStateAPI(clunum, eERR_ID_CEL_VL1))
        && (GetGSampOutCurrAPI(clunum) >= (s16)gGHardPara_104[eGHardPara104_DhgIdlePoint]))
    {
        return(3);
    }

    //预充过温则预充失败
    if(GetGShuntSampTempAPI(clunum) >= gGBmuGenPara_102[eBmuGenPara102_Perip2Lim])
    {
        return(2);
    }

    /*
    //外端电压采样异常则无法完成预充
    if(eErr_B == DiagGetErrFilStateAPI(eERR_ID_HSAMP_EXP))
    {
        return(3);
    }
    //预充温度采样异常则无法完成预充
    if(eErr_B == DiagGetErrFilStateAPI(eERR_ID_TSAMP_EXP))
    {
        return(2);
    }*/

    //外端电压在预充完成压差内则预充完成
    if(ABS(GetGSampSumVoltAPI(clunum), GetGSampPreVoltAPI(clunum)) <= gGBmuHigLevPara_103[eBmuHigLevPara103_PChgFinV])
    {
    	return(1);
    }
    //外端电压预充未满则预充未完成
    else
    {
    	return(0);
    }
}

/*=================================================================================================
** @Name      : u8 TacCheckCurrSafeStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前电流安全状态 0:不安全 1:安全
** @Function  : 检测电池组当前电流安全状态
** @The notes :
**===============================================================================================*/
u8 TacCheckCurrSafeStateHook(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    /*处于待命电流内为切断*/
	if((GetGSampOutCurrAPI(clunum) >= (0 - (s8)gGHardPara_104[eGHardPara104_ChgIdlePoint]))
		&& (GetGSampOutCurrAPI(clunum) <= gGHardPara_104[eGHardPara104_ChgIdlePoint]))
	{
		return(1);
	}

    return(0);
}
/*=================================================================================================
** @Name      : u8 TacCheckCurrCutStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前电流切断状态 0:未切断 1:切断
** @Function  : 检测电池组当前电流切断状态
** @The notes :
**===============================================================================================*/
u8 TacCheckCurrCutStateHook(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    /*处于待命电流内为切断*/
	//if((GetGSampOutCurrAPI() >= (0 - (s8)gGHardPara_104[eGHardPara104_ChgIdlePoint]))
	//	&& (GetGSampOutCurrAPI() <= gGHardPara_104[eGHardPara104_ChgIdlePoint]))
    /*处于±3A电流内为切断*/
	if((GetGSampOutCurrAPI(clunum) >= -30)
		&& (GetGSampOutCurrAPI(clunum) <= 30))
	{
		return(1);
	}

    return(0);
}
/*=================================================================================================
** @Name      : u8 TacGetUserReqStopStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 请求电池组停机状态 0:未请求 1:请求
** @Function  : 获取用户请求电池组停机状态
** @The notes : 运行状态需要停机
**===============================================================================================*/
u8 TacGetUserReqStopStateHook(u8 clunum)
{
	static u32 sCanTime[2] = {0};

    if(clunum >= 2)
    {
        return (0);
    }
    /*本簇IMIU4外部通信正常(COFF扇出按簇归属后,本簇设备断线即进入超时计时)*/
    if((eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_EXT_COFF))
        && (eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_PER_COFF)))
    {
    	/*记录时间*/
    	sCanTime[clunum] = GetSysSecondTimeAPI();
    }
    /*本簇IMIU4断线超时(2026-09-12对称化补入PER_COFF;2026-09-18扇出按簇化后等同本簇设备断线,原仅EXT_COFF/2#IMIU4触发)*/
    else if((gGBmuHigLevPara_103[eBmuHigLevPara103_CommProOffT] > 0)
    	&& (ABS(GetSysSecondTimeAPI(), sCanTime[clunum]) >= gGBmuHigLevPara_103[eBmuHigLevPara103_CommProOffT]))
 	{
		return(1);
	}

    /*断路器断闸未充放电*/
    if((eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_ISO_OFF))
    	&& (eCURR_IDLE == GetGChgDhgStateAPI(clunum)))
	{
		return(1);
	}

    /*发生故障保护*/
    if(TacGetGHappenProFlagAPI(clunum) > 0)
    {
		return(1);
    }

    return(0);
}
/*=================================================================================================
** @Name      : u8 TacGetGroupFitStartStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组适合启动状态 0:不适合 1:适合
** @Function  : 获取电池组适合启动状态
** @The notes : 空闲状态允许启动
**===============================================================================================*/
u8 TacGetGroupFitStartStateHook(u8 clunum)
{
	u16 i = 0;

    if(clunum >= 2)
    {
        return (0);
    }

    /*请求停机*/
    if(1 == TacGetUserReqStopStateHook(clunum))
	{
		return(0);
	}

	/*充放电保护*/
    if((TacGetGAskSysProFlagAPI(clunum) > 0)
	    || (eErr_SysPro == DiagGetGroupNowErrLevelAPI(clunum, 0))
        /*|| (eErr_ChgPro == DiagGetGroupNowErrLevelAPI(clunum, 1))
	    || (eErr_DhgPro == DiagGetGroupNowErrLevelAPI(clunum, 2))*/)
	{
		return(0);
	}

    /*按钮停机*/
	if((1 == GetButtonStopReqStateAPI(clunum))
		|| (1 == GetGEmergIoButtonStateAPI()))
	{
		return(0);
    }

    if((sFitStartCache[clunum] >= 0)
        && (ABS(BSPGetOSMillSecTimeAPI(), sFitStartRefreshTick[clunum]) < CAC_FIT_START_CACHE_MS))
    {
        return((u8)sFitStartCache[clunum]);
    }

    /*是否产生或将产生保护类故障*/
    for(i = 0; i < eERR_ID_FINISH; i++)
    {
        if(i > 0 && (i % SUB_SLOT_SIZE) == 0)
        {
            osDelay(1);
        }
    	/*属于充放电保护或系统故障类故障*/
        if((eErr_SysErr == DiagGetFilErrSortAPI((e_ERR_ID)i))
            || (eErr_SysPro == DiagGetFilErrSortAPI((e_ERR_ID)i)))
        {
            /*已出现故障或将出现故障(正在启动时要等待确认已出现故障)*/
            if((eErr_B == DiagGetErrLastStateAPI(clunum, (e_ERR_ID)i))
                || ((eErr_AtoB == DiagGetErrFilStateAPI(clunum,(e_ERR_ID)i))
                && (eWORK_START != GetGWorkStateAPI(clunum))))
            {
                sFitStartCache[clunum] = 0;
                sFitStartRefreshTick[clunum] = BSPGetOSMillSecTimeAPI();
         		return(0);
            }
        }
    }

    sFitStartCache[clunum] = 1;
    sFitStartRefreshTick[clunum] = BSPGetOSMillSecTimeAPI();
    return(1);
}

/*=================================================================================================
** @Name      : u8 TacGetUserSmogSignStateHook(void)
** @Input     : void
** @Output    : 电池组烟感报警状态 0:未报警 1:报警
** @Function  : 获取用户请求电池组烟感报警状态
** @The notes :
**===============================================================================================*/
u8 TacGetUserSmogSignStateHook(void)
{
    /*烟感信号*/
    if(1 == GetGSmogIoWarStateAPI())
    {
        //烟感报警
        return(1);
    }

    return(0);
}
/*=================================================================================================
** @Name      : u8 TacGetUserButtonEmergStateHook(void)
** @Input     : void
** @Output    : 电池组按钮急停状态 0:未请求 1:请求
** @Function  : 获取用户请求电池组按钮急停状态
** @The notes :
**===============================================================================================*/
u8 TacGetUserButtonEmergStateHook(void)
{
    /*按钮急停*/
	if(1 == GetGEmergIoButtonStateAPI())
	{
		//按钮急停
		return(1);
	}

    return(0);
}
/*=================================================================================================
** @Name      : u8 TacGetGroupExtCommuErrStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前外部通信异常状态 0:正常 1:异常
** @Function  : 获取电池组当前外部通信异常状态
** @The notes :
**===============================================================================================*/
u8 TacGetGroupExtCommuErrStateHook(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    /*外部通信故障*/
	if(eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_EXT_COFF))
	{
		return(1);
	}

    return(0);
}
/*=================================================================================================
** @Name      : u8 TacGetGroupBattVHStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前高压状态 0:正常 1:高压
** @Function  : 获取电池组高压状态
** @The notes :
**===============================================================================================*/
u8 TacGetGroupBattVHStateHook(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    /*单体高压或总压高压*/
	if((eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_CEL_VH2))
		|| (eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_CEL_VH1))
		|| (eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_SUM_VH2))
		|| (eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_SUM_VH1)))
	{
		return(1);
	}

    return(0);
}
/*=================================================================================================
** @Name      : u8 TacGetGroupBattVLStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前低压状态 0:正常 1:低压
** @Function  : 获取电池组低压状态
** @The notes :
**===============================================================================================*/
u8 TacGetGroupBattVLStateHook(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    /*单体低压或总压低压*/
	if((eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_CEL_VL2))
		|| (eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_CEL_VL1))
		|| (eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_SUM_VL2))
		|| (eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_SUM_VL1)))
	{
		return(1);
	}

    return(0);
}
/*=================================================================================================
** @Name      : u16 TacGetGroupChgCapHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组充电电量 1AH
** @Function  : 获取电池组充电电量
** @The notes :
**===============================================================================================*/
u16 TacGetGroupChgCapHook(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    /*充电电量积分值*/
    return(GetChgIntCapAPI(clunum));
}
/*=================================================================================================
** @Name      : u16 TacGetGroupDhgCapHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组放电电量 1AH
** @Function  : 获取电池组放电电量
** @The notes :
**===============================================================================================*/
u16 TacGetGroupDhgCapHook(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    /*放电电量积分值*/
    return(GetDhgIntCapAPI(clunum));
}
/*=================================================================================================
** @Name      : e_SysStateResult TacGroupInitStateResultHook(u8 clunum)
** @Input     : clunum:簇编号 
** @Output    : 检测结果:0:等待 1:完成 2:异常
** @Function  : 电池组初始化状态检测结果
** @The notes :
**===============================================================================================*/
e_SysStateResult TacGroupInitStateResultHook(u8 clunum)
{
    u16 time = 0;                       //故障滤波检测时间 1s
    e_ERR_ID i = eERR_ID_FINISH;
    u16 processed;
    if(clunum >= 2)
    {
        return (0);
    }
    static u8 sState[2] = {0};
    static u16 sBmuCheckTime[2] = {0};       //BMU断线检测等待时间s
    static u16 sMaxCheckTime[2] = {0};       //故障检测最大时间s
    static u16 sErrheckTimer[2] = {0};       //检测故障时间计数器
    static u16 sErrNum[2] = {0};

    e_SysStateResult result = eSYSRESULT_KEEP;//检测结果

    switch(sState[clunum])
    {
        //求最大故障等待时间
        case 0:
            processed = 0;
            //求各故障的检查时间
            for(i = (e_ERR_ID)0; i < eERR_ID_FINISH; i++)
            {
                processed++;
                if(processed % SUB_SLOT_SIZE == 0)
                {
                    osDelay(1);
                }
                if(eERR_ID_INT_COFF == i)                                   //内部主从CAN断线时间(包括上电等待BMU信息时间)
                {
                	time = 0;
                    sBmuCheckTime[clunum] = (u16)(DiagGetFilAtoBTimeAPI(i) / 10 + DIAG_WAIT_CELL_T);
                }
                else if(eErr_SysErr != DiagGetFilErrSortAPI(i))             //不是故障保护类型故障不检测
                {
                    continue;
                }
                else if((eERR_ID_EXT_COFF == i)                             //外部堆主CAN断线时间
                    && (eMODE_AUTO == GetGWorkModeAPI(clunum)))                   //自启动模式
                {
                	time = 0;                                               //不包括检CAN断线时间
                }
                else                                                        //其他故障检测时间(包括上电DT等待时间)
                {
                	time = (u16)(DiagGetFilAtoBTimeAPI(i) / 10);
                }

                //求最大时间检查然后放在自检最大时间处
                if(sMaxCheckTime[clunum] < time)
                {
                    sMaxCheckTime[clunum] = time;
                }
            }

            result = eSYSRESULT_KEEP;
            sErrheckTimer[clunum] = 0;
            sState[clunum] = 1;
            break;

        //电池故障自检
        case 1:
            sErrheckTimer[clunum]++;
            if(sSubSlotProgress[clunum] == 0)
            {
                sErrNum[clunum] = 0;
            }
            {
                u16 startIdx = (u16)sSubSlotProgress[clunum];
                u16 endIdx = startIdx + SUB_SLOT_SIZE;
                if(endIdx > (u16)eERR_ID_FINISH)
                {
                    endIdx = (u16)eERR_ID_FINISH;
                }
                for(i = (e_ERR_ID)startIdx; i < (e_ERR_ID)endIdx; i++)
                {
                    if(eErr_SysErr == DiagGetFilErrSortAPI(i))                  //判断故障保护类型故障是否产生
                    {
                        if(eErr_B == DiagGetErrLastStateAPI(clunum, (e_ERR_ID)i))       //出现故障
                        {
                            result = eSYSRESULT_ERR;                            //系统故障
                            sErrNum[clunum]++;
                            break;
                        }
                        else if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, (e_ERR_ID)i))//即将出现故障
                        {
                            sErrNum[clunum]++;
                        }
                    }
                }
                sSubSlotProgress[clunum] = (endIdx >= (u16)eERR_ID_FINISH) ? 0 : (u8)endIdx;
                osDelay(1);
            }
            if(eSYSRESULT_ERR == result)                                    //已检测出故障
            {
                result = eSYSRESULT_ERR;                                    //系统故障
                sErrheckTimer[clunum] = 0;
                sState[clunum] = 0;
                sSubSlotProgress[clunum] = 0;
            }
            else if(((0 == sErrNum[clunum])                                          //不将产生故障
                || (sErrheckTimer[clunum] > (sMaxCheckTime[clunum] * 10)))                  //等待到最大时间仍无故障
                && (((TRUE == TacGetGAllSlaveReadyHook(clunum))                   //已接收到全部从控信息
                /*&& (TRUE == TacGetGAllSubdevReadyHook())*/)               //已接收到全部子系统(IMLU1和IMEU2)信息
                || (sErrheckTimer[clunum] > sBmuCheckTime[clunum] * 10)))                   //等到判断出BMU断线故障
            {
                result = eSYSRESULT_FIN;                                    //系统空闲
                sErrheckTimer[clunum] = 0;
                sState[clunum] = 0;
            }
            else                                                            //等待故障检测时间
            {
                result = eSYSRESULT_KEEP;                                   //等待状态
            }
            break;

        default:
            result = eSYSRESULT_KEEP;                                       //等待状态
            sErrheckTimer[clunum] = 0;
            sState[clunum] = 0;
            break;
    }

    return(result);
}

/*=================================================================================================
** @Name      : void TacGroupProStateChangDealHook(u8 clunum, u8 befState, u16 aftState)
** @Input     : clunum:簇编号 befState:前置状态 aftState:后置状态
** @Output    : void
** @Function  : 电池组保护状态变换处理函数
** @The notes :
**===============================================================================================*/
void TacGroupProStateChangDealHook(u8 clunum, u8 befState, u16 aftState)
{
    /*触发发送一帧状态信息报文*/
    //SendCanGroupSystemStateInfoMsg();
}
/*=================================================================================================
** @Name      : void TacGroupWorkStateChangDealHook(u8 clunum, u8 befState, u16 aftState)
** @Input     : clunum:簇编号 befState:前置状态 aftState:后置状态
** @Output    : void
** @Function  : 电池组工作状态变换处理函数
** @The notes :
**===============================================================================================*/
void TacGroupWorkStateChangDealHook(u8 clunum, u8 befState, u16 aftState)
{
    if(clunum >= 2)
    {
        return;
    }
	/*空闲转为启动状态*/
	if((eWORK_IDLE == befState) && (eWORK_START == aftState))
	{
		/*单组系统(非并机有外侧干扰)*/
    	if(gSysGenParaRO_100[eSGenPara100_AllGNum] <= 1)
    	{
			/*请求粘连检测,后续实现*/
			//UserReqSwhAdhesDiagAPI();

			/*请求绝缘检测，后续实现*/
			UserReqInsuCheckAPI(clunum);
    	}
	}

    /*触发发送一帧状态信息报文*/
	//SendCanGroupSystemStateInfoMsg();
}
/*=================================================================================================
** @Name      : void TacGroupChgStateChangDealHook(u8 clunum, u8 befState, u16 aftState)
** @Input     : clunum:簇编号 befState:前置状态 aftState:后置状态
** @Output    : void
** @Function  : 电池组充放电状态变换处理函数
** @The notes :
**===============================================================================================*/
void TacGroupChgStateChangDealHook(u8 clunum, u8 befState, u16 aftState)
{
    /*触发发送一帧状态信息报文*/
    //SendCanGroupSystemStateInfoMsg();
}

/*=================================================================================================
** @Name      : void TacGroupWorkModeChangDealHook(u8 clunum, u8 befMode, u8 aftMode)
** @Input     : clunum:簇编号 befState:前置状态 aftState:后置状态
** @Output    : void
** @Function  : 电池组工作模式变换处理函数
** @The notes :
**===============================================================================================*/
void TacGroupWorkModeChangDealHook(u8 clunum, u8 befMode, u8 aftMode)
{
	/*进入特殊模式需要清除故障1次*/
	if((eMODE_EMERG != befMode) && (eMODE_DEBUG != befMode)         /*前置非特殊模式*/
		&& ((eMODE_EMERG == aftMode) || (eMODE_DEBUG == aftMode)))  /*后置特殊模式*/
	{
		TacClrGCtrlErrFlagAPI(clunum);                                    //清除系统控制故障标志
		//后续实现
        ClrInsuAdhesErrFlagAPI(clunum);                                   //清除接触器粘连和绝缘检测故障标志
		DiagClrPerDevExpFlagAPI(clunum);                                  //清除外围传感器故障标志
		TacClrGroupProFlagAPI(clunum);                                    //清除故障保护标志

		DiagClrErrNorStateAPI(clunum, 0xff);                                //清除所有故障
	}

    /*触发发送一帧状态信息报文*/
	//SendCanGroupSystemStateInfoMsg();
}
/*=================================================================================================
** @Name      : void TacGroupPreChgFinDealHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 电池组预充完成处理函数
** @The notes :
**===============================================================================================*/
void TacGroupPreChgFinDealHook(u8 clunum)
{
    /*事件记录*/
    //EventRcdDisplayUserAPI(eEVN_RCDSORT_USER100, eEvn_User_PrChgFin, GetGSampOutCurrAPI(), ((u16)TacGetGroupRunSwhStateAPI() << 8) | TacGetGStartFailFlagAPI(), GetGSampSumVoltAPI(), GetGSampPreVoltAPI());
}