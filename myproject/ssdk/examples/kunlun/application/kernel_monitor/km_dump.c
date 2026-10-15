/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cmsis_os2.h>


static inline const char* sdrvReturnTaskState(osThreadState_t state)
{
	switch(state)
	{
	case osThreadRunning:
		return "InRun";
	case osThreadReady:
		return "Ready";
	case osThreadBlocked:
		return "Block";
	case osThreadTerminated:
		return "Zombie";
	case osThreadError:
		return "Invad";
	default: return "Error";
	}
}

void dumpTaskStats(void)
{
    uint32_t thread_num, cnt;
    osThreadId_t *thread_array;

    printf("%-20s%-20s%-20s%-20s%-20s\r\n", "Task", "State", "Priority", "Stack", "TCB");

    thread_num = osThreadGetCount();

    if (thread_num) {
        thread_array = malloc(sizeof(osThreadId_t) * thread_num);
        osThreadEnumerate(thread_array, thread_num);

        for (cnt = 0; cnt < thread_num; cnt++) {
            osThreadId_t current_thread = thread_array[cnt];
            printf("%-20s%-20s%-20d%-20d%-20p\r\n",
                   osThreadGetName(current_thread),
                   sdrvReturnTaskState(osThreadGetState(current_thread)),
                   osThreadGetPriority(current_thread),
                   osThreadGetStackSpace(current_thread),
                   current_thread);
        }

        free(thread_array);
    }

    return;
}

static void dumpSystemTop(uint32_t interval)
{
    uint32_t thread_num, cnt, i;
    osThreadInfo_t *threadInfo_array1;
    osThreadInfo_t *threadInfo_array2;
    uint32_t uTotalTime1 = 0, uTotalTime2 = 0;
    uint32_t ulRunTimeInc, uTaskCPUPercent;

    //printf("Mem:%8dK total, %8dK free\r\n", 0, 0);
    printf("%-20s%-20s%-20s%-20s%-20s%-20s\r\n", "Task", "ID", "State", "Priority", "Time", "CPU%");

    thread_num = osThreadGetCount();

    if (thread_num) {
        threadInfo_array1 = malloc(sizeof(osThreadInfo_t) * thread_num);
        threadInfo_array2 = malloc(sizeof(osThreadInfo_t) * thread_num);
        osThreadInfoEnumerate(threadInfo_array1, thread_num, &uTotalTime1);
        osDelay(interval);
        osThreadInfoEnumerate(threadInfo_array2, thread_num, &uTotalTime2);

        for (cnt = 0; cnt < thread_num; cnt++) {
            osThreadId_t current_thread = threadInfo_array1[cnt].thread_handle;
            for (i = 0; i < thread_num; i++) {
                if (threadInfo_array1[cnt].thread_id == threadInfo_array2[i].thread_id) {
                    ulRunTimeInc = threadInfo_array2[i].runtime_counter - threadInfo_array1[cnt].runtime_counter;
                    uTaskCPUPercent = 100UL*ulRunTimeInc/(uTotalTime2 - uTotalTime1);
                    break;
                }
            }

            printf("%-20s%-20d%-20s%-20d%-20lu%-20d\r\n",
                   osThreadGetName(current_thread),
                   threadInfo_array1[cnt].thread_id,
                   sdrvReturnTaskState(osThreadGetState(current_thread)),
                   osThreadGetPriority(current_thread),
                   (unsigned long)threadInfo_array1[cnt].runtime_counter/(1000),
                   uTaskCPUPercent);
            ulRunTimeInc = 0;
            uTaskCPUPercent = -1;
        }

        free(threadInfo_array1);
        free(threadInfo_array2);
    }

    return;
}

void sdvrTaskStateInfo(void)
{
    printf("------------State abbr.------------\n");
	printf("InRun   - Current running task\n");
	printf("Ready   - Next ready task\n");
	printf("Block   - Task was blocked\n");
	printf("Suspd   - Task was suspended\n");
	printf("Wfdlt   - task was deleted but not recycled\n");
	printf("Invad   - Unexpected state\n");
	printf("Error   - Unexpected state\n");
	printf("Deleted - Task was deleted and recycled\n");
	printf("-----------------------------------\n");
	printf("Use top command to show task stats\n");
}

void sdrvDumpTaskInfo(uint32_t interval, uint32_t count)
{
    uint32_t i;

    for (i = 0;i < count;i++) {
        printf("--------------------------------------------------------------------------------------------------------\n");
        dumpSystemTop(interval*1000);
    }
    printf("--------------------------------------------------------------------------------------------------------\n");
}
