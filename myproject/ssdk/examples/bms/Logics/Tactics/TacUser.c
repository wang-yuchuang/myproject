/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : TacUser.c
** @Founder      : ZQD808
** @Date         : 2026.03.23
** @Function     : 电池策略模块用户接口
** @Instructions : 
**===============================================================================================*/

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
#include "Authority.h"
#include "InsuCheck.h"
//#include "PortCheck.h"
//#include "EventUser.h"
//#include "RecordInterface.h"
#include "ParaInterface.h"
#include "CalcInterface.h"
#include "DiagInterface.h"
#include "StaInterface.h"
#include "TacInterface.h"
#include "GroupTactic.h"
#include "GroupDebug.h"
#include "GroupMode.h"
#include "GroupState.h"
#include "GroupProtect.h"
#include "CurrLimFall.h"
//#include "EventUser.h"
//#include "GroupMsg.h"
//#include "CANUser.h"
#include "DiagCheck.h"
#include "DiagFilter.h"
#include "EnergyStatis.h"
#include "DodRcdStatis.h"
#include "TacUser.h"
#include "GroupPara.h"
#include "GroupInfo.h"
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>

extern u16 *gSysCtrlCmd_65;          /*组1系统控制命令*/
extern u16 *gSysCtrlCmd_66;          /*组2系统控制命令*/
extern u16 gGBmuGenPara_102[];  	 /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	 /*主控高级参数值*/
extern u8 gGHardPara_104[];          /*主控硬件参数配置信息*/
extern u16 *gUserCtrlCmd_69;
extern u16 *gCtrlCmdInfo_70;
/*=================================================================================================
** @Name      : void RcvGroupWorkStateCmdAPI(u8 clunum, u8 port)
** @Input     : clunum:簇编号 port:接收命令端口:[0,4]CAN口 [5,9]RS485口 [10,12]按钮或内部口
** @Output    : void
** @Function  : 电池组接收切换工作状态命令处理函数
** @The notes : 通信口或按钮命令
**===============================================================================================*/
void RcvGroupWorkStateCmdAPI(u8 clunum, u8 port)
{
    u8 cmd = 0;
    if(clunum >= 2)
    {
        return;
    }
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
    if(clunum == 0)
    {
        cmd = gSysCtrlCmd_65[eSysCtrlCmd_WorkSta];
    }
    else
    {
        cmd = gSysCtrlCmd_66[eSysCtrlCmd_WorkSta];
    }
	
    if((eCTRL_NORSTOP == cmd)                                       //正常停机命令
        || (eCTRL_FORCSTOP == cmd)                                  //强制停机命令
        || (eCTRL_EMERSTOP == cmd)                                  //紧急停机命令
        || (eCTRL_POWEROFF == cmd))                                 //关机命令
    {
    	/*产生事件*/
    	//EventRcdDisplayUserAPI(eEVN_RCDSORT_CMD, 1, GetGSampOutCurrAPI(), GetSysNowAuthorityAPI(port), cmd, port);

        TacSetGWorkStateCmdAPI(clunum, cmd);                                //设置控制状态命令

        if((eMODE_EMERG == GetGWorkModeAPI(clunum))                       //紧急模式
            || (eMODE_DEBUG == GetGWorkModeAPI(clunum)))                  //调试模式
        {
            //TacRecoverGroupNorModeAPI();                          //退出到正常模式(历史模式)
        }
    }
    else                                                            //启动命令
    {
    	/*产生事件*/
    	//EventRcdDisplayUserAPI(eEVN_RCDSORT_CMD, 1, GetGSampOutCurrAPI(), GetSysNowAuthorityAPI(port), cmd, port);

        TacSetGWorkStateCmdAPI(clunum, cmd);                                //设置控制状态命令
    }
}
/*=================================================================================================
** @Name      : void RcvGroupWorkModeCmdAPI(u8 clunum, u8 port)
** @Input     : clunum:簇编号 port:接收命令端口:[0,4]CAN口 [5,9]RS485口 [10,12]按钮或内部口
** @Output    : void
** @Function  : 电池组接收切换工作模式切换命令处理函数
** @The notes : 通信口或按钮或内部命令
**===============================================================================================*/
void RcvGroupWorkModeCmdAPI(u8 clunum, u8 port)
{
    u8 cmd = 0;
    if(clunum >= 2)
    {
        return;
    }
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
    if(clunum == 0)
    {
        cmd = gSysCtrlCmd_65[eSysCtrlCmd_WorkMode];
    }
    else
    {
        cmd = gSysCtrlCmd_66[eSysCtrlCmd_WorkMode];
    }
    

    /*紧急模式需要高级权限,调试模式需要超级权限*/
    if(((eMODE_EMERG == cmd) && (eAuthL_Service != GetSysNowAuthorityAPI(port)) && (eAuthL_Admin != GetSysNowAuthorityAPI(port)))
		|| ((eMODE_DEBUG == cmd) && (eAuthL_Admin != GetSysNowAuthorityAPI(port))))
	{
    	return;
	}

    if((eWORK_IDLE == GetGWorkStateAPI(clunum))                           //待命状态
		|| (eWORK_ERR == GetGWorkStateAPI(clunum)))                       //故障状态
	{
    	/*产生事件*/
    	//EventRcdDisplayUserAPI(eEVN_RCDSORT_CMD, 2, GetGSampOutCurrAPI(), GetSysNowAuthorityAPI(port), cmd, port);

    	TacSetGroupWorkModeAPI(clunum, (e_WorkMode)cmd);                    //设置模式命令
	}
    else if(eWORK_RUN == GetGWorkStateAPI(clunum))                        //运行状态
	{
    	/*产生事件*/
    	//EventRcdDisplayUserAPI(eEVN_RCDSORT_CMD, 2, GetGSampOutCurrAPI(), GetSysNowAuthorityAPI(port), cmd, port);

    	TacSetGroupWorkModeAPI(clunum, (e_WorkMode)cmd);                    //设置模式命令

    	/*当前处于特殊模式*/
    	if((eMODE_EMERG == GetGWorkModeAPI(clunum))
    		|| (eMODE_DEBUG == GetGWorkModeAPI(clunum)))
    	{
        	/*切换到正常模式*/
        	if((eMODE_AUTO == cmd) || (eMODE_HAND == cmd))
        	{
            	/*产生状态控制事件*/
            	//EventRcdDisplayUserAPI(eEVN_RCDSORT_CMD, 1, GetGSampOutCurrAPI(), GetSysNowAuthorityAPI(port), eCTRL_NORSTOP, port);

                TacSetGWorkStateCmdAPI(clunum, eCTRL_NORSTOP);              //设置控制停机状态命令
        	}
    	}
	}
    else if((eWORK_START == GetGWorkStateAPI(clunum))                     //启动状态
    	|| (eWORK_STOP == GetGWorkStateAPI(clunum)))                      //停机状态
	{
    	/*当前处于特殊模式*/
    	if((eMODE_EMERG == GetGWorkModeAPI(clunum))
    		|| (eMODE_DEBUG == GetGWorkModeAPI(clunum)))
    	{
        	/*产生模式事件*/
        	//EventRcdDisplayUserAPI(eEVN_RCDSORT_CMD, 2, GetGSampOutCurrAPI(), GetSysNowAuthorityAPI(port), cmd, port);

        	TacSetGroupWorkModeAPI(clunum, (e_WorkMode)cmd);                //设置模式命令

        	/*切换到正常模式*/
        	if((eMODE_AUTO == cmd) || (eMODE_HAND == cmd))
        	{
            	/*产生状态控制事件*/
            	//EventRcdDisplayUserAPI(eEVN_RCDSORT_CMD, 1, GetGSampOutCurrAPI(), GetSysNowAuthorityAPI(port), eCTRL_NORSTOP, port);

                TacSetGWorkStateCmdAPI(clunum, eCTRL_NORSTOP);              //设置控制停机状态命令
        	}
    	}
	}
}

/*=================================================================================================
** @Name      : void RcvGroupSysClrCmdAPI(u8 clunum, u8 port)
** @Input     : clunum:簇编号 port:接收命令端口:[0,4]CAN口 [5,9]RS485口 [10,12]按钮或内部口
** @Output    : void
** @Function  : 电池组接收切换工作状态命令处理函数
** @The notes : 通信口或按钮命令
**===============================================================================================*/
void RcvGroupSysClrCmdAPI(u8 clunum, u8 port)
{
    if(clunum >= 2)
    {
        return;
    }
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

    if(clunum == 0)
    {
        if(eCLR_NOWERR == gSysCtrlCmd_65[eSysCtrlCmd_SysClear])        /*清除当前故障命令*/
        {
            TacClrGCtrlErrFlagAPI(clunum);                                    //清除系统控制故障标志
            //后续实现
            ClrInsuAdhesErrFlagAPI(clunum);                                   //清除接触器粘连和绝缘检测故障标志
            DiagClrPerDevExpFlagAPI(clunum);                                  //清除外围传感器故障标志
            TacClrGroupProFlagAPI(clunum);                                    //清除故障保护标志

            DiagClrErrNorStateAPI(clunum, 0xff);                                //清除所有故障

            //DiagClrGroupSosLevel(clunum, 0);                                     /*清除SOS级别(非锁定)*/
        }
        else if(eCLR_ERRRCD == gSysCtrlCmd_65[eSysCtrlCmd_SysClear])   /*清除历史故障记录命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }
            //后续实现
            //RcdClrDateErrRcdUserAPI(0xff);                              /*清除外部EEP或FLASH故障记录*/
            //DiagClrHisErrDisplayAPI();                                  /*清除故障记录显示*/
        }
        else if(eCLR_ERRNUM == gSysCtrlCmd_65[eSysCtrlCmd_SysClear])   /*清除故障数目命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }

            ClrGroupHisErrNumAPI(clunum);                                    //清除故障计数记录
        }
        else if(eCLR_RUNTIME == gSysCtrlCmd_65[eSysCtrlCmd_SysClear])  /*清除运行计时命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }

            ClrSysSecondTimeAPI();                                      //清除运行计时
        }
        else if(eCLR_HISCAP == gSysCtrlCmd_65[eSysCtrlCmd_SysClear])   /*清除累计电量命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }

            ClrGroupEnerCapStatisInfo(clunum);                                //清除累计电量
        }
        else if(eCLR_DODRCD == gSysCtrlCmd_65[eSysCtrlCmd_SysClear])   /*清除历史DOD记录命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }
            //后续实现
            //RcdClrAllDodRcdUserAPI();                                   /*清除外部EEP或FLASH历史DOD记录(本工程无Record模块)*/
            GroupClrDodStatisRcdAPI(clunum);                                /*清除历史DOD记录显示*/
        }
        else if(eCLR_EVENRCD == gSysCtrlCmd_65[eSysCtrlCmd_SysClear])  /*清除事件记录命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }
            //后续实现
            //RcdClrDateEventRcdUserAPI(0xff);                            /*清除外部EEP或FLASH事件记录*/
            //EventClrHisRcdDisplayAPI();                                 /*清除事件记录显示*/
        }
        else if(eCLR_WAVERCD == gSysCtrlCmd_65[eSysCtrlCmd_SysClear])  /*清除故障录波记录命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }
            //后续实现
            //RcdClrDateWaveRcdUserAPI(0xff);                             /*清除外部EEP或FLASH故障录波记录*/
            //DiagClrHisWaveDisplayAPI();                                 /*清除故障录波记录显示*/
        }
        else if(eCLR_HISDATA == gSysCtrlCmd_65[eSysCtrlCmd_SysClear])  /*清除历史数据记录命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }
            //后续实现
            //RcdClrDateDataRcdUserAPI(0xff);                              /*清除外部EEP或FLASH数据记录*/
            //DataClrHisRcdDisplayAPI();                                 /*清除运行数据记录显示*/
        }
        else if(eCLR_FACTORY == gSysCtrlCmd_65[eSysCtrlCmd_SysClear])  /*恢复出厂设置命令*/
        {
            //超级权限未开启
            if(eAuthL_Admin != GetSysNowAuthorityAPI(port))
            {
                return;
            }

            //调试模式下进行
            if(eMODE_DEBUG == GetGWorkModeAPI(clunum))
            {
                ParaRefactoryUserHandleHook();                          /*清除参数配置标志*/
            }
        }
        else if(eCLR_FALCLIM == gSysCtrlCmd_65[eSysCtrlCmd_SysClear])  /*复位充放电降功率限制命令*/
        {
            ResetCurrLimFallAPI(clunum);                                      /*复位充放电降功率限制步骤清零*/
        }
        else if(eCLR_CDHGLIM == gSysCtrlCmd_65[eSysCtrlCmd_SysClear])  /*复位充放电截止限制命令*/
        {
            TacResetGroupBattStateAPI(clunum);                                /*复位充放电截止限制状态清零*/
        }
        else if(eCLR_SOSLOCK == gSysCtrlCmd_65[eSysCtrlCmd_SysClear])  /*清除SOS锁定命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }

            TacClrGCtrlErrFlagAPI(clunum);                                    //清除系统控制故障标志
            //后续实现
            ClrInsuAdhesErrFlagAPI(clunum);                                   //清除接触器粘连和绝缘检测故障标志
            DiagClrPerDevExpFlagAPI(clunum);                                  //清除外围传感器故障标志
            TacClrGroupProFlagAPI(clunum);                                    //清除故障保护标志

            DiagClrErrNorStateAPI(clunum, 0xff);                                //清除所有故障

            //DiagClrGroupSosLevel(0xff);                                 /*清除SOS级别锁定*/
        }
    }
    else
    {
        if(eCLR_NOWERR == gSysCtrlCmd_66[eSysCtrlCmd_SysClear])        /*清除当前故障命令*/
        {
            TacClrGCtrlErrFlagAPI(clunum);                                    //清除系统控制故障标志
            //后续实现
            ClrInsuAdhesErrFlagAPI(clunum);                                   //清除接触器粘连和绝缘检测故障标志
            DiagClrPerDevExpFlagAPI(clunum);                                  //清除外围传感器故障标志
            TacClrGroupProFlagAPI(clunum);                                    //清除故障保护标志

            DiagClrErrNorStateAPI(clunum, 0xff);                                //清除所有故障

            //DiagClrGroupSosLevel(clunum, 0);                                     /*清除SOS级别(非锁定)*/
        }
        else if(eCLR_ERRRCD == gSysCtrlCmd_66[eSysCtrlCmd_SysClear])   /*清除历史故障记录命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }
            //后续实现
            //RcdClrDateErrRcdUserAPI(0xff);                              /*清除外部EEP或FLASH故障记录*/
            //DiagClrHisErrDisplayAPI();                                  /*清除故障记录显示*/
        }
        else if(eCLR_ERRNUM == gSysCtrlCmd_66[eSysCtrlCmd_SysClear])   /*清除故障数目命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }

            ClrGroupHisErrNumAPI(clunum);                                    //清除故障计数记录
        }
        else if(eCLR_RUNTIME == gSysCtrlCmd_66[eSysCtrlCmd_SysClear])  /*清除运行计时命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }

            ClrSysSecondTimeAPI();                                      //清除运行计时
        }
        else if(eCLR_HISCAP == gSysCtrlCmd_66[eSysCtrlCmd_SysClear])   /*清除累计电量命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }

            ClrGroupEnerCapStatisInfo(clunum);                                //清除累计电量
        }
        else if(eCLR_DODRCD == gSysCtrlCmd_66[eSysCtrlCmd_SysClear])   /*清除历史DOD记录命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }
            //后续实现
            //RcdClrAllDodRcdUserAPI();                                   /*清除外部EEP或FLASH历史DOD记录(本工程无Record模块)*/
            GroupClrDodStatisRcdAPI(clunum);                                /*清除历史DOD记录显示*/
        }
        else if(eCLR_EVENRCD == gSysCtrlCmd_66[eSysCtrlCmd_SysClear])  /*清除事件记录命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }
            //后续实现
            //RcdClrDateEventRcdUserAPI(0xff);                            /*清除外部EEP或FLASH事件记录*/
            //EventClrHisRcdDisplayAPI();                                 /*清除事件记录显示*/
        }
        else if(eCLR_WAVERCD == gSysCtrlCmd_66[eSysCtrlCmd_SysClear])  /*清除故障录波记录命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }
            //后续实现
            //RcdClrDateWaveRcdUserAPI(0xff);                             /*清除外部EEP或FLASH故障录波记录*/
            //DiagClrHisWaveDisplayAPI();                                 /*清除故障录波记录显示*/
        }
        else if(eCLR_HISDATA == gSysCtrlCmd_66[eSysCtrlCmd_SysClear])  /*清除历史数据记录命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }
            //后续实现
            //RcdClrDateDataRcdUserAPI(0xff);                              /*清除外部EEP或FLASH数据记录*/
            //DataClrHisRcdDisplayAPI();                                 /*清除运行数据记录显示*/
        }
        else if(eCLR_FACTORY == gSysCtrlCmd_66[eSysCtrlCmd_SysClear])  /*恢复出厂设置命令*/
        {
            //超级权限未开启
            if(eAuthL_Admin != GetSysNowAuthorityAPI(port))
            {
                return;
            }

            //调试模式下进行
            if(eMODE_DEBUG == GetGWorkModeAPI(clunum))
            {
                ParaRefactoryUserHandleHook();                          /*清除参数配置标志*/
            }
        }
        else if(eCLR_FALCLIM == gSysCtrlCmd_66[eSysCtrlCmd_SysClear])  /*复位充放电降功率限制命令*/
        {
            ResetCurrLimFallAPI(clunum);                                      /*复位充放电降功率限制步骤清零*/
        }
        else if(eCLR_CDHGLIM == gSysCtrlCmd_66[eSysCtrlCmd_SysClear])  /*复位充放电截止限制命令*/
        {
            TacResetGroupBattStateAPI(clunum);                                /*复位充放电截止限制状态清零*/
        }
        else if(eCLR_SOSLOCK == gSysCtrlCmd_66[eSysCtrlCmd_SysClear])  /*清除SOS锁定命令*/
        {
            //高级权限未开启
            if(eAuthL_Service > GetSysNowAuthorityAPI(port))
            {
                return;
            }

            TacClrGCtrlErrFlagAPI(clunum);                                    //清除系统控制故障标志
            //后续实现
            ClrInsuAdhesErrFlagAPI(clunum);                                   //清除接触器粘连和绝缘检测故障标志
            DiagClrPerDevExpFlagAPI(clunum);                                  //清除外围传感器故障标志
            TacClrGroupProFlagAPI(clunum);                                    //清除故障保护标志

            DiagClrErrNorStateAPI(clunum, 0xff);                                //清除所有故障

            //DiagClrGroupSosLevel(0xff);                                 /*清除SOS级别锁定*/
        }
    }

	/*产生事件*/
	//EventRcdDisplayUserAPI(eEVN_RCDSORT_CMD, 3, GetGSampOutCurrAPI(), GetSysNowAuthorityAPI(port), gGSysCmdCtrl_86[eSysCmdCtrl86_ClrCmd], port);
}

/*=================================================================================================
** @Name      : void RcvGroupReqLeakCheckCmdAPI(u8 clunum, u8 port)
** @Input     : clunum:簇编号 port:接收命令端口:[0,4]CAN口 [5,9]RS485口
** @Output    : void
** @Function  : 电池组接收请求绝缘检测命令处理函数
** @The notes : 通信口
**===============================================================================================*/
void RcvGroupReqLeakCheckCmdAPI(u8 clunum)
{
    
    //后续再实现绝缘
	/*请求开始绝缘检测*/
	if(1 == gUserCtrlCmd_69[clunum])
	{
		/*请求重新绝缘检测*/
		UserReqInsuCheckAPI(clunum);
        gUserCtrlCmd_69[clunum] = 0;

	}

	/*产生事件*/
	//EventRcdDisplayUserAPI(eEVN_RCDSORT_CMD, 11, GetGSampOutCurrAPI(), GetSysNowAuthorityAPI(port), gGUserCtrl_87[eUserCtrl87_LeakCheck], port);
    
}
/*=================================================================================================
** @Name      : void TacCtrlGroupSwhStateAPI(e_GroupSwhSort swhSort, e_GPIOState state)
** @Input     : swhSort:接触器名称[0,11] state:控制接触器状态 0:断开 1:闭合
** @Output    : void
** @Function  : 控制电池组相应接触器状态
** @The notes : 控制HS/LS/干接点
**===============================================================================================*/
void TacCtrlGroupSwhStateAPI(e_GroupSwhSort swhSort, e_GPIOState state)
{
    u8 flag = 0;

	/*状态有效*/
	if(state < ePSTATE_NUM)
	{
		/*区分接触器名称*/
		switch(swhSort)
		{
			/*簇1主正接触器*/
			case eGroupSwhPK1_1Mai:
				/*记录接触器状态改变*/
	            if(state != TacGetGroupSwhCtrlStateAPI(eGroupSwhPK1_1Mai))
	            {
	                flag = 1;
	            }

				/*控制LS1*/
				//(void)DEVGPIOSetPortState(ePOut_LDRVK0, state);
				/*控制HS1*/
				//(void)DEVGPIOSetPortState(ePOut_DRVK0, state);
                /*控制PK1*/
                DEVGPIOSetPortState(ePOut_PK1_DRV, state);
				break;

			/*簇1负极接触器*/
			case eGroupSwhPK2_1Neg:
				/*记录接触器状态改变*/
	            if(state != TacGetGroupSwhCtrlStateAPI(eGroupSwhPK2_1Neg))
	            {
	                flag = 1;
	            }

				/*控制LS3*/
				//(void)DEVGPIOSetPortState(ePOut_LDRVK2, state);
				/*控制HS3*/
				//(void)DEVGPIOSetPortState(ePOut_DRVK2, state);
                /*控制PK2*/
                DEVGPIOSetPortState(ePOut_PK2_DRV, state);
				break;
            /*簇2主正接触器*/
			case eGroupSwhPK3_2Mai:
				/*记录接触器状态改变*/
	            if(state != TacGetGroupSwhCtrlStateAPI(eGroupSwhPK3_2Mai))
	            {
	                flag = 1;
	            }

				/*控制LS1*/
				//(void)DEVGPIOSetPortState(ePOut_LDRVK0, state);
				/*控制HS1*/
				//(void)DEVGPIOSetPortState(ePOut_DRVK0, state);
                /*控制PK1*/
                DEVGPIOSetPortState(ePOut_PK3_DRV, state);
				break;

			/*簇2负极接触器*/
			case eGroupSwhPK4_2Neg:
				/*记录接触器状态改变*/
	            if(state != TacGetGroupSwhCtrlStateAPI(eGroupSwhPK4_2Neg))
	            {
	                flag = 1;
	            }

				/*控制LS3*/
				//(void)DEVGPIOSetPortState(ePOut_LDRVK2, state);
				/*控制HS3*/
				//(void)DEVGPIOSetPortState(ePOut_DRVK2, state);
                /*控制PK2*/
                DEVGPIOSetPortState(ePOut_PK4_DRV, state);
				break;

			/*簇1禁充禁放*/
			case eGroupSwhNK1_1Pcs:
				/*控制HS4*/
				(void)DEVGPIOSetPortState(ePOut_NK1_DRV, state);
				break;

			/*簇2禁充禁放*/
			case eGroupSwhNK2_2Pcs:
				/*控制HS4*/
				(void)DEVGPIOSetPortState(ePOut_NK2_DRV, state);
				break;

			/*远程急停*/
			case eGroupSwhNK3_Emer:
				/*控制干接点3*/
				(void)DEVGPIOSetPortState(ePOut_NK3_DRV, state);
				break;

			/*空调通讯控制*/
			case eGroupSwhNK4_2Air:
				/*控制干接点3*/
				(void)DEVGPIOSetPortState(ePOut_NK4_DRV, state);
				break;

			default:
				break;
		}

		/*接触器状态改变则发送一帧报文*/
	    if(1 == flag)
	    {
	    	//PortStateCheckTask();           //更新接触器状态信息
	    	//SendCanGroupPortStateInfoMsg(); //上传输入输出状态信息
	    }
	}
}
/*=================================================================================================
** @Name      : e_GPIOState TacGetGroupSwhBackStateAPI(e_GroupSwhSort swhSort)
** @Input     : swhSort:接触器名称[0,11]
** @Output    : 电池组相应接触器当前回检状态 0:断开 1:闭合
** @Function  : 获取电池组相应接触器当前回检状态
** @The notes : 回检信号(输出信号/板级回检信号/反馈输入信号)
**===============================================================================================*/
e_GPIOState TacGetGroupSwhBackStateAPI(e_GroupSwhSort swhSort)
{
	u8 state = 0;

	/*区分接触器名称*/
	switch(swhSort)
	{
		/*簇1主正接触器*/
		case eGroupSwhPK1_1Mai:
			#if(0 == BMU_SWH_BACK_TYPE)
			/*不回检(控制信号状态) 兜底*/
			state = DEVGPIOGetOutPortState(ePOut_PK1_DRV);
			#else
			/*外部信号回检: DI1反映簇1主正接触器反馈(1=闭合/0=断开)*/
			state = DEVGPIOGetInPortState(ePIn_DI1);
			#endif
			break;

		/*簇1负极接触器*/
		case eGroupSwhPK2_1Neg:
			#if(0 == BMU_SWH_BACK_TYPE)
			/*不回检(控制信号状态) 兜底*/
			state = DEVGPIOGetOutPortState(ePOut_PK2_DRV);
			#else
			/*外部信号回检: DI2反映簇1负极接触器反馈(1=闭合/0=断开)*/
			state = DEVGPIOGetInPortState(ePIn_DI2);
			#endif
			break;

		/*簇2主正接触器*/
		case eGroupSwhPK3_2Mai:
			#if(0 == BMU_SWH_BACK_TYPE)
			/*不回检(控制信号状态) 兜底*/
			state = DEVGPIOGetOutPortState(ePOut_PK3_DRV);
			#else
			/*外部信号回检: DI3反映簇2主正接触器反馈(1=闭合/0=断开)*/
			state = DEVGPIOGetInPortState(ePIn_DI3);
			#endif
			break;

		/*簇2负极接触器*/
		case eGroupSwhPK4_2Neg:
			#if(0 == BMU_SWH_BACK_TYPE)
			/*不回检(控制信号状态) 兜底*/
			state = DEVGPIOGetOutPortState(ePOut_PK4_DRV);
			#else
			/*外部信号回检: DI4反映簇2负极接触器反馈(1=闭合/0=断开)*/
			state = DEVGPIOGetInPortState(ePIn_DI4);
			#endif
			break;

		/*簇1禁充禁放*/
		case eGroupSwhNK1_1Pcs:
			/*NK1状态*/
			state = DEVGPIOGetOutPortState(ePOut_NK1_DRV);
			break;


		/*簇2禁充禁放*/
		case eGroupSwhNK2_2Pcs:
			/*NK1状态*/
			state = DEVGPIOGetOutPortState(ePOut_NK2_DRV);
			break;

		/*急停*/
		case eGroupSwhNK3_Emer:
			/*干接点2状态*/
			state = DEVGPIOGetOutPortState(ePOut_NK3_DRV);
			break;

		/*放电干接点*/
		case eGroupSwhNK4_2Air:
			/*干接点3状态*/
			state = DEVGPIOGetOutPortState(ePOut_NK4_DRV);
			break;

		default:
			return((e_GPIOState)0xff);
	}

	return((e_GPIOState)state);
}
/*=================================================================================================
** @Name      : void TacCtrlGroupRunSwhAllOffAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 控制电池组所有运行接触器均断开
** @The notes : 控制主正/预充/主负接触器
**===============================================================================================*/
void TacCtrlGroupRunSwhAllOffAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return;
    }
    if(clunum == 0)
    {
        /*主正接触器*/
        (void)DEVGPIOSetPortState(ePOut_PK1_DRV, ePSTATE_OFF);
        /*负极接触器*/
        (void)DEVGPIOSetPortState(ePOut_PK2_DRV, ePSTATE_OFF);

    }
    else
    {
        /*主正接触器*/
        (void)DEVGPIOSetPortState(ePOut_PK3_DRV, ePSTATE_OFF);
        /*负极接触器*/
        (void)DEVGPIOSetPortState(ePOut_PK4_DRV, ePSTATE_OFF);
    }
}
/*=================================================================================================
** @Name      : e_GPIOState TacGetGroupSwhCtrlStateAPI(e_GroupSwhSort swhSort)
** @Input     : swhSort:接触器名称[0,11]
** @Output    : 电池组相应接触器当前控制输出状态 0:断开 1:闭合
** @Function  : 获取电池组相应接触器当前控制输出状态
** @The notes : 控制信号
**===============================================================================================*/
e_GPIOState TacGetGroupSwhCtrlStateAPI(e_GroupSwhSort swhSort)
{
	u8 state = ePSTATE_OFF;

	/*区分接触器名称*/
	switch(swhSort)
	{
		/*簇1主正接触器*/
		case eGroupSwhPK1_1Mai:
			/*PK1闭合*/
			if(ePSTATE_ON == DEVGPIOGetOutPortState(ePOut_PK1_DRV))
			{
				state = 1;
			}
			else
			{
				state = 0;
			}
			break;

		/*簇1负极接触器*/
		case eGroupSwhPK2_1Neg:
			/*HS2状态*/
			if(ePSTATE_ON == DEVGPIOGetOutPortState(ePOut_PK2_DRV))
			{
				state = 1;
			}
			else
			{
				state = 0;
			}
			break;

		/*簇2主正接触器*/
		case eGroupSwhPK3_2Mai:
			/*PK1闭合*/
			if(ePSTATE_ON == DEVGPIOGetOutPortState(ePOut_PK3_DRV))
			{
				state = 1;
			}
			else
			{
				state = 0;
			}
			break;

		/*簇2负极接触器*/
		case eGroupSwhPK4_2Neg:
			/*HS2状态*/
			if(ePSTATE_ON == DEVGPIOGetOutPortState(ePOut_PK4_DRV))
			{
				state = 1;
			}
			else
			{
				state = 0;
			}
			break;

		/*簇1禁充禁放*/
		case eGroupSwhNK1_1Pcs:
			/*NK1状态*/
			state = DEVGPIOGetOutPortState(ePOut_NK1_DRV);
			break;
        /*簇2禁充禁放*/
		case eGroupSwhNK2_2Pcs:
			/*NK2状态*/
			state = DEVGPIOGetOutPortState(ePOut_NK2_DRV);
			break;
        /*远程急停*/
		case eGroupSwhNK3_Emer:
			/*HS4状态*/
			state = DEVGPIOGetOutPortState(ePOut_NK3_DRV);
			break;
        /*空调通讯控制*/
		case eGroupSwhNK4_2Air:
			/*HS4状态*/
			state = DEVGPIOGetOutPortState(ePOut_NK4_DRV);
			break;

		default:
			return((e_GPIOState)0xff);
	}

	return((e_GPIOState)state);
}

/*=================================================================================================
** @Name      : u8 TacGetGroupRunSwhStateAPI(u8 clunum)
** @Input     : void
** @Output    : 电池组运行接触器状态[低4位:控制状态 高4位:回检状态](按bit位表示接触器 bit0:主正接触器控制 bit1:预充接触器控制 bit2:主负接触器控制;bit4:主正接触器回检 bit5:预充接触器回检 bit6:主负接触器回检)
** @Function  : 获取电池组运行接触器当前状态
** @The notes : 控制信号和回检信号
**===============================================================================================*/
u8 TacGetGroupRunSwhStateAPI(u8 clunum)
{
	u8 state = 0;
    if(clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        /*簇1主正接触器控制：PK1闭合*/
        if(ePSTATE_ON == DEVGPIOGetOutPortState(ePOut_PK1_DRV))
        {
            BitSet(state, eGroupSwhPK1_1Mai);
        }

        /*簇1负极接触器控制:PK2闭合*/
        if(ePSTATE_ON == DEVGPIOGetOutPortState(ePOut_PK2_DRV))
        {
            BitSet(state, eGroupSwhPK2_1Neg);
        }

        /*接触器回检: 外部DI反馈*/
        if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK1_1Mai))
        {
            BitSet(state, eGroupSwhPK1_1Mai + 4);
        }

        /*接触器回检: 外部DI反馈*/
        if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK2_1Neg))
        {
            BitSet(state, eGroupSwhPK2_1Neg + 4);
        }
    }
    else
    {
        /*簇2主正接触器控制：PK1闭合*/
        if(ePSTATE_ON == DEVGPIOGetOutPortState(ePOut_PK3_DRV))
        {
            BitSet(state, eGroupSwhPK3_2Mai);
        }

        /*簇2负极接触器控制:PK2闭合*/
        if(ePSTATE_ON == DEVGPIOGetOutPortState(ePOut_PK4_DRV))
        {
            BitSet(state, eGroupSwhPK4_2Neg);
        }

        /*接触器回检: 外部DI反馈*/
        if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK3_2Mai))
        {
            BitSet(state, eGroupSwhPK3_2Mai + 4);
        }

        /*接触器回检: 外部DI反馈*/
        if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK4_2Neg))
        {
            BitSet(state, eGroupSwhPK4_2Neg + 4);
        }
    }
	return(state);
}
/*=================================================================================================
** @Name      : u8 TacGetGroupRunSwhAllOffStateAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组运行接触器均断开状态 FALSE:未全断开 TRUE:全断开
** @Function  : 获取电池组运行接触器均断开状态
** @The notes : 回检信号(输出信号/板级回检信号/反馈输入信号)
**===============================================================================================*/
u8 TacGetGroupRunSwhAllOffStateAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return (FALSE);
    }
    if(clunum == 0)
    {
        /*主正和负极接触器均断开: 外部DI反馈*/
        if((ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK1_1Mai)) && (ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK2_1Neg)))
        {
            return(TRUE);
        }
        else
        {
            return(FALSE);
        }
    }
    //第二簇
    else
    {
        /*主正和负极接触器均断开: 外部DI反馈*/
        if((ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK3_2Mai)) && (ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK4_2Neg)))
        {
            return(TRUE);
        }
        else
        {
            return(FALSE);
        }
    }

}
/*=================================================================================================
** @Name      : u8 TacGetGroupRunSwhAllOnStateAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组运行接触器正负极均闭合状态 FALSE:未全闭合 TRUE:全闭合
** @Function  : 获取电池组运行接触器正负极均闭合状态
** @The notes : 回检信号(输出信号/板级回检信号/反馈输入信号)
**===============================================================================================*/
u8 TacGetGroupRunSwhAllOnStateAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return (FALSE);
    }
    if(clunum == 0)
    {
        /*正极和负极接触器均闭合: 外部DI反馈*/
        if((ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK1_1Mai)) && (ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK2_1Neg)))
        {
            return(TRUE);
        }
        else
        {
            return(FALSE);
        }
    }
    else
    {
        /*正极和负极接触器均闭合: 外部DI反馈*/
        if((ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK3_2Mai)) && (ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK4_2Neg)))
        {
            return(TRUE);
        }
        else
        {
            return(FALSE);
        }
    }
}