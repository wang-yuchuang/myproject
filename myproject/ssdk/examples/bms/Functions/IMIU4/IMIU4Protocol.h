/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IMIU4Protocol.h
** @Founder      : ZQD808
** @Date         : 2026.04.15
** @Function     : IMIU4 CANFD通信协议定义
** @Instructions : 定义CAN帧ID结构宏、功能码枚举、主索引枚举
**                 IMIU4是BMU下级设备(电流测量单元),支持最多2台IMIU4(地址20/21)
**===============================================================================================*/
#ifndef __IMIU4PROTOCOL_H_
#define __IMIU4PROTOCOL_H_

#include "BSPTypeDef.h"

/*=================================================================================================
** CAN帧ID构建宏: 29位扩展帧ID = PR(4位) + DP(1位) + PF(8位) + PS(8位) + SA(8位)
** PR: 功能码(bit28-25)
** DP: 数据页标志(0:外网/上行, 1:内网/下行)
** PF: 主索引(PDU格式字段)
** PS: 目标地址(PDU特定字段)
** SA: 源地址
**===============================================================================================*/
/*CAN帧ID构建宏: 组合各字段生成29位扩展帧ID*/
#define IMIU4_BUILD_ID(PR, DP, PF, PS, SA) (((u32)(PR) << 25) | ((u32)(DP) << 24) | ((u32)(PF) << 16) | ((u32)(PS) << 8) | (u32)(SA))

/*CAN帧ID解析宏: 从29位ID中提取各域*/
#define IMIU4_GET_PR(ID)    (((u32)(ID) >> 25) & 0x0fu)   /*获取PR域: 功能码(bit28-25, 4位)*/
#define IMIU4_GET_DP(ID)    (((u32)(ID) >> 24) & 0x01u)   /*获取DP域: 数据页标志(bit24)*/
#define IMIU4_GET_PF(ID)    (((u32)(ID) >> 16) & 0xffu)   /*获取PF域: 主索引(bit16-23)*/
#define IMIU4_GET_PS(ID)    (((u32)(ID) >> 8)  & 0xffu)   /*获取PS域: 目标地址(bit8-15)*/
#define IMIU4_GET_SA(ID)    ((u32)(ID) & 0xffu)           /*获取SA域: 源地址(bit0-7)*/

/*=================================================================================================
** IMIU4功能码枚举定义
** 功能码用于标识CAN帧的类型和传输方式
**===============================================================================================*/
typedef enum
{
    eIMIU4Func_TimeFrame      = 0,     /*对时帧: 用于时间同步*/
    eIMIU4Func_Heartbeat      = 1,     /*心跳帧: 用于设备在线状态检测*/
    eIMIU4Func_RequestModify  = 2,     /*请求修改: BMU向IMIU4发送控制命令*/
    eIMIU4Func_ModifyReply    = 3,     /*修改回复: IMIU4回复修改结果*/
    eIMIU4Func_ModifyConfirm  = 4,     /*修改确认: BMU确认修改结果*/
    eIMIU4Func_RequestQuery   = 5,     /*请求查询: BMU查询IMIU4数据*/
    eIMIU4Func_QueryReply     = 6,     /*查询回复: IMIU4回复查询数据*/
    eIMIU4Func_QueryConfirm   = 7,     /*查询确认: BMU确认查询结果*/
    eIMIU4Func_ActiveUpdate   = 8,     /*主动更新: IMIU4主动上报数据(1s周期)*/
    eIMIU4Func_ErrorResponse  = 9,     /*错误应答: IMIU4回复错误信息*/
    eIMIU4Func_Reserved10     = 10,    /*单帧上传功能码10*/
    eIMIU4Func_Reserved11     = 11,    /*预留功能码11*/
    eIMIU4Func_Reserved12     = 12,    /*预留功能码12*/
    eIMIU4Func_Reserved13     = 13,    /*预留功能码13*/
    eIMIU4Func_Reserved14     = 14,    /*预留功能码14*/
    eIMIU4Func_Reserved15     = 15,    /*预留功能码15*/
}e_IMIU4FuncCode;

/*=================================================================================================
** IMIU4主索引枚举定义
** 主索引(PF域)用于标识数据类型
**===============================================================================================*/
typedef enum
{
    eIMIU4Idx_DeviceInfo     = 1,      /*设备信息*/
    eIMIU4Idx_SerNum         = 2,      /*序列号*/
    eIMIU4Idx_SystemStatus   = 50,     /*系统工作模式*/
    eIMIU4Idx_SampInfo       = 51,     /*采样信息: 电压、电流、温度、绝缘阻值等*/
    eIMIU4Idx_SampCalCmd     = 52,     /*采样校准命令*/
    eIMIU4Idx_AuthCtrl       = 53,     /*权限控制: 权限切换命令和状态*/
    eIMIU4Idx_AuthStatus     = 54,     /*权限级别状态*/
    eIMIU4Idx_Control        = 55,     /*控制命令: 继电器控制、绝缘检测控制*/
    eIMIU4Idx_CtrlInfo       = 56,     /*控制命令反馈信息*/
    eIMIU4Idx_AlmFault       = 57,     /*故障告警: 告警信息、故障信息*/
    eIMIU4Idx_SampCalInfo    = 58,     /*采样校准反馈信息*/
    eIMIU4Idx_SysDebug       = 60,     /*系统调试: 调试命令*/
    eIMIU4Idx_SysDebugInfo   = 61,     /*系统调试反馈信息*/
    eIMIU4Idx_ConParaCurr    = 102,    /*常规参数当前值*/
    eIMIU4Idx_ConParaSet     = 107,    /*常规参数设置值*/
}e_IMIU4MainIndex;

/*IMIU4*/
typedef enum
{
    e_IMIU4_1 = 0,                        // 1#IMIMU4
    e_IMIU4_2,                        // 2#IMIMU4  
    e_IMIU4_NUM		        				     
}e_IMIU4NUM;



/*协议子索引列表，IMIU4采样信息*/
typedef enum
{
	eIMIUSampInfo_BatVolt = 0,								    /*电池总压采样值*/
	eIMIUSampInfo_LinkVolt,								        /*Link电压采样值*/
	eIMIUSampInfo_Hall5V,								        /*霍尔5V采样值*/
    eIMIUSampInfo_HallSamp,								        /*霍尔信号采样值*/
    eIMIUSampInfo_HallCurrL,								    /*霍尔电流低*/
    eIMIUSampInfo_HallCurrH,								    /*霍尔电流高*/
    eIMIUSampInfo_ShuntCurrL,								    /*分流器电流低*/
    eIMIUSampInfo_ShuntCurrH,								    /*分流器电流高*/
    eIMIUSampInfo_ShuntT,								        /*分流器温度*/
    eIMIUSampInfo_AFE_5V1,								        /*高压采集芯片+5V回检*/
    eIMIUSampInfo_AFE_5V2,								        /*高压采集芯片-5V回检*/
    eIMIUSampInfo_PasInsu,								        /*正极绝缘阻值*/
    eIMIUSampInfo_NegInsu,								        /*负极绝缘阻值*/
    eIMIUSampInfo_Ntc1T,								        /*NTC1温度*/
    eIMIUSampInfo_Ntc2T,								        /*NTC2温度*/
    eIMIUSampInfo_GpioSta,								        /*GPIO信号值*/
    eIMIUSampInfo_Board5V,								        /*板子供电5V回检*/
    eIMIUSampInfo_LinkShuntCurrL,								/*Link分流器电流低*/
    eIMIUSampInfo_LinkShuntCurrH,								/*Link分流器电流高*/
    eIMIUSampInfo_PEgndVolt,								    /*PEGND电压采样,机壳相对对负母线电压*/

	eIMIUSampInfo_Len   			        				    /*IMIU4采样信息长度*/
}e_IMIUSampInfo;

/*权限级别状态*/
typedef enum
{
    e_IMIUAuthStatus_AuthLev = 0,                        // 权限级别
    e_IMIUAuthStatus_AuthPort,                         // 权限端口 
    e_IMIUAuthStatus_Len	        				     
}e_IMIUAuthStatus;

/*协议子索引列表，IMIU4告警信息*/
typedef enum
{
    e_IMIUAlmInfo_Alm1 = 0,                                     /*告警信息1*/
    e_IMIUAlmInfo_Alm2,                                         /*告警信息2*/
    e_IMIUAlmInfo_Alm3,                                         /*告警信息3*/
    e_IMIUAlmInfo_Fault1,                                       /*故障信息1*/
    e_IMIUAlmInfo_Fault2,                                       /*故障信息2*/
    e_IMIUAlmInfo_Fault3,                                       /*故障信息3*/

    eIMIUAlmInfo_Len   			        				        /*IMIU4故障告警信息长度*/
}e_IMIUAlmInfo;

/*协议子索引列表，IMIU4采样校准反馈信息*/
typedef enum
{
    e_IMIUSampCalInfo_Type = 0,                                     /*采样校准类型*/
    e_IMIUSampCalInfo_Status,                                       /*采样校准状态*/
    e_IMIUSampCalInfo_Result,                                       /*采样校准结果*/

    eIMIUSampCalInfo_Len   			        				        /*IMIU4采样校准反馈信息长度*/
}e_IMIUSampCalInfo;

/*协议子索引列表，IMIU4系统调试反馈信息*/
typedef enum
{
    e_IMIUSysDebugInfo_Type = 0,                                   /*调试类型*/
    e_IMIUSysDebugInfo_Port,                                       /*调试端口号*/
    e_IMIUSysDebugInfo_State,                                      /*调试状态*/
    e_IMIUSysDebugInfo_WH_Eeprom,                                  /*写EEPROM高16位*/
    e_IMIUSysDebugInfo_WL_Eeprom,                                  /*写EEPROM低16位*/
    e_IMIUSysDebugInfo_RH_Eeprom,                                  /*读EEPROM高16位*/
    e_IMIUSysDebugInfo_RL_Eeprom,                                  /*读EEPROM低16位*/
    e_IMIUSysDebugInfo_Result,                                      /*调试结果*/

    eIMIUSysDebugInfo_Len   			        				   /*IMIU4系统调试反馈信息长度*/
}e_IMIUSysDebugInfo;

/*协议子索引列表，IMIU4系统调试反馈信息*/
typedef enum
{
    e_IMIUDeviceInfo_Label = 0,                                  /*设备标识*/
    e_IMIUDeviceInfo_Addr,                                       /*设备地址*/
    e_IMIUDeviceInfo_HardVer,                                    /*硬件版本*/
    e_IMIUDeviceInfo_LSoftVer,                                    /*软件版本L*/
    e_IMIUDeviceInfo_HSoftVer,                                  /*软件版本H*/
    e_IMIUDeviceInfo_BSPVer,                                  /*BSP版本*/    
    e_IMIUDeviceInfo_CANVer,                                  /*CAN通讯包版本*/
    e_IMIUDeviceInfo_485Ver,                                  /*485通讯包版本*/
    e_IMIUDeviceInfo_LRuntime,                                  /*软件运行时间L*/
    e_IMIUDeviceInfo_HRuntime,                                     /*软件运行时间H*/

    eIMIUDeviceInfo_Len   			        				   /*IMIU4设备信息长度*/
}e_IMIUDeviceInfo;

//所有IMIU4信息
typedef struct IMIU4_RCV_DATA
{
    u16 DeviceInfo[eIMIUDeviceInfo_Len];                          //设备信息
    u8  SerNum[23];                              //序列号
    u16 WorkMode;                                //工作模式
    s16 SampInfo[eIMIUSampInfo_Len];             //采样信息
    u16 AuthStatus[e_IMIUAuthStatus_Len];        //权限级别状态
    u16 CtrlInfo;                                //控制命令反馈信息0:自动检测 1：正在检测 2：停止检测 3：检测完成 4：检测超时 5：检测失败 
    u16 AlmFault[eIMIUAlmInfo_Len];              //告警信息
    u16 SampCalInfo[eIMIUSampCalInfo_Len];           //采样校准反馈信息
    u16 SysDebugInfo[8];          //系统调试反馈信息
    u16 ConParaCurr[16];          //常规参数当前值

}__attribute__((aligned(4))) gImiu4_data;
/*=================================================================================================
** IMIU4数据帧格式定义
** CANFD数据帧格式标识(Data[0]字段)
**===============================================================================================*/
/*长帧格式标识(Data[0]高4位)*/
#define IMIU4_FRAME_SINGLE       (0x0c)   /*单帧传输: 数据长度<=64字节,仅发送一帧*/
#define IMIU4_FRAME_START        (0x04)   /*长帧起始帧: 开始长帧传输*/
#define IMIU4_FRAME_MIDDLE       (0x00)   /*长帧中间帧: 长帧传输中间帧*/
#define IMIU4_FRAME_END          (0x08)   /*长帧结束帧: 结束长帧传输*/

/*子索引数目标识(Data[1]字段)*/
#define IMIU4_SUBIDX_NUM         (0x01)   /*子索引数目为1*/

#endif