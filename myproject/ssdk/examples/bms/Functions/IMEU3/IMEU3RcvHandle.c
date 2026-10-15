/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IMEU3RcvHandle.c
** @Founder      : ZQD808
** @Date         : 2026.04.15
** @Function     : IMEU3 CANFD接收处理实现(IMEU3→BMU)
** @Instructions : IMEU3是BMU下级设备,本模块仅实现BMU接收IMEU3上报的数据
**                 主索引2/50/51/52/53: 设备信息、系统状态、板级信息、告警信息
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "IMEU3Interface.h"
#include "IMEU3Protocol.h"
#include "IMEU3Config.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "AuxDevMsg.h"
#include "DEVCAN.h"
#include "driver/can.h"
#include <string.h>
#include "IccMsgVerSrl.h"

extern u16 gGBmuGenPara_102[];   /*主控通用参数值*/
extern u8  *gImeu3BoardInfo1;
extern u16 *gImeu3BoardInfo2;
extern u16 *gImeu3AlmInfo;
extern u8  *gImeu3DevSerial;                         /*mIndex=2*/
extern u16 *gImeu3SysStatus;                         /*mIndex=50*/
extern u16 *gImeu3CfgPara;                           /*mIndex=102*/
extern u16 *gImeu3DevAddr;                           /*mIndex=1*/

/*=============================================================================================
** @Name      : gEU3CfgReply / gEU3CfgReplyFlag
** @Function  : EU3参数配置回复缓存, 供CLI查询修改结果
**===========================================================================================*/
u16 gEU3CfgReply     = 0;       /*回复状态: 0=无回复 1=成功 2=索引非法 3=CRC非法 4=权限非法 5=状态非法 6=值非法*/
u8  gEU3CfgReplySub  = 0;       /*最后一次配置的子索引*/
u8  gEU3CfgReplyVal  = 0;       /*回复回显的配置值*/
volatile u8  gEU3CfgReplyFlag = 0;       /*有新回复标志: 0=无 1=有*/
volatile u8  gEU3DevInfoReplyFlag = 0;   /*主1设备信息查询回复标志: 0=无 1=有*/
volatile u8  gEU3SerialReplyFlag  = 0;   /*主2序列号查询回复标志: 0=无 1=有*/

/*=================================================================================================
** @Name      : IMEU3_ParseCanId
** @Input     : id - CAN标识符；frame - 帧结构指针
** @Output    : 无
** @Function  : 解析CAN ID，提取PR/DP/PF/PS/SA字段到帧结构
** @The notes : 使用IMEU3协议宏定义提取各字段
**===============================================================================================*/
static void IMEU3_ParseCanId(u32 id, t_IMEU3Frame *frame)
{
    if(frame == NULL)
    {
        return;
    }
    frame->func = (u8)IMEU3_GET_FUNC(id);  
    frame->dp = (u8)IMEU3_GET_DP(id);    
    frame->index = (u8)IMEU3_GET_INDEX(id); 
    frame->daddr = (u8)IMEU3_GET_DADDR(id); 
    frame->saddr = (u8)IMEU3_GET_SADDR(id); 
}

/*=================================================================================================
** @Name      : IMEU3_CheckAddress
** @Input     : frame - 帧结构指针；canNum - CAN通道号
** @Output    : TRUE-地址有效，FALSE-地址无效
** @Function  : 检查CAN帧地址有效性，验证CAN通道、源地址和目标地址
** @The notes : 仅处理CAN2通道，源地址需为IMEU3设备地址，目标地址需为本机地址或广播地址
**===============================================================================================*/
static u8 IMEU3_CheckAddress(t_IMEU3Frame *frame, u8 canNum)
{
    u8 bmuAddr;
    
    if(frame == NULL)
    {
        return FALSE;
    }
    if(canNum != eCAN2)    
    {
        return FALSE;
    }
    if((frame->saddr != IMEU3_PDUIN_DEVADDR) && (frame->saddr != IMEU3_PDUOUT_DEVADDR))    
    {
        return FALSE;
    }
    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];    
    if((frame->daddr != bmuAddr) && (frame->daddr != 0xff))    
    {
        return FALSE;
    }
    return TRUE;
}

/*=================================================================================================
** @Name      : IMEU3_RcvDeviceInfo
** @Input     : pdata - 数据指针；len - 数据长度；srcAddr - 源地址
** @Output    : 无
** @Function  : 接收并解析设备信息帧(IMEU3→BMU)，提取设备ID和版本信息
** @The notes : 数据长度需至少8字节，包含设备ID、软件版本和硬件版本
**===============================================================================================*/
static void IMEU3_RcvDeviceInfo(u8 *pdata, u8 len, u8 srcAddr, u8 func)
{
    u8 sIdx;
    u8 sNum;
    u8 i;

    /*参数有效性检查*/
    if(pdata == NULL || gImeu3DevAddr == NULL)
    {
        return;
    }

    /*mIndex=1查询回复, 只处理QueryReply(func=6)*/
    if(func != eIMEU3Func_QueryReply || len < 6)
    {
        return;
    }

    /*QueryReply帧头: pdata[0]=framMark, [1]=sIndexNum, [2:3]=sIndex, [4:]=data(u16)*/
    sIdx = pdata[2];
    sNum = pdata[1];
    for(i = 0; i < sNum && (sIdx + i) < eIMEUDevAddr_Len && (4 + 2 * i + 1) < len; i++)
    {
        gImeu3DevAddr[sIdx + i] = (u16)((pdata[5 + 2 * i] << 8) | pdata[4 + 2 * i]);
    }
    gEU3DevInfoReplyFlag = 1;
    ICCVerSrlReplyNotify(ICC_DEVTYPE_IMEU3, VSRL_EVT_DEVINFO);
}

/*=================================================================================================
** @Name      : IMEU3_RcvSystemStatus
** @Input     : pdata - 数据指针；len - 数据长度
** @Output    : 无
** @Function  : 接收系统状态帧(IMEU3→BMU)
** @The notes : 数据长度需至少8字节
**===============================================================================================*/
static void IMEU3_RcvSystemStatus(u8 *pdata, u8 len, u8 func)
{
    /*参数有效性检查*/
    if(pdata == NULL || gImeu3SysStatus == NULL)
    {
        return;
    }

    /*mIndex=50查询回复, 只处理QueryReply(func=6), 1项u16*/
    if(func != eIMEU3Func_QueryReply || len < 6)
    {
        return;
    }

    /*数据从pdata[4:5]开始(跳过4字节帧头)*/
    gImeu3SysStatus[0] = (u16)((pdata[5] << 8) | pdata[4]);
}

/*=================================================================================================
** @Name      : IMEU3_RcvDevSerial
** @Input     : pdata - 数据指针；len - 数据长度
** @Output    : 无
** @Function  : 接收单板序列号帧(IMEU3→BMU)，存储到全局数组gImeu3DevSerial
** @The notes : mIndex=2, 数据类型u8, 23项
**===============================================================================================*/
static void IMEU3_RcvDevSerial(u8 *pdata, u8 len, u8 func)
{
    u8 sIdx;
    u8 sNum;
    u8 i;

    /*参数有效性检查*/
    if(pdata == NULL || gImeu3DevSerial == NULL)
    {
        return;
    }

    /*mIndex=2, 只处理QueryReply(func=6), u8 23项*/
    if(func != eIMEU3Func_QueryReply || len < 5)
    {
        return;
    }

    /*QueryReply帧头: pdata[4:]=data(u8)*/
    sIdx = pdata[2];
    sNum = pdata[1];
    for(i = 0; i < sNum && (sIdx + i) < eIMEUDevSerial_Len && (4 + i) < len; i++)
    {
        gImeu3DevSerial[sIdx + i] = pdata[4 + i];
    }
    gEU3SerialReplyFlag = 1;
    ICCVerSrlReplyNotify(ICC_DEVTYPE_IMEU3, VSRL_EVT_SERIAL);
}

/*=================================================================================================
** @Name      : IMEU3_RcvQueryParaReply
** @Input     : pdata - 数据指针；len - 数据长度；func - 功能码
** @Output    : 无
** @Function  : 接收常规参数查询回复帧(IMEU3→BMU)，存储到全局数组gImeu3CfgPara
** @The notes : mIndex=102, func=6(QueryReply), 数据类型u16, 20项
**===============================================================================================*/
static void IMEU3_RcvQueryParaReply(u8 *pdata, u8 len, u8 func)
{
    u8 sIdx;
    u8 sNum;
    u8 i;

    /*参数有效性检查*/
    if(pdata == NULL || gImeu3CfgPara == NULL)
    {
        return;
    }

    /*mIndex=102, 只处理QueryReply(func=6), u16 20项*/
    if(func != eIMEU3Func_QueryReply || len < 5)
    {
        return;
    }

    /*QueryReply帧头: pdata[0]=framMark, [1]=sIndexNum, [2:3]=sIndex, [4:]=data(u16)*/
    sIdx = pdata[2];
    sNum = pdata[1];
    for(i = 0; i < sNum && (sIdx + i) < eIMEU3Cfg_Len && (4 + 2 * i + 1) < len; i++)
    {
        gImeu3CfgPara[sIdx + i] = (u16)((pdata[5 + 2 * i] << 8) | pdata[4 + 2 * i]);
    }
}

/*=================================================================================================
** @Name      : IMEU3_RcvBoardInfo1
** @Input     : pdata - 数据指针；len - 数据长度
** @Output    : 无
** @Function  : 接收板卡信息1帧(IMEU3→BMU)，解析并存储到全局数组gImeu3BoardInfo1
** @The notes : 每个数据项为1字节(u8)；数组索引对应eIMEUBoardInfo1枚举
**===============================================================================================*/
static void IMEU3_RcvBoardInfo1(u8 *pdata, u8 len)
{
    u8 i;
    
    if(pdata == NULL)
    {
        return;
    }
    if(gImeu3BoardInfo1 == NULL)
    {
        return;
    }
    for(i = 0; i < eIMEUBoardInfo1_Len; i++)
    {
        if(i >= len)
        {
            break;
        }
        gImeu3BoardInfo1[i] = pdata[i];
    }
}

/*=================================================================================================
** @Name      : IMEU3_RcvBoardInfo2
** @Input     : pdata - 数据指针；len - 数据长度
** @Output    : 无
** @Function  : 接收板卡信息2帧(IMEU3→BMU)，解析并存储到全局数组gImeu3BoardInfo2
** @The notes : 每个数据项为2字节，完整存储16位值；数组索引对应eIMEUBoardInfo2枚举
**===============================================================================================*/
static void IMEU3_RcvBoardInfo2(u8 *pdata, u8 len)
{
    u8 i;
    u16 value;
    
    if(pdata == NULL)
    {
        return;
    }
    if(gImeu3BoardInfo2 == NULL)
    {
        return;
    }
    for(i = 0; i < eIMEUBoardInfo2_Len; i++)
    {
        if((2 * i + 1) >= len)    /*检查数据边界*/
        {
            break;
        }
        value = (u16)((pdata[2 * i + 1] << 8) | pdata[2 * i]);    /*解析2字节数据*/
        gImeu3BoardInfo2[i] = value;                              /*存储16位完整值*/
    }
}

/*=================================================================================================
** @Name      : IMEU3_RcvAlarmInfo
** @Input     : pdata - 数据指针；len - 数据长度
** @Output    : 无
** @Function  : 接收告警信息帧(IMEU3→BMU)，解析并存储到全局数组gImeu3AlmInfo
** @The notes : 0912 CANFD协议 mIndex=53 布局：连续3个u16小端数据。
**              Byte0-1 = 告警信息1；Byte2-3 = 告警信息2；Byte4-5 = 告警信息3。
**              不兼容旧经典0x14D5帧的“1字节告警1 + 2个故障字”布局。
**===============================================================================================*/
static void IMEU3_RcvAlarmInfo(u8 *pdata, u8 len)
{
    if(pdata == NULL)
    {
        return;
    }
    if(gImeu3AlmInfo == NULL)
    {
        return;
    }

    if(len >= 2)    /*告警信息1: u16小端*/
    {
        gImeu3AlmInfo[eIMEUAlmInfo_Alm1] = (u16)((pdata[1] << 8) | pdata[0]);
    }
    if(len >= 4)    /*告警信息2: u16小端*/
    {
        gImeu3AlmInfo[eIMEUAlmInfo_Alm2] = (u16)((pdata[3] << 8) | pdata[2]);
    }
    if(len >= 6)    /*告警信息3: u16小端*/
    {
        gImeu3AlmInfo[eIMEUAlmInfo_Alm3] = (u16)((pdata[5] << 8) | pdata[4]);
    }
}

/*=================================================================================================
** @Name      : IMEU3_RcvErrorFrame
** @Input     : pdata - 数据指针；len - 数据长度；pf - PDU格式字段
** @Output    : 无
** @Function  : 接收错误帧(IMEU3→BMU)，解析错误索引和错误码
** @The notes : 数据长度需至少2字节；pf用于区分错误帧类型(eIMEU3Idx_Error1/Error2)
**===============================================================================================*/
static void IMEU3_RcvErrorFrame(u8 *pdata, u8 len, u8 pf)
{
    u8 errorCode;
    u8 errorIndex;
    
    if(pdata == NULL)
    {
        return;
    }
    if(len < 2)
    {
        return;
    }
    errorIndex = pdata[0];    /*错误索引，字节0*/
    errorCode = pdata[1];     /*错误码，字节1*/
}

/*=================================================================================================
** @Name      : IMEU3_RcvHandle
** @Input     : ID - CAN标识符；pdata - 数据指针；DLC - 数据长度；canNum - CAN通道号
** @Output    : 无
** @Function  : IMEU3 CANFD接收处理主函数(IMEU3→BMU)，解析CAN帧并根据PF字段分发到对应处理函数
** @The notes : 仅处理CAN2通道；DLC最大限制64字节；根据PF字段分发到不同处理函数
**===============================================================================================*/
void IMEU3_RcvHandle(u32 ID, u8 *pdata, u8 DLC, u8 canNum)
{
    t_IMEU3Frame frame;
    
    frame.len = 0;
    
    if(pdata == NULL)
    {
        return;
    }
    if(DLC == 0)
    {
        return;
    }
    
    IMEU3_ParseCanId(ID, &frame);    /*解析CAN ID*/
    
    if(IMEU3_CheckAddress(&frame, canNum) == FALSE)    /*检查地址有效性*/
    {
        return;
    }
    
    if(DLC > 64)    /*CANFD最大64字节*/
    {
        DLC = 64;
    }
    
    memcpy(frame.data, pdata, DLC);    /*复制数据到帧结构*/
    frame.len = DLC;

    switch(frame.index)    
    {
        case eIMEU3Idx_DeviceInfo:
            IMEU3_RcvDeviceInfo(frame.data, frame.len, frame.saddr, frame.func);    
            break;
            
        case eIMEU3Idx_DevSerial:
            IMEU3_RcvDevSerial(frame.data, frame.len, frame.func);
            break;
            
        case eIMEU3Idx_SystemStatus:
            IMEU3_RcvSystemStatus(frame.data, frame.len, frame.func);
            break;
            
        case eIMEU3Idx_BoardInfo1:
            IMEU3_RcvBoardInfo1(frame.data, frame.len);    
            break;
            
        case eIMEU3Idx_BoardInfo2:
            IMEU3_RcvBoardInfo2(frame.data, frame.len);    
            break;
            
        case eIMEU3Idx_AlarmInfo:
            IMEU3_RcvAlarmInfo(frame.data, frame.len);    
            break;
            
        case eIMEU3Idx_QueryPara:
            IMEU3_RcvQueryParaReply(frame.data, frame.len, frame.func);
            gEU3CfgReplyFlag = 1;                /*查询回复到达, 供ICC mIndex75同步回读等待*/
            break;
            
        case eIMEU3Idx_ModifyPara:
            /*按功能码分发: func=3修改回复 func=9错误应答*/
            if(frame.func == eIMEU3Func_ModifyReply && DLC >= 5)
            {
                gEU3CfgReply     = 1;                       /*修改成功*/
                gEU3CfgReplySub  = pdata[2];                /*子索引: pdata[2]*/
                gEU3CfgReplyVal  = pdata[4];                /*回显值: pdata[4]*/
                gEU3CfgReplyFlag = 1;
            }
            else if(frame.func == eIMEU3Func_ErrorResponse && DLC >= 2)
            {
                gEU3CfgReply     = 1 + (pdata[1] & 0x0F);   /*错误码: 1=索引 2=CRC 3=权限 4=状态 5=值*/
                gEU3CfgReplySub  = 0;                        /*ErrorResponse无子索引*/
                gEU3CfgReplyFlag = 1;
            }
            break;
            
        default:
            break;
    }
}