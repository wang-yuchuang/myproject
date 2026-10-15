/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : TaskMonitor.c
** @Founder      : ZQD808
** @Date         : 2026.05.07
** @Function     : Task monitoring implementation with registry, stats collection, and printf table output
** @Instructions :
**===========================================================================================*/
#include "TaskMonitor.h"
#include "OSUserHook.h"
#include "DiagFilter.h"
#include <stdio.h>
#include <string.h>

#ifdef CONFIG_TASK_MONITOR

/*==== Diagnostic: comment to disable ====*/
/* #define CONFIG_TASKMONITOR_DIAGNOSTIC */

#ifndef CONFIG_TASKMONITOR_PERIOD_MS
#define CONFIG_TASKMONITOR_PERIOD_MS  5000
#endif

static t_TaskMonitorEntry sMonitorEntries[eTaskMon_MAX];

static const char *StateNames[] = {"Running", "Ready", "Blocked", "Suspend", "Deleted", "Invalid"};

static const u32 sPeriodMs[eTaskMon_MAX] = {
    2,
    5,
    10,
    10,
    50,
    100,
    200,
    300,
    400,
    500,
    1000,
    2000,
    10,
    20,
    0,
    0,
    0,
    0,
    20
};

void TaskMonitorInitAPI(void)
{
    u32 i;

    memset(sMonitorEntries, 0, sizeof(sMonitorEntries));

    sMonitorEntries[eTaskMon_Task2ms].name = "Task2ms";
    sMonitorEntries[eTaskMon_Task2ms].stackSize = CONFIG_TASK2MS_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_Task5ms].name = "Task5ms";
    sMonitorEntries[eTaskMon_Task5ms].stackSize = CONFIG_TASK5MS_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_Task10ms].name = "Task10ms_A";
    sMonitorEntries[eTaskMon_Task10ms].stackSize = CONFIG_TASK10MS_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_Task10msB].name = "Task10ms_B";
    sMonitorEntries[eTaskMon_Task10msB].stackSize = CONFIG_TASK10MS_GROUPB_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_Task50ms].name = "Task50ms";
    sMonitorEntries[eTaskMon_Task50ms].stackSize = CONFIG_TASK50MS_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_Task100ms].name = "Task100ms";
    sMonitorEntries[eTaskMon_Task100ms].stackSize = CONFIG_TASK100MS_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_Task200ms].name = "Task200ms";
    sMonitorEntries[eTaskMon_Task200ms].stackSize = CONFIG_TASK200MS_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_Task300ms].name = "Task300ms";
    sMonitorEntries[eTaskMon_Task300ms].stackSize = CONFIG_TASK300MS_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_Task400ms].name = "Task400ms";
    sMonitorEntries[eTaskMon_Task400ms].stackSize = CONFIG_TASK400MS_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_Task500ms].name = "Task500ms";
    sMonitorEntries[eTaskMon_Task500ms].stackSize = CONFIG_TASK500MS_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_Task1000ms].name = "Task1000ms";
    sMonitorEntries[eTaskMon_Task1000ms].stackSize = CONFIG_TASK1000MS_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_Task2000ms].name = "Task2000ms";
    sMonitorEntries[eTaskMon_Task2000ms].stackSize = CONFIG_TASK2000MS_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_MulPortManag].name = "MulPortManag";
    sMonitorEntries[eTaskMon_MulPortManag].stackSize = CONFIG_MULPORTMANAGE_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_LowAdcSamp].name = "LowAdcSamp";
    sMonitorEntries[eTaskMon_LowAdcSamp].stackSize = CONFIG_LOWADCSAMP_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_Can1Recv].name = "Can1Recv";
    sMonitorEntries[eTaskMon_Can1Recv].stackSize = CONFIG_CanRecv_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_Can1Send].name = "Can1Send";
    sMonitorEntries[eTaskMon_Can1Send].stackSize = CONFIG_CanSend_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_Can2Recv].name = "Can2Recv";
    sMonitorEntries[eTaskMon_Can2Recv].stackSize = CONFIG_CanRecv_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_Can2Send].name = "Can2Send";
    sMonitorEntries[eTaskMon_Can2Send].stackSize = CONFIG_CanSend_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_IccMsgRcvTask].name = "IccMsgRcvTask";
    sMonitorEntries[eTaskMon_IccMsgRcvTask].stackSize = CONFIG_CanSend_THREAD_STACK_SIZE;

    sMonitorEntries[eTaskMon_UdsTester].name = "UdsTester";
    sMonitorEntries[eTaskMon_UdsTester].stackSize = CONFIG_UdsTester_THREAD_STACK_SIZE;

    for (i = 0; i < eTaskMon_MAX; i++)
    {
        sMonitorEntries[i].execTimeMin = 0xFFFFFFFF;
    }
}

void TaskMonitorRegisterTaskAPI(TaskHandle_t handle, const char *name, u32 stackSize)
{
    u32 i;

    if (handle == NULL)
    {
        return;
    }

    for (i = 0; i < eTaskMon_MAX; i++)
    {
        if (sMonitorEntries[i].name != NULL && strcmp(sMonitorEntries[i].name, name) == 0)
        {
            sMonitorEntries[i].handle = handle;
            return;
        }
    }
}

void TaskMonitorRecordIterAPI(u32 taskId, u32 execTimeMs)
{
    if (taskId >= eTaskMon_MAX)
    {
        return;
    }

    if (execTimeMs < sMonitorEntries[taskId].execTimeMin)
    {
        sMonitorEntries[taskId].execTimeMin = execTimeMs;
    }

    if (execTimeMs > sMonitorEntries[taskId].execTimeMax)
    {
        sMonitorEntries[taskId].execTimeMax = execTimeMs;
    }

    sMonitorEntries[taskId].execTimeSum += execTimeMs;
    sMonitorEntries[taskId].execTimeCount++;

    if (sPeriodMs[taskId] > 0 && execTimeMs > sPeriodMs[taskId])
    {
        sMonitorEntries[taskId].timeoutCount++;
    }
}

void TaskMonitorUpdateStackAPI(u32 taskId)
{
    u32 remaining;
    u32 used;

    if (taskId >= eTaskMon_MAX)
    {
        return;
    }

    if (sMonitorEntries[taskId].handle == NULL)
    {
        return;
    }

    remaining = (u32)osThreadGetStackSpace((osThreadId_t)sMonitorEntries[taskId].handle);

    if (sMonitorEntries[taskId].stackSize > remaining)
    {
        used = sMonitorEntries[taskId].stackSize - remaining;
        if (used > sMonitorEntries[taskId].stackPeak)
        {
            sMonitorEntries[taskId].stackPeak = used;
        }
    }
}

void TaskMonitorThreadAPI(void *argument)
{
    UBaseType_t uxArraySize = 0;
    TaskStatus_t *taskStatusArray = NULL;
    static u32 sLastTotalRunTime = 0;

    u32 totalRunTime;
    UBaseType_t taskCount;
    u32 totalRunTimeDelta;
    u32 i;
    u32 j;
    u32 cpuSum;
    u32 taskActiveCount;
    u32 avg;
    u32 remaining;

    for (;;)
    {
        {
            UBaseType_t newSize = uxTaskGetNumberOfTasks() + 4u;
            if (newSize > uxArraySize)
            {
                if (taskStatusArray != NULL)
                {
                    ExtFree(taskStatusArray);
                }
                uxArraySize = newSize;
                taskStatusArray = (TaskStatus_t *)ExtMalloc(uxArraySize * sizeof(TaskStatus_t));
                if (taskStatusArray == NULL)
                {
                    printf("TaskMonitor: ExtMalloc taskStatusArray fail!!\r\n");
                    break;
                }
            }
        }

        taskCount = uxTaskGetSystemState(taskStatusArray, uxArraySize, (u32 *)&totalRunTime);

#ifdef CONFIG_TASKMONITOR_DIAGNOSTIC
        {
            
            u32 di, dj, dmatchCount = 0, dregCount = 0;

            /* CRIT-1: total task count + scheduler state */
            printf("[DIAG] uxTaskGetNumberOfTasks()   = %u\r\n", (u32)uxTaskGetNumberOfTasks());
            printf("[DIAG] xTaskGetSchedulerState() = %u\r\n", (u32)xTaskGetSchedulerState());
            printf("[DIAG] uxArraySize                   = %u\r\n", (u32)uxArraySize);

            /* CRIT-2: counter snapshots before/after call */
            printf("[DIAG] vPortGetCurrentTimeUs() BEFORE = %8llu\r\n", vPortGetCurrentTimeUs());
            printf("[DIAG] totalRunTime (WITH ptr)       = %8u  taskCount=%u\r\n",
                   totalRunTime, (u32)taskCount);

            /* CRIT-3: NULL pointer call — isolate whether &totalRunTime pointer matters */
            UBaseType_t dc = uxTaskGetSystemState(taskStatusArray, uxArraySize, NULL);
            printf("[DIAG] vPortGetCurrentTimeUs() AFTER  = %8llu\r\n", vPortGetCurrentTimeUs());
            printf("[DIAG] totalRunTime (NULL ptr)       = %8u  taskCount=%u\r\n",
                   totalRunTime, (u32)dc);

            /* per-task counters (only if taskCount > 0) */
            for (di = 0; di < taskCount; di++)
            {
                printf("[DIAG]   T[%u] handle=%p  ulRunTimeCounter=%8u\r\n",
                       di,
                       (void *)taskStatusArray[di].xHandle,
                       (u32)taskStatusArray[di].ulRunTimeCounter);
            }

            /* handle match stats */
            for (di = 0; di < eTaskMon_MAX; di++)
            {
                if (sMonitorEntries[di].handle == NULL) { continue; }
                dregCount++;
                for (dj = 0; dj < taskCount; dj++)
                {
                    if (taskStatusArray[dj].xHandle == sMonitorEntries[di].handle)
                    { dmatchCount++; break; }
                }
            }
            printf("[DIAG] Handle match: %u matched / %u registered\r\n",
                   dmatchCount, dregCount);
        }
#endif

        totalRunTimeDelta = totalRunTime - sLastTotalRunTime;

        cpuSum = 0;
        taskActiveCount = 0;

        for (i = 0; i < eTaskMon_MAX; i++)
        {
            if (sMonitorEntries[i].handle == NULL)
            {
                continue;
            }

            for (j = 0; j < taskCount; j++)
            {
                if (taskStatusArray[j].xHandle == sMonitorEntries[i].handle)
                {
                    u32 runtimeDelta;
                    u32 cpuPercent;

                    runtimeDelta = taskStatusArray[j].ulRunTimeCounter - sMonitorEntries[i].lastRuntimeCounter;

                    if (totalRunTimeDelta > 0)
                    {
                        cpuPercent = (u32)(((u64)runtimeDelta * 1000u) / (u64)totalRunTimeDelta);
                    }
                    else
                    {
                        cpuPercent = 0;
                    }

                    sMonitorEntries[i].cpuPercent = cpuPercent;
                    sMonitorEntries[i].runtimeDeltaUs = runtimeDelta;
                    sMonitorEntries[i].lastRuntimeCounter = taskStatusArray[j].ulRunTimeCounter;

                    cpuSum += cpuPercent;

                    break;
                }
            }

            sMonitorEntries[i].state = eTaskGetState(sMonitorEntries[i].handle);

            remaining = (u32)osThreadGetStackSpace((osThreadId_t)sMonitorEntries[i].handle);
            if (sMonitorEntries[i].stackSize > remaining)
            {
                u32 used = sMonitorEntries[i].stackSize - remaining;
                if (used > sMonitorEntries[i].stackPeak)
                {
                    sMonitorEntries[i].stackPeak = used;
                }
            }

            taskActiveCount++;
        }

        printf("=============================================\r\n");
        printf("Task Monitor  [Tick: %8u]  Interval: %us\r\n",
               (u32)xTaskGetTickCount(),
               (u32)(CONFIG_TASKMONITOR_PERIOD_MS / 1000));
        printf("=============================================\r\n");
        printf("%-18s %5s %10s %9s %-9s %4s %22s\r\n",
               "Name", "CPU%", "Runtime", "Stack(B)", "State", "TO", "ExecTime(min/avg/max)");
        printf("------------------------------------------------------------------------------------\r\n");

        for (i = 0; i < eTaskMon_MAX; i++)
        {
            u32 execMin;
            u32 execMax;

            if (sMonitorEntries[i].handle == NULL)
            {
                continue;
            }

            if (sMonitorEntries[i].execTimeCount > 0)
            {
                avg = sMonitorEntries[i].execTimeSum / sMonitorEntries[i].execTimeCount;
                execMin = sMonitorEntries[i].execTimeMin;
                execMax = sMonitorEntries[i].execTimeMax;
            }
            else
            {
                avg = 0;
                execMin = 0;
                execMax = 0;
            }

            printf("%-18s %3u.%1u %7uus %4u/%-4u %-9s %4u %5u/%-4u/%-5ums\r\n",
                   sMonitorEntries[i].name,
                   sMonitorEntries[i].cpuPercent / 10,
                   sMonitorEntries[i].cpuPercent % 10,
                   sMonitorEntries[i].runtimeDeltaUs,
                    sMonitorEntries[i].stackPeak,
                    sMonitorEntries[i].stackSize,
                   StateNames[sMonitorEntries[i].state],
                   sMonitorEntries[i].timeoutCount,
                   execMin,
                   avg,
                   execMax);
        }

        printf("------------------------------------------------------------------------------------\r\n");
        printf("Total: %u tasks  CPU sum: %u.%u%%  Free heap: %u\r\n",
               taskActiveCount,
               cpuSum / 10, cpuSum % 10,
               (u32)xPortGetFreeHeapSize());
        printf("=============================================\r\n");

        for (i = 0; i < eTaskMon_MAX; i++)
        {
            sMonitorEntries[i].execTimeMin = 0xFFFFFFFF;
            sMonitorEntries[i].execTimeMax = 0;
            sMonitorEntries[i].execTimeSum = 0;
            sMonitorEntries[i].execTimeCount = 0;
            sMonitorEntries[i].timeoutCount = 0;
        }

        sLastTotalRunTime = totalRunTime;

        osDelay(pdMS_TO_TICKS(CONFIG_TASKMONITOR_PERIOD_MS));
    }
}

#endif
