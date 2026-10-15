/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IMEU3Protocol.h
** @Founder      : ZQD808
** @Date         : 2026.04.15
** @Function     : IMEU3 CAN通信协议定义
** @Instructions : 定义CAN帧ID结构宏、功能码枚举、主索引枚举
**===============================================================================================*/
#ifndef __IMEU3PROTOCOL_H_
#define __IMEU3PROTOCOL_H_

#include "BSPTypeDef.h"

/*CAN帧ID构建宏: 29位扩展帧ID = Func(4位) + DP(1位) + Index(8位) + DAddr(8位) + SAddr(8位)*/
#define IMEU3_BUILD_ID(Func, DP, Index, DAddr, SAddr) (((u32)(Func) << 25) | ((u32)(DP) << 24) | ((u32)(Index) << 16) | ((u32)(DAddr) << 8) | (u32)(SAddr))

/*CAN帧ID解析宏: 从29位ID中提取各域*/
#define IMEU3_GET_FUNC(ID)  (((u32)(ID) >> 25) & 0x0fu)   /*获取Func域: 功能码(4位)*/
#define IMEU3_GET_DP(ID)    (((u32)(ID) >> 24) & 0x01u)   /*获取DP域: 数据页标志*/
#define IMEU3_GET_INDEX(ID) (((u32)(ID) >> 16) & 0xffu)   /*获取Index域: 主索引*/
#define IMEU3_GET_DADDR(ID) (((u32)(ID) >> 8)  & 0xffu)   /*获取DAddr域: 目标地址*/
#define IMEU3_GET_SADDR(ID) ((u32)(ID) & 0xffu)           /*获取SAddr域: 源地址*/

/*IMEU3功能码枚举定义*/
typedef enum
{
    eIMEU3Func_TimeFrame      = 0,     /*对时帧*/
    eIMEU3Func_Heartbeat      = 1,     /*心跳帧*/
    eIMEU3Func_RequestModify  = 2,     /*请求修改*/
    eIMEU3Func_ModifyReply    = 3,     /*修改回复*/
    eIMEU3Func_ModifyConfirm  = 4,     /*修改确认*/
    eIMEU3Func_RequestQuery   = 5,     /*请求查询*/
    eIMEU3Func_QueryReply     = 6,     /*查询回复*/
    eIMEU3Func_QueryConfirm   = 7,     /*查询确认*/
    eIMEU3Func_ActiveUpdate   = 8,     /*主动更新*/
    eIMEU3Func_ErrorResponse  = 9,     /*错误应答*/
    eIMEU3Func_Reserved10     = 10,    /*预留*/
    eIMEU3Func_Reserved11     = 11,    /*预留*/
    eIMEU3Func_Reserved12     = 12,    /*预留*/
    eIMEU3Func_Reserved13     = 13,    /*预留*/
    eIMEU3Func_Reserved14     = 14,    /*预留*/
    eIMEU3Func_Reserved15     = 15,    /*预留*/
}e_IMEU3FuncCode;

/*IMEU3主索引枚举定义*/
typedef enum
{
    eIMEU3Idx_Time           = 0,      /*对时信息*/
    eIMEU3Idx_DeviceInfo     = 1,      /*设备信息*/
    eIMEU3Idx_DevSerial      = 2,      /*单板序列号 */
    eIMEU3Idx_SystemStatus   = 50,     /*系统状态*/
    eIMEU3Idx_BoardInfo1     = 51,     /*板级信息1: NTC温度、485传感器*/
    eIMEU3Idx_BoardInfo2     = 52,     /*板级信息2: DI/DO状态、气体浓度*/
    eIMEU3Idx_AlarmInfo      = 53,     /*告警信息*/
    eIMEU3Idx_Control        = 86,     /*控制命令*/
    eIMEU3Idx_QueryPara      = 102,    /*查询参数(功能码5/6)*/
    eIMEU3Idx_ModifyPara     = 110,    /*修改参数(功能码2/3)*/
}e_IMEU3MainIndex;

#endif