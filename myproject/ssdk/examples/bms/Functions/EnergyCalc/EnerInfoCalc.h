/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CurrIntegral.h
** @Founder      : ZSY342
** @Date         : 2019.10.30
** @Function     : 电能量SOE计算模块
** @Instructions :
**===========================================================================================*/
#ifndef _ENERINFOCALC_H
#define _ENERINFOCALC_H
#include "BSPTypeDef.h"
/*=============================================================================================
** @Header file definition
**===========================================================================================*/

/*=============================================================================================
** @Macro definition
**===========================================================================================*/
#define ENER_ZERO_POINT             1000000     //电能计算的零点值(底端容量)
#define ENER_CHANG_TO_WRITE_EEP     500         //电能进行写EEP变化值(单位:1WH)
 
/*=============================================================================================
** @Data type definition
**===========================================================================================*/
//能量计算结构体
typedef struct
{
    u32 nowEner;                //当前能量(单位:1WH)
    u32 remEner;                //剩余能量(单位:1WH)
    u32 baseEner;               //底端能量(单位:1WH)
    u32 topEner;                //顶端能量(单位:1WH)
    u32 standEner;              //标称能量(单位:1WH)
} __attribute__((aligned(4))) t_ENERFORM;
extern size_t ExtxPortGetFreeHeapSize( void );
extern void *ExtMalloc( size_t xWantedSize );
extern void ExtFree( void *pv );
/*=============================================================================================
** @Name      : void EnerInfoCalcDataInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 调用此函数时，参数配置还没初始化，先按2簇的内存分配
**===========================================================================================*/
void EnerInfoCalcDataInit(void);
/*=============================================================================================
** @Name      : void GroupEnerSoeInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组当前电能SOE计算初始化
** @The notes :
**===========================================================================================*/
void GroupEnerSoeInit(void);
/*=============================================================================================
** @Name      : void GroupEnerSoeCalcTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组当前电能SOE计算任务
** @The notes : 100ms调用,测算当前能量和SOE
**===========================================================================================*/
void GroupEnerSoeCalcTask(void);
/*=============================================================================================
** @Name      : void CorrGNowEnerBySoeAPI(u8 clunum, u16 soe)
** @Input     : clunum:簇编号 soe:目标SOE(0.1%)
** @Output    : void
** @Function  : 根据电池组SOE修正当前电能
** @The notes : 用户需要修正电池组当前电能时调用
**===========================================================================================*/
void CorrGNowEnerBySoeAPI(u8 clunum, u16 soe);
/*=============================================================================================
** @Name      : void CorrGRemainEnerBySoeAPI(u8 clunum, u16 soe)
** @Input     : clunum:簇编号 soe:目标SOE(0.1%)
** @Output    : void
** @Function  : 根据电池组SOE修正剩余电能
** @The notes : 用户需要修正电池组剩余电能时调用
**===========================================================================================*/
void CorrGRemainEnerBySoeAPI(u8 clunum, u16 soe);
/*=============================================================================================
** @Name      : void CorrGroupTotalEnerAPI(u8 clunum, u32 ener)
** @Input     : clunum:簇编号 ener:电池组总能量(单位:1Wh)
** @Output    : void
** @Function  : 修正电池组总能量
** @The notes :
**===========================================================================================*/
void CorrGroupTotalEnerAPI(u8 clunum, u32 ener);
/*=============================================================================================
** @Name      : void CorrGroupStandEnerAPI(u8 clunum, u32 ener)
** @Input     : clunum:簇编号 ener:电池组标称能量(单位:1Wh)
** @Output    : void
** @Function  : 修正电池组标称能量
** @The notes :
**===========================================================================================*/
void CorrGroupStandEnerAPI(u8 clunum, u32 ener);
/*=============================================================================================
** @Name      : u32 GetGroupNowEnerAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前电能,即可用电能(单位:1WH)
** @Function  : 获取电池组当前电能
** @The notes : 可以使用的电能
**===========================================================================================*/
u32 GetGroupNowEnerAPI(u8 clunum);
/*=============================================================================================
** @Name      : u32 GetGroupRemainEnerAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组剩余电能 (单位:1WH)
** @Function  : 获取电池组剩余电能
** @The notes : 包括不能使用的电能
**===========================================================================================*/
u32 GetGroupRemainEnerAPI(u8 clunum);
/*=============================================================================================
** @Name      : u32 GetGroupTotalEnerAPI(u8 clunum)
** @Input     : clunum:簇编号 void
** @Output    : 电池组总电能(单位:1WH)
** @Function  : 获取电池组总电能
** @The notes :
**===========================================================================================*/
u32 GetGroupTotalEnerAPI(u8 clunum);
/*=============================================================================================
** @Name      : u32 GetGroupStandEnerAPI(void)
** @Input     : clunum:簇编号
** @Output    : 电池组标称电能(单位:1WH)
** @Function  : 获取电池组标称电能
** @The notes :
**===========================================================================================*/
u32 GetGroupStandEnerAPI(u8 clunum);
/*=============================================================================================
** @Name      : u16 GetGRealSoeTenThousAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前真实SOE值(0.01%)
** @Function  : 获取电池组当前真实SOE值
** @The notes : 实际可用值，分辨率万分之一
**===========================================================================================*/
u16 GetGRealSoeTenThousAPI(u8 clunum);
/*=============================================================================================
** @Name      : u16 GetGRealSoeMilliAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前真实SOE值(0.1%)
** @Function  : 获取电池组当前真实SOE值
** @The notes : 实际可用值，分辨率千分之一
**===========================================================================================*/
u16 GetGRealSoeMilliAPI(u8 clunum);
/*=============================================================================================
** @Name      : u16 GetGRemainSoeMilliAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 剩余SOE值(0.1%)
** @Function  : 获取剩余SOE值
** @The notes : 相对总能量剩余值，分辨率千分之一
**===========================================================================================*/
u16 GetGRemainSoeMilliAPI(u8 clunum);
/*=============================================================================================
** @Name      : static void CalcGroupNowEnerHandle(void)
** @Input     : void
** @Output    : void
** @Function  : 计算电池组当前电能处理
** @The notes : 实际可放电能量
**===========================================================================================*/
static void CalcGroupNowEnerHandle(void);
/*=============================================================================================
** @Name      : static void CalcGroupRemainEnerHandle(void)
** @Input     : void
** @Output    : void
** @Function  : 计算电池组剩余电能
** @The notes : 包含不可用的电能
**===========================================================================================*/
static void CalcGroupRemainEnerHandle(void);
/*=============================================================================================
** @Name      : static void StoreGroupNowEnerToEEP(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 存储当前可用电能到EEP
** @The notes : 判断存储周期
**===========================================================================================*/
static void StoreGroupNowEnerToEEP(u8 clunum);
/*=============================================================================================
** @Name      : static void StoreGroupRemainEnerToEEP(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 存储当前可用电能到EEP
** @The notes : 判断存储周期
**===========================================================================================*/
static void StoreGroupRemainEnerToEEP(u8 clunum);
/*=============================================================================================
** @Name      : static void StoreGroupTotalEnerToEEP(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 存储实际总电能到EEP
** @The notes : 直接写存储
**===========================================================================================*/
static void StoreGroupTotalEnerToEEP(u8 clunum);
/*=============================================================================================
** @Name      : static void StoreGroupStandEnerToEEP(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 存储实际标称电能到EEP
** @The notes : 直接写存储
**===========================================================================================*/
static void StoreGroupStandEnerToEEP(u8 clunum);
#endif