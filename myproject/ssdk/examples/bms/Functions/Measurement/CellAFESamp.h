/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CellAFESamp.h
** @Founder      : ZQD808
** @Date         : 2025.01.23
** @Function     : 单体电压采样模块
** @Instructions :
**===========================================================================================*/
#ifndef _CELLAFESAMP_H__
#define _CELLAFESAMP_H__
#include "BSPTypeDef.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
#include "GroupPara.h"
/*=============================================================================================
** @Name      : void WriteSampCellVoltHook( u8 CluNum,u8 ic_Num, u8 num, u16 volt)
** @Input     : CluNum:第几簇  ic_Num:第几个从控[0,31]  num:单体电池号 volt:单节电压值 1mv
** @Output    : void
** @Function  : 输出单节电压采样值
** @The notes : 用户将输出值放到单体寄存器保存
**===========================================================================================*/
void WriteSampCellVoltHook(u8 CluNum, u8 ic_Num, u8 num, u16 volt);
/*=============================================================================================
** @Name      : void WriteCellVSampFinFlagHook(u8 flag)
** @Input     : flag:完成标志 0未完成 1完成
** @Output    : void
** @Function  : 输出单节电压采样第一遍完成标志
** @The notes : 菊花链或CAN通信接收完成
**===========================================================================================*/
void WriteCellVSampFinFlagHook(u8 CluNum,u8 flag);
/*=============================================================================================
** @Name      :u8 GetCellVSampFinFlagAPI(u8 CluNum)
** @Input     : void
** @Output    : 完成标志 0未完成 1完成
** @Function  : 获取单节电压采样第一遍完成标志
** @The notes : 菊花链或CAN通信接收完成
**===========================================================================================*/
u8 GetCellVSampFinFlagAPI(u8 CluNum);
/*=============================================================================================
** @Name      : void WriteCellTSampFinFlagHook(u8 CluNum, u8 flag)
** @Input     : flag:完成标志 0未完成 1完成
** @Output    : void
** @Function  : 输出单节温度采样第一遍完成标志
** @The notes : 菊花链或CAN通信接收完成
**===========================================================================================*/
void WriteCellTSampFinFlagHook(u8 CluNum, u8 flag);
/*=============================================================================================
** @Name      : u8 GetCellTSampFinFlagAPI(u8 CluNum)
** @Input     : CluNum ：簇编号
** @Output    : 完成标志 0未完成 1完成
** @Function  : 获取单节温度采样第一遍完成标志
** @The notes : 菊花链或CAN通信接收完成
**===========================================================================================*/
u8 GetCellTSampFinFlagAPI(u8 CluNum);
/*=============================================================================================
** @Name      : void WriteSampCellTempHook(u8 ic_Num, u8 num, s16 temp)
** @Input     : ic_Num:第几个从控[0,31] num:单体温度序号[0,3] temp:单节温度值 0.1℃
** @Output    : void
** @Function  : 输出单体电池温度采样值
** @The notes : 用户将输出值放到单体寄存器保存
**===========================================================================================*/
void WriteSampCellTempHook(u8 CluNum, u8 ic_Num, u8 num, s16 temp);
/*=============================================================================================
** @Name      : void ClrSampCellVoltHook(u8 CluNum, u8 ic_Num)
** @Input     : CluNum:簇编号 ic_Num:第几个从控[0,31]
** @Output    : void
** @Function  : 清除指定从控AFE所有单体电压采样值为0
** @The notes : AFE单体电压采样失效或CAN通信中断超时时调用
**===========================================================================================*/
void ClrSampCellVoltHook(u8 CluNum, u8 ic_Num);
/*=============================================================================================
** @Name      : void ClrSampCellTempHook(u8 CluNum, u8 ic_Num)
** @Input     : CluNum:簇编号 ic_Num:第几个从控[0,31]
** @Output    : void
** @Function  : 清除指定从控AFE所有单体温度和均衡板温温度采样值为0
** @The notes : AFE单体温度采样失效或CAN通信中断超时时调用
**===========================================================================================*/
void ClrSampCellTempHook(u8 CluNum, u8 ic_Num);
/*=============================================================================================
** @Name      : void WriteSampCellResisHook(u8 CluNum, u8 ic_Num, u8 num, u16 resis)
** @Input     : CluNum:第几簇 ic_Num:第几个从控[0,31] num:单体电池号 resis:内阻值 1μΩ
** @Output    : void
** @Function  : 输出单节内阻采样值
** @The notes : 用户将输出值放到单体寄存器保存
**===========================================================================================*/
void WriteSampCellResisHook(u8 CluNum, u8 ic_Num, u8 num, u16 resis);
/*=============================================================================================
** @Name      : void WriteCellRSampFinFlagHook(u8 CluNum, u8 flag)
** @Input     : CluNum:第几簇 flag:完成标志 0未完成 1完成
** @Output    : void
** @Function  : 输出单节内阻采样第一遍完成标志
** @The notes : CAN通信接收完成
**===========================================================================================*/
void WriteCellRSampFinFlagHook(u8 CluNum, u8 flag);
/*=============================================================================================
** @Name      : u8 GetCellRSampFinFlagAPI(u8 CluNum)
** @Input     : CluNum:第几簇
** @Output    : 完成标志 0未完成 1完成
** @Function  : 获取单节内阻采样第一遍完成标志
** @The notes : CAN通信接收完成
**===========================================================================================*/
u8 GetCellRSampFinFlagAPI(u8 CluNum);
/*=============================================================================================
** @Name      : void WriteSampAfeStateHook(u8 CluNum, u8 ic_Num, u8 afeIndex, u8 afeState)
** @Input     : CluNum:第几簇 ic_Num:第几个从控[0,31] afeIndex:AFE序号[0,26] afeState:AFE状态值(4bit)
** @Output    : void
** @Function  : 输出AFE状态采样值
** @The notes : 每个AFE IC占1个u8,状态值定义见e_AfeState
**===========================================================================================*/
void WriteSampAfeStateHook(u8 CluNum, u8 ic_Num, u8 afeIndex, u8 afeState);
/*=============================================================================================
** @Name      : void ClrSampCellResisHook(u8 CluNum, u8 ic_Num)
** @Input     : CluNum:簇编号 ic_Num:第几个从控[0,31]
** @Output    : void
** @Function  : 清除指定从控AFE所有单体内阻采样值为0
** @The notes : CAN通信中断超时时调用
**===========================================================================================*/
void ClrSampCellResisHook(u8 CluNum, u8 ic_Num);
#endif