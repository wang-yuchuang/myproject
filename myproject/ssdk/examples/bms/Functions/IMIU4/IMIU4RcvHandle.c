/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IMIU4RcvHandle.c
** @Founder      : ZQD808
** @Date         : 2026.04.15
** @Function     : IMIU4 CANFD接收处理实现(IMIU4→BMU)
** @Instructions : IMIU4是BMU下级设备(电流测量单元),本模块实现BMU接收IMIU4上报的数据
**                 主索引51: 采样信息(电压、电流、温度、绝缘)
**                 主索引57: 故障告警信息
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "IMEU3Interface.h"
#include "IMIU4Interface.h"
#include "IMIU4Protocol.h"
#include "IMIU4Config.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "CurrSample.h"
#include "DEVCAN.h"
#include "driver/can.h"
#include <string.h>
#include <stdio.h>
#include "IccMsgVerSrl.h"

/*=================================================================================================
** 外部变量声明
**===============================================================================================*/
/*主控通用参数值*/
extern u16 gGBmuGenPara_102[];

extern gImiu4_data *gImiu4_Info[e_IMIU4_NUM];           //IMIU4信息
/*主控硬件参数配置信息*/
extern u8 gGHardPara_104[];

/*=================================================================================================
** 全局变量定义
**===============================================================================================*/
/*IMIU4调试打印开关: 0关闭  1开启*/
u8 gImiu4DebugPrintEn = 0;
/*IMIU4查询回复标志: 主1设备信息 / 主2序列号*/
volatile u8 gImiu4DevInfoReplyFlag = 0;
volatile u8 gImiu4SerialReplyFlag  = 0;

/*=================================================================================================
** 静态函数: IMIU4_ParseCanId
**===============================================================================================*/
/*=================================================================================================
** @Name      : IMIU4_ParseCanId
** @Input     : id:CAN标识符  frame:帧结构指针
** @Output    : void
** @Function  : 解析CAN ID,提取PR/DP/PF/PS/SA字段到帧结构
** @The notes : 使用IMIU4协议宏定义提取各字段
**===============================================================================================*/
static void IMIU4_ParseCanId(u32 id, t_IMIU4Frame *frame)
{
    /*参数有效性检查*/
    if(frame == NULL)
    {
        return;
    }
    /*提取PR域: 功能码高位(bit25)*/
    frame->pr = (u8)IMIU4_GET_PR(id);
    /*提取DP域: 数据页标志(bit24)*/
    frame->dp = (u8)IMIU4_GET_DP(id);
    /*提取PF域: 主索引(bit16-23)*/
    frame->pf = (u8)IMIU4_GET_PF(id);
    /*提取PS域: 目标地址(bit8-15)*/
    frame->ps = (u8)IMIU4_GET_PS(id);
    /*提取SA域: 源地址(bit0-7)*/
    frame->sa = (u8)IMIU4_GET_SA(id);
}

/*=================================================================================================
** 静态函数: IMIU4_CheckAddress
**===============================================================================================*/
/*=================================================================================================
** @Name      : IMIU4_CheckAddress
** @Input     : frame:帧结构指针  canNum:CAN通道号
** @Output    : TRUE:地址有效  FALSE:地址无效
** @Function  : 检查CAN帧地址有效性,验证CAN通道、源地址和目标地址
** @The notes : 仅处理CAN2通道,源地址需为IMIU4设备地址,目标地址需为本机地址或广播地址
**===============================================================================================*/
static u8 IMIU4_CheckAddress(t_IMIU4Frame *frame, u8 canNum)
{
    u8 bmuAddr;
    
    /*参数有效性检查*/
    if(frame == NULL)
    {
        return FALSE;
    }
    /*仅处理CAN2通道*/
    if(canNum != IMIU4_CAN_CH)
    {
        return FALSE;
    }
    /*检查源地址是否为IMIU4设备地址(20或21)*/
    if((frame->sa != IMIU4_1_DEV_ADDR) && (frame->sa != IMIU4_2_DEV_ADDR))
    {
        return FALSE;
    }
    /*检查参数指针*/
    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    /*获取本机BMU设备地址*/
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    /*目标地址需为本机地址或广播地址0xFF*/
    if((frame->ps != bmuAddr) && (frame->ps != 0xff))
    {
        return FALSE;
    }
    return TRUE;
}

/*=================================================================================================
** 静态函数: IMIU4_RcvSampInfo
**===============================================================================================*/
/*=================================================================================================
** @Name      : IMIU4_RcvSampInfo
** @Input     : pdata:数据指针  len:数据长度  srcAddr:源地址
** @Output    : void
** @Function  : 接收采样信息帧(IMIU4→BMU),解析电压、电流、温度、绝缘阻值数据
** @The notes : 主索引51,每个数据项为s16类型(2字节),低位在前
**===============================================================================================*/
static void IMIU4_RcvSampInfo(u8 *pdata, u8 len, u8 srcAddr)
{
    u8 i;
    s16 value;
    
    /*参数有效性检查*/
    if(pdata == NULL)
    {
        return;
    }
    /*数据长度检查*/
    if(len < 2)
    {
        return;
    }
    
    /*根据源地址选择存储位置*/
    if(srcAddr == IMIU4_1_DEV_ADDR)
    {
        /*1#IMIU4数据存储*/
        if(gImiu4_Info[e_IMIU4_1] == NULL)
        {
            return;
        }
        /*循环解析采样信息(20个s16元素)*/
        for(i = 0; i < eIMIUSampInfo_Len; i++)
        {
            /*检查数据边界*/
            if((2 * i + 1) >= len)
            {
                break;
            }
            /*解析2字节数据,低位在前*/
            value = (s16)((pdata[2 * i + 1] << 8) | pdata[2 * i]);
            /*存储到全局数组*/
            gImiu4_Info[e_IMIU4_1]->SampInfo[i] = value;
        }
        
        /*电流采样转换: 使用分流器采集电流*/
        if(1 == BitGet(gGHardPara_104[eGHardPara104_CSampChan], eCAFE_Shunt))
        {
            /*分流器电流: 低位+高位组合为s32,除以10得到实际电流*/
            CalcSampleADToCurr(0, (s32)(((u32)(u16)gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_ShuntCurrH] << 16) | (u16)gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_ShuntCurrL]) / 10);
        }
        /*电流采样转换: 使用霍尔采集电流*/
        else
        {
            /*霍尔电流: 低位+高位组合为s32,除以10得到实际电流*/
            CalcSampleADToCurr(0, (s32)(((u32)(u16)gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_HallCurrH] << 16) | (u16)gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_HallCurrL]) / 10);
        }
    }
    else if(srcAddr == IMIU4_2_DEV_ADDR)
    {
        /*2#IMIU4数据存储*/
        if(gImiu4_Info[e_IMIU4_2] == NULL)
        {
            return;
        }
        /*循环解析采样信息(20个s16元素)*/
        for(i = 0; i < eIMIUSampInfo_Len; i++)
        {
            /*检查数据边界*/
            if((2 * i + 1) >= len)
            {
                break;
            }
            /*解析2字节数据,低位在前*/
            value = (s16)((pdata[2 * i + 1] << 8) | pdata[2 * i]);
            /*存储到全局数组*/
            gImiu4_Info[e_IMIU4_2]->SampInfo[i] = value;
        }
        
        /*电流采样转换: 使用分流器采集电流*/
        if(1 == BitGet(gGHardPara_104[eGHardPara104_CSampChan], eCAFE_Shunt))
        {
            /*分流器电流: 低位+高位组合为s32,除以10得到实际电流*/
            CalcSampleADToCurr(1, (s32)(((u32)(u16)gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_ShuntCurrH] << 16) | (u16)gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_ShuntCurrL]) / 10);
        }
        /*电流采样转换: 使用霍尔采集电流*/
        else
        {
            /*霍尔电流: 低位+高位组合为s32,除以10得到实际电流*/
            CalcSampleADToCurr(1, (s32)(((u32)(u16)gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_HallCurrH] << 16) | (u16)gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_HallCurrL]) / 10);
        }
    }
}

/*=================================================================================================
** 静态函数: IMIU4_RcvSampInfo
**===============================================================================================*/

/*=================================================================================================
** @Name      : IMIU4_RcvAuthStatus
** @Input     : pdata:数据指针  len:数据长度  srcAddr:源地址
** @Output    : void
** @Function  : 接收采样信息帧(IMIU4→BMU),解析电压、电流、温度、绝缘阻值数据
** @The notes : 主索引54,每个数据项为s16类型(2字节),低位在前
**===============================================================================================*/
static void IMIU4_RcvAuthStatus(u8 *pdata, u8 len, u8 srcAddr)
{
    u16 value;
    u8 i;
    /*参数有效性检查*/
    if(pdata == NULL)
    {
        return;
    }
    /*数据长度检查*/
    if(len < 2)
    {
        return;
    }
    
    /*根据源地址选择存储位置*/
    if(srcAddr == IMIU4_1_DEV_ADDR)
    {
        /*1#IMIU4数据存储*/
        if(gImiu4_Info[e_IMIU4_1] == NULL)
        {
            return;
        }
        /*循环解析采样信息(20个s16元素)*/
        for(i = 0; i < e_IMIUAuthStatus_Len; i++)
        {
            /*检查数据边界*/
            if((2 * i + 1) >= len)
            {
                break;
            }
            /*解析2字节数据,低位在前*/
            value = (s16)((pdata[2 * i + 1] << 8) | pdata[2 * i]);
            /*存储到全局数组*/
            gImiu4_Info[e_IMIU4_1]->AuthStatus[i] = value;
        }
    }else if(srcAddr == IMIU4_2_DEV_ADDR)
    {
        /*1#IMIU4数据存储*/
        if(gImiu4_Info[e_IMIU4_2] == NULL)
        {
            return;
        }
        /*循环解析采样信息(20个s16元素)*/
        for(i = 0; i < e_IMIUAuthStatus_Len; i++)
        {
            /*检查数据边界*/
            if((2 * i + 1) >= len)
            {
                break;
            }
            /*解析2字节数据,低位在前*/
            value = (s16)((pdata[2 * i + 1] << 8) | pdata[2 * i]);
            /*存储到全局数组*/
            gImiu4_Info[e_IMIU4_2]->AuthStatus[i] = value;
        }
    }
}
/*=================================================================================================
** @Name      : IMIU4_RcvCtrlInfo
** @Input     : pdata:数据指针  len:数据长度  srcAddr:源地址
** @Output    : void
** @Function  : 接收采样信息帧(IMIU4→BMU),解析电压、电流、温度、绝缘阻值数据
** @The notes : 主索引56,每个数据项为s16类型(2字节),低位在前
**===============================================================================================*/
static void IMIU4_RcvCtrlInfo(u8 *pdata, u8 len, u8 srcAddr)
{

    u16 value;
    
    /*参数有效性检查*/
    if(pdata == NULL)
    {
        return;
    }
    /*数据长度检查*/
    if(len < 2)
    {
        return;
    }
    
    /*根据源地址选择存储位置*/
    if(srcAddr == IMIU4_1_DEV_ADDR)
    {
        /*1#IMIU4数据存储*/
        if(gImiu4_Info[e_IMIU4_1] == NULL)
        {
            return;
        }
        gImiu4_Info[e_IMIU4_1]->CtrlInfo = (u16)(pdata[0] | (pdata[1]>>8));
    }else if(srcAddr == IMIU4_2_DEV_ADDR)
    {
        /*1#IMIU4数据存储*/
        if(gImiu4_Info[e_IMIU4_2] == NULL)
        {
            return;
        }
        gImiu4_Info[e_IMIU4_2]->CtrlInfo = (u16)(pdata[0] | (pdata[1]>>8));
    }
}
/*=================================================================================================
** 静态函数: IMIU4_RcvAlmInfo
**===============================================================================================*/
/*=================================================================================================
** @Name      : IMIU4_RcvAlmInfo
** @Input     : pdata:数据指针  len:数据长度  srcAddr:源地址
** @Output    : void
** @Function  : 接收故障告警信息帧(IMIU4→BMU),解析告警位并设置电流采样异常标志
** @The notes : 主索引57,每个数据项为u16类型(2字节),低位在前
**===============================================================================================*/
static void IMIU4_RcvAlmInfo(u8 *pdata, u8 len, u8 srcAddr)
{
    u8 i;
    u16 value;
    
    /*参数有效性检查*/
    if(pdata == NULL)
    {
        return;
    }
    /*数据长度检查*/
    if(len < 2)
    {
        return;
    }
    
    /*根据源地址选择存储位置*/
    if(srcAddr == IMIU4_1_DEV_ADDR)
    {
        /*1#IMIU4故障告警存储*/
        if(gImiu4_Info[e_IMIU4_1] == NULL)
        {
            return;
        }
        /*循环解析告警信息(6个u16元素)*/
        for(i = 0; i < eIMIUAlmInfo_Len; i++)
        {
            /*检查数据边界*/
            if((2 * i + 1) >= len)
            {
                break;
            }
            /*解析2字节数据,低位在前*/
            value = (u16)((pdata[2 * i + 1] << 8) | pdata[2 * i]);
            /*存储到全局数组*/
            gImiu4_Info[e_IMIU4_1]-> AlmFault[i]= value;
        }
        
        /*告警位解析并设置电流采样异常标志*/
        /*bit6: 霍尔过载故障*/
        if((gImiu4_Info[e_IMIU4_1]-> AlmFault[0] & 0x40) != 0)
        {
            SetCurrSampExpFlagAPI(0, eCExp_HalErr, 1, 0);
        }
        else
        {
            SetCurrSampExpFlagAPI(0, eCExp_HalErr, 0, 0);
        }
        /*bit7: 霍尔供电故障*/
        if((gImiu4_Info[e_IMIU4_1]-> AlmFault[0] & 0x80) != 0)
        {
            SetCurrSampExpFlagAPI(0, eCExp_HalOff, 1, 0);
        }
        else
        {
            SetCurrSampExpFlagAPI(0, eCExp_HalOff, 0, 0);
        }
        /*bit8: 霍尔AD值异常*/
        if((gImiu4_Info[e_IMIU4_1]-> AlmFault[0] & 0x100) != 0)
        {
            SetCurrSampExpFlagAPI(0, eCExp_HADErr, 1, 0);
        }
        else
        {
            SetCurrSampExpFlagAPI(0, eCExp_HADErr, 0, 0);
        }
        /*bit9: 分流器采样值异常*/
        if((gImiu4_Info[e_IMIU4_1]-> AlmFault[0] & 0x200) != 0)
        {
            SetCurrSampExpFlagAPI(0, eCExp_SADErr, 1, 0);
        }
        else
        {
            SetCurrSampExpFlagAPI(0, eCExp_SADErr, 0, 0);
        }
        /*bit10: 电流采样霍尔分流器采样值不一致(均使能时)*/
        if((gImiu4_Info[e_IMIU4_1]-> AlmFault[0] & 0x400) != 0)
        {
            SetCurrSampExpFlagAPI(0, eCExp_HSDiff, 1, 0);
        }
        else
        {
            SetCurrSampExpFlagAPI(0, eCExp_HSDiff, 0, 0);
        }
        /*bit1(AlmInfo[1]): 电流采样自检异常*/
        if((gImiu4_Info[e_IMIU4_1]-> AlmFault[1] & 0x02) != 0)
        {
            SetCurrSampExpFlagAPI(0, eCExp_SefErr, 1, 0);
        }
        else
        {
            SetCurrSampExpFlagAPI(0, eCExp_SefErr, 0, 0);
        }
    }
    else if(srcAddr == IMIU4_2_DEV_ADDR)
    {
        /*2#IMIU4故障告警存储*/
        if(gImiu4_Info[e_IMIU4_2] == NULL)
        {
            return;
        }
        /*循环解析告警信息(6个u16元素)*/
        for(i = 0; i < eIMIUAlmInfo_Len; i++)
        {
            /*检查数据边界*/
            if((2 * i + 1) >= len)
            {
                break;
            }
            /*解析2字节数据,低位在前*/
            value = (u16)((pdata[2 * i + 1] << 8) | pdata[2 * i]);
            /*存储到全局数组*/
           gImiu4_Info[e_IMIU4_2]-> AlmFault[i] = value;
        }
        
        /*告警位解析并设置电流采样异常标志*/
        /*bit6: 需尔过载故障*/
        if((gImiu4_Info[e_IMIU4_2]-> AlmFault[0] & 0x40) != 0)
        {
            SetCurrSampExpFlagAPI(1, eCExp_HalErr, 1, 0);
        }
        else
        {
            SetCurrSampExpFlagAPI(1, eCExp_HalErr, 0, 0);
        }
        /*bit7: 需尔供电故障*/
        if((gImiu4_Info[e_IMIU4_2]-> AlmFault[0] & 0x80) != 0)
        {
            SetCurrSampExpFlagAPI(1, eCExp_HalOff, 1, 0);
        }
        else
        {
            SetCurrSampExpFlagAPI(1, eCExp_HalOff, 0, 0);
        }
        /*bit8: 需尔AD值异常*/
        if((gImiu4_Info[e_IMIU4_2]-> AlmFault[0] & 0x100) != 0)
        {
            SetCurrSampExpFlagAPI(1, eCExp_HADErr, 1, 0);
        }
        else
        {
            SetCurrSampExpFlagAPI(1, eCExp_HADErr, 0, 0);
        }
        /*bit9: 分流器采样值异常*/
        if((gImiu4_Info[e_IMIU4_2]-> AlmFault[0] & 0x200) != 0)
        {
            SetCurrSampExpFlagAPI(1, eCExp_SADErr, 1, 0);
        }
        else
        {
            SetCurrSampExpFlagAPI(1, eCExp_SADErr, 0, 0);
        }
        /*bit10: 电流采样需尔分流器采样值不一致(均使能时)*/
        if((gImiu4_Info[e_IMIU4_2]-> AlmFault[0] & 0x400) != 0)
        {
            SetCurrSampExpFlagAPI(1, eCExp_HSDiff, 1, 0);
        }
        else
        {
            SetCurrSampExpFlagAPI(1, eCExp_HSDiff, 0, 0);
        }
        /*bit1(AlmInfo[1]): 电流采样自检异常*/
        if((gImiu4_Info[e_IMIU4_2]-> AlmFault[1] & 0x02) != 0)
        {
            SetCurrSampExpFlagAPI(1, eCExp_SefErr, 1, 0);
        }
        else
        {
            SetCurrSampExpFlagAPI(1, eCExp_SefErr, 0, 0);
        }
    }
}

/*=================================================================================================
** @Name      : IMIU4_RcvSampCalInfo
** @Input     : pdata:数据指针  len:数据长度  srcAddr:源地址
** @Output    : void
** @Function  : 接收采样信息帧(IMIU4→BMU),解析电压、电流、温度、绝缘阻值数据
** @The notes : 主索引58,每个数据项为s16类型(2字节),低位在前
**===============================================================================================*/
static void IMIU4_RcvSampCalInfo(u8 *pdata, u8 len, u8 srcAddr)
{
    u16 value;
    u8 i;
    /*参数有效性检查*/
    if(pdata == NULL)
    {
        return;
    }
    /*数据长度检查*/
    if(len < 2)
    {
        return;
    }
    
    /*根据源地址选择存储位置*/
    if(srcAddr == IMIU4_1_DEV_ADDR)
    {
        /*1#IMIU4数据存储*/
        if(gImiu4_Info[e_IMIU4_1] == NULL)
        {
            return;
        }
        /*循环解析采样信息(3个u16元素)*/
        for(i = 0; i < eIMIUSampCalInfo_Len; i++)
        {
            /*检查数据边界*/
            if((2 * i + 1) >= len)
            {
                break;
            }
            /*解析2字节数据,低位在前*/
            value = (s16)((pdata[2 * i + 1] << 8) | pdata[2 * i]);
            /*存储到全局数组*/
            gImiu4_Info[e_IMIU4_1]->SampCalInfo[i] = value;
        }
    }else if(srcAddr == IMIU4_2_DEV_ADDR)
    {
        /*1#IMIU4数据存储*/
        if(gImiu4_Info[e_IMIU4_2] == NULL)
        {
            return;
        }
        /*循环解析采样信息(3个u16元素)*/
        for(i = 0; i < eIMIUSampCalInfo_Len; i++)
        {
            /*检查数据边界*/
            if((2 * i + 1) >= len)
            {
                break;
            }
            /*解析2字节数据,低位在前*/
            value = (s16)((pdata[2 * i + 1] << 8) | pdata[2 * i]);
            /*存储到全局数组*/
            gImiu4_Info[e_IMIU4_2]->SampCalInfo[i] = value;
        }
    }
}

/*=================================================================================================
** @Name      : IMIU4_RcvSampCalInfo
** @Input     : pdata:数据指针  len:数据长度  srcAddr:源地址
** @Output    : void
** @Function  : 接收采样信息帧(IMIU4→BMU),解析电压、电流、温度、绝缘阻值数据
** @The notes : 主索引61,每个数据项为s16类型(2字节),低位在前
**===============================================================================================*/
static void IMIU4_RcvSysDebugInfo(u8 *pdata, u8 len, u8 srcAddr)
{
    u16 value;
    u8 i;
    /*参数有效性检查*/
    if(pdata == NULL)
    {
        return;
    }
    /*数据长度检查*/
    if(len < 2)
    {
        return;
    }
    
    /*根据源地址选择存储位置*/
    if(srcAddr == IMIU4_1_DEV_ADDR)
    {
        /*1#IMIU4数据存储*/
        if(gImiu4_Info[e_IMIU4_1] == NULL)
        {
            return;
        }
        /*循环解析采样信息(3个u16元素)*/
        for(i = 0; i < eIMIUSampCalInfo_Len; i++)
        {
            /*检查数据边界*/
            if((2 * i + 1) >= len)
            {
                break;
            }
            /*解析2字节数据,低位在前*/
            value = (s16)((pdata[2 * i + 1] << 8) | pdata[2 * i]);
            /*存储到全局数组*/
            gImiu4_Info[e_IMIU4_1]->SysDebugInfo[i] = value;
        }
    }else if(srcAddr == IMIU4_2_DEV_ADDR)
    {
        /*1#IMIU4数据存储*/
        if(gImiu4_Info[e_IMIU4_2] == NULL)
        {
            return;
        }
        /*循环解析采样信息(3个u16元素)*/
        for(i = 0; i < eIMIUSampCalInfo_Len; i++)
        {
            /*检查数据边界*/
            if((2 * i + 1) >= len)
            {
                break;
            }
            /*解析2字节数据,低位在前*/
            value = (s16)((pdata[2 * i + 1] << 8) | pdata[2 * i]);
            /*存储到全局数组*/
            gImiu4_Info[e_IMIU4_2]->SysDebugInfo[i] = value;
        }
    }
}
/*=================================================================================================
** 静态函数: IMIU4_RcvDeviceInfo
**===============================================================================================*/
/*=================================================================================================
** @Name      : IMIU4_RcvDeviceInfo
** @Input     : pdata:数据指针  len:数据长度  srcAddr:源地址
** @Output    : void
** @Function  : 接收设备信息帧(IMIU4→BMU),提取设备ID和版本信息
** @The notes : 主索引2,数据长度需至少8字节,包含设备ID、软件版本和硬件版本
**===============================================================================================*/
static void IMIU4_RcvDeviceInfo(u8 *pdata, u8 len, u8 srcAddr)
{
    u16 value;
    u8 i;
  
    /*参数有效性检查*/
    if(pdata == NULL)
    {
        return;
    }
    /*数据长度检查*/
    if(len < 8)
    {
        return;
    }
        /*根据源地址选择存储位置*/
    if(srcAddr == IMIU4_1_DEV_ADDR)
    {
        /*1#IMIU4数据存储*/
        if(gImiu4_Info[e_IMIU4_1] == NULL)
        {
            return;
        }
        /*循环解析采样信息(3个u16元素)*/
        for(i = 0; i < eIMIUDeviceInfo_Len; i++)
        {
            /*检查数据边界*/
            if((2 * i + 1) >= len)
            {
                break;
            }
            /*解析2字节数据,低位在前*/
            value = (u16)((pdata[2 * i + 1] << 8) | pdata[2 * i]);
            /*存储到全局数组*/
            gImiu4_Info[e_IMIU4_1]->DeviceInfo[i] = value;
        }
    }else if(srcAddr == IMIU4_2_DEV_ADDR)
    {
        /*1#IMIU4数据存储*/
        if(gImiu4_Info[e_IMIU4_2] == NULL)
        {
            return;
        }
        /*循环解析采样信息(3个u16元素)*/
        for(i = 0; i < eIMIUDeviceInfo_Len; i++)
        {
            /*检查数据边界*/
            if((2 * i + 1) >= len)
            {
                break;
            }
            /*解析2字节数据,低位在前*/
            value = (u16)((pdata[2 * i + 1] << 8) | pdata[2 * i]);
            /*存储到全局数组*/
            gImiu4_Info[e_IMIU4_2]->DeviceInfo[i] = value;
        }
    }
}

/*=================================================================================================
** 静态函数: IMIU4_RcvDeviceInfo
**===============================================================================================*/
/*=================================================================================================
** @Name      : IMIU4_RcvDeviceInfo
** @Input     : pdata:数据指针  len:数据长度  srcAddr:源地址
** @Output    : void
** @Function  : 接收设备信息帧(IMIU4→BMU),提取设备ID和版本信息
** @The notes : 主索引2,数据长度需至少8字节,包含设备ID、软件版本和硬件版本
**===============================================================================================*/
static void IMIU4_RcvSerNum(u8 *pdata, u8 len, u8 srcAddr)
{
    u8 i;
  
    /*参数有效性检查*/
    if(pdata == NULL)
    {
        return;
    }
    /*数据长度检查*/
    if(len < 8)
    {
        return;
    }
        /*根据源地址选择存储位置*/
    if(srcAddr == IMIU4_1_DEV_ADDR)
    {
        /*1#IMIU4数据存储*/
        if(gImiu4_Info[e_IMIU4_1] == NULL)
        {
            return;
        }
        /*序列号为23字节ASCII,逐字节存储*/
        for(i = 0; i < 23; i++)
        {
            /*检查数据边界*/
            if(i >= len)
            {
                break;
            }
            gImiu4_Info[e_IMIU4_1]->SerNum[i] = pdata[i];
        }
    }else if(srcAddr == IMIU4_2_DEV_ADDR)
    {
        /*2#IMIU4数据存储*/
        if(gImiu4_Info[e_IMIU4_2] == NULL)
        {
            return;
        }
        /*序列号为23字节ASCII,逐字节存储*/
        for(i = 0; i < 23; i++)
        {
            /*检查数据边界*/
            if(i >= len)
            {
                break;
            }
            gImiu4_Info[e_IMIU4_2]->SerNum[i] = pdata[i];
        }
    }
}


/*=================================================================================================
** @Name      : IMIU4_RcvCtrlInfo
** @Input     : pdata:数据指针  len:数据长度  srcAddr:源地址
** @Output    : void
** @Function  : 接收采样信息帧(IMIU4→BMU),解析电压、电流、温度、绝缘阻值数据
** @The notes : 主索引56,每个数据项为s16类型(2字节),低位在前
**===============================================================================================*/
static void IMIU4_RcvSystemStatus(u8 *pdata, u8 len, u8 srcAddr)
{

    u16 value;
    
    /*参数有效性检查*/
    if(pdata == NULL)
    {
        return;
    }
    /*数据长度检查*/
    if(len < 2)
    {
        return;
    }
    
    /*根据源地址选择存储位置*/
    if(srcAddr == IMIU4_1_DEV_ADDR)
    {
        /*1#IMIU4数据存储*/
        if(gImiu4_Info[e_IMIU4_1] == NULL)
        {
            return;
        }
        gImiu4_Info[e_IMIU4_1]->WorkMode = (u16)(pdata[0] | (pdata[1]>>8));
    }else if(srcAddr == IMIU4_2_DEV_ADDR)
    {
        /*1#IMIU4数据存储*/
        if(gImiu4_Info[e_IMIU4_2] == NULL)
        {
            return;
        }
        gImiu4_Info[e_IMIU4_2]->WorkMode = (u16)(pdata[0] | (pdata[1]>>8));
    }
}
/*=================================================================================================
** 静态函数: IMIU4_RcvQueryReply
**===============================================================================================*/
/*=================================================================================================
** @Name      : IMIU4_RcvQueryReply
** @Input     : pdata:数据指针  len:数据长度  pf:主索引  srcAddr:源地址
** @Output    : void
** @Function  : 接收查询回复帧(IMIU4→BMU),功能码6,解析IMIU4回复的查询数据
** @The notes : BMU发送查询请求(功能码5)后,IMIU4回复查询数据(功能码6)
**              根据PF字段分发到对应的采样信息或设备信息处理函数
**===============================================================================================*/
static void IMIU4_RcvQueryReply(u8 *pdata, u8 len, u8 pf, u8 srcAddr)
{
    /*参数有效性检查*/
    if(pdata == NULL)
    {
        return;
    }
    /*数据长度检查*/
    if(len < 4)
    {
        return;
    }
    
    /*根据PF字段分发处理*/
    switch(pf)
    {
        /*主索引1: 设备信息查询回复*/
        case eIMIU4Idx_DeviceInfo:
            /*调用设备信息处理函数,跳过帧格式字节(Data[0-1])*/
            IMIU4_RcvDeviceInfo(&pdata[4], len - 4, srcAddr);
            gImiu4DevInfoReplyFlag = 1;
            ICCVerSrlReplyNotify(ICC_DEVTYPE_IMIU4, VSRL_EVT_DEVINFO);
            break;

         /*主索引2: 序列号*/
        case eIMIU4Idx_SerNum:
            /*调用设备信息处理函数,跳过帧格式字节(Data[0-1])*/
            IMIU4_RcvSerNum(&pdata[4], len - 4, srcAddr);
            gImiu4SerialReplyFlag = 1;
            ICCVerSrlReplyNotify(ICC_DEVTYPE_IMIU4, VSRL_EVT_SERIAL);
            break;       
        
        /*主索引50: 系统状态查询回复*/
        case eIMIU4Idx_SystemStatus:
            IMIU4_RcvSystemStatus(&pdata[4], len - 4, srcAddr);
            /*系统状态暂不处理*/
            break;
        
        
        /*其他主索引暂不处理*/
        default:
            break;
    }
}

/*=================================================================================================
** 静态函数: IMIU4_RcvModifyReply
**===============================================================================================*/
/*=================================================================================================
** @Name      : IMIU4_RcvModifyReply
** @Input     : pdata:数据指针  len:数据长度  pf:主索引
** @Output    : void
** @Function  : 接收修改回复帧(IMIU4→BMU),功能码3,解析IMIU4回复的修改结果
** @The notes : BMU发送修改请求(功能码2)后,IMIU4回复修改结果(功能码3)
**              Data[0]=帧格式标识, Data[1]=子索引数目, Data[2]=子索引, Data[3-]=修改结果值
**===============================================================================================*/
static void IMIU4_RcvModifyReply(u8 *pdata, u8 len, u8 pf)
{
    u8 frameType;
    u8 subIdxNum;
    u8 subIdx;
    u8 result;
    
    /*参数有效性检查*/
    if(pdata == NULL)
    {
        return;
    }
    /*数据长度检查: 至少需要4字节(帧格式+子索引数目+子索引+结果)*/
    if(len < 4)
    {
        return;
    }
    
    /*解析帧格式标识: Data[0]*/
    frameType = pdata[0];
    /*解析子索引数目: Data[1]*/
    subIdxNum = pdata[1];
    /*解析子索引: Data[2]*/
    subIdx = pdata[2];
    /*解析修改结果: Data[3]*/
    result = pdata[3];
    
    /*根据PF字段处理不同修改回复*/
    switch(pf)
    {
        /*主索引55: 控制命令修改回复(继电器/绝缘检测)*/
        case eIMIU4Idx_Control:
            /*修改结果: 0成功  其他失败*/
            if(result == 0)
            {
                /*修改成功,可根据subIdx判断具体控制项*/
                /*subIdx=0: 绝缘检测控制成功*/
                /*subIdx=1: 继电器控制成功*/
            }
            else
            {
                /*修改失败,可记录错误日志*/
            }
            break;
        
        /*主索引60: 权限控制修改回复*/
        case eIMIU4Idx_AuthCtrl:
            /*权限切换结果*/
            if(result == 0)
            {
                /*权限切换成功*/
            }
            else
            {
                /*权限切换失败*/
            }
            break;
        
        /*主索引62: 系统调试修改回复*/
        case eIMIU4Idx_SysDebug:
            /*调试命令执行结果*/
            if(result == 0)
            {
                /*调试命令执行成功*/
            }
            else
            {
                /*调试命令执行失败*/
            }
            break;
        
        /*其他主索引暂不处理*/
        default:
            break;
    }
}

/*=================================================================================================
** 静态函数: IMIU4_RcvErrorFrame
**===============================================================================================*/
/*=================================================================================================
** @Name      : IMIU4_RcvErrorFrame
** @Input     : pdata:数据指针  len:数据长度  pf:PDU格式字段
** @Output    : void
** @Function  : 接收错误帧(IMIU4→BMU),解析错误索引和错误码
** @The notes : 主索引102,数据长度需至少2字节
**===============================================================================================*/
static void IMIU4_RcvErrorFrame(u8 *pdata, u8 len, u8 pf)
{
    u8 errorCode;
    u8 errorIndex;
    
    /*参数有效性检查*/
    if(pdata == NULL)
    {
        return;
    }
    /*数据长度检查*/
    if(len < 2)
    {
        return;
    }
    /*解析错误索引: 字节0*/
    errorIndex = pdata[0];
    /*解析错误码: 字节1*/
    errorCode = pdata[1];
}

/*=================================================================================================
** 公共函数: IMIU4_RcvHandle
**===============================================================================================*/
/*=================================================================================================
** @Name      : IMIU4_RcvHandle
** @Input     : ID:CAN标识符  pdata:数据指针  DLC:数据长度  canNum:CAN通道号
** @Output    : void
** @Function  : IMIU4 CANFD接收处理主函数(IMIU4→BMU),解析CAN帧并根据PF字段分发到对应处理函数
** @The notes : 仅处理CAN2通道,DLC最大限制64字节,根据PF字段分发到不同处理函数
**===============================================================================================*/
void IMIU4_RcvHandle(u32 ID, u8 *pdata, u8 DLC, u8 canNum)
{
    t_IMIU4Frame frame;
    
    /*初始化帧长度*/
    frame.len = 0;
    
    /*参数有效性检查*/
    if(pdata == NULL)
    {
        return;
    }
    /*数据长度检查*/
    if(DLC == 0)
    {
        return;
    }
    
    /*解析CAN ID*/
    IMIU4_ParseCanId(ID, &frame);
    
    /*检查地址有效性*/
    if(IMIU4_CheckAddress(&frame, canNum) == FALSE)
    {
        return;
    }
    
    /*CANFD最大64字节限制*/
    if(DLC > 64)
    {
        DLC = 64;
    }
    
    /*复制数据到帧结构*/
    memcpy(frame.data, pdata, DLC);
    frame.len = DLC;
    
    /*调试打印: 开启时打印CAN帧详细信息*/
    if(gImiu4DebugPrintEn == 1)
    {
        u8 i;
        printf("[IMIU4 RX] ID=0x%08X PR=%d DP=%d PF=%d PS=%d SA=%d LEN=%d\r\n",
               ID, frame.pr, frame.dp, frame.pf, frame.ps, frame.sa, frame.len);
        printf("[IMIU4 RX] Data: ");
        for(i = 0; i < frame.len; i++)
        {
            printf("%02X ", frame.data[i]);
        }
        printf("\r\n");
    }
    
    /*根据功能码分发处理*/
    switch(frame.pr)
    {
        /*功能码3: 修改回复(IMIU4→BMU),IMIU4回复BMU的修改请求*/
        case eIMIU4Func_ModifyReply:
            IMIU4_RcvModifyReply(frame.data, frame.len, frame.pf);
            break;
        
        /*功能码6: 查询回复(IMIU4→BMU),IMIU4回复BMU的查询请求*/
        case eIMIU4Func_QueryReply:
            IMIU4_RcvQueryReply(frame.data, frame.len, frame.pf, frame.sa);
            break;
        
        /*功能码10: 单帧上传(IMIU4→BMU),IMIU4周期上报数据*/
        case eIMIU4Func_Reserved10:
            /*根据PF字段分发处理主动上报数据*/
            switch(frame.pf)
            {               
                /*主索引51: 采样信息*/
                case eIMIU4Idx_SampInfo:
                    IMIU4_RcvSampInfo(frame.data, frame.len, frame.sa);
                    break;

                /*主索引54: 权限级别状态*/    
                case eIMIU4Idx_AuthStatus:
                    IMIU4_RcvAuthStatus(frame.data, frame.len, frame.sa);
                    break;  

                /*主索引56: 控制命令反馈信息*/    
                case eIMIU4Idx_CtrlInfo:
                    IMIU4_RcvCtrlInfo(frame.data, frame.len, frame.sa);
                    break;                

                /*主索引57: 故障告警信息*/
                case eIMIU4Idx_AlmFault:
                    IMIU4_RcvAlmInfo(frame.data, frame.len, frame.sa);
                    break;
                /*主索引58: 采样校准反馈信息*/
                case eIMIU4Idx_SampCalInfo:
                    IMIU4_RcvSampCalInfo(frame.data, frame.len, frame.sa);
                    break;
                /*主索引61: 系统调试反馈信息*/
                case eIMIU4Idx_SysDebugInfo:
                    IMIU4_RcvSysDebugInfo(frame.data, frame.len, frame.sa);
                    break;                    
                /*其他主索引暂不处理*/
                default:
                    break;
            }
            break;
        
        /*功能码9: 错误应答(IMIU4→BMU),IMIU4回复错误信息*/
        case eIMIU4Func_ErrorResponse:
            IMIU4_RcvErrorFrame(frame.data, frame.len, frame.pf);
            break;
        
        /*其他功能码暂不处理*/
        default:
            break;
    }
}