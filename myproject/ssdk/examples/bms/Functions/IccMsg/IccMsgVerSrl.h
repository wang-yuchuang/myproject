/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IccMsgVerSrl.h
** @Founder      : ZQD808
** @Date         : 2026.09.11
** @Function     : 版本号/序列号主索引(81/82/83/84)查询修改接口
** @Instructions : A核用02功能码(framMark=0x0C)下发,R核转CANFD查询下级设备后组帧回03功能码
**===============================================================================================*/
#ifndef _ICCMSGVERSRL_H_
#define _ICCMSGVERSRL_H_
/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"

/*=================================================================================================
** @Macro definition
**===============================================================================================*/
/*下级设备类型(ICC主索引81/82/83设备类型字段)*/
#define ICC_DEVTYPE_BSU          (1)                     /*BSU从控*/
#define ICC_DEVTYPE_IMIU4        (5)                     /*IMIU4电流测量单元*/
#define ICC_DEVTYPE_IMEU3        (6)                     /*IMEU3*/

/*操作类型枚举: 0无动作 1下发查询 2查询成功 3查询失败 4下发设置 5设置成功 6设置失败*/
#define ICC_OPTYPE_NONE          (0)                     /*无动作*/
#define ICC_OPTYPE_QUERY         (1)                     /*下发查询*/
#define ICC_OPTYPE_QUERY_OK      (2)                     /*查询成功*/
#define ICC_OPTYPE_QUERY_ERR     (3)                     /*查询失败*/
#define ICC_OPTYPE_SET           (4)                     /*下发设置*/
#define ICC_OPTYPE_SET_OK        (5)                     /*设置成功*/
#define ICC_OPTYPE_SET_ERR       (6)                     /*设置失败*/

/*电芯序列号单条长度/个数上限*/
#define BSU_CELLSERIAL_LEN       (24)                    /*单颗电芯条码长度(字节)*/
#define ICC_CELLSERIAL_MAXNUM    (42)                    /*电芯条码个数上限*/

/*下级设备查询回复事件(供ICCVerSrlReplyNotify通知, 事件驱动组帧回复A核)*/
#define VSRL_EVT_DEVINFO        (0)                     /*设备信息回复*/
#define VSRL_EVT_SERIAL         (1)                     /*单板序列号回复*/
#define VSRL_EVT_PACKSERIAL     (2)                     /*PACK序列号查询回复*/
#define VSRL_EVT_PACKSERIAL_WR  (3)                     /*PACK序列号写入回复*/
#define VSRL_EVT_AUTH           (4)                     /*条码写入权限回复*/
#define VSRL_EVT_CELLSERIAL     (5)                     /*电芯序列号回复*/

/*查询超时/重发配置: 单次CAN往返300ms超时, 超时自动重发, 最多重发3次*/
#define ICC_VSRL_TIMEOUT_MS     (300)                   /*单次查询回复超时(ms)*/
#define ICC_VSRL_RETRY_MAX      (2)                     /*超时最大重发次数(初始+2次重发=3次尝试)*/

/*=================================================================================================
** @Interface function declaration
**===============================================================================================*/
/*=================================================================================================
** @Name      : u8 ICCVerSrlDeal_Hook(u8 saddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum)
** @Input     : saddr:源地址  mIndex:主索引号(81-84)  sIndex:起始子索引  sIndexNum:子索引数目  iccNum:ICC通道号
** @Output    : eICCERRREAS_PENDING(自行发送回复) / 其它(框架错误应答)
** @Function  : 81/82/83/84版本号/序列号查询修改处理
** @The notes : 由ICCRcvRupUserDeal_Hook分发调用,下发CAN查询后立即返回PENDING,收到回复后组帧应答
**===============================================================================================*/
u8 ICCVerSrlDeal_Hook(u8 saddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum);

/*=================================================================================================
** @Name      : void ICCVerSrlReplyNotify(u8 devType, u8 evt)
** @Input     : devType:下级设备类型(ICC_DEVTYPE_*)  evt:回复事件(VSRL_EVT_*)
** @Output    : void
** @Function  : 下级设备查询回复通知(收到具体报文后调用, 匹配待回复上下文后组帧回复A核)
**===============================================================================================*/
void ICCVerSrlReplyNotify(u8 devType, u8 evt);

/*=================================================================================================
** @Name      : void ICCVerSrlTimeoutCheck(void)
** @Input     : void
** @Output    : void
** @Function  : 挂起查询超时巡检(50ms周期任务调用): 超时自动重发, 重发次数用尽回查询失败
**===============================================================================================*/
void ICCVerSrlTimeoutCheck(void);

#endif
