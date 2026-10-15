/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : OSUserHook.c
** @Founder      : ZQD808
** @Date         : 2025.11.12
** @Function     : BSP定时任务服务模块
** @Instructions : 用户周期函数在这里调用
**===========================================================================================*/
#define SCHED_HEARTBEAT_DIAG
#include <arch.h>
#include <cmsis_os2.h>
#include <driver/uart.h>
#include <driver/arch_timer.h>
#include <driver/taishan/fuse/fuse.h>
#include "IccMsgInterface.h"
#include <board.h>
#include <debug.h>
#include "OSUserHook.h"
#include "DEVGPIO.h"
#include "DEVADC.h"
#include "M2514.h"
#include "GD25Q32.h"
#include "DEVUART.h"
#include "DEVCAN.h"
#include "IccMsgHandle.h"
#include "IccMsgTxCycle.h"
#include "SlaveMsg.h"
#include "BalanTactic.h"
#include "FireTactic.h"
#include "CharactCalc.h"
#include "DiagCheck.h"
#include "DiagInterface.h"
#include "CurrLimFall.h"
#include "DiagMaker.h"
#include "CurrLimCalc.h"
#include "DiagUser.h"
#include "GroupState.h"
#include "EnerUser.h"
#include "SocUserAPI.h"
#include "EnergyStatis.h"
#include "UserTactic.h"
#include "TimesStatis.h"
#include "DodRcdStatis.h"
#include "EnerInfoCalc.h"
#include "CellFadeCalc.h"
#include "GroupProtect.h"
#include "VoltLimCalc.h"
#include "GroupMode.h"
#include "GroupTactic.h"
#include "DiagFilter.h"
#include "DiagAsyncLog.h"
#include "TaskMonitor.h"
#include "TacUser.h"
#include "AuxDevTactic.h"
#include "InsuCheck.h"
#include "EventRec.h"
#include "DEVWDT.h"
/*=============================================================================================
** @Name      : static void Task2ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 2ms任务处理
** @The notes : 2ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task2ms(void *argument);
/*=============================================================================================
** @Name      : static void Task5ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 5ms任务处理
** @The notes : 5ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task5ms(void *argument);
/*=============================================================================================
** @Name      : static void Task10ms_GroupA(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 10ms任务处理 GroupA — BMS 核心计算
** @The notes : 10ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task10ms_GroupA(void *argument);
/*=============================================================================================
** @Name      : static void Task10ms_GroupB(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 10ms任务处理 GroupB — 诊断状态机专用
** @The notes : DiagCheckStateMachineTask 所有调用必须在同一线程
**===========================================================================================*/
static void Task10ms_GroupB(void *argument);
/*=============================================================================================
** @Name      : static void Task50ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 50ms任务处理
** @The notes : 50ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task50ms(void *argument);
/*=============================================================================================
** @Name      : static void Task100ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 100ms任务处理
** @The notes : 100ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task100ms(void *argument);
/*=============================================================================================
** @Name      : static void Task200ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 200ms任务处理
** @The notes : 200ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task200ms(void *argument);
/*=============================================================================================
** @Name      : static void Task300ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 300ms任务处理
** @The notes : 300ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task300ms(void *argument);
/*=============================================================================================
** @Name      : static void Task400ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 400ms任务处理
** @The notes : 400ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task400ms(void *argument);
/*=============================================================================================
** @Name      : static void Task500ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 500ms任务处理
** @The notes : 500ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task500ms(void *argument);
/*=============================================================================================
** @Name      : static void Task1000ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 1000ms任务处理
** @The notes : 1000ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task1000ms(void *argument);
/*=============================================================================================
** @Name      : static void Task2000ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 2000ms任务处理
** @The notes : 2000ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task2000ms(void *argument);
/*=============================================================================================
** @Name      : void UserTaskCreate(void)
** @Input     : void
** @Output    : void
** @Function  : 用户任务创建
**===========================================================================================*/
void UserTaskCreate(void)
{
#if 0
    AsyncLogInitAPI();
#endif
    TaskMonitorInitAPI();

#ifdef CONFIG_TASK_MONITOR
    osThreadAttr_t attrMon = {
        .name = "TaskMon",
        .stack_size = CONFIG_TASKMONITOR_THREAD_STACK_SIZE,
        .priority = CONFIG_TASKMONITOR_THREAD_PRIORITY,
    };
    osThreadNew(TaskMonitorThreadAPI, NULL, &attrMon);
#endif

    //创建任务
    osThreadAttr_t attr1 = {
        .name = "MulPortManag",
        .stack_size = CONFIG_MULPORTMANAGE_THREAD_STACK_SIZE,
        .priority = CONFIG_MULPORTMANAGE_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(DEVGPIOMulPortManagTask, NULL, &attr1), "MulPortManag", attr1.stack_size);
    osThreadAttr_t attr2 = {
        .name = "LowAdcSamp",
        .stack_size = CONFIG_LOWADCSAMP_THREAD_STACK_SIZE,
        .priority = CONFIG_LOWADCSAMP_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(LowInfoSampleTask, NULL, &attr2), "LowAdcSamp", attr2.stack_size);

    //周期性任务
    osThreadAttr_t attr3 = {
        .name = "Task2ms",
        .stack_size = CONFIG_TASK2MS_THREAD_STACK_SIZE,
        .priority = CONFIG_TASK2MS_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(Task2ms, NULL, &attr3), "Task2ms", attr3.stack_size);

    osThreadAttr_t attr4 = {
        .name = "Task5ms",
        .stack_size = CONFIG_TASK5MS_THREAD_STACK_SIZE,
        .priority = CONFIG_TASK5MS_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(Task5ms, NULL, &attr4), "Task5ms", attr4.stack_size);

    osThreadAttr_t attr5 = {
        .name = "Task10ms_A",
        .stack_size = CONFIG_TASK10MS_THREAD_STACK_SIZE,
        .priority = CONFIG_TASK10MS_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(Task10ms_GroupA, NULL, &attr5), "Task10ms_A", attr5.stack_size);

    osThreadAttr_t attr5b = {
        .name = "Task10ms_B",
        .stack_size = CONFIG_TASK10MS_GROUPB_THREAD_STACK_SIZE,
        .priority = CONFIG_TASK10MS_GROUPB_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(Task10ms_GroupB, NULL, &attr5b), "Task10ms_B", attr5b.stack_size);

    osThreadAttr_t attr6 = {
        .name = "Task50ms",
        .stack_size = CONFIG_TASK50MS_THREAD_STACK_SIZE,
        .priority = CONFIG_TASK50MS_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(Task50ms, NULL, &attr6), "Task50ms", attr6.stack_size);

    osThreadAttr_t attr7 = {
        .name = "Task100ms",
        .stack_size = CONFIG_TASK100MS_THREAD_STACK_SIZE,
        .priority = CONFIG_TASK100MS_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(Task100ms, NULL, &attr7), "Task100ms", attr7.stack_size);

    osThreadAttr_t attr8 = {
        .name = "Task200ms",
        .stack_size = CONFIG_TASK200MS_THREAD_STACK_SIZE,
        .priority = CONFIG_TASK200MS_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(Task200ms, NULL, &attr8), "Task200ms", attr8.stack_size);

    osThreadAttr_t attr9 = {
        .name = "Task300ms",
        .stack_size = CONFIG_TASK300MS_THREAD_STACK_SIZE,
        .priority = CONFIG_TASK300MS_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(Task300ms, NULL, &attr9), "Task300ms", attr9.stack_size);

    osThreadAttr_t attr10 = {
        .name = "Task400ms",
        .stack_size = CONFIG_TASK400MS_THREAD_STACK_SIZE,
        .priority = CONFIG_TASK400MS_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(Task400ms, NULL, &attr10), "Task400ms", attr10.stack_size);

    osThreadAttr_t attr11 = {
        .name = "Task500ms",
        .stack_size = CONFIG_TASK500MS_THREAD_STACK_SIZE,
        .priority = CONFIG_TASK500MS_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(Task500ms, NULL, &attr11), "Task500ms", attr11.stack_size);

    osThreadAttr_t attr12 = {
        .name = "Task1000ms",
        .stack_size = CONFIG_TASK1000MS_THREAD_STACK_SIZE,
        .priority = CONFIG_TASK1000MS_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(Task1000ms, NULL, &attr12), "Task1000ms", attr12.stack_size);

    osThreadAttr_t attr13 = {
        .name = "Task2000ms",
        .stack_size = CONFIG_TASK2000MS_THREAD_STACK_SIZE,
        .priority = CONFIG_TASK2000MS_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(Task2000ms, NULL, &attr13), "Task2000ms", attr13.stack_size);



    osThreadAttr_t attr17 = {
        .name = "Can2RecvTask",
        .stack_size = CONFIG_CanRecv_THREAD_STACK_SIZE,
        .priority = CONFIG_CanRecv_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(BSPCAN2RcvTask, NULL, &attr17), "Can1Recv", attr17.stack_size);

    osThreadAttr_t attr18 = {
        .name = "Can3RecvTask",
        .stack_size = CONFIG_CanRecv_THREAD_STACK_SIZE,
        .priority = CONFIG_CanRecv_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(BSPCAN3RcvTask, NULL, &attr18), "Can2Recv", attr18.stack_size);


    osThreadAttr_t attr20 = {
        .name = "Can2SendTask",
        .stack_size = CONFIG_CanSend_THREAD_STACK_SIZE,
        .priority = CONFIG_CanSend_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(BSPCAN2SendTask, NULL, &attr20), "Can1Send", attr20.stack_size);

    osThreadAttr_t attr21 = {
        .name = "Can3SendTask",
        .stack_size = CONFIG_CanSend_THREAD_STACK_SIZE,
        .priority = CONFIG_CanSend_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(BSPCAN3SendTask, NULL, &attr21), "Can2Send", attr21.stack_size);

    osThreadAttr_t attr23 = {
        .name = "IccMsgRcvTask",
        .stack_size = CONFIG_IccMsgRcv_THREAD_STACK_SIZE,
        .priority = CONFIG_IccMsgRcv_THREAD_PRIORITY,
    };
    MONITOR_REGISTER_TASK(osThreadNew(IccMsgRcvTask, NULL, &attr23), "IccMsgRcvTask", attr23.stack_size);

#if 0
    osThreadAttr_t attrAsyncLog = {
        .name = "AsyncLog",
        .stack_size = CONFIG_ASYNCLOG_THREAD_STACK_SIZE,
        .priority = CONFIG_ASYNCLOG_THREAD_PRIORITY,
    };
    osThreadNew(AsyncLogConsumerTask, NULL, &attrAsyncLog);
#endif


    
}
/*=============================================================================================
** @Name      : static void Task2ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 2ms任务处理
** @The notes : 2ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task2ms(void *argument)
{
    u32 nextDeadline = osKernelGetTickCount();
    while(1)
    {
        uint32_t start_tick = osKernelGetTickCount();
        //任务处理函数
        uint32_t execution_time = osKernelGetTickCount() - start_tick;
        MONITOR_RECORD_ITER(eTaskMon_Task2ms, execution_time);
        nextDeadline += pdMS_TO_TICKS(2);
        if(osKernelGetTickCount() < nextDeadline)
        {
            osDelayUntil(nextDeadline);
        }
        else
        {
            osDelay(1);
            nextDeadline = osKernelGetTickCount() + pdMS_TO_TICKS(2);
        }
    }
}
/*=============================================================================================
** @Name      : static void Task5ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 5ms任务处理
** @The notes : 5ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task5ms(void *argument)
{
    u32 nextDeadline = osKernelGetTickCount();
    while(1)
    {
        uint32_t start_tick = osKernelGetTickCount();
        //任务处理函数开始

        //任务处理函数结束
        uint32_t execution_time = osKernelGetTickCount() - start_tick;
        MONITOR_RECORD_ITER(eTaskMon_Task5ms, execution_time);
        nextDeadline += pdMS_TO_TICKS(5);
        if(osKernelGetTickCount() < nextDeadline)
        {
            osDelayUntil(nextDeadline);
        }
        else
        {
            osDelay(1);
            nextDeadline = osKernelGetTickCount() + pdMS_TO_TICKS(5);
        }
    }
}
static void Task10ms_GroupA(void *argument)
{
    static u8 sTime = 0;
    u32 nextDeadline = osKernelGetTickCount();
    while(1)
    {
        uint32_t start_tick = osKernelGetTickCount();
        switch(sTime)
        {
            case 0:
                CurrLimCalcTask();
                break;

            case 1:
                CheckVoltCharaTask();
                break;

            case 2:
                GroupCapSocCalcTask();
                break;

            case 3:
                GroupProStateTask();
                break;

            case 4:
                CheckTempCharaTask();
                CheckAvgCurrTask();
                break;

            case 5:
                GroupEnerSoeCalcTask();
                break;

            case 6:
                DiagErrDisplayTask();
                break;

            case 7:
                CurrLimFallTask();
                GroupFadeSohCalcTask();
                break;

            case 8:
                GroupEnerDisplayTask();
                DiagSwhChangCStateCheckTask();
                break;

            case 9:
                GroupBattStateCheckTask();
                #if((0 == BMU_BSU_COM_TYPE) || (0 == BMU_BSU_CAN_TYPE))
                GroupVoltBalanTacticTask();
                #endif
                break;

            default:
                break;
        }
        sTime++;
        if(sTime >= TASK10MS_SUBTICK_MAX)
        {
            sTime = 0;
            //printf("GroupA: sTime=%d tick=%lu\r\n", sTime, osKernelGetTickCount());
        }

        uint32_t execution_time = osKernelGetTickCount() - start_tick;
        MONITOR_RECORD_ITER(eTaskMon_Task10ms, execution_time);
        nextDeadline += pdMS_TO_TICKS(10);
        if(osKernelGetTickCount() < nextDeadline)
        {
            osDelayUntil(nextDeadline);
        }
        else
        {
            osDelay(1);
            nextDeadline = osKernelGetTickCount() + pdMS_TO_TICKS(10);
        }
    }
}          
/*=============================================================================================
** @Name      : static void Task10ms_GroupB(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 10ms任务处理 GroupB — 诊断状态机专用
**===========================================================================================*/
static void Task10ms_GroupB(void *argument)
{
    static u8 sTime = 0;
    u32 nextDeadline = osKernelGetTickCount();
    while(1)
    {
        uint32_t start_tick = osKernelGetTickCount();
        switch(sTime)
        {
            case 0:
                DiagVoltLimFilMakerTask();
                break;

            case 1:
                DiagTempLimFilMakerTask();
                break;

            case 2:
                DiagCurrLimFilMakerTask();
                break;

            case 3:
                DiagPortExpFilMakerTask();
                break;

            case 4:
                DiagStateExpFilMakerTask();
                break;

            case 5:
                DiagInfoExpFilMakerTask();
                break;

            case 6:
                break;

            case 7:
                break;

            case 8:
                break;

            case 9:
                break;

            default:
                break;
        }
        ICCTxCellCycleTabHandle();
        sTime++;
        if(sTime >= TASK10MS_GROUPB_SUBTICK_MAX)
        {
            sTime = 0;
            //printf("GroupB: sTime=%d tick=%lu\r\n", sTime, osKernelGetTickCount());
        }

        uint32_t execution_time = osKernelGetTickCount() - start_tick;
        MONITOR_RECORD_ITER(eTaskMon_Task10msB, execution_time);
        nextDeadline += pdMS_TO_TICKS(10);
        if(osKernelGetTickCount() < nextDeadline)
        {
            osDelayUntil(nextDeadline);
        }
        else
        {
            osDelay(1);
            nextDeadline = osKernelGetTickCount() + pdMS_TO_TICKS(10);
        }
    }
}
/*=============================================================================================
** @Name      : static void Task50ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 50ms任务处理
** @The notes : 50ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task50ms(void *argument)
{
    u32 nextDeadline = osKernelGetTickCount();
    while(1)
    {
        uint32_t start_tick = osKernelGetTickCount();

        /* 每50ms喂狗（内部Kunlun WDT + 外部GPIO翻转） */
        DEVWDTFeedTask();

        //任务处理函数
        /*故障列表滤波检测*/
        DiagErrFilterTask(eERR_ID_CEL_VH2, eERR_ID_FINISH);

        /*SOC、SOE算法包总任务*/
        SocSoeLibAllTask();

        /*主控用户发送CAN报文*/
        //GroupSendUpMsg1sBy50ms_API();

        //使能自研从控BSU CAN通信信息
        #if((1 == BMU_BSU_COM_TYPE) && (0 == BMU_BSU_CAN_TYPE))
        /*主控下发报文和判断从控信息采集完成*/
        GroupCheckRcvBsuMsgAPI();
        #endif

        uint32_t execution_time = osKernelGetTickCount() - start_tick;
        MONITOR_RECORD_ITER(eTaskMon_Task50ms, execution_time);
        nextDeadline += pdMS_TO_TICKS(50);
        if(osKernelGetTickCount() < nextDeadline)
        {
            osDelayUntil(nextDeadline);
        }
        else
        {
            osDelay(1);
            nextDeadline = osKernelGetTickCount() + pdMS_TO_TICKS(50);
        }
    }
}
/*=============================================================================================
** @Name      : static void Task100ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 100ms任务处理
** @The notes : 100ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task100ms(void *argument)
{
    u32 nextDeadline = osKernelGetTickCount();
    while(1)
    {
        uint32_t start_tick = osKernelGetTickCount();
        //任务处理函数
        /*软件喂狗定时函数  */
        //BSPWatchDogTask();

        /*电池组工作状态检测*/
        GroupWorkStateTask();

        /*电池组充放电状态检测*/
        GroupChgDhgStateTask();

        /*电池组安全等级诊断*/
        //DiagGroupSosTask();   

        /*电池组故障保护控制*/
        GroupProtectTask();
        InsuCheckTask();   /* 绝缘检测 */
        EventRecordUserTask();  /* 事件记录 */
        /*故障录波数据处理(非数据存储)*/
        //RcdFaultWaveHandleTask();
        uint32_t execution_time = osKernelGetTickCount() - start_tick;
        MONITOR_RECORD_ITER(eTaskMon_Task100ms, execution_time);
        nextDeadline += pdMS_TO_TICKS(100);
        if(osKernelGetTickCount() < nextDeadline)
        {
            osDelayUntil(nextDeadline);
        }
        else
        {
            osDelay(1);
            nextDeadline = osKernelGetTickCount() + pdMS_TO_TICKS(100);
        }
    }
}
/*=============================================================================================
** @Name      : static void Task200ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 200ms任务处理
** @The notes : 200ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task200ms(void *argument)
{
    static u8 sTime = 0;
    static u32 sDeadlineMissCnt = 0;
    u32 nextDeadline = osKernelGetTickCount();
    while(1)
    {
        uint32_t start_tick = osKernelGetTickCount();
        //任务处理函数
        /*200ms驱动1s任务*/
        switch(sTime)
        {
            /*热管理控制任务*/
            case 0:
                break;

            /*电压限值计算任务*/
            case 1:
                VoltLimCalcTask();
                break;

            /*工作模式检测任务*/
            case 2:
                GroupWorkModeTask();
                break;

            /*通信诊断任务*/
            case 3:
                DiagCommuCheckTask();
                break;

            /*高压采样/电流采样校准任务*/
            case 4:
                //HigVoltSampCorrProcess();
                //OutCurrSampCorrProcess();
                break;

            default:
                break;
        }

        sTime++;
        if(sTime >= 5)
        {
            sTime = 0;
        }
        uint32_t execution_time = osKernelGetTickCount() - start_tick;
        MONITOR_RECORD_ITER(eTaskMon_Task200ms, execution_time);
        nextDeadline += pdMS_TO_TICKS(200);
        {
            static u32 sHeartbeat = 0;
            sHeartbeat++;
        }
        if(osKernelGetTickCount() < nextDeadline)
        {
            osDelayUntil(nextDeadline);
        }
        else
        {
            sDeadlineMissCnt++;
            if(sDeadlineMissCnt >= 3)
            {
                osDelay(pdMS_TO_TICKS(200));
                sDeadlineMissCnt = 0;
            }
            else
            {
                osDelay(1);
            }
            nextDeadline = osKernelGetTickCount() + pdMS_TO_TICKS(200);
        }
    }
}
/*=============================================================================================
** @Name      : static void Task300ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 300ms任务处理
** @The notes : 300ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task300ms(void *argument)
{
    static u8 sTime = 0;
    u32 nextDeadline = osKernelGetTickCount();
    while(1)
    {
        uint32_t start_tick = osKernelGetTickCount();
        //任务处理函数

        /*单次循环充放电量统计*/
        GroupCycleCapStatisTask();

        /*在线升级擦除外部FLASH程序下载区任务*/
        //LoadAppFlashErasHandleAPI();

        /*远程升级状态信息报文发送任务*/
        //CanSendRemLoadMsgAPI();

        //转化600ms周期
        if(0 == sTime)
        {
            /*历史记录处理任务*/
            //RcdStoreHandleTask();
            sTime = 1;
        }
        else
        {
            sTime = 0;
        }
        uint32_t execution_time = osKernelGetTickCount() - start_tick;
        MONITOR_RECORD_ITER(eTaskMon_Task300ms, execution_time);
        nextDeadline += pdMS_TO_TICKS(300);
        if(osKernelGetTickCount() < nextDeadline)
        {
            osDelayUntil(nextDeadline);
        }
        else
        {
            osDelay(1);
            nextDeadline = osKernelGetTickCount() + pdMS_TO_TICKS(300);
        }
    }
}
/*=============================================================================================
** @Name      : static void Task400ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 400ms任务处理
** @The notes : 400ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task400ms(void *argument)
{
    u32 nextDeadline = osKernelGetTickCount();
    while(1)
    {
        uint32_t start_tick = osKernelGetTickCount();
        //任务处理函数
        /*累计充放电量统计*/
        GroupHisCapStatisTask();

        /*累计充放电能统计*/
        GroupHisEnerStatisTask();
        uint32_t execution_time = osKernelGetTickCount() - start_tick;
        MONITOR_RECORD_ITER(eTaskMon_Task400ms, execution_time);
        nextDeadline += pdMS_TO_TICKS(400);
        if(osKernelGetTickCount() < nextDeadline)
        {
            osDelayUntil(nextDeadline);
        }
        else
        {
            osDelay(1);
            nextDeadline = osKernelGetTickCount() + pdMS_TO_TICKS(400);
        }
    }
}
/*=============================================================================================
** @Name      : static void Task500ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 500ms任务处理
** @The notes : 500ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task500ms(void *argument)
{
    u32 nextDeadline = osKernelGetTickCount();
    while(1)
    {
        uint32_t start_tick = osKernelGetTickCount();
        //任务处理函数
        /*外围故障检测任务*/
        DiagPerDevCheckTask();

        /*快充策略任务*/
        //GroupFstChgTacticsTask();

        /*RTOS诊断计时任务*/
        //OSDiagTimerAddTask();

        /*辅助设备管理任务*/
        TacAuxDevManagTask();

        /*IMIU4采样/告警ICC67/68转发(2026-09-14接入,不受Imeu3En门控)*/
        TacImiu4InfoSync();

        /*消防4级告警控制任务*/
        TacFireAlarmCtrlTask();
        uint32_t execution_time = osKernelGetTickCount() - start_tick;
        MONITOR_RECORD_ITER(eTaskMon_Task500ms, execution_time);
        nextDeadline += pdMS_TO_TICKS(500);
        if(osKernelGetTickCount() < nextDeadline)
        {
            osDelayUntil(nextDeadline);
        }
        else
        {
            osDelay(1);
            nextDeadline = osKernelGetTickCount() + pdMS_TO_TICKS(500);
        }
    }
}
/*=============================================================================================
** @Name      : static void Task1000ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 1000ms任务处理
** @The notes : 1000ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task1000ms(void *argument)
{
    static u8 sTime = 0;
    u32 nextDeadline = osKernelGetTickCount();
    while(1)
    {
        uint32_t start_tick = osKernelGetTickCount();
        //任务处理函数开始
        /*外部Flash检测任务*/
	    #if !defined(CONFIG_XIP_MODE)
        EXTFlashCheckTask();
#endif
        //PCSDelayCheckTask_1S();

        /*外围输出口控制任务*/
        TacGPeriManagTacticTask();

        /*主控系统输入IO口启停控制任务*/
        GroupSysIoUpDnCtrlTask();

        if(0 == sTime)
        {
            /*充放电次数统计任务*/
            GroupChgTimesStatisTask();
            sTime = 1;
        }
        else
        {
            /*充放电剩余时间计算*/
            GroupAllowTimeStatisTask();
            sTime = 0;
        }

        /*放电深度DOD统计记录任务*/
        GroupDodStatisRcdTask();

        //任务处理函数结束
        uint32_t execution_time = osKernelGetTickCount() - start_tick;
        MONITOR_RECORD_ITER(eTaskMon_Task1000ms, execution_time);
        nextDeadline += pdMS_TO_TICKS(1000);
        if(osKernelGetTickCount() < nextDeadline)
        {
            osDelayUntil(nextDeadline);
        }
        else
        {
            osDelay(1);
            nextDeadline = osKernelGetTickCount() + pdMS_TO_TICKS(1000);
        }
    }
}

/*=============================================================================================
** @Name      : static void Task2000ms(void *argument)
** @Input     : void
** @Output    : void
** @Function  : 2000ms任务处理
** @The notes : 2000ms周期任务的回调函数，用户自己实现
**===========================================================================================*/
static void Task2000ms(void *argument)
{
    u32 nextDeadline = osKernelGetTickCount();
    static u8 sState = 0;
    while(1)
    {
        uint32_t start_tick = osKernelGetTickCount();
        //任务处理函数开始
        /*主控系统下电关机保护任务*/
        GroupSysPwrOffProTask();

        /*故障告警次数统计任务*/
        GroupErrNumStatisTask();

        /*Modbus通信检测复位任务*/
        //MbCheckRcvOffResetTask();

        /*系统时间记录任务*/
        SysClockRecordTask();

        /*系统项目信息更新显示任务*/
        //SysDispProjInfoTask();

        /*4s任务*/
        if(0 == sState)
        {
            /*外部RTC检测任务*/
            //DEVEXTRTCCheckTask();
            sState = 1;
        }
        else
        {
            /*外部EEPROM检测任务*/
            DEVEXTEEPCheckTask();
            sState = 0;
        }
        //任务处理函数结束
        uint32_t execution_time = osKernelGetTickCount() - start_tick;
        MONITOR_RECORD_ITER(eTaskMon_Task2000ms, execution_time);
        nextDeadline += pdMS_TO_TICKS(2000);
        if(osKernelGetTickCount() < nextDeadline)
        {
            osDelayUntil(nextDeadline);
        }
        else
        {
            osDelay(1);
            nextDeadline = osKernelGetTickCount() + pdMS_TO_TICKS(2000);
        }
    }

}
