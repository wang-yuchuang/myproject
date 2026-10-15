/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : EnerInterface.h
** @Founder      : ZQD808
** @Date         : 2026.03.17
** @Function     : 电能量计算模块接口函数
** @Instructions :
**===========================================================================================*/
#ifndef _ENERINTERFACE_H
#define _ENERINTERFACE_H
#include "BSPTypeDef.h"
/*=============================================================================================
** @Name      : u8 EnerGetGroupBattReadyHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 0未就绪 1就绪
** @Function  : 获取电池信息就绪状态
** @The notes :
**===========================================================================================*/
u8 EnerGetGroupBattReadyHook(u8 clunum);
/*=============================================================================================
** @Name      : u8 EnerGetGroupVHProStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组充电高压保护状态:0正常 1充电高压保护
** @Function  : 获取电池组充电高压保护状态
** @The notes :
**===========================================================================================*/
u8 EnerGetGroupVHProStateHook(u8 clunum);
/*=============================================================================================
** @Name      : u8 EnerGetGroupVLProStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组放电低压保护状态:0正常 1充电高压保护
** @Function  : 获取电池组放电低压保护状态
** @The notes :
**===========================================================================================*/
u8 EnerGetGroupVLProStateHook(u8 clunum);
/*=============================================================================================
** @Name      : u8 EnerGetGroupVHLimStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组充电高压限制状态:0正常 1充电高压限制
** @Function  : 获取电池组充电高压限制状态
** @The notes :
**===========================================================================================*/
u8 EnerGetGroupVHLimStateHook(u8 clunum);
/*=============================================================================================
** @Name      : u8 EnerGetGroupVLLimStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组放电欠压限制状态:0正常 1放电欠压限制
** @Function  : 获取电池组放电欠压限制状态
** @The notes :
**===========================================================================================*/
u8 EnerGetGroupVLLimStateHook(u8 clunum);
/*=============================================================================================
** @Name      : u8 EnerGetUserSocStopReqHook(void)
** @Input     : void
** @Output    : TRUE:请求停止SOC FALSE:无请求
** @Function  : 获取用户SOC停止显示请求
** @The notes : 容量自动标定时停止SOC显示,停止在50%
**===========================================================================================*/
u8 EnerGetUserSocStopReqHook(void);
/*=============================================================================================
** @Name      : u8 EnerGetUserSoeStopReqHook(void)
** @Input     : void
** @Output    : TRUE:请求停止SOE FALSE:无请求
** @Function  : 获取用户SOE停止显示请求
** @The notes : 容量自动标定时停止SOE显示,停止在50%
**===========================================================================================*/
u8 EnerGetUserSoeStopReqHook(void);
/*=============================================================================================
** @Name      : void EnerChangEepGNowCapHook(u8 clunum, u32 cap)
** @Input     : clunum:簇编号 cap:当前容量 1mAH
** @Output    : void
** @Function  : 修改EEP当前容量值
** @The notes : 可用容量
**===========================================================================================*/
void EnerChangEepGNowCapHook(u8 clunum, u32 cap);
/*=============================================================================================
** @Name      : void EnerChangEepGRemainCapHook(u8 clunum, u32 cap)
** @Input     : clunum:簇编号 cap:目标剩余容量 1mAH
** @Output    : void
** @Function  : 修改EEP电池组剩余容量值
** @The notes : 包括不能放出来的容量
**===========================================================================================*/
void EnerChangEepGRemainCapHook(u8 clunum, u32 cap);
/*=============================================================================================
** @Name      : void EnerChangEepGTotalCapHook(u8 clunum, u32 cap)
** @Input     : clunum:簇编号 cap:目标总容量 1mAH
** @Output    : void
** @Function  : 修改EEP电池组总容量值
** @The notes : 实测容量
**===========================================================================================*/
void EnerChangEepGTotalCapHook(u8 clunum, u32 cap);
/*=============================================================================================
** @Name      : void EnerChangEepGStandCapHook(u8 clunum, u32 cap)
** @Input     : clunum:簇编号 cap:目标标称容量 1mAH
** @Output    : void
** @Function  : 修改EEP电池组标称容量值
** @The notes : 实测容量
**===========================================================================================*/
void EnerChangEepGStandCapHook(u8 clunum, u32 cap);
/*=============================================================================================
** @Name      : void EnerChangEepGNowSocHook(u8 clunum, u16 soc)
** @Input     : clunum:簇编号 soc:当前显示SOC 0.1%
** @Output    : void
** @Function  : 修改EEP当前显示SOC值
** @The notes : 显示SOC
**===========================================================================================*/
void EnerChangEepGNowSocHook(u8 clunum, u16 soc);
/*=============================================================================================
** @Name      : void EnerChangEepGNowEner(u8 clunum, u32 ener)
** @Input     : clunum:簇编号 ener:当前能量 1Wh
** @Output    : void
** @Function  : 修改EEP电池组当前能量值
** @The notes : 可用电能
**===========================================================================================*/
void EnerChangEepGNowEner(u8 clunum, u32 ener);
/*=============================================================================================
** @Name      : void EnerChangEepGRemainEner(u8 clunum, u32 ener)
** @Input     : clunum:簇编号 ener:剩余能量 1Wh
** @Output    : void
** @Function  : 修改EEP电池组当前剩余能量值
** @The notes : 包括不能放出来的电能
**===========================================================================================*/
void EnerChangEepGRemainEner(u8 clunum, u32 ener);
/*=============================================================================================
** @Name      : void EnerChangEepTotalEner(u8 clunum, u32 ener)
** @Input     : clunum:簇编号 ener:总能量 1Wh
** @Output    : void
** @Function  : 修改EEP总能量值
** @The notes : 实测总电能
**===========================================================================================*/
void EnerChangEepTotalEner(u8 clunum, u32 ener);
/*=============================================================================================
** @Name      : void EnerChangEepStandEner(u8 clunum, u32 ener)
** @Input     : clunum:簇编号 ener:标称能量 1Wh
** @Output    : void
** @Function  : 修改EEP标称能量值
** @The notes : 标准电能值
**===========================================================================================*/
void EnerChangEepStandEner(u8 clunum, u32 ener);
/*=============================================================================================
** @Name      : void EnerChangEepGNowSoeHook(u8 clunum, u16 soe)
** @Input     : clunum:簇编号 soe:当前显示SOE 0.1%
** @Output    : void
** @Function  : 修改EEP当前显示SOE值
** @The notes : 显示SOE
**===========================================================================================*/
void EnerChangEepGNowSoeHook(u8 clunum, u16 soe);
/*=============================================================================================
** @Name      : void EnerChangEepGRatedCapHook(u16 rateCap)
** @Input     : rateCap:电池组额定容量(0.1AH)
** @Output    : void
** @Function  : 修改电池组额定容量
** @The notes : 出厂有效容量
**===========================================================================================*/
void EnerChangEepGRatedCapHook(u16 rateCap);
/*=============================================================================================
** @Name      : void EnerChangEepGDampNumHook(u16 times)
** @Input     : times:电池额定循环次数
** @Output    : void
** @Function  : 修改电池额定循环次数
** @The notes : 有效衰减次数
**===========================================================================================*/
void EnerChangEepGDampNumHook(u16 times);
/*=============================================================================================
** @Name      : void EnerChangEepGNowCapSohHook(u8 clunum, u16 soh)
** @Input     : clunum:簇编号 soh:电池SOH(0.1%)
** @Output    : void
** @Function  : 修改电池当前总容量SOH值
** @The notes : 有效衰减次数
**===========================================================================================*/
void EnerChangEepGNowCapSohHook(u8 clunum, u16 soh);
/*=============================================================================================
** @Name      : void EnerAftTotalCapCorrDealHook(u8 clunum, u16 befcap, u16 aftcap)
** @Input     : clunum:簇编号 befcap:前置总容量 aftcap:修正后总容量
** @Output    : void
** @Function  : 总容量修正后处理
** @The notes :
**===========================================================================================*/
void EnerAftTotalCapCorrDealHook(u8 clunum, u16 befcap, u16 aftcap);
/*=============================================================================================
** @Name      : void EnerAftTotalEnerCorrDealHook(u8 clunum, u16 befener, u16 aftener)
** @Input     : clunum：簇编号 befener:前置总能量 aftener:修正后总能量
** @Output    : void
** @Function  : 总能量修正后处理
** @The notes :
**===========================================================================================*/
void EnerAftTotalEnerCorrDealHook(u8 clunum, u16 befener, u16 aftener);
#endif