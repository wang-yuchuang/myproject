/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CurrIntegral.h
** @Founder      : ZQD808
** @Date         : 2026.03.17
** @Function     : 电流积分模块
** @Instructions :
**===========================================================================================*/
#ifndef _CURRINTEGRAL_H
#define _CURRINTEGRAL_H
#include "BSPTypeDef.h"
#include "stddef.h"
#include <stdio.h>
#include <debug.h>
#include <cmsis_os2.h>
/*=============================================================================================
** @Header file definition
**===========================================================================================*/

/*=============================================================================================
** @Macro definition
**===========================================================================================*/
#define MULT_MAH_TO_10MAMS      360000      //1mAH转化为10ma1ms的倍率
#define MULT_AH_TO_MAH          1000        //1AH转化为1mah的倍率

#define MULT_WMS_TO_MWMS        1000        //1WMS转化为1mw1ms(1ma1V1ms)的倍率
#define MULT_WH_TO_WMS          3600000     //1WH转化为1W1ms(1AV1ms)的倍率
#define MULT_100WH_TO_WH        100         //100WH转化为1WH(1AVH)的倍率

#define SMALL_ESS_EN            (0)         //小储能系统使用mAH和WH作单位使能(大储能用AH和0.1kWH)

/*=============================================================================================
** @Data type definition
**===========================================================================================*/
typedef struct                  //电量积分结构体
{
	u32 chgCap10ma1ms;          //充电电量积分(单位:10mA1ms)
	u32 dhgCap10ma1ms;          //放电电量积分(单位:10mA1ms)
	u32 chgCap1ma1h;            //充电电量积分(单位:1mAH)
	u32 dhgCap1ma1h;            //放电电量积分(单位:1mAH)
	u32 chgCap1a1h;             //充电电量计算(单位:1AH)
	u32 dhgCap1a1h;             //放电电量计算(单位:1AH)
	s32 changCap1ma1h;          //充放电电量变化量(单位:1mAH,正数表放电,负数表充电)
} __attribute__((aligned(4))) t_CAPINT;

typedef struct                  //电流积分结构体
{
	u32 chgEner1w1ms;           //充电电能积分(单位:1w1ms,即1AV1ms)
	u32 dhgEner1w1ms;           //放电电能积分(单位:1w1ms,即1AV1ms)
	u32 chgEner1w1h;            //充电电能积分(单位:1WH)
	u32 dhgEner1w1h;            //放电电能积分(单位:1WH)
	u32 chgEner100w1h;          //充电电能计算(单位:100WH,即0.1KWH)
	u32 dhgEner100w1h;          //放电电能计算(单位:100WH,即0.1KWH)
	s32 changEner1w1h;          //充放电电能变化量(单位:1WH,正数表放电,负数表充电)
} __attribute__((aligned(4))) t_ENERINT;
extern size_t ExtxPortGetFreeHeapSize( void );
extern void *ExtMalloc( size_t xWantedSize );
extern void ExtFree( void *pv );
/*=============================================================================================
** @Name      : void CurrIntegralDataInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 调用此函数时，参数配置还没初始化，先按2簇的内存分配
**===========================================================================================*/
void CurrIntegralDataInit(void);
/*=============================================================================================
** @Name      : void ChgCapIntTask(u8 clunum, u32 curr, u8 cycle)
** @Input     : clunum:簇编号 curr:采样电流值(单位10mA) cycle:调用(积分)周期(1ms[1,10])
** @Output    : void
** @Function  : 充电电量积分任务
** @The notes : 最少10ms周期调用
**===========================================================================================*/
void ChgCapIntTask(u8 clunum, u32 curr, u8 cycle);
/*=============================================================================================
** @Name      : void DhgCapIntTask(u8 clunum, u32 curr, u8 cycle)
** @Input     : clunum:簇编号 curr:采样电流值(单位10mA) cycle:调用(积分)周期(1ms[1,10])
** @Output    : void
** @Function  : 放电电量积分任务
** @The notes : 最少10ms周期调用
**===========================================================================================*/
void DhgCapIntTask(u8 clunum, u32 curr, u8 cycle);
/*=============================================================================================
** @Name      : void ChgEnerIntTask(u8 clunum, u32 curr, u16 volt, u8 cycle)
** @Input     : clunum:簇编号 curr:采样电流值(单位10mA), volt采样电压值(单位:100mV) cycle:调用(积分)周期(1ms[1,10])
** @Output    : void
** @Function  : 充电电能积分任务
** @The notes : 最少10ms周期调用
**===========================================================================================*/
void ChgEnerIntTask(u8 clunum, u32 curr, u16 volt, u8 cycle);
/*=============================================================================================
** @Name      : void DhgEnerIntTask(u8 clunum, u32 curr, u16 volt, u8 cycle)
** @Input     : clunum:簇编号 curr:采样电流值(单位10mA), volt采样电压值(单位:100mV) cycle:调用(积分)周期(1ms[1,10])
** @Output    : void
** @Function  : 放电电能积分任务
** @The notes : 最少10ms周期调用
**===========================================================================================*/
void DhgEnerIntTask(u8 clunum, u32 curr, u16 volt, u8 cycle);
/*=============================================================================================
** @Name      : u32 GetChgIntCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 充电积分电量值(单位:1AH)
** @Function  : 获取充电积分电量值
** @The notes :
**===========================================================================================*/
u32 GetChgIntCapAPI(u8 clunum);
/*=============================================================================================
** @Name      : u32 GetDhgIntCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 放电积分电量值(单位:1AH)
** @Function  : 获取放电积分电量值
** @The notes :
**===========================================================================================*/
u32 GetDhgIntCapAPI(u8 clunum);
/*=============================================================================================
** @Name      : s32 GetChgDhgChangCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 充放电电量变化值(单位:1mAH,正数为放电量,负数为充电量)
** @Function  : 获取充放电电量变化值
** @The notes :
**===========================================================================================*/
s32 GetChgDhgChangCapAPI(u8 clunum);
/*=============================================================================================
** @Name      : u32 GetChgIntEnerAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 充电积分电能值(单位:0.1KWH)
** @Function  : 获取充电积分电能值
** @The notes :
**===========================================================================================*/
u32 GetChgIntEnerAPI(u8 clunum);
/*=============================================================================================
** @Name      : u32 GetDhgIntEnerAPI(u8 clunum)
** @Input     : clunum
** @Output    : 放电积分电能值(单位:0.1KWH)
** @Function  : 获取放电积分电能值
** @The notes :
**===========================================================================================*/
u32 GetDhgIntEnerAPI(u8 clunum);
/*=============================================================================================
** @Name      : s32 GetChgDhgChangEnerAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 充放电电能变化值(单位:1WH,正数为放电能,负数为充电能)
** @Function  : 获取充放电电能变化值
** @The notes :
**===========================================================================================*/
s32 GetChgDhgChangEnerAPI(u8 clunum);
#endif