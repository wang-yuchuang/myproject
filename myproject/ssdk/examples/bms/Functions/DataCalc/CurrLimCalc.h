/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CurrLimCalc.h
** @Founder      : ZSY342
** @Date         : 2019.10.30
** @Function     : 充放电电流限值计算模块
** @Instructions :
**===========================================================================================*/
#ifndef _CURRLIMCALC_H
#define _CURRLIMCALC_H
#include "BSPTypeDef.h"
/*=============================================================================================
** @Name      : void CurrLimCalcTask(void)
** @Input     : void
** @Output    : void
** @Function  : 充放电电流限值计算任务
** @The notes : 100ms
**===========================================================================================*/
void CurrLimCalcTask(void);
/*=============================================================================================
** @Name      : u16 GetChgHReqMaxCurr(void)
** @Input     : void
** @Output    : 充电过流值下允许的最大电流限值 0.1A
** @Function  : 返回充电过流值下允许的最大电流限值
** @The notes : 配置值
**===========================================================================================*/
u16 GetChgHReqMaxCurr(void);
/*=============================================================================================
** @Name      : u16 GetDhgHReqMaxCurr(void)
** @Input     : void
** @Output    : 放电过流值下允许的最大电流限值 0.1A
** @Function  : 返回放电过流值下允许的最大电流限值
** @The notes : 配置值
**===========================================================================================*/
u16 GetDhgHReqMaxCurr(void);
/*=============================================================================================
** @Name      : static u16 CalcTempChgMulCurr(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 当前温度充电倍率下的电流限值 0.1A
** @Function  : 计算当前温度充电倍率下的电流限值
** @The notes : 查表
**===========================================================================================*/
static u16 CalcTempChgMulCurr(u8 clunum);
/*=============================================================================================
** @Name      : static u16 ChgLimDownByTempH(u8 clunum, s16 startTemp, s16 endTemp, s16 nowTemp, u16 currLim)
** @Input     : clunum:簇编号 startTemp:开始降限值起始温度 endTemp:降限值为0的温度 nowTemp:当前温度  currLim:当前限值
** @Output    : 经高温降后的电流限值 0.1A
** @Function  : 根据高温降充电电流限值
** @The notes : 查表
**===========================================================================================*/
static u16 ChgLimDownByTempH(u8 clunum, s16 startTemp, s16 endTemp, s16 nowTemp, u16 currLim);
/*=============================================================================================
** @Name      : static u16 DhgLimDownByTempH(u8 clunum, s16 startTemp, s16 endTemp, s16 nowTemp, u16 currLim)
** @Input     : clunum:簇编号 startTemp:开始降限值起始温度 endTemp:降限值为0的温度 nowTemp:当前温度  currLim:当前限值
** @Output    : 经高温降后的电流限值 0.1A
** @Function  : 根据高温降放电电流限值
** @The notes : 查表
**===========================================================================================*/
static u16 DhgLimDownByTempH(u8 clunum, s16 startTemp, s16 endTemp, s16 nowTemp, u16 currLim);
/*=============================================================================================
** @Name      : static u16 ChgLimDownBySubTemp(u8 clunum, u16 currLim)
** @Input     : clunum:簇编号 currLim:当前限值 0.1A
** @Output    : 经温差降后的电流限值 0.1A
** @Function  : 根据温差降充电电流限值
** @The notes : 高温预警前只降1梯度,预警后降2梯度
**===========================================================================================*/
static u16 ChgLimDownBySubTemp(u8 clunum, u16 currLim);
/*=============================================================================================
** @Name      : static u16 DhgLimDownBySubTemp(u8 clunum, u16 currLim)
** @Input     : clunum:簇编号 currLim:当前限值 0.1A
** @Output    : 经温差降后的电流限值 0.1A
** @Function  : 根据温差降放电电流限值
** @The notes : 高温预警前只降1梯度,预警后降2梯度
**===========================================================================================*/
static u16 DhgLimDownBySubTemp(u8 clunum, u16 currLim);
/*=============================================================================================
** @Name      : static u16 ChgLimCurrByPowerLim(u8 clunum, u16 chgUpCur)
** @Input     : clunum:簇编号 chgUpCur 当前充电电流限值上限:0.1A
** @Output    : 经过充电功率限制后的充电电流限值:0.1A
** @Function  : 计算通过充电功率限制后的充电电流限值
** @The notes :
**===========================================================================================*/
static u16 ChgLimCurrByPowerLim(u8 clunum, u16 chgUpCur);
/*=============================================================================================
** @Name      : static u16 DhgLimCurrByPowerLim(u8 clunum, u16 dhgUpCur)
** @Input     : clunum:簇编号 dhgUpCur 当前放电电流限值上限0.1A
** @Output    : 经过放电功率限制后的放电电流限值0.1A
** @Function  : 计算通过放电功率限制后的放电电流限值
** @The notes :
**===========================================================================================*/
static u16 DhgLimCurrByPowerLim(u8 clunum, u16 dhgUpCur);
/*=============================================================================================
** @Name      : static u16 CalcTempDhgMulCurr(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 当前温度放电倍率下的电流限值 0.1A
** @Function  : 计算当前温度放电倍率下的电流限值
** @The notes : 查表
**===========================================================================================*/
static u16 CalcTempDhgMulCurr(u8 clunum);
#endif