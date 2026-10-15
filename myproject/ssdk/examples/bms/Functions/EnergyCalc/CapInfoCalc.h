/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CapInfoCalc.h
** @Founder      : ZSY342
** @Date         : 2019.10.30
** @Function     : SOC容量计算模块
** @Instructions :
**===========================================================================================*/
#ifndef _CAPINFOCALC_H
#define _CAPINFOCALC_H
#include "BSPTypeDef.h"
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>
/*=============================================================================================
** @Macro definition
**===========================================================================================*/
#define CAP_ZERO_POINT             1000000      /*容量计算的零点值(底端容量)*/
#define SOC_CHANG_TO_WRITE_EEP     1            /*当前容量进行写EEP的SOC变化值(单位:0.1%)*/

/*=============================================================================================
** @Data type definition
**===========================================================================================*/
//容量计算结构体
typedef struct
{
	u32 nowCap;                 /*当前容量(单位:1mAH)*/
	u32 remCap;                 /*剩余容量(单位:1mAH)*/
	u32 baseCap;                /*底端容量(单位:1mAH)*/
	u32 topCap;                 /*顶端容量(单位:1mAH)*/
	u32 standCap;               /*标称容量(单位:1mAH)*/
} __attribute__((aligned(4))) t_CAPFORM;
extern size_t ExtxPortGetFreeHeapSize( void );
extern void *ExtMalloc( size_t xWantedSize );
extern void ExtFree( void *pv );
/*=============================================================================================
** @Name      : void CapInfoCalcDataInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 调用此函数时，参数配置还没初始化，先按2簇的内存分配
**===========================================================================================*/
void CapInfoCalcDataInit(void);
/*=============================================================================================
** @Name      : void GroupCapSocInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组当前容量SOC计算初始化
** @The notes :
**===========================================================================================*/
void GroupCapSocInit(void);
/*=============================================================================================
** @Name      : void GroupCapSocCalcTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组当前容量SOC计算任务
** @The notes : 100ms调用,测算总容量和SOC
**===========================================================================================*/
void GroupCapSocCalcTask(void);
/*=============================================================================================
** @Name      : void CorrGNowCapBySocAPI(u8 clunum, u16 soc)
** @Input     : clunum:簇编号 soc:目标SOC(0.1%)
** @Output    : void
** @Function  : 根据电池组SOC修正当前容量
** @The notes :
**===========================================================================================*/
void CorrGNowCapBySocAPI(u8 clunum, u16 soc);
/*=============================================================================================
** @Name      : void CorrGRemainCapBySocAPI(u8 clunum, u16 soc)
** @Input     : clunum:簇编号 soc:目标SOC(0.1%)
** @Output    : void
** @Function  : 根据电池组SOC修正剩余电量
** @The notes : 相对总容量剩余值
**===========================================================================================*/
void CorrGRemainCapBySocAPI(u8 clunum, u16 soc);
/*=============================================================================================
** @Name      : void CorrGroupTotalCapAPI(u8 clunum, u32 cap)
** @Input     : clunum:簇编号 cap:电池组总容量(单位:1mAH)
** @Output    : void
** @Function  : 修正电池组总容量
** @The notes :
**===========================================================================================*/
void CorrGroupTotalCapAPI(u8 clunum, u32 cap);
/*=============================================================================================
** @Name      : void CorrGroupStandCapAPI(u8 clunum, u32 cap)
** @Input     : clunum:簇编号 cap:电池组标称容量(单位:1mAH)
** @Output    : void
** @Function  : 修正电池组标称容量
** @The notes :
**===========================================================================================*/
void CorrGroupStandCapAPI(u8 clunum, u32 cap);
/*=============================================================================================
** @Name      : u32 GetGroupNowCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前容量(单位:1mAH)
** @Function  : 获取电池组当前电量
** @The notes : 剩余能够使用的电量
**===========================================================================================*/
u32 GetGroupNowCapAPI(u8 clunum);
/*=============================================================================================
** @Name      : u32 GetGroupRemainCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组剩余电量(单位:1mAH)
** @Function  : 获取电池组剩余电量
** @The notes : 包括放不出的电量
**===========================================================================================*/
u32 GetGroupRemainCapAPI(u8 clunum);
/*=============================================================================================
** @Name      : u32 GetGroupTotalCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组总容量(单位:1mAH)
** @Function  : 获取电池组总容量
** @The notes :
**===========================================================================================*/
u32 GetGroupTotalCapAPI(u8 clunum);
/*=============================================================================================
** @Name      : u32 GetGroupStandCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组标称容量(单位:1mAH)
** @Function  : 获取电池组标称容量
** @The notes :
**===========================================================================================*/
u32 GetGroupStandCapAPI(u8 clunum);
/*=============================================================================================
** @Name      : u16 GetGRealSocTenThousAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组真实SOC值(单位:万分之一)
** @Function  : 获取电池组真实SOC值
** @The notes :
**===========================================================================================*/
u16 GetGRealSocTenThousAPI(u8 clunum);
/*=============================================================================================
** @Name      : u16 GetGRealSocMilliAPI(u8 clunum)
** @Input     : void
** @Output    : 电池组真实SOC值(单位:千分之一)
** @Function  : 获取电池组真实SOC值
** @The notes :
**===========================================================================================*/
u16 GetGRealSocMilliAPI(u8 clunum);
/*=============================================================================================
** @Name      : u16 GetGRemainSocMilliAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组剩余SOC值(单位:千分之一)
** @Function  : 获取电池组剩余SOC值
** @The notes :
**===========================================================================================*/
u16 GetGRemainSocMilliAPI(u8 clunum);
/*=============================================================================================
** @Name      : static void CalcGroupNowCapHandle(void)
** @Input     : void
** @Output    : void
** @Function  : 计算电池组当前容量处理
** @The notes : 实际可放电容量
**===========================================================================================*/
static void CalcGroupNowCapHandle(void);
/*=============================================================================================
** @Name      : static void CalcGroupRemainCapHandle(void)
** @Input     : void
** @Output    : void
** @Function  : 计算电池组剩余容量
** @The notes : 包括不可用的容量
**===========================================================================================*/
static void CalcGroupRemainCapHandle(void);
/*=============================================================================================
** @Name      : static void StoreGroupNowCapToEEP(void)
** @Input     : void
** @Output    : void
** @Function  : 存储电池组当前可用容量到EEP
** @The notes : 判断存储周期
**===========================================================================================*/
static void StoreGroupNowCapToEEP(void);
/*=============================================================================================
** @Name      : static void StoreGroupRemainCapToEEP(void)
** @Input     : void
** @Output    : void
** @Function  : 存储电池组剩余容量到EEP
** @The notes : 判断存储周期
**===========================================================================================*/
static void StoreGroupRemainCapToEEP(void);
/*=============================================================================================
** @Name      : static void StoreGroupTotalCapToEEP(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 存储电池组总容量到EEP
** @The notes : 直接写EEPROM
**===========================================================================================*/
static void StoreGroupTotalCapToEEP(u8 clunum);
/*=============================================================================================
** @Name      : static void StoreGroupStandCapToEEP(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 存储电池组标称容量到EEP
** @The notes : 直接写EEPROM
**===========================================================================================*/
static void StoreGroupStandCapToEEP(u8 clunum);
#endif