/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : DEVWDT.h
** @Founder      : SYS
** @Date         : 2026.07.23
** @Function     : 看门狗驱动接口（内部Kunlun WDT + 外部GPIO WDT）
** @Instructions : DEVWDTAllInit()初始化，DEVWDTFeedTask()每50ms喂狗
**===========================================================================================*/
#ifndef __DEVWDT_H__
#define __DEVWDT_H__
#include "BSPTypeDef.h"

/*=============================================================================================
** @Name      : void DEVWDTAllInit(void)
** @Input     : void
** @Output    : void
** @Function  : 初始化内部Kunlun WDT和外部GPIO WDT，启动喂狗标志
** @The notes : 必须在DEVGPIOInit()之后调用
**===========================================================================================*/
void DEVWDTAllInit(void);
/*=============================================================================================
** @Name      : void DEVWDTFeedTask(void)
** @Input     : void
** @Output    : void
** @Function  : 统一喂狗入口，每50ms由Task50ms调用
** @The notes : sIntFeed管内部Kunlun WDT寄存器刷新，sExtFeed管外部GPIO WDI翻转
**===========================================================================================*/
void DEVWDTFeedTask(void);

#endif
