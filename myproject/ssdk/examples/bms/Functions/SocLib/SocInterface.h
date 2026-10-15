/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : SocInterface.h
** @Founder      : SBK326
** @Date         : 2020.9.9
** @Function     : SOC算法包模块接口函数
** @Instructions :
**===========================================================================================*/
#ifndef _SOCINTERFACE_H
#define _SOCINTERFACE_H

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
//#include "EnerInterface.h"
#include "SocSoeSync.h"
#include "BSPTypeDef.h"
#define CAP_CHG_DHG_PER                 1015                /*充放电电量有效比率(放电容量/充电容量:0.1%)*/
#define ENER_CHG_DHG_PER                955                 /*充放电电能有效比率(放电能量/充电能量:0.1%)*/

#define CAPENER_T_PER_LEN               10                  /*不同温度下可用总容量或总能量百分率表长度*/
#define CAPENER_PER_FST_T               (s8)-30             /*不同温度下可用总容量或总能量百分率表起始温度(1℃)*/
#define CAPENER_PER_SUB_T               10                  /*不同温度下可用总容量或总能量百分率表间隔温差(1℃)*/

#define CAPENER_C_PER_LEN               10                  /*不同电流倍率下可用总容量或总能量百分率表长度*/
#define CAPENER_PER_FST_C               2                   /*不同电流倍率下可用总容量或总能量百分率表起始电流倍率(0.1C)*/
#define CAPENER_PER_SUB_C               2                   /*不同电流倍率下可用总容量或总能量百分率表间隔电流倍率(0.1C)*/

#if((3 == PRJ_PARA_NUM_INFO) || (4 == PRJ_PARA_NUM_INFO))   /*自研PACK电芯*/
#define STEP_CORR_CHG_SUBV              110                 /*充放电末端步长修正起始点与充电截止电压差(1mV)*/
#define STEP_CORR_DHG_SUBV              150                 /*充放电末端步长修正起始点与放电截止电压差(1mV)*/

#else                                                       /*CATL-PACK电芯*/
#define STEP_CORR_CHG_SUBV              150                 /*充放电末端步长修正起始点与充电截止电压差(1mV)*/
#define STEP_CORR_DHG_SUBV              150                 /*充放电末端步长修正起始点与放电截止电压差(1mV)*/
#endif

#define STEP_CORR_FIN_HSOC              993                 /*充放电末端步长修正高压端终端目标SOC值(0.1%)*/
#define STEP_CORR_FIN_LSOC              13                  /*充放电末端步长修正低压端终端目标SOC值(0.1%)*/
#define STEP_CORR_FIN_HSOE              995                 /*充放电末端步长修正高压端终端目标SOE值(0.1%)*/
#define STEP_CORR_FIN_LSOE              15                  /*充放电末端步长修正低压端终端目标SOE值(0.1%)*/

/*=============================================================================================
** @Name      : u8 SocReadCapEnerCorrInfoHook(u8 clunum, u32 *info, u8 len)
** @Input     : clunum:簇编号 info:读取到的信息 len:读取长度
** @Output    : 读取结果:TRUE:成功 FALSE:失败
** @Function  : 读取容量和能量标定检测信息
** @The notes : 从EEPROM读取
**===========================================================================================*/
u8 SocReadCapEnerCorrInfoHook(u8 clunum, u32 *info, u8 len);
/*=============================================================================================
** @Name      : u8 SocWriteCapEnerCorrInfoHook(u8 clunum, u8 sort, u32 *info, u8 len)
** @Input     : clunum:簇编号 sort:记录类型(0:低压点信息 1:高压点信息) info:需记录的信息 len:数据长度
** @Output    : 记录结果:TRUE:成功 FALSE:失败
** @Function  : 记录容量能量自动标定检测信息
** @The notes : 记录到EEPROM存储(要求存储器必须已经写记录标志)
**===========================================================================================*/
u8 SocWriteCapEnerCorrInfoHook(u8 clunum, u8 sort, u32 *info, u8 len);
/*=============================================================================================
** @Name      : u8 SocWriteCorrTotalCapEnerHook(u8 clunum, u8 sort, u32 *info, u8 len)
** @Input     : clunum:簇编号 sort:记录类型(0:低压点信息 1:高压点信息) info:需记录的信息 len:数据长度
** @Output    : 记录结果:TRUE:成功 FALSE:失败
** @Function  : 记录容量能量自动标定检测到的总容量总能量值
** @The notes : 记录到EEPROM存储(要求存储器必须已经写记录标志)
**===========================================================================================*/
u8 SocWriteCorrTotalCapEnerHook(u8 clunum, u8 sort, u32 *info, u8 len);
/*=============================================================================================
** @Name      : u8 SocWriteCapEnerCorrTimeHook(u8 clunum, u8 sort, u32 time)
** @Input     : clunum:簇编号 sort:记录类型(0:低压点信息 1:高压点信息) time:标定时长(1s)
** @Output    : 记录结果:TRUE:成功 FALSE:失败
** @Function  : 记录容量能量自动标定时长信息
** @The notes : 记录到EEPROM存储(要求存储器必须已经写记录标志)
**===========================================================================================*/
u8 SocWriteCapEnerCorrTimeHook(u8 clunum, u8 sort, u32 time);



#endif

