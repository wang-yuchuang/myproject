/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : DiagAsyncLog.h
** @Founder      : ZQD808
** @Date         : 2026.05.08
** @Function     : 异步日志队列模块 — 解耦 IccMsgRcvTask printf 到低优先级消费者任务
** @Instructions : 9600bps UART 排空 ~60s，队列 4 槽 + 溢出丢弃策略防止 Task10ms 饥饿
**===============================================================================================*/
#ifndef __DIAGASYNCLOG_H_
#define __DIAGASYNCLOG_H_

#include "BSPTypeDef.h"
#include <cmsis_os2.h>

/*=================================================================================================
** @队列配置常量 — 基于 Metis 分析：9600bps 下 475 字节 ~16.6k 字符需~16s 排空
**===============================================================================================*/
#define ASYNC_LOG_QUEUE_SIZE       4
#define ASYNC_LOG_CHARS_PER_BYTE   35
#define ASYNC_LOG_MAX_CHARS        16625
#define ASYNC_LOG_BAUD_RATE        9600
#define ASYNC_LOG_CONSUMER_PRI     5
#define ASYNC_LOG_CONSUMER_STACK   1024

/*=================================================================================================
** @通道 ID 定义 — 对应 IccMsgRcvTask 中 4 个数据通道
**===============================================================================================*/
#define ASYNC_LOG_CH_CELL           0
#define ASYNC_LOG_CH_CYCLE          1
#define ASYNC_LOG_CH_FAQ            2
#define ASYNC_LOG_CH_UPDATE         3

/*异步日志消息结构*/
typedef struct
{
    u8 channel;
    u16 len;
    u8 data[475];
} t_AsyncLogMsg;

/*溢出策略枚举*/
typedef enum
{
    eALOG_OVERFLOW_DROP_OLDEST = 0,
    eALOG_OVERFLOW_DROP_NEWEST = 1
} e_AsyncLogOverflow;

e_FunStatus AsyncLogInitAPI(void);
e_FunStatus AsyncLogSendAPI(u8 channel, u8 *data, u16 len);
u32 AsyncLogGetDropCountAPI(void);
void AsyncLogConsumerTask(void *argument);

#endif
