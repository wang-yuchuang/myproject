/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : PortCheck.h
** @Founder      : ZSY342
** @Date         : 2019.10.30
** @Function     : 绝缘检测模块
** @Instructions :
**===========================================================================================*/
#ifndef FUNTION_MEASUREMENT_PORTCHECK_H_
#define FUNTION_MEASUREMENT_PORTCHECK_H_
#include "BSPTypeDef.h"

/*=============================================================================================
** @Name      : void PortStateCheckTask(void)
** @Input     : void
** @Output    : void
** @Function  : 实时更新IO状态到相应寄存器
** @The notes : 10ms调用,后续补充
**===========================================================================================*/
void PortStateCheckTask(void);
/*=============================================================================================
** @Name      : u8 GetGEmergIoButtonStateAPI(void)
** @Input     : void
** @Output    : 急停按钮状态:0正常 1按下（急停）
** @Function  : 获取急停按钮状态
** @The notes : 用户调用
**===========================================================================================*/
u8 GetGEmergIoButtonStateAPI(void);
/*=============================================================================================
** @Name      : u8 GetGSmogIoWarStateAPI(void)
** @Input     : void
** @Output    : 烟感状态:0正常 1报警
** @Function  : 获取烟感报警状态,反馈信号，常态为闭合(高电平)
** @The notes : 用户调用
**===========================================================================================*/
u8 GetGSmogIoWarStateAPI(void);
#endif