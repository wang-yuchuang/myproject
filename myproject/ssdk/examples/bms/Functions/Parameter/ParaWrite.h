/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : ParaWrite.h
** @Founder      : ZQD808
** @Date         : 2025.12.15
** @Function     : 参数存储模块写入参数处理
** @Instructions : 
**===============================================================================================*/
#ifndef __PARAWRITE_H__
#define __PARAWRITE_H__

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "M2514.h"
#include "ParaExtEep.h"
/*=================================================================================================
** @Name      : u8 ParaWriteStoreNowCapSoh(u8 clunum,u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入当前总容量SOH值
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreNowCapSoh(u8 clunum,u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreChgTimesState(u8 clunum,u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入充放电次数检测状态
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreChgTimesState(u8 clunum,u8 index, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreCapEnerCorrInfo(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入总容量总能量自动标定信息值
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreCapEnerCorrInfo(u8 clunum, u8 index, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreCapFormInfo(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入电量计算结构信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreCapFormInfo(u8 clunum, u8 index, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreEnerFormInfo(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入电能计算结构信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreEnerFormInfo(u8 clunum, u8 index, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreSysClock(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入系统时钟(北京时间)
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreSysClock(u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreSysRunTime(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入系统运行(上电)时长 1Hour
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreSysRunTime(u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreChgDhgTimes(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入充放电次数
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreChgDhgTimes(u8 clunum, u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreHisErrNum(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入历史故障告警数目
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreHisErrNum(u8 clunum, u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreDODStaInfo(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入放电深度统计信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreDODStaInfo(u8 clunum, u8 index, u16 *data, u8 len);
/**=================================================================================================
** @Name      : u8 ParaWriteStoreHisDodInfoV2(u8 clunum, u16 index, u16 *data, u16 len)
** @Input     : clunum:组号 index:字偏移 data:需要写入的数据 len:写入长度(u16)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入历史DOD记录信息V2(整块或部分)
** @The notes : 写入外部EEPROM
**===============================================================================================*/
u8 ParaWriteStoreHisDodInfoV2(u8 clunum, u16 index, u16 *data, u16 len);
/**=================================================================================================
** @Name      : u8 ParaWriteStoreHisDodInfoRecV2(u8 clunum, u16 slot, u16 *rec)
** @Input     : clunum:组号 slot:记录槽位[0,HISDOD_REC_NUM) rec:单条记录(10个u16)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入单条历史DOD记录V2(增量写入减少E2磨损)
** @The notes : 写入外部EEPROM;写头(Id)须在写记录之后
**===============================================================================================*/
u8 ParaWriteStoreHisDodInfoRecV2(u8 clunum, u16 slot, u16 *rec);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreSOSLockInfo(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入安全状态SOS锁定信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreSOSLockInfo(u8 clunum, u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreHisChgDhgCap(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入累计充放电电量 1AH
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreHisChgDhgCap(u8 clunum, u8 index, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreHisChgDhgEner(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum: 组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入累计充放电电能 0.1kWh
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreHisChgDhgEner(u8 clunum, u8 index, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreGroupGenParaInfo(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入电池组主控通用参数信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreGroupGenParaInfo(u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreGroupHigParaInfo(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入电池组主控高级参数信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreGroupHigParaInfo(u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreGroupHWParaInfo(u8 index, u8 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入电池组主控硬件参数信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreGroupHWParaInfo(u8 index, u8 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreSystemWorkMode(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:簇编号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入系统系统工作模式信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreSystemWorkMode(u8 clunum, u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreSystemGenParaInfo(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入系统设备通用参数信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreSystemGenParaInfo(u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreSystemHWParaInfo(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入系统设备硬件参数信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreSystemHWParaInfo(u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreMonErrTime(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:簇编号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入月故障统计时间(年月)
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreMonErrTime(u8 clunum, u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreCorrZeroCurr(u8 clunum,u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入电流零飘修正值
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreCorrZeroCurr(u8 clunum,u8 index, u16 *data, u8 len);
u8 ParaWriteStoreModuleCount(u8 index, u8 *data, u8 len);
u8 ParaWriteStoreModuleType(u8 index, u8 *data, u8 len);
u8 ParaWriteStoreACDParam(u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ParaWriteStoreN2402Parameter(u8 clunum, u8 *data, u8 len)
** @Input     : clunum:簇编号 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入N2402参数
** @The notes : 写入外部EEPROM
**===============================================================================================*/
#endif