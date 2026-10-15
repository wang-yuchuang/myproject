/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : OSUserHook.h
** @Founder      : ZQD808
** @Date         : 2025.11.12
** @Function     : BSP定时任务服务模块
** @Instructions : 用户周期函数在这里调用
**===========================================================================================*/
#ifndef __OSUSERHOOK_H__
#define __OSUSERHOOK_H__

#include "BSPTypeDef.h"
#include "DEVCAN.h"

#define CONFIG_MULPORTMANAGE_THREAD_STACK_SIZE  1024
#define CONFIG_MULPORTMANAGE_THREAD_PRIORITY     26

#define CONFIG_LOWADCSAMP_THREAD_STACK_SIZE  1024
#define CONFIG_LOWADCSAMP_THREAD_PRIORITY     27

#define CONFIG_TASK2MS_THREAD_STACK_SIZE  1024
#define CONFIG_TASK2MS_THREAD_PRIORITY     25

#define CONFIG_TASK5MS_THREAD_STACK_SIZE  1024
#define CONFIG_TASK5MS_THREAD_PRIORITY     24

#define CONFIG_TASK10MS_THREAD_STACK_SIZE  4096
#define CONFIG_TASK10MS_THREAD_PRIORITY     23

#define CONFIG_TASK10MS_GROUPB_THREAD_STACK_SIZE  4096
#define CONFIG_TASK10MS_GROUPB_THREAD_PRIORITY     22

#define CONFIG_TASK50MS_THREAD_STACK_SIZE  2048
#define CONFIG_TASK50MS_THREAD_PRIORITY     21

#define CONFIG_TASK100MS_THREAD_STACK_SIZE  2048
#define CONFIG_TASK100MS_THREAD_PRIORITY     20

#define CONFIG_TASK200MS_THREAD_STACK_SIZE  2048
#define CONFIG_TASK200MS_THREAD_PRIORITY     19

#define CONFIG_TASK300MS_THREAD_STACK_SIZE  2048
#define CONFIG_TASK300MS_THREAD_PRIORITY     18

#define CONFIG_TASK400MS_THREAD_STACK_SIZE  2048
#define CONFIG_TASK400MS_THREAD_PRIORITY     17

#define CONFIG_TASK500MS_THREAD_STACK_SIZE  2048
#define CONFIG_TASK500MS_THREAD_PRIORITY     16

#define CONFIG_TASK1000MS_THREAD_STACK_SIZE  4096
#define CONFIG_TASK1000MS_THREAD_PRIORITY     15

#define CONFIG_TASK2000MS_THREAD_STACK_SIZE  2048
#define CONFIG_TASK2000MS_THREAD_PRIORITY     14

#define CONFIG_ASYNCLOG_THREAD_STACK_SIZE  1024
#define CONFIG_ASYNCLOG_THREAD_PRIORITY     5

#define CONFIG_UartSend_THREAD_STACK_SIZE  2048
#define CONFIG_UartSend_THREAD_PRIORITY     28

#define CONFIG_UartRecv_THREAD_STACK_SIZE  1024
#define CONFIG_UartRecv_THREAD_PRIORITY     24

#define CONFIG_CanRecv_THREAD_STACK_SIZE  2048
/* IPC/rpmsg 线程已提到 28(高于本任务), 故本值可提高以排空 CAN FIFO; 26 低于 IPC、高于 BMS 周期任务 */
#define CONFIG_CanRecv_THREAD_PRIORITY     26

#define CONFIG_CanSend_THREAD_STACK_SIZE  1024
#define CONFIG_CanSend_THREAD_PRIORITY     24

#define CONFIG_IccMsgRcv_THREAD_STACK_SIZE  4096
#define CONFIG_IccMsgRcv_THREAD_PRIORITY     24

#define CONFIG_UdsTester_THREAD_STACK_SIZE  2048
#define CONFIG_UdsTester_THREAD_PRIORITY     28


#define CONFIG_N2402TASK_THREAD_STACK_SIZE  4096
#define CONFIG_N2402TASK_THREAD_PRIORITY     26
#ifdef CONFIG_TASK_MONITOR
#define CONFIG_TASKMONITOR_THREAD_STACK_SIZE  2048
#define CONFIG_TASKMONITOR_THREAD_PRIORITY     28
#else
#define CONFIG_TASKMONITOR_THREAD_STACK_SIZE  0
#define CONFIG_TASKMONITOR_THREAD_PRIORITY    0
#endif

/*=================================================================================================
** @Task10ms sTime 槽位分配 — 12 槽 / 100ms 周期
** @Function  : 将 Task10ms 从 10 槽扩展到 12 槽，拆分 SOC/SOE/SOH 到独立槽位
**===============================================================================================*/
#define TASK10MS_SUBTICK_MAX        10
#define TASK10MS_GROUPB_SUBTICK_MAX        10
#define TASK10MS_SLOT_CAP_SOC        5
#define TASK10MS_SLOT_CAP_SOE        6
#define TASK10MS_SLOT_CAP_SOH        7
#define TASK10MS_SLOT_DISPLAY        8
#define TASK10MS_SLOT_PRO            9
#define TASK10MS_SLOT_BALANCE        10
#define TASK10MS_SLOT_SWDIAG         11
/*=============================================================================================
** @Name      : void UserTaskCreate(void)
** @Input     : void
** @Output    : void
** @Function  : 用户任务创建
** @The notes : 
**===========================================================================================*/
void UserTaskCreate(void);

/*=================================================================================================
** @Name      : void AsyncLogConsumerTask(void *argument)
** @Input     : argument: RTOS 线程参数
** @Output    : void
** @Function  : 异步日志消费者任务入口（见 DiagAsyncLog.h）
** @The notes : 低优先级（5）独立任务，排空 printf UART 队列
**===============================================================================================*/
void AsyncLogConsumerTask(void *argument);

#endif