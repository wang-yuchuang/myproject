/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : AuxDevMsg.c
** @Founder      : ZQD808
** @Date         : 2026.01.01
** @Function     : 电池组与辅助设备通信CAN报文处理模块
** @Instructions : 
**===============================================================================================*/
#ifndef _AUXDEVMSG_H__
#define _AUXDEVMSG_H__
#include "BSPTypeDef.h"

#define IMEU3_SYSCTRL_MAX_INDEX   (14)

#define IMEU3_PWMCTRL_MIN_INDEX    (0)
#define IMEU3_PWMCTRL_MAX_INDEX    (3)

#define IMEU3_IOCTRL_MIN_INDEX     (4)
#define IMEU3_IOCTRL_MAX_INDEX     (6)

#define IMEU3_MOTORCTRL_MIN_INDEX  (7)
#define IMEU3_MOTORCTRL_MAX_INDEX  (8)

#define IMEU3_OCCTRL_MIN_INDEX    (9)
#define IMEU3_OCCTRL_MAX_INDEX   (11)

#define IMEU3_DECTRL_MIN_INDEX    (12)
#define IMEU3_DERCTRL_MAX_INDEX   (14)
/*=================================================================================================
** @Name      : void CanRcvImeu2MsgHandle_API(u32 ID, u8 *pdata, u8 DLC, u8 canNum)
** @Input     : ID:CANID *pdata:CAN数据域  DLC:数据域长度 canNum:CAN通道号
** @Output    : void
** @Function  : CAN通信IMEU3接收处理任务
** @The notes : 用户放在CAN接收处理HOOK接口调用
**===============================================================================================*/
void CanRcvImeu3MsgHandle_API(u32 ID, u8 *pdata, u8 DLC, u8 canNum);
/*=================================================================================================
** @Name      : u32 CanBuildRupAuxMsgID(u8 func, u8 index, u8 dadrr, u8 saddr)
** @Input     : func:功能码 index:主索引 dadrr:目标地址 saddr:源地址
** @Output    : CAN报文ID
** @Function  : 用户创建上传CAN报文ID
** @The notes : 电池组内部下行报文
**===============================================================================================*/
u32 CanBuildRupAuxMsgID(u8 func, u8 index, u8 dadrr, u8 saddr);
/*=================================================================================================
** @Name      : void SendCanImeu3PwmCtrlMsg(u8 addr,u8 index, u8 data)
** @Input     : addr:IMEU3通信地址，index：主索引  data：数据
** @Output    : void
** @Function  : 发送IMEU3PWM控制信息CAN报文
** @The notes : 主索引86
**===============================================================================================*/
void SendCanImeu3PwmCtrlMsg(u8 addr,u8 index, u8 data);
/*=================================================================================================
** @Name      : void SendCanImeu3DoCtrlMsg(u8 addr, u8 ctrlnum, u8 index, u8 data)
** @Input     : addr:IMEU3通信地址，index：DO控制开始序号  data：数据
** @Output    : void
** @Function  : 发送IMEU3PWM控制信息CAN报文
** @The notes : 主索引86
**===============================================================================================*/
void SendCanImeu3DoCtrlMsg(u8 addr, u8 ctrlnum, u8 index, u8 data);
/*=================================================================================================
** @Name      : void SendCanImeu3MotorCtrlMsg(u8 addr, u8 index, u8 data)
** @Input     : addr:IMEU3通信地址，index：主索引  data：数据
** @Output    : void
** @Function  : 发送IMEU3PWM控制信息CAN报文
** @The notes : 主索引86
**===============================================================================================*/
void SendCanImeu3MotorCtrlMsg(u8 addr, u8 index, u8 data);
/*=================================================================================================
** @Name      : void SendCanImeu3OCCtrlMsg(u8 addr, u8 index, u8 data)
** @Input     : addr:IMEU3通信地址，index：主索引  data：数据
** @Output    : void
** @Function  : 发送IMEU3PWM控制信息CAN报文
** @The notes : 主索引86
**===============================================================================================*/
void SendCanImeu3OCCtrlMsg(u8 addr, u8 index, u8 data);
/*=================================================================================================
** @Name      : void SendCanImeu3DeCtrlMsg(u8 addr, u8 openhum, u8 closehum, u8 mode)
** @Input     : addr:IMEU3通信地址，openhum:开启除湿湿度 closehum: 关闭除湿湿度 mode:工作模式
** @Output    : void
** @Function  : 发送IMEU3除湿机控制信息CAN报文
** @The notes : 主索引86
**===============================================================================================*/
void SendCanImeu3DeCtrlMsg(u8 addr, u8 openhum, u8 closehum, u8 mode);

/*=================================================================================================
** @Name      : CanRcvImiu4MsgHandle_API(u32 ID, u8 *pdata, u8 DLC, u8 canNum)
** @Input     : ID:CANID *pdata:CAN数据域  DLC:数据域长度 canNum:CAN通道号
** @Output    : void
** @Function  : CAN通信IMIU4接收处理任务
** @The notes : 用户放在CAN接收处理HOOK接口调用
**===============================================================================================*/
void CanRcvImiu4MsgHandle_API(u32 ID, u8 *pdata, u8 DLC, u8 canNum);
/*=================================================================================================
** @Name      : void SendCanImeu3DeCtrlMsg(u8 addr, u8 openhum, u8 closehum, u8 mode)
** @Input     : addr:IMIU4通信地址，readynum:继电器选择  state:继电器状态
** @Output    : void
** @Function  : 发送IMEU3除湿机控制信息CAN报文
** @The notes : 主索引86
**===============================================================================================*/
void SendCanImiu4ReadyCtrlMsg(u8 addr, u8 readynum, u8 state);
/*=================================================================================================
** @Name      : void SendCanImiu4InsuCtrlMsg(u8 addr, u8 cmd)
** @Input     : addr:IMIU4通信地址，cmd:
** @Output    : void
** @Function  : 发送IMEU3除湿机控制信息CAN报文
** @The notes : 主索引86
**===============================================================================================*/
void SendCanImiu4InsuCtrlMsg(u8 addr, u8 cmd);
#endif

