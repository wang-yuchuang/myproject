/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : TaskMonitor.h
** @Founder      : ZQD808
** @Date         : 2026.05.07
** @Function     : Task monitoring types/macros/API
** @Instructions :
**===========================================================================================*/
#ifndef __TASKMONITOR_H_
#define __TASKMONITOR_H_

#include "BSPTypeDef.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"

typedef enum
{
    eTaskMon_Task2ms,
    eTaskMon_Task5ms,
    eTaskMon_Task10ms,
    eTaskMon_Task10msB,
    eTaskMon_Task50ms,
    eTaskMon_Task100ms,
    eTaskMon_Task200ms,
    eTaskMon_Task300ms,
    eTaskMon_Task400ms,
    eTaskMon_Task500ms,
    eTaskMon_Task1000ms,
    eTaskMon_Task2000ms,
    eTaskMon_MulPortManag,
    eTaskMon_LowAdcSamp,
    eTaskMon_Can1Recv,
    eTaskMon_Can1Send,
    eTaskMon_Can2Recv,
    eTaskMon_Can2Send,
    eTaskMon_IccMsgRcvTask,
    eTaskMon_UdsTester,
    eTaskMon_MAX
} e_TaskMonitorId;

typedef struct
{
    TaskHandle_t handle;
    const char *name;
    u32 stackSize;
    u32 stackPeak;
    u32 lastRuntimeCounter;
    u32 runtimeDeltaUs;
    u32 cpuPercent;
    u32 timeoutCount;
    u32 execTimeMin;
    u32 execTimeMax;
    u32 execTimeSum;
    u32 execTimeCount;
    eTaskState state;
} t_TaskMonitorEntry;

#ifdef CONFIG_TASK_MONITOR
#define MONITOR_REGISTER_TASK(handle, name_str, stack_sz)  TaskMonitorRegisterTaskAPI((handle), (name_str), (stack_sz))
#define MONITOR_RECORD_ITER(task_id, exec_time_ms)         TaskMonitorRecordIterAPI((task_id), (exec_time_ms))
#define MONITOR_STACK_UPDATE(task_id)                      TaskMonitorUpdateStackAPI(task_id)
#else
#define MONITOR_REGISTER_TASK(handle, name_str, stack_sz)  ((void)(handle))
#define MONITOR_RECORD_ITER(task_id, exec_time_ms)         ((void)0)
#define MONITOR_STACK_UPDATE(task_id)                      ((void)0)
#endif

void TaskMonitorInitAPI(void);
void TaskMonitorRegisterTaskAPI(TaskHandle_t handle, const char *name, u32 stackSize);
void TaskMonitorRecordIterAPI(u32 taskId, u32 execTimeMs);
void TaskMonitorUpdateStackAPI(u32 taskId);
void TaskMonitorThreadAPI(void *argument);

#endif
