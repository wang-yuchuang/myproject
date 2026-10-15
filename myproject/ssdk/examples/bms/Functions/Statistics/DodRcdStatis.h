/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : DodRcdStatis.h
** @Founder      : ZQD808
** @Date         : 2026.03.10
** @Function     : 放电深度DOD记录统计模块
** @Instructions :
**===========================================================================================*/

#ifndef _DODRCDSTATIS_H
#define _DODRCDSTATIS_H
#include "BSPTypeDef.h"
/*历史DOD记录容量改由GroupInfo.h的HISDOD_REC_NUM统一定义(V2环形缓冲)*/
/*=============================================================================================
** @Name      : void GroupDodRcdStatisInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组放电深度DOD统计记录初始化
** @The notes :
**===========================================================================================*/
void GroupDodStatisRcdInit(void);
/**=============================================================================================
** @Name      : void GroupDodStatisRcdTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组放电深度DOD统计记录任务
** @The notes : 1s
**===========================================================================================*/
void GroupDodStatisRcdTask(void);
/**=============================================================================================
** @Name      : void GroupClrDodStatisRcdAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 清除电池组放电深度DOD统计记录信息
** @The notes :
**===========================================================================================*/
void GroupClrDodStatisRcdAPI(u8 clunum);
#endif