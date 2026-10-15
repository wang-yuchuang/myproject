/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IccMsgVerSrl.c
** @Founder      : ZQD808
** @Date         : 2026.09.11
** @Function     : 版本号/序列号主索引(81/82/83/84)查询修改核心实现
** @Instructions : 事件驱动查询下级设备, 结果写回数组后组帧回03功能码, 超时自动重发
**===============================================================================================*/
/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "IccMsgVerSrl.h"
#include "IccMsgUser.h"
#include "IccMsgHandle.h"
#include "IccMsgInterface.h"
#include "BSPUserAPI.h"
#include "IMEU3Interface.h"
#include "IMIU4Interface.h"
#include "IMIU4Config.h"
#include "SlaveMsg.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include <string.h>
#include <cmsis_os2.h>

/*=================================================================================================
** @Global variable definition
**===============================================================================================*/
/*核间通信主索引81/82/83/84数据缓冲(ICCDeviceTableInit注册引用)*/
u8 gIccVerSrl_81[ICC_MINDEX81_VER_LEN];
u8 gIccDevSerial_82[ICC_MINDEX82_SERIAL_LEN];
u8 gIccPackSerial_83[ICC_MINDEX83_PACKSERIAL_LEN];
u8 gIccCellSerial_84[ICC_MINDEX84_CELLSERIAL_LEN];

/*回复帧缓冲: 8字节ID域 + 3字节数据域标识 + 数据域(最大1012字节)*/
static u8 sReplyFrame[ICC_MINDEX84_CELLSERIAL_LEN + 11];

/*下级设备回复缓存与标志(各模块定义)*/
extern u16 *gImeu3DevAddr;
extern u8  *gImeu3DevSerial;
extern volatile u8 gEU3DevInfoReplyFlag;
extern volatile u8 gEU3SerialReplyFlag;
extern gImiu4_data *gImiu4_Info[e_IMIU4_NUM];
extern volatile u8 gImiu4DevInfoReplyFlag;
extern volatile u8 gImiu4SerialReplyFlag;

/*=================================================================================================
** @Static function declaration
**===============================================================================================*/
static void VerSrlSendReply(u8 saddr, u8 mIndex, u8 *payload, u16 payloadLen, u8 sindexNum, u8 iccNum);
static u8 VerSrl81Deal(u8 saddr, u8 iccNum);
static u8 VerSrl82Deal(u8 saddr, u8 iccNum);
static u8 VerSrl83Deal(u8 saddr, u8 iccNum);
static u8 VerSrl84Deal(u8 saddr, u8 iccNum);
static u8 GetImiu4Idx(u8 devAddr);
static void VerSrl81Reply(void);
static void VerSrl82Reply(void);
static void VerSrl83ReadReply(void);
static void VerSrl83WriteAdvance(void);
static void VerSrl83AuthAdvance(void);
static void VerSrl83Reply(void);
static void VerSrl84Advance(void);
static void VerSrl84Reply(void);
static void VerSrlResendQuery(u8 slot);
static void VerSrlTimeoutReply(u8 slot);

/*主索引→挂起槽位映射: 81→0 82→1 83→2 84→3*/
#define VSRL_SLOT(mIndex) ((u8)((mIndex) - 81))

/*待回复查询上下文: 钩子下发CAN查询后立即返回PENDING, 收到具体报文后由ICCVerSrlReplyNotify组帧回复A核*/
typedef struct
{
    u32 deadline;      /*本次往返超时截止tick(ms)*/
    u8 active;         /*1=有待回复查询*/
    u8 saddr;          /*A核源地址*/
    u8 iccNum;         /*ICC通道号*/
    u8 devType;        /*下级设备类型*/
    u8 devAddr;        /*下级设备地址*/
    u8 opType;         /*操作类型(1查询/4设置)*/
    u8 step;           /*83写入状态机: 0=等权限开回复 1=等写入回复 2=等权限关回复*/
    u8 cellCur;        /*84当前已收电芯序号*/
    u8 cellTotal;      /*84待查电芯总数*/
    u8 retryCnt;       /*本次往返已重发次数*/
} t_VerSrlPending;

/*每主索引独立挂起槽位: [0]=81 [1]=82 [2]=83 [3]=84, 互不覆盖*/
static t_VerSrlPending sPend[4];

/*=================================================================================================
** @Function  : GetImiu4Idx
** @Input     : devAddr:IMIU4设备地址
** @Output    : IMIU4数组下标(e_IMIU4_1/e_IMIU4_2)
** @Function  : 设备地址转IMIU4数组下标
**===============================================================================================*/
static u8 GetImiu4Idx(u8 devAddr)
{
    if(IMIU4_2_DEV_ADDR == devAddr)
    {
        return e_IMIU4_2;
    }
    return e_IMIU4_1;
}

/*=================================================================================================
** @Function  : VerSrlSendReply
** @Input     : saddr:源地址 mIndex:主索引 payload:回复数据 payloadLen:数据长度 iccNum:ICC通道
** @Output    : void
** @Function  : 组帧发送03功能码(修改回复)报文
**===============================================================================================*/
static void VerSrlSendReply(u8 saddr, u8 mIndex, u8 *payload, u16 payloadLen, u8 sindexNum, u8 iccNum)
{
    u16 framLen = (u16)(11 + payloadLen);

    ICCBuildMsgID_API(framLen, eICCFUN_AUP, mIndex, saddr, sReplyFrame);
    ICCBuildMsgDataMark_API(0x0c, sindexNum, 0, &sReplyFrame[8]);
    if(payloadLen > 0)
    {
        memcpy(&sReplyFrame[11], payload, payloadLen);
    }
    ICCSendMsgUser_Hook(sReplyFrame, framLen, iccNum);
}

/*=================================================================================================
** @Function  : VerSrlResendQuery
** @Input     : slot:挂起槽位(0=81 1=82 2=83 3=84)
** @Output    : void
** @Function  : 按当前挂起状态(设备类型/83状态机步/84电芯序号)下发一次查询并刷新超时截止时间
**===============================================================================================*/
static void VerSrlResendQuery(u8 slot)
{
    t_VerSrlPending *p = &sPend[slot];

    switch(slot)
    {
        case VSRL_SLOT(81):
            if(ICC_DEVTYPE_BSU == p->devType)
            {
                gBsuDevInfoReplyFlag = 0;
                BSU_SendDevInfoQuery(p->devAddr);
            }
            else if(ICC_DEVTYPE_IMIU4 == p->devType)
            {
                gImiu4DevInfoReplyFlag = 0;
                IMIU4_SendDeviceInfoQuery(p->devAddr);
            }
            else if(ICC_DEVTYPE_IMEU3 == p->devType)
            {
                gEU3DevInfoReplyFlag = 0;
                IMEU3_SendDeviceInfoQuery(p->devAddr);
            }
            break;

        case VSRL_SLOT(82):
            if(ICC_DEVTYPE_BSU == p->devType)
            {
                gBsuSerialReplyFlag = 0;
                BSU_SendSerialQuery(p->devAddr);
            }
            else if(ICC_DEVTYPE_IMIU4 == p->devType)
            {
                gImiu4SerialReplyFlag = 0;
                IMIU4_SendSerNumQuery(p->devAddr);
            }
            else if(ICC_DEVTYPE_IMEU3 == p->devType)
            {
                gEU3SerialReplyFlag = 0;
                IMEU3_SendDevSerialQuery(p->devAddr);
            }
            break;

        case VSRL_SLOT(83):
            if(ICC_OPTYPE_SET == p->opType)
            {
                if(0 == p->step)
                {
                    gBsuAuthReplyFlag = 0;
                    BSU_SendAuthCtrl(p->devAddr, 1);
                }
                else if(1 == p->step)
                {
                    gBsuPackSerialReplyFlag = 0;
                    BSU_SendPackSerialWrite(p->devAddr, &gIccPackSerial_83[3]);
                }
                else
                {
                    gBsuAuthReplyFlag = 0;
                    BSU_SendAuthCtrl(p->devAddr, 0);
                }
            }
            else
            {
                gBsuPackSerialReplyFlag = 0;
                BSU_SendPackSerialQuery(p->devAddr);
            }
            break;

        case VSRL_SLOT(84):
            gBsuCellSerialReplyFlag = 0;
            BSU_SendCellSerialQuery(p->devAddr, (u8)(gIccCellSerial_84[1] + p->cellCur));
            break;

        default:
            return;
    }

    p->deadline = BSPGetOSMillSecTimeAPI() + ICC_VSRL_TIMEOUT_MS;
}

/*=================================================================================================
** @Function  : VerSrlTimeoutReply
** @Input     : slot:挂起槽位(0=81 1=82 2=83 3=84)
** @Output    : void
** @Function  : 重发次数用尽后组帧回查询失败/设置失败(数据区清零)
**===============================================================================================*/
static void VerSrlTimeoutReply(u8 slot)
{
    t_VerSrlPending *p = &sPend[slot];
    u8 errCode = (ICC_OPTYPE_SET == p->opType) ? ICC_OPTYPE_SET_ERR : ICC_OPTYPE_QUERY_ERR;

    switch(slot)
    {
        case VSRL_SLOT(81):
            gIccVerSrl_81[2] = errCode;
            memset(&gIccVerSrl_81[4], 0, ICC_MINDEX81_VER_LEN - 4);
            VerSrlSendReply(p->saddr, 81, gIccVerSrl_81, ICC_MINDEX81_VER_LEN, ICC_MINDEX81_VER_LEN, p->iccNum);
            break;

        case VSRL_SLOT(82):
            gIccDevSerial_82[2] = errCode;
            memset(&gIccDevSerial_82[3], 0, ICC_MINDEX82_SERIAL_LEN - 3);
            VerSrlSendReply(p->saddr, 82, gIccDevSerial_82, ICC_MINDEX82_SERIAL_LEN, ICC_MINDEX82_SERIAL_LEN, p->iccNum);
            break;

        case VSRL_SLOT(83):
            gIccPackSerial_83[2] = errCode;
            memset(&gIccPackSerial_83[3], 0, ICC_MINDEX83_PACKSERIAL_LEN - 3);
            VerSrlSendReply(p->saddr, 83, gIccPackSerial_83, ICC_MINDEX83_PACKSERIAL_LEN, ICC_MINDEX83_PACKSERIAL_LEN, p->iccNum);
            break;

        case VSRL_SLOT(84):
            gIccCellSerial_84[3] = errCode;
            memset(&gIccCellSerial_84[4], 0, (u32)p->cellTotal * BSU_CELLSERIAL_LEN);
            VerSrlSendReply(p->saddr, 84, gIccCellSerial_84, (u16)(4 + p->cellTotal * BSU_CELLSERIAL_LEN), 4, p->iccNum);
            break;

        default:
            break;
    }
}

/*=================================================================================================
** @Function  : VerSrl81Deal
** @Input     : saddr:源地址 iccNum:ICC通道
** @Output    : eICCERRREAS_PENDING
** @Function  : 81 下级设备软硬件版本号查询
**===============================================================================================*/
static u8 VerSrl81Deal(u8 saddr, u8 iccNum)
{
    t_VerSrlPending *p = &sPend[VSRL_SLOT(81)];
    u8 devType = gIccVerSrl_81[0];   /*子索引0=下级设备类型(1 BSU/5 IMIU4/6 IMEU3)*/
    u8 devAddr = gIccVerSrl_81[1];   /*子索引1=下级设备地址*/

    p->active = 1;
    p->saddr = saddr;
    p->iccNum = iccNum;
    p->devType = devType;
    p->devAddr = devAddr;
    p->opType = gIccVerSrl_81[2];
    p->retryCnt = 0;

    VerSrlResendQuery(VSRL_SLOT(81));
    return eICCERRREAS_PENDING;
}

/*=================================================================================================
** @Function  : VerSrl81Reply
** @Input     : void
** @Output    : void
** @Function  : 收到81设备信息回复后写回数组组帧回复A核
**===============================================================================================*/
static void VerSrl81Reply(void)
{
    t_VerSrlPending *p = &sPend[VSRL_SLOT(81)];
    u8 devType = p->devType;
    u8 devAddr = p->devAddr;
    u8 verType = gIccVerSrl_81[3];   /*子索引3=版本类型(0软件版本 1硬件版本)*/
    u8 ok = 0;
    u16 hwVer = 0;
    u16 swVerL = 0;
    u16 swVerH = 0;

    if(ICC_DEVTYPE_BSU == devType)
    {
        hwVer  = gBsuDevInfo[2];
        swVerL = gBsuDevInfo[3];
        swVerH = gBsuDevInfo[4];
        ok = 1;
    }
    else if(ICC_DEVTYPE_IMIU4 == devType)
    {
        u8 imiu4Idx = GetImiu4Idx(devAddr);
        if(gImiu4_Info[imiu4Idx] != NULL)
        {
            hwVer  = gImiu4_Info[imiu4Idx]->DeviceInfo[e_IMIUDeviceInfo_HardVer];
            swVerL = gImiu4_Info[imiu4Idx]->DeviceInfo[e_IMIUDeviceInfo_LSoftVer];
            swVerH = gImiu4_Info[imiu4Idx]->DeviceInfo[e_IMIUDeviceInfo_HSoftVer];
            ok = 1;
        }
    }
    else if(ICC_DEVTYPE_IMEU3 == devType)
    {
        if(gImeu3DevAddr != NULL)
        {
            hwVer  = gImeu3DevAddr[eIMEUDevAddr_HwVer];
            swVerL = gImeu3DevAddr[eIMEUDevAddr_SwVerL];
            swVerH = gImeu3DevAddr[eIMEUDevAddr_SwVerH];
            ok = 1;
        }
    }

    gIccVerSrl_81[0] = devType;
    gIccVerSrl_81[1] = devAddr;
    gIccVerSrl_81[2] = ok ? ICC_OPTYPE_QUERY_OK : ICC_OPTYPE_QUERY_ERR;
    gIccVerSrl_81[3] = verType;
    if(0 == verType)
    {
        u32 swVer = ((u32)swVerH << 16) | (u32)swVerL;
        gIccVerSrl_81[4] = (u8)swVer;         /*软件版本L(低8bit)*/
        gIccVerSrl_81[5] = (u8)(swVer >> 8);  /*软件版本L(高8bit)*/
        gIccVerSrl_81[6] = (u8)(swVer >> 16); /*软件版本H(低8bit)*/
        gIccVerSrl_81[7] = (u8)(swVer >> 24); /*软件版本H(高8bit)*/
    }
    else
    {
        gIccVerSrl_81[4] = (u8)hwVer;         /*硬件版本低8bit*/
        gIccVerSrl_81[5] = (u8)(hwVer >> 8);  /*硬件版本高8bit*/
        gIccVerSrl_81[6] = 0;
        gIccVerSrl_81[7] = 0;
    }

    VerSrlSendReply(p->saddr, 81, gIccVerSrl_81, ICC_MINDEX81_VER_LEN, ICC_MINDEX81_VER_LEN, p->iccNum);
    p->active = 0;
}

/*=================================================================================================
** @Function  : VerSrl82Deal
** @Input     : saddr:源地址 iccNum:ICC通道
** @Output    : eICCERRREAS_PENDING
** @Function  : 82 下级设备序列号查询
**===============================================================================================*/
static u8 VerSrl82Deal(u8 saddr, u8 iccNum)
{
    t_VerSrlPending *p = &sPend[VSRL_SLOT(82)];
    u8 devType = gIccDevSerial_82[0];   /*子索引0=下级设备类型*/
    u8 devAddr = gIccDevSerial_82[1];   /*子索引1=下级设备地址*/

    p->active = 1;
    p->saddr = saddr;
    p->iccNum = iccNum;
    p->devType = devType;
    p->devAddr = devAddr;
    p->opType = gIccDevSerial_82[2];
    p->retryCnt = 0;

    VerSrlResendQuery(VSRL_SLOT(82));
    return eICCERRREAS_PENDING;
}

/*=================================================================================================
** @Function  : VerSrl82Reply
** @Input     : void
** @Output    : void
** @Function  : 收到82序列号回复后写回数组组帧回复A核
**===============================================================================================*/
static void VerSrl82Reply(void)
{
    t_VerSrlPending *p = &sPend[VSRL_SLOT(82)];
    u8 devType = p->devType;
    u8 i;

    gIccDevSerial_82[0] = devType;
    gIccDevSerial_82[1] = p->devAddr;
    gIccDevSerial_82[2] = ICC_OPTYPE_QUERY_OK;

    for(i = 0; i < BSU_SERIAL_LEN; i++)
    {
        gIccDevSerial_82[3 + i] = 0;
    }

    if(ICC_DEVTYPE_BSU == devType)
    {
        for(i = 0; i < BSU_SERIAL_LEN; i++) gIccDevSerial_82[3 + i] = gBsuSerial[i];
    }
    else if(ICC_DEVTYPE_IMIU4 == devType)
    {
        u8 imiu4Idx = GetImiu4Idx(p->devAddr);
        if(gImiu4_Info[imiu4Idx] != NULL)
        {
            for(i = 0; i < BSU_SERIAL_LEN; i++) gIccDevSerial_82[3 + i] = gImiu4_Info[imiu4Idx]->SerNum[i];
        }
    }
    else
    {
        if(gImeu3DevSerial != NULL)
        {
            for(i = 0; i < BSU_SERIAL_LEN; i++) gIccDevSerial_82[3 + i] = gImeu3DevSerial[i];
        }
    }

    VerSrlSendReply(p->saddr, 82, gIccDevSerial_82, ICC_MINDEX82_SERIAL_LEN, ICC_MINDEX82_SERIAL_LEN, p->iccNum);
    p->active = 0;
}

/*=================================================================================================
** @Function  : VerSrl83Deal
** @Input     : saddr:源地址 iccNum:ICC通道
** @Output    : eICCERRREAS_PENDING
** @Function  : 83 PACK序列号查询/修改
**===============================================================================================*/
static u8 VerSrl83Deal(u8 saddr, u8 iccNum)
{
    t_VerSrlPending *p = &sPend[VSRL_SLOT(83)];
    u8 devType = gIccPackSerial_83[0];   /*子索引0=下级设备类型*/
    u8 devAddr = gIccPackSerial_83[1];   /*子索引1=下级设备地址*/
    u8 opType  = gIccPackSerial_83[2];   /*子索引2=操作类型(1查询 4设置)*/

    /*非BSU设备不支持83: 直接回错误应答*/
    if(ICC_DEVTYPE_BSU != devType)
    {
        gIccPackSerial_83[2] = (ICC_OPTYPE_SET == opType) ? ICC_OPTYPE_SET_ERR : ICC_OPTYPE_QUERY_ERR;
        memset(&gIccPackSerial_83[3], 0, ICC_MINDEX83_PACKSERIAL_LEN - 3);
        VerSrlSendReply(saddr, 83, gIccPackSerial_83, ICC_MINDEX83_PACKSERIAL_LEN, ICC_MINDEX83_PACKSERIAL_LEN, iccNum);
        return eICCERRREAS_PENDING;
    }

    p->active = 1;
    p->saddr = saddr;
    p->iccNum = iccNum;
    p->devType = devType;
    p->devAddr = devAddr;
    p->opType = opType;
    p->step = 0;
    p->retryCnt = 0;

    VerSrlResendQuery(VSRL_SLOT(83));
    return eICCERRREAS_PENDING;
}

/*=================================================================================================
** @Function  : VerSrl83ReadReply
** @Input     : void
** @Output    : void
** @Function  : 收到83 PACK序列号查询回复后写回数组组帧回复A核
**===============================================================================================*/
static void VerSrl83ReadReply(void)
{
    t_VerSrlPending *p = &sPend[VSRL_SLOT(83)];
    u8 i;

    gIccPackSerial_83[0] = p->devType;
    gIccPackSerial_83[1] = p->devAddr;
    gIccPackSerial_83[2] = ICC_OPTYPE_QUERY_OK;
    for(i = 0; i < BSU_PACKSERIAL_LEN; i++)
    {
        gIccPackSerial_83[3 + i] = gBsuPackSerial[i];
    }

    VerSrlSendReply(p->saddr, 83, gIccPackSerial_83, ICC_MINDEX83_PACKSERIAL_LEN, ICC_MINDEX83_PACKSERIAL_LEN, p->iccNum);
    p->active = 0;
}

/*=================================================================================================
** @Function  : VerSrl83AuthAdvance
** @Input     : void
** @Output    : void
** @Function  : 83写入状态机: 权限回复处理(开权限后发写入 / 关权限后组帧回复)
**===============================================================================================*/
static void VerSrl83AuthAdvance(void)
{
    t_VerSrlPending *p = &sPend[VSRL_SLOT(83)];

    if(0 == p->step)
    {
        /*开权限已回复, 下发写入*/
        p->step = 1;
        p->retryCnt = 0;
        VerSrlResendQuery(VSRL_SLOT(83));
    }
    else if(2 == p->step)
    {
        /*关权限已回复, 组帧回复写入结果*/
        VerSrl83Reply();
        p->active = 0;
    }
}

/*=================================================================================================
** @Function  : VerSrl83WriteAdvance
** @Input     : void
** @Output    : void
** @Function  : 83写入状态机: 写入回复处理(写入后发关权限)
**===============================================================================================*/
static void VerSrl83WriteAdvance(void)
{
    t_VerSrlPending *p = &sPend[VSRL_SLOT(83)];

    if(1 == p->step)
    {
        /*写入已回复, 下发关权限*/
        p->step = 2;
        p->retryCnt = 0;
        VerSrlResendQuery(VSRL_SLOT(83));
    }
}

/*=================================================================================================
** @Function  : VerSrl83Reply
** @Input     : void
** @Output    : void
** @Function  : 83写入完成写回数组组帧回复A核(SET_OK)
**===============================================================================================*/
static void VerSrl83Reply(void)
{
    t_VerSrlPending *p = &sPend[VSRL_SLOT(83)];

    gIccPackSerial_83[0] = p->devType;
    gIccPackSerial_83[1] = p->devAddr;
    gIccPackSerial_83[2] = ICC_OPTYPE_SET_OK;

    VerSrlSendReply(p->saddr, 83, gIccPackSerial_83, ICC_MINDEX83_PACKSERIAL_LEN, ICC_MINDEX83_PACKSERIAL_LEN, p->iccNum);
}

/*=================================================================================================
** @Function  : VerSrl84Deal
** @Input     : saddr:源地址 iccNum:ICC通道
** @Output    : eICCERRREAS_PENDING
** @Function  : 84 电芯序列号查询
**===============================================================================================*/
static u8 VerSrl84Deal(u8 saddr, u8 iccNum)
{
    t_VerSrlPending *p = &sPend[VSRL_SLOT(84)];
    u8 bsuAddr   = gIccCellSerial_84[0];   /*子索引0=目标BSU地址*/
    u8 cellStart = gIccCellSerial_84[1];   /*子索引1=起始电芯号*/
    u8 cellNum   = gIccCellSerial_84[2];   /*子索引2=查询电芯条码个数*/

    if(cellNum > ICC_CELLSERIAL_MAXNUM)
    {
        cellNum = ICC_CELLSERIAL_MAXNUM;
    }

    p->active = 1;
    p->saddr = saddr;
    p->iccNum = iccNum;
    p->devType = ICC_DEVTYPE_BSU;
    p->devAddr = bsuAddr;
    p->opType = gIccCellSerial_84[3];
    p->cellCur = 0;
    p->cellTotal = cellNum;
    p->retryCnt = 0;

    /*初始化回复组帧区头: [0]bsuAddr [1]cellStart [2]cellNum [3]结果码*/
    gIccCellSerial_84[0] = bsuAddr;
    gIccCellSerial_84[1] = cellStart;
    gIccCellSerial_84[2] = cellNum;
    gIccCellSerial_84[3] = ICC_OPTYPE_QUERY_OK;

    VerSrlResendQuery(VSRL_SLOT(84));
    return eICCERRREAS_PENDING;
}

/*=================================================================================================
** @Function  : VerSrl84Advance
** @Input     : void
** @Output    : void
** @Function  : 84电芯序列号: 收到一颗条码后继续查下一颗, 查完组帧回复
**===============================================================================================*/
static void VerSrl84Advance(void)
{
    t_VerSrlPending *p = &sPend[VSRL_SLOT(84)];
    u8 i;

    /*把刚收到的这颗电芯条码拷进数组对应位置*/
    for(i = 0; i < BSU_CELLSERIAL_LEN; i++)
    {
        gIccCellSerial_84[4 + p->cellCur * BSU_CELLSERIAL_LEN + i] = gBsuCellSerial[i];
    }
    p->cellCur++;

    if(p->cellCur >= p->cellTotal)
    {
        /*全部电芯查完, 组帧回复A核*/
        VerSrl84Reply();
        p->active = 0;
    }
    else
    {
        /*继续查下一颗*/
        p->retryCnt = 0;
        VerSrlResendQuery(VSRL_SLOT(84));
    }
}

/*=================================================================================================
** @Function  : VerSrl84Reply
** @Input     : void
** @Output    : void
** @Function  : 84电芯序列号查询完成, 组帧回复A核
**===============================================================================================*/
static void VerSrl84Reply(void)
{
    t_VerSrlPending *p = &sPend[VSRL_SLOT(84)];
    u16 payloadLen = (u16)(4 + p->cellTotal * BSU_CELLSERIAL_LEN);

    VerSrlSendReply(p->saddr, 84, gIccCellSerial_84, payloadLen, 4, p->iccNum);
}

/*=================================================================================================
** @Function  : ICCVerSrlDeal_Hook
** @Input     : saddr:源地址 mIndex:主索引(81-84) sIndex:起始子索引 sIndexNum:子索引数目 iccNum:ICC通道
** @Output    : eICCERRREAS_PENDING / eICCERRREAS_INDEXERR
** @Function  : 81/82/83/84分发处理
**===============================================================================================*/
u8 ICCVerSrlDeal_Hook(u8 saddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum)
{
    switch(mIndex)
    {
        case 81:
            return VerSrl81Deal(saddr, iccNum);
        case 82:
            return VerSrl82Deal(saddr, iccNum);
        case 83:
            return VerSrl83Deal(saddr, iccNum);
        case 84:
            return VerSrl84Deal(saddr, iccNum);
        default:
            return eICCERRREAS_INDEXERR;
    }
}

/*=================================================================================================
** @Function  : ICCVerSrlReplyNotify
** @Input     : devType:下级设备类型(ICC_DEVTYPE_*)  evt:回复事件(VSRL_EVT_*)
** @Output    : void
** @Function  : 下级设备查询回复通知: 匹配待回复上下文后组帧回复A核(事件驱动, 不忙等)
** @The notes : 由CAN接收函数收到具体报文后调用
**===============================================================================================*/
void ICCVerSrlReplyNotify(u8 devType, u8 evt)
{
    u8 slot;

    switch(evt)
    {
        case VSRL_EVT_DEVINFO:
        case VSRL_EVT_SERIAL:
            slot = (VSRL_EVT_DEVINFO == evt) ? VSRL_SLOT(81) : VSRL_SLOT(82);
            break;
        case VSRL_EVT_PACKSERIAL:
        case VSRL_EVT_PACKSERIAL_WR:
        case VSRL_EVT_AUTH:
            slot = VSRL_SLOT(83);
            break;
        case VSRL_EVT_CELLSERIAL:
            slot = VSRL_SLOT(84);
            break;
        default:
            return;
    }

    if((!sPend[slot].active) || (devType != sPend[slot].devType))
    {
        return;
    }

    switch(evt)
    {
        case VSRL_EVT_DEVINFO:
            VerSrl81Reply();
            break;
        case VSRL_EVT_SERIAL:
            VerSrl82Reply();
            break;
        case VSRL_EVT_PACKSERIAL:
            VerSrl83ReadReply();
            break;
        case VSRL_EVT_PACKSERIAL_WR:
            VerSrl83WriteAdvance();
            break;
        case VSRL_EVT_AUTH:
            VerSrl83AuthAdvance();
            break;
        case VSRL_EVT_CELLSERIAL:
            VerSrl84Advance();
            break;
        default:
            break;
    }
}

/*=================================================================================================
** @Function  : ICCVerSrlTimeoutCheck
** @Input     : void
** @Output    : void
** @Function  : 挂起查询超时巡检(50ms周期任务调用): 超时自动重发, 重发次数用尽回查询失败
**===============================================================================================*/
void ICCVerSrlTimeoutCheck(void)
{
    u8 slot;

    for(slot = 0; slot < 4; slot++)
    {
        t_VerSrlPending *p = &sPend[slot];

        if(!p->active)
        {
            continue;
        }
        if(BSPGetOSMillSecTimeAPI() < p->deadline)
        {
            continue;
        }

        if(p->retryCnt < ICC_VSRL_RETRY_MAX)
        {
            p->retryCnt++;
            VerSrlResendQuery(slot);
        }
        else
        {
            VerSrlTimeoutReply(slot);
            p->active = 0;
        }
    }
}
