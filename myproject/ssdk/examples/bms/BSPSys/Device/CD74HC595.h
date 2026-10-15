/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CD74HC595.h
** @Founder      : ZQD808
** @Date         : 2025.11.11
** @Function     : CD74HC595移位寄存器通信模块
** @Instructions : 根据PCB外围设备编写管理函数
**===========================================================================================*/
#ifndef __CD74HC595_H__
#define __CD74HC595_H__
#include "BSPTypeDef.h"
#include "DEVGPIO.h"
#include "GroupInfo.h"
/*=============================================================================================
** @Name      : int CD74HC595Init(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : CD74HC595初始化
** @The notes : 
**===========================================================================================*/
int CD74HC595Init(void);
/*=============================================================================================
** @Name      :void CD74HC595SendDataAsync(e_595SPI_CHANNEL Channel, u16 data)
** @Input     : Channel:595通道 data:两个595控制IO数据（按bit）
** @Output    : 无
** @Function  : CD74HC595异步发送（发后即忘，回调中自动RCLK锁存）
** @The notes : 两个595级联，非阻塞
**===========================================================================================*/
void CD74HC595SendDataAsync(e_595SPI_CHANNEL Channel, u16 data);
#endif