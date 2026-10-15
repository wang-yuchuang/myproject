/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : SN74HC165.h
** @Founder      : ZQD808
** @Date         : 2025.11.11
** @Function     : 输入移位寄存器管理函数模块
** @Instructions : 根据PCB外围设备编写管理函数
**===========================================================================================*/
#ifndef __SN74HC165_H__
#define __SN74HC165_H__
#include "BSPTypeDef.h"
#include "DEVGPIO.h"
#include "GroupInfo.h"
/*=============================================================================================
** @Name      : int SN74HC165Init(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : SN74HC165初始化
** @The notes : 此函数需在系统SPI初始化后调用
**===========================================================================================*/
int SN74HC165Init(void);
/*=============================================================================================
** @Name      :void SN74HC165GetDataAsync(void)
** @Input     : void
** @Output    : 无
** @Function  : SN74HC165异步读取（回调中自动解析数据）
** @The notes : 三个165级联，非阻塞
**===========================================================================================*/
void SN74HC165GetDataAsync(void);
#endif

