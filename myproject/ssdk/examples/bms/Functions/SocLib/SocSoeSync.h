/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : SocSoeSync.h
** @Founder      : ZQD808
** @Date         : 2026.03.13
** @Function     : SOC/SOE组间同步模块
** @Instructions : 多组并机时有效,确保组间SOC/SOE差距不大
**===========================================================================================*/
#ifndef _SOCSOESYNC_H
#define _SOCSOESYNC_H
#include "BSPTypeDef.h"
#include "CapInfoCalc.h"
/*=============================================================================================
** @Header file definition
**===========================================================================================*/

/*=============================================================================================
** @Macro definition
**===========================================================================================*/
#define G_SOCSOE_SYNC_T                 10                  /*并机组SOC同步修正周期(1s)*/
#define G_ALLOW_DIFF_VOLT               30                  /*并机组SOCSOE同步允许压差值(1mV)*/
#define G_ALLOW_DIFF_SOC                30                  /*并机组允许SOC不同差值(0.1%)*/
#define G_ALLOW_DIFF_SOE                30                  /*并机组允许SOE不同差值(0.1%)*/
/*=============================================================================================
** @Name      :  void SocSoeTogetherSyncTask(void)
** @Input     :  void
** @Output    :  void
** @Function  :  SOC/SOE相互同步修正
** @The notes :  500ms,确保SOE变化跟随SOC同步(SOE全程均不能大于SOC)
**===========================================================================================*/
void SocSoeTogetherSyncTask(void);


#endif