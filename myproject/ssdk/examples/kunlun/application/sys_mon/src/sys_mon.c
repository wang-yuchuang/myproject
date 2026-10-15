#include <CLI.h>
#include <debug.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <driver/wav_header.h>
#include "param.h"
#include "FreeRTOS.h"
#include "task.h"
#include <cmsis_os2.h>

/* 链接器符号 (名称需与 xip.ld 一致) */
extern const unsigned long __data_start;
extern const unsigned long __data_end;
extern const unsigned long __bss_start;
extern const unsigned long __bss_end;
extern const unsigned long _heap_start;
extern const unsigned long _heap_end;
extern const unsigned long _end_of_ram;

static int sys_mon_task(int argc, char *argv[])
{
    while(1) {
        unsigned long d_start = (unsigned long)&__data_start;
        unsigned long d_end   = (unsigned long)&__data_end;
        unsigned long b_start = (unsigned long)&__bss_start;
        unsigned long b_end   = (unsigned long)&__bss_end;
        unsigned long h_start = (unsigned long)&_heap_start;
        unsigned long h_end   = (unsigned long)&_heap_end;
        unsigned long eor     = (unsigned long)&_end_of_ram;
        unsigned long total = eor - d_start;
        unsigned long heap_total = eor - h_start;
        unsigned long heap_free  = (unsigned long)xPortGetFreeHeapSize();
        unsigned long used  = h_end - d_start;
        printf("  .data: 0x%08lx-%08lx (%lu B)\n", d_start, d_end, d_end-d_start);
        printf("  .bss:  0x%08lx-%08lx (%lu B)\n", b_start, b_end, b_end-b_start);
        printf("  .heap: 0x%08lx-%08lx  FreeRTOS free: %lu B, min: %lu B\n",
               h_start, h_end,
               (unsigned long)xPortGetFreeHeapSize(),
               (unsigned long)xPortGetMinimumEverFreeHeapSize());

        TaskStatus_t *pxTaskStatusArray;
        volatile UBaseType_t uxArraySize, x;
        uint32_t ulTotalRunTime;
        uxArraySize = uxTaskGetNumberOfTasks();
        pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));
        if (pxTaskStatusArray != NULL) {
            uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);
            for (x = 0; x < uxArraySize; x++)
                printf("  Task '%s': Stack min=%u words\n",
                       pxTaskStatusArray[x].pcTaskName,
                       pxTaskStatusArray[x].usStackHighWaterMark);
            vPortFree(pxTaskStatusArray);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    return 0;
}


static int sys_mon_task_init(int argc, char *argv[])
{
    //pvt setting
    osThreadAttr_t attr;
    memset(&attr, 0x00, sizeof(attr));
    attr.name = "sys_mon";
    attr.priority = osPriorityNormal;
    attr.stack_size = 1024;

    osThreadNew((osThreadFunc_t)sys_mon_task, NULL, &attr);
    return 0;
}

CLI_CMD("sys_mon",
        "sys_mon"
        "\r\n\t system stack and heap state monitor. \r\n\t",
        sys_mon_task_init);


