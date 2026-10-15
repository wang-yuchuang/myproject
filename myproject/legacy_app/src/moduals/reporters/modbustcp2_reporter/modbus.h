/*==========================================================================*
 *    Copyright(c) 2019,InfyPower Co., Ltd.
 *                     ALL RIGHTS RESERVED
 * 
 *  FILENAME : 
 *  CREATOR  :                    DATE: 2019-08-03
 *  VERSION  : V1.00
 *  PURPOSE  : define data structure for reporter
 *
 *  HISTORY  :
 *==========================================================================*/

#ifndef __REPORTER_MODBUSTCP2_H
#define __REPORTER_MODBUSTCP2_H

#include "std_sys.h"
#include "app_incs.h"
#include "data_model/blob_model.h"
#include <modbus.h>  // libmodbus头文件
#include <modbus-tcp.h>  // libmodbus头文件


#ifndef DWORD_PTR
#define DWORD_PTR DWORD
#endif //DWORD_PTR

#ifndef MAKEWORD
#define MAKEWORD(a, b)      ((WORD)(((BYTE)((DWORD_PTR)(a) & 0xff)) | ((WORD)((BYTE)((DWORD_PTR)(b) & 0xff))) << 8))
#endif //MAKEWORD

#ifndef MAKEWORDX
#define MAKEWORDX(a, b)      ((WORD)(((WORD)((DWORD_PTR)(a) & 0xfff)) | ((WORD)((BYTE)((DWORD_PTR)(b) & 0xf))) << 12))
#endif //MAKEWORD

#ifndef MAKELONG
#define MAKELONG(a, b)      ((LONG)(((WORD)((DWORD_PTR)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16))
#endif //MAKELONG

#ifndef LOWORD
#define LOWORD(l)           ((WORD)((DWORD_PTR)(l) & 0xffff))
#endif //LOWORD

#ifndef HIWORD
#define HIWORD(l)           ((WORD)((DWORD_PTR)(l) >> 16))
#endif //HIWORD

#ifndef LOBYTE
#define LOBYTE(w)           ((BYTE)((DWORD_PTR)(w) & 0xff))
#endif //LOBYTE

#ifndef HIBYTE
#define HIBYTE(w)           ((BYTE)((DWORD_PTR)(w) >> 8))
#endif //HIBYTE

/* const definitions */
#define		MAX_MODBUSFRAME_LEN				256
#define 	MODBUS_LOG_TEXT_LEN				256

#define 	MODBUS_READCOM_TIMEOUT			1000
#define 	MODBUS_WRITECOM_TIMEOUT			1000
 

#define 	MAX_READ_FAIL_TIMES 			30
#define 	TM_DATA_REFRESH_INTERVAL 		5

#define		MODBUS_CMD_NUM					3
#define		MODBUS_GETDATA_CMD				0x03
#define		MODBUS_GETDATA_CMD_04			0x04

#define		MODBUS_SETSINGLEDATA_CMD		0x06
#define		MODBUS_SETMULTIDATA_CMD			0x10

#define		MODBUS_READ_CMD_NUM				1

#define		MAX_MODBUS_SIGNAL_NUM			65536

#define		MODBUS_TWOREG_FOR_ONEDATA_FLAG	1000000
#define		MODBUS_DIVISOR					65536

 
#define		MODBUS_INVALIDDATA				0xFFFF
#define		MODBUS_INVALID_EQIUPID			-9999

#define		VAR_LONG						1
#define		VAR_FLOAT						2
#define		VAR_UNSIGNED_LONG				3
#define		VAR_DATE_TIME					4
#define		VAR_ENUM						5

#define		MODBUS_REFRESHDATA_NUM			20
#define		MODBUS_REFRESHDATA_DELAY		100


#define 	MODBUS_MBAP_LEN     			 7
#define 	MODBUS_MBAP_IDENTIFIER_LEN    	 1
#define 	MODBUS_ERRORRTN_LEN    			 3

#define 	ERR_EQP_OK						0

#define     MODBUSTCP_PROTOCOL_TYPE_INFY     0
#define     MODBUSTCP_PROTOCOL_TYPE_SMTO     1

#define 	CHRG_IDTAG_LEN					16
#define     CONNET_OVERTIME_SEC             5

// Modbus Rcp Cloud Platform Manufacture
typedef enum tagModbudTcp_CP_Manu
{
    MODBUS_TCP_INFYPOWER = 0,
    MODBUS_TCP_GHSN,	// 港华数能

}MODBUS_TCP_MANUFACTURER;


typedef union 
{
    float 	fValue;
	long  	lValue;
	ULONG  	ulValue;

    WORD 	intValue[2];
    BYTE 	byValue[4];

}UNION_COMPLEXTYPE;

typedef union 
{
	SHORT	sVal;
    BYTE 	byValue[2];

}UNION_SHORT;

/* MODBUS Event definition */
struct tagModbusEvent
{
	int					iEventType;							/* event type */
	int					iDataLength;						/* data length of the event(only for FrameEvent) */
	unsigned char		sData[MAX_MODBUSFRAME_LEN];			/* data of the event(only for FrameEvent) */
	BOOL				bDummyFlag;							/* dummy flag for FrameEvent */
	BOOL				bSkipFlag;							/* skip flag for FrameEvent  */
};
typedef struct tagModbusEvent MODBUS_EVENT;
 

enum MODBUS_OPERATION_MODE 
{
	MODBUS_MODE_SERVER	= 0,
	MODBUS_MODE_CLIENT
};

enum ERR_CODE_SYSTEM_ENUM	/* the error codes for system or public			*/
{
	ERR_OK	= 0,		/* always 0!				*/
	ERR_NO_MEMORY,		/* out of memory			*/
	ERR_INVALID_ARGS,	/* error function args		*/
	ERR_TIMEOUT	,		/* error timeout			*/
	ERR_SYS_FAULT,		/* error segment fault		*/
};

 

enum tagModbusFrameType
{
	MODBUS_NOR		= 0x00,
	MODBUS_FUNCCODEERR,	
	MODBUS_REGERR,	
	MODBUS_VALUEERR,
	MODBUS_CTLFAIL	= 0x04,
	MODBUS_FRAME_ERR    = 0xF3
};
typedef enum tagModbusFrameType MODBUS_FRAME_TYPE;

typedef enum
{
	MD_GET_VAL_SHORT = 0,
	MD_GET_VAL_USHORT,
	MD_GET_VAL_FLOAT,
	MD_GET_VAL_LONG,
	MD_GET_VAL_UL,
	MD_GET_VAL_LI,
	MD_GET_VAL_EM_VAL,
	MD_GET_VAL_EM_STR,
	MD_GET_VAL_EM_LST,
	MD_GET_VAL_MIN,
	MD_GET_VAL_MAX,
	
	MD_GET_VAL_SPECIAL_1,  
	MD_GET_VAL_SPECIAL_2

}MD_GETDATA_TYPE;

typedef enum
{
	MD_SIG_SysAvaliaStu,
	MD_SIG_SysFailRsn,
	MD_SIG_OnOffGrid,
	MD_SIG_BatCfgNum,
	MD_SIG_EmsCRC,
	MD_SIG_SW_Ver,
	MD_SIG_szResv,

	MD_SIG_PgridMeter,
	MD_SIG_PcabMeter,
	MD_SIG_PauxMeter,
	MD_SIG_PoffGridMeter,
	
	MD_SIG_Pmppt,
	MD_SIG_Pbms1,
	MD_SIG_Pbms2,
	MD_SIG_Pbms3,
	MD_SIG_Pbms4,
	MD_SIG_Pbms5,
	MD_SIG_Pbms6,
	MD_SIG_Pbms7,
	MD_SIG_Pbms8,

	MD_SIG_Ppcs1,
	MD_SIG_Ppcs2,
	MD_SIG_Ppcs3,
	MD_SIG_Ppcs4,
	MD_SIG_Ppcs5,
	MD_SIG_Ppcs6,
	MD_SIG_Ppcs7,
	MD_SIG_Ppcs8,

	MD_SIG_MaxSOC,
	MD_SIG_MinSOC,
	MD_SIG_TotalSOC,

	// SEMS
	MD_SIG_SysCtlMode,
	MD_SIG_TotalActPowTgt,
	MD_SIG_SysStaus,
	MD_SIG_RunMode,
	MD_SIG_TotalSoc,
	MD_SIG_TotalActPow_fromCabMeter,
	MD_SIG_TotalActPow_fromPcs,
	MD_SIG_TotalMaxChgPowLimit,
	MD_SIG_TotalMaxDischgPowLimit,
	MD_SIG_TotalChgkWh,
	MD_SIG_TotalOutputCurr,
	// SEMS OVER

	MD_SIG_Cab_PhAVolt, 
	MD_SIG_Cab_PhBVolt, 
	MD_SIG_Cab_PhCVolt, 
	MD_SIG_Cab_PhACurt, 
	MD_SIG_Cab_PhBCurt, 
	MD_SIG_Cab_PhCCurt, 
	MD_SIG_Cab_Pactive, 
	MD_SIG_Cab_Qreactive,
	MD_SIG_Cab_Freq , 	
	MD_SIG_Cab_Factor, 
	MD_SIG_Cab_Pos_Kwh, 
	MD_SIG_Cab_Neg_Kwh, 
	MD_SIG_Cab_Current_Pos_Sum_Kwh,
	MD_SIG_Cab_Current_Pos_Sharp_Kwh,
	MD_SIG_Cab_Current_Pos_Peak_Kwh,
	MD_SIG_Cab_Current_Pos_Flat_Kwh,
	MD_SIG_Cab_Current_Pos_Wave_Kwh,
	MD_SIG_Cab_Current_Neg_Sum_Kwh,
	MD_SIG_Cab_Current_Neg_Sharp_Kwh,
	MD_SIG_Cab_Current_Neg_Peak_Kwh,
	MD_SIG_Cab_Current_Neg_Flat_Kwh,
	MD_SIG_Cab_Current_Neg_Wave_Kwh,
	MD_SIG_Cab_Last1_Pos_Sum_Kwh,
	MD_SIG_Cab_Last1_Pos_Sharp_Kwh,
	MD_SIG_Cab_Last1_Pos_Peak_Kwh,
	MD_SIG_Cab_Last1_Pos_Flat_Kwh,
	MD_SIG_Cab_Last1_Pos_Wave_Kwh,
	MD_SIG_Cab_Last1_Neg_Sum_Kwh,
	MD_SIG_Cab_Last1_Neg_Sharp_Kwh,
	MD_SIG_Cab_Last1_Neg_Peak_Kwh,
	MD_SIG_Cab_Last1_Neg_Flat_Kwh,
	MD_SIG_Cab_Last1_Neg_Wave_Kwh,
	MD_SIG_Cab_Last2_Pos_Sum_Kwh,
	MD_SIG_Cab_Last2_Pos_Sharp_Kwh,
	MD_SIG_Cab_Last2_Pos_Peak_Kwh,
	MD_SIG_Cab_Last2_Pos_Flat_Kwh,
	MD_SIG_Cab_Last2_Pos_Wave_Kwh,
	MD_SIG_Cab_Last2_Neg_Sum_Kwh,
	MD_SIG_Cab_Last2_Neg_Sharp_Kwh,
	MD_SIG_Cab_Last2_Neg_Peak_Kwh,
	MD_SIG_Cab_Last2_Neg_Flat_Kwh,
	MD_SIG_Cab_Last2_Neg_Wave_Kwh,

	MD_SIG_Grid_PhAVolt, 
	MD_SIG_Grid_PhBVolt, 
	MD_SIG_Grid_PhCVolt, 
	MD_SIG_Grid_PhACurt, 
	MD_SIG_Grid_PhBCurt, 
	MD_SIG_Grid_PhCCurt, 
	MD_SIG_Grid_Pactive, 
	MD_SIG_Grid_Qreactive,
	MD_SIG_Grid_Freq , 	
	MD_SIG_Grid_Factor, 
	MD_SIG_Grid_Pos_Kwh, 
	MD_SIG_Grid_Neg_Kwh,
	MD_SIG_Grid_Current_Pos_Sum_Kwh,
	MD_SIG_Grid_Current_Pos_Sharp_Kwh,
	MD_SIG_Grid_Current_Pos_Peak_Kwh,
	MD_SIG_Grid_Current_Pos_Flat_Kwh,
	MD_SIG_Grid_Current_Pos_Wave_Kwh,
	MD_SIG_Grid_Current_Neg_Sum_Kwh,
	MD_SIG_Grid_Current_Neg_Sharp_Kwh,
	MD_SIG_Grid_Current_Neg_Peak_Kwh,
	MD_SIG_Grid_Current_Neg_Flat_Kwh,
	MD_SIG_Grid_Current_Neg_Wave_Kwh,
	MD_SIG_Grid_Last1_Pos_Sum_Kwh,
	MD_SIG_Grid_Last1_Pos_Sharp_Kwh,
	MD_SIG_Grid_Last1_Pos_Peak_Kwh,
	MD_SIG_Grid_Last1_Pos_Flat_Kwh,
	MD_SIG_Grid_Last1_Pos_Wave_Kwh,
	MD_SIG_Grid_Last1_Neg_Sum_Kwh,
	MD_SIG_Grid_Last1_Neg_Sharp_Kwh,
	MD_SIG_Grid_Last1_Neg_Peak_Kwh,
	MD_SIG_Grid_Last1_Neg_Flat_Kwh,
	MD_SIG_Grid_Last1_Neg_Wave_Kwh,
	MD_SIG_Grid_Last2_Pos_Sum_Kwh,
	MD_SIG_Grid_Last2_Pos_Sharp_Kwh,
	MD_SIG_Grid_Last2_Pos_Peak_Kwh,
	MD_SIG_Grid_Last2_Pos_Flat_Kwh,
	MD_SIG_Grid_Last2_Pos_Wave_Kwh,
	MD_SIG_Grid_Last2_Neg_Sum_Kwh,
	MD_SIG_Grid_Last2_Neg_Sharp_Kwh,
	MD_SIG_Grid_Last2_Neg_Peak_Kwh,
	MD_SIG_Grid_Last2_Neg_Flat_Kwh,
	MD_SIG_Grid_Last2_Neg_Wave_Kwh,

	MD_SIG_BAT1_State, 		
	MD_SIG_BAT1_Volt, 			
	MD_SIG_BAT1_Curt,			
	MD_SIG_BAT1_SOC,			
	MD_SIG_BAT1_SOH,			
	MD_SIG_BAT1_MaxChgV, 		
	MD_SIG_BAT1_MaxChgA, 		
	MD_SIG_BAT1_MaxDisChgV,	
	MD_SIG_BAT1_MaxDisChgA,	
	MD_SIG_BAT1_MaxCellT, 		
	MD_SIG_BAT1_MinCellT, 		
	MD_SIG_BAT1_MaxCellV,		
	MD_SIG_BAT1_MinCellV, 		
	MD_SIG_BAT1_NotOnChgRsn,	
	MD_SIG_BAT1_NotOnDischgRsn,

	MD_SIG_BAT2_State, 		
	MD_SIG_BAT2_Volt, 			
	MD_SIG_BAT2_Curt,			
	MD_SIG_BAT2_SOC,			
	MD_SIG_BAT2_SOH,			
	MD_SIG_BAT2_MaxChgV, 		
	MD_SIG_BAT2_MaxChgA, 		
	MD_SIG_BAT2_MaxDisChgV,	
	MD_SIG_BAT2_MaxDisChgA,	
	MD_SIG_BAT2_MaxCellT, 		
	MD_SIG_BAT2_MinCellT, 		
	MD_SIG_BAT2_MaxCellV,		
	MD_SIG_BAT2_MinCellV, 		
	MD_SIG_BAT2_NotOnChgRsn,	
	MD_SIG_BAT2_NotOnDischgRsn,

	MD_SIG_BAT3_State, 		
	MD_SIG_BAT3_Volt, 			
	MD_SIG_BAT3_Curt,			
	MD_SIG_BAT3_SOC,			
	MD_SIG_BAT3_SOH,			
	MD_SIG_BAT3_MaxChgV, 		
	MD_SIG_BAT3_MaxChgA, 		
	MD_SIG_BAT3_MaxDisChgV,	
	MD_SIG_BAT3_MaxDisChgA,	
	MD_SIG_BAT3_MaxCellT, 		
	MD_SIG_BAT3_MinCellT, 		
	MD_SIG_BAT3_MaxCellV,		
	MD_SIG_BAT3_MinCellV, 		
	MD_SIG_BAT3_NotOnChgRsn,	
	MD_SIG_BAT3_NotOnDischgRsn,

	MD_SIG_BAT4_State, 		
	MD_SIG_BAT4_Volt, 			
	MD_SIG_BAT4_Curt,			
	MD_SIG_BAT4_SOC,			
	MD_SIG_BAT4_SOH,			
	MD_SIG_BAT4_MaxChgV, 		
	MD_SIG_BAT4_MaxChgA, 		
	MD_SIG_BAT4_MaxDisChgV,	
	MD_SIG_BAT4_MaxDisChgA,	
	MD_SIG_BAT4_MaxCellT, 		
	MD_SIG_BAT4_MinCellT, 		
	MD_SIG_BAT4_MaxCellV,		
	MD_SIG_BAT4_MinCellV, 		
	MD_SIG_BAT4_NotOnChgRsn,	
	MD_SIG_BAT4_NotOnDischgRsn,

	MD_SIG_PCS1_Vout, 		
	MD_SIG_PCS1_Aout, 		
	MD_SIG_PCS1_WorkMode,	
	MD_SIG_PCS1_Resv1,		
	MD_SIG_PCS1_Resv2,		
	MD_SIG_PCS2_Vout, 		
	MD_SIG_PCS2_Aout, 		
	MD_SIG_PCS2_WorkMode,	
	MD_SIG_PCS2_Resv1,		
	MD_SIG_PCS2_Resv2,		
	MD_SIG_PCS3_Vout, 		
	MD_SIG_PCS3_Aout, 		
	MD_SIG_PCS3_WorkMode,	
	MD_SIG_PCS3_Resv1,		
	MD_SIG_PCS3_Resv2,		
	MD_SIG_PCS4_Vout, 		
	MD_SIG_PCS4_Aout, 		
	MD_SIG_PCS4_WorkMode,	
	MD_SIG_PCS4_Resv1,		
	MD_SIG_PCS4_Resv2,		

	MD_SIG_Ctrl_Lock,
	MD_SIG_Ctrl_Reboot,
	MD_SIG_Ctrl_CtrlMode,
	MD_SIG_Ctrl_RunMode,
	MD_SIG_Ctrl_ActivePower,

	MD_SIG_SectorNum,
	MD_SIG_tmStart_0,
	MD_SIG_StopSOC_0,
	MD_SIG_PbatLmt_0,
	MD_SIG_PLmt_0, 
	MD_SIG_QLmt_0,  

	MD_SIG_tmStart_1,
	MD_SIG_StopSOC_1,
	MD_SIG_PbatLmt_1,
	MD_SIG_PLmt_1, 
	MD_SIG_QLmt_1,  

	MD_SIG_tmStart_2,  
	MD_SIG_StopSOC_2,  
	MD_SIG_PbatLmt_2,  
	MD_SIG_PLmt_2, 	
	MD_SIG_QLmt_2,

	MD_SIG_tmStart_3,  
	MD_SIG_StopSOC_3,  
	MD_SIG_PbatLmt_3,  
	MD_SIG_PLmt_3, 	
	MD_SIG_QLmt_3,  

	MD_SIG_tmStart_4,  
	MD_SIG_StopSOC_4,  
	MD_SIG_PbatLmt_4,  
	MD_SIG_PLmt_4, 	
	MD_SIG_QLmt_4, 

	MD_SIG_tmStart_5,  
	MD_SIG_StopSOC_5,  
	MD_SIG_PbatLmt_5,  
	MD_SIG_PLmt_5, 	
	MD_SIG_QLmt_5, 

	MD_SIG_tmStart_6,  
	MD_SIG_StopSOC_6,  
	MD_SIG_PbatLmt_6,  
	MD_SIG_PLmt_6, 	
	MD_SIG_QLmt_6, 

	MD_SIG_tmStart_7,  
	MD_SIG_StopSOC_7,  
	MD_SIG_PbatLmt_7,  
	MD_SIG_PLmt_7, 	
	MD_SIG_QLmt_7,  

	MD_SIG_tmStart_8,  
	MD_SIG_StopSOC_8,  
	MD_SIG_PbatLmt_8,  
	MD_SIG_PLmt_8, 	
	MD_SIG_QLmt_8,  

	MD_SIG_tmStart_9,  
	MD_SIG_StopSOC_9,  
	MD_SIG_PbatLmt_9,  
	MD_SIG_PLmt_9, 	
	MD_SIG_QLmt_9,  

	MD_SIG_tmStart_10, 
	MD_SIG_StopSOC_10, 
	MD_SIG_PbatLmt_10, 
	MD_SIG_PLmt_10, 	
	MD_SIG_QLmt_10,  	

	MD_SIG_tmStart_11, 
	MD_SIG_StopSOC_11, 
	MD_SIG_PbatLmt_11, 
	MD_SIG_PLmt_11, 	
	MD_SIG_QLmt_11,  

	MD_SIG_DateKwh,
	MD_SIG_Cab_PosDay,
	MD_SIG_Cab_NegDay,
	MD_SIG_Cab_PosDayShp,
	MD_SIG_Cab_PosDayPeak,
	MD_SIG_Cab_PosDayFlat,
	MD_SIG_Cab_PosDayValy,
	MD_SIG_Cab_NegDayShp,
	MD_SIG_Cab_NegDayPeak,
	MD_SIG_Cab_NegDayFlat,
	MD_SIG_Cab_NegDayValy,
	MD_SIG_BAT1_DayChg,
	MD_SIG_BAT1_DayDischg,
	MD_SIG_BAT2_DayChg,
	MD_SIG_BAT2_DayDischg,
	MD_SIG_BAT3_DayChg,
	MD_SIG_BAT3_DayDischg,
	MD_SIG_BAT4_DayChg,
	MD_SIG_BAT4_DayDischg,

	MD_SIG_BAT1_DoorOpen,
	MD_SIG_BAT1_FireTubeAlarm,
	MD_SIG_BAT1_SmokeAlarm,
	MD_SIG_BAT1_GasAlarm,
	MD_SIG_BAT1_FloodAlarm,
	MD_SIG_BAT1_AirCondHWAlarm,
	MD_SIG_BAT1_AirMachineState,
	MD_SIG_BAT1_WholeMachineStatus,
	MD_SIG_BAT1_InternalFanStatus,
	MD_SIG_BAT1_OutsideFanStatus,
	MD_SIG_BAT1_CompressorStatus,
	MD_SIG_BAT1_ElectricHeatStatus,
	MD_SIG_BAT1_ForceAutoMode,
	MD_SIG_BAT1_CabTemp,
	MD_SIG_BAT1_CabHumi,
	MD_SIG_BAT1_Sensor1Humi,
	MD_SIG_BAT1_Sensor1Temp,
	MD_SIG_BAT1_Sensor2Humi,
	MD_SIG_BAT1_Sensor2Temp,

	MD_SIG_BAT2_DoorOpen,
	MD_SIG_BAT2_FireTubeAlarm,
	MD_SIG_BAT2_SmokeAlarm,
	MD_SIG_BAT2_GasAlarm,
	MD_SIG_BAT2_FloodAlarm,
	MD_SIG_BAT2_AirCondHWAlarm,
	MD_SIG_BAT2_AirMachineState,
	MD_SIG_BAT2_WholeMachineStatus,
	MD_SIG_BAT2_InternalFanStatus,
	MD_SIG_BAT2_OutsideFanStatus,
	MD_SIG_BAT2_CompressorStatus,
	MD_SIG_BAT2_ElectricHeatStatus,
	MD_SIG_BAT2_ForceAutoMode,
	MD_SIG_BAT2_CabTemp,
	MD_SIG_BAT2_CabHumi,
	MD_SIG_BAT2_Sensor1Humi,
	MD_SIG_BAT2_Sensor1Temp,
	MD_SIG_BAT2_Sensor2Humi,
	MD_SIG_BAT2_Sensor2Temp,

	MD_SIG_BAT3_DoorOpen,
	MD_SIG_BAT3_FireTubeAlarm,
	MD_SIG_BAT3_SmokeAlarm,
	MD_SIG_BAT3_GasAlarm,
	MD_SIG_BAT3_FloodAlarm,
	MD_SIG_BAT3_AirCondHWAlarm,
	MD_SIG_BAT3_AirMachineState,
	MD_SIG_BAT3_WholeMachineStatus,
	MD_SIG_BAT3_InternalFanStatus,
	MD_SIG_BAT3_OutsideFanStatus,
	MD_SIG_BAT3_CompressorStatus,
	MD_SIG_BAT3_ElectricHeatStatus,
	MD_SIG_BAT3_ForceAutoMode,
	MD_SIG_BAT3_CabTemp,
	MD_SIG_BAT3_CabHumi,
	MD_SIG_BAT3_Sensor1Humi,
	MD_SIG_BAT3_Sensor1Temp,
	MD_SIG_BAT3_Sensor2Humi,
	MD_SIG_BAT3_Sensor2Temp,

	MD_SIG_BAT4_DoorOpen,
	MD_SIG_BAT4_FireTubeAlarm,
	MD_SIG_BAT4_SmokeAlarm,
	MD_SIG_BAT4_GasAlarm,
	MD_SIG_BAT4_FloodAlarm,
	MD_SIG_BAT4_AirCondHWAlarm,
	MD_SIG_BAT4_AirMachineState,
	MD_SIG_BAT4_WholeMachineStatus,
	MD_SIG_BAT4_InternalFanStatus,
	MD_SIG_BAT4_OutsideFanStatus,
	MD_SIG_BAT4_CompressorStatus,
	MD_SIG_BAT4_ElectricHeatStatus,
	MD_SIG_BAT4_ForceAutoMode,
	MD_SIG_BAT4_CabTemp,
	MD_SIG_BAT4_CabHumi,
	MD_SIG_BAT4_Sensor1Humi,
	MD_SIG_BAT4_Sensor1Temp,
	MD_SIG_BAT4_Sensor2Humi,
	MD_SIG_BAT4_Sensor2Temp,
	
    MD_SIG_CELLV_BASE = 10000,   // 单体电压类基值
    MD_SIG_CELLT_BASE = 12000,  // 单体温度类基值

	//Gang Hua Shu Neng Start
	MD_SIG_Src_Cod,
	MD_SIG_TotP_Cod,
	MD_SIG_TotQ_Cod,
	MD_SIG_Run_Cod,
	MD_SIG_OffGrd_Cod,
	MD_SIG_NowP,
	MD_SIG_NowQ,
	MD_SIG_SOC,
	MD_SIG_SOH,
	MD_SIG_Allow_ChgPow,
	MD_SIG_Allow_DischgPow,
	MD_SIG_Sta,
	MD_SIG_Src_Cod_Feedback,
	MD_SIG_TotP_Feedback,
	MD_SIG_TotQ_Feedback,
	MD_SIG_Run_Feedback,
	MD_SIG_OffGrd_Feedback,

	MD_SIG_Forward_ActPow_Meter1,
	MD_SIG_Backward_ActPow_Meter1,
	MD_SIG_Meter_Pow_Meter1,
	MD_SIG_Forward_SharpActPow_Meter1,
	MD_SIG_Forward_SPeakActPow_Meter1,
	MD_SIG_Forward_SFlatActPow_Meter1,
	MD_SIG_Forward_SValleyActPow_Meter1,
	MD_SIG_Reverse_SharpActPow_Meter1,
	MD_SIG_Reverse_SPeakActPow_Meter1,
	MD_SIG_Reverse_SFlatActPow_Meter1,
	MD_SIG_Reverse_SValleyActPow_Meter1,
	MD_SIG_Real_Demand_Meter1,
	MD_SIG_Max_Demand_Meter1,
	MD_SIG_Meter_CommSta_Meter1,

	MD_SIG_Forward_ActPow_Meter2,
	MD_SIG_Backward_ActPow_Meter2,
	MD_SIG_Meter_Pow_Meter2,
	MD_SIG_Forward_SharpActPow_Meter2,
	MD_SIG_Forward_SPeakActPow_Meter2,
	MD_SIG_Forward_SFlatActPow_Meter2,
	MD_SIG_Forward_SValleyActPow_Meter2,
	MD_SIG_Reverse_SharpActPow_Meter2,
	MD_SIG_Reverse_SPeakActPow_Meter2,
	MD_SIG_Reverse_SFlatActPow_Meter2,
	MD_SIG_Reverse_SValleyActPow_Meter2,
	MD_SIG_Real_Demand_Meter2,
	MD_SIG_Max_Demand_Meter2,
	MD_SIG_Meter_CommSta_Meter2,

	MD_SIG_Forward_ActPow_Meter3,
	MD_SIG_Backward_ActPow_Meter3,
	MD_SIG_Meter_Pow_Meter3,
	MD_SIG_Forward_SharpActPow_Meter3,
	MD_SIG_Forward_SPeakActPow_Meter3,
	MD_SIG_Forward_SFlatActPow_Meter3,
	MD_SIG_Forward_SValleyActPow_Meter3,
	MD_SIG_Reverse_SharpActPow_Meter3,
	MD_SIG_Reverse_SPeakActPow_Meter3,
	MD_SIG_Reverse_SFlatActPow_Meter3,
	MD_SIG_Reverse_SValleyActPow_Meter3,
	MD_SIG_Real_Demand_Meter3,
	MD_SIG_Max_Demand_Meter3,
	MD_SIG_Meter_CommSta_Meter3,
	//Gang Hua Shu Neng Over

	MAX_MD_SIG_NUMBER,

}MD_SIG_TYPE;

typedef enum tagEmRegPos
{
	_REG_START_POS = 0,

	_MAX_REG_NUM = 120
}REG_IDS;

typedef struct tagGUIDataItem
{
	MD_SIG_TYPE			emSigType;

	BOOL				bNeedSpecialPro;

	int					nDevUnitId;
	int					nSigType;
	int					nSigId;

	MD_GETDATA_TYPE		emDataType;
	//void*				pOutData;
	int					nDataSize;
	int					nMultiNum;

	int					nRegAdrStart;
	int					nRegAdrEnd;

	int					nFlag0;
	int					nFlag1;

	void				*ptrValue;

}MD_DATA_ITEM;

union tegModbusVARData
{
	float ftemp;
	char ctemp[4];
	int  itemp;
};
typedef union tegModbusVARData MODBUS_VAR_Data;

struct tagModbusDataValue
{
	int		iDataType;			/* event type */	
	MODBUS_VAR_Data	VData;
};
typedef struct tagModbusDataValue MODBUS_DataValue;
 
/* err code for CFG module */
enum ERR_CODE_CFG_ENUM
{
	ERR_CFG_OK					= ERR_OK,
	ERR_CFG_NO_MEMORY			= ERR_NO_MEMORY,
	ERR_CFG_FAIL,
	ERR_CFG_BADCONFIG,
	ERR_CFG_FILE_OPEN,
	ERR_CFG_FILE_READ,
	ERR_CFG_BADRUNINFO,

	/* input param is wrong */
	ERR_CFG_PARAM,

	ERR_CFG_RUNINFO_MUTEX_TIMEOUT,
};


struct tagModbusTypeMapInfo				  /* type mapping */
{
	int					iFunctionCode;			/* Modbus command CID1 */
	int					iLengthFix;				/* the Modbus command length is fix */
	int					iLength;				/* the Modbus command length is fix */
};
typedef struct tagModbusTypeMapInfo MODBUS_TYPE_MAP_INFO;
 

struct tagModbusMapEntriesInfo	/* sig mapping */
{
	int					iModbusPort;		/* sig serial no. or command type*/
	int					iEquipType;			/* equip type ID*/
	int					iSigType;		/* defined by SIG_TYPE enum */
	int					iSigID;			/* signal ID in equip*/
	int					iFlag0;
	int					iFlag1;
	int					iLen;
	int					iStatusValue[5];
	float				fPeakValue[5];

	int	iAccessAuthority;//register access authority, enum MODBUS_REG_ACCESS_AUTHORITY_TYPE
};
typedef struct tagModbusMapEntriesInfo MODBUS_MAPENTRIES_INFO;
 
struct tagModbusModelConfigInfo					/* Modbus model config file info */
{
	int							iTypeMapNum;		
	int							iMapEntriesNum;			/* the number of current Modbus model type */ 
	int							iUnitNum;			/* the number of units for current type */
	MODBUS_TYPE_MAP_INFO		pTypeMapInfo[MODBUS_CMD_NUM];		  /* type mapping info */
};
typedef struct tagModbusModelConfigInfo MODBUSMODEL_CONFIG_INFO;


typedef struct tagStrSysSigMap
{
	WORD	emSysAvaliaStatu;
	WORD	emSysFailReason;
	WORD 	emOnOffGrid;
	WORD	nBatCfgNum;
	WORD	emEmsCRC;
	ULONG	ulSW_Ver;
	BYTE	szResv[8];

	float	fPgridMeter;
	float	fPcabMeter;
	float	fPauxMeter;
	float	fPoffGridMeter;
	float	fPmppt;

	float	fPbms[8];
	float	fPpcs[8];

	WORD    emMaxSoc;
	WORD    emMinSoc;
	float   fTotalSoc_old; //not use

	USHORT	usEmsCtlMode;
	float	fTotalActPowTgt;
	USHORT	usSysStatus;
	USHORT	usRunMode;
	float	fTotalSoc;
	float	fTotalActPow_fromCabMeter;
	float	fSysCtlMode_fromPcs;
	float	fTotalMaxChgPowLimit;
	float	fTotalMaxDishgPowLimit;
	float	fTotalChgkWh;
	float	fTotalOutputCurr;

}STR_SYS_SIGMAP;



typedef struct strYC_Meter
{
	float	fPhaseA_Volt;
	float	fPhaseB_Volt;
	float	fPhaseC_Volt;

	float	fPhaseA_Curt;
	float	fPhaseB_Curt;
	float	fPhaseC_Curt;

	float	fActive_Power;
	float	fReactive_Power;

	float	f_Freq;
	float	f_Factor;

	float 	fPos_Kwh;
	float 	fNeg_Kwh;

	float	fCurrent_Pos_Sum_Kwh;
	float   fCurrent_Pos_Sharp_Kwh;
	float   fCurrent_Pos_Peak_Kwh;
	float   fCurrent_Pos_Flat_Kwh;
	float   fCurrent_Pos_Wave_Kwh;
	float   fCurrent_Neg_Sum_Kwh;
	float   fCurrent_Neg_Sharp_Kwh;
	float   fCurrent_Neg_Peak_Kwh;
	float   fCurrent_Neg_Flat_Kwh;
	float   fCurrent_Neg_Wave_Kwh;
	float   fLast1_Pos_Sum_Kwh;
	float   fLast1_Pos_Sharp_Kwh;
	float   fLast1_Pos_Peak_Kwh;
	float   fLast1_Pos_Flat_Kwh;
	float   fLast1_Pos_Wave_Kwh;
	float   fLast1_Neg_Sum_Kwh;
	float   fLast1_Neg_Sharp_Kwh;
	float   fLast1_Neg_Peak_Kwh;
	float   fLast1_Neg_Flat_Kwh;
	float   fLast1_Neg_Wave_Kwh;
	float   fLast2_Pos_Sum_Kwh;
	float   fLast2_Pos_Sharp_Kwh;
	float   fLast2_Pos_Peak_Kwh;
	float   fLast2_Pos_Flat_Kwh;
	float   fLast2_Pos_Wave_Kwh;
	float   fLast2_Neg_Sum_Kwh;
	float   fLast2_Neg_Sharp_Kwh;
	float   fLast2_Neg_Peak_Kwh;
	float   fLast2_Neg_Flat_Kwh;
	float   fLast2_Neg_Wave_kwh;

}STR_PARAM_METER;

//Battery Group#1 IOA Start Addr:4301H
typedef struct strYC_BMS
{
	float 	fRunState;
	float	fRack_Volt;
	float	fRack_Curt;

	float	f_SOC;
	float	f_SOH;
	
	float	fMax_AllowChgVolt;
	float	fMax_AllowChgCurt;

	float	fMax_AllowDisChgVolt;
	float	fMax_AllowDisChgCurt;

	float	fMax_CellTemp;
	float	fMin_CellTemp;

	float	fMax_CellVolt;
	float	fMin_CellVolt;

	float 	fNotStartChgReason;
	float 	fNotStartDischgReason;

}STR_PARAM_BMS;

//PCS Group #1~#4 IOA Start Addr:4501H
typedef struct strYC_PCS
{
	float	fOutputVolt_Grp1;
	float	fOutputCurt_Grp1;
	float	emWorkMode_Grp1;
	float	fResv1_Grp1;
	float	fResv2_Grp1;

	float	fOutputVolt_Grp2;
	float	fOutputCurt_Grp2;
	float	emWorkMode_Grp2;
	float	fResv1_Grp2;
	float	fResv2_Grp2;

	float	fOutputVolt_Grp3;
	float	fOutputCurt_Grp3;
	float	emWorkMode_Grp3;
	float	fResv1_Grp3;
	float	fResv2_Grp3;

	float	fOutputVolt_Grp4;
	float	fOutputCurt_Grp4;
	float	emWorkMode_Grp4;
	float	fResv1_Grp4;
	float	fResv2_Grp4;

}STR_PARAM_PCS;


typedef struct strYC_KWh
{
	float 		fDate;
	float	 	fCab_PosDailyKwh;
	float	 	fCab_NegDailyKwh;

	float	 	fCab_PosDailySharpKwh;
	float	 	fCab_PosDailyPeakKwh;
	float	 	fCab_PosDailyFlatKwh;
	float	 	fCab_PosDailyValleyKwh;

	float	 	fCab_NegDailySharpKwh;
	float	 	fCab_NegDailyPeakKwh;
	float	 	fCab_NegDailyFlatKwh;
	float	 	fCab_NegDailyValleyKwh;

	float		fBAT1_DailyChgKwh;
	float		fBAT1_DailyDischgKwh;

	float		fBAT2_DailyChgKwh;
	float		fBAT2_DailyDischgKwh;

	float		fBAT3_DailyChgKwh;
	float		fBAT3_DailyDischgKwh;

	float		fBAT4_DailyChgKwh;
	float		fBAT4_DailyDischgKwh;


}STR_PARAM_KWH;


typedef struct tagEmsSectorParam
{
	UINT		tmStart;
   // UINT		nInterval; //start + interval = end time
   //UINT       nBatt_WorkMode;
	UINT		nStopSOC;
	UINT		nBattGrpPwrLmt;
	int			nActvPwrLmt;
	UINT		nReActvPwrLmt;
}EMS_SECTOR_PARAM;

#define MAX_EMS_SECTOR	12
typedef struct strYCEmsSectorParam
{
	UINT 				nEmsSectorNum;
    EMS_SECTOR_PARAM    arryEmsParam[MAX_EMS_SECTOR];

}STR_PARAM_EMS;

typedef struct strParamCtrl
{
	WORD	emSysLock; 	//0-Unlock, 1-Lock
	WORD	emSWReboot; //1-Reboot
	WORD	emCtrlMode; //0-LocalCtrl 1-REMS
	WORD	emRunMode; //0-关机 1-整流  2-逆变
	float	fActivePower;

}STR_PARAM_CTRL;

//-----------------------GangHuaShuNeng begin--------------------
typedef struct strYC_Meter_GHSN
{
	float	fForwardActPow;
	float	fBackwardActPow;
	float	fMeterPow;
	float	fForwardSharpActPow;
	float	fForwardPeakActPow;
	float	fForwardFlatActPow;
	float	fForwardValleyActPow;
	float	fReverseSharpActPow;
	float	fReversePeakActPow;
	float	fReverseFlatActPow;
	float	fReverseValleyActPow;
	float	fRealDemand;
	float	fMaxDemand;
	float	fMeterCommSta;	// 0-normal 1-abnormal

}STR_PARAM_METER_GHSN;
typedef struct strGHSNParam
{
	// W
	float	fSrcCod;
	float	fTotPCod;
	float	fTotQCod;

	float	fRunCod;
	float	fOffGrdCod;

	// RO
	float	fNowP;
	float	fNowQ;
	float	fSoc;
	float	fSoh;
	float	fAllowChgPow;
	float	fAllowDischgPow;
	float	fSta;
	float	fSrcCodFeedback;
	float	fTotPCodFeedback;
	float	fTotQCodFeedback;
	float	fRunCodFeedback;
	float	fOffGrdCodFeedback;

	STR_PARAM_METER_GHSN	strMeter[3]; // 0:measure 2:Anti-overcapacitance 3:Anti-backflow

}STR_PARAM_GHSN;
//-----------------------GangHuaShuNeng over--------------------

typedef struct strYC_CellTemp
{
	float 	fcellVolt[A_CABINET_CELL_MAX_NUM];
	float 	fcellTemp[A_CABINET_CELL_MAX_NUM];
}STR_YC_PARAM_CELLINFO;


typedef struct strParamThermalAndFire
{
	WORD	emDoorOpen;
	WORD	emFireTubeAlarm;
	WORD	emSmokeAlarm;
	WORD	emGasAlarm;
	WORD	emFloodAlarm;
	WORD	emAirCondHWAlarm;
	WORD	emAirMachineState;
	WORD	emWholeMachineStatus;
	WORD	emInternalFanStatus;
	WORD	emOutsideFanStatus;
	WORD	emCompressorStatus;
	WORD	emElectricHeatStatus;
	WORD	emForceAutoMode;
	float	fCabTemp;
	float	fCabHumi;
	float	fSensor1Humi;
	float	fSensor1Temp;
	float	fSensor2Humi;
	float	fSensor2Temp;

}STR_PARAM_THERMAL_FIRE;

typedef struct tagModbusGlobals
{
	MODBUSMODEL_CONFIG_INFO		ModbusModelCfg;

	// ===== libmodbus相关 =====
    modbus_t *ctx;                      // libmodbus上下文
    modbus_mapping_t *mb_mapping;       // libmodbus寄存器映射
    int server_socket;                  // 服务器socket
    int client_socket;                  // 客户端socket
    pthread_mutex_t data_mutex;         // 数据互斥锁

	WORD 						szRespData1[MAX_MODBUS_SIGNAL_NUM];// derectly send buffer
	WORD 						szRespData2[MAX_MODBUS_SIGNAL_NUM];// szRespData1's buffer

	unsigned char				sReceivedBuff[MAX_MODBUSFRAME_LEN];		/* used for CHAE-based frame recognize */

	STR_SYS_SIGMAP				strSys_SigMap;
	STR_PARAM_METER				strMeter[2];//0-Cab Meter, 1-Grid T Meter
	STR_PARAM_BMS				strBAT[MAX_SINGLE_STBATT_NUM];
	STR_PARAM_PCS				strPCS;
	STR_PARAM_EMS				strLocal_Ems;
	STR_PARAM_KWH				strKWh;
	STR_PARAM_CTRL				strCtrl;

	STR_PARAM_EMS				strC2S_Ems;

	STR_PARAM_GHSN				strGHSN;// only ghsn need

	STR_PARAM_THERMAL_FIRE		strThermalFire[MAX_SINGLE_STBATT_NUM];

	MD_DATA_ITEM				*pmdDataItems;
	int							nDataItemNum;

	int							iCurPos;								// for Received Buff
	int							iLen;									// for Received Buff

	BOOL						bIsCommOpend;
	HANDLE						hThreadComm;

	HANDLE						hSamplerLock;

	char						*szSamplerSetting;

	BOOL						bAcceptConnOK;

	HANDLE 						hMDDataComm;

	int							nReadFailtTimes;

	time_t 						tmLastDataRefresh;

	A_BLOB_EMS_CONFIG_PLAN 		emsPlan;
	
	int 							nCfgBMSNum;
	A_BLOB_ES_RUN_DATA				esRunData;
	A_BLOB_ES_STATISTIC_DATA		blobStatData;
	A_BLOB_SINGLE_STBATT_RUNINFO 	blobSiglBatt;
	A_BLOB_SEMS_CLIENTINFO_SUM		clientInfoSum;
	A_BLOB_IFY_BMS_DATAINFO			blobIfyBMS;
	A_BLOB_BATTCAB_IMEU2_V20		blobImeu2;
	STR_YC_PARAM_CELLINFO			strYC_CellInfo[MAX_SINGLE_STBATT_NUM];

	time_t 							tmLastCommTime;     // add by dong.wang 2023-04-20
	USHORT 							usCommTimeoutTime;  // Modbus Communicate Timeout Time, add by dong.wang 2023-04-20
	BOOL 							bCommTimeoutEnable; // Modbus Communicate Timeout Enable, add by dong.wang 2023-04-20
 
	MODBUS_TCP_MANUFACTURER		emManufacture; // mdtcp 云平台厂家 0:英飞源 1:港华数能
	time_t 						tmLastConnctTime;     	
}MODBUS_DATA_STU;
 
struct tagModbusFrameInfo
{
    WORD wTranId;       // Transaction Identifier, recopied by the server
    WORD wProtocolId;   // Protocol Identifier, recopied by the server
    WORD wCmdDataLen;   // Cmd Data Length
    BYTE byUnitId;      // Unit Identifier, recopied by the server
    BYTE byFunCode;     // Function Code, recopied by the server
    WORD wRegStartAddr; // Register Start Addr

};
typedef struct tagModbusFrameInfo MODBUS_FRAME_DATA;  // add by dong.wang 2023-04-20
 
/* Frame Analyse sub-module */
MODBUS_FRAME_TYPE Modbus_ParseFrame(IN REPORTER *pReporter,
	IN char *pFrame, IN int iLen,
	OUT unsigned char *pCmdData,
	OUT unsigned char *pMBAPhead);

DWORD InitModbusDataMap(REPORTER *pReporter);
void Stuff_Modbus_ArrayData(REPORTER *pReporter);
void Refresh_SigMapData(REPORTER *pReporter);
void Refresh_KWhStatistic_SigMap(REPORTER *pReporter);
DWORD InitModbusDataMap_GHSN(REPORTER *pReporter);
void Refresh_SigMapData_GHSN(REPORTER *pReporter);

BOOL SetModbusSigValues(REPORTER *pReporter, MD_DATA_ITEM *pMdDataItem, int iData);
MODBUS_FRAME_TYPE Modbus_FindRegInfoForConfig(REPORTER *pReporter, int iRegAddr,int *piEquipID,MD_DATA_ITEM **ppMdDataItem);

void Modbus_PackResponseFrame( REPORTER *pReporter,
				IN  MODBUS_FRAME_TYPE RTN,
				IN  int iAddr,
				IN  int iFunctionCode,
				unsigned char *pCmdData,
				unsigned char *pMBAPhead,
				OUT unsigned char *pFrameData,
				OUT int *piFrameDataLen);

#endif
