/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CellFadeCalc.h
** @Founder      : ZS
** @Date         : 2019.10.30
** @Function     : 电池总容量和SOH计算模块
** @Instructions :
**===========================================================================================*/
#ifndef _CELLFADECALC_H_
#define _CELLFADECALC_H_
#include "BSPTypeDef.h"
/*=============================================================================================
** @Header file definition
**===========================================================================================*/

/*=============================================================================================
** @Macro definition
**===========================================================================================*/
//电池衰减结构体
typedef struct
{
    u16 fadeCap;               //衰减总容量(单位:0.1AH)
    u16 ratedCap;              //额定总容量(单位:0.1AH)
    u16 fadeCycle;             //衰减循环次数(单位:1次)
    u16 ratedCycle;            //额定循环次数(单位:1次)
} __attribute__((aligned(4))) t_FADEFORM;

extern size_t ExtxPortGetFreeHeapSize( void );
extern void *ExtMalloc( size_t xWantedSize );
extern void ExtFree( void *pv );
/*=============================================================================================
** @Name      : void CellFadeCalcDataInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 调用此函数时，参数配置还没初始化，先按2簇的内存分配
**===========================================================================================*/
void CellFadeCalcDataInit(void);
/*=============================================================================================
** @Name      : void GroupFadeSohInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池衰减程度SOH计算初始化
** @The notes :
**===========================================================================================*/
void GroupFadeSohInit(void);
/*=============================================================================================
** @Name      : void GroupFadeSohCalcTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池容量衰减程度SOH计算任务
** @The notes : 100ms调用,测算总容量和SOH
**===========================================================================================*/
void GroupFadeSohCalcTask(void);
/*=============================================================================================
** @Name      : void CorrGroupRatedCapAPI(u8 clunum, u16 rateCap)
** @Input     : clunum:簇编号 rateCap:电池组额定容量(单位:0.1AH)
** @Output    : void
** @Function  : 修正电池组额定容量
** @The notes :
**===========================================================================================*/
void CorrGroupRatedCapAPI(u8 clunum, u16 rateCap);
/*=============================================================================================
** @Name      : void CorrGroupRatedCycleAPI(u8 clunum, u16 rateCycle)
** @Input     : clunum:簇编号 rateCycle:电池组额定循环次数
** @Output    : void
** @Function  : 修正电池组额定循环次数
** @The notes :
**===========================================================================================*/
void CorrGroupRatedCycleAPI(u8 clunum, u16 rateCycle);
/*=============================================================================================
** @Name      : u16 GetGFadeTotalCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组衰减后的总容量值(0.1AH)
** @Function  : 获取电池组衰减后的总容量值
** @The notes :
**===========================================================================================*/
u16 GetGFadeTotalCapAPI(u8 clunum);
/*=============================================================================================
** @Name      : u16 GetGroupRatedCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组额定容量(单位:0.1AH)
** @Function  : 获取电池组额定容量
** @The notes :
**===========================================================================================*/
u16 GetGroupRatedCapAPI(u8 clunum);
/*=============================================================================================
** @Name      : u16 GetGroupFadeCycleAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池实际衰减循环次数(1次)
** @Function  : 获取电池组实际衰减循环次数
** @The notes :
**===========================================================================================*/
u16 GetGroupFadeCycleAPI(u8 clunum);
/*=============================================================================================
** @Name      : u16 GetGroupRatedCycleAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池额定循环次数(1次)
** @Function  : 获取电池组额定循环次数
** @The notes :
**===========================================================================================*/
u16 GetGroupRatedCycleAPI(u8 clunum);
/*=============================================================================================
** @Name      : u16 GetGRealSohMilliAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前健康度SOH值(0.1%)
** @Function  : 获取电池组当前健康度SOH值
** @The notes : 分辨率千分之一
**===========================================================================================*/
u16 GetGRealSohMilliAPI(u8 clunum);
/*=============================================================================================
** @Name      : static void CalcGroupCellFadeHandle(void)
** @Input     : void
** @Output    : void
** @Function  : 计算电池组电池衰减状态
** @The notes :
**===========================================================================================*/
static void CalcGroupCellFadeHandle(void);
#endif