/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : DEVUART.c
** @Founder      : ZQD808
** @Date         : 2025.12.01
** @Function     : RS485通讯管理函数模块
** @Instructions :
**===========================================================================================*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>

#include <board.h>
#include <reg.h>
#include <param.h>
#include <CLI_type.h>
#include <driver/kunlun/uart/dw_uart_drv.h>
#include <regs_base.h>
#include <driver/uart.h>
#include "DEVUART.h"
#include "BSPConfig.h"
#include "BSPUserHook.h"

#include <cmsis_os2.h>

#if(0)
extern volatile u_BSPFunCfg gBSPFunCfg;

static dw_uart_dev_t sdrv_uart5;
static uart_dev_t *dev_uart5;

static dw_uart_dev_t sdrv_uart9;
static uart_dev_t *dev_uart9;


static osMessageQueueId_t uart5_msg_queue = NULL;  //串口5发送消息队列
static osMessageQueueId_t uart9_msg_queue = NULL;  //串口9发送消息队列
static osMutexId_t uart5_send_mutex = NULL;        //串口5发送互斥锁
static osMutexId_t uart9_send_mutex = NULL;        //串口9发送互斥锁
/* uart config structures */
static uart_config_t uart5_config = {
    .base = DEVICE_BASE(UART5),
    .irq = DEVICE_INTR(UART5),
    .baud = 9600,
    .clk_freq = UART_SAF_CLK,
    .data_bits = UART_CHAR_8_BITS,
    .stop_bits = UART_STOP_1_BIT,
    .parity = UART_NO_PARITY,
};
/* uart config structures */
static uart_config_t uart9_config = {
    .base = DEVICE_BASE(UART9),
    .irq = DEVICE_INTR(UART9),
    .baud = 9600,
    .clk_freq = UART_SEC0_CLK,
    .data_bits = UART_CHAR_8_BITS,
    .stop_bits = UART_STOP_1_BIT,
    .parity = UART_NO_PARITY,
};
/*=============================================================================================
** @Name      : u8 DEVUARTResetInit(e_UARTNUM UartNum,u32 Baudrate)
** @Input     : UartNum:SCI通道号
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 串口复位重新初始化
** @The notes : 用户调用
**===========================================================================================*/
u8 DEVUARTResetInit(e_UARTNUM UartNum,u32 Baudrate)
{
    u8 result = FALSE;
    switch(UartNum)
    {
        case eUART5:
            uart5_config.baud = Baudrate;
            dev_uart5 = dw_uart_probe(&sdrv_uart5, &uart5_config);
            if(dev_uart5 != NULL)
            {
                uart_init(dev_uart5);
                result = TRUE;
            }
            else
            {
                ssdk_printf(SSDK_WARNING, "%s: uart5 dw_uart_probe is null!!!\r\n", __func__);
                result = FALSE;
            }
            break;
        case eUART9:
            // Ensure SEC clock gate is enabled for UART9 (id=0x2e)
            // Sometimes the board-level clock init misses this, causing UART9 bus access to fail
            {
                volatile uint32_t *gating = (volatile uint32_t *)(0xF8000000 + (((0x400u + 0x2eu * 4u) << 10)));
                uint32_t gv = *gating;
                gv &= ~(1u << 31);   // unlock
                *gating = gv;
                gv &= ~(1u << 1);    // clear SW_DIS (enable clock)
                *gating = gv;
                gv |= (1u << 31);    // lock
                *gating = gv;
            }
            uart9_config.baud = Baudrate;
            dev_uart9 = dw_uart_probe(&sdrv_uart9, &uart9_config);
            if (dev_uart9 != NULL)
            {
                uart_init(dev_uart9);
                result = TRUE;
            }
            else
            {
                ssdk_printf(SSDK_WARNING, "%s: uart9 dw_uart_probe is null!!!\r\n", __func__);
                result = FALSE;
            }
            break;
        default:
        result = FALSE;
        break;
    }
    return (result);
}
/*=============================================================================================
** @Name      : u8 DEVUARTSendInit(void)
** @Input     : void
** @Output    : 0:失败 1：成功
** @Function  : 串口发送初始化
** @The notes : 创建消息队列与互斥锁
**===========================================================================================*/
u8 DEVUARTSendBufInit(void)
{
    static u8 initialized = 0;
    
    if(initialized)
    {
        return TRUE;
    }
    
    uart5_msg_queue = osMessageQueueNew(UART_QUEUE_COUNT, sizeof(uart_message_t), NULL);
    if(uart5_msg_queue == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: uart5_msg_queue create fail!!!\r\n", __func__);
        return FALSE;
    }
    
    uart9_msg_queue = osMessageQueueNew(UART_QUEUE_COUNT, sizeof(uart_message_t), NULL);
    if(uart9_msg_queue == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: uart9_msg_queue create fail!!!\r\n", __func__);
        osMessageQueueDelete(uart5_msg_queue);
        uart5_msg_queue = NULL;
        return FALSE;
    }
    
    uart5_send_mutex = osMutexNew(NULL);
    if(uart5_send_mutex == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: uart5_send_mutex create fail!!!\r\n", __func__);
        osMessageQueueDelete(uart5_msg_queue);
        osMessageQueueDelete(uart9_msg_queue);
        uart5_msg_queue = NULL;
        uart9_msg_queue = NULL;
        return FALSE;
    }
    
    uart9_send_mutex = osMutexNew(NULL);
    if(uart9_send_mutex == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: uart9_send_mutex create fail!!!\r\n", __func__);
        osMessageQueueDelete(uart5_msg_queue);
        osMessageQueueDelete(uart9_msg_queue);
        osMutexDelete(uart5_send_mutex);
        uart5_msg_queue = NULL;
        uart9_msg_queue = NULL;
        uart5_send_mutex = NULL;
        return FALSE;
    }
    
    initialized = 1;
    return TRUE;
}
/*=============================================================================================
** @Name      : u8 DEVUARTSendDataOnAsk(e_UARTNUM UartNum, u8 *sendbuf, u8 size)
** @Input     : sciNum:SCI通道号 *sendbuf:发送的数据 size:数据长度
** @Output    : TRUE:成功 FALSE:失败
** @Function  : RS485询问方式发送串口数据
** @The notes : 直接发送,任务调用
**===========================================================================================*/
u8 DEVUARTSendDataOnAsk(e_UARTNUM UartNum, u8 *sendbuf, u8 size)
{
    size_t sent = 0;
    size_t total = (size_t)size;

    if((UartNum != eUART5) && (UartNum != eUART9))
        return FALSE;
    if((sendbuf == NULL) || (size == 0))
        return FALSE;
    if(UartNum == eUART5)
    {
        if(dev_uart5 != NULL)
        {
            if(uart5_send_mutex != NULL) osMutexAcquire(uart5_send_mutex, osWaitForever);
            while(sent < total)
            {
                size_t ret = dev_uart5->ops->transmit_async(dev_uart5, (char *)sendbuf + sent, total - sent);
                if(ret == 0)
                {
                    osDelay(1);
                    continue;
                }
                sent += ret;
            }
            if(uart5_send_mutex != NULL) osMutexRelease(uart5_send_mutex);
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if(UartNum == eUART9)
    {
        if(dev_uart9 != NULL)
        {
            if(uart9_send_mutex != NULL) osMutexAcquire(uart9_send_mutex, osWaitForever);
            while(sent < total)
            {
                size_t ret = dev_uart9->ops->transmit_async(dev_uart9, (char *)sendbuf + sent, total - sent);
                if(ret == 0)
                {
                    osDelay(1);
                    continue;
                }
                sent += ret;
            }
            if(uart9_send_mutex != NULL) osMutexRelease(uart9_send_mutex);
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    return TRUE;
}
/*=============================================================================================
** @Name      : u8 DEVUARTSendDataToBuf(e_UARTNUM UartNum, u8 *sendbuf, u8 size,u8 Prio)
** @Input     : sciNum:SCI通道号 *sendbuf:发送的数据 size:数据长度 Prio:优先级：数字越大，优先级越高
** @Output    : TRUE:成功 FALSE:失败
** @Function  : RS485询问方式发送串口数据到缓冲区
** @The notes : 直接发送,用户调用
**===========================================================================================*/
u8 DEVUARTSendDataToBuf(e_UARTNUM UartNum, u8 *sendbuf, u8 size, u8 Prio)
{
    uart_message_t uart_msg;
    osStatus_t status;
    osMessageQueueId_t msg_queue;
    osMutexId_t send_mutex;
    
    if((UartNum != eUART5) && (UartNum != eUART9))
        return FALSE;
    if((sendbuf == NULL) || (size == 0))
        return FALSE;
    if(size > UART_MSG_DATA_LEN)
    {
        ssdk_printf(SSDK_WARNING, "%s: size %d > max %d!!!\r\n", __func__, size, UART_MSG_DATA_LEN);
        return FALSE;
    }
    
    if(UartNum == eUART5)
    {
        msg_queue = uart5_msg_queue;
        send_mutex = uart5_send_mutex;
    }
    else
    {
        msg_queue = uart9_msg_queue;
        send_mutex = uart9_send_mutex;
    }
    
    if((msg_queue == NULL) || (send_mutex == NULL))
    {
        ssdk_printf(SSDK_WARNING, "%s: queue or mutex is null!!!\r\n", __func__);
        return FALSE;
    }
    
    uint32_t available = osMessageQueueGetSpace(msg_queue);
    if(available == 0)
    {
        ssdk_printf(SSDK_WARNING, "%s: queue is full!\r\n", __func__);
        return FALSE;
    }
    
    status = osMutexAcquire(send_mutex, 100);
    if(status != osOK)
    {
        ssdk_printf(SSDK_WARNING, "%s: mutex acquire timeout!\r\n", __func__);
        return FALSE;
    }
    
    memcpy(uart_msg.data, sendbuf, size);
    uart_msg.length = size;
    
    status = osMessageQueuePut(msg_queue, &uart_msg, Prio, 50);
    osMutexRelease(send_mutex);
    
    if(status != osOK)
    {
        ssdk_printf(SSDK_WARNING, "%s: queue put fail!\r\n", __func__);
        return FALSE;
    }
    
    return TRUE;
}
/*=============================================================================================
** @Name      : void UartSendTask(void *argument)
** @Input     : *argument:形参,任务函数参数需要
** @Output    : void
** @Function  : 串口发送任务
** @The notes : 
**===========================================================================================*/
void UartSendTask(void *argument)
{
    uart_message_t uart5_msg;
    uart_message_t uart9_msg;
    while(1)
    {
        if(uart5_msg_queue != NULL)
        {
            if(osMessageQueueGet(uart5_msg_queue, &uart5_msg, NULL, 0) == osOK)
            {
                DEVUARTSendDataOnAsk(eUART5,uart5_msg.data, uart5_msg.length);
                osDelay(1);
            }
            else
            {
                osDelay(10);
            } 
        }
        if(uart9_msg_queue != NULL)
        {
            if(osMessageQueueGet(uart9_msg_queue, &uart9_msg, NULL, 0) == osOK)
            {
                DEVUARTSendDataOnAsk(eUART9,uart9_msg.data, uart9_msg.length);
                osDelay(1);
            }
            else
            {
                osDelay(10);
            } 
        }
    }
}
static void uart5_recv_callback(void *arg)
{
    static u8 _dbg_once = 1;
    char *recv_data = (char *)arg;
    if(_dbg_once)
    {
        _dbg_once = 0;
        printf("[U5-CB] arg=%p recv_data=%p val=0x%02X\r\n", arg, recv_data, recv_data ? *recv_data : 0);
    }
    if(recv_data != NULL)
    {
        UartRcvMsgHandle_Hook(eUART5, *recv_data);
    }
}
static void uart9_recv_callback(void *arg)
{
    char *recv_data = (char *)arg;
    if(recv_data != NULL)
    {
        UartRcvMsgHandle_Hook(eUART9, *recv_data);
    }
}
/*=============================================================================================
** @Name      : void UartRecvTask(void *argument)
** @Input     : *argument:形参,任务函数参数需要
** @Output    : void
** @Function  : 串口接收任务
** @The notes : 
**===========================================================================================*/
void UartRecvTask(void *argument)
{
    char uart5_recv_data;
    char uart9_recv_data;
    
    while(1)
    {
        if(dev_uart5 != NULL)
        {
            uart_receive_async(dev_uart5, &uart5_recv_data, 1, uart5_recv_callback);
        }
        if(dev_uart9 != NULL)
        {
            uart_receive_async(dev_uart9, &uart9_recv_data, 1, uart9_recv_callback);
        }
        osDelay(1);
    }
}
#if(1) // 开启桌面双 485 交叉闭环测试工具

/*=============================================================================================
** @Name      : static int devuart_test(int argc, char *argv[])
** @Input     : argc: 参数个数, argv: 参数字符串数组
** @Output    : CLI_FALSE
** @Function  : 完整版串口底层通路双向交叉验证工具
** @Instructions : 
** 1. devuart_test init          -> 初始化两个串口到 9600 波特率
** 2. devuart_test send5         -> UART5 单向发送，测试 UART9 是否能接收
** 3. devuart_test send9         -> UART9 单向发送，测试 UART5 是否能接收
**===========================================================================================*/
static int devuart_test(int argc, char *argv[])
{
    if (argc == 0 || argv[0] == NULL) {
        printf("\r\n[错误] 参数为空！\r\n");
        return CLI_FALSE;
    }

    // 1. 初始化通道
    if (!strcmp(argv[0], "init")) {
        printf("\r\n--- 正在重新初始化 UART5 和 UART9 (9600bps) ---\r\n");
        u8 ret5 = DEVUARTResetInit(eUART5, 9600);
        u8 ret9 = DEVUARTResetInit(eUART9, 9600);
        printf("[Init] UART5 初始化结果: %s\r\n", ret5 ? "成功" : "失败");
        printf("[Init] UART9 初始化结果: %s\r\n", ret9 ? "成功" : "失败");
    }
    
    // 2. UART5 单向发起发送 -> 期待 UART9 接收
    else if (!strcmp(argv[0], "send5")) {
        u8 sendbuff[] = "Hello UART9! I am UART5.";
        u8 len = (u8)strlen((char *)sendbuff);
        
        printf("\r\n--- [TEST] UART5 -> UART5_485 -> (总线) -> UART9_485 -> UART9 ---\r\n");
        printf("[TX] UART5 正在尝试发送: \"%s\" (len: %d)\r\n", sendbuff, len);
        
        u8 ret = DEVUARTSendDataToBuf(eUART5, sendbuff, len, 1);
        printf("[TX] 压入 OS 发送队列结果: %d (1:成功, 0:队列满/锁死)\r\n", ret);
    }
    
    // 3. UART9 单向发起发送 -> 期待 UART5 接收
    else if (!strcmp(argv[0], "send9")) {
        u8 sendbuff[] = "Hello UART5! I am UART9.";
        u8 len = (u8)strlen((char *)sendbuff);
        
        printf("\r\n--- [TEST] UART9 -> UART9_485 -> (总线) -> UART5_485 -> UART5 ---\r\n");
        printf("[TX] UART9 正在尝试发送: \"%s\" (len: %d)\r\n", sendbuff, len);
        
        u8 ret = DEVUARTSendDataToBuf(eUART9, sendbuff, len, 1);
        printf("[TX] 压入 OS 发送队列结果: %d (1:成功, 0:队列满/锁死)\r\n", ret);
    }
    
    // 4. 打印帮助菜单
    else {
        printf("\r\n[错误] 未知命令: %s\r\n", argv[0]);
        printf("可用命令格式:\r\n");
        printf("  devuart_test init     : 初始化物理串口\r\n");
        printf("  devuart_test send5    : UART5单向发, 观察UART9是否打印接收\r\n");
        printf("  devuart_test send9    : UART9单向发, 观察UART5是否打印接收\r\n");
    }

    return CLI_FALSE;
}

// 注册命令到 FreeRTOS CLI
CLI_CMD("devuart_test", "芯驰UART底层通路硬核验证工具\n\r", devuart_test); 

#endif

#endif





