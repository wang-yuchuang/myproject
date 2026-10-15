#ifndef _HIS_PARAM_2018_04_03_
#define _HIS_PARAM_2018_04_03_

#include "base_defines/basetypes.h"
#include "data_model/device_model.h"
#include "data_model/blob_model.h"

#define MAX_HISDATA_RECORDS					10000
// LJH240925 RunData extend:30000
//#define MAX_HIS_BMS_RUNTIME_DATALIST		15000 //store it in every [15S]
#define MAX_HIS_BMS_RUNTIME_DATALIST		50

typedef enum tagHisDataTableType
{
	TP_HISDATA_WEEKHOURS = 0,
	TP_HISDATA_DAILY,
	TP_HISDATA_YEARLY,
	_MAX_TP_HISDATA

}HISDATA_TABLETYPE;

typedef enum tagHisDataTriggle
{
	HISTRIGGLE_PERIODLY = 0,
	HISTRIGGLE_COND,
	HISTRIGGLE_TIME_OR_COND,
	HISTRIGGLE_TIME_AND_COND
}HIS_TRIGGLE;

typedef struct tagHisFloatValues
{
	float 	fValue;
	double 	tmSampled;
}HIS_FLOAT_VALUES;

typedef struct tagHisParamInfo
{
	//PARAMETER*			pParam; //must be status param to record!
	int					nDevId;
	int					nParamId;
	HIS_TRIGGLE			emTriggleType;
	float				fPeriod_mSec;
	float				fTriggle_GreaterThan; //record when >= threashhold

	//int					nPeriod_Sec; //at least in 1 sec
	//CLUSTER_EXP_ELEMENT	condExpression;
#define MAX_TEMP_STORED_HISVALS		50
	int					nHisValsTotal;
	HIS_FLOAT_VALUES	hisValus[MAX_TEMP_STORED_HISVALS];
	//float				fLastValue_Recorded;//support float value only
	double				tmLastRecord;
}HISPARAM_INFO;

typedef enum tagMetetEleType
{
	METER_ELETYPE_Not_Supported = 0, //should be 0 to identify
	METER_ELETYPE_Current_Export,
	METER_ELETYPE_Current_Import,
	METER_ELETYPE_Current_Offered,
	METER_ELETYPE_Engergy_Active_Export_Register,
	METER_ELETYPE_Engergy_Active_Import_Register,
	METER_ELETYPE_Engergy_Reactive_Export_Registe,
	METER_ELETYPE_Engergy_Reactive_Import_Register,
	METER_ELETYPE_Engergy_Active_Export_Interval,
	METER_ELETYPE_Engergy_Active_Import_Interval,
	METER_ELETYPE_Engergy_Reactive_Export_Interval,
	METER_ELETYPE_Engergy_Reactive_Import_Interval,
	METER_ELETYPE_Frequency,
	METER_ELETYPE_Power_Active_Export,
	METER_ELETYPE_Power_Active_Import,
	METER_ELETYPE_Power_Factor,
	METER_ELETYPE_Power_Offered,
	METER_ELETYPE_Power_Reactive_Export,
	METER_ELETYPE_Power_Reactive_Import,
	METER_ELETYPE_Fan_RPM,
	METER_ELETYPE_Battery_SOC,
	METER_ELETYPE_Temperature,
	METER_ELETYPE_Voltage,

	////for AC
	//ABC stored in 3 different SHORT bytes
	METER_ELETYPE_Voltage_ABC,
	METER_ELETYPE_Current_Import_ABC,
	//METER_ELETYPE_Voltage_A,
	//METER_ELETYPE_Voltage_B,
	//METER_ELETYPE_Voltage_C,
	//METER_ELETYPE_Current_Import_A,
	//METER_ELETYPE_Current_Import_B,
	//METER_ELETYPE_Current_Import_C

}METER_ELE_TYPE;

typedef union tagMeterVal
{
	BYTE		byRaw[6];
	float		fVal; //all other valuse should be fload
	SHORT	sVal_ABS[3]; //phase values
}VAL_METER;

typedef struct tagMeterVal_Element
{
	//METER_ELE_TYPE	tp;
	BYTE				tp;
	VAL_METER		val;
}METERVALUE_ELEMENT;

#define MAX_SUPPORT_METER_KEYS_CHARGING		5
#define MAX_SUPPORT_METER_KEYS_IDLE				10
typedef struct tagMeter_ValueItem
{
	BYTE					byValidElements; //0 means this Item is EMPTY
	METERVALUE_ELEMENT	meterValues[MAX_SUPPORT_METER_KEYS_CHARGING];
	//time for identify which sample in historical data
	time_t				tmSampled;
	//When OCPP reported this successfully, this will be set = 1, in case it needed be sent again
	BYTE					bIsReportedOK;
}A_CHARGE_METERVALUE_ITEM;

typedef struct tagMeterItemList
{
	A_CHARGE_METERVALUE_ITEM	stMeterVal_Start;
	A_CHARGE_METERVALUE_ITEM	stMeterVal_Stop;
#define MAX_HIS_STORE_METER_VALUES		500 //store it in every [1 min, or OCPP setting]
	A_CHARGE_METERVALUE_ITEM	stMeterValList[MAX_HIS_STORE_METER_VALUES];
	int						nMeterValTotal;

}CHARGE_METERITEM_LIST;

typedef struct tagBlobCargeMeterValue
{
	int						nGunNum; //use gun and time start to Identify
	time_t					tmChargeStart;

	CHARGE_METERITEM_LIST		meterData;
}HIS_METER_VALUE_REC;

typedef struct tagPcsRunInfo
{
	int						nExist; //0-doesnt exist, 1-exist
	int						nRectIndex; //which rect it is
	float					fOutputVolt;
	float					fOutputCurr;
	UINT					emWorkMode; //0-off, 1-rectify, 2-invert
	float					fAmbit_Temp;
	float					fTemp; //=ambit temp for now
}PCS_RUN_INFO_REC;

//added by John 20210930 for BMS diagnostic // LJH240925 RunData 增加V1兼容旧版本 LJH250303 delete BMS_RUNTIMEDATA_LIST_V1
// typedef struct tagA_BMS_RUNTIME_DATA_ITEM_V1
// {
// 	BYTE				nBmsIdx;  

// 	time_t				tmSaved; //the sampled time(ms) from  

// 	//Grid ACMeter 
// 	float				fGridMeter_Volt[3]; //either 3-phase AC meter 
// 	float				fGridMeter_Curr[3]; //either 3-phase AC meter 
// 	float				fGridMeter_Power[3]; //0:GridMeter_kWh(In)|1:GridMeter_kW|2:GridMeter_kWh(Out)

// 	//Cabinet ACMeter
// 	float				fCabMeter_Volt[3]; //either 3-phase AC meter 
// 	float				fCabMeter_Curr[3]; //either 3-phase AC meter 
// 	float				fCabMeter_Power[3]; //Kwh|KW|Neg KWh  

// 	//Battery RealTime Data
// 	STBATT_RUN_STATU    emBattStatus_Run;
// 	STBATT_ALARM_STATU  emBattStatus_Alm;

// 	BYTE                nSOC;
// 	BYTE				nSOH;

// 	float               fPackTotalVolt;
//     float               fPackTotalCurr;
	
// 	//Ctrl/Output Volt/Curr
// 	float               fPMCtrl_Volt;
//     float               fPMCtrl_Curr;

//  	float               fPMOutput_Volt;
//     float               fPMOutput_Curr;

//     float             	fAllow_LmtChargeVolt;
//     float               fAllow_LmtDisChargeVolt;
//     float               fAllow_MaxChargeCurr;
//     float               fAllow_MaxDisChargeCurr;
	
//     USHORT              nLifeCycleTimes;
// 	BYTE 				emCtrlPcsCmd;
//  	//S2A_SUMDATA3 
// 	USHORT 				uiBAT_Ucell_Max_CSC;
// 	USHORT				uiBAT_Ucell_Max_CELL;
// 	USHORT				uiBAT_Ucell_Min_CSC;
// 	USHORT				uiBAT_Ucell_Min_CELL;

// 	float				fMax_CellVolt;
// 	float				fMin_CellVolt; 
// 	float				fAvg_CellVolt;

// 	//S2A_SUMDATA4
// 	float				fMax_CellTemp;
// 	float				fMin_CellTemp;
// 	float				fAvg_CellTemp;
	
// 	USHORT				nBAT_T_Max_CSC;
// 	USHORT				nBAT_T_Min_CSC;
// 	USHORT 				uiBAT_T_Max_CELL;
// 	USHORT		 		uiBAT_T_Min_CELL;

// 	float				fChrg_Energy;
// 	float				fDisChrging_Energy;

//  	float				fACDCOutRelayVolt;	
// 	float 				fQ_PCSGrp;

// 	float 				f_QgridMeter;
// 	float 				f_QcabMeter;

// 	float				fQ_PCSGrpCtrl;		 

// 	//Cabinet DataInfo
// 	BYTE				byOnOrOffd;

// 	float				fAir_Temp1;
// 	float				fAir_Temp2;
// 	float				fAir_Humidity;

// 	float 				f_PauxMeter;
// 	float				fPacload4Chg;  
// 	float				fPacload4Dischg; 
// 	float 				fTotal_PessChgAvaliable;
// 	float 				fTotal_PessDischgLmt;

// 	float				fPowerCubeTemp[3];

// 	BYTE				byPCSStatu;

// 	float				fCalc_Chg_Power;
// 	float				fCalc_DisChg_Power;

// 	float				fAvg_Calc_Chg_Power;
// 	float				fAvg_Calc_DisChg_Power;

// 	float 				f_PauxMeter_3;
// 	float 				f_PmaxGridDemand;
// 	BYTE				bySTSSysStatu;
// 	BYTE 				byPCSSysStatu[MAX_SINGLE_STBATT_NUM];

// 	//---------------------20230617------------------//
	
// 	float				fInsulation_P_R;
// 	float				fInsulation_N_R;

// 	float				fOutsideVolt;
// 	float 				fNegativeBusVolt;
// 	USHORT 				usBattWorkState;

// 	BYTE				szFault[CAN_DATA_AREA_LEN];
// 	BYTE				szAlarm[CAN_DATA_AREA_LEN];
// 	BYTE			 	szDIDOStatu[CAN_DATA_AREA_LEN];

// 	float               fSingleChargeEnergy;
// 	float               fSingleDischargeEnergy; 

//  	//following is reserved for future update
// 	BYTE 				byReserved[11];

// } A_BMS_RUNTIME_DATA_ITEM_V1;

// typedef struct tagBmsRuntimeDataList_V1
// {
// 	UINT						nBmsRuntimeDataTotal;
// 	A_BMS_RUNTIME_DATA_ITEM_V1		stBmsRuntimeDataList[15000];
// 	BYTE 						byReserved[16]; //reserved for future update
// }BMS_RUNTIMEDATA_LIST_V1;

typedef struct tagA_BMS_RUNTIME_DATA_ITEM
{
	BYTE				nBmsIdx;  

	time_t				tmSaved; //the sampled time(ms) from  

	//Grid ACMeter 
	float				fGridMeter_Volt[3]; //either 3-phase AC meter 
	float				fGridMeter_Curr[3]; //either 3-phase AC meter 
	float				fGridMeter_kWPhs[3];
	float				fGridMeter_Power[3]; //0:GridMeter_kWh(In)|1:GridMeter_kW|2:GridMeter_kWh(Out)

	//Cabinet ACMeter
	float				fCabMeter_Volt[3]; //either 3-phase AC meter 
	float				fCabMeter_Curr[3]; //either 3-phase AC meter 
	float				fCabMeter_Power[3]; // CabMeter_kWh(In),CabMeter_kW,CabMeter_kWh(Out),

	//Battery RealTime Data
	STBATT_RUN_STATU    emBattStatus_Run;
	STBATT_ALARM_STATU  emBattStatus_Alm;

	BYTE                nSOC;
	BYTE				nSOH;

	float               fPackTotalVolt;
    float               fPackTotalCurr;
	
	//Ctrl/Output Volt/Curr
	float               fPMCtrl_Volt;
    float               fPMCtrl_Curr;

	float               fActvPwr_PCSGrp; // LJH0606
 	float               fPMOutput_Volt; // PCSOut_V
    float               fPMOutput_Curr; // PCSOut_A

	float				fMPPTOutPut_Volt; //MPPTOut_V    
	float				fMPPTOutPut_Curr;  //MPPTOut_A

    float             	fAllow_LmtChargeVolt;
    float               fAllow_LmtDisChargeVolt;
    float               fAllow_MaxChargeCurr; // LmtChg_A
    float               fAllow_MaxDisChargeCurr;
	
    USHORT              nLifeCycleTimes;
	BYTE 				emCtrlPcsCmd; // GRP_RECT_CTRL_CMDTYPE
 	//S2A_SUMDATA3 
	USHORT 				uiBAT_Ucell_Max_CSC;
	USHORT				uiBAT_Ucell_Max_CELL;
	USHORT				uiBAT_Ucell_Min_CSC;
	USHORT				uiBAT_Ucell_Min_CELL;

	float				fMax_CellVolt;
	float				fMin_CellVolt; 
	float				fAvg_CellVolt;

	//S2A_SUMDATA4
	float				fMax_CellTemp;
	float				fMin_CellTemp;
	float				fAvg_CellTemp;
	
	USHORT				nBAT_T_Max_CSC;
	USHORT				nBAT_T_Min_CSC;
	USHORT 				uiBAT_T_Max_CELL;
	USHORT		 		uiBAT_T_Min_CELL;

	float				fChrg_Energy;
	float				fDisChrging_Energy;
	float				fSingleChrgCapacity; //单次充电容量
	float				fSingleDischrgCapacity; //单次放电容量
	float				AccuChrgCapacity; //累计充电容量
	float				fAccuDischrgCapacity; //累计放电容量
	float				fAccuChrgEnergy; //累计充电能量
	float				fAccuDischrgEnergy; //累计放电能量


 	float				fACDCOutRelayVolt;	
	float 				fQ_PCSGrp;

	float 				f_QgridMeter;
	float 				f_QcabMeter;

	float				fQ_PCSGrpCtrl;		 

	//Cabinet DataInfo
	BYTE				byOnOrOffd;

	float				fAir_Temp1;
	float				fAir_Temp2;
	float				fAir_Humidity;

	float 				f_PauxMeter;
	float				fPacload4Chg;  
	float				fPacload4Dischg; 
	float 				fTotal_PessChgAvaliable;
	float 				fTotal_PessDischgLmt;

	float				fPowerCubeTemp[3];

	BYTE				byPCSStatu;

	float				fCalc_Chg_Power;    // fACDC_Distri_4_BattChrg_Pwr
	float				fCalc_DisChg_Power; // fBatt_Distri_4_ACDC_Pwr

	float				fAvg_Calc_Chg_Power;
	float				fAvg_Calc_DisChg_Power;

	float 				f_PauxMeter_3;
	float 				f_PmaxGridDemand;
	BYTE				bySTSSysStatu;
	BYTE 				byPCSSysStatu[MAX_SINGLE_STBATT_NUM];

	//---------------------20230617------------------//
	
	float				fInsulation_P_R;
	float				fInsulation_N_R;

	float				fOutsideVolt;
	float 				fNegativeBusVolt;
	USHORT 				usBattWorkState;

	BYTE				szFault[CAN_DATA_AREA_LEN];
	BYTE				szAlarm[CAN_DATA_AREA_LEN];
	BYTE			 	szDIDOStatu[CAN_DATA_AREA_LEN];
	BYTE				szLONGALARM[LONGALARM_NUM]; 

	//Air Conditioner DataInfo
	float				fBattInWaterTemp;
	float				fBattOutWaterTemp;
	float    			fSysAmbientTemp;

	float               fSingleChargeEnergy;
	float               fSingleDischargeEnergy;

	// LJH240925 RunData
 	float               fMax_Chrg_PwrAbility;
	float               fMax_DisChrg_PwrAbility;
	float               fBatt_Max_Chrg_Pwr_Req;
	float               fBatt_Max_DisChrg_Pwr;
	// LJH250610 fPwrAbility0
	float               fMax_Chrg_PwrAbility0; // MIN(fBatt_Max_Chrg_Pwr_Req, fACDC_Max_Output_Pwr_Ability);
	float               fMax_DisChrg_PwrAbility0; // MIN(fBatt_Max_DisChrg_Pwr, fACDC_Max_Output_Pwr_Ability);
	float               fACDC_Max_Output_Pwr_Ability; // LSYS
	BYTE			 	antiMode[3]; // 0:防逆流 1:防过载 2:防超需量
	// LJH241007 AntiPhase RunData
	float                           fGridRemainACPwr2Chrg;
	float                           fRemainPwr4Chrg;    // 剩余充电功率
	float                           fRemainDemand4Chrg; // 剩余需量
	float                           fAtRflxMtr_kWPhs[MAX_PHASE_NUM]; // 防逆流表单相功率值
	float                           fAtRflxMtr_Factor;
	float                           fDiffPwr4Dischrg_SumPhs; // 差值，防逆流时(保护值-单相最小功率值)*3
	float                           fAtOvlMtr_APhs[MAX_PHASE_NUM]; // 防过载表的单相电流值
	float                           fAtOvlMtr_Factor;
	float                           fMaxPwr4Chrg_SumPhs; // 按设置的最大电流值计算出来的3相最大功率和
	BYTE                            antiMode_Phase[2]; // bEnterAnti_Reflux_Phase bEnter_OverLoad_Phase
	BYTE                            autoChrgDischrg[2]; // bEnter_AutoChrg_SEMS bEnter_AutoDisc_SEMS
	float                           fAtRflx_kW; // 防逆流功率值
	float                           fAtOvl_kW;  // 防过载功率值
	// LJH241202 SEMS rundata REMS
	float                           f_PcabMeter_Client; // 客户端的柜内表
	BYTE                            bCommFailSemsClient;
	BYTE                            emEMSCtrlMode_REMS; // EMS控制模式
	float                           fActvPwrReq_REMS; // 有功功率需求 chrg+ dischrg-
	// LJH250726 东博上报MaxChg|DisChg都为0
	float                           fRatedPower; // 额定功率
	UINT                            nBattGrpPwrLmt; // 单组电池组功率限制
	// XSY250815 上报液冷空调信息
	BYTE							byLCAC_Type; // 液冷空调 型号 0-not install 1-infy 2-tf 3-eu3 4-xm

	BYTE							byLCAC_WorkMode; // 液冷空调 工作模式
	BYTE							byLCAC_BmsSetWM; // BMS设置的工作模式
	BYTE							byLCAC_CompState; // 压缩机状态--根据频率判断
	float							fLCAC_CompSpeed; // 压缩机转速
	float							fLCAC_ColTemp; // 制冷温度
	float							fLCAC_ColOpenDiff; // 制冷启动温差
	float							fLCAC_ColStopDiff; // 制冷停止温差
	float							fLCAC_HeatTemp; // 制热温度
	float							fLCAC_HeatOpenDiff; // 制热启动温差
	float							fLCAC_HeatStopDiff; // 制热停止温差
	BYTE							byLCAC_NoiseReductionMode; // 降噪模式
	float							fLCAC_XmSetTemp; // 新淼专用设定温度

	float							fLCAC_AmbiT; // 环境温度
	float							fLCAC_FeedT; //  进水温度
	float							fLCAC_ReturnT; // 回水温度
	float							fLCAC_ModFeedT; //  模块侧进水温度 2419
	float							fLCAC_ModReturnT; // 模块侧回水温度 2419
	float							fLCAC_ExPre; // 排气压力
	float							fLCAC_FeedPre; //  进水压力
	float							fLCAC_ReturnPre; // 回水压力

	USHORT							usLCAC_FaultCode[6]; // 故障码

	BYTE							by_BatEu3_DIStatus[3];
	BYTE							by_BatEu3_DOStatus[3];
	float							fNTC_BatEu3_Temp[4];  
	float							fTemperatureSen; //温湿度传感器  温度值	 
	float							fHumiditySen;    //温湿度传感器  湿度值
	float							fGasConcent1H2;			
	float							fGasConcent2C0;			
	float   						fHumidity;    //除湿机  湿度值

	BYTE							by_PowerEu3_DIStatus[3];
	BYTE							by_PowerEu3_DOStatus[3];
	float							fNTC_PowerEu3_Temp[4];
	BYTE                            emACDC_WorkStatu_REMS;
	
	BYTE                            byReserved[32]; //reserved for future update
	
}A_BMS_RUNTIME_DATA_ITEM; // 1788

typedef struct tagBmsRuntimeDataList
{

	UINT						nBmsRuntimeDataTotal;

	A_BMS_RUNTIME_DATA_ITEM		stBmsRuntimeDataList[MAX_HIS_BMS_RUNTIME_DATALIST];

	BYTE 						byReserved[16]; //reserved for future update
	
}BMS_RUNTIMEDATA_LIST;

typedef struct tagHisChargeRuntimeDataRec
{
	UINT 						nDay;
	time_t						tmLogTime;
	time_t						tmLastWriteRec; //just for write protect, no save
	//runtime data structure
	BMS_RUNTIMEDATA_LIST		bmsRunData;

}HIS_BMS_RUNTIMEDATA_REC;
 


//added for ES statistic ONE-DAY data
typedef struct tagESStatic_Daily
{
	ES_DAILY_STAT_DATA es_Data;

}HIS_ES_STATISTIC_DAILY;


//John 20220512, Added For BMS Health Statistic Daily Data

typedef struct tagDailyBMSDataInfo
{
	time_t				tmTimeStamp;

	float				fMax_CellVolt;
    float				fMin_CellVolt; 
	float 				fAvg_CellVolt;
 	
	USHORT 				uiBAT_Ucell_Max_CSC;  //Max = 14 Or 16
	USHORT 				uiBAT_Ucell_Max_CELL; //=14*4 Or 16*4
	USHORT 				uiBAT_Ucell_Min_CSC;
	USHORT 				uiBAT_Ucell_Min_CELL;

	float				fMax_CellTemp;
	float				fMin_CellTemp;
	float 				fAvg_CellTemp;

	USHORT				uiBAT_T_Max_CSC; 
	USHORT				uiBAT_T_Max_CELL;
	USHORT				uiBAT_T_Min_CSC;
	USHORT				uiBAT_T_Min_CELL;

	float				fChg_Start_MaxCellTemp;
	float				fChg_Start_MinCellTemp;

	float				fDischg_Start_MaxCellTemp;
	float				fDischg_Start_MinCellTemp;

	float				fPackTotalCurr;

	BYTE 				bySOC;

	BYTE				byRecv[32];


}A_RELEVANT_HEALTH_ELEM; 

typedef struct tagHealthStaticItem
{
	//-----		Charging		-----//
	float					fKey_Chg_TmsvDif_CellVolt;
	A_RELEVANT_HEALTH_ELEM	chgTmsvDifCellVolt;

	float					fKey_Chg_TmsvDif_CellTemp;
	A_RELEVANT_HEALTH_ELEM	chgTmsvDifCellTemp;

	float					fKey_Chg_Max_CellTemp;
	A_RELEVANT_HEALTH_ELEM	chgMaxCellTemp;

	float 					fKey_Chg_Min_CellTemp;
	A_RELEVANT_HEALTH_ELEM	chgMinCellTemp;

	//-----		Discharging		-----//
	float					fKey_Dischg_TmsvDif_CellVolt;
	A_RELEVANT_HEALTH_ELEM	dischgTmsvDifCellVolt;

 	float					fKey_Dischg_TmsvDif_CellTemp;
	A_RELEVANT_HEALTH_ELEM	dischgTmsvDifCellTemp;

	float 					fKey_Dischg_Max_CellTemp;
	A_RELEVANT_HEALTH_ELEM	dischgMaxCellTemp;

	float 					fKey_Dischg_Min_CellTemp;
	A_RELEVANT_HEALTH_ELEM	dischgMinCellTemp;


	float 					fRack_TotalCapacity;

	float 					fPack_Ohm[16];//RESV

	float					fKey_Resv[4];
	A_RELEVANT_HEALTH_ELEM	Resv[4];


	BYTE					byRecv[32];

}A_BMS_HEALTH_ITEM;


typedef struct tagHealthDataList
{
	UINT 				nInstalledNum;

	A_BMS_HEALTH_ITEM	healthItem[MAX_SINGLE_STBATT_NUM];

}A_BMS_HEALTH_LIST;

typedef struct tagBMSDailyHealthStatic
{
	UINT					nIdx;

	UINT					nDay;

	time_t					tmLogTime;

	A_BMS_HEALTH_LIST		bmsHealthData;

}HIS_HEALTH_STATIC_DAILY;


typedef struct tag_Efficiency_Data_Daily
{
	time_t					tmTimeSaved;

	float					fTotalACInputKWH_Daily;

	float					fTotalACOutputKWH_Daily;

	float					fTotalACInputKWH_Monthly;

	float					fTotalACOutputKWH_Monthly;

	float					fTotalACInputKWH_Yearly;

	float					fTotalACOutputKWH_Yearly;

	int						nFlagIsMonthDataRecordNeeded;

	int						nFlagIsYearDataRecordNeeded;

}EFFICIENCY_DATA_DAILY;


typedef struct tag_Efficiency_Data_Query
{
	int						nType;

	time_t					tmTime;
	
	float					fValue;
}EFFICIENCY_DATA_QUERY;


// //Max support 1 year (366 days) data
// #define MAX_SUPPORT_ESSTATISTIC_DAYS_DATA	366
// typedef struct tagESStatic_Year
// {
// 	int						nTotalRecordDays;
// 	//This is to identify which day the data was from
// 	time_t					tmDayofYear[MAX_SUPPORT_ESSTATISTIC_DAYS_DATA];
// 	HIS_ES_STATISTIC_DAILY	data_perDay[MAX_SUPPORT_ESSTATISTIC_DAYS_DATA];

// 	time_t					tmLastUpdateRecords; //last time write record to DB
// 	BOOL 					bNeedUdateDailyData[MAX_SUPPORT_ESSTATISTIC_DAYS_DATA];
// }HIS_ES_STATISTIC_YEAR;


#endif
