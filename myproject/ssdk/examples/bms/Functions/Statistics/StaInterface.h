/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : StaInterface.h
** @Founder      : SBK326
** @Date         : 2019.11.1
** @Function     : 统计模块用户接口
** @Instructions : CAN通信
**===============================================================================================*/
#ifndef _STAINTERFACE_H
#define _STAINTERFACE_H
#include "BSPTypeDef.h"
#include "TimesStatis.h"
/*=============================================================================================
** @Name      : u8 StaGetBattVStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组电压状态:0正常 1充满 2放空
** @Function  : 获取电池组电压状态
** @The notes :
**===========================================================================================*/
u8 StaGetBattVStateHook(u8 clunum);
/*=============================================================================================
** @Name      : u8 StaGetVHLimStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 高压截止状态:0正常 1高压充电截止
** @Function  : 获取高压截止状态
** @The notes : 单体电压高2级(充电截止)
**===========================================================================================*/
u8 StaGetVHLimStateHook(u8 clunum);
/*=============================================================================================
** @Name      : u8 StaGetVLLimStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 低压截止状态:0正常 1低压放电截止
** @Function  : 获取低压截止状态
** @The notes : 低压2级,限制放电
**===========================================================================================*/
u8 StaGetVLLimStateHook(u8 clunum);
#endif