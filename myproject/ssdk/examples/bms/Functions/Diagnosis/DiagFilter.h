/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : DiagFilter.h
** @Founder      : ZQ808
** @Date         : 2026.03.05
** @Function     : 电池故障诊断模块故障滤波接口
** @Instructions : 
**===============================================================================================*/
#ifndef _DIAGFILTER_H
#define _DIAGFILTER_H
#include "BSPTypeDef.h"
#include "DiagInterface.h"
#include <cmsis_os2.h>
#define DIAG_FILTER_TICK            50                      /*故障诊断滤波周期[必须与实际周期匹配](1ms)*/

extern size_t ExtxPortGetFreeHeapSize( void );
extern void *ExtMalloc( size_t xWantedSize );
extern void ExtFree( void *pv );

/*=============================================================================================
** @Name      :void DiagFilterInfoInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 由于片内内存限制，使用外部EMMC空间，全部使用ExtMalloc申请内存
**===========================================================================================*/
void DiagFilterInfoInit(void);
/*=================================================================================================
** @Name      : void DiagErrFilterInit(void)
** @Input     : swhSort:接触器名称[0,11] state:控制接触器状态 0:断开 1:闭合
** @Output    : void
** @Function  : 故障诊断滤波初始化
** @The notes : 滤波处理数组清零
**===============================================================================================*/
void DiagErrFilterInit(void);
/*=================================================================================================
** @Name      : void DiagErrFilterTask(u8 from, u8 to)
** @Input     : from-to:检测故障滤波起始和结束ID(包括from不包to)
** @Output    : void
** @Function  : 故障诊断滤波任务
** @The notes : 50ms周期调用,与DIAG_FILTER_TICK一致
**===============================================================================================*/
void DiagErrFilterTask(e_ERR_ID from, e_ERR_ID to);
/*=================================================================================================
** @Name      : void DiagErrFilStartAPI(u8 clunum, e_ERR_ID id)
** @Input     : clunum:簇编号 id:故障ID
** @Output    : void
** @Function  : 启动该故障诊断滤波
** @The notes :
**===============================================================================================*/
void DiagErrFilStartAPI(u8 clunum, e_ERR_ID id);
/*=================================================================================================
** @Name      : void DiagErrFilStopAPI(u8 clunum, e_ERR_ID id)
** @Input     : clunum:簇编号 id:故障ID
** @Output    : void
** @Function  : 停止该故障诊断滤波
** @The notes :
**===============================================================================================*/
void DiagErrFilStopAPI(u8 clunum, e_ERR_ID id);
/*=================================================================================================
** @Name      : void DiagSetErrNowStateAPI(u8 clunum, e_ERR_ID id, e_ErrState state)
** @Input     : clunum:簇编号 id:故障ID state:目标故障状态
** @Output    : void
** @Function  : 设置故障诊断滤波当前瞬时状态
** @The notes :
**===============================================================================================*/
void DiagSetErrNowStateAPI(u8 clunum, e_ERR_ID id, e_ErrState state);
/*=================================================================================================
** @Name      : void DiagSetErrFilStateAPI(u8 clunum, e_ERR_ID id, e_ErrState state)
** @Input     : clunum:簇编号 id:故障ID state:目标故障状态
** @Output    : void
** @Function  : 设置故障诊断滤波当前稳定状态
** @The notes : 只能eErr_A或eErr_B
**===============================================================================================*/
void DiagSetErrFilStateAPI(u8 clunum, e_ERR_ID id, e_ErrState state);
/*=================================================================================================
** @Name      : void DiagClrErrNorStateAPI(u8 clunum, e_ERR_ID id)
** @Input     : clunum:簇编号 id:故障ID,0xff所有故障
** @Output    : void
** @Function  : 清除故障诊断滤波当前故障状态
** @The notes :
**===============================================================================================*/
void DiagClrErrNorStateAPI(u8 clunum, e_ERR_ID id);
/*=================================================================================================
** @Name      : e_ErrSort DiagGetFilErrSortAPI(e_ERR_ID id)
** @Input     : id:故障ID
** @Output    : 故障类型[0,8]
** @Function  : 获取故障类型
** @The notes :
**===============================================================================================*/
e_ErrSort DiagGetFilErrSortAPI(e_ERR_ID id);
/*=================================================================================================
** @Name      : e_ErrState DiagGetErrNowStateAPI(u8 clunum, e_ERR_ID id)
** @Input     : clunum:簇编号 id:故障ID
** @Output    : 当前瞬时故障状态
** @Function  : 获取故障当前瞬时状态
** @The notes : 不包括过渡状态
**===============================================================================================*/
e_ErrState DiagGetErrNowStateAPI(u8 clunum, e_ERR_ID id);
/*=================================================================================================
** @Name      : e_ErrState DiagGetErrFilStateAPI(u8 clunum, e_ERR_ID id)
** @Input     : clunum:簇编号 id:故障ID
** @Output    : 当前滤波故障状态
** @Function  : 获取故障当前滤波状态
** @The notes : 包括过渡状态
**===============================================================================================*/
e_ErrState DiagGetErrFilStateAPI(u8 clunum, e_ERR_ID id);
/*=================================================================================================
** @Name      : e_ErrState DiagGetErrLastStateAPI(u8 clunum, e_ERR_ID id)
** @Input     : clunum:簇编号  id:故障ID
** @Output    : 当前稳定故障状态
** @Function  : 获取故障当前稳定状态
** @The notes : 不包括过渡状态
**===============================================================================================*/
e_ErrState DiagGetErrLastStateAPI(u8 clunum, e_ERR_ID id);
/*=================================================================================================
** @Name      : u16 DiagGetFilAtoBTimeAPI(u8 id)
** @Input     : id:故障ID
** @Output    : 故障滤波故障发生检测时长 0.1s
** @Function  : 获取故障滤波故障发生检测时长
** @The notes : 故障发生延时
**===============================================================================================*/
u16 DiagGetFilAtoBTimeAPI(u8 id);
/*=================================================================================================
** @Name      : e_ErrSort DiagGetGroupNowErrLevelAPI(u8 clunum, u8 type)
** @Input     : clunum:簇编号 type:故障类别:0::全部故障 1;充电类故障 2:放电类故障
** @Output    : 电池组当前故障级别[0,8]
** @Function  : 获取电池组当前故障级别
** @The notes : 当前故障的最高级别
**===============================================================================================*/
e_ErrSort DiagGetGroupNowErrLevelAPI(u8 clunum, u8 type);
#endif