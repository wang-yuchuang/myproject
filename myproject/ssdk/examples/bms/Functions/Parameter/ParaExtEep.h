/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : ParaExtEep.h
** @Founder      : ZQD808
** @Date         : 2025.12.13
** @Function     : 参数存储模块读写外部EEPROM参数
** @Instructions : 
**===============================================================================================*/
#ifndef _PARAEXTEEP_H__
#define _PARAEXTEEP_H__

#include "BSPTypeDef.h"
#include "M2514.h"
/*=================================================================================================
** @Name      : u8 ReadExtEepMonErrTime(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:簇编号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取月故障数目统计时间(年月)
** @The notes :
**===============================================================================================*/
u8 ReadExtEepMonErrTime(u8 clunum, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepCorrZeroCurr(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:簇编号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取电流零飘修正值(有符号,单位10mA)
** @The notes :
**===============================================================================================*/
u8 ReadExtEepCorrZeroCurr(u8 clunum, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepNowCapSoh(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组标号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取当前总容量SOH值
** @The notes :
**===============================================================================================*/
u8 ReadExtEepNowCapSoh(u8 clunum, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepChgTimesState(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号  data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取充放电次数检测状态
** @The notes :
**===============================================================================================*/
u8 ReadExtEepChgTimesState(u8 clunum, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepCapEnerCorrInfo(u8 clunum, u32 *data, u8 len)
** @Input     : clunum: 组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取总容量总能量自动标定信息值
** @The notes :
**===============================================================================================*/
u8 ReadExtEepCapEnerCorrInfo(u8 clunum, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepCapFormInfo(u8 clunum, u32 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取电量计算结构信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepCapFormInfo(u8 clunum, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepEnerFormInfo(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取电能计算结构信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepEnerFormInfo(u8 clunum, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepSysClock(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取系统时钟(北京时间)
** @The notes :
**===============================================================================================*/
u8 ReadExtEepSysClock(u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepSysRunTime(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取系统运行(上电)时长 1Hour
** @The notes :
**===============================================================================================*/
u8 ReadExtEepSysRunTime(u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepChgDhgTimes(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取充放电次数
** @The notes :
**===============================================================================================*/
u8 ReadExtEepChgDhgTimes(u8 clunum, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepHisErrNum(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取历史故障告警数目
** @The notes :
**===============================================================================================*/
u8 ReadExtEepHisErrNum(u8 clunum, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepDODStaInfo(u8 clunum, u16 *data, u8 len)
** @Input     : clunum: 组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取放电深度DOD统计信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepDODStaInfo(u8 clunum, u16 *data, u8 len);
/**=================================================================================================
** @Name      : u8 ReadExtEepHisDodInfoV2(u8 clunum, u16 *data, u16 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度(u16=PARA_DOD_HIS_V2_LEN)
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取历史DOD记录信息V3(标志+Id+50条环形记录×10字)
** @The notes : 逐条校验记录id瓤槽不变量+DOD越界;失败回写默认值
**===============================================================================================*/
u8 ReadExtEepHisDodInfoV2(u8 clunum, u16 *data, u16 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepSOSLockInfo(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取安全等级SOS锁定信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepSOSLockInfo(u8 clunum, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepHisChgDhgCap(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取累计充放电电量 1AH
** @The notes :
**===============================================================================================*/
u8 ReadExtEepHisChgDhgCap(u8 clunum, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepHisChgDhgEner(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取累计充放电电能 0.1kWh
** @The notes :
**===============================================================================================*/
u8 ReadExtEepHisChgDhgEner(u8 clunum, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepGroupGenParaInfo(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取电池组主控通用参数信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepGroupGenParaInfo(u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepGroupHigParaInfo(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取电池组主控高级参数信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepGroupHigParaInfo(u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepGroupHWParaInfo(u8 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取电池组主控硬件参数信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepGroupHWParaInfo(u8 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepSystemHWParaInfo(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取系统设备硬件参数信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepSystemHWParaInfo(u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepSystemWorkMode(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:簇编号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取系统工作模式
** @The notes :
**===============================================================================================*/
u8 ReadExtEepSystemWorkMode(u8 clunum, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 ReadExtEepSystemGenParaInfo(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取系统通用参数信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepSystemGenParaInfo(u16 *data, u8 len);
//BMU01未使用外部EEPROM单个写，可能会造成失败
#if(1)
/*=================================================================================================
** @Name      : u8 WriteExtEepNowCapSoh(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入当前总容量SOH值
** @The notes :
**===============================================================================================*/
u8 WriteExtEepNowCapSoh(u8 clunum, u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepChgTimesState(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入充放电次数检测状态
** @The notes :
**===============================================================================================*/
u8 WriteExtEepChgTimesState(u8 clunum, u8 index, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepCapEnerCorrInfo(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入总容量总能量自动标定信息值
** @The notes :
**===============================================================================================*/
u8 WriteExtEepCapEnerCorrInfo(u8 clunum, u8 index, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepCapFormInfo(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入电量计算结构信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepCapFormInfo(u8 clunum, u8 index, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepEnerFormInfo(u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入电能计算结构信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepEnerFormInfo(u8 clunum,u8 index, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepSysClock(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入系统时钟(北京时间)
** @The notes :
**===============================================================================================*/
u8 WriteExtEepSysClock(u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepSysRunTime(u8 index, u16 data)
** @Input     : index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入系统运行(上电)时长 1Hour
** @The notes :
**===============================================================================================*/
u8 WriteExtEepSysRunTime(u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepChgDhgTimes(u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入充放电次数
** @The notes :
**===============================================================================================*/
u8 WriteExtEepChgDhgTimes(u8 clunum,u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepHisErrNum(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入历史故障告警数目
** @The notes :
**===============================================================================================*/
u8 WriteExtEepHisErrNum(u8 clunum, u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepDODStaInfo(u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入放电深度统计信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepDODStaInfo(u8 clunum, u8 index, u16 *data, u8 len);
/**=================================================================================================
** @Name      : u8 WriteExtEepHisDodInfoV2(u8 clunum, u16 index, u16 *data, u16 len)
** @Input     : clunum:组号 index:字偏移 data:需要写入的数据 len:写入长度(u16)
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入历史DOD记录信息V2(整块或部分写入)
** @The notes : 底层驱动自动处理跨页/跨64字节分块
**===============================================================================================*/
u8 WriteExtEepHisDodInfoV2(u8 clunum, u16 index, u16 *data, u16 len);
/**=================================================================================================
** @Name      : u8 WriteExtEepHisDodInfoRec(u8 clunum, u16 slot, u16 *rec)
** @Input     : clunum:组号 slot:记录槽位[0,HISDOD_REC_NUM) rec:单条记录(10个u16)
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入单条DOD记录V2(仅12字节,减少E2磨损)
** @The notes : 写头(Id)必须在写记录之后(掉电一致性)
**===============================================================================================*/
u8 WriteExtEepHisDodInfoRec(u8 clunum, u16 slot, u16 *rec);
/*=================================================================================================
** @Name      : u8 WriteExtEepSOSLockInfo(u8 clunum,u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入安全等级信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepSOSLockInfo(u8 clunum,u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepHisChgDhgCap(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入累计充放电电量 1AH
** @The notes :
**===============================================================================================*/
u8 WriteExtEepHisChgDhgCap(u8 clunum, u8 index, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepHisChgDhgEner(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入累计充放电电能 0.1kWh
** @The notes :
**===============================================================================================*/
u8 WriteExtEepHisChgDhgEner(u8 clunum, u8 index, u32 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepGroupGenParaInfo(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入电池组主控通用参数信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepGroupGenParaInfo(u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepGroupHigParaInfo(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入电池组主控高级参数信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepGroupHigParaInfo(u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepGroupHWParaInfo(u8 index, u8 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入电池组主控硬件参数信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepGroupHWParaInfo(u8 index, u8 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepSystemWorkMode(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:簇编号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入系统工作模式信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepSystemWorkMode(u8 clunum, u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepSystemGenParaInfo(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入系统设备通用参数信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepSystemGenParaInfo(u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepSystemHWParaInfo(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入系统设备硬件参数信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepSystemHWParaInfo(u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepMonErrTime(u8 index, u16 data)
** @Input     : index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入月历史故障统计时间(年月)
** @The notes :
**===============================================================================================*/
u8 WriteExtEepMonErrTime(u8 clunum, u8 index, u16 *data, u8 len);
/*=================================================================================================
** @Name      : u8 WriteExtEepCorrZeroCurr(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入电流零飘修正值
** @The notes :
**===============================================================================================*/
u8 WriteExtEepCorrZeroCurr(u8 clunum, u8 index, u16 *data, u8 len);
#endif
#endif