/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : DiagMaker.h
** @Founder      : ZQD808
** @Date         : 2026.03.05
** @Function     : 电池组故障诊断模块启动滤波接口
** @Instructions : 
**===============================================================================================*/
#ifndef _DIAGMAKER_H
#define _DIAGMAKER_H

#include "BSPTypeDef.h"
#include "SocSoeSync.h"
#include <cmsis_os2.h>
/*=================================================================================================
** @Name      : e_DiagCheckState
** @Input     : 
** @Output    : 
** @Function  : 诊断检查状态机 — 将 5 个 DiagMaker 任务分布在 10 个状态中
**              每帧最多执行一个状态，最坏帧时间 <10ms
** @The notes : 
**===============================================================================================*/
typedef enum
{
    DIAG_SM_IDLE = 0,
    DIAG_SM_VOLT_LIM,
    DIAG_SM_TEMP_LIM,
    DIAG_SM_CURR_LIM,
    DIAG_SM_PORT_EXP,
    DIAG_SM_INFO_EXP,
    DIAG_SM_DONE
} e_DiagCheckState;
/*=================================================================================================
** @Name      : void DiagVoltLimFilMakerTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电压限制类故障诊断任务
** @The notes : 100ms周期
**===============================================================================================*/
void DiagVoltLimFilMakerTask(void);
/* 被 DiagCheckStateMachineTask 取代 */
/*=================================================================================================
** @Name      : void DiagTempLimFilMakerTask(void)
** @Input     : void
** @Output    : void
** @Function  : 温度限制类故障诊断任务
** @The notes : 100ms周期
**===============================================================================================*/
void DiagTempLimFilMakerTask(void);
/*=================================================================================================
** @Name      : void DiagCurrLimFilMakerTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电流限制类故障诊断任务
** @The notes : 100ms周期
**===============================================================================================*/
void DiagCurrLimFilMakerTask(void);
/*=================================================================================================
** @Name      : void DiagPortExpFilMakerTask(void)
** @Input     : void
** @Output    : void
** @Function  : 输入输出端口异常类故障诊断任务
** @The notes : 100ms周期
**===============================================================================================*/
void DiagPortExpFilMakerTask(void);
/*=================================================================================================
** @Name      : void DiagInfoExpFilMakerTask(void)
** @Input     : void
** @Output    : void
** @Function  : 数据信息异常类故障诊断任务
** @The notes : 100ms周期
**===============================================================================================*/
void DiagInfoExpFilMakerTask(void);
/*=================================================================================================
** @Name      : void DiagStateExpFilMakerTask(void)
** @Input     : void
** @Output    : void
** @Function  : 状态信息异常类故障诊断任务
** @The notes : 100ms周期
**===============================================================================================*/
void DiagStateExpFilMakerTask(void);
#endif