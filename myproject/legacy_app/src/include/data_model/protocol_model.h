/********************************************************************************
* Copyright(c) Shenzhen Infypower Co., Ltd.
* @author LJH936
* @brief 站级EMS的协议数据结构
* @history 2024-12-02 created
1. LJH250401 *与佛山川崎版本不兼容 topology#6#7 
	1) idxClient由BYTE改为UNIT
	2) 删除tmStamp SEMS_B6_LSYS_DATA_INFO
	3) 删除客户端详细信息增加电表信息
2. LJH250428 增加cmd155 cmd157上报协议
3. LJH250605 fix:主控SOC因无小数部分放电时过早停止 *版本不兼容V25.0620LEF002前的版本 git:b6fdb8d7 250628
	1) B6_LSYS a.增加: nSOCMinSum nSOCMaxSum fMax_DisChrg_PwrAbilitySum0 fMax_Chrg_PwrAbilitySum0
	           b.修改: BYTE nSOCSum->float fSOCSum
	2) SEMS_A6_USYS_DATA_INFO // LJH250612 planSector 增加EMS_LIMIT_SECTOR的参数
	3) S63Uctl_30WorkPwr 删除nStopSoc
4. CFGG LJH250630 SEMSProtVer:协议版本号:1
5. // LJH250930 加快自动充放响应 SEMSProtVer:协议版本号:2
	1) SEMS_61_CFGG_DATA_INFO a.增加:nStopSOC_DiscSet/nStopSOC_ChrgSet b.主机也发CFGG到从机
	2) 增加:S6360_Uctl_Standby
    3) SEMS_B6_LSYS_DATA_INFO 增加:bStandby,byWorkAsap,fMax_Chrg_PwrAbilitySumA, fMax_DisChrg_PwrAbilitySumA
	4) S63Uctl_30WorkPwr 增加:byWorkAsap:bAutoChrg+bAutoDisc
*******************************************************************************/

#ifndef __PROTOCOL_STRUCTS_H
#define __PROTOCOL_STRUCTS_H

#include "base_defines/basetypes.h"

// #define SEMS_PROTOCAL_VER             (1) // LJH250630 SEMSProtVer
#define SEMS_PROTOCAL_VER             (2) // LJH250930 加快自动充放响应
#define MAX_LEN_MU2SWVER              (32)

#define MAX_S_COMMFAIL_SEMS           (10)   // 判断通信失败的时间
#define MAX_S_WAIT_FIRSTBOOT          (120)  // 系统启动后等待时间
#define MAX_MS_VALIDDATA_SEMS         (5000)
#define MAX_MS_DIFF_4CALIBRATETIME    (5000) // 校准时间差

#define MAX_QUERY_ACTV_ALM        50

#define MAX_BATTERY_NUM	4
#define MAX_HOURS_SECTION_202 48
#define MAX_PSC_NUM 4
#define MAX_BUFFER_LNE 24

// 自动手动模式
typedef enum tagCtrlRectMode
{
	CtrlRectMode_Auto = 0xA0, // 手动控制充放电
	CtrlRectMode_Manu = 0xA1  // 自动控制充放电
} CtrlRectMode;

// STS:自动/手动并离网(需关机)
typedef enum tagAutoManuSwitchMode
{
	AutoManuSwitchMode_Auto = 0xA0, // 自动离并网切换（默认）
	AutoManuSwitchMode_Manu = 0xA1  // 手动离并网切换
} AutoManuSwitchMode;

// STS:并/离网
typedef enum tagOnOffGridMode
{
	OnOffGridMode_On_Invert = 0xA1, // A1-并网逆变（默认）
	OnOffGridMode_Off =       0xA2  // A2-离网
} OnOffGridMode;

typedef enum tagLCCommStatus
{
	LC_COMM_FAIL, // 0:中断
	LC_COMM_SUCC  // 1:正常
} LCCommStatus;

#pragma pack(1)
// 61站级EMS获取配置参数（SEMS_CFGG）// LJH250930 加快自动充放响应,主从发送一样的数据帧
typedef struct tagSEMS61_CFGGDataInfo
{
	BYTE                    reserve[17];
	BYTE                    reserveBits:7; // old:reserve[20]
	BYTE                    bSendCmdByLC1stOnly:1;
	BYTE                    nStopSOC_DiscSet;
	BYTE                    nStopSOC_ChrgSet; // LJH250930 加快自动充放响应,设置的截止SOC参数
	UINT                    SEMS_LSYS_Period;
	
	BcdTimeStmp             tmStamp;
	char                    szIMMU2_SWVer[MAX_LEN_MU2SWVER];
	UINT                    idxClient;
	UINT                    nSlaveIP;
	BYTE                    byVerSEMSProt; // LJH250630 SEMSProtVer
}SEMS_61_CFGG_DATA_INFO;

// 62站级EMS设置从机配置参数（SEMS_CFGS）
#define MAX_BYTES_SINGLE_VALUE  (20)
typedef struct tagS62CFGS_Single
{
	BYTE                    subCmd;
	union
	{
		BYTE                bytesVal[MAX_BYTES_SINGLE_VALUE];
		BcdTimeStmp         sub02BcdTimeStmp;
		UINT                sub04Period;
	};
} S62CFGS_Single;

typedef struct tagS63UCTL_Single
{
	BYTE                    subCmd;
	UINT                    idxClient;
	union
	{
		BYTE                bytesVal[MAX_BYTES_SINGLE_VALUE];
		BYTE                byteVal;
	};
} S63UCTL_Single;

// 63-01并离网自动切换模式
typedef struct tagS63Uctl_01GridOnOffAutoSwitchMode
{
	BYTE					GridOnOffAutoSwitchMode;		//ABU：（地址FA）0xA0-自动离并网切换（默认） 0xA1-手动离并网切换；(注：只有关机状态下，设置模式才生效)
	UINT                    idxClient;
	BYTE                    subCmd;
} S63Uctl_01GridOnOffAutoSwitchMode;

// 63-02并离网模式(reserve)
typedef struct tagS63Uctl_02GridOnOffMode
{
	BYTE					GridOnOffMode; 		//ABU：A1-并网逆变（默认）；A2-离网；(注：只有关机状态下，设置模式才生效)
	UINT                    idxClient;
	BYTE                    subCmd;
} S63Uctl_02GridOnOffMode;

// 63-14手动控制充放电功率
typedef struct tagS63Uctl_14ManualChargeDischargePower
{
	BYTE                    nEnergySchedulingMode;//        0x00-DC直流调度   0x01-AC交流调度
	SHORT                   usReActv_Pwr; 	//%
	float                   fCurrent;		//A
	float                   fVolt;			//V
	float                   fActv_Pwr;    //充/放电功率需求
	BYTE                    nStopSoc;
	BYTE                    emACDC_WorkStatu;
	UINT                    idxClient;
	BYTE                    subCmd;
} S63Uctl_14ManualChargeDischargePower;

// 63-17手动模式
typedef struct tagS63Uctl_17ManualMode
{
	BYTE					ManualMode;			//0xA0-Exit Debug Mode   0xA1-Enter Debug Mode (IMMU2 should Alarm when Enter Debug Mode)
	UINT                    idxClient;
	BYTE                    subCmd;
} S63Uctl_17ManualMode;

typedef union tagU6330WorkAsap
{
	BYTE byWorkAsapData; 
	struct
	{
		BYTE                    bChrgAsap : 1;
		BYTE                    bDiscAsap : 1;
		BYTE                    reserve :   6;
	};
} U6330WorkAsap;

//6330自动控制充放电功率
typedef struct tagS63Uctl_30WorkPwr
{
	BYTE                    byWorkAsap; // U6330WorkAsap LJH250930 加快自动充放响应,WorkAsap,AutoChrg Disc bCtrlByREMS
	float                   fActv_PwrCtrl; // 分配的功率值
	// BYTE                    nStopSocCtrl; // LJH250612 改为由USYS下发
	BYTE                    emACDC_WorkStatuCtrl;
	UINT                    idxClient;
	BYTE                    subCmd;
} S63Uctl_30WorkPwr;

// 6330自动控制充放电功率
typedef struct tagSems6330_CtrlWorkPwr_Response
{
	BYTE                    RTN;  //0- success  1-fail
	UINT                    idxClient;
	BYTE                    subcmd;
} Sems6330_CtrlWorkPwr_Response;

// 63-60热待机模式 // LJH250930 加快自动充放响应
typedef struct tagS6360_Uctl_Standby
{
	BYTE                    bStandby_BLL;
	UINT                    idxClient;
	BYTE                    subCmd;
} S6360_Uctl_Standby;

// 63-90启用/禁用系统
typedef struct tagS63Uctl_90DisEnableSys
{
	BYTE					DisEnableSYS;		//0x00-idle   0x01-lock
	UINT                    idxClient;
	BYTE                    subCmd;
} S63Uctl_90DisEnableSys;

//63-90站级EMS OTA升级从机
typedef struct tagS63Uctl_UpdateFirmwareInfo
{
	char					szURL[255]; //"local" means from LCD SD card downloading
	time_t					dtRetrieveDate;
	UINT                    idxClient;
	BYTE                    subCmd;
} S63Uctl_EmsUpgradeCmd;

// 64获取从机信息(SEMS_RTIF)
typedef struct tagSEMS64_RTIFDataInfo
{
	UINT                    idxClient;

}SEMS_64_RTIF_DATA_INFO;

typedef struct tagPCSInfoData
{
	int 				  	nPCSNum;
	char					szVersion[MAX_PSC_NUM][MAX_BUFFER_LNE];
	char					szBar_code[MAX_PSC_NUM][MAX_BUFFER_LNE]; 
	UINT                    idxClient;
	BYTE                    subCmd;
}PCS_INFO_DATA;

// A6主机状态信息（USYS）
typedef struct tagSEMSA6_USYSDataInfo
{
	UINT                    nBattGrpPwrLmtPlan; // 单组电池组功率限制
	int                     nReActvPwrLmtPlan;
	UINT                    nActvPwrLmtPlan; // 总功率限制
	BYTE                    nStopSOCPlan;
	BYTE                    emBatt_WorkModePlan; // STBATT_PRI_WORKMODE
	// LJH250612 增加EMS_LIMIT_SECTOR的参数，主控管理计划数据

	BYTE                    emOnOffGridMode; // OnOffGridMode
	BYTE                    emAutoManuSwitchMode; // AutoManuSwitchMode
	BYTE                    emCtrlRectMode; // CtrlRectMode
	BYTE                    emUSYSStationEMSAvaiStatus; // emCCUState
	BcdTimeStmp             tmStamp; // don't move! offsetof(SEMS_A6_USYS_DATA_INFO, tmStamp)
	UINT                    idxClient;
} SEMS_A6_USYS_DATA_INFO;

// typedef struct tagSemsB6_LSYSBattMessageData
// {
// 	float		fChrgedEnergy;
// 	float		fMax_DisChrg_PwrAbility;
	
// 	float		fMax_Chrg_PwrAbility;
// 	float		fBatt_Max_DisChrg_Pwr;
// 	float		fBatt_Max_Chrg_Pwr_Req;
// 	float		fPackTotalCurr;
// 	float		fPackTotalVolt;
	
// 	BYTE		nSOC;
// 	float		fQ_PCSGrp;
// 	float		fActvPwr_PCSGrp;
// 	float		fACDCOutRelayVolt;
// 	BYTE		emACDC_WorkStatu;

// 	float 		fACDC_Max_Output_Pwr_Ability;
// 	float 		fACDC_RealTime_Curr;
// 	float		fACDC_RealTime_Volt;
// 	BYTE		nOnlineACDCNum;
// 	BYTE 		bBattNum;

// }SEMS_B6_LSYS_BATT_GRP_INFO;

typedef union tagUB6State
{
	BYTE byStateData[3];
	struct
	{
		BYTE                    byWorkAsapSum; // U6330WorkAsap LJH250930 加快自动充放响应,WorkAsap,AutoChrg Disc bCtrlByREMS

		BYTE                    bStandby_BLLSum :     1;
		BYTE                    bStandby_SetSum :     1; // LJH250930 加快自动充放响应,standbySetReq
		BYTE                    reserve :             2;
		BYTE                    emACDC_WorkStatuSum : 4; //ACDC charge/discharge STATU_ACDC_IDLE = 0,

		BYTE                    emPMCtrlModeSum : 4; //从机-d运行模式 自动模式-手动模式 STAT_PM_CTRL_MODE
		BYTE                    emCCUStateSum :   4; //从机系统状态 CCU_STATE
	};
} UB6State;

// 5.2.2 B6从机状态信息（LSYS）
typedef struct tagSEMSB6_LSYSDataInfo
{
	// LJH250401 topology#6#7 deleted:BATT_GRP_INFO另加协议
	// BYTE					nOnlineBattGrpNum;  
	// USHORT  				usBattGrpMessageLen;

	// LJH250401 topology#6#7 added
	// float                   fPacload4DischgSum;
	// float                   fPacload4ChgSum;

	float                   f_PcabMeterSum;  // 16. 柜内表
	float                   f_PgridMeterSum; // 15. 网侧表

	float                   fACDC_RealTime_CurrSum;
	float                   fACDC_RealTime_VoltSum;
	float                   fRealTime_MaxCurrSum;
	float 					fChrgedEnergySum;       //11 sys current total charged kwh

	float                   fMax_DisChrg_PwrAbilitySum; // 业务相关 sys current total max discharge power
	float                   fMax_Chrg_PwrAbilitySum;    //sys current total max charge power
	float                   fMax_DisChrg_PwrAbilitySumA;
	float                   fMax_Chrg_PwrAbilitySumA; // bSystemOK, bStopChrgBySoc_SetSEMS // LJH250930 加快自动充放响应,WorkASAP
	float                   fMax_DisChrg_PwrAbilitySum0;
	float                   fMax_Chrg_PwrAbilitySum0; // LJH250606 added:PCS BMS上报的原始值

	float 					fQ_PCSGrpSum;        //sys PCS Total output reactive power 
	float 					fActvPwr_PCSGrpSum;  //sys PCS Total output active power  （positive ：charge    negative：discharge）
	float 					fSOCSum;             //sys Total soc // LJH250606 fix:主控SOC因无小数部分放电时过早停止
	BYTE                    nSOCMaxSum;
	BYTE                    nSOCMinSum;

	BYTE                    byWorkAsapSum; // U6330WorkAsap LJH250930 加快自动充放响应,WorkAsap,AutoChrg Disc bCtrlByREMS
	BYTE                    bStandby_BLLSum :     1; // 业务待机
	BYTE                    bStandby_SetSum :     1; // LJH250930 加快自动充放响应,standbySetReq
	BYTE                    reserve :             2;
	BYTE                    emACDC_WorkStatuSum : 4; //ACDC charge/discharge STATU_ACDC_IDLE = 0,
	BYTE                    emPMCtrlModeSum : 4; //从机-d运行模式 自动模式-手动模式 STAT_PM_CTRL_MODE
	BYTE                    emCCUStateSum :   4; //从机系统状态 CCU_STATE

	// BYTE                    emACDC_WorkStatuSum; //ACDC charge/discharge STATU_ACDC_IDLE = 0,
	// BYTE                    emPMCtrlModeSum;     //从机-d运行模式 自动模式-手动模式 STAT_PM_CTRL_MODE
	// BYTE                    emCCUStateSum;       //从机系统状态 CCU_STATE
	BcdTimeStmp             tmStamp;
	UINT                    idxClient;           //从机编号
} SEMS_B6_LSYS_DATA_INFO;

// 5.2.3 C6储能柜信息（LDEV）
typedef struct tagSC6LDEV_CMDHead
{
	BYTE                    byDevNum; // 单信息个数
	USHORT                  usLenSingle; // 单信息长度 sizeof(SC6_LDEV_xxx)
	UINT                    idxClient; // 上报本条LDEV的从机编号
	USHORT                  usCmd; // 101~
} SC6LDEV_CMDHead;

//edit by KYA01205 20250509
typedef struct tagSC6_LDEV_206
{
    char 					time[32];
    float 					valley_neg_kwh;
    float 					flat_neg_kwh;        // Flat period negative active energy (kWh), mandatory

	float 					peak_neg_kwh;
    float 					sharp_neg_kwh;
    float 					sum_neg_kwh;
    float 					valley_pos_kwh;
    float 					flat_pos_kwh;        // Flat period positive active energy (kWh), mandatory

    float 					peak_pos_kwh;
    float 					sharp_pos_kwh;
    float 					sum_pos_kwh;
    BYTE 					time_period;
    USHORT 					data_from;           // Meter source: 407-power cabinet, 401-user branch 1, 402-user branch 2, mandatory

} SC6_LDEV_206;

typedef struct tagSC6_LDEV_204
{
    float 					value_batt_dischrg_times;	
    float					value_batt_chrg_times;	
    float 					org_acload_valleycost_kwh;	
    float 					org_acload_flatcost_kwh;	
    float 					org_acload_peakcost_kwh; 	// User-side load peak period energy consumption (kWh), mandatory

    float 					org_acload_sharpcost_kwh;	
    float 					local_ac_valleyoutput_kwh;	
    float 					local_ac_flatoutput_kwh;	
    float 					local_ac_peakoutput_kwh; 	
    float 					local_ac_sharpoutput_kwh; 	// Cabinet AC meter sharp period output energy (kWh), mandatory

    float 					local_ac_valleyinput_kwh;	
    float 					local_ac_flatinput_kwh;	
    float 					local_ac_peakinput_kwh;	
    float 					local_ac_sharpinput_kwh;	
    float 					value_batt_dischrg;     	// Battery cluster discharge energy (kWh), mandatory
	
    float 					value_batt_chrg;       	
    float 					value_mppt_valley_output;	
    float 					value_mppt_flat_output;	
    float 					value_mppt_peak_output;	
    float 					value_mppt_sharp_output; 	// MPPT group sharp period output energy (kWh), mandatory

	float 					value_mppt_total_output;	
	float 					dcload_cost_kwh;	
	float 					local_ac_output_kwh;	
	float 					local_ac_input_kwh;	
	float 					rmt_ac_revs_kwh;        	// User-side total meter reverse output energy (kWh), mandatory

    float 					org_acload_cost_kwh;	
    float 					rmt_ac_cost_kwh;	
    float 					rmt_ac_realtime_kwh;	
    char 					time_stamp[32];	
    USHORT 					data_from;              	// Data source: 0 for power cabinet, mandatory

} SC6_LDEV_204;


typedef struct tagSC6_LDEV_202
{
	float 					value_batt_dischrg[MAX_BATTERY_NUM][MAX_HOURS_SECTION_202];     // Battery cluster discharge energy (kWh, 1-48 segments), mandatory
	float 					value_batt_chrg[MAX_BATTERY_NUM][MAX_HOURS_SECTION_202];

    float 					value_mppt_output[MAX_BATTERY_NUM][MAX_HOURS_SECTION_202];

    float 					dcload_cost_kwh[MAX_HOURS_SECTION_202];
    float 					local_ac_output_kwh[MAX_HOURS_SECTION_202];
    float 					local_ac_input_kwh[MAX_HOURS_SECTION_202];     // Cabinet AC meter input energy (kWh, 1-48 segments), mandatory

	float 					rmt_ac_revs_kwh[MAX_HOURS_SECTION_202];
    float 					org_acload_cost_kwh[MAX_HOURS_SECTION_202];
    float 					rmt_ac_cost_kwh[MAX_HOURS_SECTION_202];
    float 					rmt_ac_realtime_kwh[MAX_HOURS_SECTION_202];
	int						data_from;
	int						iBattGroupNum;
} SC6_LDEV_202;

typedef struct tagSC6_LDEV_111
{
    char 					time[32];
    BYTE 					sub_id;
    BYTE 					trig_msg;       	// Supported message trigger codes: 101, 105, 107, 206, mandatory
} SC6_LDEV_111;

typedef struct tagSC6_LDEV_110
{
	int						iCellNum;
    BYTE 					batt_idx;
} SC6_LDEV_110;

typedef struct tagSC6_LDEV_CELL_110
{
	BYTE 					cell_balance; 	// Battery cell balance status values, max 250, mandatory
	float 					cell_temp;
    float 					cell_volt;
	BYTE					cell_safe;
   
} SC6_LDEV_CELL_110;

typedef struct tagSC6_LDEV_109
{
	time_t			tmEnd;
	time_t			tmStart;
    float			fExParamVal;
	ALARM_STATUS	status;
	int				nAlarmId;
	int				nDevId;

} SC6_LDEV_109;

// 5.2.3.1 C6-9B系统能量图数据(155) // LJH250428 cmd155
typedef struct tagSC6LDEV_CMD155_BranchData
{
	BYTE                    byESCNum;
	USHORT                  usLenSingle;

	float                   kw_acload;
	float                   kw_meter_cab;

	float                   kw_meter_grid;
	// BYTE                    position_auxmeter; // apply_scenery;
	float                   ac_currs[3];
	float                   ac_volts[3]; // 0-C 1-B 2-A
	BYTE                    ac_workstat;
	UINT                    dev_id;
} SC6LDEV_CMD155_BranchData;

typedef struct tagSC6LDEV_CMD155_ESCData
{
	float                   batt_curr;
	float                   batt_volt;
	BYTE                    batt_soc;
	BYTE                    batt_workstat;

	float                   acdc_ext_volt;
	float                   acdc_curr;
	float                   acdc_volt;
	BYTE                    acdc_workstat;

	float                   kw_meter_cab;
	float                   ac_currs[3];
	float                   ac_volts[3]; // 0-C 1-B 2-A
	BYTE                    ac_workstat;

	BcdTimeStmp             tmStamp;
	UINT                    dev_id; // idxClient
} SC6LDEV_CMD155_ESCData;


// 5.2.3.2 C6-9D配电数据信息(157) // LJH250520 cmd157
typedef struct tagSC6LDEV_CMD157
{
	float 					ac_currentdemand;
	float 					ac_pmaxdemand;
	float 					ac_freq;
	float 					power_factor;
	float 					reactive_power;   	// Reactive power (kW), mandatory

    float 					active_power;
    float 					kvarh_reading;
    float 					neg_kwh_reading;
    float 					kwh_reading;
    float 					phase_pf[3]; // reserved ABC phase power factor (reserved), optional

	float 					phase_kvar[3]; // reserved
	float 					phase_kw[3];   // reserved
	float 					phase_currs[3];
	float 					phase_volts[3]; // 0-C 1-B 2-A
	USHORT 					data_from;          // Meter source (0, 99, 100, etc.), mandatory

	BcdTimeStmp             tmStamp;
	UINT                    dev_id; // idxClient
} SC6LDEV_CMD157;

// edit by KYA 20250520
typedef struct tagSC6_LDEV_108_Data
{
    char 					hw_lccu_ver[32];
    char 					sw_lccu_ver[8];
    char 					hw_main_ver[32];
    char 					sw_main_ver[32];
	UINT 					serial_id;
    USHORT 					data_from;          // Data source: 0 for power cabinet, mandatory

} SC6_LDEV_108_Data;

// edit by KYA 20250520
typedef struct tagSC6_LDEV_106_Data
{
    float   				reactive_power;
    float   				active_power;
    BYTE    				bar_code[24];               	// Barcode information, optional

	float   				pfc_var;
    float   				dcdc_ver;
    float   				output_curr;
    float   				output_volt;
	BYTE    				invrt_mode_errcode[16];     	// Fault code in inversion mode, optional

    BYTE    				rect_mode_errcode[16];
    BYTE     				err_code;
    BYTE     				work_mode;
    BYTE     				belong_grp;
    BYTE     				comm_statu;                 	// Communication status: 0-normal, 1-interrupted, optional

    USHORT     				data_from;                      // 0 for power cabinet, mandatory

} SC6_LDEV_106_Data;

typedef struct tagAllSingleRectInfo_106
{
	BYTE     				online_mppt_num;
	BYTE 					online_dcdc_num;
	BYTE     				online_acdc_num;

	UINT 					nOnlineNum[13];
	SC6_LDEV_106_Data 		arrRectPcs[13][MAX_SINGLE_RECT_NUM];
	UINT                    dev_id; // idxClient // KYA20250802

}A_BLOB_SINGLE_RECT_INFO_106;

// edit by KYA 20250520
typedef struct tagSC6_LDEV_104_Data
{
	float 					acdc_distri4batt_chrg_pwr;       
	float 					remain_batt_chrg_pwr_req;         
	float 					remain_batt_dischrg_pwr;          
	float 					dcdc_rate_pwr;                    
    float 					acdc_rate_pwr;                    	// Rated output power of PCS group (kW), mandatory

    float 					batt_distri4dcdc_pwr;             
    float 					batt_distri4acdc_pwr;            
	float 					batt_max_chrg_pwr_req;            
	float 					batt_max_dischrg_pwr;             
    BYTE 					branch_idx;   				  		// Branch index (0-9), mandatory

} SC6_LDEV_104_Data;

// edit by KYA 20250520
typedef struct tagSC6_LDEV_103_Data
{
	float 					batt_ver;                   
	BYTE 					gpio_info[36];                
	BYTE 					fault_info[36];               
	BYTE 					alarm2_info[36];              
    BYTE 					alarm_info[36];               // BMS alarm information, optional

	BYTE 					alarm_status;                 
	BYTE 					dischrg_forbid;               
	BYTE 					chrg_forbid;                  
	float 					remain_discharge_energy;     
    float 					remain_charge_energy;        // Remaining charge energy (kWh), optional

	// Battery group data array
	float 					temp_min_csc;                
	float 					temp_max_csc;                
	float 					ucell_min_cell;              
	float 					ucell_min_csc;               
    float 					ucell_max_cell;              // Cell position of max cell voltage, optional

	float 					ucell_max_csc;               
	float 					cell_avg_temp;               
	float 					cell_min_temp;               
    float 					cell_max_temp;               
    float 					cell_avg_volt;               // Average cell voltage (V), optional

	// Common fields
    float 					cell_min_volt;              
    float 					cell_max_volt;               
    float 					max_allow_dischg_curr;       
    float 					max_allow_dischg_volt;       
    float 					max_allow_chg_curr;          // Max allowed charge current (A), mandatory

	// Optional fields
    float 					max_allow_chg_volt;          
    BYTE 					cycle;                         
	BYTE 					soc;                           
	BYTE 					soh;                          
    float 					pack_curr;                   // Battery current (A), mandatory

    // Mandatory fields
	float 					pack_volt;                   
	BYTE 					run_statu;                    
	BYTE 					batt_idx;                                            
} SC6_LDEV_103_Data;

// edit by KYA 20250520
typedef struct tagSC6_LDEV_102_Data
{

#define MAX_HUMITEMP_NUM			8
#define MAX_GASSENSOR_NUM			4
#define MAX_PT1000_NUM			    10

	float					comb_gas_concentration_value[MAX_GASSENSOR_NUM];		
	BYTE 					combustible_gas_alarm_stat;
	BYTE 					combustible_gas_comm_stat[MAX_GASSENSOR_NUM];
    float 					dehumi_stop_humidity;            	
	float 					dehumi_start_humidity;           	// Dehumidifier start humidity in %rh
	//Comb Gas Potency Detection
	UINT 					nGasSensorNum;

	float 					dehumi_stop_temperature;         	
    float 					dehumi_start_temperature;        	
	BYTE 					aircond_machine_state;            	
	float 					aircond_heater_start_temp;       	
    float 					aircond_compressor_start_temp;   	// Air conditioner compressor start temperature

	float 					aircond_condenser_temp;          	
    float 					aircond_evaporator_temp;         	
	float 					aircond_ret_temp;                	
	BYTE 					aircond_alarm;                    	
    BYTE 					aircond_comm_stat;                	// Air conditioner communication status: 0-normal, 1-alarm

	float 					exhaust_fan_speed_value;         	
	float                   comb_pt1000_value[MAX_PT1000_NUM];		
	float 					cab_humi_value[MAX_HUMITEMP_NUM];    							
	float 					cab_temp_value[MAX_HUMITEMP_NUM]; 							
	BYTE 					temp_humi_sensor_comm_stat[MAX_HUMITEMP_NUM];  //(0-正常，1-告警)

    BYTE 					electric_blinds_close_stat;       	
	BYTE 					electric_blinds_open_stat;        	
	BYTE 					dump_alarm;                       	
	BYTE 					combustible_gas_alarm;            	
	BYTE 					fire_alarm;             		  	// Fire alarm: 0-normal, 1-alarm

	BYTE 					smoke_alarm;            			
	BYTE 					dooropen_alarm;         			
	BYTE 					flood_alarm;            			
	BYTE 					comm_statu;             			
	BYTE 					cab_idx;                			// Battery cabinet index, max value 10

	BYTE					batt_cab_num;						
	USHORT                  data_from;  						// 1.数据来源，0-电源柜
	//----------Temperature & Humi-------//
	UINT 					nHumiTempSensorNumber;
	UINT                    dev_id; // idxClient // KYA20250609
}SC6_LDEV_102_Data;

// edit by KYA 20250520
typedef struct tagSC6_LDEV_101_Data
{
	USHORT					comm_status;
	BYTE 					data_from;

	BYTE					chrg_slvcab_num;		
	float 					powercube_sys_temp;			
	BYTE					cp_fault;				
	USHORT					grid_status;
	BYTE 					cp_status;				// 1.储能系统状态
	UINT                    dev_id; // idxClient // KYA20250802

}SC6_LDEV_101_Data;

#pragma pack()

typedef enum tagEMSCtrlMode
{
	EMSCtrlMode_Local,
	EMSCtrlMode_Remote
} EMSCtrlMode;

#endif
