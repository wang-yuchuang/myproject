/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : IccMsgHandle.c
** @Founder      : ZQD808
** @Date         : 2026.01.29
** @Function     : 核间通信接收数据处理模块
** @Instructions : 处理A核下发数据处理
**===========================================================================================*/
#ifndef _ICCMSGHANDLE_H__
#define _ICCMSGHANDLE_H__
/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "IccMsgInterface.h"

/*=================================================================================================
** @Macro definition
**===============================================================================================*/
#define ICC_MAX_ICC_NUM                 6                       /*最大ICC通道数目*/

#define ICC_TX_UPM_SPA_T                10                     /*发送周期帧间最短间隔时间(1ms)*/
#define ICC_MAX_COUNT_TIME              0xfffffff              /*CAN通信包计时最大时间数(1ms)*/
/*=================================================================================================
** @Data type definition
**===============================================================================================*/

/*=================================================================================================
** @Interface function declaration
**===============================================================================================*/
/*=============================================================================================
** @Name      : void BSPCAN1RcvTask(void *argument)
** @Input     : *p:任务函数格式需要空指针
** @Output    : void
** @Function  : 核间通信数据接收处理
** @The notes : A核只下发问答信息指令和升级信息指令
**===========================================================================================*/
void IccMsgRcvTask(void *argument);

/*=================================================================================================
** @Name      : void ICCDevTabEntryInit_API(u8 devAddr, u8 mIndexNum const s_ICCIndexTab  *pIndexTab, u8 devId)
** @Input     : devAdd:设备地址 mIndexNum:主索引数目 pIndexTab:设备主动引表 devId:创建设备表序号
** @Output    : void
** @Function  : CAN通信创建设备表初始化
** @The notes : 用户配置CAN通信设备表信息
**===============================================================================================*/
void ICCDevTabEntryInit_API(u8 devAddr, u8 mIndexNum, const t_ICCIndexTab *pIndexTab, u8 devId);

/*=================================================================================================
** @Name      : void ICCBuildMsgID_API(u16 dataNum, e_ICCFunCode funCode, u8 mIndex, u8 destAddr, u8* idData)
** @Input     : dataNum:数据域长度 funCode:功能码 mIndex:主索引号 destAddr:目标地址  idData:帧头id域地址
** @Output    : void
** @Function  : 组建8字节ICC通信帧ID值
** @The notes :
**===============================================================================================*/
void ICCBuildMsgID_API(u16 dataNum, e_ICCFunCode funCode, u8 mIndex, u8 destAddr, u8* idData);

/*=================================================================================================
** @Name      : void ICCBuildMsgDataMark_API(u8 frammark, u8 sindexnum, u8 sindex, u8 *pdata)
** @Input     : frammark:帧标识 sindexnum:子索引数目 pdata:起始子索引 *pdata:读到的数据
** @Output    : void
** @Function  : 组建3字节ICC通信帧数据域标识
** @The notes : 
**===============================================================================================*/
void ICCBuildMsgDataMark_API(u8 frammark, u8 sindexnum, u8 sindex, u8 *pdata);

/*=================================================================================================
** @Name      : void ICCLibTxRcvHandleTask_API(void)
** @Input     : void
** @Output    : void
** @Function  : 核间通信报文收发处理任务(收发通信表处理)
** @The notes : 用户必须1ms周期调用
**===============================================================================================*/
void ICCLibTxRcvHandleTask_API(void);

/*=================================================================================================
** @Name      : u8 ICCLibRcvMsgHandle_API(u8 *pdata, u8 DLC, u8 iccNum)
** @Input     : *pdata:核间通信ID域和数据域  DLC:ID域和数据域长度 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : ICC通信报文接收处理任务
** @The notes : 用户放在ICC接收处理HOOK接口调用
**===============================================================================================*/
u8 ICCLibRcvMsgHandle_API(u8 *pdata, u8 DLC, u8 iccNum);

/*=================================================================================================
** @Name      : u8 ICCBuildSendAupMsg_API(t_ICCFram *iccFram, u8 iccNum)
** @Input     : *iccFram:接收到的ICC通信帧 iccNum:ICC通道号
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 组建和发送修改回复报文帧
** @The notes : 用户自行发送修改回复报文帧
**===============================================================================================*/
u8 ICCBuildSendAupMsg_API(t_ICCFram *iccFram, u8 iccNum);

/*=================================================================================================
** @Name      : u8 ICCBuildSendCupMsg_API(u8 destAddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum)
** @Input     : destAddr:目标设备地址 mIndex:主索引号 sIndex:起始子索引号 sIndexNum:子索引数目 conFlag:是否需要确认 iccNum:ICC通道号
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 组建和发送修改确认报文帧
** @The notes : 用户自行发送修改确认报文帧
**===============================================================================================*/
u8 ICCBuildSendCupMsg_API(u8 destAddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum);

/*=================================================================================================
** @Name      : u8 ICCBuildSendArqMsg_API(t_ICCFram *iccFram, u8 iccNum)
** @Input     : *iccFram:接收到的ICC通信帧 iccNum:ICC通道号
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 组建和发送查询回复报文帧
** @The notes : 用户自行发送查询回复报文帧
**===============================================================================================*/
u8 ICCBuildSendArqMsg_API(t_ICCFram *iccFram, u8 iccNum);

/*=================================================================================================
** @Name      : u8 ICCBuildSendCrqMsg_API(t_ICCFram *iccFram, u8 iccNum)
** @Input     : *iccFram:接收到的ICC通信帧 iccNum:ICC通道号
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 组建和发送查询确认报文帧
** @The notes : 用户自行发送查询确认报文帧
**===============================================================================================*/
u8 ICCBuildSendCrqMsg_API(t_ICCFram *iccFram, u8 iccNum);

/*=================================================================================================
** @Name      : u8 ICCBuildSendUpmMsg_API(u8 destAddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum)
** @Input     : destAddr:目标地址 mIndex:主索引号 sIndex:起始子索引号 sIndexNum:子索引数目  iccNum:ICC通道
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 组建和发送主动更新报文帧
** @The notes : 用户自行发送主动更新报文帧
**===============================================================================================*/
u8 ICCBuildSendUpmMsg_API(u8 destAddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum);
#endif
