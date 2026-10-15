/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : TacInterface.h
** @Founder      : SBK326
** @Date         : 2019.11.1
** @Function     : 电池组策略模块用户接口
** @Instructions : 
**===============================================================================================*/
#ifndef _TACINTERFACE_H
#define _TACINTERFACE_H

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "PublicInfo.h"

#define TAC_HS_ON_CURR                  1                   /*高边开关闭合驱动电流 1mA*/
#define TAC_SMG_ONCON_T                 30                  /*烟感电源复位后等待信号确认时间(默认10s) 1s*/


/*外围接触器类型定义*/
typedef enum
{
    eGroupSwhPK1_1Mai = 0,                  /*簇1主正接触器*/
    eGroupSwhPK2_1Neg,                      /*簇1负极接触器*/
    eGroupSwhPK3_2Mai,                      /*簇2主正接触器*/
    eGroupSwhPK4_2Neg,                      /*簇2负极接触器*/

    eGroupSwhNK1_1Pcs,                      /*簇1PCS禁充禁放*/
    eGroupSwhNK2_2Pcs,                      /*簇2PCS禁充禁放*/
    eGroupSwhNK3_Emer,                      /*急停*/
    eGroupSwhNK4_2Air,                      /*空调通讯控制*/

	eGroupSwh_Num                           /*接触器个数*/
}e_GroupSwhSort;

/*电池组启动失败原因*/
typedef enum
{
    eStartFailReason_SysFault = 0,      /*系统故障*/
    eStartFailReason_StartLim = 1,      /*启动受限*/
    eStartFailReason_PeripExp = 2,      /*外设异常*/
    eStartFailReason_SwtchExp = 3,      /*接触器异常*/
    eStartFailReason_PChgTOvr = 4,      /*预充过温*/
    eStartFailReason_PChgFail = 5,      /*预充未满*/
    eStartFailReason_PChgCErr = 6,      /*预充电流异常*/

    eStartFailReason_Fin
}e_StartFailReason;

/*电池组停机失败原因*/
typedef enum
{
    eStopFailReason_SysFault = 0,       /*系统故障*/
    eStopFailReason_CurrFail = 1,       /*等待安全电流失败*/
    eStopFailReason_SwtchExp = 2,       /*接触器异常*/
    eStopFailReason_OffFail  = 3,       /*接触器断开失败*/

    eStopFailReason_Fin
}e_StopFailReason;

/*电池组紧急停机原因*/
typedef enum
{
    eEmergStopReason_SysInt = 0,        /*系统内部急停*/
    eEmergStopReason_ExtCmd = 1,        /*外部命令急停*/
    eEmergStopReason_Button = 2,        /*按钮命令急停*/

    eEmergStopReason_Fin
}e_EmergStopReason;
/*=================================================================================================
** @Name      : e_ErrSort TacGetGNowErrLevelHook(u8 clunum, u8 type)
** @Input     : clunum:簇编号 type:故障类别 0:全部故障 1:充电类故障 2:放电类故障
** @Output    : 电池组当前故障级别:0无故障 1仅提示 2充电限制 3放电限制 4充放电限制 5充电保护 6放电保护 7充放电保护 8系统故障保护
** @Function  : 获取电池组当前故障级别
** @The notes :
**===============================================================================================*/
e_ErrSort TacGetGNowErrLevelHook(u8 clunum, u8 type);
/*=================================================================================================
** @Name      : u8 TacGetGAllSlaveReadyHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组所有从机就绪状态 TRUE:就绪 FALSE:未就绪
** @Function  : 获取电池组所有从机就绪状态
** @The notes : 包括从控通信状态和采样状态
**===============================================================================================*/
u8 TacGetGAllSlaveReadyHook(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGAllSubdevReadyHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组所有子系统设备就绪状态 TRUE:就绪 FALSE:未就绪
** @Function  : 获取电池组所有子系统设备就绪状态
** @The notes : 包括IMLU1和IMEU2
**===============================================================================================*/
u8 TacGetGAllSubdevReadyHook(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGAllPeriphReadyHook(u8 clunum)
** @Input     : void
** @Output    : 电池组所有外设就绪状态 TRUE:就绪 FALSE:未就绪
** @Function  : 获取电池组所有外围设备就绪状态
** @The notes : 包括电流传感器和接触器、绝缘等
**===============================================================================================*/
u8 TacGetGAllPeriphReadyHook(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGNegCirReadyHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组所有从机就绪状态 TRUE:就绪 FALSE:未就绪
** @Function  : 获取电池组负极电路就绪状态
** @The notes : GND接外侧采预充电压正常
**===============================================================================================*/
u8 TacGetGNegCirReadyHook(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGPreChgFinStateHook(void)
** @Input     : void
** @Output    : 电池组当前直流侧预充状态 0:未预充 1:预充成功 2:预充过温 3:预充电流异常
** @Function  : 获取电池组直流侧预充状态
** @The notes :
**===============================================================================================*/
u8 TacGetGPreChgFinStateHook(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacCheckCurrSafeStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前电流安全状态 0:不安全 1:安全
** @Function  : 检测电池组当前电流安全状态
** @The notes :
**===============================================================================================*/
u8 TacCheckCurrSafeStateHook(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacCheckCurrCutStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前电流切断状态 0:未切断 1:切断
** @Function  : 检测电池组当前电流切断状态
** @The notes :
**===============================================================================================*/
u8 TacCheckCurrCutStateHook(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetUserReqStopStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 请求电池组停机状态 0:未请求 1:请求
** @Function  : 获取用户请求电池组停机状态
** @The notes : 运行状态需要停机
**===============================================================================================*/
u8 TacGetUserReqStopStateHook(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGroupFitStartStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组适合启动状态 0:不适合 1:适合
** @Function  : 获取电池组适合启动状态
** @The notes : 空闲状态允许启动
**===============================================================================================*/
u8 TacGetGroupFitStartStateHook(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGroupFitStartStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组适合启动状态 0:不适合 1:适合
** @Function  : 获取电池组适合启动状态
** @The notes : 空闲状态允许启动
**===============================================================================================*/
u8 TacGetGroupFitStartStateHook(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetUserSmogSignStateHook(void)
** @Input     : void
** @Output    : 电池组烟感报警状态 0:未报警 1:报警
** @Function  : 获取用户请求电池组烟感报警状态
** @The notes :
**===============================================================================================*/
u8 TacGetUserSmogSignStateHook(void);
/*=================================================================================================
** @Name      : u8 TacGetUserButtonEmergStateHook(void)
** @Input     : void
** @Output    : 电池组按钮急停状态 0:未请求 1:请求
** @Function  : 获取用户请求电池组按钮急停状态
** @The notes :
**===============================================================================================*/
u8 TacGetUserButtonEmergStateHook(void);
/*=================================================================================================
** @Name      : u8 TacGetGroupExtCommuErrStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前外部通信异常状态 0:正常 1:异常
** @Function  : 获取电池组当前外部通信异常状态
** @The notes :
**===============================================================================================*/
u8 TacGetGroupExtCommuErrStateHook(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGroupExtCommuErrStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前外部通信异常状态 0:正常 1:异常
** @Function  : 获取电池组当前外部通信异常状态
** @The notes :
**===============================================================================================*/
u8 TacGetGroupExtCommuErrStateHook(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGroupBattVHStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前高压状态 0:正常 1:高压
** @Function  : 获取电池组高压状态
** @The notes :
**===============================================================================================*/
u8 TacGetGroupBattVHStateHook(u8 clunum);
/*=================================================================================================
** @Name      : u8 TacGetGroupBattVLStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前低压状态 0:正常 1:低压
** @Function  : 获取电池组低压状态
** @The notes :
**===============================================================================================*/
u8 TacGetGroupBattVLStateHook(u8 clunum);
/*=================================================================================================
** @Name      : u16 TacGetGroupChgCapHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组充电电量 1AH
** @Function  : 获取电池组充电电量
** @The notes :
**===============================================================================================*/
u16 TacGetGroupChgCapHook(u8 clunum);
/*=================================================================================================
** @Name      : u16 TacGetGroupDhgCapHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组放电电量 1AH
** @Function  : 获取电池组放电电量
** @The notes :
**===============================================================================================*/
u16 TacGetGroupDhgCapHook(u8 clunum);
/*=================================================================================================
** @Name      : e_SysStateResult TacGroupInitStateResultHook(u8 clunum)
** @Input     : clunum:簇编号 
** @Output    : 检测结果:0:等待 1:完成 2:异常
** @Function  : 电池组初始化状态检测结果
** @The notes :
**===============================================================================================*/
e_SysStateResult TacGroupInitStateResultHook(u8 clunum);
/*=================================================================================================
** @Name      : void TacGroupProStateChangDealHook(u8 clunum, u8 befState, u16 aftState)
** @Input     : clunum:簇编号 befState:前置状态 aftState:后置状态
** @Output    : void
** @Function  : 电池组保护状态变换处理函数
** @The notes :
**===============================================================================================*/
void TacGroupProStateChangDealHook(u8 clunum, u8 befState, u16 aftState);
/*=================================================================================================
** @Name      : void TacGroupWorkStateChangDealHook(u8 clunum, u8 befState, u16 aftState)
** @Input     : clunum:簇编号 befState:前置状态 aftState:后置状态
** @Output    : void
** @Function  : 电池组工作状态变换处理函数
** @The notes :
**===============================================================================================*/
void TacGroupWorkStateChangDealHook(u8 clunum, u8 befState, u16 aftState);
/*=================================================================================================
** @Name      : void TacGroupChgStateChangDealHook(u8 clunum, u8 befState, u16 aftState)
** @Input     : clunum:簇编号 befState:前置状态 aftState:后置状态
** @Output    : void
** @Function  : 电池组充放电状态变换处理函数
** @The notes :
**===============================================================================================*/
void TacGroupChgStateChangDealHook(u8 clunum, u8 befState, u16 aftState);
/*=================================================================================================
** @Name      : void TacGroupWorkModeChangDealHook(u8 clunum, u8 befMode, u8 aftMode)
** @Input     : clunum:簇编号 befState:前置状态 aftState:后置状态
** @Output    : void
** @Function  : 电池组工作模式变换处理函数
** @The notes :
**===============================================================================================*/
void TacGroupWorkModeChangDealHook(u8 clunum, u8 befMode, u8 aftMode);
/*=================================================================================================
** @Name      : void TacGroupPreChgFinDealHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 电池组预充完成处理函数
** @The notes :
**===============================================================================================*/
void TacGroupPreChgFinDealHook(u8 clunum);
#endif