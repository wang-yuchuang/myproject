/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : TacUser.h
** @Founder      : ZQD808
** @Date         : 2026.03.23
** @Function     : 电池组策略模块用户接口
** @Instructions : 
**===============================================================================================*/
#ifndef _TACUSER_H
#define _TACUSER_H

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "TacInterface.h"
/*=================================================================================================
** @Name      : void RcvGroupWorkStateCmdAPI(u8 clunum, u8 port)
** @Input     : clunum:簇编号 port:接收命令端口:[0,4]CAN口 [5,9]RS485口 [10,12]按钮或内部口
** @Output    : void
** @Function  : 电池组接收切换工作状态命令处理函数
** @The notes : 通信口或按钮命令
**===============================================================================================*/
void RcvGroupWorkStateCmdAPI(u8 clunum, u8 port);
/*=================================================================================================
** @Name      : void RcvGroupWorkStateCmdAPI(u8 clunum, u8 port)
** @Input     : clunum:簇编号 port:接收命令端口:[0,4]CAN口 [5,9]RS485口 [10,12]按钮或内部口
** @Output    : void
** @Function  : 电池组接收切换工作状态命令处理函数
** @The notes : 通信口或按钮命令
**===============================================================================================*/
void RcvGroupWorkStateCmdAPI(u8 clunum, u8 port);
/*=================================================================================================
** @Name      : void RcvGroupWorkModeCmdAPI(u8 clunum, u8 port)
** @Input     : clunum:簇编号 port:接收命令端口:[0,4]CAN口 [5,9]RS485口 [10,12]按钮或内部口
** @Output    : void
** @Function  : 电池组接收切换工作模式切换命令处理函数
** @The notes : 通信口或按钮或内部命令
**===============================================================================================*/
void RcvGroupWorkModeCmdAPI(u8 clunum, u8 port);
/*=================================================================================================
** @Name      : void RcvGroupWorkModeCmdAPI(u8 clunum, u8 port)
** @Input     : clunum:簇编号 port:接收命令端口:[0,4]CAN口 [5,9]RS485口 [10,12]按钮或内部口
** @Output    : void
** @Function  : 电池组接收切换工作模式切换命令处理函数
** @The notes : 通信口或按钮或内部命令
**===============================================================================================*/
void RcvGroupWorkModeCmdAPI(u8 clunum, u8 port);
/*=================================================================================================
** @Name      : void RcvGroupSysClrCmdAPI(u8 clunum, u8 port)
** @Input     : clunum:簇编号 port:接收命令端口:[0,4]CAN口 [5,9]RS485口 [10,12]按钮或内部口
** @Output    : void
** @Function  : 电池组接收切换工作状态命令处理函数
** @The notes : 通信口或按钮命令
**===============================================================================================*/
void RcvGroupSysClrCmdAPI(u8 clunum, u8 port);
/*=================================================================================================
** @Name      : void RcvGroupReqLeakCheckCmdAPI(u8 clunum, u8 port)
** @Input     : clunum:簇编号 port:接收命令端口:[0,4]CAN口 [5,9]RS485口
** @Output    : void
** @Function  : 电池组接收请求绝缘检测命令处理函数
** @The notes : 通信口
**===============================================================================================*/
void RcvGroupReqLeakCheckCmdAPI(u8 clunum);
/*=================================================================================================
** @Name      : void TacCtrlGroupSwhStateAPI(e_GroupSwhSort swhSort, e_GPIOState state)
** @Input     : swhSort:接触器名称[0,11] state:控制接触器状态 0:断开 1:闭合
** @Output    : void
** @Function  : 控制电池组相应接触器状态
** @The notes : 控制HS/LS/干接点
**===============================================================================================*/
void TacCtrlGroupSwhStateAPI(e_GroupSwhSort swhSort, e_GPIOState state);
/*=================================================================================================
** @Name      : void TacCtrlGroupRunSwhAllOffAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 控制电池组所有运行接触器均断开
** @The notes : 控制主正/预充/主负接触器
**===============================================================================================*/
void TacCtrlGroupRunSwhAllOffAPI(u8 clunum);
/*=================================================================================================
** @Name      : e_GPIOState TacGetGroupSwhCtrlStateAPI(e_GroupSwhSort swhSort)
** @Input     : swhSort:接触器名称[0,11]
** @Output    : 电池组相应接触器当前控制输出状态 0:断开 1:闭合
** @Function  : 获取电池组相应接触器当前控制输出状态
** @The notes : 控制信号
**===============================================================================================*/
e_GPIOState TacGetGroupSwhCtrlStateAPI(e_GroupSwhSort swhSort);
/*=================================================================================================
** @Name      : e_GPIOState TacGetGroupSwhBackStateAPI(e_GroupSwhSort swhSort)
** @Input     : swhSort:接触器名称[0,11]
** @Output    : 电池组相应接触器当前回检状态 0:断开 1:闭合
** @Function  : 获取电池组相应接触器当前回检状态
** @The notes : 回检信号(输出信号/板级回检信号/反馈输入信号)
**===============================================================================================*/
e_GPIOState TacGetGroupSwhBackStateAPI(e_GroupSwhSort swhSort);
/*=================================================================================================
** @Name      : u8 TacGetGroupRunSwhStateAPI(u8 clunum)
** @Input     : void
** @Output    : 电池组运行接触器状态[低4位:控制状态 高4位:回检状态](按bit位表示接触器 bit0:主正接触器控制 bit1:预充接触器控制 bit2:主负接触器控制;bit4:主正接触器回检 bit5:预充接触器回检 bit6:主负接触器回检)
** @Function  : 获取电池组运行接触器当前状态
** @The notes : 控制信号和回检信号
**===============================================================================================*/
u8 TacGetGroupRunSwhStateAPI(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGroupRunSwhAllOffStateAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组运行接触器均断开状态 FALSE:未全断开 TRUE:全断开
** @Function  : 获取电池组运行接触器均断开状态
** @The notes : 回检信号(输出信号/板级回检信号/反馈输入信号)
**===============================================================================================*/
u8 TacGetGroupRunSwhAllOffStateAPI(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGroupRunSwhAllOnStateAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组运行接触器正负极均闭合状态 FALSE:未全闭合 TRUE:全闭合
** @Function  : 获取电池组运行接触器正负极均闭合状态
** @The notes : 回检信号(输出信号/板级回检信号/反馈输入信号)
**===============================================================================================*/
u8 TacGetGroupRunSwhAllOnStateAPI(u8 clunum);
#endif