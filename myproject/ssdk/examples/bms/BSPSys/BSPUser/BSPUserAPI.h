/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : BSPUserAPI.h
** @Founder      : ZQD808
** @Date         : 2025.12.13
** @Function     : 用户接口函数模块
** @Instructions :
**===========================================================================================*/
#ifndef _BSPUSERAPI_H_
#define _BSPUSERAPI_H_

#include "BSPTypeDef.h"
#include "DEVGPIO.h"
#include "DEVADC.h"
#include "M2514.h"
#include "GD25Q32.h"
#include "DEVUART.h"
#include "DEVCAN.h"
#include "CD74HC595.h"
#include "SN74HC165.h"
#include "ParaInterface.h"

/*=============================================================================================
** @Name      : void BSPDisIRQUserInit(void)
** @Input     : void
** @Output    : void
** @Function  : 禁止中断后的用户初始化
** @The notes : 用户添加硬件和任务初始化等
**===========================================================================================*/
void BSPDisIRQUserInit(void);
/*=============================================================================================
** @Name      : void BSPEnIRQUserInit(void)
** @Input     : void
** @Output    : void
** @Function  : 使能中断后的用户初始化
** @The notes : 用户添加功能初始化等
**===========================================================================================*/
void BSPEnIRQUserInit(void);
/*=============================================================================================
** @Name      : u32 BSPGetOSMillSecTimeAPI(void)
** @Input     : void
** @Output    : RTOS毫秒计时时间 1ms
** @Function  : 获取RTOS毫秒计时时间
** @The notes :
**===========================================================================================*/
u32 BSPGetOSMillSecTimeAPI(void);
#endif