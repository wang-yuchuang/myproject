/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : DEVCAN.c
** @Founder      : ZQD808
** @Date         : 2025.11.28
** @Function     : CAN通讯管理函数模块
** @Instructions :
**===========================================================================================*/
#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <driver/irq.h>

#include "CLI.h"
#include "can_cfg.h"
#include "driver/kunlun/can/flexcan_driver.h"
#include "driver/can.h"
#include "irq_num.h"
#include "regs_base.h"
#include "DEVCAN.h"
#include <stdio.h>
#include <debug.h>

#include "regs_base.h"
#include <param.h>
#include "BSPUserHook.h"
#include "BSPConfig.h"

extern volatile u_BSPFunCfg gBSPFunCfg;

static rx_mb_cfg_t flexcan_rx_mb_cfg[7] = {
    /* ID, MASK, RTR, STD */
    {0x0, 0x0, false, false},
    {0x0, 0x0, false, false},
    {0x0, 0x0, false, false},
    {0x0, 0x0, false, false},
    {0x0, 0x0, false, false},
    {0x0, 0x0, false, true},
    {0x0, 0x0, false, true}
};

static tx_mb_cfg_t flexcan_tx_mb_cfg[7] = {
    /* padding val */
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
};

static flexcan_mb_config_t g_flexcan_mb_cfg[CAN_NUM_ON_BOARD] = {
    #ifdef CONFIG_CAN1
    {
        .maxMBNum = CANFD_MB_NUM,
        .rx_mb_count = CANFD_RX_MB_NUM,
        .tx_mb_count = CANFD_TX_MB_NUM,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN2
    {
        .maxMBNum = CANFD_MB_NUM,
        .rx_mb_count = CANFD_RX_MB_NUM,
        .tx_mb_count = CANFD_TX_MB_NUM,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN3
    {
        .maxMBNum = CANFD_MB_NUM,
        .rx_mb_count = CANFD_RX_MB_NUM,
        .tx_mb_count = CANFD_TX_MB_NUM,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN4
    {
        .maxMBNum = CANFD_MB_NUM,
        .rx_mb_count = CANFD_RX_MB_NUM,
        .tx_mb_count = CANFD_TX_MB_NUM,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
};

static flexcan_controller_cfg_t g_flexcan_cfg[CAN_NUM_ON_BOARD] = {
    #ifdef CONFIG_CAN1
    {
        .channel = 0,
        .base = APB_CAN1_BASE,
        .irq = CANFD1_IPI_BUSOFF_INTR_NUM,
        .is_canfd_enabled = false,
        .enableSelfWakeup = true,
        .enableIndividMask = true,
        #if(CAN0_BPS == 125)
        BAUDRATE_125K_CAN20,
        #elif(CAN0_BPS == 500)
        BAUDRATE_500K_CAN20,
        #elif(CAN0_BPS == 5000)
        BAUDRATE_1M_5M,
        #endif
        .buf_cfg = {
            .txfifo_size = 50,
            .rxfifo_size = 100
        }
    },
    #endif
    #ifdef CONFIG_CAN2
    {
        .channel = 1,
        .base = APB_CAN2_BASE,
        .irq = CANFD2_IPI_BUSOFF_INTR_NUM,
        .is_canfd_enabled = true,
        .enableSelfWakeup = true,
        .enableIndividMask = true,
        #if(CAN1_BPS == 125)
        BAUDRATE_125K_CAN20,
        #elif(CAN1_BPS == 500)
        BAUDRATE_500K_CAN20,
        #elif(CAN1_BPS == 5000)
        BAUDRATE_1M_5M,
        #endif
        .buf_cfg = {
            .txfifo_size = 50,
            .rxfifo_size = 100
        }
    },
    #endif
    #ifdef CONFIG_CAN3
    {
        .channel = 2,
        .base = APB_CAN3_BASE,
        .irq = CANFD3_IPI_BUSOFF_INTR_NUM,
        .is_canfd_enabled = true,
        .enableSelfWakeup = true,
        .enableIndividMask = true,
        #if(CAN2_BPS == 125)
        BAUDRATE_125K_CAN20,
        #elif(CAN2_BPS == 500)
        BAUDRATE_500K_CAN20,
        #elif(CAN2_BPS == 5000)
        BAUDRATE_1M_5M,
        #endif
        .buf_cfg = {
            .txfifo_size = 50,
            .rxfifo_size = 100
        }
    },
    #endif
    #ifdef CONFIG_CAN4
    {
        .channel = 3,
        .base = APB_CAN4_BASE,
        .irq = CANFD4_IPI_BUSOFF_INTR_NUM,
        .is_canfd_enabled = true,
        .enableSelfWakeup = true,
        .enableIndividMask = true,
        #if(CAN3_BPS == 125)
        BAUDRATE_125K_CAN20,
        #elif(CAN3_BPS == 500)
        BAUDRATE_500K_CAN20,
        #elif(CAN3_BPS == 5000)
        BAUDRATE_1M_5M,
        #endif
        .buf_cfg = {
            .txfifo_size = 50,
            .rxfifo_size = 100
        }
    },
    #endif
};
can_device_t *can_dev0;
can_device_t *can_dev1;
can_device_t *can_dev2;
can_device_t *can_dev3;
/*=============================================================================================
** @Name      : u8 DEVCANResetInit(e_CANNUM canNum, bool enableIrq)
** @Input     : canNum:CAN通道号
** @Input     : enableIrq:是否使能中断 TRUE:使能 FALSE:关闭
** @Output    : TRUE:成功 FALSE:失败
** @Function  : CAN口复位重新初始化(设置通信波特率)
** @The notes : 用户调用
**===========================================================================================*/
u8 DEVCANResetInit(e_CANNUM canNum, bool enableIrq)
{
    u8 result = FALSE;
    switch(canNum)
    {
        case eCAN0:
        flexcan_probe(&g_flexcan_cfg[0], &g_flexcan_mb_cfg[0]);
        can_dev0 = can_get_probe_device(eCAN0);    
        if(!enableIrq && can_dev0 != NULL)
        {
            irq_disable(can_dev0->irq_nr);
        }
        if(can_dev0 == NULL) 
        {
            ssdk_printf(SSDK_WARNING, "%s: can_dev0 is null!!!\r\n", __func__);
            result = FALSE;
        }
        else
        {
            result = TRUE;
            if(enableIrq == TRUE)
            {
                irq_enable(can_dev0->irq_nr);
            }
        }
        break;

        case eCAN1:
        flexcan_probe(&g_flexcan_cfg[1], &g_flexcan_mb_cfg[1]);
        can_dev1 = can_get_probe_device(eCAN1);    
        if(!enableIrq && can_dev1 != NULL)
        {
            irq_disable(can_dev1->irq_nr);
        }
        if(can_dev1 == NULL) 
        {
            ssdk_printf(SSDK_WARNING, "%s: can_dev1 is null!!!\r\n", __func__);
            result = FALSE;
        }
        else
        {
            result = TRUE;
            if(enableIrq == TRUE)
            {
                irq_enable(can_dev1->irq_nr);
            }
        }
        break;

        case eCAN2:
        flexcan_probe(&g_flexcan_cfg[2], &g_flexcan_mb_cfg[2]);
        can_dev2 = can_get_probe_device(eCAN2);    
        if(!enableIrq && can_dev2 != NULL)
        {
            irq_disable(can_dev2->irq_nr);
        }
        if(can_dev2 == NULL) 
        {
            ssdk_printf(SSDK_WARNING, "%s: can_dev2 is null!!!\r\n", __func__);
            result = FALSE;
        }
        else
        {
            result = TRUE;
            if(enableIrq == TRUE)
            {
                irq_enable(can_dev2->irq_nr);
            }
        }
        break;

        case eCAN3:
        flexcan_probe(&g_flexcan_cfg[3], &g_flexcan_mb_cfg[3]);
        can_dev3 = can_get_probe_device(eCAN3);    
        if(!enableIrq && can_dev3 != NULL)
        {
            irq_disable(can_dev3->irq_nr);
        }
        if(can_dev3 == NULL) 
        {
            ssdk_printf(SSDK_WARNING, "%s: can_dev3 is null!!!\r\n", __func__);
            result = FALSE;
        }
        else
        {
            result = TRUE;
            if(enableIrq == TRUE)
            {
                irq_enable(can_dev3->irq_nr);
            }
        }
        break;

        default:
        return FALSE;
        break;
    }
    return (result);
}
/*=============================================================================================
** @Name      : void DEVCANEnableAllIRQ(void)
** @Input     : void
** @Output    : void
** @Function  : 使能所有已注册 CAN 通道的中断（启动阶段使用）
** @The notes : 必须在 CAN Recv 任务创建后调用
**==============================================================================================*/
void DEVCANEnableAllIRQ(void)
{
    if(can_dev0 != NULL)
    {
        irq_enable(can_dev0->irq_nr);
    }
    if(can_dev1 != NULL)
    {
        irq_enable(can_dev1->irq_nr);
    }
    if(can_dev2 != NULL)
    {
        irq_enable(can_dev2->irq_nr);
    }
    if(can_dev3 != NULL)
    {
        irq_enable(can_dev3->irq_nr);
    }
    ssdk_printf(SSDK_INFO, "DEVCANEnableAllIRQ: all CAN IRQ enabled\r\n");
}
/*=============================================================================================
** @Name      : u8 BSPCANSendQin(e_CANNUM canNum, can_frame_t *msg)
** @Input     : canNum:通道号 *msg:发送信息结构体
** @Output    : 发送结果:TRUE-成功 FALSE-失败
** @Function  : CAN发送信息(数据压入队列)
** @The notes : 
**===========================================================================================*/
u8 BSPCANSendQin(e_CANNUM canNum, can_frame_t *msg)
{
    u8 result = 0;
    //static u8 Can0MbIndex = 0;
    //static u8 Can1MbIndex = 0;
    //static u8 Can2MbIndex = 0;
    /*模块未使能不进行初始化*/
    if(((eCAN0 == canNum) && (1 != gBSPFunCfg.Bits.CAN0En))
		|| ((eCAN1 == canNum) && (1 != gBSPFunCfg.Bits.CAN1En))
		|| ((eCAN2 == canNum) && (1 != gBSPFunCfg.Bits.CAN2En)))
    {
        return(FALSE);
    }
    
    if(canNum >= eCANNUM)
    {
        return(FALSE);
    }
    // if((eCAN0 == canNum) && (can_dev0 != NULL))
    // {
    //     result = can_write(can_dev0, msg, 0);
    //     if (result)
    //     {
    //         ssdk_printf(SSDK_WARNING, "%s: can_dev0 write fail!!!\r\n", __func__);
    //         return FALSE;
    //     }
    //     //if(Can0MbIndex < CANFD_MB_NUM - 1)
    //     //    Can0MbIndex++;
    //     //else
    //     //    Can0MbIndex = 0;
    // }
    else if((eCAN1 == canNum) && (can_dev1 != NULL))
    {
        result = can_write(can_dev1, msg, 1);
        if (result)
        {
            ssdk_printf(SSDK_DEBUG, "%s: can_dev1 write fail!!!\r\n", __func__);
            return FALSE;
        }
        //if(Can1MbIndex < CANFD_MB_NUM - 1)
        //    Can1MbIndex++;
        //else
        //    Can1MbIndex = 0;
    }
    else if((eCAN2 == canNum) && (can_dev2 != NULL))
    {
        result = can_write(can_dev2, msg, 2);
        if (result)
        {
            ssdk_printf(SSDK_WARNING, "%s: can_dev2 write fail!!!\r\n", __func__);
            return FALSE;
        }
        //if(Can2MbIndex < CANFD_MB_NUM - 1)
        //    Can2MbIndex++;
        //else
        //    Can2MbIndex = 0;
    }
    return TRUE;
}
#if(0)
/*=============================================================================================
** @Name      : void BSPCAN1SendTask(void *argument)
** @Input     : *p:任务函数格式需要空指针
** @Output    : void
** @Function  : CAN1信息发送任务
** @The notes : BSP调用
**===========================================================================================*/
void BSPCAN1SendTask(void *argument)
{
    while(1)
    {
        if(can_dev0 != NULL)
        {
            can_transmit_buf(can_dev0);
        }
        osDelay(1);
    }
}

/*=============================================================================================
** @Name      : void BSPCAN1RcvTask(void *argument)
** @Input     : *p:任务函数格式需要空指针
** @Output    : void
** @Function  : CAN1信息接收任务
** @The notes : BSP调用
**===========================================================================================*/
void BSPCAN1RcvTask(void *argument)
{
    struct can_frame frame;
    int ret = 0;
    while(1)
    {
        if(can_dev0 != NULL)
        {
            ret = can_receive_buf(can_dev0, &frame);
            if (!ret) {
                CANRcvMsgHandle_Hook(eCAN0,&frame);
            }
        }
    }
}
#endif
/*=============================================================================================
** @Name      : void BSPCAN2SendTask(void *argument)
** @Input     : *p:任务函数格式需要空指针
** @Output    : void
** @Function  : CAN2信息发送任务
** @The notes : BSP调用
**===========================================================================================*/
void BSPCAN2SendTask(void *argument)
{
    while(1)
    {
        if(can_dev1 != NULL)
        {
            can_transmit_buf(can_dev1);
        }
        osDelay(1);
    }
}
/*=============================================================================================
** @Name      : void BSPCAN2RcvTask(void *argument)
** @Input     : *p:任务函数格式需要空指针
** @Output    : void
** @Function  : CAN信息接收任务
** @The notes : BSP调用
**===========================================================================================*/
void BSPCAN2RcvTask(void *argument)
{
    struct can_frame frame;
    int ret = 0;
    while(1)
    {
        if(can_dev1 != NULL)
        {
            ret = can_receive_buf(can_dev1, &frame);
            if (!ret) {
                CANRcvMsgHandle_Hook(eCAN1,&frame);
            }
        }
        osDelay(1);
    }
}
/*=============================================================================================
** @Name      : void BSPCAN3SendTask(void *argument)
** @Input     : *p:任务函数格式需要空指针
** @Output    : void
** @Function  : CAN2信息发送任务
** @The notes : BSP调用
**===========================================================================================*/
void BSPCAN3SendTask(void *argument)
{
    while(1)
    {
        if(can_dev2 != NULL)
        {
            can_transmit_buf(can_dev2);
        }
        osDelay(1);
    }
}
/*=============================================================================================
** @Name      : void BSPCAN3RcvTask(void *argument)
** @Input     : *p:任务函数格式需要空指针
** @Output    : void
** @Function  : CAN信息接收任务
** @The notes : BSP调用
**===========================================================================================*/
void BSPCAN3RcvTask(void *argument)
{
    struct can_frame frame;
    int ret = 0;
    while(1)
    {
        if(can_dev2 != NULL)
        {
            ret = can_receive_buf(can_dev2, &frame);
            if (!ret) {
                CANRcvMsgHandle_Hook(eCAN2,&frame);
            }
        }
        osDelay(1);
    }
}
#if(0) //测试使用
static int can_test_write(int argc, char *argv[])
{
    uint8_t Data[64] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                        26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
                        39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
                        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};

    struct can_frame frame;
    int can_id, mb_idx;
    u8 ret = 0;
    can_device_t *can_dev;

    if (argc != 3) {
        ssdk_printf(SSDK_INFO, "\nargc must be 3");
        ssdk_printf(SSDK_INFO, "\t\t:can_test write [can_id] [id] [fd]\n");
        return 0;
    }

    can_id = strtoul(argv[0], NULL, 0);
    frame.id = strtoul(argv[1], NULL, 0);
    frame.fd = strtoul(argv[2], NULL, 0);

    if (frame.fd)
        frame.len = 64;
    else
        frame.len = 8;

    if (frame.id > 0x7FF) {
        frame.ide = CAN_EXTID; // 自动切换为 29 位扩展帧
    } else {
        frame.ide = CAN_STDID; // 自动切换为 11 位标准帧
    }
    frame.rtr = CAN_DTR;
    memcpy(&frame.data, &Data, frame.len);
    ret = BSPCANSendQin(can_id,&frame);
    if(!ret)
    {
        ssdk_printf(SSDK_WARNING, "%s:  %d BSPCANSendQin write fail!!!\r\n", __func__,can_id);
    }
    return 0;
}
static int devcan_test_main(int argc, char *argv[])
{
    if (argc == 0) {
        return 0;
    }

    if (!strcmp(argv[0], "write")) {
        can_test_write(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[0], "init")) {
        //DEVCANResetInit(eCAN0, TRUE);
        DEVCANResetInit(eCAN1, TRUE);
        DEVCANResetInit(eCAN2, TRUE);
        //DEVCANResetInit(eCAN3, TRUE);
    }
    return CLI_FALSE;
}
CLI_CMD("devcan_test", "add board test for can", devcan_test_main);
#endif



