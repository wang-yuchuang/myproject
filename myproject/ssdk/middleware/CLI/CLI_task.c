/*
 * CLI_task.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: console function.
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <stdio.h>

#include "cmsis_os2.h"
#include "CLI_type.h"

/*
 * Task Stats commands.
 */
static int CLITaskStatsCommand(int argc, char *argv[])
{
    uint32_t thread_num, cnt;
    osThreadId_t *thread_array;

    printf("%-20s%-20s%-20s%-20s\r\n", "Task", "State", "Priority", "Stack");

    thread_num = osThreadGetCount();

    if (thread_num) {
        thread_array = malloc(sizeof(osThreadId_t) * thread_num);
        osThreadEnumerate(thread_array, thread_num);

        for (cnt = 0; cnt < thread_num; cnt++) {
            osThreadId_t current_thread = thread_array[cnt];
            printf("%-20s%-20d%-20d%-20d\r\n",
                   osThreadGetName(current_thread),
                   osThreadGetState(current_thread),
                   osThreadGetPriority(current_thread),
                   osThreadGetStackSpace(current_thread));
        }

        free(thread_array);
    }

    return CLI_FALSE;
}

CLI_CMD("task-stats", "Showing the state of RTOS task", CLITaskStatsCommand);
