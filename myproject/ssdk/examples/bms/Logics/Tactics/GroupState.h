/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : GroupState.h
** @Founder      : ZQD808
** @Date         : 2026.03.20
** @Function     : 电池工作状态模块接口
** @Instructions : 
**===============================================================================================*/
#ifndef _GROUPSTATE_H
#define _GROUPSTATE_H

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "PublicInfo.h"
/*=================================================================================================
** @Name      : void GroupStateInfoInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组状态信息初始化
** @The notes : 保护状态/工作状态/充放电状态
**===============================================================================================*/
void GroupStateInfoInit(void);
/*=================================================================================================
** @Name      : void GroupProStateTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组保护状态检测任务
** @The notes : 100ms周期调用
**===============================================================================================*/
void GroupProStateTask(void);
/*=================================================================================================
** @Name      : void void GroupWorkStateTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组工作状态检测任务
** @The notes : 100ms周期调用
**===============================================================================================*/
void GroupWorkStateTask(void);
/*=================================================================================================
** @Name      : void void GroupChgDhgStateTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组充放电状态检测任务
** @The notes : 100ms周期调用
**===============================================================================================*/
void GroupChgDhgStateTask(void);
/*=================================================================================================
** @Name      : void GroupBattStateCheckTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组电池电压状态检测任务
** @The notes : 100ms周期调用
**===============================================================================================*/
void GroupBattStateCheckTask(void);
/*=================================================================================================
** @Name      : void TacSetGroupWorkStateAPI(u8 clunum, e_WorkState state)
** @Input     : clunum:簇编号 state:电池组工作状态[0,6]
** @Output    : void
** @Function  : 设置电池组工作状态
** @The notes :
**===============================================================================================*/
void TacSetGroupWorkStateAPI(u8 clunum, e_WorkState state);
/*=================================================================================================
** @Name      : void TacResetGroupBattStateAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组电池状态: 1充满 2放空
** @Function  : 复位电池组电池状态
** @The notes : 电池电压状态清零(0正常)
**===============================================================================================*/
void TacResetGroupBattStateAPI(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGroupBattStateAPI(void)
** @Input     : clunum:簇编号
** @Output    : 电池组电池状态:0正常 1充满 2放空
** @Function  : 获取电池组电池状态
** @The notes : 电压状态
**===============================================================================================*/
u8 TacGetGroupBattStateAPI(u8 clunum);
/*=================================================================================================
** @Name      : static void GroupInitStateHandle(u8 clunum)
** @Input     : clunum：簇编号
** @Output    : void
** @Function  : 电池组初始化工作状态处理函数
** @The notes : 工作状态检测100ms周期任务调用
**===============================================================================================*/
static void GroupInitStateHandle(u8 clunum);
/*=================================================================================================
** @Name      : static void GroupIdleStateHandle(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 电池组待命工作状态处理函数
** @The notes : 工作状态检测100ms周期任务调用
**===============================================================================================*/
static void GroupIdleStateHandle(u8 clunum);
/*=================================================================================================
** @Name      : static void GroupStartStateHandle(void)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 电池组启动工作状态处理函数
** @The notes : 工作状态检测100ms周期任务调用
**===============================================================================================*/
static void GroupStartStateHandle(u8 clunum);
/*=================================================================================================
** @Name      : static void GroupRunStateHandle(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 电池组运行工作状态处理函数
** @The notes : 工作状态检测100ms周期任务调用
**===============================================================================================*/
static void GroupRunStateHandle(u8 clunum);
/*=================================================================================================
** @Name      : static void GroupStopStateHandle(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组停机工作状态处理函数
** @The notes : 工作状态检测100ms周期任务调用
**===============================================================================================*/
static void GroupStopStateHandle(u8 clunum);
/*=================================================================================================
** @Name      : static void GroupErrStateHandle(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 电池组故障工作状态处理函数
** @The notes : 工作状态检测100ms周期任务调用
**===============================================================================================*/
static void GroupErrStateHandle(u8 clunum);
/*=================================================================================================
** @Name      : static void GroupOffStateHandle(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 电池组关机工作状态处理函数
** @The notes : 工作状态检测100ms周期任务调用
**===============================================================================================*/
static void GroupOffStateHandle(u8 clunum);
#endif