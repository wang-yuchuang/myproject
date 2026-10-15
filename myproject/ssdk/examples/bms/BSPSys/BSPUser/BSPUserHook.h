/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : BSPUserHook.h
** @Founder      : ZQD808
** @Date         : 2025.12.04
** @Function     : BSP用户服务函数模块
** @Instructions : 用户完善
**===========================================================================================*/
#ifndef __BSPUSERHOOK_H__
#define __BSPUSERHOOK_H__
#include "BSPTypeDef.h"
#include "DEVCAN.h"
#include "DEVUART.h"


/*=============================================================================================
** @Name      : void CANRcvMsgHandle_Hook(e_CANNUM CANNum, can_frame_t *pWriteMsg)
** @Input     : CANNum:CAN通道 pWriteMsg:CAN报文信息结构指针
** @Output    : void
** @Function  : CAN接收信息处理接口
** @The notes : 该函数已经在读CAN接收缓冲区调用
**===========================================================================================*/
void CANRcvMsgHandle_Hook(e_CANNUM CANNum, can_frame_t *pWriteMsg);
#endif