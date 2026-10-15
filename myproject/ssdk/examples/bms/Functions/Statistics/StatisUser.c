/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : StatisUser.c
** @Founder      : ZQD808
** @Date         : 2026.03.10
** @Function     : 电池组统计模块用户函数
** @Instructions :
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "EnergyStatis.h"
#include "TimesStatis.h"
#include "DodRcdStatis.h"
#include "StatisUser.h"

/*=============================================================================================
** @Name      : void GroupStatisAllInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组统计模块总初始化
** @The notes :
**===========================================================================================*/
void GroupStatisAllInit(void)
{
    /*累计电能电量统计初始化*/
    GroupEnerCapStatisInit();

    /*电池组时间次数统计初始化*/
    GroupTimesStatisInit();

    /*电池组放电深度DOD统计记录初始化*/
    GroupDodStatisRcdInit();
}


