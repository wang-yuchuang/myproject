
#ifndef	CANMODSENDINTERFACE_H
#include "../ccu_sampler_main.h"
#define	CANMODSENDINTERFACE_H

#include "../ccu_sampler_main.h"

#define R_PRPTO_DEVNO_0A	    0x0A            /*设备号：0x0A-单模块*/
#define R_PRPTO_DEVNO_0B	    0x0B            /*设备号：0x0B-模块组*/
#define R_PRPTO_DEVNO_RAM       0x09            /*设备号：0x09-模块RAM*/

#define W_VALUE_A0              0xA0            /*设置值：0xA0*/
#define W_VALUE_A1              0xA1            /*设置值：0xA1*/
#define W_VALUE_A2              0xA2            /*设置值：0xA2*/

#define ABU_Addr                0xFA            /*ABU地址*/
#define MPPT_MODULE_BASE_ADDR   0x80            /*MPPT单模块地址：0x80-0xDF*/

#define	RECT_STAT_CODE_0_ALM_BITs	0x2F	
#define	RECT_STAT_CODE_1_ALM_BITs	0xBE
#define	RECT_STAT_CODE_2_ALM_BITs	0x7F

#define	RECT_STAT_CODE_0_ALM_BITs_PCS	0x00	
#define	RECT_STAT_CODE_1_ALM_BITs_PCS	0x0F
#define	RECT_STAT_CODE_2_ALM_BITs_PCS	0xFF
#define	RECT_STAT_CODE_3_ALM_BITs_PCS	0xFF

//兼容应用层发送报文结构
typedef struct
{
    UINT Id;
    UINT Data[2];
} canmsg;

#define	GRP_SerialParallelMode_SET_Serial	2   /*高低压模式：0-不需要设置 1-低压模式 2-高压模式*/
/*=================================================================================================
** @Data type definition
**===============================================================================================*/

/*=================================================================================================
** @Interface function declaration
**===============================================================================================*/
/*=================================================================================================
** @Name      : BYTE TxCAN3Buff(canmsg * pRcvMsg, BYTE u8Len)
** @Input     : pRcvMsg:发送报文 u8Len:报文长度
** @Output    : FALSE:发送失败 TRUE：发送成功
** @Function  : CAN1报文发送
** @The notes :
**===============================================================================================*/
BYTE TxCAN3Buff(ROUGH_CCU_DATA* pCCU, canmsg * pRcvMsg, BYTE u8Len);
/*=================================================================================================
** @Name      : BYTE u8PackSendMsg(BYTE u8DevNo,BYTE u8Cmd, BYTE u8GSNo,UINT Dh,UINT Dl)
** @Input     : u8DevNo:设备号；u8Cmd:命令号；u8GSNo:模块地址；Dh:发送报文高32位；Dl:发送报文低32位
** @Output    : FALSE:发送失败 TRUE：发送成功
** @Function  : 发送信息写入标准的发送报文
** @The notes :
**===============================================================================================*/
BYTE u8PackSendMsg(ROUGH_CCU_DATA* pCCU, BYTE u8DevNo,BYTE u8Cmd, BYTE u8GSNo,UINT Dh,UINT Dl);
/*=================================================================================================
** @Name      : void ABU_PackSendMsg(BYTE u8DevNo, BYTE u8Cmd, UINT Dh, UINT Dl)
** @Input     : u8DevNo:设备号 u8Cmd：命令号 Dh：发送报文高32位 Dl：发送报文低32位
** @Output    : void
** @Function  : 向ABU发送报文信息
** @The notes : 
**===============================================================================================*/
void ABU_PackSendMsg(ROUGH_CCU_DATA* pCCU, BYTE u8DevNo, BYTE u8Cmd, UINT Dh, UINT Dl);
/*=================================================================================================
** @Name      : void GetGrp_VoltCurr(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读系统类信息报文
** @The notes : 内部协议 CMD = 0X01：读取系统电压和电流
**===============================================================================================*/
void GetGrp_VoltCurr(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetGrp_NumAndStatus(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读系统类信息报文
** @The notes : 内部协议 CMD = 0X02：读取协议转换器版本、系统状态表、模块数
**===============================================================================================*/
void GetGrp_NumAndStatus(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=============================================================================================
** @Name      : void GetSingle_VoltCurr(BYTE SGNo)
** @Input     : SGNo:模块地址0-1-2-3...  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : CMD = 0X03：读取模块N电压、读取模块N电流
**===========================================================================================*/
void GetSingle_VoltCurr(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetSingle_GrpAndStatus(BYTE SGNo)
** @Input     : SGNo:模块号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 内部协议 CMD = 0X04：读取模块组号、高低压模式、模块环温、模块状态表0-2  8S周期读
**===============================================================================================*/
void GetSingle_GrpAndStatus(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetSingle_VerInfo(BYTE SGNo)
** @Input     : SGNo:模块号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 内部协议 CMD = 0X07：读取DC版本、AC版本、HW版本
**===============================================================================================*/
void GetSingle_VerInfo(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetGrp_MaxCapacity(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 内部协议 CMD = 0X0A：读取最大电压、最小电压、最大电流、额定功率（N2002无）
**===============================================================================================*/
void GetGrp_MaxCapacity(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetSingle_SerialNo(BYTE SGNo)
** @Input     : SGNo:模块号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 内部协议 CMD = 0X0B：读取产品系列、条码信息	
**===============================================================================================*/
void GetSingle_SerialNo(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetGrp_ExtVolt(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 内部协议 CMD = 0X0C：读取模块电压、最大允许电流
**===============================================================================================*/
void GetGrp_ExtVolt(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void SetGrp_SerialParellMode(BYTE SGNo, BYTE u8Mode)
** @Input     : SGNo:组号  u8Mode:高低压模式 0-不需要设置 1-低压模式 2-高压模式
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 内部协议 CMD = 0X0F：写扩展	
**===============================================================================================*/
void SetGrp_SerialParellMode(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8Mode);
/*=================================================================================================
** @Name      : void SetDCDCMode_0x0F(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 内部协议 CMD = 0X0F：写扩展	  (//test 设置DCDC模块模式？)
**===============================================================================================*/
void SetDCDCMode_0x0F(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode);
/*=================================================================================================
** @Name      : void SetMPPTMode_0x0F(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 内部协议 CMD = 0X0F：写扩展	  (//test 设置光伏模块模式？)
**===============================================================================================*/
void SetMPPTMode_0x0F(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode);
/*=================================================================================================
** @Name      : void SetGrp_WalkIn(BYTE SGNo, USHORT u16RectLoadStartTime)
** @Input     : SGNo:组号 u16RectLoadStartTime:Walkin时间
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 内部协议 CMD = 0X13：设置Walkin使能和Walkin时间
**===============================================================================================*/
void SetGrp_WalkIn(ROUGH_CCU_DATA* pCCU, BYTE SGNo, USHORT u16RectLoadStartTime);
/*=================================================================================================
** @Name      : void SetGrp_OnOff_No10(BYTE SGNo)
** @Input     : SGNo:组号  u8OnOff:开关机状态 
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 内部协议 CMD = 0X1A：开关机 1为关机，0为开机 
//test 这里要变成开关机参数直接传进来
**===============================================================================================*/
void SetGrp_OnOff_No10(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8OnOff);
/*=================================================================================================
** @Name      : void SetGrp_AdjVoltCurr_No13(BYTE SGNo, UINT u32Volt, UINT u32Curr)
** @Input     : SGNo:组号  u32Volt:设置电压值 u32Curr:设置电流值 
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 内部协议 CMD =  0X1B：设置模块电压（mV）、模块电流（mA）
**===============================================================================================*/
void SetGrp_AdjVoltCurr_No13(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32Volt, UINT u32Curr);
/*=================================================================================================
** @Name      : void SettModePara_Time(BYTE SGNo, BYTE u8DevNo)
** @Input     : SGNo:组号  u8DevNo:设备号
** @Output    : void
** @Function  : 向模块发送设置信息报文 
** @The notes : 内部协议 CMD = 0X2D：设置模块时间 年月日时 分秒
**===============================================================================================*/
void SettModePara_Time(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8DevNo);
/*=================================================================================================
** @Name      : void GetGrp_ActivePower_0X23(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 外部协议 CMD = 0X23：读取系统(0X0B)总有功功率（mW）
**===============================================================================================*/
void GetGrp_ActivePower_0X23(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetGrp_ReactivePower_0X23(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取系统(0X0B)总无功功率（mVar）（mW）
**===============================================================================================*/
void GetGrp_ReactivePower_0X23(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetModeInfo_SysDCVolt(BYTE SGNo);
** @Input     : SGNo:组号
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取系统直流侧电压mV
**===============================================================================================*/
void GetModeInfo_SysDCVolt(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetModeInfo_SysDCCurr(BYTE SGNo)
** @Input     : SGNo:组号
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取系统直流侧总电流mA
**===============================================================================================*/
void GetModeInfo_SysDCCurr(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetModeInfo_ModeNum(BYTE SGNo)
** @Input     : SGNo:组号 
 ** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取模块数
**===============================================================================================*/
void GetModeInfo_ModeNum(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetModeInfo_DCVolt(BYTE SGNo, BYTE u8DevNo)
** @Input     : SGNo:组号 u8DevNo:设备号
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取模块直流侧电压1101
**===============================================================================================*/
void GetModeInfo_DCVolt(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8DevNo);
/*=================================================================================================
** @Name      : void GetModeInfo_DCCurr(BYTE SGNo, BYTE u8DevNo)
** @Input     : SGNo:组号 u8DevNo:设备号
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取模块直流侧电流1102
**===============================================================================================*/
void GetModeInfo_DCCurr(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8DevNo);
/*=================================================================================================
** @Name      : void GetModeInfo_LineABCVolt(BYTE SGNo, BYTE u8Num)
** @Input     : SGNo:组号 u8Num：A/B/C相
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 外部协议 CMD = 0X23：读取交流AB、BC、CA相电压
**===============================================================================================*/
void GetModeInfo_LineABCVolt(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8Num);
/*=================================================================================================
** @Name      : void GetModeInfo_ModeAmbTmp(BYTE SGNo, BYTE u8DevNo)
** @Input     : SGNo:组号 u8DevNo:设备号
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 外部协议 CMD = 0X23：读取模块环境温度
**===============================================================================================*/
void GetModeInfo_ModeAmbTmp(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8DevNo);
/*=================================================================================================
** @Name      : void GetSingle_ConvertStat_10(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 外部协议 CMD = 0X23：读取逆变状态表
**===============================================================================================*/
void GetSingle_ConvertStat_10(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetSingle_ConvertStat_11(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 外部协议 CMD = 0X23：读取逆变状态表
**===============================================================================================*/
void GetSingle_ConvertStat_11(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetSingle_ConvertStat_12(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 外部协议 CMD = 0X23：读取逆变状态表
**===============================================================================================*/
void GetSingle_ConvertStat_12(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetSingle_ConvertStat_13(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取逆变状态表2_13
**===============================================================================================*/
void GetSingle_ConvertStat_13(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetSingle_ConvertStat_14(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取逆变状态表3_14
**===============================================================================================*/
void GetSingle_ConvertStat_14(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetSingle_ConvertStat_15(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取逆变状态表4_15
**===============================================================================================*/
void GetSingle_ConvertStat_15(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetSingle_GrpNum(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取模块组号
**===============================================================================================*/
void GetSingle_GrpNum(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetSingle_Barcode(BYTE SGNo, UINT u32DataH)
** @Input     : SGNo:组号  u32DataH:
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取（mA）条码信息	0x1124-0x1129
**===============================================================================================*/
void GetSingle_Barcode(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32DataH);
/*=================================================================================================
** @Name      : void GetGrp_MaxCapacity_0x23(int SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X23：直流额定功率（mW） 
**===============================================================================================*/
void GetGrp_MaxCapacity_0x23(ROUGH_CCU_DATA* pCCU, int SGNo);
void GetGrp_MaxCurr_0x23(ROUGH_CCU_DATA* pCCU, int SGNo);
/*=================================================================================================
** @Name      : void PCS_ReadPhaseVolt_0x23(BYTE ch, BYTE cmd)
** @Input     : ch:模块地址  cmd:从命令号
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 外部协议 CMD = 0X23：读取（mV）交流A相相电压、交流B相相电压、交流C相相电压
**===============================================================================================*/
void PCS_ReadPhaseVolt_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch, BYTE cmd);
/*=================================================================================================
** @Name      : void PCS_ReadPhaseCurr_0x23(BYTE ch, BYTE cmd)
** @Input     : ch:模块地址  cmd:从命令号
 ** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取（mA）交流A相电流、交流B相电流、交流C相电流
**===============================================================================================*/
void PCS_ReadPhaseCurr_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch, BYTE cmd);
/*=================================================================================================
** @Name      : void GetGrp_ACFreq_0x23(int SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读取信息报文
** @The notes : 外部协议 CMD = 0X23：读取交流频率（mHZ）
**===============================================================================================*/
void GetGrp_ACFreq_0x23(ROUGH_CCU_DATA* pCCU, int SGNo);
/*=================================================================================================
** @Name      : void PCS_ReadReactivePower_0x23(BYTE ch, BYTE cmd)
** @Input     : ch:模块地址  cmd:从命令号
 ** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取（mVA）总无功功率、A相无功功率、B相无功功率、C相无功功率
**===============================================================================================*/
void PCS_ReadReactivePower_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch, BYTE cmd);
/*=================================================================================================
** @Name      : void PCS_ReadActivePower_0x23(BYTE ch, BYTE cmd)
** @Input     : ch:模块地址  cmd:从命令号
 ** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取（mW）总有功功率、A相有功功率、B相有功功率、C相有功功率
**===============================================================================================*/
void PCS_ReadActivePower_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch, BYTE cmd);
/*=================================================================================================
** @Name      : void PCS_ReadApparentPower_0x23(BYTE ch, BYTE cmd)
** @Input     : ch:模块地址  cmd:从命令号
 ** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取（mVA）总视在功率、A相视在功率、B相视在功率、C相视在功率
**===============================================================================================*/
void PCS_ReadApparentPower_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch, BYTE cmd);
/*=================================================================================================
** @Name      : void PCS_ReadPowerFactor_0x23(BYTE ch, BYTE cmd)
** @Input     : ch:模块地址  cmd:从命令号
 ** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取（0.001）总功率因素、A相功率因素、B相功率因素、C相功率因素
**===============================================================================================*/
void PCS_ReadPowerFactor_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch, BYTE cmd);
/*=================================================================================================
** @Name      : void GetSys_BusVolt_0X23(BYTE SGNo)
** @Input     : SGNo:组号  u32CmdDataH
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : (DCDC模块调用) 系统母线侧电压（mV）
**===============================================================================================*/
void GetSys_BusVolt_0X23(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetGrp_BusVolt_0X23(BYTE SGNo)
** @Input     : SGNo:组号  u32CmdDataH
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 20221206 LKF 这个目前是DCDC模块调用 
**===============================================================================================*/
void GetGrp_BusVolt_0X23(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void GetGrp_BusCurr_0X23(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取模块直流高压侧电压（mV）/读取模块直流侧电压（mV）（用于DC模块）
**===============================================================================================*/
void GetGrp_BusCurr_0X23(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void SetGrp_AdjVolt_0x24_BEG(BYTE SGNo, UINT Volt)
** @Input     : SGNo:组号  Volt:电压值
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置系统(0X0B)直流电压
**===============================================================================================*/
void SetGrp_AdjVolt_0x24_BEG(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT Volt);
/*=================================================================================================
** @Name      : void SetGrp_AdjCurr_0x24_BEG(BYTE SGNo, UINT Curr)
** @Input     : SGNo:组号  Curr:电流值
 ** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置系统(0X0B)直流总电流
**===============================================================================================*/
void SetGrp_AdjCurr_0x24_BEG(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT Curr);
/*=================================================================================================
** @Name      : void SetSysPower_0x24(BYTE SGNo, UINT u32Power)
** @Input     : SGNo:组号 u32Power:功率值
 ** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置系统(0X0B)总有功功率（mW）
**===============================================================================================*/
void SetSysPower_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32Power);
/*=================================================================================================
** @Name      : void SetBattSideOutVolt_0x24(BYTE SGNo, UINT u32Volt)
** @Input     : SGNo:组号 u32Volt:电压值
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置电池侧输出电压（mV）
**===============================================================================================*/
void SetBattSideOutVolt_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32Volt);
/*=================================================================================================
** @Name      : void SetBattSideOutCurr_0x24(BYTE SGNo,UINT u32Curr)
** @Input     : SGNo:组号 u32Curr:电流值
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置电池侧输出电压（mV）
**===============================================================================================*/
void SetBattSideOutCurr_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo,UINT u32Curr);
/*=================================================================================================
** @Name      : void SetOnOff_0x24(BYTE SGNo, BYTE u8OnOff)
** @Input     : SGNo:组号  u8OnOff:开关机状态 1为关机，0为开机
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置模块开关机 0xA1 为关机 0xA0 为开机
**===============================================================================================*/
void SetOnOff_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8OnOff);
/*=================================================================================================
** @Name      : void SetBattSideCurrLimit_0x24(BYTE SGNo, UINT u32Curr)
** @Input     : SGNo:组号  u32Curr:电流值
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置电池侧限流值
**===============================================================================================*/
void SetBattSideCurrLimit_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32Curr);
/*=================================================================================================
** @Name      : void SetStartupMode_0x24(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置起机模式  A0-直流起机模式 A1-交流起机模式
**===============================================================================================*/
void SetStartupMode_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode);
/*=================================================================================================
** @Name      : void SetBattSideCutOffVolt_0x24(BYTE SGNo, UINT u32CutOffVolt)
** @Input     : SGNo:组号  u8GSNo:设备号 u32CutOffVolt:截止电压
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置电池侧放电截止电压
**===============================================================================================*/
void SetBattSideCutOffVolt_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32CutOffVolt);
/*=================================================================================================
** @Name      : void SetACOutPhaseVolt_0x24(BYTE SGNo, UINT u32Volt)
** @Input     : SGNo:组号 u32Volt:交流输出相电压mV
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置交流输出相电压mV
**===============================================================================================*/
void SetACOutPhaseVolt_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32Volt);
/*=================================================================================================
** @Name      : void SetModeACFreq_0x24(BYTE SGNo, UINT u32Freq)
** @Input     : SGNo:组号 u32Freq:交流输出频率mHZ
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置交流输出频率mHZ
**===============================================================================================*/
void SetModeACFreq_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32Freq);
/*=================================================================================================
** @Name      : void SetOnOffGridWrokMode_0x24(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置工作模式  A0-整流(默认) A1-并网逆变 A2-离网
**===============================================================================================*/
void SetOnOffGridWrokMode_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode);
/*=================================================================================================
** @Name      : void SetEnergyStorMode_0x24(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文 
** @The notes : 外部协议 CMD = 0X24：设置储能模式使能 A0-不使能  A1-使能
**===============================================================================================*/
void SetEnergyStorMode_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode);
/*=================================================================================================
** @Name      : void SetOnOffGridSysType_0x24(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置并离网系统  A0-并离网系统；A1-纯并网系统
**===============================================================================================*/
void SetOnOffGridSysType_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode);
/*=================================================================================================
** @Name      : void SetEnergyDispatchMode_0x24(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置能量调度模式   A1-交流调度，A0-直流调度  
**===============================================================================================*/
void SetEnergyDispatchMode_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode);
/*=================================================================================================
** @Name      : void SetChargMode_0x24(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置充电模式模式   A1-恒功率模式，A0-恒流模式  
**===============================================================================================*/
void SetChargMode_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode);
/*=================================================================================================
** @Name      : void SetActivePower_0x24(BYTE SGNo, BYTE u8GSNo, UINT u32Power)
** @Input     : SGNo:组号  u8GSNo:设备号  u32Power:有功功率设置值
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置交流有功功率（mW）  按照单模块的有功功率设置值下发
**===============================================================================================*/
void SetActivePower_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, UINT u32Power);
/*=================================================================================================
** @Name      : void SetRePowerEnable_0x24(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置无功使能   A1-无功使能，A0-禁止 
**===============================================================================================*/
void SetRePowerEnable_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode);
/*=================================================================================================
** @Name      : void SetRePowerType_0x24(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置无功功率设置类型   
                0xA0：不设置无功功率输出功能 （默认）
                0xA1：通过PF设置命令 ,对应的控制命令小类为0x21 05
                0xA2：通过无功功率设置命令，对应的控制命令小类为0x21 08
                0xA3：N2003 离网老化占用，增加AC过载阈值以便满足老化要求，不对客户开放
                0xA4：通过cos(p)自动调节无功模式 -4105要求
                0xA5：通过Q(U)自动调节无功模式 -4105要求                                         
                0xA6：active power-Reactive power模式-IEEE1547要求 
**===============================================================================================*/
void SetRePowerType_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode);
/*=================================================================================================
** @Name      : void SetRePower_0x24(BYTE SGNo, BYTE u8GSNo)
** @Input     : SGNo:组号  u8GSNo:设备号 u32RePower:无功功率值
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置无功功率（正负代表感性容性）（mW）		
**===============================================================================================*/
void SetRePower_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, UINT u32RePower);
/*=================================================================================================
** @Name      : void SetGrp_AdjVolt_0x24(BYTE SGNo)
** @Input     : SGNo:组号
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置DCDC模块直流高压侧（母线侧）电压(mV)
**===============================================================================================*/
void SetGrp_AdjVolt_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT volt);
/*=================================================================================================
** @Name      : void SetGrp_AdjCurr_0x24(BYTE SGNo, UINT u32Curr)
** @Input     : SGNo:组号  u32Curr:电流值 
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置DCDC模块直流高压侧（母线侧）电流(mA)
**===============================================================================================*/
void SetGrp_AdjCurr_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32Curr);
/*=================================================================================================
** @Name      : void ManualClearPCSFaltCmd(void)
** @Input     : u8GrpNo：组号
** @Output    : void
** @Function  : 向模块发送设置信息报文 外部协议 CMD = 0X24
** @The notes : 故障清除：0xA0-不清除(默认) 0xA1-故障清除(锁死故障清除，不下电重新开机)
**===============================================================================================*/
void ManualClearPCSFaltCmd(ROUGH_CCU_DATA* pCCU, BYTE u8GrpNo);
/*=================================================================================================
** @Name      : void SendAddressingCmd(void)
** @Input     : void
** @Output    : void
** @Function  : 模块之间互相通信的通讯协议 
** @The notes : 发送并机信息
**===============================================================================================*/
void SendAddressingCmd(ROUGH_CCU_DATA* pCCU);
/*=================================================================================================
** @Name      : void SendRectSillenceModeBroadcast(BYTE byRectAddress, BYTE u8Data)
** @Input     : void
** @Output    : void
** @Function  : 屏蔽模块DC CAN数据  0xA0-模块间数据正常 0xA1-屏蔽模块均流数据 0xA2-屏蔽模块间数据
** @The notes : 
**===============================================================================================*/
void SendRectSillenceModeBroadcast(ROUGH_CCU_DATA* pCCU, BYTE byRectAddress, BYTE u8Data);
/*=================================================================================================
** @Name      : void ReadModuleRAMCmd(BYTE SGNo)
** @Input     : SGNo：组号
** @Output    : void
** @Function  : 读取模块RAM信息
** @The notes : 
**===============================================================================================*/
void ReadModuleRAMCmd(ROUGH_CCU_DATA* pCCU, BYTE SGNo);
/*=================================================================================================
** @Name      : void ABU_CheckABUIsConfig(void)
** @Input     : void
** @Output    : void
** @Function  : 检查ABU是否使能
** @The notes : 
**===============================================================================================*/
void ABU_CheckABUIsConfig(ROUGH_CCU_DATA* pCCU);
/*=================================================================================================
** @Name      : void ABU_ReadSoftWareVersion(BYTE ch)
** @Input     : ch：
** @Output    : void
** @Function  : 读取ABU软件版本号
** @The notes : 
**===============================================================================================*/
void ABU_ReadSoftWareVersion(ROUGH_CCU_DATA* pCCU, BYTE ch);
/*=================================================================================================
** @Name      : void ABU_ReadOutputVolt_0x23(BYTE ch)
** @Input     : ch：通道
** @Output    : void
** @Function  : 输出A、B、C相电压
** @The notes : 协议里面是预留？？？？
**===============================================================================================*/
void ABU_ReadOutputVolt_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch);
/*=================================================================================================
** @Name      : void ABU_ReadACLineVoltage_0x23(BYTE ch)
** @Input     : ch：通道
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取模块交流线电压
**===============================================================================================*/
void ABU_ReadACLineVoltage_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch);
/*=================================================================================================
** @Name      : void ABU_ReadPowerTemperature_0x23(void)
** @Input     : void
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取模块环境温度
**===============================================================================================*/
void ABU_ReadPowerTemperature_0x23(ROUGH_CCU_DATA* pCCU);
/*=================================================================================================
** @Name      : void ABU_ReadACVoltage_0x23(BYTE ch)
** @Input     : ch：通道
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取模块交流A、B、C相相电压
**===============================================================================================*/
void ABU_ReadACVoltage_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch);
/*=================================================================================================
** @Name      : void ABU_ReadACCurrent_0x23(BYTE ch)
** @Input     : ch：通道
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取模块交流A、B、C相电流
**===============================================================================================*/
void ABU_ReadACCurrent_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch);
/*=================================================================================================
** @Name      : void ABU_ReadACFrequency_0x23(void)
** @Input     : void
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取交流频率
**===============================================================================================*/
void ABU_ReadACFrequency_0x23(ROUGH_CCU_DATA* pCCU);
/*=================================================================================================
** @Name      : void ABU_ReadActivePower_0x23(BYTE ch)
** @Input     : void
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取总有功功率,A、B、C相有功功率（mW）
**===============================================================================================*/
void ABU_ReadActivePower_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch);
/*=================================================================================================
** @Name      : void ABU_ReadReactivePower_0x23(BYTE ch)
** @Input     : ch：通道
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取总无功功率,A、B、C相无功功率（mW）
**===============================================================================================*/
void ABU_ReadReactivePower_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch);
/*=================================================================================================
** @Name      : void ABU_ApparentPower_0x23(BYTE ch)
** @Input     : ch：通道
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取总视在功率,A、B、C相视在功率（mW）
**===============================================================================================*/
void ABU_ApparentPower_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch);
/*=================================================================================================
** @Name      : void ABU_ReadPowerFactor_0x23(BYTE ch)
** @Input     : ch：通道
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取总功率因素,A、B、C相功率因素（0.001）
**===============================================================================================*/
void ABU_ReadPowerFactor_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch);
/*=================================================================================================
** @Name      : void ABU_Read_status(BYTE Byte7, BYTE Byte6)
** @Input     : Byte7：报送报文高八位  Byte6：发送报文低八位
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：（这条内部外部协议里面都没有）
**===============================================================================================*/
void ABU_Read_status(ROUGH_CCU_DATA* pCCU, BYTE Byte7, BYTE Byte6);
/*=================================================================================================
** @Name      : void ABU_Read_alm(BYTE Byte7, BYTE Byte6)
** @Input     : Byte7：报送报文高八位  Byte6：发送报文低八位
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：（这条内部外部协议里面都没有）
**===============================================================================================*/
void ABU_Read_alm(ROUGH_CCU_DATA* pCCU, BYTE Byte7, BYTE Byte6);
/*=================================================================================================
** @Name      : void ABU_Set_OnOff(BYTE Cmd)
** @Input     : Cmd：设置值
** @Output    : void
** @Function  : 向ABU发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置ABU开关机
**===============================================================================================*/
void ABU_Set_OnOff(ROUGH_CCU_DATA* pCCU, BYTE Cmd);
/*=================================================================================================
** @Name      : void ABU_Set_OnOffGridModeSwitchAuto(BYTE Cmd)
** @Input     : Cmd：设置值
** @Output    : void
** @Function  : 向ABU发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置ABU并离网自动切换模式（模块协议里无）
**===============================================================================================*/
void ABU_Set_OnOffGridModeSwitchAuto(ROUGH_CCU_DATA* pCCU, BYTE Cmd);
/*=================================================================================================
** @Name      : void ABU_Set_OnOffGridMode(BYTE Cmd)
** @Input     : Cmd：设置值
** @Output    : void
** @Function  : 向ABU发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置ABU工作模式，并网逆变/离网（模块协议里无）
**===============================================================================================*/
void ABU_Set_OnOffGridMode(ROUGH_CCU_DATA* pCCU, BYTE Cmd);
/*=================================================================================================
** @Name      : void ReadModuleACRAMCmd(BYTE SGNo, UINT u32StartAddress, UINT u32EndAddress)
** @Input     : SGNo：组号
** @Output    : void
** @Function  : 读取模块AC侧RAM信息，地址为0xA000-0x10000
** @The notes : 读取当前模块AC侧的RAM信息 
**===============================================================================================*/
void ReadModuleACRAMCmd(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32StartAddress, UINT u32EndAddress);
/*=================================================================================================
** @Name      : void ReadModuleDCRAMCmd(BYTE SGNo, UINT u32StartAddress, UINT u32EndAddress)
** @Input     : SGNo：组号
** @Output    : void
** @Function  : 读取模块DC侧RAM信息，地址为0xA000-0x10000
** @The notes : 读取当前模块DC侧的RAM信息
**===============================================================================================*/
void ReadModuleDCRAMCmd(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32StartAddress, UINT u32EndAddress);



/*=================================================================================================
** @Name      : void N2402_SetModuleGrpNum(BYTE modAddr, BYTE grpNo)
** @Input     : modAddr - 模块地址（拨码值），grpNo - 组号
** @Output    : void
** @Function  : 设置 N2402 模块组号
** @The notes : 协议 02 A4 xx F0 11 03 00 00 00 00 yy yy
**===============================================================================================*/
extern void N2402_SetModuleGrpNum(ROUGH_CCU_DATA* pCCU, BYTE modAddr, BYTE grpNo);
#endif