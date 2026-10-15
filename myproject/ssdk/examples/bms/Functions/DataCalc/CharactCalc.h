/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CharactCalc.h
** @Founder      : ZSY342
** @Date         : 2019.10.30
** @Function     : 电池特征信息计算模块
** @Instructions :
**===========================================================================================*/
#ifndef _CHARACTCALC_H
#define _CHARACTCALC_H
#include "BSPTypeDef.h"
/*=============================================================================================
** @Header file definition
**===========================================================================================*/

/*=============================================================================================
** @Macro definition
**===========================================================================================*/
#define AVG_CURR_TIME      (5)      	/*计算平均电流周期(s)*/
#define CUR_FIT_LEN        (200)    	/*电流滤波长度*/


extern size_t ExtxPortGetFreeHeapSize( void );
extern void *ExtMalloc( size_t xWantedSize );
extern void ExtFree( void *pv );
/*=============================================================================================
** @Name      :void CharactCalcDataInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 由于片内内存限制，使用外部EMMC空间，全部使用ExtMalloc申请内存
**===========================================================================================*/
void CharactCalcDataInit(void);
/*=============================================================================================
** @Name      : void GroupDataInfoInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池数据信息初始化
** @The notes :
**===========================================================================================*/
void GroupDataInfoInit(void);
/*=============================================================================================
** @Name      : void CheckVoltCharaTask(void)
** @Input     : void
** @Output    : void
** @Function  : 检测电压特征信息任务
** @The notes : 100ms
**===========================================================================================*/
void CheckVoltCharaTask(void);
/*=============================================================================================
** @Name      : void CheckTempCharaTask(void)
** @Input     : void
** @Output    : void
** @Function  : 检测温度特征信息任务
** @The notes : 100ms
**===========================================================================================*/
void CheckTempCharaTask(void);
/*=============================================================================================
** @Name      : void CheckAvgCurrTask(void)
** @Input     : void
** @Output    : void
** @Function  : 检测电池组平均电流任务
** @The notes : 100ms
**===========================================================================================*/
void CheckAvgCurrTask(void);
/*=============================================================================================
** @Name      : void CurrFilterDeal(u8 clunum, s32 curr, u8 flag)
** @Input     : clunum:簇编号 cur:采样电流值(0.1A) flag:立即显示标志(1:立即显示)
** @Output    : void
** @Function  : 对电流进行滤波
** @The notes : 电流采样调用
**===========================================================================================*/
void CurrFilterDeal(u8 clunum, s32 curr, u8 flag);
/*=============================================================================================
** @Name      : void SampSumVoltFilterDeal(u8 clunum, u16 volt)
** @Input     : clunum:簇编号 volt:采样电压值(0.1V)
** @Output    : void
** @Function  : 对电池端总电压采样值进行滤波
** @The notes : 电池输出端总电压采样值处理调用
**===========================================================================================*/
void SampSumVoltFilterDeal(u8 clunum, u16 volt);
/*=============================================================================================
** @Name      : void OutPreVoltFilterDeal(u8 clunum, u16 volt)
** @Input     : clunum:簇编号 volt:采样电压值(0.1V)
** @Output    : void
** @Function  : 对输出端预充电压采样值进行滤波
** @The notes : 电池输出端预充电压采样值处理调用
**===========================================================================================*/
void OutPreVoltFilterDeal(u8 clunum, u16 volt);
/*=============================================================================================
** @Name      : s16 GetGroupAvgCurrAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组平均电流值(单位:0.1A)
** @Function  : 获取电池组平均电流值
** @The notes :
**===========================================================================================*/
s16 GetGroupAvgCurrAPI(u8 clunum);
#endif