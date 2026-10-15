#ifndef __REPORTER_ES_IEC104_H
#define __REPORTER_ES_IEC104_H
 
#include "std_sys.h"
#include "app_incs.h"

#include "../../../include/3rd_party/libiec104/cs104_slave.h"
#include "../../../include/3rd_party/libiec104/hal_thread.h"
#include "../../../include/3rd_party/libiec104/hal_time.h"
#include "../../../include/3rd_party/libiec104/buffer_frame.h"
#include "../../../include/3rd_party/libiec104/cs104_connection.h"

//#define TRACE		printf
//// trace with function and line number
//#define TRACEX		printf("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), printf


typedef enum tagIEC104ConnStatus
{
	STAT_Connect_Opened = 0,
	STAT_Connect_Closed,
	STAT_Activated,
	STAT_DeActivated,
	STAT_DisConnected,

 }IEC104_CONN_STATUS;


typedef enum tagObjValueDataType
{
	OBJVALUE_TYPE_INT,
	OBJVALUE_BOOL,
	OBJVALUE_FLOAT_DOT1, //1-dot float number

	OBJVALUE_String32,
	
}OBJVALUE_DATATYPE;

typedef struct tagIEC104sigMap
{
	//char				*ConfigName;
	UINT				nSigIdx;

	UINT				nIOA;
	
	QualityDescriptorP	emQuality;
	
	OBJVALUE_DATATYPE	emDataType;

	UINT				nScale;

	void				*ptrValue;

}IEC104_CONFIG_SIGMAP;

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

}STR_YC_PARAM_EMS;

typedef struct strYCSysInfo
{
	UINT 		emSysStatus;
	UINT		emSysFailReason;
	UINT		nBatCfgNum;
	UINT		nCycle_Meter;
	UINT		nCycle_BMS;
	UINT		nCycle_PCS;
	UINT		nCycle_Kwh;
	UINT		nEmsPlanCRC;
	UINT		emCabMeter_CommStatu;
	UINT		emGridMeter_CommStatu;
	UINT		emAuxMeter_CommStatu;
	UINT		emAux3Meter_CommStatu;

}STR_YC_PARAM_SYSINFO;


typedef struct strYCGrp1Data
{
	float	f1;
	float	f2;
	float	f3;

}STR_YC_PARAM_GRP1;

//IOA Start Addr:4201H
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

	//20230808 Added.
	float	fPmax_Demand;
	float	fPhA_Kvar;
	float	fPhB_Kvar;
	float	fPhC_Kvar;

	float	fPhA_KW;
	float	fPhB_KW;
	float	fPhC_KW;

	float	fPhA_Factor;
	float	fPhB_Factor;
	float	fPhC_Factor;

}STR_YC_PARAM_METER;

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

	float 	fAlmState; //20230808 Added.
	float	fUcell_Max_CSC;
	float	fUcell_Max_CELL;
	
	float	fUcell_Min_CSC;
	float	fUcell_Min_CELL;

	float 	fT_Max_CSC;
	float 	fT_Max_CELL;
	
	float 	fT_Min_CSC;
	float	fT_Min_CELL;
	
	float	fInsulation_P_R;
	float	fInsulation_N_R;

}STR_YC_PARAM_BMS;

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

}STR_YC_PARAM_PCS;


//Single PCS  #1~#4 IOA Start Addr:4601H
typedef struct strYC_SinglePCS
{
	float	fOnlineNum_Grp[MAX_SINGLE_STBATT_NUM];
	float	fPsum_Grp[MAX_SINGLE_STBATT_NUM];
	float	fPphA_Grp[MAX_SINGLE_STBATT_NUM];
	float	fPphB_Grp[MAX_SINGLE_STBATT_NUM];
	float	fPphC_Grp[MAX_SINGLE_STBATT_NUM];
 	float	fQsum_Grp[MAX_SINGLE_STBATT_NUM];
	float	fQphA_Grp[MAX_SINGLE_STBATT_NUM];
	float	fQphB_Grp[MAX_SINGLE_STBATT_NUM];
	float	fQphC_Grp[MAX_SINGLE_STBATT_NUM];

}STR_YC_PARAM_SINGLEPCS;

//MPPT Group #1~#4 IOA Start Addr:4701H
typedef struct strYC_MpptGrp
{
	float	emInstStatu_Grp1;//0-Not Installed, 1-Installed.
	float	fOutputVolt_Grp1;
	float	fOutputCurt_Grp1;
	float	fResv1_Grp1;
	float	fResv2_Grp1;
	float	fResv3_Grp1;

	float	emInstStatu_Grp2;//0-Not Installed, 1-Installed.
	float	fOutputVolt_Grp2;
	float	fOutputCurt_Grp2;
	float	fResv1_Grp2;
	float	fResv2_Grp2;
	float	fResv3_Grp2;

	float	emInstStatu_Grp3;//0-Not Installed, 1-Installed.
	float	fOutputVolt_Grp3;
	float	fOutputCurt_Grp3;
	float	fResv1_Grp3;
	float	fResv2_Grp3;
	float	fResv3_Grp3;

	float	emInstStatu_Grp4;//0-Not Installed, 1-Installed.
	float	fOutputVolt_Grp4;
	float	fOutputCurt_Grp4;
	float	fResv1_Grp4;
	float	fResv2_Grp4;
	float	fResv3_Grp4;

}STR_YC_PARAM_MPPTGRP;

// Station EMS Param IOA Start Addr:5001H
typedef struct strYC_LocalSystemInfo
{
	float	fMode_SysControl;		// 0-Local EMS, 1-Upper EMS
	float	TargetTotalActivePower;
	float	fStatus_SysAvailable;	// 0-Normal, 1-Locked, 2-Abnormal
	float	fStatus_Work;	// 0-Idle, 1-Rectifying, 2-Inverting, 3-Standby
	float	fTotalSOC;
	float	fActivePower_CabinetMeterMeasured;
	float	fActivePower_PCSMeasured;
	float	fMaxChargePowerAbility;
	float	fMaxDischargePowerAbility;
	float	fTotalChargekWh;
	float	fTotalOutputCurrent;

}STR_YC_LOCAL_SYSTEM;

typedef struct strYC_AirInfo
{
	float 	fLiqColConMode;
	float 	fOutletWaterTemp;
	float 	fReturnWaterTemp;
	float 	fOutletWaterPress;
	float 	fReturnWaterPress;
	
}STR_YC_PARAM_AIRINFO;

//YNT need Battery Cell Information #1 IOA Start Addr:5401H
typedef struct strYC_CellTemp
{
	float 	fcellVolt[A_CABINET_CELL_MAX_NUM];
	float 	fcellTemp[A_CABINET_CELL_MAX_NUM];
}STR_YC_PARAM_CELLINFO;

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


}STR_YC_PARAM_KWH;

//YaoXin IOA Start Addr:4501H
typedef struct strYXSigMap
{
	BOOL	bMainAC_Breaker;
	BOOL	bBAT1_Contactor;
	BOOL	bBAT2_Contactor;
	BOOL	bBAT3_Contactor;
	BOOL	bBAT4_Contactor;
	BOOL	bLiqAirWorkMode;

}STR_YX_SIGMAP;

typedef struct strYXBmsAlmPart1SigMap
{
	BOOL	bBmsNotPowerOn;
	BOOL	bBmsHVPowerOnFail;
	BOOL	bBmsHVPowerOffFail;
	BOOL	bAL_Over_Ucell;
	BOOL	bAL_Under_Ucell;
	BOOL	bAL_Over_Tcell;
	BOOL	bAL_Under_Tcell;
	BOOL	bAL_Over_U_HVS;
	BOOL	bAL_Under_U_HVS;
	BOOL	bAL_Over_T_HVS;
	BOOL	bAL_Under_T_HVS;
	BOOL	bAL_Over_CC;
	BOOL	bAL_Over_DC;
	BOOL	bAL_Diff_Tcell;
	BOOL	bAL_PreChFailThree;
	BOOL	bAL_HSD_Coil;
	BOOL	bAL_Diff_Ucell;
	BOOL	bAL_Err_Bal_Circuit;
	BOOL	bAL_Rly_OverLoad;
	BOOL	bAL_Pre_Rly;
	BOOL	bAL_Main_Rly_Pos;
	BOOL	bAL_Main_Rly_Neg;
	BOOL	bAL_Rly_Stick_Pos;
	BOOL	bAL_Rly_Stick_Neg;
	BOOL	bAL_Over_Cur;
	BOOL	bAL_Err_Mea_Ucell;
	BOOL	bAL_Err_Mea_Tcell;
	BOOL	bAL_Diff_Tvol;
	BOOL	bAL_Err_Mea_I_HVS;
	BOOL	bAL_MCANComFault;
	BOOL	bAL_Err_MCU;
	BOOL	bAL_Extre_Tvol;
	BOOL	bAL_Err_CCAN;
	BOOL	bAL_Err_SCAN;
	BOOL	bAL_Err_Mea_AllTcell;
	BOOL	bAL_Extre_T;
	BOOL	bAL_Extre_Ucell;
	BOOL	bAL_PowerSample_LEM;
	BOOL	bAL_PowerSample_CSC;
	BOOL	bAL_SBMU_PowerDown;
	BOOL	bAL_Err_Fog;
	BOOL	bAL_Err_HvCrcuitBreak;
	BOOL	bAL_Err_HvMsdFault;
	BOOL	bAL_Err_Fan1Fault;
	BOOL	bAL_Err_Fan2Fault;
	BOOL	bAL_Err_Rly_STB;
	BOOL	bCellV_HV1;
	BOOL	bCellV_LV1;
	BOOL	bSumV_HV1;
	BOOL	bSumV_LV1;
	BOOL	bChg_HT1;
	BOOL	bChg_LT1;
	BOOL	bDischg_HT1;
	BOOL	bDischg_LT1;
	BOOL	bChg_OC1;
	BOOL	bDischg_OC1;
	BOOL	bExtT_HT1;
	BOOL	bBattery_insul1;
	BOOL	bCellV_diff1;
	BOOL	bCellT_diff1;
	BOOL	bBattBox_fan_recheck;
	BOOL	bBattCab_ShuttFan_or_AirCondFail;
	BOOL	bBattCab_Aux_Dev_CommFail;
	BOOL	bBattCube_SPD;
	BOOL	bBatt_Cube_Tip;
	BOOL	bPower_IMEU3_CommFail;
	BOOL	bCab_IMEU3_CommFail;
	BOOL	bLeakCurrCommFail;
	BOOL	bAir_Cond_CommFail;
	BOOL	bDehumid_CommFail;
	BOOL	bGasSensor_CommFail;
	BOOL	bPDU_Flow_Fan_Recheck;
	BOOL	bPCS_Fan_Recheck;
	BOOL	bExhaust_Fan_Recheck;
	BOOL	bExplos_Recheck;
	BOOL	bLeakCurrAbnormal;
	BOOL	bFireAlm;
	BOOL	bFire_Lv1_Alm;
	BOOL	bFire_Lv2_Alm;
	BOOL	bFire_Lv3_Alm;
	BOOL	bPDU_High_EnvirTemp_Lv1_Alm;
	BOOL	bPDU_High_CableTemp_Lv1_Alm;
	BOOL	bPCS_High_OutletTemp_Lv1_Alm;
	BOOL	bPACK1_Water_Alm;
	BOOL	bPACK1_MSD_Alm;
	BOOL	bPACK1_Pos_HT_Alm;
	BOOL	bPACK1_Neg_HT_Alm;
	BOOL	bPACK1_Aerosol_Alm;
	BOOL	bPACK1_CO_Alm;
	BOOL	bPACK1_H2_Alm;
	BOOL	bPACK2_Water_Alm;
	BOOL	bPACK2_MSD_Alm;
	BOOL	bPACK2_Pos_HT_Alm;
	BOOL	bPACK2_Neg_HT_Alm;
	BOOL	bPACK2_Aerosol_Alm;
	BOOL	bPACK2_CO_Alm;
	BOOL	bPACK2_H2_Alm;
	BOOL	bPACK3_Water_Alm;
	BOOL	bPACK3_MSD_Alm;
	BOOL	bPACK3_Pos_HT_Alm;
	BOOL	bPACK3_Neg_HT_Alm;
	BOOL	bPACK3_Aerosol_Alm;
	BOOL	bPACK3_CO_Alm;
	BOOL	bPACK3_H2_Alm;
	BOOL	bPACK4_Water_Alm;
	BOOL	bPACK4_MSD_Alm;
	BOOL	bPACK4_Pos_HT_Alm;
	BOOL	bPACK4_Neg_HT_Alm;
	BOOL	bPACK4_Aerosol_Alm;
	BOOL	bPACK4_CO_Alm;
	BOOL	bPACK4_H2_Alm;
	BOOL	bPACK5_Water_Alm;
	BOOL	bPACK5_MSD_Alm;
	BOOL	bPACK5_Pos_HT_Alm;
	BOOL	bPACK5_Neg_HT_Alm;
	BOOL	bPACK5_Aerosol_Alm;
	BOOL	bPACK5_CO_Alm;
	BOOL	bPACK5_H2_Alm;
	BOOL	bSlv_Temp_Alm;
	BOOL	bSlv_Inner_Fault;
	BOOL	bMas_Slv_CommFail;
	BOOL	bMas_Software_Alm;
	BOOL	bMas_Inner_Fault;
	BOOL	bHV_Contactor;
	BOOL	bPeriph_backInspec;
	BOOL	bHVPowOnFail;
	BOOL	bHVPowOffFail;

}STR_YX_BMS_ALM_PART1_SIGMAP;

typedef struct strYXBmsAlmPart2SigMap
{
	BOOL	bPeriph_Stat_NotReady;
	BOOL	bGrpContactor_Change_OnLoad;
	BOOL	bSOS_Alm_Occur;
	BOOL	bUcell_Over;
	BOOL	bUcell_diff;
	BOOL	bUcell_Drop;
	BOOL	bTcell_Over;
	BOOL	bTcell_diff;
	BOOL	bTcell_Rise;
	BOOL	bEqualChip_OT;
	BOOL	bPowerChip_OT;
	BOOL	bInstant_OC;
	BOOL	bStandbyCurrAlm;
	BOOL	bTotalCapacityDecay;
	BOOL	bCellChgDischgProtFail;
	BOOL	bCellColFail;
	BOOL	bLongDODLow;
	BOOL	bChgDisChgEffLow;
	// BOOL	bDI_1_AuxPower;

}STR_YX_BMS_ALM_PART2_SIGMAP;

typedef struct strYXPcsTotalAlmSigMap
{
	BOOL	bCcu_CommFail;
	BOOL	bAC_OV;
	BOOL	bAC_UV;
	BOOL	bACGrid_Input_MCB_QF1;
	BOOL	bACLoad_Output_MCB_QF2;
	BOOL	bAC_Contactor;
	BOOL	bABU1_CommFail;
	BOOL	bABU2_CommFail;
	BOOL	bABU3_CommFail;
	BOOL	bABU4_CommFail;
	BOOL	bLV_MCBBBreaker;
	BOOL	bN_LineContactor;
	BOOL	bInsula1_CommFail;	
	BOOL	bInsula1_DeteAlm;	
	BOOL	bInsula2_CommFail;	
	BOOL	bInsula2_DeteAlm;	
	BOOL	bInsula3_CommFail;	
	BOOL	bInsula3_DeteAlm;	
	BOOL	bInsula4_CommFail;	
	BOOL	bInsula4_DeteAlm;	
	BOOL	bInsula5_CommFail;	
	BOOL	bInsula5_DeteAlm;	
	BOOL	bInsula6_CommFail;	
	BOOL	bInsula6_DeteAlm;	
	BOOL	bInsula7_CommFail;	
	BOOL	bInsula7_DeteAlm;	
	BOOL	bInsula8_CommFail;	
	BOOL	bInsula8_DeteAlm;	
	BOOL	bInsula9_CommFail;	
	BOOL	bInsula9_DeteAlm;	
	BOOL	bInsula10_CommFai;	
	BOOL	bInsula10_DeteAlm;	
	BOOL	bBattGrp1_ForbidRun;
	BOOL	bBattGrp2_ForbidRun;
	BOOL	bBattGrp3_ForbidRun;
	BOOL	bBattGrp4_ForbidRun;
	BOOL	bBattGrp1_InputContact;
	BOOL	bBattGrp2_InputContact;
	BOOL	bBattGrp3_InputContact;
	BOOL	bBattGrp4_InputContact;
	BOOL	bBattGrp1_IsoSwitch;
	BOOL	bBattGrp2_IsoSwitch;
	BOOL	bBattGrp3_IsoSwitch;
	BOOL	bBattGrp4_IsoSwitch;
	BOOL	bBms1Can_CommFail;
	BOOL	bBms2Can_CommFail;
	BOOL	bBms3Can_CommFail;
	BOOL	bBms4Can_CommFail;
	BOOL	bAllPMGrp_Fail;
	BOOL	bAllPMGrp_CommFail;
	BOOL	bPM1_Fail;
	BOOL	bPM1_CommFail;
	BOOL	bPM2_Fail;
	BOOL	bPM2_CommFail;
	BOOL	bPM3_Fail;
	BOOL	bPM3_CommFail;
	BOOL	bPM4_Fail;
	BOOL	bPM4_CommFail;
	BOOL	bPM5_Fail;
	BOOL	bPM5_CommFail;
	BOOL	bPM6_Fail;
	BOOL	bPM6_CommFail;
	BOOL	bPM7_Fail;
	BOOL	bPM7_CommFail;
	BOOL	bPM8_Fail;
	BOOL	bPM8_CommFail;
	BOOL	bPM9_Fail;
	BOOL	bPM9_CommFail;
	BOOL	bPM10_Fail;
	BOOL	bPM10_CommFail;
	BOOL	bPM11_Fail;
	BOOL	bPM11_CommFail;
	BOOL	bPM12_Fail;
	BOOL	bPM12_CommFail;
	BOOL	bPCSGrp1_Fail;
	BOOL	bPCSGrp2_Fail;
	BOOL	bPCSGrp3_Fail;
	BOOL	bPCSGrp4_Fail;

}STR_YX_PCS_TOTAL_ALM_SIGMAP;

typedef struct strYXPcsGrpAlmSigMap
{
	BOOL	bDischgErr;
	BOOL	bBusFail;
	BOOL	bSciFail;
	BOOL	bUVdc;
	BOOL	bShort;
	BOOL	bMCCFail;
	BOOL	bOvo;
	BOOL	bOT;
	BOOL	bFault;
	BOOL	bOVin;
	BOOL	bUVin;
	BOOL	bInUnBal;
	BOOL	bInLessPhase;
	BOOL	bOutUnBal;
	BOOL	bIDDup;
	BOOL	bGridFault;
	BOOL	bPllFault;
	BOOL	bAcFail;
	BOOL	bBusErr;
	BOOL	bVinOff;
	BOOL	bGrpUVdc;
	BOOL	bVGridFault;
	BOOL	bFGridFault;
	BOOL	bACLessPhase;
	BOOL	bPhaseErr;
	BOOL	bDCTZ;
	BOOL	bOIin;
	BOOL	bACTZ;
	BOOL	bOVBUS;
	BOOL	bOP;
	BOOL	bGridVolt_OverBus;
	BOOL	bHighOrLowFault;
	BOOL	bBAT_EOD;
	BOOL	bRly_Fault;
	BOOL	bSoftStart_Fault;
	BOOL	bAuxPower_Fault;
	BOOL	bEPO_Fault;
	BOOL	bDuplicate_ID;
	BOOL	bUnEven_Curr;
	BOOL	bFan_Fail;
	BOOL	bOTemp;
	BOOL	bPhase_Lock;
	BOOL	bIsland_Prot;
	BOOL	bCommFail_CanSciSpi;
	BOOL	bOutput_Derating;
	BOOL	bOutput_OL;
	BOOL	bOutput_Short;
	BOOL	bInter_OC;
	BOOL	bBus_Fault;
	BOOL	bDC_Fault;
	BOOL	bAC_Fault;
	BOOL	bSys_Fail;
	BOOL	bGrid_OV;
	BOOL	bGrid_UV;
	BOOL	bGrid_OF;
	BOOL	bGrid_UF;
	BOOL	bPhase_Loss;
	BOOL	bRevGrid_lackN;
	BOOL	bGrid_ReverPhase;
	BOOL	bGrid_PowerOff;
	BOOL	bRevGrid_Fast_OV1;
	BOOL	bRevGrid_Fast_OV2;
	BOOL	bRevGrid_Unbal;
	BOOL	bPhase_Lock_Fault;
	BOOL	bHVRT_Fault;
	BOOL	bLVRT_Fault;
	BOOL	bIslandProt;
	BOOL	bInterBus_FastUnbal;
	BOOL	bInterBus_SlowUnbal;
	BOOL	bInterBus_FastOV;
	BOOL	bInterBus_SlowOV;
	BOOL	bInterBus_FastUV;
	BOOL	bInterBus_SlowUV;
	BOOL	bRevBus_Short;
	BOOL	bAC_High_DC;
	BOOL	bDC_OV;
	BOOL	bDC_UV;
	BOOL	bRev_DC_Reversed;
	BOOL	bDC_OC;
	BOOL	bDC_ChgOV;
	BOOL	bDC_EOD_Shutdown;
	BOOL	bAC_FastOC;
	BOOL	bOutput_OLoad;
	BOOL	bPhA_PBPFault;
	BOOL	bPhB_PBPFault;
	BOOL	bPhC_PBPFault;
	BOOL	bAC_CurrUnbal;
	BOOL	bEff_Fail;
	BOOL	bBus_SoftStart_Fault;
	BOOL	bInv_SoftStart_Fault;
	BOOL	bGrid_RlyOpen;
	BOOL	bGrid_RlyShort;
	BOOL	bInv_RlyOpen;
	BOOL	bInv_RlyShort;
	BOOL	bDC_RlyFault;
	BOOL	bDC_Side_FaultSum;
	BOOL	bDC_Curr_SampFail;
	BOOL	bInductor_Curr_SampFail;
	BOOL	bAC_Curr_SampFail;
	BOOL	bDC_SoftStart_OL;
	BOOL	bDC_SoftStart_RlyFail;
	BOOL	bParallel_CommFail;
	BOOL	bAC_ShortCircuit;
	BOOL	bDSP_FaultRev;
	BOOL	bSTS_FaultRev;
	BOOL	bCAN_CommFailRev;
	BOOL	bEEPROM_faultRev;
	BOOL	bEPO_Fail;
	BOOL	bCarrier_SyncFail;
	BOOL	bAC_SyncFail;
	BOOL	bCAN_CommFail;
	BOOL	bSCI_CommFail;
	BOOL	bSPI_CommFail;
	BOOL	bBalBridge_OC;
	BOOL	bBus_OV;
	BOOL	bBus_UV;
	BOOL	bRadiator_OT;
	BOOL	bAmbient_OT;
	BOOL	bAux_PowerFault;
	BOOL	bDSP_PWM_Fault;
	BOOL	bCPLD_PWM_Fault;
	BOOL	bFan_Fault;
	BOOL	bVo_Set_OverProtPoint;
	BOOL	bVo_Setting_Low;
	BOOL	bDC_Short_Circuit;
	BOOL	bDuplicated_ID;

}STR_YX_PCS_GRP_ALM_SIGMAP;


typedef struct strYXSTSAlmSigMap
{
	BOOL	bSTS_GridOVP;
	BOOL	bSTS_GridUVP;
	BOOL	bSTS_GridOFP;
	BOOL	bSTS_GridUFP;
	BOOL	bSTS_GridFreqLose;
	BOOL	bSTS_GridLosePhase;
	BOOL	bSTS_GridPhaseSeqInv;
	BOOL	bSTS_GridLoseN;
	BOOL	bSTS_GridUnbal;
	BOOL	bSTS_VGridLineFault;
	BOOL	bSTS_PllFail;
	BOOL	bSTS_VgridCrossZeroLose;
	BOOL	bSTS_GridDQFastFault;
	BOOL	bSTS_GridFreErr;
	BOOL	bSTS_GridFreChg;
	BOOL	bSTS_OutputOL;
	BOOL	bSTS_GridOCP;
	BOOL	bSTS_IacUnbal;
	BOOL	bSTS_OVRT;
	BOOL	bSTS_LVRT;
	BOOL	bSTS_IslandFault;
	BOOL	bSTS_RefluxFault;
	BOOL	bSTS_OutputShort;
	BOOL	bSTS_VoutCrossZeroLose;
	BOOL	bGri_UaSampErr;
	BOOL	bGri_UbSampErr;
	BOOL	bGri_UcSampErr;
	BOOL	bSTS_RlyErr;
	BOOL	bSTS_OnGridStartFault;
	BOOL	bSTS_CanErr;
	BOOL	bSTS_SpiErr;
	BOOL	bSTS_E2PROMErr;
	BOOL	bSTS_OTSink;
	BOOL	bSTS_OTAmb;
	BOOL	bSTS_OTCabinet;
	BOOL	bSTS_FanERR;
	BOOL	bSTS_GridFault;
	BOOL	bSTS_SysFault_1114;
	BOOL	bSTS_SCRShort;
	BOOL	bSTS_SCROpen;
	BOOL	bSTS_U2CanErr;
	BOOL	bSTS_U2SpiErr;
	BOOL	bSTS_U2E2PROMErr;
	BOOL	bSTS_EPO;
	BOOL	bSTS_OTempTrsfm;
	BOOL	bSTS_AuxPowerErr;
	BOOL	bSTS_InvContctErr;
	BOOL	bSTS_ByContctErr;
	BOOL	bSTS_FanErr;
	BOOL	bSTS_IoReadyErr;
	BOOL	bSTS_U2SysFault;
	BOOL	bSTS_Fan1Err;
	BOOL	bSTS_Fan2Err;
	BOOL	bSTS_Fan3Err;
	BOOL	bSTS_Fan4Err;
	BOOL	bSTS_N_RlyErr;

}STR_YX_STS_ALM_SIGMAP;

typedef struct strYXFireAlmSigMap
{
	BOOL	bPowerCab_Smoke_Sensor;
	BOOL	bCombGas_Sensor1_CommFail;
	BOOL	bCombGas_Sensor2_CommFail;
	BOOL	bCombGas_Sensor3_CommFail;
	BOOL	bCombGas_Sensor4_CommFail;
	BOOL	bFireTube_Alm;
	BOOL	bSmoke_Alm;
	BOOL	bFlamGas_Alm;
	BOOL	bElecShut_FullCls;
	BOOL	bElecShut_FullOpen;
	BOOL	bEnterProt_Lv1;
	BOOL	bEnterProt_Lv2;
	BOOL	bEnterProt_Lv3;
	BOOL	bEnterProt_Lv4;
	BOOL	bSlvAerosol_DensityAlm;
	BOOL	bCombGas_concentraAlm;
	BOOL	bBatGrp_FirepProtAlm;
	BOOL	bGrp_SmokeAlm;

}STR_YX_FIRE_ALM_SIGMAP;

typedef struct  strIec104DataRough
{
	HANDLE							hReporter;
	HANDLE							hMutex;

	HANDLE							hThread;
    BOOL							bNeedExitThread;

    char 							szLastRecvData[512];

	//HANDLE						hqMsg2Sent; 
	IEC104_CONN_STATUS				em104ConnStatus;

	A_BLOB_ES_BACKEND_CFG			blobBackendSet;

	A_BLOB_EMS_CONFIG_PLAN 			emsPlan;

	A_BLOB_ES_RUN_DATA				esRunData;

	A_BLOB_ES_STATISTIC_DATA		blobStatData;

	A_BLOB_SINGLE_STBATT_RUNINFO 	blobSiglBatt;

	A_BLOB_IFY_BMS_DATAINFO			blobIfyBMS;

	UINT 							nCfgBMSNum;
	
	BOOL							bOn_Interrogating;
	BOOL							bTrigger;
	BOOL							bTriggerEmsYC;
	time_t							tmLastSent_Sys;
	time_t							tmLastSent_Meter;
	time_t							tmLastSent_BMS;
	time_t							tmLastSent_PCS;
	time_t							tmLastSent_KWh;
	time_t							tmLastSent_LocalSystem;
	time_t							tmLastSent_Cell;

	BOOL							bNeedUploadCellInfo;

	//-------------Yao Xin SigMap----0x01H---------//
	STR_YX_SIGMAP				strYX_SigMap;
	IEC104_CONFIG_SIGMAP 		*pYXEle;
	UINT						nYXNum;

	STR_YX_BMS_ALM_PART1_SIGMAP	strBmsPart1YX_SigMap[MAX_SINGLE_STBATT_NUM];
	IEC104_CONFIG_SIGMAP 		*pYX_BmsPart1[MAX_SINGLE_STBATT_NUM];
	UINT						nYX_BmsPart1Num;

	STR_YX_BMS_ALM_PART2_SIGMAP	strBmsPart2YX_SigMap[MAX_SINGLE_STBATT_NUM];
	IEC104_CONFIG_SIGMAP 		*pYX_BmsPart2[MAX_SINGLE_STBATT_NUM];
	UINT						nYX_BmsPart2Num;

	STR_YX_PCS_TOTAL_ALM_SIGMAP	strPcsTotalYX_SigMap;
	IEC104_CONFIG_SIGMAP 		*pYX_PcsTotal;
	UINT						nYX_PcsTotalNum;

	STR_YX_PCS_GRP_ALM_SIGMAP	strPcsGrpYX_SigMap[MAX_SINGLE_STBATT_NUM];
	IEC104_CONFIG_SIGMAP 		*pYX_PcsGrp[MAX_SINGLE_STBATT_NUM];
	UINT						nYX_PcsGrpNum;

	STR_YX_STS_ALM_SIGMAP		strSTSYX_SigMap;
	IEC104_CONFIG_SIGMAP 		*pYX_STS;
	UINT						nYX_STSNum;

	STR_YX_FIRE_ALM_SIGMAP		strFireYX_SigMap;
	IEC104_CONFIG_SIGMAP 		*pYX_Fire;
	UINT						nYX_FireNum;

	//-----------Yao Ce EMS Param----0x4001H-------//
	STR_YC_PARAM_EMS			strYC_Ems;
	IEC104_CONFIG_SIGMAP 		*pYCEle_Ems;
	UINT						nYCNum_Ems;

	STR_YC_PARAM_EMS			strC2S_Ems;
	IEC104_CONFIG_SIGMAP 		*pYTEle_C2SEms;
	UINT						nC2S_EmsNum;

	STR_YC_PARAM_SYSINFO		strYC_SysInfo;
	IEC104_CONFIG_SIGMAP 		*pYCEle_Sys;
	UINT						nYCNum_Sys;

	STR_YC_PARAM_METER			strYC_Meter[4];//0-Cab Meter, 1-Grid T Meter, 2-Aux Meter, 3-Aux3 ACMeter
	IEC104_CONFIG_SIGMAP 		*pYCEle_CabMeter;
	UINT						nYCNum_CabMeter;

	IEC104_CONFIG_SIGMAP 		*pYCEle_GridMeter;
	UINT						nYCNum_GridMeter;

	IEC104_CONFIG_SIGMAP 		*pYCEle_AuxMeter;
	UINT						nYCNum_AuxMeter;

	IEC104_CONFIG_SIGMAP 		*pYCEle_Aux3Meter;
	UINT						nYCNum_Aux3Meter;

	STR_YC_PARAM_BMS			strYC_BAT[MAX_SINGLE_STBATT_NUM];
	IEC104_CONFIG_SIGMAP 		*pYCEle_BatG1;
	UINT						nYCNum_BatG1;

	IEC104_CONFIG_SIGMAP 		*pYCEle_BatG2;
	UINT						nYCNum_BatG2;

	IEC104_CONFIG_SIGMAP 		*pYCEle_BatG3;
	UINT						nYCNum_BatG3;

	IEC104_CONFIG_SIGMAP 		*pYCEle_BatG4;
	UINT						nYCNum_BatG4;

	STR_YC_PARAM_PCS			strYC_PCS;
	IEC104_CONFIG_SIGMAP		*pYCEle_PCS;
	UINT						nYCNum_PCS;

	STR_YC_PARAM_SINGLEPCS		strYC_SiglPCS;
	IEC104_CONFIG_SIGMAP		*pYCEle_SiglPCS;
	UINT						nYCNum_SiglPCS;
	

	STR_YC_PARAM_MPPTGRP		strYC_MpptGrp;
	IEC104_CONFIG_SIGMAP		*pYCEle_MpptGrp;
	UINT						nYCNum_MpptGrp;
	BOOL 						bNeedUploadMppt;

	STR_YC_PARAM_AIRINFO		strYC_AirInfo;
	IEC104_CONFIG_SIGMAP 		*pYCEle_AirInfo;
	UINT						nYCNum_AirInfo;

	// YNT need cell temp and volt----A001-A1E1-A3C1-A5A1
	STR_YC_PARAM_CELLINFO		strYC_CellInfo[MAX_SINGLE_STBATT_NUM];
	IEC104_CONFIG_SIGMAP 		*pYCEle_CellG1;
	UINT						nYCNum_CellG1;

	IEC104_CONFIG_SIGMAP 		*pYCEle_CellG2;
	UINT						nYCNum_CellG2;

	IEC104_CONFIG_SIGMAP 		*pYCEle_CellG3;
	UINT						nYCNum_CellG3;

	IEC104_CONFIG_SIGMAP 		*pYCEle_CellG4;
	UINT						nYCNum_CellG4;
	// YNT over


	STR_YC_LOCAL_SYSTEM			strYC_LocalSystem;
	IEC104_CONFIG_SIGMAP		*pYCEle_LocalSystem;
	UINT						nYCNum_LocalSystem;
	//-------------Yao Tiao SigMap-------------//


	//-------------Yao Kong SigMap-------------//

	//--------------KWH Statistic--------------//
	STR_YC_PARAM_KWH			strYC_KWh;
	IEC104_CONFIG_SIGMAP		*pYCEle_KWh;
	UINT						nYCNum_KWh;
	BOOL                        enableREMSCtrl;
	time_t						tmLastDisconnect;
	UINT						nTDisconDelayIOA6191;
	UINT						nLocalEMSInfoReportingCycle;
	BOOL						bDisconTrigger;
	EMSCtrlMode                 emEMSCtrlMode_REMS; // REMS下发
	A_BLOB_SEMS_CLIENTINFO_SUM  clientInfoSum;
	LCCfg                       lcCfg; // LJH250428 cmd
}IEC104_DATA_ROUGH;
 
//-----------------------End----------------------//
//void rawMessageHandler(void* parameter, IMasterConnection conneciton, uint8_t* msg, int msgSize, bool sent);
bool clockSyncHandler (void* parameter, IMasterConnection connection, CS101_ASDU asdu, CP56Time2a newTime);
bool interrogationHandler(void* parameter, IMasterConnection connection, CS101_ASDU asdu, uint8_t qoi);
bool asduHandler(void* parameter, IMasterConnection connection, CS101_ASDU asdu);
bool connectionRequestHandler(void* parameter, const char* ipAddress);
void connectionEventHandler(void* parameter, IMasterConnection con, CS104_PeerConnectionEvent event);

BOOL Init_SysParamConfig(IEC104_DATA_ROUGH* pIEC104);
BOOL Init_IEC104_SigMap(IEC104_DATA_ROUGH* pIEC104);
void Refresh_YC_YX_SigMapValue(IEC104_DATA_ROUGH* pIEC104);
void Refresh_YC_KWh_DataMap(IEC104_DATA_ROUGH* pIEC104);
void* _Thread_IEC104_SimpleClient(void* pArg);
void Handle_ParamStat(IEC104_DATA_ROUGH* pIEC104, int nIOA, float nSetValue);

#define DUMP_ESiOCPPMSG(pReporter, nRW, szData) do{ \
	(pReporter)->pDAI->Set((HANDLE)(pReporter), DAITYPE_COMM_DATA_DUMP, \
	DUMP_SRC_ES_iOCPP, (nRW), (szData), 0, 0);\
	}while(0)

#endif

