/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : BSPUserHook.c
** @Founder      : ZQD808
** @Date         : 2025.12.04
** @Function     : BSP用户服务函数模块
** @Instructions : 用户完善
**===========================================================================================*/
#include "BSPUserHook.h"
#include "SlaveMsg.h"
#include "DEVCAN.h"
#include "AuxDevMsg.h"
#include "IMEU3Interface.h"
#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include "AuxDevTactic.h"
#include "DiagCheck.h"
#include "can_uds_interface.h"
/*=============================================================================================
** @Name      : void CANRcvMsgHandle_Hook(e_CANNUM CANNum, can_frame_t *pWriteMsg)
** @Input     : CANNum:CAN通道 pWriteMsg:CAN报文信息结构指针
** @Output    : void
** @Function  : CAN接收信息处理接口
** @The notes : 该函数已经在读CAN接收缓冲区调用
**===========================================================================================*/
void CANRcvMsgHandle_Hook(e_CANNUM CANNum, can_frame_t *pWriteMsg)
{
    uds_can_route_frame(CANNum, pWriteMsg);

    //接收PCS数据处理
    if(CANNum == eCAN0)
    {
        //CanRcvModuleMsgHandle_API(pWriteMsg);模块相关功能迁移至A核处理

    }
    //接收BSU从控信息处理
    else if(CANNum == eCAN1)
    {
        //BMU接收自研从控BSU信息
        #if((1 == BMU_BSU_COM_TYPE) && (0 == BMU_BSU_CAN_TYPE))
        CanRcvSlaveMsgHandleAPI(pWriteMsg->id,pWriteMsg->data,pWriteMsg->len,CANNum);
        #endif
    }
    //接收IMIU4和IMEU3信息处理
    else if(CANNum == eCAN2)
    {
        IMEU3_RcvHandle(pWriteMsg->id,pWriteMsg->data,pWriteMsg->len,CANNum);
        CanRcvImiu4MsgHandle_API(pWriteMsg->id,pWriteMsg->data,pWriteMsg->len,CANNum);
    }
    return;
}

