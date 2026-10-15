/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : DiagAsyncLog.c
** @Founder      : ZQD808
** @Date         : 2026.05.08
** @Function     : 异步日志队列消费者任务 — 解耦 printf 以消除 Task10ms 饥饿
** @Instructions : 消费者以低优先级（5）运行，从 osMessageQueue 取出消息并 printf 到 UART
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "DiagAsyncLog.h"
#include <stdio.h>
#include <string.h>
#include <cmsis_os2.h>

static osMessageQueueId_t   sAsyncLogQueue      = NULL;
static u32                  sAsyncLogDropCount   = 0;
static u8                   sAsyncLogInitDone    = 0;

e_FunStatus AsyncLogInitAPI(void)
{
    if (sAsyncLogInitDone)
    {
        return ok;
    }

    sAsyncLogQueue = osMessageQueueNew(ASYNC_LOG_QUEUE_SIZE, sizeof(t_AsyncLogMsg), NULL);
    if (sAsyncLogQueue == NULL)
    {
        printf("AsyncLogInitAPI: queue create fail!!\r\n");
        return failed;
    }

    sAsyncLogDropCount = 0;
    sAsyncLogInitDone = 1;
    return ok;
}

e_FunStatus AsyncLogSendAPI(u8 channel, u8 *data, u16 len)
{
    t_AsyncLogMsg msg;
    osStatus_t    status;

    if (!sAsyncLogInitDone)
    {
        return errP0;
    }
    if (len == 0 || len > 475)
    {
        return errP1;
    }
    if (channel > 3)
    {
        return errP2;
    }

    msg.channel = channel;
    msg.len = len;
    (void)memcpy(msg.data, data, len);

    status = osMessageQueuePut(sAsyncLogQueue, &msg, 0, 0);
    if (status == osErrorResource)
    {
        sAsyncLogDropCount++;
        return ok;
    }

    return ok;
}

u32 AsyncLogGetDropCountAPI(void)
{
    return sAsyncLogDropCount;
}

void AsyncLogConsumerTask(void *argument)
{
    t_AsyncLogMsg msg;
    u16           i;

    (void)argument;

    for (;;)
    {
        if (osMessageQueueGet(sAsyncLogQueue, &msg, NULL, osWaitForever) != osOK)
        {
            continue;
        }

        printf("--- CH%d LEN=%d ---\r\n", msg.channel, msg.len);

        for (i = 0; i < msg.len; i++)
        {
            printf("%d\r\n", msg.data[i]);
        }

        osDelay(1);
    }
}
