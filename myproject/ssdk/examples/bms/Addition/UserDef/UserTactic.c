/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : UserTactic.c
** @Founder      : SBK326
** @Date         : 2019.11.1
** @Function     : 用户自定义策略模块
** @Instructions : 
**===============================================================================================*/

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "PublicInfo.h"
#include "GroupPara.h"
#include "GroupInfo.h"
#include "PortCheck.h"
#include "DiagInterface.h"
#include "TacUser.h"
#include "UserTactic.h"
#include "DiagFilter.h"
//#include "osif.h"
#include "CalcInterface.h"
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>

static u8 sButtonStopReq[2] = {0};                               /*用户按钮请求停机*/
extern u16 gGBmuGenPara_102[];		                        /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];                           /*主控高级参数值*/

extern u16 *gSysCtrlCmd_65;                                 /*组1系统控制命令*/
extern u16 *gSysCtrlCmd_66;                                 /*组2系统控制命令*/

/*=================================================================================================
** @Name      : void GroupSysIoUpDnCtrlTask(void)
** @Input     : void
** @Output    : void
** @Function  : 主控系统按钮启停控制任务
** @The notes : 1s周期
**===============================================================================================*/
void GroupSysIoUpDnCtrlTask(void)
{
    static u32 sHisTime[2] = {0};
    if (gSysCtrlCmd_65 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysCtrlCmd_65 null!!!\r\n", __func__);
        return;
    }
    if (gSysCtrlCmd_66 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysCtrlCmd_66 null!!!\r\n", __func__);
        return;
    }
	#if(0 == BMU_DC_PWR_TYPE)
	static u8 FirstPowerFlag = 0;
	#endif
    /*电池堆未使能时有效*/
    #if(1 != PILE_MANAG_EN)
    //自供电按钮开关机控制
    #if(0 == BMU_DC_PWR_TYPE)
    u8 state = 0;
    static u8 sTimer = 0;
    static u8 sOffFlag = 0;
    static u8 sHisState = 0;
    #endif
	#if(0 == BMU_DC_PWR_TYPE)
	if(BSPGetOSMillSecTimeAPI() > 5000)
	{
		FirstPowerFlag = 1;
	}
	#endif
    //判断急停干接点回检状态
    if(1 == GetGEmergIoButtonStateAPI())
    {
        //自供电按钮开关机控制
        #if(0 == BMU_DC_PWR_TYPE)
        sTimer = 0;
        sOffFlag = 0;
        sHisState = 0;
        #endif

        for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++) //按两簇初始化
        {
            if(clunum >= 2)
            {
                return;
            }
            //未请求或接触器未断开
            if((0 == sButtonStopReq[clunum]) || (FALSE == TacGetGroupRunSwhAllOffStateAPI(clunum)))
            {
                //设置紧急停机命令
                sButtonStopReq[clunum] = 1;
                if(clunum == 0)
                {
                    gSysCtrlCmd_65[eSysCtrlCmd_WorkSta] = eCTRL_EMERSTOP;
                }
                else
                {
                    gSysCtrlCmd_66[eSysCtrlCmd_WorkSta] = eCTRL_EMERSTOP;
                }
                RcvGroupWorkStateCmdAPI(clunum, eCMDPORT_INTE);
            }
        }


    }
    //无急停信号
    else
    {
        //清除紧急停机请求
        sButtonStopReq[0] = 0;
        sButtonStopReq[1] = 0;
    }

    //自供电按钮开关机控制(软件自锁电源接触器)
    #if(0 == BMU_DC_PWR_TYPE)
    //获取按钮干接点回检状态
    state = GetGPwrIoButtonStateAPI();

    //开机3s后松开按钮
    if((0 == state)
        && (1 == sHisState)
        && ((BSPGetOSMillSecTimeAPI() >= 3000) || (FirstPowerFlag == 1)))
    {
        //记录按钮松开标志
        sOffFlag = 1;
    }
    //开机5s后按钮为松开状态
    else if((0 == state)
        && ((BSPGetOSMillSecTimeAPI() >= 5000) || (FirstPowerFlag == 1)))
    {
        //当前为关机状态
        if(eWORK_OFF == GetGWorkStateAPI())
        {
            //清除按钮松开标志
            sOffFlag = 0;
        }
        //当前非关机状态
        else
        {
            //记录按钮松开标志
            sOffFlag = 1;
        }
    }

    //按钮已松开过
    if(1 == sOffFlag)
    {
        //再次按下按钮
        if(1 == state)
        {
            //连续按下4s,切换系统为关机
            if(sTimer >= 4)
            {
                //设置停机请求
                sButtonStopReq = 1;

                //非充放电
                //if((eCURR_IDLE == GetGChgDhgStateAPI()))
                if((GetGSampOutCurrAPI() >= -20) && (GetGSampOutCurrAPI() <= 30))
                {
                    //设置关机命令
                    gGSysCmdCtrl_86[eSysCmdCtrl86_State] = eCTRL_POWEROFF;
                    RcvGroupWorkStateCmdAPI(eCMDPORT_INTE);
                }
            }
            //连续按下2s,切换系统为停机
            else if(sTimer >= 2)
            {
                //设置停机请求
                sButtonStopReq = 1;

                //非充放电
                //if((eCURR_IDLE == GetGChgDhgStateAPI()))
                if((GetGSampOutCurrAPI() >= -20) && (GetGSampOutCurrAPI() <= 30))
                {
                    //设置正常停机命令
                    gGSysCmdCtrl_86[eSysCmdCtrl86_State] = eCTRL_NORSTOP;
                    RcvGroupWorkStateCmdAPI(eCMDPORT_INTE);
                    sTimer++;
                }
            }
            else
            {
                sTimer++;
            }
        }
        //松开按钮
        else
        {
            sTimer = 0;

            //清除停机请求
            sButtonStopReq = 0;
        }
    }
    //开机未松开过按钮
    else
    {
        //按钮按着
        if(1 == state)
        {
            //连续按下30s,系统充放电保护或故障保护则进入紧急模式再启动
            if(sTimer >= 30)
            {
                //系统充放电保护或故障保护
                if((ePRO_SYSPRO == GetGProtectStateAPI())
                    || (eWORK_ERR == GetGWorkStateAPI()))
                {
                    //切换紧急模式命令
                    gGSysCmdCtrl_86[eSysCmdCtrl86_Mode] = eMODE_EMERG;
                    RcvGroupWorkModeCmdAPI(eCMDPORT_INTE);
                }
                //系统待机状态
                else if(eWORK_IDLE == GetGWorkStateAPI())
                {
                    //设置正常启动命令
                    gGSysCmdCtrl_86[eSysCmdCtrl86_State] = eCTRL_NORSTART;
                    RcvGroupWorkStateCmdAPI(eCMDPORT_INTE);
                }
            }
            //连续按下3s,切换系统为启动
            else if(sTimer >= 3)
            {
                //设置正常启动命令
                gGSysCmdCtrl_86[eSysCmdCtrl86_State] = eCTRL_NORSTART;
            	RcvGroupWorkStateCmdAPI(eCMDPORT_INTE);
                sTimer++;
            }
            //连续按下1s,切换系统为开机
            else if(sTimer >= 1)
            {
                //设置开机命令
                gGSysCmdCtrl_86[eSysCmdCtrl86_State] = eCTRL_POWERON;
            	RcvGroupWorkStateCmdAPI(eCMDPORT_INTE);
                sTimer++;
            }
            else
            {
                sTimer++;
            }
        }
        //松开按钮
        else
        {
            sTimer = 0;
        }

        //清除停机请求
        sButtonStopReq = 0;
    }

    //记录当前IO状态
    sHisState = state;
    #endif
    #endif
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++) //按两簇初始化
    {
        if(clunum >= 2)
        {
            return;
        }
        //故障状态(间隔12H自动清除故障, SOS相关后续再实现
        if((eWORK_ERR == GetGWorkStateAPI(clunum)) || (ePRO_SYSPRO == GetGProtectStateAPI(clunum)))
        {
            //超过12H
            if(GetSysSecondTimeAPI() >= (sHisTime[clunum] + 43200))
            {
                //设置清除当前故障命令
                if(clunum == 0)
                {
                    gSysCtrlCmd_65[eSysCtrlCmd_SysClear] = eCLR_NOWERR;
                }
                else
                {
                    gSysCtrlCmd_66[eSysCtrlCmd_SysClear] = eCLR_NOWERR;
                }
                RcvGroupSysClrCmdAPI(clunum, eCMDPORT_INTE);
                //记录时间
                sHisTime[clunum] = GetSysSecondTimeAPI();
            }
            //时间变小(清除时间)
            else if((GetSysSecondTimeAPI() + 10) < sHisTime[clunum])
            {
                //记录时间
                sHisTime[clunum] = GetSysSecondTimeAPI();
            }
        }
        //非故障状态
        else
        {
            //记录时间
            sHisTime[clunum] = GetSysSecondTimeAPI();
        }
    }
}
/*=================================================================================================
** @Name      : void GroupSysPwrOffProTask(void)
** @Input     : void
** @Output    : void
** @Function  : 主控系统关机下电保护任务
** @The notes : 2s周期
**===============================================================================================*/
void GroupSysPwrOffProTask(void)
{
    if (gSysCtrlCmd_65 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysCtrlCmd_65 null!!!\r\n", __func__);
        return;
    }
    if (gSysCtrlCmd_66 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysCtrlCmd_66 null!!!\r\n", __func__);
        return;
    }
	/*电池堆未使能时有效*/
	#if(1 != PILE_MANAG_EN)
	//自供电(按钮或开关)时才进行下电保护
	#if((0 == BMU_DC_PWR_TYPE) || (1 == BMU_DC_PWR_TYPE))
	{
//		static u32 sHL3Timer = 0;
		static u32 sHL4Timer[2] = {0};
		static u32 sErrTimer[2] = {0};
		static u32 sCanTimer[2] = {0};
        for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++) //按两簇初始化
        {
            if(clunum >= 2)
            {
                return;
            }
            //欠压1级
            if((eErr_B == DiagGetErrFilStateAPI(clunum, eERR_ID_CEL_VL1))                 	    //单体欠压1级
                && (GetGCellMinVoltAPI(clunum) <= gGBmuGenPara_102[eBmuGenPara102_VL1Lim])    //电压未回升
                && (0 != gGBmuHigLevPara_103[eBmuHigLevPara103_VL1PwrOffT])             //需要保护
                && (eErr_A == DiagGetErrFilStateAPI(clunum, eERR_ID_LEAK2))                     //未发生绝缘告警
                && (eCURR_CHG != GetGChgDhgStateAPI(clunum))                                  //不充电
                && ((eMODE_AUTO == GetGWorkModeAPI(clunum))                                   //自动模式
                || (eMODE_HAND == GetGWorkModeAPI(clunum))))                                  //手动模式
            {
                //达到延时
                if(sHL4Timer[clunum] >= gGBmuHigLevPara_103[eBmuHigLevPara103_VL1PwrOffT])
                {
                    //设置关机命令
                    if(clunum == 0)
                    {
                        gSysCtrlCmd_65[eSysCtrlCmd_WorkSta] = eCTRL_POWEROFF;
                    }
                    else
                    {
                        gSysCtrlCmd_66[eSysCtrlCmd_WorkSta] = eCTRL_POWEROFF;
                    }
                    
                    RcvGroupWorkStateCmdAPI(clunum, eCMDPORT_INTE);
                }
                else
                {
                    sHL4Timer[clunum]++;
                }
            }
            else
            {
                sHL4Timer[clunum] = 0;
            }
            //系统故障
            if((eWORK_ERR == GetGWorkStateAPI(clunum))                                        //故障状态
                && (0 != gGBmuHigLevPara_103[eBmuHigLevPara103_ErrPWrOffT])             //需要保护
                && (eErr_A == DiagGetErrFilStateAPI(clunum, eERR_ID_LEAK2))                     //未发生绝缘告警
                && ((eMODE_AUTO == GetGWorkModeAPI(clunum))                                   //自动模式
                || (eMODE_HAND == GetGWorkModeAPI(clunum))))                                  //手动模式
            {
                //达到延时
                if(sErrTimer[clunum] >= gGBmuHigLevPara_103[eBmuHigLevPara103_ErrPWrOffT])
                {
                    //设置关机命令
                    if(clunum == 0)
                    {
                        gSysCtrlCmd_65[eSysCtrlCmd_WorkSta] = eCTRL_POWEROFF;
                    }
                    else
                    {
                        gSysCtrlCmd_66[eSysCtrlCmd_WorkSta] = eCTRL_POWEROFF;
                    }
                    RcvGroupWorkStateCmdAPI(clunum, eCMDPORT_INTE);
                }
                else
                {
                    sErrTimer[clunum]++;
                }
            }
            else
            {
                sErrTimer[clunum] = 0;
            }

            /*本簇IMIU4外部通信正常(COFF扇出按簇归属后,两ID中仅本簇设备会被置位)*/
            if((eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_EXT_COFF))
                && (eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_PER_COFF)))
            {
                /*记录时间*/
                sCanTimer[clunum] = GetSysSecondTimeAPI();
            }
            /*本簇IMIU4断线超时(2026-09-18随COFF按簇扇出补入PER_COFF,原仅EXT_COFF)*/
            else if(ABS(GetSysSecondTimeAPI(), sCanTimer[clunum]) >= gGBmuHigLevPara_103[eBmuHigLevPara103_CommPwrOffT])
            {
                //外部通讯故障下电保护
                if((0 != gGBmuHigLevPara_103[eBmuHigLevPara103_CommPwrOffT])            //需要保护
                    && (eErr_A == DiagGetErrFilStateAPI(clunum, eERR_ID_LEAK2))                 //未发生绝缘告警
                    && (eCURR_IDLE == GetGChgDhgStateAPI(clunum))                             //不充放电
                    && ((eMODE_AUTO == GetGWorkModeAPI(clunum))                               //自动模式
                    || (eMODE_HAND == GetGWorkModeAPI(clunum))))                              //手动模式
                {
                    //设置关机命令
                    if(clunum == 0)
                    {
                        gSysCtrlCmd_65[eSysCtrlCmd_WorkSta] = eCTRL_POWEROFF;
                    }
                    else
                    {
                        gSysCtrlCmd_66[eSysCtrlCmd_WorkSta] = eCTRL_POWEROFF;
                    }
                    RcvGroupWorkStateCmdAPI(clunum, eCMDPORT_INTE);
                }
            }
        }
	}
	#endif
	#endif
}

/*=================================================================================================
** @Name      : u8 GetButtonStopReqStateAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 按钮请求停机状态标志:0:未请求 1:请求停机
** @Function  : 获取按钮请求停机状态
** @The notes :
**===============================================================================================*/
u8 GetButtonStopReqStateAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
	return(sButtonStopReq[clunum]);
}