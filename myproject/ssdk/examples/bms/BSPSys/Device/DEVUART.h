/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : DEVUART.h
** @Founder      : ZQD808
** @Date         : 2025.12.01
** @Function     : RS485通讯管理函数模块
** @Instructions :
**===========================================================================================*/
#ifndef __DEVUART_H__
#define __DEVUART_H__

#include "BSPTypeDef.h"
#include "GroupInfo.h"
#include "BSPConfig.h"

#if(0)
//UART通道定义
typedef enum 
{
    eUART5 = 0,      // UART5
    eUART9 = 1,      // UART9
    
    eUARTNUM
}e_UARTNUM;
#define UART_QUEUE_SIZE        256     // 每个缓冲区大小
#define UART_QUEUE_COUNT       20      // 缓冲区数量
#define UART_MSG_DATA_LEN      255     // 单条消息最大数据长度(u8最大值)

#define UART_SAF_CLK        80000000
#define UART_SEC0_CLK       60000000
#define UART_SEC1_CLK       120000000

typedef struct {
    u8 length;          // 实际数据长度
    u8 data[UART_QUEUE_SIZE]; // 数据缓冲区
} uart_message_t;
/*=============================================================================================
** @Name      : u8 DEVUARTResetInit(e_UARTNUM UartNum,u32 baud)
** @Input     : UartNum:SCI通道号
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 串口复位重新初始化
** @The notes : 用户调用
**===========================================================================================*/
u8 DEVUARTResetInit(e_UARTNUM UartNum,u32 Baudrate);
/*=============================================================================================
** @Name      : u8 DEVUARTSendInit(void)
** @Input     : void
** @Output    : 0:失败 1：成功
** @Function  : 串口发送初始化
** @The notes : 创建消息队列
**===========================================================================================*/
u8 DEVUARTSendBufInit(void);
/*=============================================================================================
** @Name      : u8 DEVUARTSendDataToBuf(e_UARTNUM UartNum, u8 *sendbuf, u8 size,u8 Prio)
** @Input     : sciNum:SCI通道号 *sendbuf:发送的数据 size:数据长度 Prio:优先级：数字越大，优先级越高
** @Output    : TRUE:成功 FALSE:失败
** @Function  : RS485询问方式发送串口数据到缓冲区
** @The notes : 直接发送,用户调用
**===========================================================================================*/
u8 DEVUARTSendDataToBuf(e_UARTNUM UartNum, u8 *sendbuf, u8 size,u8 Prio);
/*=============================================================================================
** @Name      : vvoid UartSendTask(void *argument)
** @Input     : *argument:形参,任务函数参数需要
** @Output    : void
** @Function  : 串口发送采样任务
** @The notes : 
**===========================================================================================*/
void UartSendTask(void *argument);
/*=============================================================================================
** @Name      : void UartRecvTask(void *argument)
** @Input     : *argument:形参,任务函数参数需要
** @Output    : void
** @Function  : 串口接收任务
** @The notes : 
**===========================================================================================*/
void UartRecvTask(void *argument);
/*=============================================================================================
** @Name      : u8 DEVUARTSendDataOnAsk(e_UARTNUM UartNum, u8 *sendbuf, u8 size)
** @Input     : sciNum:SCI通道号 *sendbuf:发送的数据 size:数据长度
** @Output    : TRUE:成功 FALSE:失败
** @Function  : RS485询问方式发送串口数据
** @The notes : 直接发送,任务调用
**===========================================================================================*/
u8 DEVUARTSendDataOnAsk(e_UARTNUM UartNum, u8 *sendbuf, u8 size);
static void uart5_recv_callback(void *arg);
static void uart9_recv_callback(void *arg);
#endif
#endif
