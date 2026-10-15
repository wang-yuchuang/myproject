/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : DiagUser.h
** @Founder      : ZQD808
** @Date         : 2026.03.05
** @Function     : 电池组故障诊断模块用户接口
** @Instructions : 
**===============================================================================================*/
#ifndef _DIAGUSER_H
#define _DIAGUSER_H
#include "BSPTypeDef.h"
#include "BalanTactic.h"
#include "DiagInterface.h"
#include <cmsis_os2.h>
#define DIAG_SLAVE_NUM              16                      /*监测从控数目*/
#define DIAG_WAIT_CELL_T            5                       /*等待电芯信息就绪时间(1s)*/
#define DIAG_LVPWR_WAR_T            80                      /*低压电源告警温度(1℃)*/
#define DIAG_LVPWR_EXT_T            100                     /*低压电源极限温度(1℃)*/
#define DIAG_BALAN_PRO_T            BALAN_PAS_PRO_TEMP      /*单体均衡保护温度(1℃)*/
#define DIAG_BALAN_EXT_T            110                     /*单体均衡温度极限值(1℃)*/
#if defined(CONFIG_XIP_MODE)
/* 外部Flash异常检测:XIP形态OSPI2为执行介质,自检会重配OSPI2/擦写Flash与R5取指互斥 → 编译期强制关闭 */
#define DIAG_MFLA_CHECK_EN          0
#else
/* 外部Flash异常检测使能(需配合GD25Q32驱动编译+OSUserHook挂载EXTFlashCheckTask+BSPConfig挂载NORFlashGD25Init) */
#define DIAG_MFLA_CHECK_EN          1
#endif

#define ALARM_DATATYPE              16                      /*告警数据类型*/
#define ALARM_INDEX_LEN             10                       /*告警索引个数*/

#define FAULT_DATATYPE              16                      /*故障数据类型*/
#define FAULT_INDEX_LEN             8                       /*故障索引个数*/

#define ERR_RCD_ERRINFO_LEN         6                       /*一条历史故障信息长度(Word)*/
#define ERR_RCD_WARCODE_FST         0x0100                  /*故障记录告警码前缀*/
#define ERR_RCD_ERRCODE_FST         0x0200                  /*故障记录故障码前缀*/
extern size_t ExtxPortGetFreeHeapSize( void );
extern void *ExtMalloc( size_t xWantedSize );
extern void ExtFree( void *pv );

/*故障记录类型*/
typedef enum
{
    eERR_RCDSORT_WARSET = 1,                /*告警产生*/
    eERR_RCDSORT_WARCLR = 2,                /*告警消除*/
    eERR_RCDSORT_ERRSET = 3,                /*故障产生*/
    eERR_RCDSORT_ERRCLR = 4,                /*故障消除*/

    eERR_RCDSORT_LEN                        /*故障类型*/
}eErr_RcdSort;
/*=================================================================================================
** @Name      : void DiagErrDisplayTask(void)
** @Input     : void
** @Output    : void
** @Function  : 故障显示任务
** @The notes : 100ms周期,将滤波故障转入故障显示位
**===============================================================================================*/
void DiagErrDisplayTask(void);
/*=============================================================================================
** @Name      : void DiagUserInfoInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 由于片内内存限制，使用外部EMMC空间，全部使用ExtMalloc申请内存
**===========================================================================================*/
void DiagUserInfoInit(void);
#endif