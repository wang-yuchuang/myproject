/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IMEU3Interface.h
** @Founder      : ZQD808
** @Date         : 2026.04.15
** @Function     : IMEU3模块公共接口定义
** @Instructions : IMEU3是BMU下级设备,本模块仅定义BMU对IMEU3的接收处理和控制命令发送接口
**===============================================================================================*/
#ifndef __IMEU3INTERFACE_H_
#define __IMEU3INTERFACE_H_

#include "BSPTypeDef.h"
#include "IMEU3Protocol.h"
#include "IMEU3Config.h"

/*IMEU3 CAN帧数据结构*/
typedef struct
{
    u8 func;             
    u8 dp;               
    u8 index;            
    u8 daddr;            
    u8 saddr;            
    u8 data[64];        
    u8 len;             
} t_IMEU3Frame;

/*IMEU3错误码枚举*/
typedef enum
{
    eIMEU3Err_IndexInvalid = 1,     /*读写索引非法*/
    eIMEU3Err_CRCInvalid = 2,       /*CRC校验非法*/
    eIMEU3Err_PermissionInvalid = 3,/*写入权限非法*/
    eIMEU3Err_StateInvalid = 4,     /*写入状态非法*/
    eIMEU3Err_ValueInvalid = 5      /*写入值非法*/
} e_IMEU3ErrorCode;

/*=================================================================================================
** @Name      : IMEU3_RcvHandle
** @Input     : ID:CAN帧ID  pdata:数据域指针  DLC:数据长度  canNum:CAN通道号
** @Output    : void
** @Function  : IMEU3 CAN接收处理入口(IMEU3→BMU)
** @The notes : 处理IMEU3上报的设备信息、系统状态、板级信息、告警信息等数据
**===============================================================================================*/
void IMEU3_RcvHandle(u32 ID, u8 *pdata, u8 DLC, u8 canNum);

/*=================================================================================================
** @Name      : IMEU3_SendFrame
** @Input     : frame:IMEU3帧结构指针
** @Output    : void
** @Function  : IMEU3 CAN帧发送接口
** @The notes : 支持CANFD 64字节长帧
**===============================================================================================*/
void IMEU3_SendFrame(t_IMEU3Frame *frame);

/*=================================================================================================
** @Name      : IMEU3_BuildCanId
** @Input     : func:功能码  dp:数据页  index:主索引  daddr:目标地址  saddr:源地址
** @Output    : 29位CAN帧ID
** @Function  : 构建IMEU3 CAN帧ID
** @The notes : ID格式: Func(4位)+DP(1位)+Index(8位)+DAddr(8位)+SAddr(8位)
**===============================================================================================*/
u32 IMEU3_BuildCanId(u8 func, u8 dp, u8 index, u8 daddr, u8 saddr);

/*=================================================================================================
** @Name      : IMEU3_SendSingleFrame
** @Input     : func:功能码  dp:数据页  index:主索引  daddr:目标地址  saddr:源地址  data:数据指针  len:数据长度
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3单帧数据(最大8字节)
** @The notes : 适用标准CAN帧
**===============================================================================================*/
u8 IMEU3_SendSingleFrame(u8 func, u8 dp, u8 index, u8 daddr, u8 saddr, u8 *data, u8 len);

/*=================================================================================================
** @Name      : IMEU3_SendLongFrame
** @Input     : func:功能码  dp:数据页  index:主索引  daddr:目标地址  saddr:源地址  data:数据指针  len:数据长度
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3长帧数据(CANFD最大64字节)
**===============================================================================================*/
u8 IMEU3_SendLongFrame(u8 func, u8 dp, u8 index, u8 daddr, u8 saddr, u8 *data, u16 len);

/*=================================================================================================
** @Name      : IMEU3_SendErrorResp
** @Input     : targetAddr:IMEU3目标地址  mainIndex:主索引  errorCode:错误代码
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3错误应答帧(BMU→IMEU3)
** @The notes : 功能码9(ErrorResponse),用于BMU回复IMEU3参数读写错误
**===============================================================================================*/
u8 IMEU3_SendErrorResp(u8 targetAddr, u8 mainIndex, u8 errorCode);

/*=================================================================================================
** @Name      : IMEU3_SendDeviceInfoQuery
** @Input     : targetAddr:IMEU3目标地址
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3设备信息查询(BMU→IMEU3)
** @The notes : 主索引1,功能码5(请求查询),查询7个子索引(起始子索引0)
**===============================================================================================*/
u8 IMEU3_SendDeviceInfoQuery(u8 targetAddr);

/*=================================================================================================
** @Name      : IMEU3_SendPwmCtrl
** @Input     : targetAddr:IMEU3目标地址  subIdx:子索引(0-3)  duty:PWM占空比(0-100)
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3 PWM控制命令(BMU→IMEU3)
** @The notes : 主索引86,子索引0-3,用于风扇PWM控制
**===============================================================================================*/
u8 IMEU3_SendPwmCtrl(u8 targetAddr, u8 subIdx, u8 duty);

/*=================================================================================================
** @Name      : IMEU3_SendDoCtrl
** @Input     : targetAddr:IMEU3目标地址  ctrlNum:控制个数  startIdx:起始序号  state:状态
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3 DO控制命令(BMU→IMEU3)
** @The notes : 主索引86,子索引4,用于数字输出控制(DO1-24)
**===============================================================================================*/
u8 IMEU3_SendDoCtrl(u8 targetAddr, u8 ctrlNum, u8 startIdx, u8 state);

/*=================================================================================================
** @Name      : IMEU3_SendMotorCtrl
** @Input     : targetAddr:IMEU3目标地址  subIdx:子索引(7-8)  state:状态(0/1)
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3泄爆阀控制命令(BMU→IMEU3)
** @The notes : 主索引86,子索引7-8,用于泄爆阀开关控制
**===============================================================================================*/
u8 IMEU3_SendMotorCtrl(u8 targetAddr, u8 subIdx, u8 state);

/*=================================================================================================
** @Name      : IMEU3_SendOcCtrl
** @Input     : targetAddr:IMEU3目标地址  subIdx:子索引(9-11)  state:状态(0/1)
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3 OC门控制命令(BMU→IMEU3)
** @The notes : 主索引86,子索引9-11,用于OC门开关控制
**===============================================================================================*/
u8 IMEU3_SendOcCtrl(u8 targetAddr, u8 subIdx, u8 state);

/*=================================================================================================
** @Name      : IMEU3_SendDeCtrl
** @Input     : targetAddr:IMEU3目标地址  openHum:开启除湿湿度  closeHum:关闭除湿湿度  mode:工作模式
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3除湿机控制命令(BMU→IMEU3)
** @The notes : 主索引86,子索引12-14,用于除湿机参数设置
**===============================================================================================*/
u8 IMEU3_SendDeCtrl(u8 targetAddr, u8 openHum, u8 closeHum, u8 mode);

/*=================================================================================================
** @Name      : IMEU3_SendDevSerialQuery
** @Input     : targetAddr:IMEU3目标地址
** @Output    : 发送结果: 0失败  1成功
** @Function  : 查询IMEU3单板序列号(mIndex=2, 23项u8)(BMU→IMEU3)
** @The notes : 功能码5(请求查询), 上电后调用
**===============================================================================================*/
u8 IMEU3_SendDevSerialQuery(u8 targetAddr);

/*=================================================================================================
** @Name      : IMEU3_SendCfgParaQuery
** @Input     : targetAddr:IMEU3目标地址
** @Output    : 发送结果: 0失败  1成功
** @Function  : 查询IMEU3常规参数配置(mIndex=102, 20项u16)(BMU→IMEU3)
** @The notes : 功能码5(请求查询), 上电后和配置修改后调用
**===============================================================================================*/
u8 IMEU3_SendCfgParaQuery(u8 targetAddr);

/*=================================================================================================
** @Name      : IMEU3_SendModifyPara
** @Input     : targetAddr:IMEU3目标地址  startSubIdx:起始子索引  subNum:子索引数目  values:值数组(u16)
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3参数配置命令(BMU→IMEU3), 一条帧写多个子索引
** @The notes : mIndex=110, 功能码2(请求修改), 每个值u16低位在前, 最多13项
**===============================================================================================*/
u8 IMEU3_SendModifyPara(u8 targetAddr, u8 startSubIdx, u8 subNum, u16 *values);

/*=================================================================================================
** @Name      : IMEU3_SendSystemStatusQuery
** @Input     : targetAddr:IMEU3目标地址
** @Output    : 发送结果: 0失败  1成功
** @Function  : 查询IMEU3系统状态(BMU→IMEU3)
** @The notes : mIndex=50, 功能码5(请求查询), 1项(系统工作模式)
**===============================================================================================*/
u8 IMEU3_SendSystemStatusQuery(u8 targetAddr);

#endif