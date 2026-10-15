/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : DEVCAN.h
** @Founder      : ZQD808
** @Date         : 2025.11.28
** @Function     : CAN通讯管理函数模块
** @Instructions :
**===========================================================================================*/
#ifndef __DEVCAN_H__
#define __DEVCAN_H__

#include "BSPTypeDef.h"
#include "GroupInfo.h"
#include "driver/kunlun/can/flexcan_driver.h"
#include "driver/can.h"
#include "BSPConfig.h"

//CAN通道号定义
typedef enum
{
    eCAN0 = 0,
    eCAN1 = 1,
    eCAN2 = 2,
    eCAN3 = 3,
    eCANNUM
}e_CANNUM;
#include <driver/can.h>
#include <driver/kunlun/can/flexcan_driver.h>

#ifndef CONFIG_CLOCK_OVER_DRIVE_MODE
/* for 80MHz  */
#undef BAUDRATE_1M_5M
#define BAUDRATE_1M_5M  .nominal_bit_timing = {   /* 1Mbps, sample point 80% */ \
                            .preDivider = 2U, \
                            .rJumpwidth = 5U, \
                            .propSeg = 10U, \
                            .phaseSeg1 = 21U, \
                            .phaseSeg2 = 8U \
                        }, \
                        .data_bit_timing = {  /* 5Mbps, sample point 75% */ \
                            .preDivider = 2U, /* Should be the same as nominalBitTiming.preDivider. */ \
                            .rJumpwidth = 1U, \
                            .propSeg = 2U, \
                            .phaseSeg1 = 3U, \
                            .phaseSeg2 = 2U \
                        }, \
                        .fd_tdc = 12
#define BAUDRATE_500K_CAN20  .nominal_bit_timing = {   /* 500Kbps, sample point 80% */ \
                            .preDivider = 8U, \
                            .rJumpwidth = 2U, \
                            .propSeg = 7U, \
                            .phaseSeg1 = 8U, \
                            .phaseSeg2 = 4U \
                        }, \
                        .data_bit_timing = {  /* CAN 2.0不使用数据域，但保持相同配置 */ \
                            .preDivider = 8U, \
                            .rJumpwidth = 2U, \
                            .propSeg = 7U, \
                            .phaseSeg1 = 8U, \
                            .phaseSeg2 = 4U \
                        }, \
                        .fd_tdc = 0

#define BAUDRATE_125K_CAN20  .nominal_bit_timing = {   /* 125Kbps, sample point 80% */ \
                            .preDivider = 16U, \
                            .rJumpwidth = 5U, \
                            .propSeg = 10U, \
                            .phaseSeg1 = 21U, \
                            .phaseSeg2 = 8U \
                        }, \
                        .data_bit_timing = {  /* CAN 2.0不使用数据域，但保持相同配置 */ \
                            .preDivider = 16U, \
                            .rJumpwidth = 5U, \
                            .propSeg = 10U, \
                            .phaseSeg1 = 21U, \
                            .phaseSeg2 = 8U \
                        }, \
                        .fd_tdc = 0
#endif

#define CAN_NUM_ON_BOARD 4
#define CONFIG_CAN1
#define CONFIG_CAN2
#define CONFIG_CAN3
#define CONFIG_CAN4

#define CAN20_MB_NUM     (64)
#define CAN20_TX_MB_NUM  (32)
#define CAN20_RX_MB_NUM  (32)


#define CANFD_MB_NUM     (14)
#define CANFD_TX_MB_NUM  (7)
#define CANFD_RX_MB_NUM  (7)

/*=============================================================================================
** @Name      : u8 DEVCANResetInit(e_CANNUM canNum, bool enableIrq)
** @Input     : canNum:CAN通道号 enableIrq:是否使能中断
** @Output    : TRUE:成功 FALSE:失败
** @Function  : CAN口复位重新初始化(设置通信波特率)
** @The notes : 用户调用
**===========================================================================================*/
u8 DEVCANResetInit(e_CANNUM canNum, bool enableIrq);
/*=============================================================================================
** @Name      : void DEVCANEnableAllIRQ(void)
** @Input     : void
** @Output    : void
** @Function  : 使能所有CAN中断
** @The notes : 
**===========================================================================================*/
void DEVCANEnableAllIRQ(void);
/*=============================================================================================
** @Name      : u8 BSPCANSendQin(e_CANNUM canNum, can_frame_t *msg)
** @Input     : canNum:通道号 *msg:发送信息结构体
** @Output    : 发送结果:TRUE-成功 FALSE-失败
** @Function  : CAN发送信息(数据压入队列)
** @The notes : 
**===========================================================================================*/
u8 BSPCANSendQin(e_CANNUM canNum, can_frame_t *msg);

/*=============================================================================================
** @Name      : void BSPCANRcvTask(void *argument)
** @Input     : *p:任务函数格式需要空指针
** @Output    : void
** @Function  : CAN信息接收任务
** @The notes : BSP调用
**===========================================================================================*/
void BSPCAN1RcvTask(void *argument);
/*=============================================================================================
** @Name      : void BSPCAN2RcvTask(void *argument)
** @Input     : *p:任务函数格式需要空指针
** @Output    : void
** @Function  : CAN信息接收任务
** @The notes : BSP调用
**===========================================================================================*/
void BSPCAN2RcvTask(void *argument);
/*=============================================================================================
** @Name      : void BSPCAN3RcvTask(void *argument)
** @Input     : *p:任务函数格式需要空指针
** @Output    : void
** @Function  : CAN信息接收任务
** @The notes : BSP调用
**===========================================================================================*/
void BSPCAN3RcvTask(void *argument);
/*=============================================================================================
** @Name      : void BSPCAN1SendTask(void *argument)
** @Input     : *p:任务函数格式需要空指针
** @Output    : void
** @Function  : CAN1信息发送任务
** @The notes : BSP调用
**===========================================================================================*/
void BSPCAN1SendTask(void *argument);
/*=============================================================================================
** @Name      : void BSPCAN2SendTask(void *argument)
** @Input     : *p:任务函数格式需要空指针
** @Output    : void
** @Function  : CAN2信息发送任务
** @The notes : BSP调用
**===========================================================================================*/
void BSPCAN2SendTask(void *argument);
/*=============================================================================================
** @Name      : void BSPCAN3SendTask(void *argument)
** @Input     : *p:任务函数格式需要空指针
** @Output    : void
** @Function  : CAN2信息发送任务
** @The notes : BSP调用
**===========================================================================================*/
void BSPCAN3SendTask(void *argument);
#endif