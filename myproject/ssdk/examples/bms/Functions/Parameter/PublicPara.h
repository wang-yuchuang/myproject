/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : PublicPara.h
** @Founder      : SBK326
** @Date         : 2025.12.12
** @Function     : 系统公共参数存储模块
** @Instructions : 
**===============================================================================================*/
#ifndef _PUBLICPARA_H
#define _PUBLICPARA_H
#include "BSPTypeDef.h"

#define SINDEX100_LEN			(eSGenPara100_CfgFin)		/*系统通用设置结果数组长度*/
#define SINDEX101_LEN			(eSHWPara101_CfgFin)		/*系统高级设置结果数组长度*/

#define SINDEX105_LEN			(eSGenPara105_CfgFin)		/*系统通用设置数组长度*/
#define SINDEX106_LEN			(eSHWPara106_CfgFin)		/*系统硬件设置数组长度*/

#define MIN_BMU_NUM_CFG                 (1)                 /*并机组数最小配置值*/
#define MAX_BMU_NUM_CFG                 (16)                /*并机组数最大配置值*/
#define MIN_EXT_ADDR_CFG                (70)                /*上层设备最小配置值*/
#define MAX_EXT_ADDR_CFG                (89)                /*上层设备最大配置值*/

//协议100子索引列表
typedef enum
{
	eSGenPara100_CfgFlag = 0,           	/*系统通用参数配置标志 0x5a5a(23130)有效*/
	eSGenPara100_AutConn = 1,				/*电池组动态并机使能状态 [0,1] 0:同时接入 1:动态接入*/
	eSGenPara100_ErrSepa = 2,				/*电池组故障切除使能 状态[0,1] 0:同时切除 1:动态切除*/
	eSGenPara100_ChgSepa = 3,				/*电池组充电切除使能 状态[0,1] 0:同时切除 1:动态切除*/
	eSGenPara100_DhgSepa = 4,				/*电池组放电切除使能状态 [0,1] 0:同时切除 1:动态切除*/
	eSGenPara100_MinGNum = 5,				/*电池组最小运行组数值 [1,16](最少正常电池组数)*/
	eSGenPara100_AllGNum = 6,				/*电池组并机总组数值 [1,16](母线上电池组数)*/
	eSGenPara100_ExtAddr = 7,				/*上层通信设备地址(BSMU或PCS) [70,89]*/

	eSGenPara100_CfgFin
}e_SysGenParaR100;

//协议101子索引列表
typedef enum
{
	eSHWPara101_CfgFlag = 0,		        /*系统高级参数配置标志 0x5a5a(23130)有效*/
	eSHWPara101_PermPort,					/*权限默认开启端口 [0,9]	0:均不开启 1:全部开启 [2,5]:CAN口1-4 [6,9]:RS485口1-4*/
	eSHWPara101_CAN0Rate,					/*CAN#1口波特率  1Kbps	125,250,500,1000*/
	eSHWPara101_CAN1Rate,					/*CAN#2口波特率  1Kbps	125,250,500,1000*/
	eSHWPara101_CAN2Rate,					/*CAN#3口波特率  1Kbps	125,250,500,1000*/

	eSHWPara101_RemoteAddr,					/*远程端口站地址  [1,250]*/
	eSHWPara101_RemoteRate,					/*远程端口波特率  1bps	 9600,19200,38400*/
	eSHWPara101_LocalAddr,					/*就地端口站地址  [1,250]*/
	eSHWPara101_LocalRate,					/*就地端口波特率  1bps	 9600,19200,38400*/
	eSHWPara101_MonAddr,					/*监控端口站地址  [1,250]*/
	eSHWPara101_MonRate,					/*监控端口波特率  1bps	 9600,19200,38400*/
	eSHWPara101_SelfIPNet,					/*自身IP网段号  [1,254]*/
	eSHWPara101_SelfIPHost,					/*自身IP主机号  [1,254]*/
	eSHWPara101_SrvIPNet,					/*服务器IP网段号  [1,254]*/
	eSHWPara101_SrvIPHost,					/*服务器IP主机号  [1,254]*/
	eSHWPara101_DevAddr,					/*设备地址  [50,65]*/

	eSHWPara101_CfgFin
}e_SysHWParaR101;

//协议105子索引列表
typedef enum
{
	eSGenPara105_CfgFlag = 0,		        /*读系统常规参数配置标志 0x5a5a(23130)有效*/
	eSGenPara105_SynchEN,					/*电池组动态并机使能设置 [0,1]	0:同时接入 1:动态接入*/
	eSGenPara105_ErrSepEn,					/*电池组故障切除使能设置 [0,1]	0:同时切除 1:动态切除*/
	eSGenPara105_ChgSepEn,					/*电池组充电切除使能设置 [0,1]	0:同时切除 1:动态切除*/
	eSGenPara105_DhgSepEn,					/*电池组放电切除使能设置 [0,1]	0:同时切除 1:动态切除*/
	eSGenPara105_MinRunNum,					/*电池组最小运行组数设置 [1,16]	最少正常电池组数*/
	eSGenPara105_ManagGNum,					/*电池组并机总组数设置 [1,16]	母线上电池组数*/
	eSGenPara105_ExtAddr,			        /*上层通信设备地址(BSMU或PCS) [70,89]*/
	eSGenPara105_GroupEn,					/*电池组激活状态设置 按位表示，每bit表示一个电池组*/

	eSGenPara105_CfgFin						/*系统通用参数设置结束*/
}e_SysGenParaCfg105;

//协议106子索引列表
typedef enum
{
	eSHWPara106_CfgFlag = 0,		        /*写系统高级参数配置标志 0恢复出厂设置*/
	eSHWPara106_PermPort,					/*权限默认开启端口设置 [0,9]	0:均不开启 1:全部开启 [2,5]:CAN口1-4 [6,9]:RS485口1-4*/
	eSHWPara106_CAN0Rate,					/*CAN#1口波特率设置  1Kbps	125,250,500,1000*/
	eSHWPara106_CAN1Rate,					/*CAN#2口波特率设置  1Kbps	125,250,500,1000*/
	eSHWPara106_CAN2Rate,					/*CAN#3口波特率设置  1Kbps	125,250,500,1000*/

	eSHWPara106_RemoteAddr,					/*远程端口站地址设置  [1,250]*/
	eSHWPara106_RemoteRate,					/*远程端口波特率设置  1bps	 9600,19200,38400*/
	eSHWPara106_LocalAddr,					/*就地端口站地址设置  [1,250]*/
	eSHWPara106_LocalRate,					/*就地端口波特率设置  1bps	 9600,19200,38400*/
	eSHWPara106_MonAddr,					/*监控端口站地址设置  [1,250]*/
	eSHWPara106_MonRate,					/*监控端口波特率设置  1bps	 9600,19200,38400*/
	eSHWPara106_SelfIPNet,					/*自身IP网段号设置  [1,254]*/
	eSHWPara106_SelfIPHost,					/*自身IP主机号设置  [1,254]*/
	eSHWPara106_SrvIPNet,					/*服务器IP网段号设置  [1,254]*/
	eSHWPara106_SrvIPHost,					/*服务器IP主机号设置  [1,254]*/
	eSHWPara106_DevAddr,					/*设备地址设置  [50,65]*/

	eSHWPara106_CfgFin
}e_SysHWParaCfg106;

/*=================================================================================================
** @Interface function declaration
**===============================================================================================*/
/*=================================================================================================
** @Name      : u8 ParaCheckSGenParaEffect(u32 addr, u32 para)
** @Input     : addr:参数地址 para:参数值
** @Output    : TRUE:有效合法 FALSE:非法无效
** @Function  : 检测系统通用参数值是否有效
** @The notes : U16
**===============================================================================================*/
u8 ParaCheckSGenParaEffect(u32 addr, u32 para);

/*=================================================================================================
 ** @Name      : u8 ParaCheckSHWParaEffect(u32 addr, u32 para)
 ** @Input     : addr:参数地址 para:参数值
 ** @Output    : TRUE:有效合法 FALSE:非法无效
 ** @Function  : 检测系统硬件参数值是否有效
 ** @The notes : U16
 **===============================================================================================*/
 u8 ParaCheckSHWParaEffect(u32 addr, u32 para);
#endif