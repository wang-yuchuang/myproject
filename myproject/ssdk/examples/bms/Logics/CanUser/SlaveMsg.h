/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : SlaveMsg.c
** @Founder      : ZQD808
** @Date         : 2025.01.07
** @Function     : 从控CAN报文处理模块
** @Instructions : 
**===============================================================================================*/
#ifndef _SLAVEMSG_H__
#define _SLAVEMSG_H__
#include "BSPUserHook.h"
#include "BSPConfig.h"
#include "BSPTypeDef.h"
#include "GroupPara.h"
#include "GroupInfo.h"
#include "DEVCAN.h"
#include "CLI_type.h"
#include <compiler.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <debug.h>
#include "CLI.h"

/*=================================================================================================
** @Name      : void GroupCheckRcvBsuMsgAPI(void)
** @Input     : void
** @Output    : void
** @Function  : 主控判断从控信息采集完成任务
** @The notes : 用户50ms周期接口调用
**===============================================================================================*/
void GroupCheckRcvBsuMsgAPI(void);

/*=================================================================================================
** @Name      : void CanRcvSlaveMsgHandleAPI(u32 ID, u8 *pdata, u8 DLC, u8 canNum)
** @Input     : ID:CANID *pdata:CAN数据域  DLC:数据域长度 canNum:CAN通道号
** @Output    : void
** @Function  : 自研从控协议CAN报文接收处理
** @The notes : CAN接收回调函数调用
**===============================================================================================*/
void CanRcvSlaveMsgHandleAPI(u32 ID, u8 *pdata, u8 DLC, u8 canNum);

/*=================================================================================================
** BSU通用信息(02通用信息)主索引定义
**===============================================================================================*/
#define BSU_MINDEX_DEVINFO      (1)                     /*主1 设备信息(RO)*/
#define BSU_MINDEX_SERIAL       (230)                   /*主230 单板序列号(WR) 0xE6*/
#define BSU_MINDEX_PACKSERIAL   (231)                   /*主231 PACK序列号(WR) 0xE7*/
#define BSU_MINDEX_CELLSERIAL   (232)                   /*主232 电芯序列号(WR) 0xE8*/
#define BSU_MINDEX_AUTH         (233)                   /*主233 条码写入权限(WR) 0xE9*/

/*BSU内阻扫描控制指令(主88)主索引*/
#define BSU_MINDEX_INSU_CTRL    (88)                    /*主88 内阻扫描控制指令(WR) 0x58*/

/*BSU信息长度定义*/
#define BSU_DEVINFO_LEN         (10)                    /*设备信息数组长度(u16): 主1设备信息共10个子索引*/
#define BSU_SERIAL_LEN          (23)                    /*单板序列号长度(字节)*/
#define BSU_PACKSERIAL_LEN      (20)                    /*PACK序列号长度(字节)*/
#define BSU_CELLSERIAL_LEN      (24)                    /*电芯序列号长度(字节)*/

/*BSU条码写入权限密码(主233)*/
#define BSU_AUTH_OPEN_PWD0      (20)
#define BSU_AUTH_OPEN_PWD1      (25)
#define BSU_AUTH_OPEN_PWD2      (12)
#define BSU_AUTH_OPEN_PWD3      (12)
#define BSU_AUTH_CLOSE_PWD0     (13)
#define BSU_AUTH_CLOSE_PWD1     (57)
#define BSU_AUTH_CLOSE_PWD2     (24)
#define BSU_AUTH_CLOSE_PWD3     (68)

/*=================================================================================================
** @Name      : BSU版本号/序列号/条码查询与写入接口
** @Input     : addr:BSU从控地址  cellIdx:电芯序号  data:写入数据  open:1开启/0关闭权限
** @Output    : 发送结果(0失败/非0成功)
** @Function  : BMU向BSU下发通用信息查询/写入命令(CAN1)
**===============================================================================================*/
u8 BSU_SendDevInfoQuery(u8 addr);
u8 BSU_SendSerialQuery(u8 addr);
u8 BSU_SendPackSerialQuery(u8 addr);
u8 BSU_SendPackSerialWrite(u8 addr, u8 *data);
u8 BSU_SendCellSerialQuery(u8 addr, u8 cellIdx);
u8 BSU_SendAuthCtrl(u8 addr, u8 open);
u8 BSU_SendInsuScanCtrl(u8 addr, u16 freqIdx, u16 mode);

/*BSU查询缓存与回复标志(供ICC主索引81/82/83/84同步回读)*/
extern u16 gBsuDevInfo[BSU_DEVINFO_LEN];
extern u8  gBsuSerial[BSU_SERIAL_LEN];
extern u8  gBsuPackSerial[BSU_PACKSERIAL_LEN];
extern u8  gBsuCellSerial[BSU_CELLSERIAL_LEN];
extern volatile u8 gBsuDevInfoReplyFlag;
extern volatile u8 gBsuSerialReplyFlag;
extern volatile u8 gBsuPackSerialReplyFlag;
extern volatile u8 gBsuCellSerialReplyFlag;
extern volatile u8 gBsuAuthReplyFlag;

#endif


