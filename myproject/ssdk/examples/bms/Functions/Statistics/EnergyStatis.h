/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : EnergyStatis.h
** @Founder      : ZSY342
** @Date         : 2019.10.30
** @Function     : 电池组电能量统计模块
** @Instructions :
**===========================================================================================*/
#ifndef _ENERGYSTATIS_H
#define _ENERGYSTATIS_H
#include "BSPTypeDef.h"

#define HISCAP_CHANG_TO_WRITE_EEP   1           //历史电量进行写EEP变化量(单位:1AH)
#define HISENER_CHANG_TO_WRITE_EEP  5           //历史电能进行写EEP变化量(单位:0.1KWH)
/*=============================================================================================
** @Name      : void GroupEnerCapStatisInit(void)
** @Input     : void
** @Output    : void
** @Function  : 累计充放电电能电量统计初始化
** @The notes :
**===========================================================================================*/
void GroupEnerCapStatisInit(void);
/*=============================================================================================
** @Name      : void GroupCycleCapStatisTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组单次循环充放电电量统计任务
** @The notes : 400ms
**===========================================================================================*/
void GroupCycleCapStatisTask(void);
/*=============================================================================================
** @Name      : void GroupHisCapStatisTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组累计历史充放电电量统计任务
** @The notes : 400ms
**===========================================================================================*/
void GroupHisCapStatisTask(void);
/*=============================================================================================
** @Name      : void GroupHisEnerStatisTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组累计历史充放电电能统计任务
** @The notes : 400ms
**===========================================================================================*/
void GroupHisEnerStatisTask(void);
/*=============================================================================================
** @Name      : void ClrGroupEnerCapStatisInfo(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 清除充放电电能电量统计值
** @The notes : 单次/累计/电能/电量
**===========================================================================================*/
void ClrGroupEnerCapStatisInfo(u8 clunum);
#endif