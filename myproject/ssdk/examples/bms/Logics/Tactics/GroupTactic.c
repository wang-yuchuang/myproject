/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : GroupTactic.c
** @Founder      : ZQD808
** @Date         : 2019.11.1
** @Function     : 电池控制策略模块接口
**===============================================================================================*/

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "GroupPara.h"
#include "TacInterface.h"
#include "TacUser.h"
#include "GroupInfo.h"
#include "GroupState.h"
#include "GroupProtect.h"
#include "DiagInterface.h"
#include "GroupTactic.h"
#include "DEVGPIO.h"
#include "AuxDevMsg.h"
#include "AuxDevTactic.h"
#include "IMEU3Config.h"                         /*烟感电源DO3控制需要IMEU3设备地址*/
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>

static u8 sGroupWorkCtrlCmd[2] = {0};                            /*记录接收到的工作状态控制命令[0,8]: 0:正常停机 1:正常预充 2:正常启动 3:充电启动 4:放电启动 5:强制停机 6:紧急停机 7:关机 8:开机*/
static u8 sGroupStartStep[2] = {0xff,0xff};                      /*电池组启动步骤(高压上电步骤)*/
static u8 sGroupStopStep[2] = {0xff,0xff};                       /*电池组停机步骤(高压下电步骤)*/
static u8 sEmergStopFlag[2] = {0};                               /*电池组急停标志(按位表示):Bit0:系统内部急停 Bit1:外部命令急停 bit2:按钮急停*/

static u8 sGroupStartFailFlag[2] = {0};                          /*电池组启动异常标志(按位表示):Bit0:系统故障 Bit1:启动受限 Bit2:接触器异常 Bit3:预充失败*/
static u8 sGroupStopFailFlag[2] = {0};                           /*电池组停机异常标志(按位表示):Bit0:系统故障 Bit1:等待电流失败 Bit2:接触器异常 Bit3:断开接触器失败*/

static u16 sGSwhOnCtrlFailFlag[2] = {0};                         /*电池组接触器控制闭合失败标志(按位表示):Bit0:主正接触器 Bit1:预充接触器 Bit2:负极接触器 Bit3:电源接触器*/
static u16 sGSwhOffCtrlFailFlag[2] = {0};                        /*电池组接触器控制断开失败标志(按位表示):Bit0:主正接触器 Bit1:预充接触器 Bit2:负极接触器 Bit3:电源接触器*/

#if(1 == BMU_SMG_SWH_EN)									     /*烟感电源控制使能(有烟感检测时)*/
static u8 sSmgAlarmState = 0;								     /*烟感报警状态(包括电源重启确认真实信号,0:正常 1:正在核实告警 2:已确认告警)*/
#endif


extern u16 gGBmuGenPara_102[];                  	        /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];							/*主控高级参数值*/
extern u8 gGHardPara_104[]; 			                    /*主控硬件参数配置信息*/
extern u16 *gSysStaInfo_50;                                 /*系统状态信息*/
u32 SInfy_Idletime[2] = {0};                                /*系统空闲时间*/

/*=================================================================================================
** @Name      : void TacSetGWorkStateCmdAPI(u8 clunum, u8 cmd)
** @Input     : clunum:簇编号 cmd:电池组工作状态切换控制命令[0,8]:0:正常停止 1:正常预充 2:正常启动 3:充电启动 4:放电启动 5:强制停机 6:紧急停机 7:下电关机 8:上电开机
** @Output    : void
** @Function  : 设置电池组工作状态切换控制命令
** @The notes : 接收到通信命令或按钮命令
**===============================================================================================*/
void TacSetGWorkStateCmdAPI(u8 clunum, u8 cmd)
{
    if(clunum >= 2)
    {
        return;
    }
	/*分辨命令值*/
    switch(cmd)
    {
        /*正常停机命令*/
        case eCTRL_NORSTOP:
        /*强制停机命令*/
        case eCTRL_FORCSTOP:
            if((eWORK_START == GetGWorkStateAPI(clunum))                  /*正在启动*/
                || (eWORK_RUN == GetGWorkStateAPI(clunum)))               /*正在运行*/
            {
                sGroupStopStep[clunum] = 0;                                 /*复位停机步骤*/

                sGroupWorkCtrlCmd[clunum] = cmd;                            /*记录控制命令*/
                TacClrGCtrlErrFlagAPI(clunum);                            /*清除控制失败标志*/
                TacSetGroupWorkStateAPI(clunum, eWORK_STOP);                /*切换为正在停止*/

                sGroupStartStep[clunum] = 0xff;                             /*清除启动步骤*/
            }/*
            else if(eWORK_OFF == GetGWorkStateAPI())                //正在关机
            {
                sGroupStartStep = 0xff;                             //清除启动步骤
                sGroupStopStep = 0xff;                              //清除停机步骤

                sGroupWorkCtrlCmd = eCTRL_POWERON;                  //记录为开机命令
                TacClrGCtrlErrFlagAPI(clunum);                            //清除控制失败标志
                //TacSetGroupWorkStateAPI(eWORK_INIT);              //切换为初始化状态(由开机完成后切换)
            }*/
            else if((eWORK_IDLE == GetGWorkStateAPI(clunum))              /*系统空闲*/
                || (eWORK_ERR == GetGWorkStateAPI(clunum)))               /*系统故障*/
            {
                sGroupWorkCtrlCmd[clunum] = cmd;                            /*记录控制命令*/
                TacClrGCtrlErrFlagAPI(clunum);                            /*清除控制失败标志*/

                sGroupStartStep[clunum] = 0xff;                             /*清除启动步骤*/
                sGroupStopStep[clunum] = 0xff;                              /*清除停机步骤*/
            }
            break;

        /*正常预充命令*/
        case eCTRL_PRECHG:
        /*正常启动命令*/
        case eCTRL_NORSTART:
        /*充电启动命令*/
        case eCTRL_CHGSTART:
        /*放电启动命令*/
        case eCTRL_DHGSTART:
            if((eWORK_IDLE == GetGWorkStateAPI(clunum))                   /*正空闲待命*/
               && (1 == TacGetGroupFitStartStateHook(clunum)))            /*适合启动*/
            {
                sGroupStartStep[clunum] = 0;                                /*复位启动步骤*/
                sGroupWorkCtrlCmd[clunum] = cmd;                            /*记录控制命令*/
                TacClrGCtrlErrFlagAPI(clunum);                            /*清除控制失败标志*/
                TacSetGroupWorkStateAPI(clunum,eWORK_START);               /*切换为正在启动*/

                sGroupStopStep[clunum] = 0xff;                              /*清除停机步骤*/
            }
            else if((eWORK_START == GetGWorkStateAPI(clunum))             /*正在启动*/
                && (eCTRL_PRECHG == sGroupWorkCtrlCmd[clunum])              /*正在预充*/
                && (eCTRL_NORSTART == cmd))                         /*只有预充允许变换正常启动命令*/
            {
                sGroupWorkCtrlCmd[clunum] = cmd;                            /*更新控制命令*/

                sGroupStopStep[clunum] = 0xff;                              /*清除停机步骤*/
            }
            break;

        /*紧急停机命令*/
        case eCTRL_EMERSTOP:
            sGroupStopStep[clunum] = 0xff;
            sGroupStartStep[clunum] = 0xff;

            if((eWORK_START == GetGWorkStateAPI(clunum))                  /*正在启动*/
                || (eWORK_RUN == GetGWorkStateAPI(clunum))                /*正在运行*/
                || (eWORK_STOP == GetGWorkStateAPI(clunum))               /*正在停机*/
                || (eWORK_ERR == GetGWorkStateAPI(clunum)))               /*系统故障*/
            {
                sGroupWorkCtrlCmd[clunum] = cmd;                            /*记录控制命令*/
                TacSetGroupWorkStateAPI(clunum,eWORK_ERR);                 /*切换为故障状态*/

                if(1 == TacGetUserButtonEmergStateHook())           /*按钮信号*/
                {
                    TacSetGEmergStopReason(clunum, eEmergStopReason_Button);/*记录按钮急停*/
                }
                else                                                /*外部命令*/
                {
                    TacSetGEmergStopReason(clunum, eEmergStopReason_ExtCmd);/*记录命令急停*/
                }
            }
            break;

        /*关机机命令*/
        case eCTRL_POWEROFF:
            sGroupStartStep[clunum] = 0xff;

            if((eWORK_START == GetGWorkStateAPI(clunum))                  /*正在启动*/
                || (eWORK_RUN == GetGWorkStateAPI(clunum)))               /*正在运行*/
            {
                sGroupStopStep[clunum] = 0;
                sGroupWorkCtrlCmd[clunum] = cmd;                            /*记录控制命令*/
                TacSetGroupWorkStateAPI(clunum, eWORK_STOP);                /*切换为正在停止状态*/
            }
            else if(eWORK_IDLE == GetGWorkStateAPI(clunum))               /*系统空闲待命*/
            {
                sGroupStopStep[clunum] = 0xff;
                sGroupWorkCtrlCmd[clunum] = cmd;                            /*记录控制命令*/
                TacSetGroupWorkStateAPI(clunum, eWORK_OFF);                 /*切换为关机状态*/
            }
            else if((eWORK_ERR == GetGWorkStateAPI(clunum))               /*系统故障*/
                || (eWORK_STOP == GetGWorkStateAPI(clunum)))              /*正在停机*/
            {
                sGroupWorkCtrlCmd[clunum] = cmd;                            /*记录控制命令*/
                /*TacSetGroupWorkStateAPI(eWORK_OFF);*/             /*切换为关机状态(由停机完成后切换)*/
            }
            break;

        /*开机命令*/
        case eCTRL_POWERON:
            if((eWORK_OFF == GetGWorkStateAPI(clunum))                    /*系统关机*/
                || (eMODE_DEBUG == GetGWorkModeAPI(clunum)))              /*正在调试*/
            {
                sGroupStopStep[clunum] = 0xff;
                sGroupStartStep[clunum] = 0xff;

                sGroupWorkCtrlCmd[clunum] = cmd;                            /*记录控制命令*/
                TacClrGCtrlErrFlagAPI(clunum);                            /*清除控制失败标志*/
                /*TacSetGroupWorkStateAPI(eWORK_INIT);*/            /*切换为初始化状态(由开机完成后切换)*/
            }
            break;

        default:
            break;
    }
}
/*=================================================================================================
** @Name      : void TacGPeriManagTacticTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组外围管控策略任务
** @The notes : 1s周期调用,控制显示灯、输出干接点和ACC激活保持信号
**===============================================================================================*/
void TacGPeriManagTacticTask(void)
{
    u16 expSort = 0;

    //烟感电源控制使能(有烟感检测时)
	#if(1 == BMU_SMG_SWH_EN)
    static u8 sSmgOffStep = 0;
    static u8 sSmgOffFlag = 0;
    static u8 sSmgOffTime = 0;
	#endif
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++) 
    {
        if(clunum >= 2)
        {
            return;
        }
        //系统空闲
        if(eCURR_IDLE == GetGChgDhgStateAPI(clunum))
        {
            if(SInfy_Idletime[clunum] < 0xfffffff0)
            {
                SInfy_Idletime[clunum]++;
            }
        }
        else
        {
            SInfy_Idletime[clunum] = 0;
        }
        /*输出干接点控制策略(运行/禁充/禁放时闭合)*/
        if(eWORK_RUN == GetGWorkStateAPI(clunum))                                     //系统运行
        {
            //充放电禁止干接点在一块
            #if(1 == BMU_CDHG_DO_EN)
            //获取故障等级状态
            expSort = TacGetGNowErrLevelHook(clunum, 0);

            //充放电禁止干接点
            if(((0 == GetGChgLimCurrAPI(clunum))                                      //充电禁止
                && (0 == GetGDhgLimCurrAPI(clunum)))                                  //放电禁止
                || ((TacGetGAskChgProFlagAPI(clunum) > 0)                             //请求充电保护
                && (eCURR_CHG == GetGChgDhgStateAPI(clunum)))                         //仍在充电
                || ((TacGetGAskDhgProFlagAPI(clunum) > 0)                             //请求放电保护
                && (eCURR_DHG == GetGChgDhgStateAPI(clunum)))                         //仍在放电
                || (eErr_SysLim == expSort)                                           //充放电限制
                || (eErr_SysPro == expSort)                                           //充放电保护
                //后续再实现SOS|| (DiagGetGroupSosLevel(clunum) > 0)                                 //SOS异常保护
                || (TacGetGAskSysProFlagAPI(clunum) > 0))                             //请求充放电保护
            {
                if(clunum == 0)
                {
                    TacCtrlGroupSwhStateAPI(eGroupSwhNK1_1Pcs, ePSTATE_ON);             //充放电禁止干接点3闭合
                }
                else
                {
                    TacCtrlGroupSwhStateAPI(eGroupSwhNK2_2Pcs, ePSTATE_ON);             //充放电禁止干接点3闭合
                }
                
            }
            else                                                                    //允许充电或放电
            {
                if(clunum == 0)
                {
                    TacCtrlGroupSwhStateAPI(eGroupSwhNK1_1Pcs, ePSTATE_OFF);                //充放电禁止干接点3断开
                }
                else
                {
                    TacCtrlGroupSwhStateAPI(eGroupSwhNK2_2Pcs, ePSTATE_OFF);                //充放电禁止干接点3断开
                }
            }
            //仅放电禁止干接点在DO3
            #else
            //获取故障等级状态
            expSort = TacGetGNowErrLevelHook(2);

            //放电禁止干接点
            if((0 == GetGDhgLimCurrAPI())                                       //放电禁止
                || (eErr_DhgLim == expSort)                                     //放电限制
                || (eErr_DhgPro == expSort)                                     //放电保护
                || (eErr_SysLim == expSort)                                     //充放电限制
                || (eErr_SysPro == expSort)                                     //充放电保护
                || (DiagGetGroupSosLevel() > 0)                                 //SOS异常保护
                || (TacGetGAskDhgProFlagAPI() > 0))                             //请求放电保护
            {
                TacCtrlGroupSwhStateAPI(eGroupSwh_Dhg, ePSTATE_ON);             //放电禁止干接点3闭合
            }
            else                                                                //允许放电
            {
                TacCtrlGroupSwhStateAPI(eGroupSwh_Dhg, ePSTATE_OFF);            //放电禁止干接点3断开
            }
            #endif
        }
        else                                                                    //系统未运行
        {
            if(clunum == 0)
            {
                TacCtrlGroupSwhStateAPI(eGroupSwhNK1_1Pcs, ePSTATE_ON);                 //放电禁止干接点3闭合
            }
            else
            {
                TacCtrlGroupSwhStateAPI(eGroupSwhNK2_2Pcs, ePSTATE_ON);                 //放电禁止干接点3闭合
            }
            
        }
        //两簇只接一个BMU05，烟感由簇一监控DI信号和控制触发
        if(clunum == 0)
        {
            //烟感电源(有烟感时)
            #if(1 == BMU_SMG_SWH_EN)
            {
                //常态闭合烟感电源判断是否有告警
                if(0 == sSmgOffStep)
                {
                    //发生烟感告警
                    if(1 == TacGetUserSmogSignStateHook())
                    {
                        //未复位过电源
                        if(0 == sSmgOffFlag)
                        {
                            //记录1次事件
                            if(1 != sSmgAlarmState)
                            {
                                //,后续由A核实现DiagUserSelfErrRecord(eUDef_ErrRcd_SmgWarn, eERR_RCDSORT_WARSET, 0,  0);
                            }
                            sSmgAlarmState = 1;										//正在核实烟感告警

                            SendCanImeu3DoCtrlMsg(IMEU3_DEV_ADDR, 1, 3, 0); /*控制烟感电源断开*/
                            sSmgOffStep = 1;										//进行等待2s使烟感完全下电
                            sSmgOffFlag = 1;
                            sSmgOffTime = 0;
                        }
                        //已复位过电源
                        else
                        {
                            //记录1次事件
                            if(2 != sSmgAlarmState)
                            {
                                //DiagUserSelfErrRecord(eUDef_ErrRcd_SmgWarn, eERR_RCDSORT_WARSET, 0,  0);
                            }
                            sSmgOffStep = 0;
                            sSmgAlarmState = 2;										//确认有烟感告警
                            SendCanImeu3DoCtrlMsg(IMEU3_DEV_ADDR, 1, 3, 1); /*保持闭合烟感电源*/
                        }
                    }
                    //未发生烟感告警
                    else
                    {
                        sSmgOffStep = 0;
                        sSmgOffTime = 0;
                        sSmgOffFlag = 0;
                        sSmgAlarmState = 0;											//确认无烟感告警
                        SendCanImeu3DoCtrlMsg(IMEU3_DEV_ADDR, 1, 3, 1); /*保持闭合烟感电源*/
                    }
                }
                //断开电源后等待烟感完全下电
                else if(1 == sSmgOffStep)
                {
                    SendCanImeu3DoCtrlMsg(IMEU3_DEV_ADDR, 1, 3, 0); 		/*保持断开烟感电源*/
                    sSmgOffStep = 2;
                }
                //到达烟感完全下电时间
                else if(2 == sSmgOffStep)
                {
                    SendCanImeu3DoCtrlMsg(IMEU3_DEV_ADDR, 1, 3, 0); 		/*保持断开烟感电源*/
                    sSmgOffStep = 3;
                    sSmgOffFlag = 1;
                    sSmgOffTime = 0;
                }
                //断开电源后再次闭合
                else
                {
                    SendCanImeu3DoCtrlMsg(IMEU3_DEV_ADDR, 1, 3, 1); 		/*保持闭合烟感电源*/
                    if(0 == sSmgOffFlag)                                            //未记录断开电源(错误)
                    {
                        sSmgOffStep = 0;											//直接返回
                        sSmgOffTime = 0;
                        sSmgAlarmState = 0;											//无烟感告警
                    }
                    else															//已记录断开电源(正常)
                    {
                        if((sSmgOffTime >= TAC_SMG_ONCON_T)							//等待10s仍无告警(电源重新上电后等待实际状态)
                            || ((sSmgOffTime >= 5) && (1 == TacGetUserSmogSignStateHook()))) //等待5s已发生烟雾告警
                        {
                            if(1 == TacGetUserSmogSignStateHook())					//仍是发生告警状态
                            {
                                //记录1次事件
                                if(2 != sSmgAlarmState)
                                {
                                    //DiagUserSelfErrRecord(eUDef_ErrRcd_SmgWarn, eERR_RCDSORT_WARSET, 0,  0);
                                }
                                sSmgAlarmState = 2;									//确认告警发生
                            }
                            else
                            {
                                sSmgAlarmState = 0;									//确认告警消除
                                sSmgOffFlag = 0;									//恢复下一周期
                            }

                            sSmgOffStep = 0;
                            sSmgOffTime = 0;
                        }
                        else
                        {
                            sSmgOffTime++;
                        }
                    }
                }
            }
            #endif
        }
    }
}

/*=================================================================================================
** @Name      : void TacGroupStartCtrlTactic(u8 clunum)
** @Input     : void
** @Output    : void
** @Function  : 电池组启动控制策略
** @The notes : 高压上电策略
**===============================================================================================*/
void TacGroupStartCtrlTactic(u8 clunum)
{
    u8 result = 0;
    static u16 sTime[2] = {0};
    static u16 sWaitT[2] = {0};
    static u16 PreWaitTime[2] = {0};  //等待预充电阻闭合时间
    if(clunum >= 2)
    {
        return;
    }
    /*电池充放电保护启动受限*/
    if(0 == TacGetGroupFitStartStateHook(clunum))                         /*不适合启动*/
    {
        /*等待5s*/
    	if(sWaitT[clunum] >= 50)
    	{
			sTime[clunum] = 0;
	    	sWaitT[clunum] = 0;
			sGroupStartStep[clunum] = 0xff;

			/*记录启动失败*/
			TacSetGStartFailReason(clunum, eStartFailReason_StartLim);
			/*TacSetGroupWorkStateAPI(eWORK_STOP);*/           		/*切换到停机状态(由启动失败故障导致切换为故障状态)*/
    	}
    	else
    	{
    		sWaitT[clunum]++;
    	}
    }
    else                                                            /*系统正常无故障*/
    {
    	sWaitT[clunum] = 0;
        switch(sGroupStartStep[clunum])
        {
            /*变量复位*/
            case 0:
                sGroupStartStep[clunum] = 100;                              /*检测外部电路设备*/
                sTime[clunum] = 0;
                break;

            /*等待外部检测完成(30s)*/
            case 100:
                if(TRUE == TacGetGAllPeriphReadyHook(clunum))             /*外部检测完成(电流零点校准/粘连检测/绝缘检测)*/
                {
                    sTime[clunum] = 0;
                    sGroupStartStep[clunum] = 1;                            /*检测负极辅助接触器*/
                }
                else if(sTime[clunum] >= 300)                               /*等待超时*/
                {
                    sTime[clunum] = 0;
                    sGroupStartStep[clunum] = 0xff;

                    /*记录启动失败*/
                    TacSetGStartFailReason(clunum, eStartFailReason_PeripExp);
                    /*TacSetGroupWorkStateAPI(eWORK_STOP);*/        /*切换到停机状态(由启动失败故障导致切换为故障状态)*/
                }
                else
                {
                    sTime[clunum]++;
                }
                break;

            /*检测负极线路开路状态*/
            case 1:
                if(TRUE == TacGetGNegCirReadyHook(clunum))                /*负极线路就绪(GND接外侧采预充电压正常)*/
                {
                    sTime[clunum] = 0;

                    if(eCTRL_CHGSTART == sGroupWorkCtrlCmd[clunum])         /*充电启动*/
                    {
                        sGroupStartStep[clunum] = 7;                        /*先闭合正极主接触器*/
                    }
                    else                                                    /*正常预充或启动*/
                    {
                        sGroupStartStep[clunum] = 2;                        /*先闭合负极辅助接触器*/
                    }
                }
                else
                {
                    sTime[clunum]++;
                    /*长时间等待直至产生回检故障*/
                    if(sTime[clunum] >= 100)
                    {
                        /*记录启动失败*/
                        TacSetGStartFailReason(clunum, eStartFailReason_SwtchExp);
                        /*记录负极接触器断开失败*/
                        if(clunum == 0)
                        {
                            TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK2_1Neg);
                        }
                        else
                        {
                            TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK4_2Neg);
                        }
                        

                        /*切换到停机状态(由启动失败故障导致切换为故障状态)*/
                        /*TacSetGroupWorkStateAPI(eWORK_STOP);*/
                    }
                }
                break;

            /*闭合负极接触器*/
            case 2:
                if(clunum == 0)
                {
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK2_1Neg, ePSTATE_ON);
                }
                else
                {
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK4_2Neg, ePSTATE_ON);
                }
                
                if(clunum == 0)
                {
                    /*负极接触器闭合*/
                    if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK2_1Neg))
                    {
                        sTime[clunum] = 0;

                        /*进入负极接触器闭合线路检测*/
                        sGroupStartStep[clunum] = 3;
                    }
                    /*未闭合*/
                    else
                    {
                        sTime[clunum]++;
                        /*长时间等待直至产生回检故障*/
                        if(sTime[clunum] >= 100)
                        {
                            /*记录启动失败*/
                            TacSetGStartFailReason(clunum, eStartFailReason_SwtchExp);
                            /*记录负极接触器闭合失败*/
                            if(clunum == 0)
                            {
                                TacSetGSwhOnCtrlFailFlag(clunum, eGroupSwhPK2_1Neg);
                            }
                            else
                            {
                                TacSetGSwhOnCtrlFailFlag(clunum, eGroupSwhPK4_2Neg);
                            }
                            

                            /*切换到停机状态(由启动失败故障导致切换为故障状态)*/
                            /*TacSetGroupWorkStateAPI(eWORK_STOP);*/
                        }
                    }
                }
                else
                {
                    /*负极接触器闭合*/
                    if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK4_2Neg))
                    {
                        sTime[clunum] = 0;

                        /*进入负极接触器闭合线路检测*/
                        sGroupStartStep[clunum] = 3;
                    }
                    /*未闭合*/
                    else
                    {
                        sTime[clunum]++;
                        /*长时间等待直至产生回检故障*/
                        if(sTime[clunum] >= 100)
                        {
                            /*记录启动失败*/
                            TacSetGStartFailReason(clunum, eStartFailReason_SwtchExp);
                             /*记录负极接触器闭合失败*/
                            if(clunum == 0)
                            {
                                TacSetGSwhOnCtrlFailFlag(clunum, eGroupSwhPK2_1Neg);
                            }
                            else
                            {
                                TacSetGSwhOnCtrlFailFlag(clunum, eGroupSwhPK4_2Neg);
                            }


                            /*切换到停机状态(由启动失败故障导致切换为故障状态)*/
                            /*TacSetGroupWorkStateAPI(eWORK_STOP);*/
                        }
                    }
                }

                break;

            /*检测负极线路闭合状态*/
            case 3:
            	/*负极线路就绪(总压/预充采样正常/正极未粘连)*/
                if(TRUE == TacGetGNegCirReadyHook(clunum))
                {
                    sTime[clunum] = 0;

                    /*充电启动(主->辅助)*/
                    if(eCTRL_CHGSTART == sGroupWorkCtrlCmd[clunum])
                    {
                        /*进入启动结果判断*/
                        sGroupStartStep[clunum] = 9;
                    }
                    /*放电启动(辅助->主)*/
                    else if(eCTRL_DHGSTART == sGroupWorkCtrlCmd[clunum])
                    {
                        /*进入主接触器闭合控制*/
                        sGroupStartStep[clunum] = 7;
                    }
                    /*正常启动(辅助->预充->主)*/
                    else
                    {
                        /*进入预充接触器闭合控制*/
                        sGroupStartStep[clunum] = 7;
                    }
                }
                /*闭合负极接触器负载电压异常可能主正接触器粘连*/
                else
                {
                    sTime[clunum]++;
                    /*长时间等待直至产生回检故障*/
                    if(sTime[clunum] >= 100)
                    {
                        /*记录启动失败*/
                        TacSetGStartFailReason(clunum, eStartFailReason_SwtchExp);
                        /*记录负极接触器闭合失败*/
                        //TacSetGSwhOnCtrlFailFlag(eGroupSwh_Neg);
                        /*记录主正接触器异常(未断开)*/
                        if(clunum == 0)
                        {
                            TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK1_1Mai);
                        }
                        else
                        {
                            TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK3_2Mai);
                        }
                        

                        /*切换到停机状态(由启动失败故障导致切换为故障状态)*/
                        /*TacSetGroupWorkStateAPI(eWORK_STOP);*/
                    }
                }
                break;

            /*闭合主接触器*/
            case 7:
                if(clunum == 0)
                {
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK1_1Mai, ePSTATE_ON);
                    /*主接触器闭合*/
                    if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK1_1Mai))
                    {
                        sTime[clunum] = 0;

                        /*充电启动(主->辅助)*/
                        if(eCTRL_CHGSTART == sGroupWorkCtrlCmd[clunum])
                        {
                            /*返回控制辅助接触器*/
                            sGroupStartStep[clunum] = 2;
                        }
                        /*放电启动(辅助->主)*/
                        else if(eCTRL_DHGSTART == sGroupWorkCtrlCmd[clunum])
                        {
                            /*进入启动结果判断*/
                            sGroupStartStep[clunum] = 9;
                        }
                        /*正常启动(辅助->预充->主)*/
                        else
                        {
                            /*进入预充断开控制*/
                            sGroupStartStep[clunum] = 9;
                        }
                    }
                    /*未闭合*/
                    else
                    {
                        sTime[clunum]++;
                        /*长时间等待直至产生回检故障*/
                        if(sTime[clunum] >= 100)
                        {
                            /*记录启动失败*/
                            TacSetGStartFailReason(clunum, eStartFailReason_SwtchExp);

                            /*记录正极接触器闭合失败*/
                            TacSetGSwhOnCtrlFailFlag(clunum, eGroupSwhPK1_1Mai);

                            /*切换到停机状态(由启动失败故障导致切换为故障状态)*/
                            /*TacSetGroupWorkStateAPI(eWORK_STOP);*/
                        }
                    }
                }
                else
                {
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK3_2Mai, ePSTATE_ON);
                    /*主接触器闭合*/
                    if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK3_2Mai))
                    {
                        sTime[clunum] = 0;

                        /*充电启动(主->辅助)*/
                        if(eCTRL_CHGSTART == sGroupWorkCtrlCmd[clunum])
                        {
                            /*返回控制辅助接触器*/
                            sGroupStartStep[clunum] = 2;
                        }
                        /*放电启动(辅助->主)*/
                        else if(eCTRL_DHGSTART == sGroupWorkCtrlCmd[clunum])
                        {
                            /*进入启动结果判断*/
                            sGroupStartStep[clunum] = 9;
                        }
                        /*正常启动(辅助->预充->主)*/
                        else
                        {
                            /*进入预充断开控制*/
                            sGroupStartStep[clunum] = 9;
                        }
                    }
                    /*未闭合*/
                    else
                    {
                        sTime[clunum]++;
                        /*长时间等待直至产生回检故障*/
                        if(sTime[clunum] >= 100)
                        {
                            /*记录启动失败*/
                            TacSetGStartFailReason(clunum, eStartFailReason_SwtchExp);

                            /*记录正极接触器闭合失败*/
                            TacSetGSwhOnCtrlFailFlag(clunum, eGroupSwhPK3_2Mai);

                            /*切换到停机状态(由启动失败故障导致切换为故障状态)*/
                            /*TacSetGroupWorkStateAPI(eWORK_STOP);*/
                        }
                    }
                }
                break;

            /*断开预充(延迟1s)*/
            case 8:
                sTime[clunum]++;
                if(sTime[clunum] > 10)
                {
                    //TacCtrlGroupSwhStateAPI(eGroupSwh_Pre, ePSTATE_OFF);
                    sTime[clunum] = 0;
                    sGroupStartStep[clunum] = 9;
                }
                break;

            /*启动完成*/
            case 9:
                sGroupWorkCtrlCmd[clunum] = 0xff;
                if(clunum == 0)
                {
                    /*负极/主接触器闭合*/
                    if((ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK2_1Neg))
                        && (ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK1_1Mai)))
                    {
                        /*进入运行状态*/
                        sTime[clunum] = 0;
                        sGroupStartStep[clunum] = 0xff;
                        TacSetGroupWorkStateAPI(clunum, eWORK_RUN);
                    }
                    /*未闭合*/
                    else
                    {
                        sTime[clunum]++;
                        /*长时间等待直至产生回检故障*/
                        if(sTime[clunum] >= 100)
                        {
                            /*记录启动失败*/
                            TacSetGStartFailReason(clunum, eStartFailReason_SwtchExp);

                            if(ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK2_1Neg))
                            {
                                /*记录负极接触器闭合失败*/
                                TacSetGSwhOnCtrlFailFlag(clunum, eGroupSwhPK2_1Neg);
                            }
                            //else if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwh_Pre))
                            //{
                                /*记录预充接触器断开失败*/
                            //    TacSetGSwhOffCtrlFailFlag(eGroupSwh_Pre);
                            //}
                            //else
                            //{
                                /*记录正极接触器闭合失败*/
                            //    TacSetGSwhOnCtrlFailFlag(eGroupSwh_Mai);
                            //}

                            /*切换到停机状态(由启动失败故障导致切换为故障状态)*/
                            /*TacSetGroupWorkStateAPI(eWORK_STOP);*/
                        }
                    }
                }
                else
                {
                    /*负极/主接触器闭合*/
                    if((ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK4_2Neg))
                        && (ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK3_2Mai)))
                    {
                        /*进入运行状态*/
                        sTime[clunum] = 0;
                        sGroupStartStep[clunum] = 0xff;
                        TacSetGroupWorkStateAPI(clunum, eWORK_RUN);
                    }
                    /*未闭合*/
                    else
                    {
                        sTime[clunum]++;
                        /*长时间等待直至产生回检故障*/
                        if(sTime[clunum] >= 100)
                        {
                            /*记录启动失败*/
                            TacSetGStartFailReason(clunum, eStartFailReason_SwtchExp);
                            if(ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK4_2Neg))
                            {
                                /*记录负极接触器闭合失败*/
                                TacSetGSwhOnCtrlFailFlag(clunum, eGroupSwhPK4_2Neg);
                            }
                            //else if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwh_Pre))
                            //{
                                /*记录预充接触器断开失败*/
                            //    TacSetGSwhOffCtrlFailFlag(eGroupSwh_Pre);
                            //}
                            //else
                            //{
                                /*记录正极接触器闭合失败*/
                            //    TacSetGSwhOnCtrlFailFlag(eGroupSwh_Mai);
                            //}

                            /*切换到停机状态(由启动失败故障导致切换为故障状态)*/
                            /*TacSetGroupWorkStateAPI(eWORK_STOP);*/
                        }
                    }
                }
                break;

            default:
                break;
        }
    }
}
/*=================================================================================================
** @Name      : void TacGroupNorStopCtrlTactic(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 电池组正常停机控制策略
** @The notes : 高压下电策略
**===============================================================================================*/
void TacGroupNorStopCtrlTactic(u8 clunum)
{
    static u8 sTime[2] = {0};
    if(clunum >= 2)
    {
        return;
    }

    switch(sGroupStopStep[clunum])
    {
        /*数据复位*/
        case 0:
            sTime[clunum] = 0;
            sGroupStopStep[clunum] = 1;
            //TacCtrlGroupSwhStateAPI(eGroupSwh_Pre, ePSTATE_OFF);            /*断开预充*/
            break;

        /*检测安全电流*/
        case 1:
            if((1 == TacCheckCurrSafeStateHook(clunum))                           /*主接触器电流安全*/
                || (eCTRL_FORCSTOP == sGroupWorkCtrlCmd[clunum]))                   /*强制停机*/
            {
                /*根据电流方向判定先断哪个接触器*/
                if(eCURR_CHG == GetGChgDhgStateAPI(clunum))                       /*正在充电*/
                {
                    sGroupStopStep[clunum] = 4;                                     /*先断开负极辅助接触器*/
                }
                else                                                        /*正常停机或放电停机*/
                {
                    sGroupStopStep[clunum] = 2;                                     /*先正极主接触器*/
                }

                sTime[clunum] = 0;
            }
            else                                                            /*接触器电流过大不能直接切断*/
            {
                sTime[clunum]++;

                if(sTime[clunum] >= 100)                                            /*等待PCS停止充放电超时*/
                {
                    sTime[clunum] = 0;
                    sGroupStopStep[clunum] = 0xff;
                    TacSetGStopFailReason(clunum, eStopFailReason_CurrFail);
                    TacSetGroupWorkStateAPI(clunum, eWORK_ERR);                     /*切换到故障状态*/
                }
            }
            break;

        /*断开主接触器*/
        case 2:
            if(clunum == 0)
            {
                TacCtrlGroupSwhStateAPI(eGroupSwhPK1_1Mai, ePSTATE_OFF);

                /*主接触器断开*/
                if(ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK1_1Mai))
                {
                    sTime[clunum] = 0;

                    /*进入接触器断开状态检测*/
                    sGroupStopStep[clunum] = 3;
                }
                /*未断开*/
                else
                {
                    sTime[clunum]++;

                    if(sTime[clunum] >= 20)
                    {
                        TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK1_1Mai);
                        TacSetGStopFailReason(clunum, eStopFailReason_SwtchExp);

                        /*切换到停机状态(由启动失败故障导致切换为故障状态)*/
                        /*TacSetGroupWorkStateAPI(eWORK_ERR);*/
                    }
                }
            }
            else
            {
                TacCtrlGroupSwhStateAPI(eGroupSwhPK3_2Mai, ePSTATE_OFF);

                /*主接触器断开*/
                if(ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK3_2Mai))
                {
                    sTime[clunum] = 0;

                    /*进入接触器断开状态检测*/
                    sGroupStopStep[clunum] = 3;
                }
                /*未断开*/
                else
                {
                    sTime[clunum]++;

                    if(sTime[clunum] >= 20)
                    {
                        TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK3_2Mai);
                        TacSetGStopFailReason(clunum, eStopFailReason_SwtchExp);

                        /*切换到停机状态(由启动失败故障导致切换为故障状态)*/
                        /*TacSetGroupWorkStateAPI(eWORK_ERR);*/
                    }
                }
            }

            break;

        /*检测主接触器是否断开*/
        case 3:
            sTime[clunum]++;
            if(((1 == TacCheckCurrCutStateHook(clunum))                           /*主接触器断开*/
                && (1 == TacGetGNegCirReadyHook(clunum)))                         /*负极线路就绪(GND接外侧采预充电压,负极接触器闭合正常)*/
                || (sTime[clunum] >= 50))                                           /*控制断开超时*/
            {
                if(sTime[clunum] >= 50)
                {
                    if(0 == TacCheckCurrCutStateHook(clunum))                     /*主接触器未断开*/
                    {
                        if(clunum == 0)
                        {
                            TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK1_1Mai);           /*记录主接触器控制断开失败*/
                        }
                        else
                        {
                            TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK3_2Mai);           /*记录主接触器控制断开失败*/
                        }
                    	
                    }

                    if(0 == TacGetGNegCirReadyHook(clunum))                       /*负极线路异常(采集总压失效)*/
                    {
                        ;                                                   /*由接触器检测任务产生负极接触器控制闭合失败故障*/
                    }
                }
                if(clunum == 0)
                {
                    if(ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK2_1Neg))/*负极接触器已断开(负极->主)*/
                    {
                        sGroupStopStep[clunum] = 6;                                     /*进入停机结果检测*/
                    }
                    else                                                        /*负极接触器未断开(主->负极)*/
                    {
                        sGroupStopStep[clunum] = 4;                                     /*进入负极接触器控制*/
                    }
                }
                else
                {
                    if(ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK4_2Neg))/*负极接触器已断开(负极->主)*/
                    {
                        sGroupStopStep[clunum] = 6;                                     /*进入停机结果检测*/
                    }
                    else                                                        /*负极接触器未断开(主->负极)*/
                    {
                        sGroupStopStep[clunum] = 4;                                     /*进入负极接触器控制*/
                    }
                }

                sTime[clunum] = 0;
            }
            break;

        /*断开负极接触器*/
        case 4:
            if(clunum == 0)
            {
                TacCtrlGroupSwhStateAPI(eGroupSwhPK2_1Neg, ePSTATE_OFF);

                /*负极接触器断开*/
                if(ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK2_1Neg))
                {
                    sTime[clunum] = 0;

                    /*进入接触器断开状态检测*/
                    sGroupStopStep[clunum] = 5;
                }
                /*未闭合*/
                else
                {
                    sTime[clunum]++;
                    /*长时间等待直至产生回检故障*/
                    if(sTime[clunum] >= 20)
                    {
                        TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK2_1Neg);
                        TacSetGStopFailReason(clunum, eStopFailReason_SwtchExp);

                        /*切换到停机状态(由启动失败故障导致切换为故障状态)*/
                        /*TacSetGroupWorkStateAPI(eWORK_ERR);*/
                    }
                }
            }
            else
            {
                TacCtrlGroupSwhStateAPI(eGroupSwhPK4_2Neg, ePSTATE_OFF);

                /*负极接触器断开*/
                if(ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK4_2Neg))
                {
                    sTime[clunum] = 0;

                    /*进入接触器断开状态检测*/
                    sGroupStopStep[clunum] = 5;
                }
                /*未闭合*/
                else
                {
                    sTime[clunum]++;
                    /*长时间等待直至产生回检故障*/
                    if(sTime[clunum] >= 20)
                    {
                        TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK4_2Neg);
                        TacSetGStopFailReason(clunum, eStopFailReason_SwtchExp);

                        /*切换到停机状态(由启动失败故障导致切换为故障状态)*/
                        /*TacSetGroupWorkStateAPI(eWORK_ERR);*/
                    }
                }
            }

            break;

        /*检测负极接触器是否断开*/
        case 5:
            sTime[clunum]++;
            if(((1 == TacCheckCurrCutStateHook(clunum))                           /*负极接触器断开*/
                && (1 == TacGetGNegCirReadyHook(clunum)))                         /*负极线路就绪(GND接外侧采预充电压,接触器未粘连)*/
                || (sTime[clunum] >= 50))                                           /*等待断开超时*/
            {
                if(sTime[clunum] >= 50)
                {
                    if(clunum == 0)
                    {
                        TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK2_1Neg);               /*记录负极接触器控制断开失败*/
                    }
                    else
                    {
                        TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK4_2Neg);               /*记录负极接触器控制断开失败*/
                    }
                	
                }
                if(clunum == 0)
                {
                    if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK1_1Mai)) /*主接触器未断开(辅助->主)*/
                    {
                        sGroupStopStep[clunum] = 2;                                     /*进入主接触器控制*/
                    }
                    else                                                        /*主接触器已断开(主->辅助)*/
                    {
                        sGroupStopStep[clunum] = 6;                                     /*进入停机结果检测*/
                    }
                }
                else
                {
                    if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK3_2Mai)) /*主接触器未断开(辅助->主)*/
                    {
                        sGroupStopStep[clunum] = 2;                                     /*进入主接触器控制*/
                    }
                    else                                                        /*主接触器已断开(主->辅助)*/
                    {
                        sGroupStopStep[clunum] = 6;                                     /*进入停机结果检测*/
                    }
                }

                sTime[clunum] = 0;
            }
            break;

        /*停机完成检测*/
        case 6:
            if(clunum == 0)
            {
                /*辅助/主接触器/预充接触器断开*/
                if((ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK1_1Mai))
                    && (ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK2_1Neg)))
                {
                    /*清除参数*/
                    sTime[clunum] = 0;
                    sGroupStopStep[clunum] = 0xff;

                    /*关机命令*/
                    if(eCTRL_POWEROFF == sGroupWorkCtrlCmd[clunum])
                    {
                        /*进入关机状态*/
                        TacSetGroupWorkStateAPI(clunum, eWORK_OFF);
                    }
                    else
                    {
                        /*进入空闲状态*/
                        sGroupWorkCtrlCmd[clunum] = 0xff;
                        TacSetGroupWorkStateAPI(clunum, eWORK_IDLE);
                    }
                }
                /*有闭合*/
                else
                {
                    sTime[clunum]++;
                    /*长时间等待直至产生回检故障*/
                    if(sTime[clunum] >= 100)
                    {
                        /*记录停机失败*/
                        TacSetGStopFailReason(clunum, eStopFailReason_SwtchExp);

                        if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK2_1Neg))
                        {
                            /*记录负极接触器断开失败*/
                            TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK2_1Neg);
                        }
                        //else if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwh_Pre))
                        //{
                        //    /*记录预充接触器断开失败*/
                        //    TacSetGSwhOffCtrlFailFlag(eGroupSwh_Pre);
                        //}
                        else
                        {
                            /*记录正极接触器断开失败*/
                            TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK1_1Mai);
                        }

                        /*切换到停机状态(由启动失败故障导致切换为故障状态)*/
                        /*TacSetGroupWorkStateAPI(eWORK_ERR);*/
                    }
                }
            }
            else
            {
                /*辅助/主接触器/预充接触器断开*/
                if((ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK3_2Mai))
                    && (ePSTATE_OFF == TacGetGroupSwhBackStateAPI(eGroupSwhPK4_2Neg)))
                {
                    /*清除参数*/
                    sTime[clunum] = 0;
                    sGroupStopStep[clunum] = 0xff;

                    /*关机命令*/
                    if(eCTRL_POWEROFF == sGroupWorkCtrlCmd[clunum])
                    {
                        /*进入关机状态*/
                        TacSetGroupWorkStateAPI(clunum, eWORK_OFF);
                    }
                    else
                    {
                        /*进入空闲状态*/
                        sGroupWorkCtrlCmd[clunum] = 0xff;
                        TacSetGroupWorkStateAPI(clunum, eWORK_IDLE);
                    }
                }
                /*有闭合*/
                else
                {
                    sTime[clunum]++;
                    /*长时间等待直至产生回检故障*/
                    if(sTime[clunum] >= 100)
                    {
                        /*记录停机失败*/
                        TacSetGStopFailReason(clunum, eStopFailReason_SwtchExp);

                        if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK4_2Neg))
                        {
                            /*记录负极接触器断开失败*/
                            TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK4_2Neg);
                        }
                        //else if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwh_Pre))
                        //{
                        //    /*记录预充接触器断开失败*/
                        //    TacSetGSwhOffCtrlFailFlag(eGroupSwh_Pre);
                        //}
                        else
                        {
                            /*记录正极接触器断开失败*/
                            TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK3_2Mai);
                        }

                        /*切换到停机状态(由启动失败故障导致切换为故障状态)*/
                        /*TacSetGroupWorkStateAPI(eWORK_ERR);*/
                    }
                }
            }

            break;

        default:
            break;
    }
}
/*=================================================================================================
** @Name      : void TacGroupErrStopCtrlTactic(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 电池组故障停机控制策略
** @The notes : 高压下电策略
**===============================================================================================*/
void TacGroupErrStopCtrlTactic(u8 clunum)
{
    static u8 sTime[2] = {0};
    static u8 sFlag[2] = {0};

    if(clunum >= 2)
    {
        return;
    }

    /*无停机命令*/
    if((eCTRL_NORSTOP != sGroupWorkCtrlCmd[clunum])
        && (eCTRL_FORCSTOP != sGroupWorkCtrlCmd[clunum])
        && (eCTRL_EMERSTOP != sGroupWorkCtrlCmd[clunum])
        && (eCTRL_POWEROFF != sGroupWorkCtrlCmd[clunum]))
    {
        /*不控制(由故障保护执行)*/
    	sTime[clunum] = 0;
        sFlag[clunum] = 0;
        sGroupWorkCtrlCmd[clunum] = 0xff;
        return;
    }

    /*控制预充接触器断开*/
    //TacCtrlGroupSwhStateAPI(eGroupSwh_Pre, ePSTATE_OFF);

    if((1 == sFlag[clunum]) || (eCTRL_EMERSTOP == sGroupWorkCtrlCmd[clunum]))               /*已控制有接触器先断或需要急停*/
    {
    	TacCtrlGroupRunSwhAllOffAPI(clunum);                                      /*断开所有运行接触器*/
    }

    if((eCTRL_FORCSTOP == sGroupWorkCtrlCmd[clunum])                                /*强制停机*/
    	|| (1 == TacCheckCurrSafeStateHook(clunum))                               /*主接触器电流安全*/
		|| (sGroupStopFailFlag[clunum] > 0))                                        /*停机失败*/
	{
    	sTime[clunum] = 0;

        /*检测充放电状态确定断哪个接触器*/
        if(eCURR_CHG == GetGChgDhgStateAPI(clunum))                               /*正在充电*/
        {
            if(clunum == 0)
            {
                if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK2_1Neg))     /*负极接触器未断开*/
                {
                    sFlag[clunum] = 1;                                                  /*记录已控制*/
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK2_1Neg, ePSTATE_OFF);        /*先断开负极接触器*/
                }
                else                                                            /*负极接触器已断开*/
                {
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK2_1Neg, ePSTATE_OFF);        /*确保断开负极接触器*/
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK1_1Mai, ePSTATE_OFF);        /*再断开正极主接触器*/
                }
            }
            else
            {
                if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK4_2Neg))     /*负极接触器未断开*/
                {
                    sFlag[clunum] = 1;                                                  /*记录已控制*/
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK4_2Neg, ePSTATE_OFF);        /*先断开负极接触器*/
                }
                else                                                            /*负极接触器已断开*/
                {
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK4_2Neg, ePSTATE_OFF);        /*确保断开负极接触器*/
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK3_2Mai, ePSTATE_OFF);        /*再断开正极主接触器*/
                }
            }

        }
        else                                                                /*正常停机或放电停机*/
        {
            if(clunum == 0)
            {
                if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK1_1Mai))     /*正极主接触器未断开*/
                {
                    sFlag[clunum] = 1;                                                  /*记录已控制*/
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK1_1Mai, ePSTATE_OFF);        /*先断开正极主接触器*/
                }
                else                                                            /*正极主接触器已断开*/
                {
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK1_1Mai, ePSTATE_OFF);        /*确保断开正极主接触器*/
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK2_1Neg, ePSTATE_OFF);        /*再断开负极接触器*/
                }
            }
            else
            {
                if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK3_2Mai))     /*正极主接触器未断开*/
                {
                    sFlag[clunum] = 1;                                                  /*记录已控制*/
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK3_2Mai, ePSTATE_OFF);        /*先断开正极主接触器*/
                }
                else                                                            /*正极主接触器已断开*/
                {
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK3_2Mai, ePSTATE_OFF);        /*确保断开正极主接触器*/
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK4_2Neg, ePSTATE_OFF);        /*再断开负极接触器*/
                }
            }
        }
	}
	else                                                                    /*接触器电流过大不能直接切断*/
	{
		sTime[clunum]++;

		if(sTime[clunum] >= 30)                                                     /*等待PCS停止充放电超时*/
		{
			sGroupStopStep[clunum] = 0xff;
			TacSetGStopFailReason(clunum, eStopFailReason_CurrFail);                /*带载超时停机失败*/
		}
	}

    if(TRUE == TacGetGroupRunSwhAllOffStateAPI(clunum))                           /*运行接触器全断开*/
    {
    	sTime[clunum] = 0;
        sFlag[clunum] = 0;                                                          /*清除已控制*/

        if(eCTRL_POWEROFF == sGroupWorkCtrlCmd[clunum])                             /*关机命令*/
        {
            TacSetGroupWorkStateAPI(clunum, eWORK_OFF);                             /*进入关机状态*/
        }
        else
        {
            sGroupWorkCtrlCmd[clunum] = 0xff;
        }
    }
}

/*=================================================================================================
** @Name      : void TacGroupPwrOffCtrlTactic(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 电池组故障停机控制策略
** @The notes : 低压下电策略
**===============================================================================================*/
void TacGroupPwrOffCtrlTactic(u8 clunum)
{
    if(clunum >= 2)
    {
        return;
    }
	TacCtrlGroupRunSwhAllOffAPI(clunum);                                          /*断开所有运行接触器*/

    if(TRUE == TacGetGroupRunSwhAllOffStateAPI(clunum))                           /*运行接触器全断开*/
    {
        if((eCTRL_POWEROFF == sGroupWorkCtrlCmd[clunum]))                   /*关机命令*/
        {
			#if(0 == BMU_DC_PWR_TYPE)                                       /*自供电按钮上下电*/
        	//TacCtrlGroupSwhStateAPI(eGroupSwh_Pwr, ePSTATE_OFF);            /*断开电源接触器*/
			#else                                                           /*自供电手动上下电*/
			//TacCtrlGroupSwhStateAPI(eGroupSwh_Pwr, ePSTATE_ON);             /*闭合主控电源接触器电源(高电平弹开使断电)*/
			#endif
        }
    }

    if(eCTRL_POWERON == sGroupWorkCtrlCmd[clunum])                                  /*开机命令*/
    {
        TacSetGroupWorkStateAPI(clunum,eWORK_INIT);                                /*进入初始化状态*/
    }
}

/*=================================================================================================
** @Name      : void TacSetGStartFailReason(u8 clunum, e_StartFailReason reason)
** @Input     : clunum:簇编号 reason:失败原因[0,3]
** @Output    : void
** @Function  : 设置电池组启动失败原因
** @The notes : 设置启动失败标志
**===============================================================================================*/
void TacSetGStartFailReason(u8 clunum, e_StartFailReason reason)
{   
    if(clunum >= 2)
    {
        return;
    }
	/*原因有效*/
	if(reason < eStartFailReason_Fin)
	{
		BitSet(sGroupStartFailFlag[clunum], reason);
	}
}

void TestGetStartFailReason(void)
{
    printf("sGroupStartFailFlag[0] = %d", sGroupStartFailFlag[0]);
    printf("sGroupStartFailFlag[1] = %d", sGroupStartFailFlag[1]);
}
/*=================================================================================================
** @Name      : void TacSetGStopFailReason(u8 clunum, e_StopFailReason reason)
** @Input     : clunum:簇编号 reason:失败原因[0,3]
** @Output    : void
** @Function  : 设置电池组停机失败原因
** @The notes : 设置停机失败标志
**===============================================================================================*/
void TacSetGStopFailReason(u8 clunum, e_StopFailReason reason)
{
    if(clunum >= 2)
    {
        return;
    }
	/*原因有效*/
	if(reason < eStopFailReason_Fin)
	{
		BitSet(sGroupStopFailFlag[clunum], reason);
	}
}

/*=================================================================================================
** @Name      : void TacSetGEmergStopReason(u8 clunum, e_EmergStopReason reason)
** @Input     : clunum:簇编号 reason:失败原因[0,1]
** @Output    : void
** @Function  : 设置电池组紧急停机原因
** @The notes : 设置紧急停机标志
**===============================================================================================*/
void TacSetGEmergStopReason(u8 clunum, e_EmergStopReason reason)
{
    if(clunum >= 2)
    {
        return;
    }
	/*原因有效*/
	if(reason < eEmergStopReason_Fin)
	{
		BitSet(sEmergStopFlag[clunum], reason);
	}
}
/*=================================================================================================
** @Name      : void TacSetGSwhOnCtrlFailFlag(u8 clunum, e_GroupSwhSort swhSort)
** @Input     : clunum:簇编号 swhSort:接触器名称[0,11]
** @Output    : void
** @Function  : 电池组接触器控制闭合失败标志
** @The notes : 控制后回检电压电流
**===============================================================================================*/
void TacSetGSwhOnCtrlFailFlag(u8 clunum, e_GroupSwhSort swhSort)
{
    if(clunum >= 2)
    {
        return;
    }
	/*接触器号有效*/
	if(swhSort < eGroupSwh_Num)
	{
		BitSet(sGSwhOnCtrlFailFlag[clunum], swhSort);
	}
}
/*=================================================================================================
** @Name      : void TacSetGSwhOffCtrlFailFlag(u8 clunum, e_GroupSwhSort swhSort)
** @Input     : clunum:簇编号 swhSort:接触器名称[0,11]
** @Output    : void
** @Function  : 电池组接触器控制断开失败标志
** @The notes : 控制后回检电压电流
**===============================================================================================*/
void TacSetGSwhOffCtrlFailFlag(u8 clunum, e_GroupSwhSort swhSort)
{
    if(clunum >= 2)
    {
        return;
    }
	/*接触器号有效*/
	if(swhSort < eGroupSwh_Num)
	{
		BitSet(sGSwhOffCtrlFailFlag[clunum], swhSort);
	}
}
/*=================================================================================================
** @Name      : void TacClrGCtrlErrFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 清除电池组控制失败标志
**===============================================================================================*/
void TacClrGCtrlErrFlagAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return;
    }
	/*接触器控制失败*/
	sGSwhOnCtrlFailFlag[clunum] = 0;
	sGSwhOffCtrlFailFlag[clunum] = 0;

	/*启停控制失败*/
	sGroupStartFailFlag[clunum] = 0;
	sGroupStopFailFlag[clunum] = 0;

	/*取消急停按钮才清除急停故障*/
	if(0 == TacGetUserButtonEmergStateHook())
	{
		sEmergStopFlag[clunum] = 0;
	}
}

/*=================================================================================================
** @Name      : u8 TacGetGStartFailFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 启动失败标志(按bit位表示 bit0:系统故障 bit1:启动受限 bit2:外设异常 bit3:接触器异常 bit4:预充过温 bit5:预充未满失败 bit6:预充电流异常)
** @Function  : 获取电池组启动失败标志
** @The notes : 启动失败原因
**===============================================================================================*/
u8 TacGetGStartFailFlagAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
	return(sGroupStartFailFlag[clunum]);
}
/*=================================================================================================
** @Name      : u8 TacGetGStopFailFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 停机失败标志(按bit位表示 bit0:系统故障 bit1:等待安全电流失败 bit2:接触器异常 bit3:接触器断开失败)
** @Function  : 获取电池组停机失败标志
** @The notes : 停机失败原因
**===============================================================================================*/
u8 TacGetGStopFailFlagAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
	return(sGroupStopFailFlag[clunum]);
}
/*=================================================================================================
** @Name      : u8 TacGetGEmergStopFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 紧急停机标志(按bit位表示 bit0:内部软件急停 bit1:外部命令急停 bit2:按钮急停)
** @Function  : 获取电池组紧急停机标志
** @The notes : 紧急停机原因
**===============================================================================================*/
u8 TacGetGEmergStopFlagAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
	return(sEmergStopFlag[clunum]);
}

/*=================================================================================================
** @Name      : u8 TacGetGSwhCtrlFailFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 控制接触器闭合失败标志(按bit位表示接触器 bit0:主正断开控制 bit1:预充断开控制 bit2:主负断开控制;bit4:主正闭合控制 bit5:预充闭合控制 bit6:主负闭合控制)
** @Function  : 获取电池组控制接触器闭合失败标志
** @The notes : 高4位控制闭合失败的接触器,低4位控制断开失败的接触器
**===============================================================================================*/
u8 TacGetGSwhCtrlFailFlagAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
	return((sGSwhOnCtrlFailFlag[clunum] << 4) | (sGSwhOffCtrlFailFlag[clunum] & 0x0f));
}

/*=================================================================================================
** @Name      : u8 TacGetGSmogAlarmFlagAPI(void)
** @Input     : void
** @Output    : 烟感告警状态标志(0:正常 1:正在核实告警 2:已确认告警)
** @Function  : 获取烟感告警状态标志
** @The notes : 包括电源重启确认真实信号
**===============================================================================================*/
u8 TacGetGSmogAlarmFlagAPI(void)
{
	/*烟感电源控制使能(有烟感检测时)*/
	#if(1 == BMU_SMG_SWH_EN)
	return(sSmgAlarmState);
	#else
	return(0);
	#endif
}