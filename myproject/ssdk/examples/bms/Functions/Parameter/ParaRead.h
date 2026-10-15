/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : ParaRead.h
** @Founder      : ZQD808
** @Date         : 2025.12.13
** @Function     : 参数存储模块读写参数初始化
** @Instructions : 
**===============================================================================================*/
#ifndef _PARAREAD_H__
#define _PARAREAD_H__

#include "BSPTypeDef.h"
#include "M2514.h"
#include "ParaExtEep.h"

/*=================================================================================================
** @Name      : u8 ParaReadStoreDataByte(u16 addr, u8 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取指定长度字节数
** @The notes : 读取先后顺序:内部EEPROM
**===============================================================================================*/
u8 ParaReadStoreDataByte(u16 addr, u8 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreCorrZeroCurr(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取电流零飘修正值初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreCorrZeroCurr(u8 clunum, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreNowCapSoh(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取当前容量SOH值初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreNowCapSoh(u8 clunum, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreMonErrTime(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:簇编号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取月故障统计时间(年月)初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreMonErrTime(u8 clunum, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreChgTimesState(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取充放电次数检测状态初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreChgTimesState(u8 clunum, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreCapEnerCorrInfo(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取总容量总能量自动标定信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreCapEnerCorrInfo(u8 clunum, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreCapFormInfo(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取电量计算结构信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreCapFormInfo(u8 clunum, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreEnerFormInfo(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取电能计算结构信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreEnerFormInfo(u8 clunum, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreSysClock(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取系统时钟(北京时间)初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreSysClock(u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreSysRunTime(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取系统运行(上电)时长初始化 1Hour
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreSysRunTime(u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreChgDhgTimes(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取充放电次数初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreChgDhgTimes(u8 clunum, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreHisErrNum(u8 clunum,u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取历史故障告警数目初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreHisErrNum(u8 clunum,u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreDODStaInfo(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取放电深度统计信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreDODStaInfo(u8 clunum, u16 *data, u8 len);
/**=================================================================================================
** @Name      : u8 ParaReadStoreHisDodInfoV2(u8 clunum, u16 *data, u16 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度(u16)
** @Output    : 读取成功或失败
** @Function  : 从存储区读取历史DOD记录信息V2初始化(环形缓冲)
** @The notes : 读取先后顺序:外部EEPROM
**===============================================================================================*/
u8 ParaReadStoreHisDodInfoV2(u8 clunum, u16 *data, u16 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreSOSLockInfo(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取安全等级SOS锁定信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreSOSLockInfo(u8 clunum, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreHisChgDhgCap(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取累计充放电电量初始化 1AH
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreHisChgDhgCap(u8 clunum, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreHisChgDhgEner(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取累计充放电电能初始化 0.1kWh
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreHisChgDhgEner(u8 clunum, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreGroupGenParaInfo(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取电池组主控通用参数信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreGroupGenParaInfo(u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreGroupHigParaInfo(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取电池组主控高级参数信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreGroupHigParaInfo(u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreGroupHWParaInfo(u8 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取电池组主控硬件参数信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreGroupHWParaInfo(u8 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreSystemWorkMode(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:簇编号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取系统工作模式
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreSystemWorkMode(u8 clunum, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreSystemGenParaInfo(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取系统通用参数信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreSystemGenParaInfo(u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreSystemHWParaInfo(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取系统硬件参数信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreSystemHWParaInfo(u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaReadStoreN2402Parameter(u8 clunum, u8 *data, u8 len)
** @Input     : clunum:簇编号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取N2402参数初始化
** @The notes : 读取外部EEPROM
**===============================================================================================*/
#endif