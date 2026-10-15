#include "uds_upgrade_manager.h"
#include "icc_upgrade_handler.h"
#include "TaskMonitor.h"
#include <cmsis_os2.h>

/*
 * UDS 升级 tester 端 RTOS 任务入口。
 * 由调度器创建后循环执行，是整个升级流程的"心脏"：每轮先处理 ICC 通信
 * (icc_upgrade_process 负责文件接收/超时重传)，再驱动升级总协调器
 * (uds_upgrade_manager_proc 负责状态机推进/重试/进度上报)，最后记录本轮耗时供任务监控。
 */
void TaskUdsTester(void *argument)
{
    (void)argument;   // 参数未使用，消除编译告警

    while (1) {
        uint32_t start_tick = osKernelGetTickCount();   // 记录本轮起始 tick，用于计算单次执行耗时

        icc_upgrade_process();        // 1. 处理 ICC 升级通信：分块文件接收、超时重传
        uds_upgrade_manager_proc();   // 2. 驱动升级总协调器：状态切换、CAN 通道切换、重试、进度上报

        uint32_t execution_time = osKernelGetTickCount() - start_tick;   // 本轮执行耗时(ms)
        MONITOR_RECORD_ITER(eTaskMon_UdsTester, execution_time);         // 上报任务监控，便于观测负载
        osDelay(1);                   // 延时 1 tick 让出 CPU，等待下一轮循环
    }
}
