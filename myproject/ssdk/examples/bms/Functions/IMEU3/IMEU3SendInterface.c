/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IMEU3SendInterface.c
** @Founder      : ZQD808
** @Date         : 2026.04.15
** @Function     : IMEU3 CAN帧发送接口模块(BMU→IMEU3)
** @Instructions : IMEU3是BMU下级设备,本模块仅实现BMU下发给IMEU3的控制命令发送
**                 主索引86: PWM/DO/泄爆阀/OC门/除湿机控制命令
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "IMEU3Interface.h"
#include "IMEU3Protocol.h"
#include "IMEU3Config.h"
#include "DEVCAN.h"
#include "driver/can.h"
#include "PublicInfo.h"
#include "GroupPara.h"
#include "GroupInfo.h"                            /*需要eIMEU3DevInfo_Len等枚举*/
#include "AuxDevMsg.h"
#include <string.h>
#include <stdio.h>                                 /*DeCtrl debug printf*/

extern u16 gGBmuGenPara_102[];   /*主控通用参数值*/

/*=================================================================================================
** @Name      : IMEU3_BuildCanId
** @Input     : pr:功能码  dp:数据页  pf:主索引  ps:目标地址  sa:源地址
** @Output    : 29位CAN帧ID
** @Function  : 构建IMEU3 CAN帧ID
** @The notes : ID格式: PR(1位)+DP(1位)+PF(8位)+PS(8位)+SA(8位)
**===============================================================================================*/
u32 IMEU3_BuildCanId(u8 func, u8 dp, u8 index, u8 daddr, u8 saddr)
{
    return IMEU3_BUILD_ID(func, dp, index, daddr, saddr);
}

/*=================================================================================================
** @Name      : IMEU3_SendFrame
** @Input     : frame:IMEU3帧结构指针
** @Output    : void
** @Function  : IMEU3 CAN帧发送接口
** @The notes : 支持CANFD 64字节长帧
**===============================================================================================*/
void IMEU3_SendFrame(t_IMEU3Frame *frame)
{
    can_frame_t msg;
    u32 canId;

    if(frame == NULL)
    {
        return;
    }
    if(frame->len > 64)
    {
        return;
    }

    canId = IMEU3_BuildCanId(frame->func, frame->dp, frame->index, frame->daddr, frame->saddr);
    msg.id = canId;
    msg.fd = 1;
    msg.ide = CAN_EXTID;
    msg.rtr = CAN_DTR;
    msg.len = frame->len;

    if(frame->len > 0)
    {
        memcpy(msg.data, frame->data, frame->len);
    }

    BSPCANSendQin(eCAN2, &msg);
}

/*=================================================================================================
** @Name      : IMEU3_SendSingleFrame
** @Input     : pr:功能码  dp:数据页  pf:主索引  ps:目标地址  sa:源地址  data:数据指针  len:数据长度
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3单帧数据(最大8字节)
** @The notes : 适用标准CAN帧
**===============================================================================================*/
u8 IMEU3_SendSingleFrame(u8 func, u8 dp, u8 index, u8 daddr, u8 saddr, u8 *data, u8 len)
{
    can_frame_t msg;
    u32 canId;

    if(len > 8)
    {
        return FALSE;
    }
    if(data == NULL && len > 0)
    {
        return FALSE;
    }

    canId = IMEU3_BuildCanId(func, dp, index, daddr, saddr);
    msg.id = canId;
    msg.fd = 1;
    msg.ide = CAN_EXTID;
    msg.rtr = CAN_DTR;

    if(len > 0)
    {
        memcpy(msg.data, data, len);
        msg.len = len;
    }
    else
    {
        msg.len = 0;
    }

    return BSPCANSendQin(eCAN2, &msg);
}

/*=================================================================================================
** @Name      : IMEU3_SendLongFrame
** @Input     : pr:功能码  dp:数据页  pf:主索引  ps:目标地址  sa:源地址  data:数据指针  len:数据长度
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3长帧数据(CANFD最大64字节)
**===============================================================================================*/
u8 IMEU3_SendLongFrame(u8 func, u8 dp, u8 index, u8 daddr, u8 saddr, u8 *data, u16 len)
{
    can_frame_t msg;
    u32 canId;

    if(len > 64)
    {
        return FALSE;
    }
    if(data == NULL && len > 0)
    {
        return FALSE;
    }

    canId = IMEU3_BuildCanId(func, dp, index, daddr, saddr);
    msg.id = canId;
    msg.fd = 1;
    msg.ide = CAN_EXTID;
    msg.rtr = CAN_DTR;

    if(len > 0)
    {
        memcpy(msg.data, data, len);
        msg.len = len;
    }
    else
    {
        msg.len = 0;
    }

    return BSPCANSendQin(eCAN2, &msg);
}

/*=================================================================================================
** @Name      : IMEU3_SendErrorResp
** @Input     : targetAddr:目标地址  errorCode:错误代码
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3错误应答帧(BMU→IMEU3)
** @The notes : 主索引102/110,错误代码1-5,用于BMU回复IMEU3的错误响应
**===============================================================================================*/
u8 IMEU3_SendErrorResp(u8 targetAddr, u8 mainIndex, u8 errorCode)
{
    u8 data[2];
    u8 bmuAddr;
    
    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];

    data[0] = mainIndex;
    data[1] = errorCode;

    return IMEU3_SendSingleFrame(eIMEU3Func_ErrorResponse, 1, mainIndex, targetAddr, bmuAddr, data, 2);
}

/*=================================================================================================
** @Name      : IMEU3_SendDeviceInfoQuery
** @Input     : targetAddr:IMEU3目标地址
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3设备信息查询(BMU→IMEU3)
** @The notes : 主索引1,功能码5(请求查询),BMU查询IMEU3设备信息(7个子索引,起始子索引0)
**===============================================================================================*/
u8 IMEU3_SendDeviceInfoQuery(u8 targetAddr)
{
    u8 data[4];
    u8 bmuAddr;
    
    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    
    data[0] = 0x0c;
    data[1] = eIMEUDevAddr_Len;
    data[2] = 0;
    data[3] = 0;
    return IMEU3_SendSingleFrame(eIMEU3Func_RequestQuery, 1, eIMEU3Idx_DeviceInfo, targetAddr, bmuAddr, data, 4);
}

/*=================================================================================================
** @Name      : IMEU3_SendPwmCtrl
** @Input     : targetAddr:IMEU3目标地址  subIdx:子索引(0-3)  duty:PWM占空比(0-100)
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3 PWM控制命令(BMU→IMEU3)
** @The notes : 主索引86,子索引0-3,功能码2(请求修改),用于风扇PWM控制
**===============================================================================================*/
u8 IMEU3_SendPwmCtrl(u8 targetAddr, u8 subIdx, u8 duty)
{
    u8 data[5];
    u8 bmuAddr;

    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    
    if(subIdx > IMEU3_PWMCTRL_MAX_INDEX)
    {
        return FALSE;
    }
    if(duty > 100)
    {
        return FALSE;
    }

    data[0] = 0x0c;
    data[1] = 0x01;
    data[2] = subIdx;
    data[3] = 0x00;
    data[4] = duty;

    return IMEU3_SendSingleFrame(eIMEU3Func_RequestModify, 1, eIMEU3Idx_Control, targetAddr, bmuAddr, data, 5);
}

/*=================================================================================================
** @Name      : IMEU3_SendDoCtrl
** @Input     : targetAddr:IMEU3目标地址  ctrlNum:控制个数  startIdx:起始序号  state:状态
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3 DO控制命令(BMU→IMEU3)
** @The notes : 主索引86,子索引4,功能码2(请求修改),用于数字输出控制(DO1-24)
**===============================================================================================*/
u8 IMEU3_SendDoCtrl(u8 targetAddr, u8 ctrlNum, u8 startIdx, u8 state)
{
    u8 data[7];
    u8 bmuAddr;

    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    
    if((ctrlNum != 1) && (ctrlNum != 0xff))
    {
        return FALSE;
    }
    if((startIdx < 1) || (startIdx > 24))
    {
        return FALSE;
    }
    if(state > 1)
    {
        return FALSE;
    }

    data[0] = 0x0c;
    data[1] = 0x03;  // DO控制个数 + DO控制开始序号 + DO控制命令
    data[2] = 0x04;  // DO控制个数开始
    data[3] = 0x00;
    data[4] = ctrlNum;
    data[5] = startIdx;
    data[6] = state;

    return IMEU3_SendSingleFrame(eIMEU3Func_RequestModify, 1, eIMEU3Idx_Control, targetAddr, bmuAddr, data, 7);
}

/*=================================================================================================
** @Name      : IMEU3_SendMotorCtrl
** @Input     : targetAddr:IMEU3目标地址  subIdx:子索引(7-8)  state:状态(0/1)
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3泄爆阀控制命令(BMU→IMEU3)
** @The notes : 主索引86,子索引7-8,功能码2(请求修改),用于泄爆阀开关控制
**===============================================================================================*/
u8 IMEU3_SendMotorCtrl(u8 targetAddr, u8 subIdx, u8 state)
{
    u8 data[5];
    u8 bmuAddr;

    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    
    if((subIdx < IMEU3_MOTORCTRL_MIN_INDEX) || (subIdx > IMEU3_MOTORCTRL_MAX_INDEX))
    {
        return FALSE;
    }
    if(state > 1)
    {
        return FALSE;
    }

    data[0] = 0x0c;
    data[1] = 0x01;
    data[2] = subIdx;
    data[3] = 0x00;
    data[4] = state;

    return IMEU3_SendSingleFrame(eIMEU3Func_RequestModify, 1, eIMEU3Idx_Control, targetAddr, bmuAddr, data, 5);
}

/*=================================================================================================
** @Name      : IMEU3_SendOcCtrl
** @Input     : targetAddr:IMEU3目标地址  subIdx:子索引(9-11)  state:状态(0/1)
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3 OC门控制命令(BMU→IMEU3)
** @The notes : 主索引86,子索引9-11,功能码2(请求修改),用于OC门开关控制
**===============================================================================================*/
u8 IMEU3_SendOcCtrl(u8 targetAddr, u8 subIdx, u8 state)
{
    u8 data[5];
    u8 bmuAddr;

    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    
    if((subIdx < IMEU3_OCCTRL_MIN_INDEX) || (subIdx > IMEU3_OCCTRL_MAX_INDEX))
    {
        return FALSE;
    }
    if(state > 1)
    {
        return FALSE;
    }

    data[0] = 0x0c;
    data[1] = 0x01;
    data[2] = subIdx;
    data[3] = 0x00;
    data[4] = state;

    return IMEU3_SendSingleFrame(eIMEU3Func_RequestModify, 1, eIMEU3Idx_Control, targetAddr, bmuAddr, data, 5);
}

/*=================================================================================================
** @Name      : IMEU3_SendDeCtrl
** @Input     : targetAddr:IMEU3目标地址  openHum:开启除湿湿度  closeHum:关闭除湿湿度  mode:工作模式
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3除湿机控制命令(BMU→IMEU3)
** @The notes : 主索引86,子索引12-14,功能码2(请求修改),开湿度必须大于关湿度
**===============================================================================================*/
u8 IMEU3_SendDeCtrl(u8 targetAddr, u8 openHum, u8 closeHum, u8 mode)
{
    u8 data[7];
    u8 bmuAddr;

    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    
    if((mode != 0) && (mode != 0xff))
    {
        return FALSE;
    }
    if(openHum > 100)
    {
        return FALSE;
    }
    if(closeHum > 100)
    {
        return FALSE;
    }
    if(openHum <= closeHum)
    {
        return FALSE;
    }

    data[0] = 0x0c;
    data[1] = 0x03;
    data[2] = 12;
    data[3] = 0x00;
    data[4] = openHum;
    data[5] = closeHum;
    data[6] = mode;

    return IMEU3_SendSingleFrame(eIMEU3Func_RequestModify, 1, eIMEU3Idx_Control, targetAddr, bmuAddr, data, 7);
}

/*=================================================================================================
** @Name      : IMEU3_SendDevSerialQuery
** @Input     : targetAddr:IMEU3目标地址
** @Output    : 发送结果: 0失败  1成功
** @Function  : 查询IMEU3单板序列号(mIndex=2, 23项u8)(BMU→IMEU3)
** @The notes : 功能码5(请求查询), 上电后调用
**===============================================================================================*/
u8 IMEU3_SendDevSerialQuery(u8 targetAddr)
{
    u8 data[4];
    u8 bmuAddr;

    if(gGBmuGenPara_102 == NULL)
        return FALSE;
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];

    data[0] = 0x0C;
    data[1] = eIMEUDevSerial_Len;
    data[2] = 0;
    data[3] = 0;

    return IMEU3_SendSingleFrame(eIMEU3Func_RequestQuery, 1, eIMEU3Idx_DevSerial,
                                 targetAddr, bmuAddr, data, 4);
}

/*=================================================================================================
** @Name      : IMEU3_SendCfgParaQuery
** @Input     : targetAddr:IMEU3目标地址
** @Output    : 发送结果: 0失败  1成功
** @Function  : 查询IMEU3常规参数配置(mIndex=102, 20项u16)(BMU→IMEU3)
** @The notes : 功能码5(请求查询), 上电后和配置修改后调用
**===============================================================================================*/
u8 IMEU3_SendCfgParaQuery(u8 targetAddr)
{
    u8 data[4];
    u8 bmuAddr;

    if(gGBmuGenPara_102 == NULL)
        return FALSE;
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];

    data[0] = 0x0C;
    /*暂用12, 原值eIMEU3Cfg_Len(13)待EU3协议确认后恢复*/
    // data[1] = eIMEU3Cfg_Len;
    data[1] = 12;
    data[2] = 0;
    data[3] = 0;

    return IMEU3_SendSingleFrame(eIMEU3Func_RequestQuery, 1, eIMEU3Idx_QueryPara,
                                 targetAddr, bmuAddr, data, 4);
}

/*=================================================================================================
** @Name      : IMEU3_SendModifyPara
** @Input     : targetAddr:IMEU3目标地址  startSubIdx:起始子索引  subNum:子索引数目  values:值数组(u16)
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMEU3参数配置命令(BMU→IMEU3), 一条帧写多个子索引
** @The notes : mIndex=110, 功能码2(请求修改), 每个值u16低位在前, 最多13项
**===============================================================================================*/
u8 IMEU3_SendModifyPara(u8 targetAddr, u8 startSubIdx, u8 subNum, u16 *values)
{
    u8 data[30];                     /* 4字节帧头 + 最多13项×2字节 = 30字节 */
    u8 bmuAddr;
    u8 i;

    if(gGBmuGenPara_102 == NULL || values == NULL)
    {
        return FALSE;
    }
    if((subNum == 0) || ((startSubIdx + subNum) > 13))       /* mIndex=110共13项(sub0-12) */
    {
        return FALSE;
    }

    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];

    data[0] = 0x0C;                  /* SOF长帧仅单帧 */
    data[1] = subNum;                /* 子索引数目 */
    data[2] = startSubIdx;           /* 起始子索引 */
    data[3] = 0x00;                  /* 预留 */
    for(i = 0; i < subNum; i++)
    {
        data[4 + 2 * i]     = (u8)(values[i] & 0xFF);        /* 低位在前 */
        data[4 + 2 * i + 1] = (u8)((values[i] >> 8) & 0xFF); /* 高位在后 */
    }

    return IMEU3_SendLongFrame(eIMEU3Func_RequestModify, 1, eIMEU3Idx_ModifyPara,
                               targetAddr, bmuAddr, data, (u16)(4 + 2 * subNum));
}

/*=================================================================================================
** @Name      : IMEU3_SendSystemStatusQuery
** @Input     : targetAddr:IMEU3目标地址
** @Output    : 发送结果: 0失败  1成功
** @Function  : 查询IMEU3系统状态(BMU→IMEU3)
** @The notes : mIndex=50, 功能码5(请求查询), 1项(系统工作模式)
**===============================================================================================*/
u8 IMEU3_SendSystemStatusQuery(u8 targetAddr)
{
    u8 data[4];
    u8 bmuAddr;

    if(gGBmuGenPara_102 == NULL)
        return FALSE;
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];

    data[0] = 0x0C;                  /* SOF长帧仅单帧 */
    data[1] = 1;                     /* 子索引数目=1(系统工作模式) */
    data[2] = 0;                     /* 起始子索引 */
    data[3] = 0x00;                  /* 预留 */

    return IMEU3_SendSingleFrame(eIMEU3Func_RequestQuery, 1, eIMEU3Idx_SystemStatus,
                                 targetAddr, bmuAddr, data, 4);
}