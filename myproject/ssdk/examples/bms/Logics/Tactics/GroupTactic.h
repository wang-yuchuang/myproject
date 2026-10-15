/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : GroupTactic.h
** @Founder      : ZQD808
** @Date         : 2026.03.23
** @Function     : 电池组控制策略模块接口
** @Instructions : 
**===============================================================================================*/
#ifndef _GROUPTACTIC_H
#define _GROUPTACTIC_H

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "TacInterface.h"
extern u32 SInfy_Idletime[2];                            /*系统空闲时间*/

/*=================================================================================================
** @Name      : void TacSetGWorkStateCmdAPI(u8 clunum, u8 cmd)
** @Input     : clunum:簇编号 cmd:电池组工作状态切换控制命令[0,8]:0:正常停止 1:正常预充 2:正常启动 3:充电启动 4:放电启动 5:强制停机 6:紧急停机 7:下电关机 8:上电开机
** @Output    : void
** @Function  : 设置电池组工作状态切换控制命令
** @The notes : 接收到通信命令或按钮命令
**===============================================================================================*/
void TacSetGWorkStateCmdAPI(u8 clunum, u8 cmd);
/*=================================================================================================
** @Name      : void TacGPeriManagTacticTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组外围管控策略任务
** @The notes : 1s周期调用,控制显示灯、输出干接点和ACC激活保持信号
**===============================================================================================*/
void TacGPeriManagTacticTask(void);
/*=================================================================================================
** @Name      : void TacGroupStartCtrlTactic(u8 clunum)
** @Input     : void
** @Output    : void
** @Function  : 电池组启动控制策略
** @The notes : 高压上电策略
**===============================================================================================*/
void TacGroupStartCtrlTactic(u8 clunum);
/*=================================================================================================
** @Name      : void TacGroupNorStopCtrlTactic(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 电池组正常停机控制策略
** @The notes : 高压下电策略
**===============================================================================================*/
void TacGroupNorStopCtrlTactic(u8 clunum);
/*=================================================================================================
** @Name      : void TacGroupErrStopCtrlTactic(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 电池组故障停机控制策略
** @The notes : 高压下电策略
**===============================================================================================*/
void TacGroupErrStopCtrlTactic(u8 clunum);
/*=================================================================================================
** @Name      : void TacGroupPwrOffCtrlTactic(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 电池组故障停机控制策略
** @The notes : 低压下电策略
**===============================================================================================*/
void TacGroupPwrOffCtrlTactic(u8 clunum);
/*=================================================================================================
** @Name      : void TacSetGStartFailReason(u8 clunum, e_StartFailReason reason)
** @Input     : clunum:簇编号 reason:失败原因[0,3]
** @Output    : void
** @Function  : 设置电池组启动失败原因
** @The notes : 设置启动失败标志
**===============================================================================================*/
void TacSetGStartFailReason(u8 clunum, e_StartFailReason reason);
/*=================================================================================================
** @Name      : void TacSetGStopFailReason(u8 clunum, e_StopFailReason reason)
** @Input     : clunum:簇编号 reason:失败原因[0,3]
** @Output    : void
** @Function  : 设置电池组停机失败原因
** @The notes : 设置停机失败标志
**===============================================================================================*/
void TacSetGStopFailReason(u8 clunum, e_StopFailReason reason);
/*=================================================================================================
** @Name      : void TacSetGEmergStopReason(u8 clunum, e_EmergStopReason reason)
** @Input     : clunum:簇编号 reason:失败原因[0,1]
** @Output    : void
** @Function  : 设置电池组紧急停机原因
** @The notes : 设置紧急停机标志
**===============================================================================================*/
void TacSetGEmergStopReason(u8 clunum, e_EmergStopReason reason);
/*=================================================================================================
** @Name      : void TacSetGSwhOnCtrlFailFlag(u8 clunum, e_GroupSwhSort swhSort)
** @Input     : clunum:簇编号 swhSort:接触器名称[0,11]
** @Output    : void
** @Function  : 电池组接触器控制闭合失败标志
** @The notes : 控制后回检电压电流
**===============================================================================================*/
void TacSetGSwhOnCtrlFailFlag(u8 clunum, e_GroupSwhSort swhSort);
/*=================================================================================================
** @Name      : void TacSetGSwhOffCtrlFailFlag(u8 clunum, e_GroupSwhSort swhSort)
** @Input     : clunum:簇编号 swhSort:接触器名称[0,11]
** @Output    : void
** @Function  : 电池组接触器控制断开失败标志
** @The notes : 控制后回检电压电流
**===============================================================================================*/
void TacSetGSwhOffCtrlFailFlag(u8 clunum, e_GroupSwhSort swhSort);
/*=================================================================================================
** @Name      : void TacClrGCtrlErrFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 清除电池组控制失败标志
** @The notes :
**===============================================================================================*/
void TacClrGCtrlErrFlagAPI(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGStartFailFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 启动失败标志(按bit位表示 bit0:系统故障 bit1:启动受限 bit2:外设异常 bit3:接触器异常 bit4:预充过温 bit5:预充未满失败 bit6:预充电流异常)
** @Function  : 获取电池组启动失败标志
** @The notes : 启动失败原因
**===============================================================================================*/
u8 TacGetGStartFailFlagAPI(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGStopFailFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 停机失败标志(按bit位表示 bit0:系统故障 bit1:等待安全电流失败 bit2:接触器异常 bit3:接触器断开失败)
** @Function  : 获取电池组停机失败标志
** @The notes : 停机失败原因
**===============================================================================================*/
u8 TacGetGStopFailFlagAPI(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGEmergStopFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 紧急停机标志(按bit位表示 bit0:内部软件急停 bit1:外部命令急停 bit2:按钮急停)
** @Function  : 获取电池组紧急停机标志
** @The notes : 紧急停机原因
**===============================================================================================*/
u8 TacGetGEmergStopFlagAPI(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGSwhCtrlFailFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 控制接触器闭合失败标志(按bit位表示接触器 bit0:主正断开控制 bit1:预充断开控制 bit2:主负断开控制;bit4:主正闭合控制 bit5:预充闭合控制 bit6:主负闭合控制)
** @Function  : 获取电池组控制接触器闭合失败标志
** @The notes : 高4位控制闭合失败的接触器,低4位控制断开失败的接触器
**===============================================================================================*/
u8 TacGetGSwhCtrlFailFlagAPI(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGSmogAlarmFlagAPI(void)
** @Input     : void
** @Output    : 烟感告警状态标志(0:正常 1:正在核实告警 2:已确认告警)
** @Function  : 获取烟感告警状态标志
** @The notes : 包括电源重启确认真实信号
**===============================================================================================*/
u8 TacGetGSmogAlarmFlagAPI(void);
void TestGetStartFailReason(void);
#endif