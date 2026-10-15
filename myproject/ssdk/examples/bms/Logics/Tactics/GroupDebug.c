/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : GroupDebug.c
** @Founder      : ZQD808
** @Date         : 2026.03.19 
** @Function     : 电池簇调试模式处理模块接口，BMU开关调试实现
** @Instructions : 新增两段式确认分发+7种调试类型(BMU开关已实现,其余桩函数待实现)
**===============================================================================================*/

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "GroupPara.h"
#include "GroupInfo.h"
#include "ParaInterface.h"
#include "AuxDevMsg.h"
#include "TacUser.h"
#include "TacInterface.h"
#include "AuxDevTactic.h"
#include "GroupProtect.h"
#include "GroupTactic.h"
#include "GroupDebug.h"
#include "PublicInfo.h"
#include "BalanTactic.h"
#include "IMEU3Config.h"                             /*调试模式: 需要IMEU3_DEV_ADDR*/
#include "IMEU3Interface.h"                          /*配置: IMEU3_SendSingleFrame*/
#include "PortCheck.h"                               /*急停按钮状态 GetGEmergIoButtonStateAPI*/
#include <stdlib.h>

extern u16 gEU3CfgReply;                             /*配置回复状态*/
extern u8  gEU3CfgReplySub;                          /*配置回复子索引*/
extern u8  gEU3CfgReplyVal;                          /*配置回复回显值*/
extern volatile u8  gEU3CfgReplyFlag;                /*配置回复标志*/
extern u16 gGBmuGenPara_102[];                       /*主控通用参数值(需要BMU地址)*/
extern u16 *gImeu3CfgPara;                           /*配置参数数组, 回读验证*/
extern u16 *gImeu3DevAddr;                           /*设备信息*/
extern u16 *gImeu3SysStatus;                         /*系统状态*/
extern u8  *gImeu3DevSerial;                         /*单板序列号*/
extern u8  *gImeu3BoardInfo1;                        /*EU3板级信息1*/
extern u16 *gImeu3BoardInfo2;                        /*EU3板级信息2*/
extern u16 *gImeu3AlmInfo;                           /*EU3告警信息*/
#include "CLI.h"                                    /*SetGroupVoltPasBalanReqState*/

extern u16 gGBmuGenPara_102[];                          /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];                       /*主控高级参数值*/
extern u8  gGHardPara_104[];                            /*主控硬件参数配置信息*/
extern u16 *gSysDbgCmd_71;                              /*系统调试命令数组 主索引71*/
extern u16 *gSysStaInfo_50;                             /*系统状态信息数组(用于自测设置调试模式)*/

/*===============================================================================================
** @Name      : static u8 TacDebugBsuBalanceDeal(u8 clunum, u8 affirm)
** @Input     : clunum: 协议簇号(0=全部,1=簇1,2=簇2 本函数未使用,由上层校验), affirm: 确认类型
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 从控BSU被动均衡调试, 按BSU绝对编号设置均衡请求状态, SlaveMsg任务自动CAN下发
**   sub2: BSU绝对编号 [0, totalBsu-1], 不支持全选(由A核轮询)
**   sub3: 均衡类型 1=主动(暂不支持) 2=被动
**   sub4: 均衡状态 0=关闭 2=放电均衡 1=充电均衡(被动不支持)
**   sub5-8: 64位电芯bit位 cellState = sub5|(sub6<<16)|(sub7<<32)|(sub8<<48)
**=============================================================================================*/
static u8 TacDebugBsuBalanceDeal(u8 clunum, u8 affirm)
{
    if(1 != affirm)
    {
        return TRUE;
    }

    u8  bsuProto   = (u8)gSysDbgCmd_71[eDbgCmd71_DevNum];       /*sub2: BSU绝对编号 [0, totalBsu-1]*/
    u8  balType    = (u8)gSysDbgCmd_71[eDbgCmd71_FuncP1];       /*sub3: 均衡类型 1=主动 2=被动*/
    u8  balStat    = (u8)gSysDbgCmd_71[eDbgCmd71_FuncP2];       /*sub4: 均衡状态 0=关闭 2=放电*/
    u64 cellState  = 0;
    u8  totalBsu;

    /* 只支持被动均衡 */
    if(2 != balType)
    {
        gGDebugResult[eDbgRes_DbgResult] = eDBGRES_ERRPA;
        return FALSE;
    }

    /* 均衡状态: 0=关闭 2=放电均衡, 被动均衡不支持充电均衡(1) */
    if(0 == balStat)
    {
        cellState = 0ULL;
    }
    else if(2 == balStat)
    {
        cellState  = (u64)gSysDbgCmd_71[eDbgCmd71_CellW0];
        cellState |= (u64)gSysDbgCmd_71[eDbgCmd71_CellW1] << 16;
        cellState |= (u64)gSysDbgCmd_71[eDbgCmd71_CellW2] << 32;
        cellState |= (u64)gSysDbgCmd_71[eDbgCmd71_CellW3] << 48;
    }
    else
    {
        gGDebugResult[eDbgRes_DbgResult] = eDBGRES_ERRPA;
        return FALSE;
    }

    totalBsu = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]
             * (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];

    if(bsuProto >= totalBsu)
    {
        gGDebugResult[eDbgRes_DbgResult] = eDBGRES_ERRPA;
        return FALSE;
    }

    SetGroupVoltPasBalanReqState(bsuProto, cellState);          /*写入gGPasBalanReqState, SlaveMsg任务50ms周期CAN下发*/

    gGDebugResult[eDbgRes_DbgResult] = eDBGRES_SUCCE;
    return TRUE;
}

/*===============================================================================================
** @Name      : static u8 TacDebugBsuEepDeal(u8 clunum, u8 affirm)
** @Input     : clunum: 协议簇号(0=全部,1=簇1,2=簇2), affirm: 确认类型 1=读 2=写
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 从控BSU EEPROM读写调试 - R核通过CAN转发读写命令到指定BSU
** @The notes : 待实现CAN转发逻辑
**   sub2: BSU编号
**   sub3: EEPROM地址
**   sub4: 数据类型(0=8bit 1=16bit 2=32bit)
**   sub5-6: 写数据(H16+L16)
**=============================================================================================*/
static u8 TacDebugBsuEepDeal(u8 clunum, u8 affirm)
{
    if(1 != affirm && 2 != affirm)
    {
        return TRUE;
    }
    return TRUE;
}

/*===============================================================================================
** @Name      : static u8 TacDebugBsuAfeTestDeal(u8 clunum, u8 affirm)
** @Input     : clunum: 协议簇号(0=全部,1=簇1,2=簇2), affirm: 确认类型
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 从控BSU AFE自检调试 - R核通过CAN下发AFE自检命令到指定BSU
** @The notes : 待实现CAN转发逻辑
**   sub2: BSU编号
**   sub3: 自检范围(0=全部 1=电压 2=温度 3=通信)
**=============================================================================================*/
static u8 TacDebugBsuAfeTestDeal(u8 clunum, u8 affirm)
{
    if(1 != affirm)
    {
        return TRUE;
    }
    return TRUE;
}

/*===============================================================================================
** @Name      : static u8 TacDebugBsuResisScanDeal(u8 clunum, u8 affirm)
** @Input     : clunum: 协议簇号(0=全部,1=簇1,2=簇2), affirm: 确认类型
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 从控BSU内阻扫描调试 - R核通过CAN下发内阻检测命令到指定BSU
** @The notes : 待实现CAN转发逻辑
**   sub2: BSU编号
**   sub3: 扫描范围(0=全部电芯 1=单电芯)
**   sub5-6: 单电芯编号(仅sub3=1有效)
**=============================================================================================*/
static u8 TacDebugBsuResisScanDeal(u8 clunum, u8 affirm)
{
    if(1 != affirm)
    {
        return TRUE;
    }
    return TRUE;
}

/*===============================================================================================
** @Name      : static u8 TacDebugBmuSwitchDeal(u8 clunum, u8 affirm)
** @Input     : clunum: 协议簇号(0=全部,1=簇1,2=簇2), affirm: 确认类型
** @Output    : TRUE:成功 FALSE:失败
** @Function  : R核接触器调试(sub0=4)，控制PK1-4(主回路接触器)
** @The notes : 同步BMU05协议, 干接点(NK1-2)独立为sub0=5
**   sub3: 接触器序号 0=全部 1=PK1(簇1主正) 2=PK2(簇1主负) 3=PK3(簇2主正) 4=PK4(簇2主负)
**   sub4: 接触器状态 0=断开 1=闭合
**=============================================================================================*/
static u8 TacDebugBmuSwitchDeal(u8 clunum, u8 affirm)
{   
    if(1 != affirm)
    {
        return TRUE;
    }

    u8 swhNum  = gSysDbgCmd_71[eDbgCmd71_FuncP1];             /* 接触器序号:0=全部 1=PK1 2=PK2 3=PK3 4=PK4 */
    u8 swhStat = gSysDbgCmd_71[eDbgCmd71_FuncP2];             /* 接触器状态:0=断开 1=闭合 */

    if(swhStat > 1)
    {
        gGDebugResult[eDbgRes_DbgResult] = eDBGRES_ERRPA;
        return FALSE;
    }

    if(0 == swhNum)
    {
        if(0 == clunum || 1 == clunum)
        {
            TacCtrlGroupSwhStateAPI(eGroupSwhPK1_1Mai, (e_GPIOState)swhStat);
            TacCtrlGroupSwhStateAPI(eGroupSwhPK2_1Neg, (e_GPIOState)swhStat);
        }
        if(0 == clunum || 2 == clunum)
        {
            TacCtrlGroupSwhStateAPI(eGroupSwhPK3_2Mai, (e_GPIOState)swhStat);
            TacCtrlGroupSwhStateAPI(eGroupSwhPK4_2Neg, (e_GPIOState)swhStat);
        }
    }
    else if(swhNum >= 1 && swhNum <= 4)
    {
        TacCtrlGroupSwhStateAPI((e_GroupSwhSort)(swhNum - 1), (e_GPIOState)swhStat);
    }
    else
    {
        gGDebugResult[eDbgRes_DbgResult] = eDBGRES_ERRPA;
        return FALSE;
    }

    gGDebugResult[eDbgRes_DbgResult] = eDBGRES_SUCCE;
    return TRUE;
}

/*===============================================================================================
** @Name      : static u8 TacDebugBmuDryContactDeal(u8 clunum, u8 affirm)
** @Input     : clunum: 协议簇号(0=全部,1=簇1,2=簇2), affirm: 确认类型
** @Output    : TRUE:成功 FALSE:失败
** @Function  : R核干接点调试(sub0=5)，控制NK1-2(禁充禁放干接点)
** @The notes : 同步BMU05协议新增
**   sub3: 干接点序号 0=全部 1=NK1(簇1禁充禁放) 2=NK2(簇2禁充禁放)
**   sub4: 干接点状态 0=断开 1=闭合
**=============================================================================================*/
static u8 TacDebugBmuDryContactDeal(u8 clunum, u8 affirm)
{   
    if(1 != affirm)
    {
        return TRUE;
    }

    u8 dryNum  = gSysDbgCmd_71[eDbgCmd71_FuncP1];             /* 干接点序号:0=全部 1=NK1 2=NK2 */
    u8 dryStat = gSysDbgCmd_71[eDbgCmd71_FuncP2];             /* 干接点状态:0=断开 1=闭合 */

    if(dryStat > 1)
    {
        gGDebugResult[eDbgRes_DbgResult] = eDBGRES_ERRPA;
        return FALSE;
    }

    if(0 == dryNum)
    {
        if(0 == clunum || 1 == clunum)
        {
            TacCtrlGroupSwhStateAPI(eGroupSwhNK1_1Pcs, (e_GPIOState)dryStat);
        }
        if(0 == clunum || 2 == clunum)
        {
            TacCtrlGroupSwhStateAPI(eGroupSwhNK2_2Pcs, (e_GPIOState)dryStat);
        }
    }
    else if(1 == dryNum)
    {
        TacCtrlGroupSwhStateAPI(eGroupSwhNK1_1Pcs, (e_GPIOState)dryStat);
    }
    else if(2 == dryNum)
    {
        TacCtrlGroupSwhStateAPI(eGroupSwhNK2_2Pcs, (e_GPIOState)dryStat);
    }
    else
    {
        gGDebugResult[eDbgRes_DbgResult] = eDBGRES_ERRPA;
        return FALSE;
    }

    gGDebugResult[eDbgRes_DbgResult] = eDBGRES_SUCCE;
    return TRUE;
}

/*===============================================================================================
** @Name      : static u8 TacDebugBmuEepDeal(u8 affirm)
** @Input     : affirm: 确认类型 1=读EEPROM 2=写EEPROM
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 主控BMU EEPROM读写调试 - 直接操作板上M2514芯片
** @The notes : 待实现 M2514地址空间[0,65535]
**   sub3: EEPROM地址
**   sub4: 数据类型(0=8bit 1=16bit 2=32bit)
**   sub5: 写数据高16位(32bit数据时有效)
**   sub6: 写数据低16位(16bit=全部数据,32bit=低16位)
**   读结果存 gGDebugResult[eDbgRes_RDataLW/eDbgRes_RDataHW]
**=============================================================================================*/
static u8 TacDebugBmuEepDeal(u8 affirm)
{
    if(1 != affirm && 2 != affirm)
    {
        return TRUE;
    }
    return TRUE;
}

/*===============================================================================================
** @Name      : static u8 TacDebugBmuAuxDevDeal(u8 affirm)
** @Input     : affirm: 确认类型
** @Output    : TRUE:成功 FALSE:失败
** @Function  : EU3外设调试(sub0=7) - R核通过CAN/485/GPIO控制外围设备(IMEU3)
** @The notes : 同步BMU05协议，外设清单完全更换
**   sub3: 外设编号(1-10)   sub4: 控制(0=关 1=开)
**   sub5-6: 按位风扇控制(H16/L16)
**   1泄爆阀排气扇1#  2泄爆阀排气扇2#  3泄爆阀电源1#  4泄爆阀电源2#
**   5除湿机控制  6消防触发  7声光告警  8烟感电源控制  9QS1隔离开关  10远程急停
**=============================================================================================*/
static u8 TacDebugBmuAuxDevDeal(u8 affirm)
{
    if(1 != affirm)
    {
        return TRUE;
    }

    u8 devNum = (u8)gSysDbgCmd_71[eDbgCmd71_FuncP1];     /*sub3: 外设编号[1,10]*/
    u8 state  = (u8)gSysDbgCmd_71[eDbgCmd71_FuncP2];     /*sub4: 控制状态 0:关/停止 1:开/启动*/

    /*外设编号有效性检查*/
    if(devNum < 1 || devNum > 11)
    {
        gGDebugResult[eDbgRes_DbgResult] = eDBGRES_ERRPA;
        return FALSE;
    }

    /*控制状态有效性检查(除湿机/PDU风扇不检查state范围)*/
    if(state > 1 && devNum != 5 && devNum != 11)
    {
        gGDebugResult[eDbgRes_DbgResult] = eDBGRES_ERRPA;
        return FALSE;
    }

    switch(devNum)
    {
        /*===========================================================
        ** sub3=1 泄爆阀排气扇1#: PWM1子索引0, 0=停 1=满转100%
        **===========================================================*/
        case 1:
            SendCanImeu3PwmCtrlMsg(IMEU3_DEV_ADDR, 0, state ? 100 : 0);
            break;

        /*===========================================================
        ** sub3=2 泄爆阀排气扇2#: PWM4子索引3, 0=停 1=满转100%
        **===========================================================*/
        case 2:
            SendCanImeu3PwmCtrlMsg(IMEU3_DEV_ADDR, 3, state ? 100 : 0);
            break;

        /*===========================================================
        ** sub3=3 泄爆阀电源1#: Motor子索引7, 0=断开 1=闭合
        **===========================================================*/
        case 3:
            SendCanImeu3MotorCtrlMsg(IMEU3_DEV_ADDR, 7, state);
            break;

        /*===========================================================
        ** sub3=4 泄爆阀电源2#: Motor子索引8, 0=断开 1=闭合
        **===========================================================*/
        case 4:
            SendCanImeu3MotorCtrlMsg(IMEU3_DEV_ADDR, 8, state);
            break;

        /*===========================================================
        ** sub3=5 除湿机: 0=切回自动模式(关闭), 1=手动模式强制开启
        ** 开: DeCtrl(mode=0手动, onHum=1%强制启动, offHum=0)
        ** 关: DeCtrl(mode=0xFF自动, onHum/offHum=EEPROM配置值)
        **===========================================================*/
        case 5:
            if(state == 1)
            {
                SendCanImeu3DeCtrlMsg(IMEU3_DEV_ADDR, 1, 0, 0);
            }
            else
            {
                u8 onHum  = (u8)gGHardPara_104[eGHardPara104_AirDeHumOn_H];
                u8 offHum = (u8)gGHardPara_104[eGHardPara104_AirDeHumOff_H];
                SendCanImeu3DeCtrlMsg(IMEU3_DEV_ADDR, onHum, offHum, 0xFF);
            }
            break;

        /*===========================================================
        ** sub3=6 消防触发 DO1: 预留, 暂不实现
        **===========================================================*/
        case 6:
            break;

        /*===========================================================
        ** sub3=7 声光告警 DO2: startIdx=2, 0=关 1=开
        **===========================================================*/
        case 7:
            SendCanImeu3DoCtrlMsg(IMEU3_DEV_ADDR, 1, 2, state);
            break;

        /*===========================================================
        ** sub3=8 烟感电源 DO3: startIdx=3, 0=关 1=开
        **===========================================================*/
        case 8:
            SendCanImeu3DoCtrlMsg(IMEU3_DEV_ADDR, 1, 3, state);
            break;

        /*===========================================================
        ** sub3=9 QS1隔离开关: 暂不实现
        **===========================================================*/
        case 9:
            break;

        /*===========================================================
        ** sub3=10 远程急停 DO5: startIdx=5, 0=关 1=开  这个DO5低电平会导致下高压 
        **===========================================================*/
        case 10:
            SendCanImeu3DoCtrlMsg(IMEU3_DEV_ADDR, 1, 5, state);
            break;

        /*PDU风扇 PWM2: 0=停 1=满转*/
        case 11:
            SendCanImeu3PwmCtrlMsg(IMEU3_DEV_ADDR, 1, state ? 100 : 0);
            break;

        default:
            gGDebugResult[eDbgRes_DbgResult] = eDBGRES_ERRPA;
            return FALSE;
    }

    gGDebugResult[eDbgRes_DbgResult] = eDBGRES_SUCCE;
    return TRUE;
}



/*===============================================================================================
** @Name      : static u8 DebugModeCheck(u8 dbgType, u8 protoClu)
** @Input     : dbgType: 调试类型 protoClu: 协议簇号
** @Output    : TRUE:模式OK FALSE:模式非法
** @Function  : 校验当前是否处于调试模式
** @The notes : 接触器/干接点(type4/5)按簇独立校验, 其他类型任意簇即可
**=============================================================================================*/
static u8 DebugModeCheck(u8 dbgType, u8 protoClu)
{
    if(4 == dbgType || 5 == dbgType)                                 /*接触器/干接点:按簇独立*/
    {
        if(1 == protoClu)
            return (eMODE_DEBUG == GetGWorkModeAPI(0));
        else if(2 == protoClu)
            return (eMODE_DEBUG == GetGWorkModeAPI(1));
        else
            return (eMODE_DEBUG == GetGWorkModeAPI(0)
                 && eMODE_DEBUG == GetGWorkModeAPI(1));
    }
    return (eMODE_DEBUG == GetGWorkModeAPI(0)                        /*其他:任意簇*/
         || eMODE_DEBUG == GetGWorkModeAPI(1));
}

/*===============================================================================================
** @Name      : void TacSetGroupDebugAffirmCmdAPI(u8 affirm)
** @Input     : affirm: 确认类型 1=确认功能调试 2=确认写EEPROM
** @Output    : void
** @Function  : 调试确认命令分发入口，按设备地址(sub2)路由到对应设备的调试处理函数
** @The notes : 同步BMU05协议, devAddr→设备, dbgType→操作, 一一对应
**=============================================================================================*/
void TacSetGroupDebugAffirmCmdAPI(u8 affirm)
{
    u8 dbgType  = gSysDbgCmd_71[eDbgCmd71_DbgType];                /* 调试类型(sub0):0=BSU均衡 1=BSU E2P 2=BSU AFE 3=BSU内阻 4=R核接触器 5=R核干接点 6=R核E2P 7=EU3外设 */
    u8 protoClu = gSysDbgCmd_71[eDbgCmd71_CluNum];                 /* 协议簇号(sub1):0=所有簇 1=第一簇 2=第二簇 */

    gGDebugResult[eDbgRes_DbgResult] = eDBGRES_UNEXE;

    if(1 != affirm && 2 != affirm)
    {
        return;
    }
    if(protoClu > 2)
    {
        gGDebugResult[eDbgRes_DbgResult] = eDBGRES_ERRPA;
        return;
    }
    /*校验调试模式: PK/NK按簇独立, EU3等任意簇即可*/
    if(0 == DebugModeCheck(dbgType, protoClu))
    {
        gGDebugResult[eDbgRes_DbgResult] = eDBGRES_NOMOD;     /*非调试模式, 拒绝执行*/
        return;
    }

    switch(dbgType)
    {
        case 0:
            (void)TacDebugBsuBalanceDeal(protoClu, affirm);
            break;                                                   /*BSU均衡调试*/
        case 1:
            (void)TacDebugBsuEepDeal(protoClu, affirm);
            break;                                                   /*BSU E2P读写调试*/
        case 2:
            (void)TacDebugBsuAfeTestDeal(protoClu, affirm);
            break;                                                   /*BSU AFE自检*/
        case 3:
            (void)TacDebugBsuResisScanDeal(protoClu, affirm);
            break;                                                   /*BSU内阻扫描*/
        case 4:
            (void)TacDebugBmuSwitchDeal(protoClu, affirm);
            break;                                                   /*R核接触器调试:PK1-4*/
        case 5:
            (void)TacDebugBmuDryContactDeal(protoClu, affirm);
            break;                                                   /*R核干接点调试:NK1-2*/
        case 6:
            (void)TacDebugBmuEepDeal(affirm);
            break;                                                   /*R核E2P读写调试*/
        case 7:
            (void)TacDebugBmuAuxDevDeal(affirm);
            break;                                                   /*EU3外设调试*/
        default:
            gGDebugResult[eDbgRes_DbgResult] = eDBGRES_ERRPA;
            break;
    }
}

#if(0)  /*自测CLI命令*/

/*===============================================================================================
** @Name      : DebugTestSetMode
** @Input     : clunum: 簇号(1/2)  mode: 0=自动 5=调试
** @Output    : void
** @Function  : 自测用: 设置工作模式
**=============================================================================================*/
static void DebugTestSetMode(u8 clunum, u16 mode)
{
    if(clunum == 1)
    {
        gSysStaInfo_50[eSysStateInfo50_1SysMod] = mode;
        printf("[DBG] 簇1 -> %s\r\n", (mode == eMODE_DEBUG) ? "调试模式" : "自动模式");
    }
    else if(clunum == 2)
    {
        gSysStaInfo_50[eSysStateInfo50_2SysMod] = mode;
        printf("[DBG] 簇2 -> %s\r\n", (mode == eMODE_DEBUG) ? "调试模式" : "自动模式");
    }
    else
    {
        gSysStaInfo_50[eSysStateInfo50_1SysMod] = mode;
        gSysStaInfo_50[eSysStateInfo50_2SysMod] = mode;
        printf("[DBG] 全部簇 -> %s\r\n", (mode == eMODE_DEBUG) ? "调试模式" : "自动模式");
    }
}

/*===============================================================================================
** @Name      : DebugTestExecSw
** @Input     : dbgType: 4=接触器 5=干接点  clunum: 簇号  swhNum: 开关号  swhStat: 0分1合
** @Output    : void
** @Function  : 自测用: 执行单次开关/干接点调试
**=============================================================================================*/
static void DebugTestExecSw(u8 dbgType, u8 clunum, u8 swhNum, u8 swhStat)
{
    const char *typeName = (4 == dbgType) ? "接触器" : "干接点";
    const char *stateName = swhStat ? "合" : "分";
    const char *swhName = "全部";

    if(4 == dbgType)
    {
        static const char *pkNames[] = {"全部", "PK1", "PK2", "PK3", "PK4"};
        if(swhNum <= 4) swhName = pkNames[swhNum];
    }
    else
    {
        static const char *nkNames[] = {"全部", "NK1", "NK2"};
        if(swhNum <= 2) swhName = nkNames[swhNum];
    }

    gSysDbgCmd_71[eDbgCmd71_DbgType] = dbgType;
    gSysDbgCmd_71[eDbgCmd71_CluNum]  = clunum;
    gSysDbgCmd_71[eDbgCmd71_FuncP1]  = swhNum;
    gSysDbgCmd_71[eDbgCmd71_FuncP2]  = swhStat;

    TacSetGroupDebugAffirmCmdAPI(1);

    u16 result = gGDebugResult[eDbgRes_DbgResult];
    const char *resStr = "未知";
    switch(result)
    {
        case eDBGRES_SUCCE: resStr = "成功"; break;
        case eDBGRES_NOMOD: resStr = "非调试模式"; break;
        case eDBGRES_ERRPA: resStr = "参数错误"; break;
        default:            resStr = "未执行"; break;
    }

    printf("[DBG] 簇%d %s %s %s -> %s\r\n", clunum, typeName, swhName, stateName, resStr);
}

/*===============================================================================================
** @Name      : DebugTestExecBalan
** @Input     : bsu: BSU绝对编号  balStat: 0关2放  cellW0-3: 4×u16电芯位(可写hex)
** @Output    : void
** @Function  : 自测用: 执行单次BSU被动均衡调试
**=============================================================================================*/
static void DebugTestExecBalan(u8 bsu, u8 balStat, u16 cellW0, u16 cellW1, u16 cellW2, u16 cellW3)
{
    u8 totalBsu = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]
                * (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];

    if(bsu >= totalBsu)
    {
        printf("[DBG] BSU编号超限 (0-%d)\r\n", totalBsu - 1);
        return;
    }

    gSysDbgCmd_71[eDbgCmd71_DbgType] = 0;                     /*type0: BSU均衡*/
    gSysDbgCmd_71[eDbgCmd71_CluNum]  = 0;                     /*簇号 均衡由上层校验*/
    gSysDbgCmd_71[eDbgCmd71_DevNum]  = bsu;                   /*BSU绝对编号*/
    gSysDbgCmd_71[eDbgCmd71_FuncP1]  = 2;                     /*被动均衡*/
    gSysDbgCmd_71[eDbgCmd71_FuncP2]  = balStat;               /*0=关 2=放电*/
    gSysDbgCmd_71[eDbgCmd71_CellW0]  = cellW0;                /*电芯bit[15:0]*/
    gSysDbgCmd_71[eDbgCmd71_CellW1]  = cellW1;                /*电芯bit[31:16]*/
    gSysDbgCmd_71[eDbgCmd71_CellW2]  = cellW2;                /*电芯bit[47:32]*/
    gSysDbgCmd_71[eDbgCmd71_CellW3]  = cellW3;                /*电芯bit[63:48]*/

    TacSetGroupDebugAffirmCmdAPI(1);

    u16 result = gGDebugResult[eDbgRes_DbgResult];
    const char *resStr = "未知";
    switch(result)
    {
        case eDBGRES_SUCCE: resStr = "成功";     break;
        case eDBGRES_NOMOD: resStr = "非调试模式"; break;
        case eDBGRES_ERRPA: resStr = "参数错误";  break;
        default:            resStr = "未执行";     break;
    }

    u64 cell = (u64)cellW0 | ((u64)cellW1 << 16) | ((u64)cellW2 << 32) | ((u64)cellW3 << 48);
    printf("[DBG] BSU%d 被动均衡 %s 电芯=0x%016llX -> %s\r\n",
           bsu, (balStat == 0) ? "关闭" : "放电", cell, resStr);
}

/*===============================================================================================
** @Name      : DebugTestExecEU3
** @Input     : devNum: 外设编号(1-11)  state: 0关/1开
** @Output    : void
** @Function  : 自测用: 执行单次EU3外设调试, 通过ICC通道模拟A核命令
**=============================================================================================*/
static void DebugTestExecEU3(u8 devNum, u8 state)
{
    static const char *devNames[] = {
        "", "排气扇1#", "排气扇2#", "泄爆阀电源1#", "泄爆阀电源2#",
        "除湿机", "消防(预留)", "声光告警", "烟感电源", "QS1(预留)", "远程急停", "PDU风扇"
    };
    const char *devName = (devNum <= 11) ? devNames[devNum] : "未知";
    const char *stateName = (devNum == 1 || devNum == 2) ? (state ? "满转" : "停")
                          : (devNum == 5) ? (state ? "手动开" : "自动")
                          : (state ? "开" : "关");

    gSysDbgCmd_71[eDbgCmd71_DbgType] = 7;          /*sub0=7: EU3外设调试*/
    gSysDbgCmd_71[eDbgCmd71_CluNum]  = 0;          /*EU3不分簇*/
    gSysDbgCmd_71[eDbgCmd71_FuncP1]  = devNum;     /*sub3: 外设编号*/
    gSysDbgCmd_71[eDbgCmd71_FuncP2]  = state;      /*sub4: 控制状态*/

    TacSetGroupDebugAffirmCmdAPI(1);

    u16 result = gGDebugResult[eDbgRes_DbgResult];
    const char *resStr = "未知";
    switch(result)
    {
        case eDBGRES_SUCCE: resStr = "成功";     break;
        case eDBGRES_NOMOD: resStr = "非调试模式"; break;
        case eDBGRES_ERRPA: resStr = "参数错误";  break;
        default:            resStr = "未执行";     break;
    }

    printf("[DBG] EU3 %s %s -> %s\r\n", devName, stateName, resStr);
}

/*===============================================================================================
** @Name      : DebugTestExecEU3Cfg
** @Input     : subIdx: 子索引  value: 配置值
** @Output    : void
** @Function  : 自测用: 向IMEU3下发参数配置并打印回复结果
**=============================================================================================*/
static void DebugTestExecEU3Cfg(u8 subIdx, u8 value)
{
    static const char *cfgNames[] = {
        "", "CAN波特率1", "485波特率", "CAN版本",
        "温度通道", "DI通道", "PWM参数", "压力传感器",
        "可燃气体", "485个数", "除湿机", "灯带", "泄爆阀"
    };
    const char *cfgName = (subIdx <= 12) ? cfgNames[subIdx] : "未知";

    u8 bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    u8 data[]  = {0x0C, 0x01, subIdx, 0x00, value};

    gEU3CfgReplyFlag = 0;
    IMEU3_SendSingleFrame(eIMEU3Func_RequestModify, 1, eIMEU3Idx_ModifyPara,
                          IMEU3_DEV_ADDR, bmuAddr, data, 5);

    /*等待IMEU3回复(最多300ms)*/
    u32 start = BSPGetOSMillSecTimeAPI();
    while(BSPGetOSMillSecTimeAPI() - start < 300)
    {
        if(gEU3CfgReplyFlag) break;
    }

    if(gEU3CfgReplyFlag == 0)
    {
        printf("[EU3CFG] %s(subIdx=%d)=%d -> 超时无回复\r\n", cfgName, subIdx, value);
        return;
    }

    const char *resStr = "未知";
    if(gEU3CfgReply == 1 && gEU3CfgReplyVal != value)
    {
        resStr = "值不一致";
    }
    else switch(gEU3CfgReply)
    {
        case 1:  resStr = "成功"; break;
        case 2:  resStr = "索引非法"; break;
        case 3:  resStr = "CRC非法"; break;
        case 4:  resStr = "权限非法"; break;
        case 5:  resStr = "状态非法"; break;
        case 6:  resStr = "值非法";   break;
        default: resStr = "未知错误"; break;
    }
    printf("[EU3CFG] %s(subIdx=%d)=%d -> %s\r\n", cfgName, subIdx, value, resStr);

    /*修改成功后, 回读验证值是否一致*/
    if(gEU3CfgReply == 1)
    {
        gEU3CfgReplyFlag = 0;
        IMEU3_SendCfgParaQuery(IMEU3_DEV_ADDR);

        u32 start = BSPGetOSMillSecTimeAPI();
        while(BSPGetOSMillSecTimeAPI() - start < 300)
        {
            if(gEU3CfgReplyFlag) break;
        }

        if(gEU3CfgReplyFlag && subIdx < eIMEU3Cfg_Len)
        {
            u16 readback = gImeu3CfgPara[subIdx];
            printf("       回读=%d %s\r\n", readback,
                   (readback == value) ? "✓" : "✗ 不一致!");
        }
        else
        {
            printf("       回读=超时无回复\r\n");
        }
    }
}

/*===============================================================================================
** @Name      : static int DebugTestMain
** @Input     : argc: 参数个数  argv: 参数数组
** @Output    : 0:成功  CLI_FALSE:参数错误
** @Function  : 调试命令自测CLI入口
** @The notes : 用法:
**   dbg_test mode  <0-2> <auto|debug>    进入/退出调试模式
**   dbg_test pk    <0-2> <0-4>  <0/1>    接触器调试(簇 开关 分/合)
**   dbg_test nk    <0-2> <0-2>  <0/1>    干接点调试(簇 开关 分/合)
**   dbg_test stat                         查看当前状态
**=============================================================================================*/
static int DebugTestMain(int argc, char *argv[])
{
    if(argc < 1)
    {
        printf("Usage:\r\n");
        printf("  dbg_test mode    <0-2> <auto|debug>    进入/退出调试模式\r\n");
        printf("  dbg_test pk      <0-2> <0-4>  <0/1>    接触器调试(簇 开关 分/合)\r\n");
        printf("  dbg_test nk      <0-2> <0-2>  <0/1>    干接点调试(簇 开关 分/合)\r\n");
        printf("  dbg_test eu3     <name> <0/1>            EU3外设调试\r\n");
        printf("    name: fan1/fan2/pdufan/switch1/switch2/dehum/alarm/smoke/estop\r\n");
        printf("  dbg_test eu3cfg  <all|subIdx> <value>     EU3参数配置(回复)\r\n");
        printf("    eu3cfg all: 温度NTC+PWM12V+可燃4+除湿2\r\n");
        printf("  dbg_test eu3info <type>                    EU3数据打印(内存)\r\n");
        printf("  dbg_test eu3query <type>                   EU3主动查询+打印\r\n");
        printf("    type: devaddr/serial/status/board1/board2/alarm/cfg/all\r\n");
        printf("    type: devaddr/serial/status/board1/board2/alarm/cfg/all\r\n");
        printf("  dbg_test balance <bsu> <0/2> <w0> <w1> <w2> <w3>\r\n");
        printf("                  BSU被动均衡(bsu=0-15, 0=关2=放, w0-3=电芯bit位hex)\r\n");
        printf("  dbg_test stat                            查看当前状态\r\n");
        return CLI_FALSE;
    }

    /* stat: 查看当前状态 */
    if(!strcmp(argv[0], "stat"))
    {
        printf("[DBG] 簇1模式=%d  簇2模式=%d  上次结果=%d\r\n",
               gSysStaInfo_50[eSysStateInfo50_1SysMod],
               gSysStaInfo_50[eSysStateInfo50_2SysMod],
               gGDebugResult[eDbgRes_DbgResult]);

        u8 di = DEVGPIOGetSPI6DiState3();
        printf("[反馈] DI1(主正1)=%d(%s) DI2(主负1)=%d(%s) DI3(主正2)=%d(%s) DI4(主负2)=%d(%s)\r\n",
               (di>>0)&1, ((di>>0)&1)?"断开":"闭合",
               (di>>1)&1, ((di>>1)&1)?"断开":"闭合",
               (di>>2)&1, ((di>>2)&1)?"断开":"闭合",
               (di>>3)&1, ((di>>3)&1)?"断开":"闭合");

        /*DI9急停按钮: 原始bit(0=低电平/1=高电平) + 代码最终判定*/
        {
            u8 emergBtn = GetGEmergIoButtonStateAPI();
            printf("[急停] DI9原始bit=%d 按钮判定=%d(%s)\r\n",
                   BitGet(GetDevSampInfoAPI(eDevSamp_MuxGpioIn1), 9),
                   emergBtn, emergBtn ? "按下" : "未按下");
        }

        /*EU3 DI状态: DI17-DI21, 常闭(1=正常/闭合, 0=异常/断开)*/
        u16 eu3di = GetImeu3BoardInfo2API(eIMEUBoardInfo2_DI17_22);
        printf("[EU3 DI] DI17(泄爆阀1)=%d(%s) DI18(泄爆阀2)=%d(%s) DI19(排气扇1)=%d(%s) DI20(排气扇2)=%d(%s) DI21(PDU风扇)=%d(%s)\r\n",
               BitGet(eu3di,0), BitGet(eu3di,0)?"闭合":"断开",
               BitGet(eu3di,1), BitGet(eu3di,1)?"闭合":"断开",
               BitGet(eu3di,2), BitGet(eu3di,2)?"闭合":"断开",
               BitGet(eu3di,3), BitGet(eu3di,3)?"闭合":"断开",
               BitGet(eu3di,4), BitGet(eu3di,4)?"闭合":"断开");
        return 0;
    }

    /* mode: 切换调试模式 */
    if(!strcmp(argv[0], "mode"))
    {
        if(argc < 3) { printf("dbg_test mode <0-2> <auto|debug>\r\n"); return CLI_FALSE; }
        u8 clunum = (u8)atoi(argv[1]);
        u16 mode = strcmp(argv[2], "debug") ? eMODE_AUTO : eMODE_DEBUG;
        DebugTestSetMode(clunum, mode);
        return 0;
    }

    /* balance: BSU被动均衡调试 */
    if(!strcmp(argv[0], "balance"))
    {
        if(argc < 7) { printf("dbg_test balance <bsu> <0/2> <w0> <w1> <w2> <w3>\r\n"); return CLI_FALSE; }
        u8  bsu     = (u8)strtoul(argv[1], NULL, 0);
        u8  balStat = (u8)strtoul(argv[2], NULL, 0);
        u16 w0      = (u16)strtoul(argv[3], NULL, 0);
        u16 w1      = (u16)strtoul(argv[4], NULL, 0);
        u16 w2      = (u16)strtoul(argv[5], NULL, 0);
        u16 w3      = (u16)strtoul(argv[6], NULL, 0);
        DebugTestExecBalan(bsu, balStat, w0, w1, w2, w3);
        return 0;
    }

    /* eu3query: 主动下发查询EU3数据并打印回复 */
    if(!strcmp(argv[0], "eu3query"))
    {
        if(argc < 2) { printf("dbg_test eu3query <devaddr|serial|cfg|all>\r\n"); return CLI_FALSE; }

        if(!strcmp(argv[1], "devaddr") || !strcmp(argv[1], "all"))
            IMEU3_SendDeviceInfoQuery(IMEU3_DEV_ADDR);
        if(!strcmp(argv[1], "serial") || !strcmp(argv[1], "all"))
            IMEU3_SendDevSerialQuery(IMEU3_DEV_ADDR);
        if(!strcmp(argv[1], "cfg") || !strcmp(argv[1], "all"))
            IMEU3_SendCfgParaQuery(IMEU3_DEV_ADDR);

        /*等待IMEU3回复(最多500ms)*/
        u32 start = BSPGetOSMillSecTimeAPI();
        while(BSPGetOSMillSecTimeAPI() - start < 500) {}

        /*复用eu3info的打印逻辑: 将argv[0]改为eu3info, argv[1]不变*/
        argv[0] = "eu3info";
        /*继续往下走到eu3info分支*/
    }

    /* eu3info: 打印EU3上报/查询回复的已存储数据 */
    if(!strcmp(argv[0], "eu3info"))
    {
        if(argc < 2) { printf("dbg_test eu3info <devaddr|serial|status|board1|board2|alarm|cfg|all>\r\n"); return CLI_FALSE; }

        if(!strcmp(argv[1], "devaddr") || !strcmp(argv[1], "all"))
        {
            printf("\r\n=== EU3 设备信息 (mIndex=1) ===\r\n");
            if(gImeu3DevAddr == NULL) { printf("  未初始化\r\n"); }
            else
            {
                static const char *names[] = {"设备标识","设备地址","硬件版本","软件版本L","软件版本H",
                    "BSP版本","CAN通讯包版本","UART通讯包版本","运行时间L","运行时间H"};
                for(u8 i = 0; i < eIMEUDevAddr_Len; i++)
                    printf("  [%d] %-14s = %d\r\n", i, names[i], gImeu3DevAddr[i]);
            }
        }

        if(!strcmp(argv[1], "serial") || !strcmp(argv[1], "all"))
        {
            printf("\r\n=== EU3 单板序列号 (mIndex=2) ===\r\n");
            if(gImeu3DevSerial == NULL) { printf("  未初始化\r\n"); }
            else { printf("  "); for(u8 i = 0; i < eIMEUDevSerial_Len; i++) printf("%c", gImeu3DevSerial[i]); printf("\r\n"); }
        }

        if(!strcmp(argv[1], "status") || !strcmp(argv[1], "all"))
        {
            printf("\r\n=== EU3 系统状态 (mIndex=50) ===\r\n");
            if(gImeu3SysStatus == NULL) { printf("  未初始化\r\n"); }
            else printf("  系统工作模式 = %d\r\n", gImeu3SysStatus[0]);
        }

        if(!strcmp(argv[1], "board1") || !strcmp(argv[1], "all"))
        {
            printf("\r\n=== EU3 板级信息1 (mIndex=51, NTC温度) ===\r\n");
            if(gImeu3BoardInfo1 == NULL) { printf("  未初始化\r\n"); }
            else
            {
                for(u8 i = 0; i < eIMEUBoardInfo1_Len; i++)
                {
                    s8 ctx = (s8)gImeu3BoardInfo1[i];
                    printf("  [%2d] = %d (raw) = %d℃\r\n", i, gImeu3BoardInfo1[i], (int)ctx - 50);
                }
            }
        }

        if(!strcmp(argv[1], "board2") || !strcmp(argv[1], "all"))
        {
            printf("\r\n=== EU3 板级信息2 (mIndex=52) ===\r\n");
            if(gImeu3BoardInfo2 == NULL) { printf("  未初始化\r\n"); }
            else
            {
                printf("  DI1-16:  0x%04X\r\n", gImeu3BoardInfo2[eIMEUBoardInfo2_DI1_16]);
                printf("  DI17-22: 0x%02X\r\n", gImeu3BoardInfo2[eIMEUBoardInfo2_DI17_22]);
                printf("  DO1-16:  0x%04X\r\n", gImeu3BoardInfo2[eIMEUBoardInfo2_DO1_16]);
                printf("  DO17-24: 0x%02X\r\n", gImeu3BoardInfo2[eIMEUBoardInfo2_DO17_24]);
            }
        }

        if(!strcmp(argv[1], "alarm") || !strcmp(argv[1], "all"))
        {
            printf("\r\n=== EU3 告警信息 (mIndex=53) ===\r\n");
            if(gImeu3AlmInfo == NULL) { printf("  未初始化\r\n"); }
            else printf("  Alm1=0x%04X  Alm2=0x%04X  Alm3=0x%04X\r\n",
                          gImeu3AlmInfo[eIMEUAlmInfo_Alm1], gImeu3AlmInfo[eIMEUAlmInfo_Alm2], gImeu3AlmInfo[eIMEUAlmInfo_Alm3]);
        }

        if(!strcmp(argv[1], "cfg") || !strcmp(argv[1], "all"))
        {
            printf("\r\n=== EU3 常规参数 (mIndex=102) ===\r\n");
            if(gImeu3CfgPara == NULL) { printf("  未初始化\r\n"); }
            else
            {
                static const char *cfgNames[] = {"配置标志","CAN波特率","485波特率","CAN版本",
                    "温度通道","DI通道","PWM参数","压力传感器","可燃气体","485个数",
                    "除湿机","灯带","泄爆阀","预留","预留","预留","预留","预留","预留","预留"};
                for(u8 i = 0; i < eIMEU3Cfg_Len; i++)
                    printf("  [%2d] %-8s = %d\r\n", i, cfgNames[i], gImeu3CfgPara[i]);
            }
        }
        return 0;
    }

    /* eu3cfg: EU3参数配置 */
    if(!strcmp(argv[0], "eu3cfg"))
    {
        if(argc < 2) { printf("dbg_test eu3cfg <all|subIdx> <value>\r\n"); return CLI_FALSE; }

        if(!strcmp(argv[1], "all"))
        {
            DebugTestExecEU3Cfg(4, 0x1F);   /*温度通道: NTC*/
            DebugTestExecEU3Cfg(6, 2);      /*PWM参数: 12V*/
            DebugTestExecEU3Cfg(8, 4);      /*可燃气体: ATRS-1010*/
            DebugTestExecEU3Cfg(10, 2);     /*除湿机: 航同*/
        }
        else
        {
            u8 subIdx = (u8)atoi(argv[1]);
            u8 value  = (u8)atoi(argv[2]);
            DebugTestExecEU3Cfg(subIdx, value);
        }
        return 0;
    }

    /* eu3: EU3外设调试 */
    if(!strcmp(argv[0], "eu3"))
    {
        if(argc < 3) { printf("dbg_test eu3 <name> <0/1>\r\n"); return CLI_FALSE; }

        u8 devNum = 0;

        if(!strcmp(argv[1], "fan1"))         devNum = 1;
        else if(!strcmp(argv[1], "fan2"))    devNum = 2;
        else if(!strcmp(argv[1], "pdufan"))  devNum = 11;
        else if(!strcmp(argv[1], "switch1")) devNum = 3;
        else if(!strcmp(argv[1], "switch2")) devNum = 4;
        else if(!strcmp(argv[1], "dehum"))   devNum = 5;
        else if(!strcmp(argv[1], "alarm"))   devNum = 7;
        else if(!strcmp(argv[1], "smoke"))   devNum = 8;
        else if(!strcmp(argv[1], "estop"))   devNum = 10;
        else { printf("外设名: fan1/fan2/pdufan/switch1/switch2/dehum/alarm/smoke/estop\r\n"); return CLI_FALSE; }

        u8 state = (u8)atoi(argv[2]);
        if(state > 1) { printf("状态 0=停/关 1=满转/开\r\n"); return CLI_FALSE; }

        DebugTestExecEU3(devNum, state);
        return 0;
    }

    /* pk / nk: 开关调试 */
    if(argc < 4) { printf("dbg_test %s <0-2> <开关号> <0/1>\r\n", argv[0]); return CLI_FALSE; }

    u8 dbgType = (u8)(!strcmp(argv[0], "pk") ? 4 : 5);
    u8 clunum  = (u8)atoi(argv[1]);
    u8 swhNum  = (u8)atoi(argv[2]);
    u8 swhStat = (u8)atoi(argv[3]);

    if(clunum > 2) { printf("簇号 0=全部 1=簇1 2=簇2\r\n"); return CLI_FALSE; }
    if(swhStat > 1) { printf("状态 0=分 1=合\r\n"); return CLI_FALSE; }

    DebugTestExecSw(dbgType, clunum, swhNum, swhStat);
    return 0;
}

CLI_CMD("dbg_test", "Debug command self-test", DebugTestMain);

#endif
