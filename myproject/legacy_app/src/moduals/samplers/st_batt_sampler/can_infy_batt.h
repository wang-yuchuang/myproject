#ifndef __CAN_INFY_BATT_H
#define __CAN_INFY_BATT_H

#include "st_batt_sampler.h"

#define CELL_BALANCE_SUB_IDX_START 	0x0A
#define CELL_VOLT_SUB_IDX_START 	0x32
#define CELL_TEMP_SUB_IDX_START 	0x96
#define CELL_AFE_SUB_IDX_START 		0x14
#define CELL_SAFE_SUB_IDX_START 	0x14

#ifdef IMMU3
	#define CELL_BALANCE_SUB_IDX_END 	0x13
	#define CELL_VOLT_SUB_IDX_END 		0x72  
	#define CELL_TEMP_SUB_IDX_END 		0xD6 // 2524项目 增加到 52*5 = 260个电芯
	#define CELL_AFE_SUB_IDX_END 		0x54
	#define CELL_SAFE_SUB_IDX_END  		0x1D
#else
	#define CELL_BALANCE_SUB_IDX_END 	0x13
	#define CELL_VOLT_SUB_IDX_END 		0x6D
	#define CELL_TEMP_SUB_IDX_END 		0xD1 // update
	#define CELL_AFE_SUB_IDX_END 		0x4F
	#define CELL_SAFE_SUB_IDX_END  		0x1D
#endif

#define	STBATT_YFY_POWEROFF_CMD     0
#define	STBATT_YFY_POWERON_CMD     	2

typedef enum tagYFY_C2SCmd
{ 
	CAN_S_CMD_C2S_CMD_HV = 0,
	CAN_S_CMD_C2S_RTC_TIME,

	CAN_RD_CMD_C2S_REC_NUM,
	
	CAN_RD_CMD_C2S_FAULT_REC,
	CAN_CFM_CMD_C2S_FAULT_REC,

	CAN_RD_CMD_C2S_LOG_REC,
	CAN_CFM_CMD_C2S_LOG_REC,

	CAN_RD_CMD_C2S_WAVE_REC,
	CAN_CFM_CMD_C2S_WAVE_REC,

	CAN_RD_CMD_C2S_DOD_REC,
	CAN_CFM_CMD_C2S_DOD_REC,

	CAN_S_CMD_C2S_REBOOT,

	CAN_S_CMD_C2S_ISOLATION_CTRL,

 	CAN_S_CMD_C2S_PERMISS_LEV_CTRL,
 
 	CAN_S_CMD_C2S_CLEAR_FAULT,

	CAN_S_CMD_C2S_WORK_MODE,

	CAN_S_CMD_C2S_EU3AirNoise,

	CAN_S_CMD_C2S_KEEP_POWER_MODE,

	CAN_S_CMD_C2S_EMSSTATUS,

	// LJH240715 aircond
	CAN_RD_CMD_C2S_AIRCOND,
	CAN_S_CMD_C2S_AIRCOND_BATT_COOLON,
    CAN_S_CMD_C2S_AIRCOND_BATT_COOLOFF,
    CAN_S_CMD_C2S_AIRCOND_BATT_HEATON,
    CAN_S_CMD_C2S_AIRCOND_BATT_HEATOFF,
    CAN_S_CMD_C2S_AIRCOND_AMBIT_COOLON,
    CAN_S_CMD_C2S_AIRCOND_AMBIT_COOLOFF,
    CAN_S_CMD_C2S_AIRCOND_AMBIT_HEATON,
    CAN_S_CMD_C2S_AIRCOND_AMBIT_HEATOFF,

	CAN_S_CMD_C2S_RINTPOINT,

	CAN_S_CMD_C2S_AFESRTEST,//XSY250417
	CAN_S_CMD_C2S_AFE_RESET,   /* C2S_CMD_AfeReset: SubId 0x1F */
	CAN_S_CMD_C2S_BSU_RESET,   /* C2S_CMD_BsuReset: SubId 0x20 */

	CAN_S_CMD_C2S_CalibratedSOC,   /*C2S_CMD_CalibratedSOC*/

	CAN_S_CMD_C2M_SELFCHECK_RES1,
	CAN_S_CMD_C2M_SELFCHECK_RES2,
	CAN_S_CMD_C2M_SELFCHECK_RES3,
	CAN_S_CMD_C2M_SELFCHECK_RES4,
	CAN_S_CMD_C2M_CHARGECHECK_RES1,
	CAN_S_CMD_C2M_CHARGECHECK_RES2,
	CAN_S_CMD_C2M_CHARGECHECK_RES3,
	CAN_S_CMD_C2M_DISCHARGECHECK_RES1,
	CAN_S_CMD_C2M_DISCHARGECHECK_RES2,
	CAN_S_CMD_C2M_DISCHARGECHECK_RES3,
	CAN_S_CMD_C2M_TOTAL_RES,
	CAN_S_CMD_C2M_HEARTBEAT

}YFY_C2S_CMD;


typedef union tagIFYBattLigHTStat
{
	BYTE	szByLigHTState[2];
	struct
	{
		BYTE	Chg_Stat 				: 1;
		BYTE	Dischg_Stat 			: 1;
		BYTE	SysFault 				: 1;
		BYTE	IdleStat 				: 1;
		BYTE	OffGridStat 			: 1;
        BYTE	MpptStat 				: 1;
	}Stat;

}IFY_BATT_LIGHT_STAT;

typedef struct tagYFYBattRunData
{
	YFY_BATT_SYS_STATE 			emBattSysState;
	
	//YFY_BATT_WORK_STATE 		emBattWorkState;
	USHORT						usBattWorkState;
	
	YFY_BATT_CHG_STATE 			emBattChgState;
	
	//YFY_BATT_POWERON_STAT 	emPowerONStatu;

	float						fAllow_MaxChargeCurr;
	float						fAllow_MaxDisChargeCurr;
	float						fAllow_MaxChargePow;
	float						fAllow_MaxDisChargePow;
	float						fAllow_MaxChargeVolt;
	float						fAllow_MaxDisChargeVolt;
	
	IFY_BATT_ALARM_CODE			unioBattAlarmCode;
	IFY_BATT_FAULT_CODE			unioBattFaultCode;
	IFY_BATT_DIDO_STAT			unioBattDIDOStat;
	
	IFY_BATT_LIGHT_STAT			unioLigHT;
	
 	//BYTE						szS2A_AlarmInfo[36];
	BYTE						szInfy_AlarmInfo[36];
	BYTE						szInfy_FaultInfo[36];
	BYTE						szInfy_GPIOInfo[36];

	float						fPackTotalVolt;
	float						fOutputCurrent;
	float						fOutputPower;
	float						fOutsideVolt;

	float						fSOC;
	float						fSOE;
	float						fSOH;
	float						fTotalCapacity;

	float						fCellAddTotalVolt;
	USHORT						usCellAverageVolt;  //mv
	SHORT						sCellAverageTemp;
	SHORT						sBattCabinetTemp;

	USHORT						usMaxCellVlot_Cell; 
	USHORT						usMaxCellVlot_Volt; //mv
	USHORT						usMinCellVlot_Cell; 
	USHORT						usMinCellVlot_Volt; //mv

	USHORT						usMaxCellTemp_Cell;
	SHORT						sMaxCellTemp_Temp;
	USHORT						usMinCellTemp_Cell;
	SHORT						sMinCellTemp_Temp;

	float						fNegativeBusVolt;

	YFY_ISOLATION_RESULT		emIsoResult;

	float						fInsulation_P_R;
	float						fInsulation_N_R;

	USHORT						usSystemRunTime;
	USHORT						usCanChargeTime;
	USHORT						usCanDischargeTime;
	USHORT						usCycleTimes;

	USHORT						usMonAlarmNum;
	USHORT						usMonFaultNum;
	USHORT						usAccuAlarmNum;
	USHORT						usAccuFaultNum;
	
	float						fRemainEnergy;
	float						fTotalEnergy;
	float						fSingleChgCapacity;       //The Last Time
	float						fSingleDischgCapacity;    //The Last Time
	
	float						fAccuChgCapacity;          //Accumulate 
	float						fAccuDischgCapacity;       //Accumulate 
	
	float						fAccuChgEnergy;          //Accumulate 
	float						fAccuDischgEnergy;       //Accumulate 
	
	float						fNowDOD;      
	float						fDischgStartDOD;
	
	USHORT						usDiagSOSLev;
	USHORT						usDiagSOSCode;
	
	BYTE						byCellBalanceStat[A_CABINET_CELL_MAX_NUM];
	BYTE						byCellSafeStat[A_CABINET_CELL_MAX_NUM];
	USHORT						usCellVolt[A_CABINET_CELL_MAX_NUM];
	float						cCellTemp[A_CABINET_CELL_MAX_NUM];
	float 						fCellRes[A_CABINET_CELL_MAX_NUM];
	USHORT 						usCellZrRes[A_CABINET_CELL_MAX_NUM];
	USHORT 						usCellZiRes[A_CABINET_CELL_MAX_NUM];
	
	BYTE						byCellAFEWorkMode[A_CABINET_CELL_MAX_NUM];//XSY250417
	USHORT						usNewCellAFESrStatus[A_CABINET_CELL_MAX_NUM];
	BOOL						bAfeWorkModeAbnormal; // 0-normal, 1-abnormal
	
	BYTE						byBMS_Alive;
	
	USHORT						usCellVoltTotalNum;
	USHORT						usCellTempTotalNum;
	float						fStandTotalVoltage;
	float						fStandCapacity;
	float						fStandEnergy;
	BYTE						bySystemVersion[9];
	BYTE						bySoftwareVersion[9];//edit by Kuangyue 20250227
	BYTE						byHardwareVersion[9];
	UINT						uiPermissionLevel;

	USHORT						nFaultRecNum;
	USHORT						nLogRecNum;
	USHORT						nWaveRecNum;
	USHORT						nDODRecNum;

	BOOL						bRead_WaveRecOK;
	
	STBATT_SYS_FAULT_MAP		mapSysFault;
	BOOL						bIsCommFail;
	BOOL						bFoundReply;
	BOOL						bBattInfo_Regisited;
		
	BYTE						byPCS_Cmd;
	BYTE						byBattAddr;
	
	int							nInterrupt_Times;

 	double						t_LastSendC2S_CMD_HV;

	double						t_LastSendC2S_RTC;

	double						t_LastSendC2S_RecNum;

	double						t_LastSendC2S_AfeSrTest;//XSY250417
	
	double						t_LastSendC2M_Res;

	//---For Remote Upgrade---//
	
	int 						nRemtDownIndex;
	BMS_REPORT_STATE			emRemtReptStat;
		
 	int							nRemtRunModel;
	int							nRemtRunState;
	int							nRemtVersion;
	BYTE   	 					byData[CAN_DATA_AREA_LEN]; 

	//add by lpl 20240807 
	AIR_STATUS_ALARM_FAULT      stAirStatusAlarm ;
	
	LIQUID_COOLING_PACK_STATUS  stLiquidCoolingPack;
	TONGFEI_AIR_STATUS_ALARM_FAULT  stTongFeiAirStatusAlarm;

	//edit by kuangyue 20250217
	IMEU3_AIR_STATUS_ALARM_FAULT	  stIMEU3AirStatusFault;
	IMEU3_AIR_STATUS_CONTROL_INFO 	  stIMEU3AirControlStatus;//edit by kuangyue 20250811
	XINMIAO_AIR_STATUS_ALARM_FAULT    stXinMiaoAirStatusFault;

	// LJH240715 aircond
	BOOL						bNeedGetTemp4Aircond;
    double						t_GetTemp4Aircond;

	float						fLeakageCurrent;// mA
	float						nSoftVer;

	//edit by Kuangyue 20241202  S2C_LONGALARM  0x1035ffxx 
    UINT 						nAlarmInfo1;     //告警信息1
    UINT 						nAlarmInfo2;     //告警信息2
    UINT 						nAlarmInfo3;     //告警信息3

    UINT 						nAlarmInfo4;     //告警信息4
    UINT 						nAlarmInfo5;     //告警信息5
    UINT 						nAlarmInfo6;     //告警信息6

    UINT 						nAlarmInfo7;     //告警信息7

}YFY_BATT_RUN_DATA;

typedef enum tagstYFYBattCmd_Idx
{ 
	//S2C: S,Server-BMS  C:Client-Monitor
	_CAN_R_CMD_S2C_STATUS = 0,
	_CAN_R_CMD_S2C_LIMIT1, 
	_CAN_R_CMD_S2C_LIMIT2, 
	_CAN_R_CMD_S2C_ALARM, 
	_CAN_R_CMD_S2C_FAULT, 
	_CAN_R_CMD_S2C_GPIO,
	_CAN_R_CMD_S2C_SUMDATA1,
	_CAN_R_CMD_S2C_SUMDATA2,
	_CAN_R_CMD_S2C_SUMDATA3,
	_CAN_R_CMD_S2C_SUMDATA4,
	_CAN_R_CMD_S2C_SUMDATA5,
	_CAN_R_CMD_S2C_SUMDATA7,
	_CAN_R_CMD_S2C_SUMDATA8,
	_CAN_R_CMD_S2C_STATISTICS1,
	_CAN_R_CMD_S2C_STATISTICS2,
	_CAN_R_CMD_S2C_STATISTICS3,
	_CAN_R_CMD_S2C_STATISTICS4,
	_CAN_R_CMD_S2C_STATISTICS5,
	_CAN_R_CMD_S2C_STATISTICS6,
	_CAN_R_CMD_S2C_DIAGNOSIS,
	_CAN_R_CMD_S2C_CELLDATA1,
	_CAN_R_CMD_S2C_CELLDATA8,
	_CAN_R_CMD_S2C_CELLDATA2,
	_CAN_R_CMD_S2C_CELLDATA3,
	_CAN_R_CMD_S2C_CELL_BARCODE_QUERY,
	_CAN_R_CMD_S2C_PACK_BARCODE_QUERY,
	_CAN_R_CMD_S2C_BSU_BARCODE_QUERY,
	_CAN_R_CMD_S2C_Permission_BARCODE,
	_CAN_R_CMD_S2C_CELL_BARCODE_WRITE,
	_CAN_R_CMD_S2C_PACK_BARCODE_WRITE,
	_CAN_R_CMD_S2C_BSU_BARCODE_WRITE,
	_CAN_R_CMD_S2C_CELLAFEWORKMODE,
	_CAN_R_CMD_S2C_CELLDATA6,//XSY250417
	_CAN_R_CMD_S2C_DEVINFO,
	_CAN_R_CMD_S2C_CFGINFO,
	_CAN_R_CMD_S2C_ISOLATRES,
	_CAN_R_CMD_S2C_AUTH,
	
	_CAN_R_CMD_S2C_RECORDNUM,

	_CAN_R_CMD_S2C_FAULTREC,
	_CAN_R_CMD_S2C_LOGREC,
	_CAN_R_CMD_S2C_WAVEREC,
	_CAN_R_CMD_S2C_DODREC,
	_CAN_R_CMD_S2C_INNERRES_1,
	_CAN_R_CMD_S2C_INNERRES_2,
	_CAN_R_CMD_S2C_ZR_INNERRES,
	_CAN_R_CMD_S2C_ZI_INNERRES,

	_CAN_R_CMD_C2S_UPGRADESTATU,

	
	_CAN_R_CMD_S2C_AIRINFO1,  //add by lpl 240805
	_CAN_R_CMD_S2C_AIRINFO2,
	_CAN_R_CMD_S2C_AIRINFO3,

	_CAN_R_CMD_Parse_S2C_PACKINFO1,
	_CAN_R_CMD_Parse_S2C_PACKINFO2,
	_CAN_R_CMD_Parse_S2C_PACKINFO3,

	_CAN_R_CMD_Parse_S2C_TFAIRPARAINFO,
	_CAN_R_CMD_Parse_S2C_TFAIRSTAINFO,
	_CAN_R_CMD_Parse_S2C_S2C_TFAIRERRINFO,

	_CAN_R_CMD_Parse_S2C_LEAKAGEDATA,//xsy

	_CAN_R_CMD_Parse_S2C_LONGALARM,//edit by Kuangyue 20241202

	_CAN_R_CMD_Parse_S2C_IMEU3AIRCONTROL,//edit by kuangyue 20250811

	_CAN_R_CMD_Parse_S2C_IMEU3AIR,//edit by kuangyue 20250217
	
	_CAN_R_CMD_Parse_S2C_XINMIAOAIR1,

	_CAN_R_CMD_Parse_S2C_XINMIAOAIR2,
	
	_CAN_R_CMD_Parse_S2C_AFEINFO,

	// LJH240715 aircond
	_CAN_R_CMD_S2C_AIRCOND_RD,
	_CAN_R_CMD_S2C_AIRCOND_SET,

	_MAX_YFY_CAN_CMD,
}ID_CMD_YFY_BATT;

typedef struct tagA_YFY_BattFrame
{
	BYTE 			byRawData[CAN_FRAME_LEN];
	
	/*CAN Protocal Head Info,Total 29 bits,extend frame*/ 
	//BYTE			byErrCode;
 	//BYTE			byResv1;
	//BYTE			byCmdType;

	ID_CMD_YFY_BATT emMsgType;
 	BYTE  			byFuncCode;
	BYTE  			bySubIndex;
    BYTE    		byBMSGrpAddr;   //0x32~
	UINT    		nBMSGrpMapIdx;  //0x0~
	BYTE 			byEMSAddr;

	UINT 			uiFrmID; 	//Send CAN ID
	UINT 			uiDataLen; 	//Send DLC
	BYTE   	 		byData[CAN_DATA_AREA_LEN+1];      /* Data Area */

	BOOL    		bIsCtrlCmd;     /* if true, need send urgently */
	// LJH240715 aircond to push data
	STBATT_SAMPLER_ROUGH *pSTBatt_Rough;
}A_CAN_YFY_BATT_FRAME;

typedef BOOL (*CAN_YFY_BATT_SAMPLE_PROC)(A_CAN_YFY_BATT_FRAME *pFrmReply);

typedef struct tagYFYBattPro
{
	ID_CMD_YFY_BATT emMsgType;
	int  			nFuncCode;
	int  			nSubIndex;

	CAN_YFY_BATT_SAMPLE_PROC pfnCmdHandler;

}TAG_YFY_BATT_PRO;

void YFY_ReadAndParseCanData(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);
BYTE getIMEU3Addr(int iDevIdx);


extern void Cal_Crc32Init(uint32_t *curCrc);
extern void Cal_ComputeCrc32(uint32_t *curCrc, const uint8_t *buf, const uint32_t size);
extern void Cal_CrcFinalize(uint32_t *curCrc);

extern uint8_t Crc_ComputeCRC8(const uint16_t Msg_Id, const uint8_t* Crc_DataPtr, uint16_t Crc_Length);
#endif



