/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : BSPUserAPI.c
** @Founder      : ZQD808
** @Date         : 2025.12.13
** @Function     : BSP用户接口函数模块
** @Instructions : 用户调用
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "BSPConfig.h"
#include "ParaInterface.h"
#include "PublicInfo.h"
#include "CurrSample.h"
#include "Authority.h"
#include "CharactCalc.h"
#include "DiagFilter.h"
#include "EnerUser.h"
#include "StatisUser.h"
#include "GroupState.h"
#include "GroupMode.h"
#include "GroupProtect.h"
#include "SocUserAPI.h"
#include <cmsis_os2.h>
/*=============================================================================================
** @Name      : void BSPDisIRQUserInit(void)
** @Input     : void
** @Output    : void
** @Function  : 禁止中断后的用户初始化
** @The notes : 用户添加硬件和任务初始化等
**===========================================================================================*/
void BSPDisIRQUserInit(void)
{
    ParaStoreParaInfoInit();
    CurrSampleInit();
	//InsuAdhesMeasurInit();
	//CANLibAllInit_API();
	AuthorityInit();
	SysPublicInfoInit();
	GroupDataInfoInit();
	DiagErrFilterInit();
}
/*=============================================================================================
** @Name      : void BSPEnIRQUserInit(void)
** @Input     : void
** @Output    : void
** @Function  : 使能中断后的用户初始化
** @The notes : 用户添加功能初始化等
**===========================================================================================*/
void BSPEnIRQUserInit(void)
{
   	//OutCurrSampCorrInit();
	//HigVoltSampCorrInit();
	EnerCapCalcAllInit();
	GroupStatisAllInit();
	GroupStateInfoInit();
	GroupModeInfoInit();
	GroupProtectInit();
    //DiagGroupSosInit();
	SocSoeLibAllInit();
}
/*=============================================================================================
** @Name      : u32 BSPGetOSMillSecTimeAPI(void)
** @Input     : void
** @Output    : RTOS毫秒计时时间 1ms
** @Function  : 获取RTOS毫秒计时时间
** @The notes :
**===========================================================================================*/
u32 BSPGetOSMillSecTimeAPI(void)
{
	return(osKernelGetTickCount());
}


