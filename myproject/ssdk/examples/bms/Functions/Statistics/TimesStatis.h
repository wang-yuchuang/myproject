/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : TimesStatis.h
** @Founder      : ZSY342
** @Date         : 2019.10.30
** @Function     : 时间与次数统计模块
** @Instructions :
**===========================================================================================*/
#ifndef _TIMESSTATIS_H
#define _TIMESSTATIS_H

#include "BSPTypeDef.h"


#define CALC_TIMES_BY_TIME      0       /*通过充放电时间计算充放电次数使能*/
#define CHGDHG_TIMES_ADD_TIME   600     /*充放电次数累计1次需要持续的时间(s)*/

#define HISERR_TIMES_WR_TIMES   1       /*累计故障数目写EEPROM记录需变化次数*/
/*=============================================================================================
** @Name      : void GroupTimesStatisInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组时间次数统计初始化
** @The notes :
**===========================================================================================*/
void GroupTimesStatisInit(void);
/*=============================================================================================
** @Name      : void GroupChgTimesStatisTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组充放电次数统计任务
** @The notes : 2s
**===========================================================================================*/
void GroupChgTimesStatisTask(void);
/*=============================================================================================
** @Name      : void GroupHisErrNumAddHandle(u8 clunum, u8 type)
** @Input     : clunum:簇编号 type:故障类型 0:告警 1:故障
** @Output    : void
** @Function  : 电池组故障告警累加处理
** @The notes : 确定是故障或告警后调用
**===========================================================================================*/
void GroupHisErrNumAddHandle(u8 clunum, u8 type);
/*=============================================================================================
** @Name      : void GroupAllowTimeStatisTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组可充放电时间统计任务
** @The notes : 2s
**===========================================================================================*/
void GroupAllowTimeStatisTask(void);
/*=============================================================================================
** @Name      : void GroupErrNumStatisTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组故障告警次数统计任务
** @The notes : 2s
**===========================================================================================*/
void GroupErrNumStatisTask(void);
/*=============================================================================================
** @Name      : void ClrGroupChgDhgTimesAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 清除电池组充放电次数
** @The notes :
**===========================================================================================*/
void ClrGroupChgDhgTimesAPI(u8 clunum);
/*=============================================================================================
** @Name      : void ClrGroupHisErrNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 清除电池组累计故障告警数目
** @The notes : 当月值和累计值
**===========================================================================================*/
void ClrGroupHisErrNumAPI(u8 clunum);

#endif