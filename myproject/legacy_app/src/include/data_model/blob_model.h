#ifndef _BLOB_MODEL_2018_06_09_
#define _BLOB_MODEL_2018_06_09_

#include "base_defines/basetypes.h"
#include "protocol_model.h"

// Cell Capacity
#define AIR_CELL_CAPACITY 280

// A2415 
#define LIQUIDED_CELL_CAPACITY 314
// #define LIMITCURR_IN_RUN 165
#define LIMITCURR_IN_RUN 160
#define DTEST_TEMP_GAP 6
#define DTEST_POW_EFF 92
#define DTEST_SYS_EFF 86
// A2445
// #define LIQUIDED_CELL_CAPACITY 285
// #define LIMITCURR_IN_RUN 283
// #define DTEST_TEMP_GAP 8
// #define DTEST_POW_EFF 91
// #define DTEST_SYS_EFF 85

//edit by Kuangyue 20241114
#define SYSTYPE_BATTERY_NUM                    4
#define SYSTYPE_ESS_AIR_PACK_NUM                    15
#define SYSTYPE_ESS_AIR_SINGLE_CELL_NUM 			16
//edit by Kuangyue 20241114
#define SYSTYPE_ESS_LIQUIDED_PACK_NUM               5

#ifdef IMMU3
	#define SYSTYPE_ESS_LIQUIDED_SINGLE_CELL_NUM 		52 // 2524 是52个电芯一个pack，以后其他mu3版可能会变，需想办法兼容
#else
	#define SYSTYPE_ESS_LIQUIDED_SINGLE_CELL_NUM 		48
#endif

#define SYSTYPE_ESS_LIQUIDED_836_SINGLE_CELL_NUM			52
#define LIQUIDED_836_PACK_NUM								8

#define LIQUID_COOLING_BACKUP_PACK_NUM 				4

#define MAX_BLOB_DATALEN			(100*1024)	//max support 32K blob data
#define MAX_SINGLE_STBATT_NUM		4
#define MAX_SEMS_CLIENTS_NUM        (20) // LJH241202 SEMS Clients Number
#define MAX_NUM_ESC                 (20) // LJH250428 cmd 最大储能柜数量
#define MAX_NUM_BRANCH              (4)  // LJH250428 cmd 最大支路数量
#define MAX_NUM_METER               (30)
//#define MAX_NODE_NUMBER 			8
#define MAX_SLVMETER_NUM			10
#define MAX_PHASE_NUM		        3

#define MAX_HOURS_SECTOR            48   //Considering Peak Wave
#define MAX_DAYS_SECTOR             32
#define MAX_WEEKLY_SECTOR           7
#define MAX_YEARS_SECTOR            20 //2020 - 1900 -120

#define MAX_MPPT_GROUP_NUM          4
#define MAX_SINGLEPCS_RECT_NUM		8

// #define MAX_SINGLE_RECT_NUM			10

#define MAX_BATTCABINET_NUM			4

#define MAX_SUPPORTED_CLIENTS 		3

#define MAX_HUMITEMP_NUM			8
#define MAX_GASSENSOR_NUM			4
#define MAX_PT1000_NUM			    10

#define	CELL_BALANCE_MSG_NUM 		10 
#define	CELL_VOLT_MSG_NUM 			60 
#define	CELL_TEMP_MSG_NUM 			30 

#define	A_BALANCE_MSG_DATA_NUM 		64
#define	A_SAFE_MSG_DATA_NUM 		64
#define	A_VOLT_MSG_DATA_NUM 		4 
#define	A_AFEWORKMODE_MSG_DATA_NUM 	16	 

#ifdef IMMU3
	#define A_CABINET_CELL_MAX_NUM		260 //Max = Pack Num * Pack Cell Num
#else
	// #define A_CABINET_CELL_MAX_NUM		240 //Max = Pack Num * Pack Cell Num
	#define A_CABINET_CELL_MAX_NUM		416 //Max = Pack Num * Pack Cell Num
#endif

#define BATT_CELL_MAX_NUM			416 // d9 836

#define LONGALARM_NUM				14	 		

#define STAT_ECO_MODE				1

#define MAX_FILTER_POINT			30
#define MIN_FILTER_POINT			20

#define CAN_DATA_AREA_LEN			8
#define CAN_FRAME_LEN		        16

//--- For Remote Upgrade Bin File Size Define ---//
#define MIN_BINFILE_SIZE 	4*1024
#define MAX_BINFILE_SIZE	10*1024*1024

#define NUM_GRID_PHASE		3

#define MAX_CAN_MSGLEN      12*20
#define	MAX_CAN_CYCLETIMES  40
#define MAX_INVALID_FLOAT   (9999999.9f)
#define MIN_INVALID_FLOAT   (-9999999.9f)

#define MAX_TEMPERATURE_HUMIDITY_NUM	3
#define MAX_GAS_SENSOR_NUM	3

#define MAX_MSG13_SUBID_NUM 8

#define MAX_BSU_NUM 5  //Liquid cooling ESS has 5 BSU
#define MAX_IMEU3_NUM 2  //Liquid cooling ESS has 2 IMUE3
// LJH241111 PcsSCADA
#define MAX_SCADA_PCS_NUM     (1)

//add pv inverter
#define MAX_SN_LEN 32
#define MAX_VERSION_LEN 16
#define MAX_PV_NUM 16
#define MAX_VALUE_NUM 125 
#define MAX_PVInverter_NUM 10

//add kwhMeter Index deviceID start
#define KWHMETER_INDEX_START 406

#define MAX_BATT_GROUP_NUM 2  //YFY BMU02 has 2 battery groups

#define RPMSG_CELL_INFO			"11|A0|504|cell_info"			// 单体核间通信端口
#define RPMSG_CYCLE_INFO		 "8|A1|506|cycle_info"
#define RPMSG_FAO_INFO			 "2|A2|508|faq_info"
#define RPMSG_UPDATA_INFO		"13|A3|510|update_info"
#define RPMSG_PCS_INFO			"15|A4|512|pcs_info"			// PCS信息核间通信端口
#define RPMSG_PCS_CFG_INFO		"17|A5|514|pcscfg_info"


#define ADDR_R_KERNEL          50
#define ADDR_A_KERNEL_MIN     90
#define ADDR_A_KERNEL_MAX     99
#define FRAME_FLAG        0x0c

#define MAX_PSC_NUM 4
#define MAX_BUFFER_LNE 24
#define MAX_CLOUDSAVE_NUM 100
//Blob data type
typedef enum tagBlobType
{
	BLOB_TYPE_UPGRADE_CCU = 101,	//101--UPGRADE Cmd To CCU
	//BLOB_TYPE_LOACL_LIST,			//102--Local Authorization List(OCPP)
	BLOB_TYPE_NETWORKPARAMS = 103,		//103--Network params
	BLOB_TYPE_CHARGE_START_CMD = 104,	//104--Charge start cmd
	BLOB_TYPE_RUN_CHARGE_INFO = 105,	//105--Runtime Charging info
	BLOB_TYPE_RUN_BMS_INFO = 106,		//106--Runtime BMS info
	BLOB_TYPE_RESERVE_CMD = 107,		//107--Reserve cmd(usually issued by server), re-use as display reserver-info
	BLOB_TYPE_CHARGE_FEE_SET = 108,		//108--just for China local setting, for OCPP we can use DataTransfer.conf for setting
	//BLOB_TYPE_AUTHORIZE_CACHE,	//109--Authorization Cache
	BLOB_TYPE_CHARGE_STOP_CMD = 110,	//110--Charge stop cmd
	//BLOB_TYPE_UPDATE_LOCALLIST,	//111--Set/Update Local List(for OCPP)
	//BLOB_TYPE_UPDATE_AUTHSTATUS_ANDCACHE,	//112--Update Authorization status and/or Cache(for OCPP)
	BLOB_TYPE_OCPP_CONFIGURATIONS = 113,	//113--OCPP Configuration files
	BLOB_TYPE_SINGLE_CHARGEPROFILE = 114,//114--Notice! this is only one single profile
	BLOB_TYPE_METERVALUES,				//115--A single Meter value set
	BLOB_TYPE_UPDATE_CHARG_STAT_METER = 116, //116--Update charging/meter status(for OCPP)
	BLOB_TYPE_CHARGER_BOOTINFO = 117,	//117--Charger boot information(for OCPP BootNotification.req)
	BLOB_TYPE_UPDATE_FIRMWARE_CMD = 118,//118--Update firmware cmd(for OCPP)
	//BLOB_TYPE_CHAGER_STATUSINFO, //119--StatusNotific.req(for OCPP)
	BLOB_TYPE_CHARGER_SPECIFIC_SETTING = 120,	//120--Charge specific settings, e,g ,CCU address, Power limit enable Charger/CCU Specific Settings CCU专有设置
	BLOB_TYPE_UPLOAD_DIAGNOSTIC = 121,	//121--OCPP requires uploading Diagnostic info
	BLOB_TYPE_MANUALCTRL_RECT = 122,		//122--Rect Manual Ctrl Parameter
	//BLOB_TYPE_BATT_MGMT_PLAN = 123,			//123--Energy Storage Management Plan
	BLOB_TYPE_ES_RUNDATA = 124,			//124--Energy Storage Run Data
	BLOB_TYPE_ES_BACKEND_CFG = 125, //125--ES Backend Configs
	BLOB_TYPE_SINGLE_STBATT_RUNINFO = 126,	//126--Single stBatt Run Info
	BLOB_TYPE_GRP_PM_DATAINFO = 127,		//127--Group PM DATAINFO
	BLOB_TYPE_GROUP_STBATT_RUNINFO = 128,	//128--Group stBatt Run Info
	BLOB_TYPE_ES_STATISTICDATA = 129, //129--Energy Storage Statistic data(Daily)
	BLOB_TYPE_BRANCH_GRP_RECT_DATA = 130, //130
	BLOB_TYPE_SINGLE_RECT_DATAINFO = 131, //131
	BLOB_TYPE_BRANCH_NODE_CALC_DATA = 132, //132 -- Branch Node Calculate Data 
	BLOB_TYPE_RECTGRP_AUTOCTRLCMD = 133, //133 -- Rect Group Ctrl Cmd
	BLOB_TYPE_EMS_CONFIG_PLAN = 134, 		//134 -- EMS Management Plan
	BLOB_TYPE_IFYBMS_DATAINFO = 135, 	//135-- IFY BMS DataInfo
	BLOB_TYPE_BATTCAB_IMEU2_V20 = 136, 	//136--IMEU2 V20 DataInfo
	BLOB_TYPE_CCU2PCSCAN_MSG = 137, 	//CCU->PCS CAN Origin Msg.
	BLOB_TYPE_BMS_TESTMODE_SET = 138, //138 -- BMS Test Mode
	BLOB_TYPE_MANUALCTRL_RECT_SCADA = 139,		//139-- delete SOC item, similar to BLOB_TYPE_MANUALCTRL_RECT
	BLOB_TYPE_SEMS_CLIENTNFO = 140, //140 // LJH241202 SEMS ClientInfo A_BLOB_SEMS_CLIENTINFO
	BLOB_TYPE_SEMS2SlAVE_6330 = 141, //141 	 SEMS Send to Slave ctrl parameter
	BLOB_TYPE_PCS_ONGRID_CTRL = 142, //142 // CJJ for PcsSCADA to ctrl PCS
	BLOB_TYPE_PCS_OFFGRID_CTRL = 143, //143 // CJJ for PcsSCADA to ctrl PCS
	BLOB_TYPE_KEEPALIVEDINFO = 144,
	BLOB_TYPE_SEMS_CLIENTNFO_SUM = 145, // LJH250313 ClientSum A_BLOB_SEMS_CLIENTINFO_SUM
	BLOB_TYPE_SEMS_A6_USYS = 146, // LJH250612 planSector 增加EMS_LIMIT_SECTOR的参数 A_BLOB_SEMS_A6_USYS
	BLOB_TYPE_PV_INVERTER_DATA = 147,//光伏逆变器 pv inverter data
	BLOB_TYPE_PV_INVERTER_DATA_TOTAL = 148,//光伏逆变器 pv inverter Total data
	BLOB_TYPE_PV_SRTTING_BY_AEMS = 149,	//AEMS控制光伏参数
	// BLOB_TYPE_SEMS_CMD100_SUM = 150, // LJH250428 cmd reserved for cmd
	BLOB_TYPE_SEMS_CMD101 = 151, // KYA20250802 cmd151 A_BLOB_CMD151
	BLOB_TYPE_SEMS_CMD102 = 152, // KYA20250802 cmd152 A_BLOB_CMD152
	BLOB_TYPE_SEMS_CMD106 = 156, // KYA20250802 cmd156 A_BLOB_CMD156
	BLOB_TYPE_SEMS_CMD108 = 158, // KYA20250802 cmd158 A_BLOB_CMD158
	BLOB_TYPE_SEMS_CMD103 = 153,
	BLOB_TYPE_SEMS_CMD104 = 154,
	BLOB_TYPE_CMD155 = 155, // LJH250428 cmd155 A_BLOB_CMD155
	BLOB_TYPE_CMD157 = 157, // LJH250520 cmd157 A_BLOB_CMD157
	BLOB_TYPE_SEMS_CMD109 = 159,
	BLOB_TYPE_SEMS_CMD110 = 160,
	BLOB_TYPE_SEMS_CMD202 = 162,
	BLOB_TYPE_DEVINSTALLCONFIG = 163,
	BLOB_TYPE_SEMS_CMD902 = 164,// edit by Kuangyue 20251112
	BLOB_TYPE_DATASTORAGE = 165, 
	BLOB_TYPE_CELL_BARCODE = 166, // Cell Pack BSU Barcode Raw Data And ASCII Data
	BLOB_TYPE_ENERGY_STAT = 167, // Energy Power DOD Info (主索引52+55+56+62整合)
	BLOB_TYPE_STATUS_STAT = 168, // Status SOS Info (主索引50+51+53+54整合)
	BLOB_TYPE_DIAG_STAT = 169, // Feature Diag Info (主索引59+60+61+63+70整合)
	BLOB_TYPE_END
}BLOB_TYPE;

#define MAX_BLOB_TYPE (BLOB_TYPE_END - BLOB_TYPE_UPGRADE_CCU)

typedef struct tagHisAlarm
{
	int		nDevId;
	int		nAlarmId;
	ALARM_STATUS	status;
	float		fExParamVal;
	time_t	tmStart;
	time_t	tmEnd;
}EVENT_ALARM;

typedef enum tagBatt_Manufacturer
{
    BATT_CFG_NO_BATT = 0,
    BATT_MFG_ND_V33 ,
    BATT_MFG_NIFY_V33_500K , 	//Infy CATL V33 Protocal
    BATT_MFG_INFY_SELF_500K ,	//Infy Self Protocal
    BATT_MFG_IMTCU_V33_500K ,	//CATL BMS V33 Add IMTCU to 500K 
    BATT_MFG_FENGC_MODBUS ,		//Fengchao Modbus Protocal
	BATT_MFG_INFY_SELF_RPMSG,   // Infy Self Protocal with RPMSG
	BATT_MFG_TEST_NO_BATT,

}BATT_MANUFACTURER;

typedef enum tag_PMCtrlMode
{
    AUTO_CTRL_PM_MODE = 0,
    MANUAL_CTRL_PM_MODE,
    TESTMODE_CTRL_PM_MODE,
	TESTMODE_CTRL_PM_MODE_FIRST_CHARGE, //单簇测试模式下首充
	TESTMODE_TOW_CTRL_PM_MODE, // 对拖测试
	TESTMODE_ShortTime_CTRL_PM_MODE, //短时间测试
	
}STAT_PM_CTRL_MODE;

typedef enum tag_STSAutoManualOnOffGrid
{
	AUTO_ONOFFGRID_MODE = 0,
	MANUAL_ONOFFGRID_MODE,

}STS_AUTOMANUAL_ONOFFGRID;


typedef enum tagPcsStatus
{
	STAT_PCS_ON = 0, //A0:ON , A1: OFF 
	STAT_PCS_OFF,
}PCS_RUN_STATU;

typedef enum tagABUStatus
{
	STAT_ABU_ONGRID = 0, //0:ON-Grid(Cmd:A0) , 1: OFF-Grid (Cmd:A1)
	STAT_ABU_OFFGRID,
}ABU_ONOFFGIRD_STATU; 

typedef enum emDIState
{	
	CONNECTED = 0,
	NOT_CONNECTED
	
}DI_STATE;

typedef enum tagTestMachineStep
{
	STATE_FAIL = -1,   
	STATE_IDLE = 0,

    STATE_SELF_CHECK,
    STATE_FULLDSICHARGE_STEP1,
    STATE_SLEEP_STEP2,
    STATE_FULLCHARGING_STEP3, 
    STATE_SLEEP_STEP4,
    STATE_FULLDSICHARGE_STEP5,
    STATE_SLEEP_STEP6,
    STATE_CHARGE_HALF_STEP7,
    STATE_TEST_FINISHED,
    STATE_ON_PAUSED,

	STATE_MAX_ST

}STATE_MACHINE_STAT_TEST;

typedef enum tagChargeStartMode
{
	CHARGE_MODE_AUTO,
	CHARGE_MODE_BYTIME,
	CHARGE_MODE_BYKWH,
	CHARGE_MODE_BYMONEY,	
	CHARGE_MODE_BYSOC,	
	CHARGE_MODE_BYTIMER, //timer ,no time
	//Jimmy added 20191017 to record the V2B record
	//CHARGE_MODE_V2B = 11,
	//CHARGE_MODE_V2G
    CHARGE_MODE_BYCURRENT //added 20200326, for limiting current
}CHARGE_MODE;

typedef enum tagESSystem_Type
{
	ESSTYPE_UNINSTALL = 0,
	ESSTYPE_UDP_SERVER,
	ESSTYPE_UDP_CLIENT1,
	ESSTYPE_UDP_CLIENT2,
	ESSTYPE_UDP_CLIENT3,
	ESSTYPE_UDP_CLIENT4,
	ESSTYPE_UDP_CLIENT5,
	ESSTYPE_UDP_CLIENT6,
	ESSTYPE_UDP_CLIENT7,
	ESSTYPE_UDP_CLIENT8,
	ESSTYPE_UDP_CLIENT9,
 	
	ESSTYPE_MAX,
 }ESSYS_TYPE;

typedef enum tagV2XMode
{
    V2X_STATE_NOT = 0,
 	V2X_STATE_V2B,
    V2X_STATE_B2G,
    V2X_STATE_V2G
}V2X_MODE;

typedef enum tagSTS_OnOffGridStatus
{
	STATE_STS_ONGRID = 0,
	STATE_STS_OFFGRID,
	
}STS_ONOFFGRID_STATE;

typedef enum tagCtrlRectRunState
{
    RECT_STATE_RECFYING_START = 0,
 	RECT_STATE_RECTFING_CURR_ADJ,
    RECT_STATE_INVERT_START,
	RECT_STATE_INVERT_CURR_ADJ,
    RECT_STATE_STOP
}STATE_RECT_CTRL;

typedef enum tagGridMeterApplyScenery
{
	SCENERY_NOT_INSTALL = 0,
	SCENERY_ACTIVE_STANDBY,
	SCENERY_MULTI_BRANCH
	
}GRID_APPLY_SCENERY;

// LJH241202 SEMS add:MAX_SEMS_CLIENTS_NUM
typedef enum emFILTER_TYPE
{
	// LJH241202 SEMS
    FILTERTYPE_CALC_CHRGPOWER_GRP0 = 0,
	// FILTERTYPE_CALC_CHRGPOWER_GRP1 = 1,
	// FILTERTYPE_CALC_CHRGPOWER_GRP2 = 2,
	// FILTERTYPE_CALC_CHRGPOWER_GRP3 = 3,

	FILTERTYPE_CALC_DISCHRGPOWER_GRP0 = FILTERTYPE_CALC_CHRGPOWER_GRP0 + MAX_SEMS_CLIENTS_NUM,
	// FILTERTYPE_CALC_DISCHRGPOWER_GRP0 = 4,
	// FILTERTYPE_CALC_DISCHRGPOWER_GRP1 = 5,
	// FILTERTYPE_CALC_DISCHRGPOWER_GRP2 = 6,
	// FILTERTYPE_CALC_DISCHRGPOWER_GRP3 = 7,

	FILTERTYPE_CALC_QLMT_GRP0 = FILTERTYPE_CALC_DISCHRGPOWER_GRP0 + MAX_SEMS_CLIENTS_NUM,
	// FILTERTYPE_CALC_QLMT_GRP0 = 8,
	// FILTERTYPE_CALC_QLMT_GRP1 = 9,
	// FILTERTYPE_CALC_QLMT_GRP2 = 10,
	// FILTERTYPE_CALC_QLMT_GRP3 = 11,

	FILTERTYPE_REMAIN_CHRGING_LMT = FILTERTYPE_CALC_QLMT_GRP0 + MAX_SEMS_CLIENTS_NUM,
	// FILTERTYPE_REMAIN_CHRGING_LMT,
    FILTERTYPE_REMAIN_DISCHRG_PWR,
    FILTERTYPE_ACLOAD4CHG,
    FILTERTYPE_ACLOAD4DISCHGING,

	FILTERTYPE_REMAIN_REACTVPWR_LMT,

	FILTERTYPE_PID_ERROR,

    FILTERTYPE_MAX
}FILTER_TYPE;

typedef enum emStateManualCtrl
{
	STAT_MANUAL_STOP = 0,
	STAT_MANUAL_START_CHG,
	STAT_MANUAL_ADJ_CHG,
	STAT_MANUAL_START_DISCHG,
	STAT_MANUAL_ADJ_DISCHG,
	
}STATE_MANUL_CTRL;

typedef enum emPosOfAuxMeter
{
    AUXMETER_NOT_INSTALLED = 0,
    AUXMETER_POS_BRANCH_TRANSFER,
    AUXMETER_POS_CIVIL_ELECTRIC,
    AUXMETER_POS_ACLOAD,
	AUXMETER_MULTI_BANCH,
	// LJH250401 topology#6#7
	MULTI_BANCH_ESS_WITH_HVMETER,
	MULTI_BANCH_ESS_WITHOUT_HVMETER,
}POS_AUXMETER;

typedef enum emIMEU_TYPE
{
	IMEU_TYPE_NOT_INSTALLED = 0,
	IMEU_TYPE_IMEU2,
	IMEU_TYPE_IMEU3,
}IMEU_TYPE;

//Precision of Battety Cell temperature
typedef enum emBattCellTempPrecision
{
	TEMP_PRECISION_ONE = 0,		  // 1
	TEMP_PRECISION_ONE_TENTH, // 0.1

}BATT_CELL_TEMP_PRECISION;

typedef enum emAIR_TYPE
{
	AIR_NOT_INSTALLED = 0,
	INFYPOWER_AIR,
	TONGFEI_AIR,
	IMEU3_AIR,//edit by kuangyue 20250217
	XINMIAO_AIR,
}AIR_TYPE;

typedef enum emSORT_METHOD
{
	SORT_BY_SOC = 0,
	SORT_BY_CHGENG
}SORT_METHOD;

typedef enum tag_WorkModeReq
{
	CHARGE = 0,
	DISCHARGE,
	WORKMODE_MAX
}WORKMODE_REQ;

typedef enum tagFunctionCode
{
	FUNCODE_SYNC = 0,			// 0:对时帧
	FUNCODE_HEART_BEAT,			// 1:心跳帧
	FUNCODE_REQ_MODIFY,			// 2:请求修改
	FUNCODE_MODIFY_REPLY,		// 3:修改回复
	FUNCODE_MODIFY_CONFIRM,		// 4:修改确认
	FUNCODE_REQ_QUERY,			// 5:请求查询
	FUNCODE_QUERY_REPLY,		// 6:查询恢复
	FUNCODE_QUERY_CONFIRM,		// 7:查询确认
	FUNCODE_PROACTIVE_UDP,		// 8:主动更新
	FUNCODE_ERR_RESP,			// 9:错误应答
	FUNCODE_SIG_UPLOAD = 11,	// 11:单体上传 
	FUNCODE_PARAM_UPLOAD,		// 12:参数上传
	FUNCODE_UDP_INFO,			// 13:升级信息
	MAX_FUNCODE
}FUNCTION_CODE;

typedef union tagSTSRunStatus
{
	BYTE	byRunStatus;
	struct
	{
		//Byte0
		BYTE	Power_OnOff				: 1;//bit-0; 0-OFF,1-ON
		BYTE	OnOffGrid_State			: 1;//0-On-Grid, 1-Off-Grid
		BYTE	STS2Pcs_OnOffGridState	: 1;
		BYTE	FaultState				: 1;
		BYTE	Byte0_ResvBit4			: 1;
        BYTE	Byte0_ResvBit5			: 1;
		BYTE	ManualAuto_Mode			: 1;
		BYTE	NphaseRelay				: 1;
	}Status;

}STS_RUN_STATUS;

typedef union tagABURunStatus1
{
	BYTE	byRunStatus;
	struct
	{
 		BYTE 	SetOnOrOffGrid     		: 1;
		BYTE 	SysOnOrOff				: 1;
		BYTE	SysOnOrOffGridMode		: 1;
		BYTE	ExtBus_OnOrOffGrid		: 1;
		BYTE	SysSingleOrParallel 	: 1;
		BYTE	Byte1_Resv2				: 1;
		BYTE	Byte1_Resv1				: 1;
		BYTE	Slave0rMaster 			: 1; //0-Slave, 1-Master
	}Status;

}ABU_RUN_STATUS1;

typedef union tagPCSSysStatus
{
	BYTE	byPCSSysStatu;
	struct
	{
		//1112-Byte0
		BYTE	Byte0_Bit0to2 	: 3;//0-power Off,1-Run,2-Wait,3-Fault,4-Alm
		BYTE	OnOffGrid		: 1;//0-On-Grid,1-OffGrid
		BYTE	ChgOrDisChg		: 1;//0-Chg,1-DisChging
		BYTE	Byte0_Bit5		: 1;
		BYTE	Byte0_Bit6		: 1;
        BYTE	Byte0_Bit7		: 1;
	}SysStatu;

}PCS_SYS_STATUCODE;

typedef struct tagBlobManualCtrlRectParam
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;
	
	STATE_MANUL_CTRL 		emState;
    UINT 					nGrp;

	float					fGrpVolt;
	float					fGrpCurr;

	UINT					nStopSOC;//added by John For BAT Rack Test

	UINT					nAdjMode; //0-DC
	float					fActv_Pwr;
	float					fReactv_Pwr;
 
}A_BLOB_MANUALCTRL_RECT_CMD;


 typedef struct tagBlobUpradeCtrlCmd4CCU
{
	//must always put first!!!
	BLOB_TYPE			emBlobType;

	#define UPDT_TP_IMSU	0	//IMSU_APP.bin
	#define UPDT_TP_PFC		1	//U1-PFC:PCS
	#define UPDT_TP_DCDC	2	//U2-DCDC:PCS
	#define UPDT_TP_ABU_U1 	3
	#define UPDT_TP_ABU_U2 	4
	#define UPDT_TP_EU2_1 	5
	#define UPDT_TP_EU2_2 	6
	#define UPDT_TP_PFC_BOOT 	7
	#define UPDT_TP_DCDC_BOOT 	8
	#define UPDT_TP_MPPT_U1		9
	#define UPDT_TP_MPPT_U2		10
	#define UPDT_TP_BMS		11	//YFBMS.TXT

	int					emUpdtType;
	
	//#define RECT_UPDT_MODE_NORMAL_ONEBYONE	0
	//#define RECT_UPDT_MODE_FORCE_4SINGLE	1 //forcely upgrade single rectifier
	int					emUpdtMode;
	
	//DCDC info
	char				szDCDCFilePath[256];
	float				fDCDC_SWVer_4Updt;
	//PFC info
	char				szPFCFilePath[256];
	float				fPFC_SWVer_4Updt;
	//added by jimmy 20201104 for WEB
	BOOL				bIs4Web;

	//IMSU-D
	char				szIMSUFilePath[256];
	float				fIMSU_SWVer_4Updt;

	//IMEU2
	char				szIMEU2FilePath[256];
	float				fIMEU2_SWVer_4Updt;

	//YFBMS.txt
	char				szBMSFilePath[256];
	float				fBMS_SWVer_4Updt;

	int					iDevAddr;

	char                file_name[64]; //网页前端上传升级固件文件名
}A_BLOB_UPDT_INFO_4CCU;


typedef struct tagBlobStartChargeParam
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;

	//data 
	CHARGE_MODE				emChargeMode;
	VAR_VALUE				var_ChgModeParam_1;
	VAR_VALUE				var_ChgModeParam_2;
	char					szUserID[64];
	char					szParentID[64]; //for grouping function
	float					fCardLeftMoney;
#define USER_ATTRIBUTE_IN_WHITELIST	0x01
	UINT					nUserFlag;

	//BOOL				bAuxType_24V; //default false=12V

	//if OCPP server start, then transID is provided, or else ID would be generated by Local
	BOOL					bHasChargeSession_SN; //LCD do NOT generate this
	int						nSessionID;
	//for EiOCPP server who has string type of SN
	BOOL					bHas_StringChgSN;
	char					szChargeSN[64];
    //the start type: 0-charge, 1-V2B, 2-B2G, 3-V2G
    V2X_MODE				em_V2X_Status; //for identifying Charging/V2B/V2G
}A_BLOB_CHARGE_START_CMD;

 
 typedef struct tagMeterValueTable
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;

	time_t					tmRefresh;
	ESSYS_TYPE				emDataFrom;
	UINT 					nPriority[ESSTYPE_MAX];
	BYTE 					szIP[32];
	
	float 					fRealTime_ActivePower;
	float 					fPhA_ActivePower;
	float 					fPhB_ActivePower;
	float 					fPhC_ActivePower;

	float 					fReactivePower;
	float 					fPhA_ReactivePower;
	float 					fPhB_ReactivePower;
	float 					fPhC_ReactivePower;

	float 					fRealTime_kWh;
	float 					fRealTime_kVarh1;

	float 					fPhA_Volt;
	float 					fPhA_Curr;

	float 					fPhB_Volt;
	float 					fPhB_Curr;

	float 					fPhC_Volt;
	float 					fPhC_Curr;

	float 					fRealTime_kVarh2;

 	float 					fReverse_kWh;

 	float 					fAC_frequency;

	float 					fPmaxDemand;

	float 					fPower_Factor;
	float 					fPhA_PowerFactor;
	float 					fPhB_PowerFactor;
	float 					fPhC_PowerFactor;
	 
}A_BLOB_METER_VALUES;
 
 
 

typedef enum tagCharge_StopType
{
	CHARGE_STOPCLASS__NORMAL_STOP = 0,
	CHARGE_STOPCLASS_CHARGER_STOP,
	CHARGE_STOPCLASS_EV_STOP,
	CHARGE_STOPCLASS_CANCELED,
	CHARGE_STOPCLASS_OTHER

}CHARGE_STOPTYPE;

typedef enum tagChargePointErrCode
{
	CP_ERRCODE_ConnLockFailure,
	CP_ERRCODE_EVCommError,
	CP_ERRCODE_GroundFailure,
	CP_ERRCODE_HighTemperature,
	CP_ERRCODE_InternalError,
	CP_ERRCODE_LocalListConflict,
	CP_ERRCODE_NoError,
	CP_ERRCODE_OtherError,
	CP_ERRCODE_OverCurrFailure,
	CP_ERRCODE_OverVoltFailure,
	CP_ERRCODE_PowerMeterFailure,
	CP_ERRCODE_ReaderFailure,
	CP_ERRCODE_ReserFailure,
	CP_ERRCODE_UnderVoltFailure,
	CP_ERRCODE_WeakSignal
}CHARGEPOINT_ERRCODE;

typedef enum tagChargePointStatus
{
	CP_STATUS_Available,
	CP_STATUS_Preparing,
	CP_STATUS_Charging,
	CP_STATUS_SuspendedEVSE,
	CP_STATUS_SuspendedEV,
	CP_STATUS_Finishing,
	CP_STATUS_Reserved,
	CP_STATUS_Unavailable,
	CP_STATUS_Faulted
}CHARGEPOINT_STATUS;

typedef enum tagBattRunStatu
{
    STBATT_RUNSTAT_NOT_INSTALLED = 0,
    STBATT_RUNSTAT_COMM_FAIL,
    STBATT_RUNSTAT_SLEEP, //means contact open and not available
    STBATT_RUNSTAT_IDLE,
    STBATT_RUNSTAT_CHARGE,
    STBATT_RUNSTAT_DISCHARGE,
	STBATT_RUNSTAT_FULL_CHARGE,
	STBATT_RUNSTAT_FULL_DISCHARGE
}STBATT_RUN_STATU;

typedef enum tagBattAlmStatus
{
    STBATT_ALMSTAT_NOMAL = 0,
    STBATT_ALMSTAT_ALARMING,
    STBATT_ALMSTAT_PROTECT,
    STBATT_ALMSTAT_FAULT
}STBATT_ALARM_STATU;
typedef struct tagBlobChargePointStatusInfo
{
	//must always put first!!!
	BLOB_TYPE	emBlobType;

	//connector ID no need specify, because this structure is related to the device!
	CHARGEPOINT_ERRCODE		emChargePointErrcode;
	BOOL						bHasOptinalInfo; //do we have optional info
	char						szInfo[50]; //optional
	CHARGEPOINT_STATUS		emChargePointStatus;
	time_t					tmTimeStamp; //this is required for us!!!

	BOOL						bHasOptinalVerdorErrInfo;
	char						szVendorId[255];
	char						szVendorErrorCode[50];
}A_BLOB_CAHRGEPOINT_STATUSINFO; 

typedef struct tagNewInfo
{
#define NETPORT_STATUS_NOTINST		0
#define NETPORT_STATUS_DISCONN		1
#define NETPORT_STATUS_CONNECTED	2
	UINT		emNetPort_Status; //0--Not Installed, 1--Disconnected, 2--Connected
	ULONG		ulIpV4_IP;
	ULONG		ulIpV4_Mask;
	ULONG		ulIpV4_Gateway;
	ULONG		ulIpV4_DNSServer;
	//Only readable
	char		szMacAddr[6];

	char		szIPV6Info[128]; //reserved for future use
}NET_INFO;

typedef struct tagLTEModemInfo
{
	char		szIMSI[20];
	char		szICCID[32];
	char		szNetwork_Type[32]; //e.g, GSM, GPRS, CDMA, EVDO, EHRPD, UMTS, HSDPA, HSUPA, HSPA, HSPA+, LTE, TD-SCDMA
	//BOOL		bIsLogin_Network; //True means already login to network
	int			nSignal_dBm; //unit:dBm.-113dBm ~ -53dBm
	int			nSignal_Level; // <= -113dBm->0, <= -107dBm->1, <=-103dBm->2, <= -101dBm->3, <= - 91dBm->4, >-91dBm->5

	//Jimmy note 20221213: reuse this as the VPN authenticaton type
#define VPN_AUTHTYPE_NONE	0
#define VPN_AUTHTYPE_PAP	1
#define VPN_AUTHTYPE_CHAP	2
	BOOL		bIsManulSet_APN; //if false, will use the modem obtained default APN, else should be set mauanlly by user
	char		szAPN[64];
	char 		szAPN_User[64];
	char 		szAPN_Passwd[64];
	char 		szSN[64];

	// added by jinhuang 2023-11-28 sn、version
	char 	    szId[64];
	char 	    szSWVer[16];
}LTEMODEM_INFO;

#define LTEMODE_INSTTYPE_NOTINST	0
#define LTEMODE_INSTTYPE_ME3630		1
#define LTEMODE_EXTTTYPE_ZR2000		2
#define LTEMODE_INSTTYPE_EC200U		3
#define LTEMODE_INSTTYPE_EC200A		4
#define LTEMODE_INSTTYPE_EC200M		5

typedef struct tagBlobNetworkInfo
{
	//must always put first!!!
	BLOB_TYPE		emBlobType;
	// LJH241101 MU3 ETHNAME_FIRST:192.168.1.100 ETHNAME_SECOND:192.168.100.101
#ifdef IMMU3
	#define ETHNAME_FIRST   "eth1"
	#define ETHNAME_SECOND  "eth0"
#else
	#define ETHNAME_FIRST   "eth0"
	#define ETHNAME_SECOND  "eth1"
#endif

	//eth0, can be get/set
	BOOL			bDHCP_On_eth0;
	NET_INFO		netInfo_eht0;
	//3G-4G modem, ip only readable
	NET_INFO		netInfo_usb0;
	//IMMU2 A01 - dual wired net port
	BOOL			bDHCP_On_eth1;
	NET_INFO		netInfo_eht1;
	//modem extra info
	LTEMODEM_INFO	lteModemINfo;

	//added by Jimmy 20200703 for WLAN reachable check
	BOOL 			bIsWLAN_Reachable;

	//added by Jimmy 20201214: as LTEM modem has problem, so we disable it default
	UINT			bIsEnable_LTEModem; //0-disabled, 1-Internal LTE, 2-ZLWL-Router@ETH1, 3-ZLWL-Router@ETH1
}A_BLOB_NETWORKINFO;

typedef struct tagBlobKeepalivedInfo
{
	//must always put first!!!
	BLOB_TYPE		emBlobType;

	BOOL			bEnable;

	char 	    	szRouter_Id[64];

	BOOL			bState;    		//0-master 1-backup
	BOOL 			nInterface;		//0-eth0 1-eth1
	UINT 			nVirtual_Router_Id;
	UINT			nPriority;
	UINT			nAdvert_Int;
	BOOL 	    	bAuth_Type;    //0-PASS  1-AH
	char 	    	szAuth_Pass[64];
	char 	    	szVirtual_Ipaddress[64];

}A_BLOB_KEEPALIVEDINFO;

typedef enum tagFeeRateAtti
{
	FEERATE_SHARP = 0,
	FEERATE_PEAK,
	FEERATE_FLAT,
	FEERATE_VALLEY,

}FEERATE_ATTRI;

typedef struct tagFeeRateLimitSector
{
	time_t			tmStart;

	int				nInterval; //start + interval = end time
    
	FEERATE_ATTRI	emFeeRate;
	
	float			fPrice;
 
}FEERATE_LMT_SECTOR;

typedef struct tagChargeFeeSet
{
	//must always put first!!!
	BLOB_TYPE 				emBlobType;
	
	int 					nTotal_FeeRate_Sectors;

    FEERATE_LMT_SECTOR     	feeRateSectors[48];
 
}A_BLOB_CHARGE_FEESET;

typedef enum tagChargingProfilePurpose
{
	CP_MaxProfile,
	CP_DefaultProfile,
	CP_NormalProfile,
}CP_PURPOSE;

typedef enum tagChargingProfileKind
{
	CP_Kind_Absolute,
	CP_Kind_Recurring,
	CP_Kind_Relative
}CP_KIND;

typedef enum tagChargingProfileRecurencyKind
{
	CP_Recurrency_Daily,
	CP_Recurrency_Weekly
}CP_RECURRENCY_KIND;

typedef enum tagCP_CharingRateUnit
{
	CP_ChargingRateUnit_W,
	CP_ChargingRateUnit_A
}CP_CHARGINGRATE_UNIT;

typedef struct tagChargingSchedulePeriod
{
	int			nStartPeriod; //in seconds
	float		fLimit;
	int			nNumberPhases;

}CHARGINGSCHEDULE_PERIOD;

#define MAX_CP_CHARGINGPERIOD_NUMBDER	48		//now we support at most 48 points
typedef struct tagChargingSchedule
{
	int							nDuration; //in seconds
	_DATE_TIME					dtStartSchedule;
	CP_CHARGINGRATE_UNIT		emChargingRateUnit;
	CHARGINGSCHEDULE_PERIOD		charingSchedulePeriodList[MAX_CP_CHARGINGPERIOD_NUMBDER];
	int							nNumberCharingSchedulePeriod; //now use how many points
	float						fMinCharingRate;

}CHARGING_SCHEDULE;

typedef struct tagSingleChargingProfile
{
	//must always put first!!!
	BLOB_TYPE			emBlobType;

	int					nChargingProfile_ID;
	int					nTransaction_ID;
	int					nStackLevel; //for priority
	CP_PURPOSE			emChargingProfile_Purpose;
	CP_KIND				emChargingProfile_Kind;
	CP_RECURRENCY_KIND	emRecurrencyKind;
	_DATE_TIME			dtValidFrom;
	_DATE_TIME			dtValidTo;

	CHARGING_SCHEDULE	chargingSchedule;

}A_BLOB_SINGLE_CHARGINGPROFILE;

#define MAX_CHARGINGPROFILE_NUMBER		10
typedef struct tagChargingProfile
{
	//must always put first!!!
	BLOB_TYPE							emBlobType;

	A_BLOB_SINGLE_CHARGINGPROFILE		chargingProfiles[MAX_CHARGINGPROFILE_NUMBER];
	int									nChargingProfileNumber;

}A_BLOB_CHARGINGPROFILE;

 
 
typedef enum tagChargeMeterUpdateType
{
	CMUPDATE_StartTrans,
	CMUPDATE_StopTrans,
	CMUPDATE_Omited, //this reflect that this record should be omitted.
	CMUPDATE_SampleMeterVal,
	CMUPDATE_Pending202, //for ESiOCPP only

}CMUPTDATE_TYPE;
typedef struct tagUpdateChargingMeterStatus
{
	//must always put first!!!
	BLOB_TYPE		emBlobType;
#define NOTIFY_BY_OCPP_REPORTER		0
#define NOTIFY_BY_OTHER_REPORTER	1
	BYTE				emSenderType; //default should be OCPP, support 2 reported at a time

	CMUPTDATE_TYPE	emUpdateType;
	//for conn0 (historical record, need GunIdx and tmStart)
	int				nGunIdx; //1--A, 2--B...
	time_t			tmMeterVal_Sampled;
	time_t			tmChargeStart; //for identify historical charge record
	UINT			nTransactionID; //this needs to reflect real ID for Server!!!!
	BOOL			bHasString_ChgSN;
	char			szChargeSN[64];
	float			fBalance; //for money display only
}A_BLOB_UPDATE_CHARGEMETER_STAT;

typedef struct tagChargeBootOptionalItemMap
{
	BYTE		chargerPointSN : 1; //do not support
	BYTE		firmwareVer : 1;
	BYTE		iccid : 1;
	BYTE		imsi : 1;
	BYTE		meterSN : 1; //this is the main power meter
	BYTE		meterType: 1;
}BOOT_INFO_OPTIONS;
typedef struct tagChargeBootupInfo
{
	//must always put first!!!
	BLOB_TYPE	emBlobType;

	BOOT_INFO_OPTIONS	emBootInfoOptions;
	char		szChargerBoxSerialNum[25]; //do not support
	char		szChargePointModel[20];
	char		szChargePointSerualNuml[32]; // "serial_no"
	char		szChargePointVendor[20]; // "vendor"
	char		szFirmwareVer[50];
	char		szICCID[32];
	char		szIMSI[20];
	char		szMeterSerialNum[25];
	char		szMeterType[25];

	BYTE		emESSystemType; //ES system type: 0-ES with single batt, 1-ES with 2 batt, 255--No ES batt
	BYTE		byIMMU2Addr; //0-StandAlone, 255-Master, 1~254-Slave
	BYTE		byReserved[64]; //reserved for future updates
}A_BLOB_CHARGER_BOOTINFO;

typedef struct tagBlobUpdateFirmwareInfo
{
	//must always put first!!!
	BLOB_TYPE	emBlobType;

	int         nSemsIdx;
	char		szURL[255]; //"local" means from LCD SD card downloading
	int			nDownloadRetries; //0 means not present
	time_t		dtRetrieveDate;
	int			nRetryInterv; //0 means charger decide

}A_BLOB_UPDATEFIRMWARE_CMD;

typedef struct tagBlobUploadDiagnosticCmd
{
	//must always put first!!!
	BLOB_TYPE	emBlobType;

	char		szURL[255];
	int			nRetries; //0 means not present
	int			nRetrieveInterv; //0 means not present
	time_t		dtStartTime;
	time_t		dtStopTime;
	//diagnostic msg shall response a file name immediately, so OCPP generate this file name
	char		szDiagnosticFileName[64];

}A_BLOB_UPLOADDIAGN_CMD;


typedef union tagSysAttributesMap1
{
	USHORT	usFlag;
	struct
	{
		USHORT	EPOVoltLevel : 1;
		USHORT	DoorOpenEnable : 1;
		USHORT	SPDEnable : 1;
 		USHORT	ACGridInputMCB: 1;
		USHORT	ACLoadOutputMCB: 1;
		USHORT	Aux_MCBreaker: 1;
 		USHORT	AC_InputContactor: 1;
 		USHORT	PE_ConnectDetection: 1;
 		USHORT	BattCab_DI_VoltLevel: 1;
 		USHORT	Mppt_DI_VoltLevel: 1;
 		USHORT	N_LineContactorDI: 1;
		USHORT	Leakage_Curr_Cfg: 1;
		USHORT	Smoke_Sensor: 1;
		USHORT	Flooding_DI: 1;
		USHORT	Topple_DI: 1;
		USHORT	LowVoltMCBBreaker: 1;
	}Set;
}SYS_ATTRIBUTES_MAP1;

typedef union tagSysAttributesMap2
{
	USHORT	usFlag;
	struct
	{
		USHORT	isBattGrp1_Installed : 1;
		USHORT	isBattGrp2_Installed : 1;
		USHORT	isBattGrp3_Installed : 1;
 		USHORT	isBattGrp4_Installed : 1;
		USHORT	STS_Grp1_Installed : 1;
		USHORT	STS_Grp2_Installed : 1;
		USHORT	STS_Grp3_Installed : 1;
 		USHORT	STS_Grp4_Installed : 1;
		USHORT	IMEU2_Grp1_Installed : 1;
		USHORT	IMEU2_Grp2_Installed : 1;
		USHORT	BmsGrp1_CANInstalled : 1;
		USHORT	BmsGrp2_CANInstalled : 1;
		USHORT	BmsGrp3_CANInstalled : 1;
		USHORT	BmsGrp4_CANInstalled : 1;
	}Set;

}SYS_ATTRIBUTES_MAP2;


typedef union tagSysAttributesMap3
{
	USHORT	usFlag;
	struct
	{
		USHORT	BattGrp1_ForBidRun : 1;
		USHORT	BattGrp2_ForBidRun : 1;
		USHORT	BattGrp3_ForBidRun : 1;
 		USHORT	BattGrp4_ForBidRun : 1;
		USHORT	BattGrp1_InputContactor : 1;
		USHORT	BattGrp2_InputContactor : 1;
		USHORT	BattGrp3_InputContactor : 1;
 		USHORT	BattGrp4_InputContactor : 1;
		USHORT	BattGrp1_IsolationSwitch : 1;
		USHORT	BattGrp2_IsolationSwitch : 1;
		USHORT	BattGrp3_IsolationSwitch : 1;
 		USHORT	BattGrp4_IsolationSwitch : 1; 
 
	}Set;
}SYS_ATTRIBUTES_MAP3;

typedef union tagSysAttributesMap4
{
	USHORT	usFlag;
	struct
	{
		USHORT	PVGrp1_Contactor : 1;
		USHORT	PVGrp2_Contactor : 1;
		USHORT	PVGrp3_Contactor : 1;
 		USHORT	PVGrp4_Contactor : 1;
		USHORT	PVGrp1_SPD_Detection : 1;
		USHORT	PVGrp2_SPD_Detection : 1;
		USHORT	PVGrp3_SPD_Detection : 1;
 		USHORT	PVGrp4_SPD_Detection : 1;
		USHORT	PVGrp1_IsolationSwitch : 1;
		USHORT	PVGrp2_IsolationSwitch : 1;
		USHORT	PVGrp3_IsolationSwitch : 1;
 		USHORT	PVGrp4_IsolationSwitch : 1;
 
	}Set;
}SYS_ATTRIBUTES_MAP4;

typedef union tagSysAttributesMap5
{
	USHORT	usFlag;
	struct
	{
		USHORT	DCLaodGrp1_SPD : 1;
		USHORT	DCLaodGrp2_SPD : 1;
		USHORT	DCLaodGrp3_SPD : 1;
 		USHORT	DCLaodGrp4_SPD : 1;
		USHORT	DCLaodGrp1_IsolationSwitch : 1;
		USHORT	DCLaodGrp2_IsolationSwitch : 1;
		USHORT	DCLaodGrp3_IsolationSwitch : 1;
 		USHORT	DCLaodGrp4_IsolationSwitch : 1;
  		USHORT	ACFanOrDCFan : 1;//0-Default ACFan. 
 		USHORT	DCFan_DI_AlmLevel : 1;//0-DEfault Alm.

	}Set;
}SYS_ATTRIBUTES_MAP5;

typedef union tagSysAttributesMap6
{
	USHORT	usFlag;
	struct
	{
		USHORT	isIMIU1_Installed : 1;
		USHORT	isIMIU2_Installed : 1;
		USHORT	isIMIU3_Installed : 1;
 		USHORT	isIMIU4_Installed : 1;
		USHORT	isIMIU5_Installed : 1;
		USHORT	isIMIU6_Installed : 1;
		USHORT	isIMIU7_Installed : 1;
 		USHORT	isIMIU8_Installed : 1;
 		USHORT	isIMIU9_Installed : 1;
 		USHORT	isIMIU10_Installed : 1;

	}Set;
}SYS_ATTRIBUTES_MAP6;
/*
typedef union tagBranchCfgAttributes
{
	BYTE	byFlag;
	struct
	{
		BYTE	ACDC_GrpA_Exist : 1;
		BYTE	ACDC_GrpB_Exist : 1;
		BYTE	DCDC_GrpA_Exist : 1;
		BYTE	DCDC_GrpB_Exist : 1;
		BYTE	MPPT_GrpA_Exist : 1;
		BYTE	MPPT_GrpB_Exist : 1;

	}Set;
}BRANCH_CFG_ATTRIBUTES;
*/

typedef union tagInterSlvAttributes
{
	USHORT	usFlag;
	struct
	{
		USHORT	EPOVoltLevel : 2;
		USHORT	DoorOpenEnable : 2;
		USHORT	CRReaderEnable : 2;
	}Set;
}INTERSLV_ATTRIBUTES;

typedef enum tagACDCType
{
	ACDC_NONE = 0,
	ACDC_N2101_35KW,
	ACDC_N2002_62_5KW,
	ACDC_N2402_235KW,
	ACDC_MAX
} ACDC_TYPE;

typedef enum tagDCDCType
{
	DCDC_NONE = 0,
	DCDC_N2302_40KW,
	DCDC_MAX
} DCDC_TYPE;

typedef enum tagMPPTType
{
	MPPT_NONE = 0,
	MPPT_30KW,
	MPPT_MAX
} MPPT_TYPE;

#define MAX_PM_GRPNUM	10
typedef struct tagCCUBootupInfo
{
	int						nCCUAddress;
	char					szBarcode[24];
	char					szMacAaddress[16];
 	char					szSWVer[8];
	time_t					tmIMSU_RealTime;
	
	SYS_ATTRIBUTES_MAP1		sysAtributesMap1;
	SYS_ATTRIBUTES_MAP2		sysAtributesMap2;
	SYS_ATTRIBUTES_MAP3		sysAtributesMap3;

	SYS_ATTRIBUTES_MAP4		sysAtributesMap4;
	SYS_ATTRIBUTES_MAP5		sysAtributesMap5;
	SYS_ATTRIBUTES_MAP6		sysAtributesMap6;
				
 
	UINT					byFan1_SpdLevel; //used to set system fan speed level(half/full...)
 	UINT					byFan2_SpdLevel; //used to set system fan speed level(half/full...)
	UINT					byFan3_SpdLevel; //used to set system fan speed level(half/full...)



	UINT 					byNumGrp_1;
	UINT 					byNumGrp_2;
	UINT 					byNumGrp_3;
	UINT 					byNumGrp_4;
	UINT 					byNumGrp_5;
	UINT 					byNumGrp_6;
	UINT 					byNumGrp_7;
	UINT 					byNumGrp_8;
	UINT 					byNumGrp_9;
	UINT 					byNumGrp_10;
	UINT 					byNumGrp_11;	
	UINT 					byNumGrp_12;

	BYTE					byFan1_WorkMode_M;
	BYTE					byFan1_FanWorkMode_N;
	UINT					nFan1_OverTemp4Cool;
	UINT					nFan1_UltraTemp4Cool;
	
	BYTE					byFan2_WorkMode_M;
	BYTE					byFan2_FanWorkMode_N;
	UINT					nFan2_OverTemp4Cool;
	UINT					nFan2_UltraTemp4Cool;

	BYTE					byFan3_WorkMode_M;
	BYTE					byFan3_FanWorkMode_N;
	UINT					nFan3_OverTemp4Cool;  // 风扇起转点
	UINT					nFan3_UltraTemp4Cool; // 风扇全速运行点

	UINT					byPCSType; //0-N2001 22KW , 1-N2002-62.5KW 2-N2101(35KW), 3-N2402SP(235kW)
	
	UINT					byDCorAC_AdjMode;
	//reserved 32 bytes
	BYTE					byReserved[24];

	UINT					nStartupMode;
	UINT					nworkMode;
	UINT					nessMode;

}CCU_BOOTUP_INFO;

typedef struct tagBlobChargerSpecificSet
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;

	//CCU stored set
	CCU_BOOTUP_INFO			ccuSettings;


}A_BLOB_CHARGERSPECIFIC_SETTINGS;

#define MAKE_BLOB(a, tp)		(a.emBlobType = tp)

typedef enum tagESMgmtPlanRepeatType
{
	ESMGMT_REPEAT_DAILY = 0,
	ESMGMT_REPEAT_WEEKLY,
	ESMGMT_REPEAT_MONTHLY
}ES_MGMT_REPEATTYPE;

/*
typedef struct tagSTBATTLimitSector
{
	time_t			tmStart;
	int				nInterval; //start + interval = end time
	#define STBATT_MGMT_WORKMODE_AC_FIRST	0
	#define STBATT_MGMT_WORKMODE_BATT_FIRST	1
	int				emSTBatt_WorkMode;
	UINT			nSOC_Force_StartCharge; //> lmt, start charge
	UINT			nSOC_Force_StopCharge; //< lmt, stop charge	
 
}STBATT_LIMIT_SECTOR;


typedef struct tagAblobESMgmtPlan
{
	//must always put first!!!
	BLOB_TYPE						emBlobType;
	int 							nESMgmt_Ver;
	#define MAX_ESMGMT_LMT_SETOR	96
    int                             nTotal_STBattLmt_Sectors;
	STBATT_LIMIT_SECTOR				stBattLmtSectors[MAX_ESMGMT_LMT_SETOR];
	ES_MGMT_REPEATTYPE				emBattChgPlan_Repeat;

}A_BLOB_BATT_MGMT_PLAN;
*/

typedef enum tagSource_AC_WorkSatu
{
	SOURCE_AC_POWEROFF = 0, // 断电
	SOURCE_AC_IDLE,         // 空闲 means on but no engergy out
	SOURCE_AC_PENDING_OUT,  // 充电 export to STBatt or EV
	SOURCE_AC_PENDING_IN    // 放电 V2G or B2G
}SOURCE_AC_WORKSTATU;

typedef enum tagstBATT_ACDC_WorkStatu
{
    STBATT_ACDC_IDLE = 0,
    STBATT_ACDC_CHRG2STBATT,
    STBATT_ACDC_B2G
}STBATT_ACDC_WORKSTATU;

/*
typedef enum tagstBatt_WorkStatu
{
	STBATT_NOT_INSTALL = 0,
	STBATT_COMMFAIL = 1,
	STBATT_SLEEP = 2,
	ST_BATT_NORMAL = 3,
	ST_BATT_FULL_CHARGING = 4,
	ST_BATT_FULL_DISCHARGING = 5,
 
}STBATT_WORK_STATU;
*/

typedef enum tagstACDC_WorkStatu
{
    STATU_ACDC_IDLE = 0,
    STATU_ACDC_RECTIFY,
    STATU_ACDC_INVERTER,
	STATU_ACDC_STANDBY, //2024-01-06.PCS ON, But NotChg and Not Dischg.
	STATU_ACDC_MAX
}ACDC_WORKSTATU;

typedef enum tagDCDC_WorkStatu
{
    STATU_DCDC_IDLE = 0,
    STATU_DCDC_RECTIFY = 1,
  
}DCDC_WORKSTATU;

typedef enum tagMPPT_WorkStatu
{
    STATU_MPPT_IDLE = 0,
    STATU_MPPT_RECTIFY = 1,
  
}MPPT_WORKSTATU;

typedef enum tagEV_ACDC_WorkStatu
{
	EV_ACDC_NOT_INSTALL = 0,
	EV_ACDC_IDLE = 1,
	EV_ACDC_CHARGING2EV = 2,
	EV_ACDC_CHARGING2RING = 3,
	EV_ACDC_V2G = 4,
}EV_ACDC_WROK_STATU;

typedef enum tagEV_WorkStatu
{
	EV_NOT_CONNECTED = 0,
	EV_IN_CHARGING = 1,
	EV_IN_DISCHARGING = 2,

}EV_WORK_STATU;

typedef enum tagGunABMerger_Status
{
    GUN_MERGER_OFF = 0,
    GUN_MERGER_A2B = 1,
    GUN_MERGER_B2A = 2,
    GUN_MERGER_INVERT_A2B = 3,
    GUN_MERGER_INVERT_B2A = 4,
}GUN_MERGER_STAU;

typedef enum tagCCUState
{
	CCU_STATE_IDLE = 0,
	CCU_STATE_LOCKED,
	CCU_STATE_FAULTED,

	CCU_STATE_EXITING, //now exiting
}CCU_STATE;

typedef enum tagSysFailReason
{
	SYSFAIL_REASON_NOFAIL = 0,
	SYSFAIL_REASON_DOOR_OPENED,  
	SYSFAIL_REASON_EPO_PRESSED,  
	SYSFAIL_REASON_SPD_ALARM,  
	SYSFAIL_REASON_CCU_FAULTED,
	SYSFAIL_REASON_INPUTQF_DISCNECT,
	SYSFAIL_REASON_METER_COMFAIL,
	SYSFAIL_REASON_CCU_COMFAIL,

	SYSFAIL_REASON_ALLRECT_FAIL,  
	SYSFAIL_REASON_ALLRECT_COMMFAIL, 
	SYSFAIL_REASON_INSULATION_COMMFAIL, 
	SYSFAIL_REASON_INSULATION_DETECTALM, 
	SYSFAIL_REASON_MASTER_FAULT,
	SYSFAIL_REASON_BATTCABINET_FAULT,
	SYSFAIL_REASON_LEAKAGECURR_ALM,
	SYSFAIL_REASON_ABU_COMMFAIL,
	// LJH240919增加
	SYSFAIL_REASON_BMS_FAULT,

	SYSFAIL_REASON_OTHER,

}SYSTEM_FAILREASON;

// LJH240911 AvgPwrFirst
typedef enum tagPwrCtrlMode
{
	POW_CTRL_MODE_EFFICIENCY,
	POW_CTRL_MODE_AVGPWD
} POW_CTRL_MODE;

typedef union tagHMIDIStatus
{
	USHORT	usFlag;
	struct
	{
		USHORT 	DI_UserGridQF:1; //HT Off-Grid UserACSide QF
		USHORT 	DI_PwrCabQF1:1; //Cab MainQF
 		USHORT 	DI_PhaN_Contactor:1; //ABU Ctrl
		USHORT 	DI_PCSGrp1_Contactor:1;
		USHORT 	DI_PCSGrp2_Contactor:1;
		USHORT 	DI_PCSGrp3_Contactor:1;
		USHORT 	DI_PCSGrp4_Contactor:1;
		USHORT 	DI_ACLoadOutputMCBQF2:1;
	}Status;

}HMI_DI_STATUS;

typedef struct tagESRunInfo
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;

	//System Param
	CCU_STATE				emCCUState;
	SYSTEM_FAILREASON		emSystemFailReason;
	UINT					emCCUAutoOrManual;
	UINT 					emABUInstalled;
	UINT 					emEPO;
	//Grid ACMeter DataInfo
	SOURCE_AC_WORKSTATU		emGridAC_WorkStatus; //1--Normal, 0--Power off
 	float					fGrid_ACVolt_L1;
	float					fGrid_ACVolt_L2;
	float					fGrid_ACVolt_L3;
	float					fGrid_ACCurr_L1;
	float					fGrid_ACCurr_L2;
	float					fGrid_ACCurr_L3;

	//Local AC Meter info
	SOURCE_AC_WORKSTATU		emAC_WorkStatus; //1--Normal, 0--Power off
	float					fLocalAC_Power;
	float					fACVolt_L1;
	float					fACVolt_L2;
	float					fACVolt_L3;
	float					fACCurr_L1;
	float					fACCurr_L2;
	float					fACCurr_L3;
	
	//Calc ACLoad DataInfo
	SOURCE_AC_WORKSTATU		emACLoad_WorkStatus; //1--Normal, 0--Power off
	float					fACLoad_Power;
	float					fACLoad_ACVolt_L1;
	float					fACLoad_ACVolt_L2;
	float					fACLoad_ACVolt_L3;
	float					fACLoad_ACCurr_L1;
	float					fACLoad_ACCurr_L2;
	float					fACLoad_ACCurr_L3;

	//ST.Batt single info
	UINT					nBattCfgNum;	
    STBATT_RUN_STATU		emSTBatt_WorkStatu[MAX_SINGLE_STBATT_NUM]; //0--Not installed,1--commfail,2--sleep,3--normal,4--Charge,5--Discharge,6--Full Charge, 7--Full Discharge
	UINT					nSTBatt_SOC[MAX_SINGLE_STBATT_NUM];
	float					fSTBatt_OutVolt[MAX_SINGLE_STBATT_NUM];
	float					fSTBatt_OutCurr[MAX_SINGLE_STBATT_NUM];
	float					fSTBatt_Tmax[MAX_SINGLE_STBATT_NUM];

	//Batt ACDC info
	ACDC_WORKSTATU 			emACDC_WorkStatu[MAX_SINGLE_STBATT_NUM];;
	float					fACDC_OutVolt[MAX_SINGLE_STBATT_NUM];
	float					fACDC_OutCurr[MAX_SINGLE_STBATT_NUM];
	float 					fACDCOutRelayVolt[MAX_SINGLE_STBATT_NUM];
	float 					fActvPwr_PCSGrp[MAX_SINGLE_STBATT_NUM];
	float 					fQ_PCSGrp[MAX_SINGLE_STBATT_NUM];
	float 					fV_PCS3Phase[MAX_SINGLE_STBATT_NUM][NUM_GRID_PHASE];
	float 					fI_PCS3Phase[MAX_SINGLE_STBATT_NUM][NUM_GRID_PHASE];

	//DCDC info
	DCDC_WORKSTATU 			emDCDC_WorkStatu[MAX_SINGLE_STBATT_NUM];;
	float					fDCDC_OutVolt[MAX_SINGLE_STBATT_NUM];
	float					fDCDC_OutCurr[MAX_SINGLE_STBATT_NUM];
	
	//MPPT Info
	float					fTotalMPPT_Power;
	MPPT_WORKSTATU 			emMPPT_WorkStatu[MAX_SINGLE_STBATT_NUM];
	float					fMPPT_OutVolt[MAX_SINGLE_STBATT_NUM]; 
	float					fMPPT_OutCurr[MAX_SINGLE_STBATT_NUM]; 
	
	//DCLoad info
	float					fDCLoad_Volt;
	float					fDCLoad_Curr;

	//Param Config 
	BOOL					isACDC_Grp_Exist[MAX_SINGLE_STBATT_NUM];
	BOOL					isDCDC_Grp_Exist[MAX_SINGLE_STBATT_NUM];
	BOOL					isMPPT_Grp_Exist[MAX_SINGLE_STBATT_NUM];

	//------------------Statistic Data-------------------------//
	float					fCabAC_InputKWH_D;
	float					fCabAC_OutputKWH_D;
    float 					fBattGrpChrgKWH_D;
    float 					fBattGrpDisChrgKWH_D; 
    float 					fPVGrp_OutPutKWH_D; 
	float 					fACLoad_InputKWH_D;
	float					fGridLoad_InputKWH_D;

 	HMI_DI_STATUS			mapDIStatus;

	
	STS_RUN_STATUS 					mapSTSCode;
	ABU_RUN_STATUS1					mapAbuStatu1;
	//---20230226.added For MeterValue Topology---//
	POS_AUXMETER					emAuxPos;
	float 							f_PgridMeter;
	float 							f_PcabMeter;
	float 							f_PauxMeter;
	float 							f_PauxMeter_3;
	float 							f_PoffGridMeter;

	float 							f_PmaxGridDemand;

	float 							f_QgridMeter;
	float 							f_QcabMeter;
	float 							f_QauxMeter;
	float 							f_QauxMeter_3;
	float 							f_QoffGridMeter;
	
	float							fPacload4Chg;  
	float							fPacload4Dischg; 
	//float 						fAvg_Pacload4Chg; 
	float 							fAvg_Pacload4Dischg; 
	float 							fTotal_PessChgAvaliable;
	float 							fTotal_PessDischgLmt;
	int 							nTMSector;

	//------------Add 20230316 OnOrOffGrid--------------//
	float 							fPoffgrid_LoadMeter;
	float 							fQoff_LoadMeter;
	float 							fFreqoff_LoadMeter;
	float 							fVoffLoad[NUM_GRID_PHASE];

	float 							fSTS_InputVolt[NUM_GRID_PHASE];
 	float 							fSTS_InputCurt[NUM_GRID_PHASE];
	float 							fSTS_OutputVolt[NUM_GRID_PHASE];
	float 							fAC_Freq;
	float 							fAmbit_Temp;
	float							fPtotalActive;
	float							fPtotalReactive;
	float							fPtotalFactor;

 	//float 						fPCS_InputVolt[NUM_GRID_PHASE];
	PCS_SYS_STATUCODE				mapPCSSysStatu[MAX_SINGLE_STBATT_NUM];
	PCS_SYS_STATUCODE				mapPCSSysStatuBelong2[MAX_SINGLE_STBATT_NUM];

	// BYTE							On2OffState[MAX_SINGLE_STBATT_NUM];
	// BYTE							Off2OnState[MAX_SINGLE_STBATT_NUM];

	float 							f_PMeter[MAX_SLVMETER_NUM];
	float 							f_QMeter[MAX_SLVMETER_NUM];
	ESSYS_TYPE						emDataFrom[MAX_SLVMETER_NUM];
	BYTE							szIP[MAX_SLVMETER_NUM][32];
	BYTE							szRefreshTM[MAX_SLVMETER_NUM][32];

	//UINT 							nPriority[MAX_SLVMETER_NUM];
	// LJH240925 RunData
	BOOL							bEnterAnti_Reflux;
    BOOL 							bEnter_OverLoad;
	BOOL 							bEnter_OverLoadHV;
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
	BOOL 							bEnterAnti_Reflux_Phase;
	BOOL 							bEnter_OverLoad_Phase;
	BOOL 							bEnter_AutoChrg_SEMS;
	BOOL 							bEnter_AutoDisc_SEMS;
	float                           fAtRflx_kW; // 防逆流功率值
	float                           fAtOvl_kW;  // 防过载功率值
	// LJH250408 CtrlByREMS
	EMSCtrlMode                     emEMSCtrlMode_REMS;
	ACDC_WORKSTATU                  emACDC_WorkStatu_REMS;
	float                           fActvPwrReq_REMS;
	// LJH250726 东博上报MaxChg|DisChg都为0
	float                           fRatedPower; // 额定功率
	UINT                            nBattGrpPwrLmt; // 单组电池组功率限制
}A_BLOB_ES_RUN_DATA;

#define MAX_SG_GROUP_NUM 	3
#define MAX_SG_SIGLGRP_ELE	4
typedef struct tagESBackend_Config
{
	//must always put first!!! 624Bytes
	BLOB_TYPE				emBlobType;

	char					szCharger_ID[64];
	char					szServer_URL[255];
	UINT 					nProtect_SOC;

	float 					fSysACInput_PwrLmt; // 变压器可用功率(防过载)

	float 					fMax_Output2AC_PwrLmt; // abort, 输出到AC侧最大功率限制(限制电池通过ACDC逆变输出到AC侧的最大功率)
	float 					fOutput2AC_PwrRate; // 输出到AC侧跟随百分比(放电)-防逆流 PowerRate_Chrg // LJH250703 分开充放电保护/跟随比(放电)

 	float 					fMin_Output2AC_PwrLmt; // 输出到AC侧最小功率限制(慎)
	float 					fPreProt4RefluxAndOverLoad; // 预保护值(放电)-防逆流 fPreProt_Chrg

	float 					fUrgetAlterDischrgCurr; // 紧急转化放电模式电流 RESVERED:If Remote AC Current Over This Current Setting, Start Dischrg ASAP
	float 					fAdjustUrgetModeCurr; //RESERVED:if enter Urget Dischrge Mode, compare RealTimeCurr > (fUrgetAlterDischrgCurr-fAdjust4UrgetModeCurr)
	
	float 					fEnter_UrgetDischrgPwr; // 进入紧急放电模式功率点 Grid ACMeter Power > fEnter_UrgetDischrgPwr, Enter Urget Dischrge Mode
	float 					fExit_UrgetDischrgPwr; // 退出紧急放电模式功率点 ACLoad Req Power < fExit_UrgetDischrgPwr, Exit Urget Dischrge Mode
	

	float					fACLoadPwrProtcPoint; // 交流电网侧功率保护点

	BOOL					isACDC_Grp_Exist[MAX_SINGLE_STBATT_NUM];
	BOOL					isDCDC_Grp_Exist[MAX_SINGLE_STBATT_NUM];
	BOOL					isMPPT_Grp_Exist[MAX_SINGLE_STBATT_NUM];

	UINT					nCmd_101_Interval; 
	UINT					nCmd_102_Interval; 
	UINT					nCmd_103_Interval; 
	UINT					nCmd_104_Interval; 
	UINT					nCmd_105_Interval; 
	UINT					nCmd_106_Interval; 
	UINT					nCmd_107_Interval;
	UINT					nCmd_108_Interval; 
	UINT					nCmd_109_Interval; 
	UINT					nCmd_202_Interval; 
	UINT					nCmd_204_Interval;
	
	UINT					nCmd_110_Interval; //BMS Single Battry Info.

	float					fMin_ACLoadPwr_Start_Dischg;// 系统最小启动放电功率阈值 If ACload is Below This Value,System Not Start DisChg.


	float 					arrySGvalue[MAX_SG_GROUP_NUM][MAX_SG_SIGLGRP_ELE];

	float 					fQmaxOutput2ACLmt;
	float 					fPfactory_SettDemand; // 正向有功总最大需量
	// LJH240705 AitiOverLoad HVMeter
	float 					fSysACInput_PwrLmtHV; // 最大需量初始值(高压) (用于需量清0后可充电) old:fResv2 Transformer Available Power(HV)
	UINT					nCmd_206_Interval; // old:fResv3
	// LJH240911 AvgPwrFirst
	POW_CTRL_MODE           emPowCtrlMode; // old:float fResv4;
	float 					fPwrLmtMin_AvgPwrMode; // 同充同放最小功率 old: szResv[88]
	// LJH241007 AntiPhase
#define CHRGCURLMT_PHASE_VALID  (0.5f)
	float 					fPreProt4Reflux_Phase; // kW 预保护值(单相防逆流) old: szResv[84]
	float 					fMaxChrgCurLmt_Phase;   // A 单相电流最大值(单相防过载) old: szResv[84] fMaxChrgCurLmt_Phase
	
	// Jeffery20241204 Dynamic Grid Input Limit // 250523同步到站级EMS版本
	UINT					emDynamicGridInputLimit_Mode; // 是否使能动态网侧输入限制值 old:szResv[76]
	float					fFixed_GridInput_Limit; // kW Electra动态网侧输入限制值 old:szResv[76]

	// Jeffery 20250305 PCS Hot Stand-by
	UINT					emPCS_HotStandby_Mode; // PCS热备份模式提高响应速度 old:szResv[68]
	UINT					backend_reserved;
	// LJH250703 分开充放电保护/跟随比(充电) old:szResv[60]
	float 					PowerRate_Chrg; // 输出到AC侧跟随百分比(充电)-防过载
	float 					fPreProt_Chrg; // 预保护值(充电)-防过载
	
	// Jeffery 20250728 Electra需要把155/157命令单独配置
	UINT					nCmd_155_Interval; 
	UINT					nCmd_157_Interval; 

	BOOL					bEnableQuickResponseChannel;  //快速响应通道（用于快速防逆流）
	char					szResvbak[4];	//delete nFilterPointNum
	
	// Jeffery 20250921 Electra新增Q200报文
	UINT					nCmd_200_Interval;
	UINT                    nCmd_112_Interval; //PV old:szResv[32];
	UINT                    nCmd_114_Interval; //PCS old:szResv[28];
	UINT                    nCmd_115_Interval; //Summary old:szResv[24];	
	char					szResv[20];
}A_BLOB_ES_BACKEND_CFG; // !!!不可更改结构体!!! 624B

typedef union tagBattGrpAlarm
{
	BYTE	    szByAlarm[8];
	struct
	{
 		BYTE    AL_Over_Ucell : 2;
        BYTE    AL_Under_Ucell  : 2;
        BYTE    AL_Over_Tcell  : 2;
        BYTE    AL_Under_Tcell : 2;
        BYTE    AL_Over_U_HVS : 2;
        BYTE    AL_Under_U_HVS : 2;
        BYTE    AL_Over_T_HVS : 2;
        BYTE    AL_Under_T_HVS : 2;
        BYTE    AL_Over_CC : 2;
        BYTE    AL_Over_DC : 2;
        BYTE    AL_Diff_Tcell : 2;
        BYTE    AL_PreChFailThree : 2;
        BYTE    AL_Under_SOC : 2;
        BYTE    AL_HSD_Coil : 2;
        BYTE    AL_Diff_Ucell : 2;
        BYTE    AL_Err_Bal_Circuit : 2;
        BYTE    RESV1 : 1;
        BYTE    AL_Relay_OverLoad : 1;
        BYTE    AL_Pre_Relay : 1;
        BYTE    AL_Main_Relay_Pos : 1;
        BYTE    AL_Main_Relay_Neg : 1;
        BYTE    AL_Relay_Stick_Pos : 1;
        BYTE    AL_Relay_Stick_Neg : 1;
        BYTE    AL_Over_Cur : 1;
        BYTE    AL_Err_Mea_Ucell : 1;
        BYTE    AL_Err_Mea_Tcell : 1;
        BYTE    AL_Diff_Tvol : 1;
        BYTE    AL_Err_Mea_I_HVS : 1;
        BYTE    AL_MCANComFault : 1;
        BYTE    RESV2 : 1;
        BYTE    RESV3 : 1;
        BYTE    RESV4 : 1;
        BYTE    AL_Err_MCU : 1;
        BYTE    AL_Extre_Tvol : 1;
        BYTE    AL_Err_CCAN : 1;
        BYTE    AL_Err_SCAN : 1;
        BYTE    AL_Err_Mea_AllTcell : 1;
        BYTE    AL_Extre_T : 1;
        BYTE    AL_Extre_Ucell : 1;
        BYTE    AL_PowerSample_LEM : 1;
        BYTE    AL_PowerSample_CSC : 1;
        BYTE    AL_SBMU_PowerDown : 1;
        BYTE    AL_Err_Fog : 1;
        BYTE    AL_Err_HvCrcuitBreak : 1;
        BYTE    AL_Err_HvMsdFault : 1;
        BYTE    AL_Err_Fan1Fault : 1;
        BYTE    AL_Err_Fan2Fault : 1;
        BYTE    AL_Err_Relay_STB : 1;

	}Alarm;

}BATT_GRP_ALARM;

typedef enum emYFY_IsolationRes
{
	STAT_AUTO_DETECTION = 0,
	STAT_ON_DETECTING ,
	STAT_ON_STOPPING ,
	STAT_ON_FINISHED ,

}YFY_ISOLATION_RESULT;

typedef enum emISOTrigStat
{
    TRIG_NO_TRIGGER = -1,

    TRIG_AUTO_DETECTION = 0,
    TRIG_START_DETECTING,
    TRIG_STOP_DETECTION,

}TRIGGER_ISO_STATUS;

typedef enum emStartChgReason
{
	REASON_CHG_ALLOW_START = 0,
	REASON_CHG_BMS_FORBIDDEN,
	REASON_CHG_BMS_ISOPENDING,
	REASON_CHG_LASTSTARTTM_SHORT,
	REASON_CHG_LASTSTOPTM_SHORT,
	REASON_CHG_AVGPWR_BELOWALLOW,
	REASON_CHG_CALCPWR_LITTLE,
	REASON_CHG_MAXCURR_LITTLE,
	REASON_CHG_NOTCHGMODE,
	REASON_CHG_EMSPLAN_NOTALLOW,


}REASON_CHARGE_START;

typedef enum tagStartDisChargingReason
{
	REASON_DISCHG_ALLOW_ON = 0,
 	REASON_DISCHG_BMS_FORBIDDEN,
	REASON_DISCHG_BMS_ISOPENDING,
	REASON_DISCHG_LASTSTARTTM_SHORT,
	REASON_DISCHG_LASTSTOPTM_SHORT,
	REASON_DISCHG_AVGPWR_BELOWALLOW,
	REASON_DISCHG_CALCPWR_LITTLE,
	REASON_DISCHG_MAXCURR_LITTLE,
	REASON_DISCHG_NOTDISCHGMODE,
	REASON_DISCHG_HAD_ONDISCHG,
	REASON_DISCHG_ENDBY_STOPSOC,

}REASON_DISCHGING_ON;

//------------ Added By John For BMS StateMachine Stuts Show 20240119 -----------//
typedef enum tagBMSMgmtStatus
{
	STAT_MGMT_IDLE = 0,
 	STAT_MGMT_WAITCHG,
	STAT_MGMT_CHGING,
	STAT_MGMT_DISCHG,
	STAT_MGMT_AUTOOFFGRID,
	STAT_MGMT_FAULT,
	STAT_MGMT_MANUALCTRL,

}STAT_BMSMGMT_STATUS;

typedef enum tagBMSMgmtPcsCtrlCmd
{
	CMD_MGMT_STOP = 0,
 	CMD_MGMT_CHRG_ON,
	CMD_MGMT_DISCHG_ON,
	CMD_MGMT_CHRG_ADJ,
	CMD_MGMT_DISCHG_ADJ,
	CMD_MGMT_DISCHG_STANDBY,
	CMD_MGMT_CHRG_STOP,
	CMD_MGMT_DISCHG_STOP,

}CMD_BMSMGMT_PCSCTRL;

typedef enum tagExecCmdReason
{
	REASON_FAULT_ALARM = 0,
 	REASON_CHG_ALLOW,
	REASON_DISCHG_ALLOW,
	REASON_BMS_SLEEP,
	REASON_PCHRGAVG_LOW,
	REASON_PDISCHGAVG_LOW,
	REASON_BMSFORBIDDEN_CHRG,
	REASON_BMSFORBIDDEN_DISCHG,

}REASON_BMSMGMT_EXEC;
//------------------------------------ End --------------------------------------//

typedef struct tagSingle_STBattDataInfo
{
	BLOB_TYPE			emBlobType;

	UINT				emPowerONStatu;
	
	STBATT_RUN_STATU    emBattStatus_Run;
	STBATT_RUN_STATU    emLast_BattStatus_Run;

	STBATT_ALARM_STATU  emBattStatus_Alm;
	UINT                nSOC;
	int					nSOH;

	float               fPackTotalVolt;
    float               fPackTotalCurr;
	
    float             	fAllow_LmtChargeVolt;
    float               fAllow_LmtDisChargeVolt;
    float               fAllow_MaxChargeCurr;
    float               fAllow_MaxDisChargeCurr;
    float               fAllow_MaxChargePow;
    float               fAllow_MaxDisChargePow;

    float               fMax_CellVolt;
    float               fMin_CellVolt; 
	float				fAvg_CellVolt;

    float               fMax_CellTemp;
    float               fMin_CellTemp;
	float 				fAvg_CellTemp;

	float 				fChrg_Energy;
	float 				fDisChrging_Energy;
	float				fSingleChrgCapacity; //单次充电容量
	float				fSingleDischrgCapacity; //单次放电容量
	float				AccuChrgCapacity; //累计充电容量
	float				fAccuDischrgCapacity; //累计放电容量
	float				fAccuChrgEnergy; //累计充电能量
	float				fAccuDischrgEnergy; //累计放电能量
	
	//---Infy BMS ---//
	float				fTotal_RateCapacity;//Reality Capacity AH

	float				fAdd_ChgCapacity; 	//Added Charge Capacity AH
	float				fAdd_DischgCapacity;//Added DisCharge Capacity AH

 	float				fAdd_ChgEnergy; 	//Added Charge Energy KWH
	float				fAdd_DischgEnergy; 	//Added DisCharge Energy KWH

	float				fInsulation_P_R;
	float				fInsulation_N_R;

	float				fOutsideVolt;
	float 				fNegativeBusVolt;
	USHORT 				usBattWorkState;

	//-------End-----//


    USHORT              nLifeCycleTimes;
	USHORT				nNow_DOD;
	//S2A Alarm
	BYTE				szS2A_AlarmInfo[36];
	BYTE				szInfy_AlarmInfo[36];
	BYTE				szInfy_FaultInfo[36];
	BYTE				szInfy_GPIOInfo[36];
 	//S2A_SUMDATA3 
	USHORT 				uiBAT_Ucell_Max_CSC;
	USHORT				uiBAT_Ucell_Max_CELL;
	USHORT				uiBAT_Ucell_Min_CSC;
	USHORT				uiBAT_Ucell_Min_CELL;

	//float				fBAT_Ucell_Max;
	//float				fBAT_Ucell_Min;

	//S2A_SUMDATA4
	//float				fBAT_Ucell_Avg;
	//int 				nBAT_T_Max;
	//int				nBAT_T_Min;
	//int 				nBAT_T_Avg;

	USHORT 				nBAT_T_Max_CSC;
	USHORT 				nBAT_T_Min_CSC;
	USHORT 				uiBAT_T_Max_CELL;
	USHORT		 		uiBAT_T_Min_CELL;

	BOOL				bIsBatt_ChargeForbidden;
	BOOL 				bIsBatt_DisChargeForbidden;

	BYTE				byBattAddr;
	float				fVersion;
	
	UINT 				uiBattCabDevAlm;
	BATT_GRP_ALARM		unioBattGrpAlm;

	BYTE				szFault[CAN_DATA_AREA_LEN];
	BYTE				szAlarm[CAN_DATA_AREA_LEN];
	BYTE			 	szDIDOStatu[CAN_DATA_AREA_LEN];

	//20220515
	float 				fChg_Start_MaxCellTemp;
	float 				fChg_Start_MinCellTemp;	
	float 				fDischg_Start_MaxCellTemp;
	float 				fDischg_Start_MinCellTemp;

	//Added Iso Detection Logic For BMS. By John 2022-08-23.
	YFY_ISOLATION_RESULT	emIsoResult;
	TRIGGER_ISO_STATUS		emTrigIsoStatus;
	
	REASON_CHARGE_START		emReason_ChgStart;
	REASON_DISCHGING_ON		emReason_DisChgON;

	STAT_BMSMGMT_STATUS		emMgmtStatus;
	CMD_BMSMGMT_PCSCTRL		emMgmtPcsCtrlCmd;
	REASON_BMSMGMT_EXEC		emMgmtExecReason;

	UINT					uiPermissionLevel;

}A_BLOB_SINGLE_STBATT_RUNINFO;

typedef struct tagSTBattGroup_RunInfo
{
	BLOB_TYPE			emBlobType;
	STBATT_RUN_STATU	emBattGrpRunStatus;
	STBATT_ALARM_STATU  emBattGrpAlmStatus;
	UINT				nTotalOnlineBatt;
	UINT				nTotalInUseBatt; //excluded those who are faluted

	float				fBattBusVolt; //from Batt Meter or the max of all batt
	float				fBattBusCurr; //ON_Charging is Positive, ON_DisCharging is Negative

	float				fAvgSOC;
	float				fMaxSOC;
	float				fMinSOC;
	float				fAvgSOH;
	float				fMaxSOH;
	float				fMinSOH;

	float             	fAll_Allow_LmtChargeVolt;
    float               fAll_Allow_LmtDisChargeVolt;
    float               fAll_Allow_MaxChargeCurr;
    float               fAll_Allow_MaxDisChargeCurr;

	double              fAll_ChargedEnergy; //get from Batt meter or the sum of all batt
    double              fAll_DisChargedEnergy; //get from Batt meter or the sum of all batt

	BOOL				bIsBatt_ChargeForbidden;
	BOOL 				bIsBatt_DisChargeForbidden;

	//for energy dispatch purpose
	int					nCurr_StBatt_WorkMode; //ST-Batt/AC prio

}A_BLOB_GROUP_STBATT_RUNINFO;


typedef enum tagCCU_Ctrl_CmdType
{
    //CTRL_CCU_STOP_DISCHARGE_4STBatt = 0, //emergency notice CCU to stop using batt
    //AC-DC charge to STBatt ctrl
    CTRL_CCU_START_CHARGE_4STBatt = 0,
    CTRL_CCU_ADJUST_CHARGEPOWER_4STBatt,
	CTRL_CCU_STOP_CHARGE_4STBatt,
	//following is for V2B
    //CTRL_CCU_V2B_START,
    //CTRL_CCU_V2B_ADJUST_CURRENT,
    //CTRL_CCU_V2B_STOP,
	// //following is for V2G
    //CTRL_CCU_V2G_START,
    //CTRL_CCU_V2G_ADJUST_CURRENT,
    //CTRL_CCU_V2G_STOP,
	//following is for B2G
	CTRL_CCU_B2G_START,
	CTRL_CCU_B2G_STOP,
}CCU_CTRL_CMDTYPE;

typedef struct tagBlobstBattChargeCtrlParam
{
	//must always put first!!!
	BLOB_TYPE			emBlobType;
	
	//int				nSTBattGrp; //reserved for future
	CCU_CTRL_CMDTYPE	emCtrl_Type;

	float				fChargeVoltLmt;
	float				fChargeCurrLmt;

 
}A_BLOB_STBATT_CHARGE_CTRL_CMD;


typedef struct tagTimeVariantFloatValue
{
	time_t		tmTimeStamp;
	float		fValue;
}A_TIMEVAR_FLOATVALUE; // !!!不可更改结构体!!!


#define MAX_TIME_VARIANT_VAL_ITEMS	100
typedef struct tagES_24HourStatData
{
	//24 Hour Sector Remote AC RealTime Reading 
	float                	fAC_CurrKWH_Reading[MAX_HOURS_SECTOR];//电表H 实时电量

    //24 Hours Sector Remote AC Cost KWH
    float                	fAC_CostKWH_Sector[MAX_HOURS_SECTOR];//电表H实时电量相减  增量

	//24 Hours Org ACLoad Cost = Remote AC Meter Total Cost KWH - Local AC Meter Pos KWH + Local AC Meter Neg KWH
	float 					fOrgACLoad_CostKWH_Sector[MAX_HOURS_SECTOR];//用户负载消耗电量

	//24 Hours Sector Remote AC Reverse Output KWH
    float                	fAC_RevrtOutPutKWH_Sector[MAX_HOURS_SECTOR]; //电表H 反向有功总电能相减  增量

	//24 Hour Sector Local AC Meter Input KWH 
	float                	fLocalAC_INPUTKWH_Sector[MAX_HOURS_SECTOR];//计算柜内表正向消耗电量  增量

	//24 Hour Sector Local AC Meter Output KWH 
	float                	fLocalAC_OUTPUTKWH_Sector[MAX_HOURS_SECTOR];//计算柜内表反向消耗电量  增量

	//24 Hour Sector  DC Load Cost KWH
	float					fDCLoad_CostKWH_Sector[MAX_HOURS_SECTOR]; 					

    //24 Hours Sector MPPT Generate KWH
    float                	fMppt_GKWH_Sector[MAX_MPPT_GROUP_NUM][MAX_HOURS_SECTOR];

    //24 Hours Sector Battery Grp Charged KWH
    float                	fBatt_ChrgKWH_Sector[MAX_SINGLE_STBATT_NUM][MAX_HOURS_SECTOR];

    //24 Hours Sector Battery Grp DisCharged KWH
    float                	fBatt_DisChrgKWH_Sector[MAX_SINGLE_STBATT_NUM][MAX_HOURS_SECTOR];

	//Just For Resv
	float					fResv1_KWH_Sector[MAX_HOURS_SECTOR];

	//24 Hour Sector off-Grid Load Meter Total cost KWH 
	float					fOffGridLoad_CostKWH_Sector[MAX_HOURS_SECTOR]; //计算离网负载电表消耗电量

	//SOC For STBatt
	int  					nSOC_STBatt_TotalPoint[MAX_SINGLE_STBATT_NUM];
	A_TIMEVAR_FLOATVALUE 	soc_RT_STBatt[MAX_SINGLE_STBATT_NUM][MAX_TIME_VARIANT_VAL_ITEMS];

	//24 hours Sector Battery Grp Temperature
	float  					fBattMaxTemp[MAX_SINGLE_STBATT_NUM][MAX_HOURS_SECTOR];

	float  					fBattMinTemp[MAX_SINGLE_STBATT_NUM][MAX_HOURS_SECTOR];

	float  					fBattAvgTemp[MAX_SINGLE_STBATT_NUM][MAX_HOURS_SECTOR];

	float  					fBattInWaterTemp[MAX_SINGLE_STBATT_NUM][MAX_HOURS_SECTOR];

	float  					fBattOutWaterTemp[MAX_SINGLE_STBATT_NUM][MAX_HOURS_SECTOR];

}ES_24HOUR_STAT_DATA; // !!!不可更改结构体!!!


typedef struct tagES_WeekStatData
{
	time_t					tmTimeStamp_W[MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];

	//Weekly 24 Hours Sector AC Meter RealTime Reading 
	float 					fACRealTimeKWH_W[MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];

    //Weekly 24 Hours Sector Remote AC Meter Cost
    float 					fACTotalCostKWH_W[MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];

	//Weekly 24 Hours sector Org ACLoad Cost(Not Include Backup Sys Self Chrged & DisChrged KWH) = 
	// Remote AC Meter Total Cost - Local AC Meter Pos KWH + Local AC Meter Neg Kwh
	float 					fOrgACLoad_CostKWH_W[MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];

	//Weekly 24 Hours sector Remote AC Reverse Outout KWH
    float       			fAC_RevrtOutPutKWH_W[MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];
	
	//Weekly 24 Hours sector Local AC Meter Input KWH 
	float                	fLocalAC_INPUTKWH_W[MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];

	//Weekly 24 Hours sector Local AC Meter Output KWH 
	float                	fLocalAC_OUTPUTKWH_W[MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];

	//Weekly 24 Hours sector DC Load Cost KWH
	float					fDCLoad_CostKWH_W[MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];		

    //Weekly 24 Hours sector MPPT Generate KWH  
    float 					fMpptTotalGKWH_W[MAX_MPPT_GROUP_NUM][MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];

    //Weekly 24 Hours sector Battery Grp Chrged KWH
    float 					fBattTotalChrgKWH_W[MAX_SINGLE_STBATT_NUM][MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];

    //Weekly 24 Hours sector Battery Grp DisChrged KWH
    float					fBattTotalDisChrgKWH_W[MAX_SINGLE_STBATT_NUM][MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];
	
	float					fResv1_KWH_W[MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];

	float					fResv2_KWH_W[MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];

	//Weekly 24 hours sector Battery Grp Temperature
	float  					fBattMaxTemp_W[MAX_SINGLE_STBATT_NUM][MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];

	float  					fBattMinTemp_W[MAX_SINGLE_STBATT_NUM][MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];

	float  					fBattAvgTemp_W[MAX_SINGLE_STBATT_NUM][MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];

	float  					fBattInWaterTemp_W[MAX_SINGLE_STBATT_NUM][MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];

	float  					fBattOutWaterTemp_W[MAX_SINGLE_STBATT_NUM][MAX_WEEKLY_SECTOR][MAX_HOURS_SECTOR];

}ES_WEEKLY_STAT_DATA; // !!!不可更改结构体!!!


typedef struct tagES_365DayStatData
{
	time_t			tmTimeStamp_D[MAX_DAYS_SECTOR];

	//365 Days Remote AC Meter RealTime Reading
	float 			fAC_CurrKWH_Reading[MAX_DAYS_SECTOR];

    //365 Days Remote AC Meter Total Cost KWH
    float 			fACTotalCostKWH_D[MAX_DAYS_SECTOR];

	//365 Days Org ACLoad Cost KWH = Remote AC Meter Cost - Local Ac Meter Pos + Local AC Meter Neg
	float 			fOrgACLoad_CostKWH_D[MAX_DAYS_SECTOR];

	//365 Days Remote AC Reverse Output KWH
	float 			fAC_RevrtOutPutKWH_D[MAX_DAYS_SECTOR];

	//365 Days Local AC Meter Input KWH 
	float           fLocalAC_INPUTKWH_D[MAX_DAYS_SECTOR];

	//365 Days Local AC Meter Output KWH 
	float           fLocalAC_OUTPUTKWH_D[MAX_DAYS_SECTOR];

	//365 Days DC Load Cost KWH
	float			fDCLoad_CostKWH_D[MAX_DAYS_SECTOR];	

    //365 Days MPPt Generate KWH
    float 			fMpptTotalGKWH_D[MAX_MPPT_GROUP_NUM][MAX_DAYS_SECTOR];

    //365 Days Battery Grp Chrged KWH
    float 			fBattTotalChrgKWH_D[MAX_SINGLE_STBATT_NUM][MAX_DAYS_SECTOR];

    //365 Days Battery Grp DisChrged KWH
    float 			fBattTotalDisChrgKWH_D[MAX_SINGLE_STBATT_NUM][MAX_DAYS_SECTOR];

	//365 Days Battery Grp Chrged Times
	float 			fBattChrgTimes_D[MAX_SINGLE_STBATT_NUM][MAX_DAYS_SECTOR];

    //365 Days Battery Grp DisChrged Times
    float 			fBattDisChrgTimes_D[MAX_SINGLE_STBATT_NUM][MAX_DAYS_SECTOR];


	//365 Days Peak Flat Valley Chrged/DisChrged KWH
	float			fLocalAC_Sharp_InputKWH_D[MAX_DAYS_SECTOR];
	float			fLocalAC_Peak_InputKWH_D[MAX_DAYS_SECTOR];
	float			fLocalAC_Flat_InputKWH_D[MAX_DAYS_SECTOR];
	float			fLocalAC_Valley_InputKWH_D[MAX_DAYS_SECTOR];

	float			fLocalAC_Sharp_OutPutKWH_D[MAX_DAYS_SECTOR];
	float			fLocalAC_Peak_OutPutKWH_D[MAX_DAYS_SECTOR];
	float			fLocalAC_Flat_OutputKWH_D[MAX_DAYS_SECTOR];
	float			fLocalAC_Valley_OutputKWH_D[MAX_DAYS_SECTOR];

	float 			fOrgACLoad_Sharp_CostKWH_D[MAX_DAYS_SECTOR];
	float 			fOrgACLoad_Peak_CostKWH_D[MAX_DAYS_SECTOR];
	float 			fOrgACLoad_Flat_CostKWH_D[MAX_DAYS_SECTOR];
	float 			fOrgACLoad_Valley_CostKWH_D[MAX_DAYS_SECTOR];

	float 			fMppt_Sharp_GKWH_D[MAX_MPPT_GROUP_NUM][MAX_DAYS_SECTOR];
	float 			fMppt_Peak_GKWH_D[MAX_MPPT_GROUP_NUM][MAX_DAYS_SECTOR];
	float 			fMppt_Flat_GKWH_D[MAX_MPPT_GROUP_NUM][MAX_DAYS_SECTOR];
	float 			fMppt_Valley_GKWH_D[MAX_MPPT_GROUP_NUM][MAX_DAYS_SECTOR];


	float			fResv1_KWH_D[MAX_DAYS_SECTOR];

	//365 Days Off-Grid Load Meter Cost KWH
	float 			fOffGridLoad_CostKWH_D[MAX_DAYS_SECTOR];

	BOOL			bReportedOK[MAX_DAYS_SECTOR];

}ES_365DAY_STAT_DATA; // !!!不可更改结构体!!!

typedef struct tagES_YearStatData
{
	time_t		tmTimeStamp_Y[MAX_YEARS_SECTOR];

	//Remote AC Meter Realtime Reading KWH
	float 		fAC_CurrKWH_Reading[MAX_YEARS_SECTOR];

	//Yearly Remote AC Meter Total Cost KWH
    float 		fACTotalCostKWH_Y[MAX_YEARS_SECTOR]; 

	//Yearly Org AC Load Cost KWH = Remote AC Total Cost - Local AC Meter Pos KWH + Local AC Meter Neg KWH
	float 		fOrgACLoad_CostKWH_Y[MAX_YEARS_SECTOR];

	//Yearly Remote ACMeter Reverse Output KWH
	float 		fAC_RevrtOutPutKWH_Y[MAX_YEARS_SECTOR];

	//Yearly Local AC Meter Input KWH 
	float       fLocalAC_INPUTKWH_Y[MAX_YEARS_SECTOR];

	//Yearly Local AC Meter Output KWH 
	float       fLocalAC_OUTPUTKWH_Y[MAX_YEARS_SECTOR];

	//Yearly DC Load Cost KWH
	float		fDCLoad_CostKWH_Y[MAX_YEARS_SECTOR];	

	//Yearly Mppt Total Generate KWH
    float 		fMpptTotalGKWH_Y[MAX_MPPT_GROUP_NUM][MAX_YEARS_SECTOR]; 

	//Yearly Battery Grp Total Chrged KWH
    float 		fBattTotalChrgKWH_Y[MAX_SINGLE_STBATT_NUM][MAX_YEARS_SECTOR];

	//Yearly Battery Grp Total DisChrged KWH
    float 		fBattTotalDisChrgKWH_Y[MAX_SINGLE_STBATT_NUM][MAX_YEARS_SECTOR];

	//Yearly Battery Grp Chrged Times
	float 		fBattChrgTimes_Y[MAX_SINGLE_STBATT_NUM][MAX_YEARS_SECTOR];

    //Yearly Battery Grp DisChrged Times
    float 		fBattDisChrgTimes_Y[MAX_SINGLE_STBATT_NUM][MAX_YEARS_SECTOR];

	//Yearly Peak Flat Valley Chrged/DisChrged KWH
	float		fLocalAC_Sharp_InputKWH_Y[MAX_YEARS_SECTOR];
	float		fLocalAC_Peak_InputKWH_Y[MAX_YEARS_SECTOR];
	float		fLocalAC_Flat_InputKWH_Y[MAX_YEARS_SECTOR];
	float		fLocalAC_Valley_InputKWH_Y[MAX_YEARS_SECTOR];

	float		fLocalAC_Sharp_OutPutKWH_Y[MAX_YEARS_SECTOR];
	float		fLocalAC_Peak_OutPutKWH_Y[MAX_YEARS_SECTOR];
	float		fLocalAC_Flat_OutputKWH_Y[MAX_YEARS_SECTOR];
	float		fLocalAC_Valley_OutputKWH_Y[MAX_YEARS_SECTOR];

	float		fResv1_KWH_Y[MAX_YEARS_SECTOR];

	float		fResv2_KWH_Y[MAX_YEARS_SECTOR];

}ES_YEAR_STAT_DATA; // !!!不可更改结构体!!!


#define MAX_TIME_SECTOR	48
#define MAX_YEAR_DAYS	366
typedef struct tagES_DailyStatData
{	
	time_t				 	tmTimeSave;

	ES_24HOUR_STAT_DATA     hours_Data;
    
	//ES_WEEKLY_STAT_DATA     week_Data;
    
	ES_365DAY_STAT_DATA     days_Data;
    
	ES_YEAR_STAT_DATA       years_Data;
	 
}ES_DAILY_STAT_DATA; // !!!不可更改结构体!!!

typedef struct tagES_StatisticData_Daily
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;

	ES_DAILY_STAT_DATA		es_Data;

}A_BLOB_ES_STATISTIC_DATA; // !!!不可更改结构体!!!



typedef struct tagSyncSlave_Config
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;

	BOOL					bLocalAuthentEnable;
	BOOL					bEnableChargeModeSel;

	char					strQRCode_Gun1[256];
	char					strShortCode_Gun1[32];
	UINT					nNoCardPasswd_Gun1;
	char					strNoCardAssosiateIdTag_Gun1[32];

	char					strQRCode_Gun2[256];
	char					strShortCode_Gun2[32];
	UINT					nNoCardPasswd_Gun2;
	char					strNoCardAssosiateIdTag_Gun2[32];
	INTERSLV_ATTRIBUTES 	mapInterSlvAtributes;
	
}A_BLOB_SYNC_SLAVE_CONFIG;

typedef struct tagRectGrpData
{// LSYS
	//must always put first!!!
	BLOB_TYPE				emBlobType;
	
	UINT 					nOnlineACDCNum;
	UINT					nOnlineDCDCNum;
	UINT					nOnlineMPPTNum;

	float					fACDC_RealTime_Volt;
	float					fACDC_RealTime_Curr; // +
	float					fACDC_Max_Output_Pwr_Ability;
	ACDC_WORKSTATU 			emACDC_WorkStatu;
	
	float					fMPPT_RealTime_Volt; 
	float					fMPPT_RealTime_Curr; 
	float 					fMPPT_Max_DisChrg_Pwr;
	float					fMPPT_Max_Output_Pwr_Ability;
	MPPT_WORKSTATU 			emMPPT_WorkStatu;

	float					fDCDC_RealTime_Volt;
	float					fDCDC_RealTime_Curr;
	float					fDCDC_Max_Output_Pwr_Ability;
	DCDC_WORKSTATU 			emDCDC_WorkStatu;


	float	 				fBatt_RealTime_Volt;
	float 					fBatt_RealTime_Curr; // chrg+ disc-
	UINT 					nSOC;

    float 					fBatt_Max_DisChrg_Pwr; // BMS的 U*I
    float 					fBatt_Max_Chrg_Pwr_Req;

 
	double					fMPPT_Total_Generate_KWH;

	float 					fACDCOutRelayVolt; // ACDC外侧电压

	float 					fActvPwr_PCSGrp; // chrg+ disc-
	float 					fQ_PCSGrp;

	float 					fCtrl_RealTime_Q;
	// LJH241202 SEMS 收到各个客户端数据
	SEMS_B6_LSYS_DATA_INFO  b6LSYS_SEMS;
	SEMS_B6_LSYS_DATA_INFO  b6LSYSLast_SEMS;
}A_BLOB_BRANCH_GRP_RECTDATA;

//新增单个模块系统状态
typedef enum tagSingleRectSystemStatus
{
    SYS_STATUS_SHUTDOWN = 0,  // 关机
    SYS_STATUS_RUNNING  = 1,  // 运行
    SYS_STATUS_STANDBY  = 2,  // 待机
    SYS_STATUS_FAULT    = 3,  // 故障
    SYS_STATUS_ALARM    = 4,  // 告警
}SystemStatus;

typedef union tagPCSStatuCode
{
	UINT	uiPCSStatus;
	struct
	{
		//Byte0
		// UINT	Byte0_Resv0 			: 1;
		// UINT	Byte0_Resv1				: 1;
		// UINT	Byte0_Resv2				: 1;
		SystemStatus SystemStatus 		: 3;//单个模块系统状态：0:关机 1:运行 2:待机 3:故障 4:告警
		UINT	Byte0_Resv3				: 1;
		UINT	Byte0_Resv4				: 1;
        UINT	Byte0_Resv5				: 1;
		UINT	Byte0_Resv6				: 1;
		UINT	Byte0_Resv7				: 1;
		
		//Byte1
		UINT	GridVolt_OverBus		: 1;
		UINT	HighOrLowFault			: 1;
		UINT	BAT_DODALM				: 1;
		UINT	BAT_EOD					: 1;
		UINT	ParamSettErr			: 1;
		UINT	Relay_Fault				: 1;
		UINT	SoftStartFault			: 1;
		UINT	AuxPower_Fault			: 1;
		
		//Byte2
		UINT	EPO_Fault 				: 1;
		UINT	Duplicate_ID 			: 1;
		UINT	UnEven_Currt 			: 1;
		UINT	Fan_Fail 				: 1;
		UINT	Over_Temp				: 1;
		UINT	Phase_Lock				: 1;
		UINT	Island_Prot				: 1;
		UINT	CommFail				: 1;

		//Byte3
		UINT	OutputDerating 			: 1;
		UINT	OutputOverLoad 			: 1;
		UINT	OutputShortCircuit 		: 1;
		UINT	Inter_OverCurrt 		: 1;
		UINT	BusFault 				: 1;
		UINT	DC_Fault 				: 1;
		UINT	AC_Fault 				: 1;
		UINT	System_Fail				: 1;

	}PCSStatu;

}PCS_STATU_CODE;

typedef union tagSTS_1113Code
{
	UINT	uiSTS_1113Status;
	struct
	{
		//Byte0
		UINT	GridOVP 				: 1; //bit0
		UINT	GridUVP					: 1;
		UINT	GridOFP					: 1;
		UINT	GridUFP					: 1;
		UINT	GridFreqLose			: 1;
        UINT	GridLosePhase			: 1;
		UINT	GridPhaseSeqInvert		: 1;
		UINT	GridLoseN				: 1;//bit7
		

		//Byte1
		UINT	GridUnblance			: 1;
		UINT	VGridLineFault			: 1;
		UINT	PllFail					: 1;
		UINT	VgridCrossZeroLose		: 1;
		UINT	GridDQFastFault			: 1;
		UINT	GridFreErr				: 1;
		UINT	GridFreChg				: 1;
		UINT	OutPutOverLoad			: 1;
		
		//Byte2
		UINT	GridOCP 				: 1;
		UINT	IacUnbalance 			: 1;
		UINT	Byte2_Bit2 				: 1;
		UINT	Byte2_Bit3 				: 1;
		UINT	Byte2_Bit4				: 1;
		UINT	Byte2_Bit5				: 1;
		UINT	RefluxFault				: 1;
		UINT	OutputShort				: 1;

		//Byte3
		UINT	VoutCrossZeroLose 		: 1;
		UINT	Byte3_Bit1	 			: 1;
		UINT	GridVoltASampleErr		: 1;
		UINT	GridVoltBSampleErr		: 1;
		UINT	GridVoltCSampleErr		: 1;
		UINT	Byte3_Bit5				: 1;
		UINT	Byte3_Bit6 				: 1;
		UINT	Byte3_Bit7				: 1;

	}STS1113_Statu;

}STS_1113_CODE;
  

typedef union tagSTS_1114Code
{
	UINT	uiSTS_1114Status;
	struct
	{
		//Byte0
		UINT	RlyErr 					: 1; //bit0
		UINT	Byte0_Bit1				: 1;
		UINT	OnGridStartFault		: 1;
		UINT	Byte0_Bit3				: 1;
		UINT	Byte0_Bit4				: 1;
        UINT	Byte0_Bit5				: 1;
		UINT	Byte0_Bit6				: 1;
		UINT	Byte0_Bit7				: 1;//bit7
		

		//Byte1
		UINT	Byte1_Bit0				: 1;
		UINT	Byte1_Bit1				: 1;
		UINT	Byte1_Bit2				: 1;
		UINT	Byte1_Bit3				: 1;
		UINT	Byte1_Bit4				: 1;
		UINT	Byte1_Bit5				: 1;
		UINT	Byte1_Bit6				: 1;
		UINT	Byte1_Bit7				: 1;
		
		//Byte2
		UINT	CanErr 					: 1;
		UINT	SpiErr 					: 1;
		UINT	Byte2_Bit2 				: 1;
		UINT	E2PROMErr 				: 1;
		UINT	OTSink					: 1;
		UINT	OTAmb					: 1;
		UINT	OTCabinet				: 1;
		UINT	FanERR					: 1;

		//Byte3
		UINT	GridFault 				: 1;
		UINT	SysFault	 			: 1;
		UINT	Byte3_Bit2				: 1;
		UINT	Byte3_Bit3				: 1;
		UINT	Byte3_Bit4				: 1;
		UINT	Byte3_Bit5				: 1;
		UINT	Byte3_Bit6 				: 1;
		UINT	Byte3_Bit7				: 1;

	}STS1114_Statu;

}STS_1114_CODE;



typedef union tagSTS_1115Code
{
	UINT	uiSTS_1115Status;
	struct
	{
		//Byte0
		UINT	U2CanErr 				: 1; //bit0
		UINT	U2SpiErr				: 1;
		UINT	Byte0_Bit2				: 1;
		UINT	U2E2PROMErr				: 1;
		UINT	Byte0_Bit4				: 1;
        UINT	Byte0_Bit5				: 1;
		UINT	Byte0_Bit6				: 1;
		UINT	Byte0_Bit7				: 1;//bit7
		
		//Byte1
		UINT	EPO						: 1;
		UINT	OTempTrsfm1				: 1;
		UINT	PowerErr				: 1;
		UINT	Byte1_Bit3				: 1;
		UINT	Byte1_Bit4				: 1;
		UINT	FanErr					: 1;
		UINT	IoReadyErr				: 1;
		UINT	U2SysFault				: 1;
		
		//Byte2
		UINT	Fan1Err 				: 1;
		UINT	Fan2Err 				: 1;
		UINT	Fan3Err 				: 1;
		UINT	Fan4Err 				: 1;
		UINT	Byte2_Bit4				: 1;
		UINT	Byte2_Bit5				: 1;
		UINT	Byte2_Bit6				: 1;
		UINT	Byte2_Bit7				: 1;

		//Byte3
		UINT	Byte3_Bit0 				: 1;
		UINT	Byte3_Bit1	 			: 1;
		UINT	Byte3_Bit2				: 1;
		UINT	Byte3_Bit3				: 1;
		UINT	Byte3_Bit4				: 1;
		UINT	Byte3_Bit5				: 1;
		UINT	Byte3_Bit6 				: 1;
		UINT	Byte3_Bit7				: 1;

	}STS1115_Statu;

}STS_1115_CODE;

typedef enum tagSingleRectType
{
    TYPE_SIG_RECT_ACDC_GRP1 = 0,
	TYPE_SIG_RECT_ACDC_GRP2,
    TYPE_SIG_RECT_ACDC_GRP3,
    TYPE_SIG_RECT_ACDC_GRP4,

	TYPE_SIG_RECT_MPPT_GRP1,
	TYPE_SIG_RECT_MPPT_GRP2,
	TYPE_SIG_RECT_MPPT_GRP3,
	TYPE_SIG_RECT_MPPT_GRP4,

	TYPE_SIG_RECT_DCDC_GRP1,
 	TYPE_SIG_RECT_DCDC_GRP2,
	TYPE_SIG_RECT_DCDC_GRP3,
	TYPE_SIG_RECT_DCDC_GRP4,

	TYPE_SIG_RECT_STS,

	MAX_RECT_TYPE,

}TYPE_SIGNLE_RECT;

typedef enum tagModuleRunningMode
{
	MODULE_RUNNING_NORMAL = 0,
	MODULE_RUNNING_ALARM,
	MODULE_RUNNING_FAULT,
	MODULE_RUNNING_UPGRADE,

	MAX_MODULE_RUNNING
} MODULE_RUNNING_MODE;

typedef enum tagModuleCtrlMode
{
	MODULE_CTRL_AUTO = 0,
	MODULE_CTRL_MANU,
	MAX_MODULE_CTRL
} MODULE_CTRL_MODE;

typedef enum tagChgDischgCtrlFailReason
{
	CHG_DISCHG_CTRL_NORAML = 0,
	CHG_DISCHG_CTRL_ICCLOSS,  // InterCoreCommLoss
	CHG_DISCHG_CTRL_INSULATION_FAULT,
	CHG_DISCHG_CTRL_MODULE_EXCULDE_ADDR,
	CHG_DISCHG_CTRL_BMS_CHG_DISCHG_PROC,
	CHG_DISCHG_CTRL_MODULE_COM_FAIL,
	MAX_CHG_DISCHG_CTRL
} CHG_DIDCHG_CTRL_FAIL_REASON;

typedef union tagModuleGrpCfgInfo
{
	USHORT	usModuleGrpCfg;

	struct
	{
		USHORT	ACDC_Grp_1 				: 1;
		USHORT	ACDC_Grp_2	 			: 1;
		USHORT	DCDC_Grp_1 				: 1;
		USHORT	DCDC_Grp_2 				: 1;
		USHORT	MPPT_Grp_1	 			: 1;
        USHORT	MPPT_Grp_2 				: 1;
	}Info;

} MODULE_GRP_CFG_INFO;

typedef union tagModuleGrpOnlineInfo
{
	USHORT	usModuleGrpOnline;

	struct
	{
		USHORT	ACDC_Grp_1 				: 1;
		USHORT	ACDC_Grp_2	 			: 1;
		USHORT	DCDC_Grp_1 				: 1;
		USHORT	DCDC_Grp_2 				: 1;
		USHORT	MPPT_Grp_1	 			: 1;
        USHORT	MPPT_Grp_2 				: 1;
	}Info;

} MODULE_GRP_ONLINE_INFO;

typedef enum tagCtrlStatus
{
	CTRL_STATUS_OFF = 0,     			// 关机
	CTRL_STATUS_RECTIFY_POSITIVE,       // ACDC:整流  DCDC:正向模式（电池充电）
	CTRL_STATUS_INVERTER_REVERSE,       // ACDC:逆变  DCDC:反向模式（电池放电）
	MAX_CTRL_STATUS
} CTRL_STATUS;

typedef enum tagRunningStatus
{
	RUNNING_STATUS_ACDC_OFF = 0,
	RUNNING_STATUS_ACDC_RECTIFY,
	RUNNING_STATUS_ACDC_INVERTER,
	RUNNING_STATUS_ACDC_FAULT,
	RUNNING_STATUS_ACDC_ALARM,
	RUNNING_STATUS_ACDC_INCONSISTENT,

	RUNNING_STATUS_DCDC_OFF,
	RUNNING_STATUS_DCDC_POSITIVE,
	RUNNING_STATUS_DCDC_RESERVE,
	RUNNING_STATUS_DCDC_FAULT,
	RUNNING_STATUS_DCDC_ALARM,
	RUNNING_STATUS_DCDC_INCONSISTENT,

	RUNNING_STATUS_MPPT_OFF,
	RUNNING_STATUS_MPPT_ON,
	RUNNING_STATUS_MPPT_FAULT,
	RUNNING_STATUS_MPPT_ALARM,
	RUNNING_STATUS_MPPT_INCONSISTENT,

	MAX_RUNNING_STATUS
} RUNNING_STATUS;

typedef union tagRAMStatus
{
	UINT							uiInverterStatus;
	// TODO:后续需要修改补充
	struct
	{
		USHORT	emRunningStatus				: 3;
		USHORT	emGridStatus				: 1;
		USHORT	bDischarge					: 1;
		USHORT  bCharge						: 1;
		USHORT  bOnOff						: 1;
		USHORT  bReserved1					: 1;
		USHORT  bOverBusVolt				: 1;
		USHORT  bVrtFault					: 1;
		USHORT  bBattDodAlarm				: 1;
		USHORT  bBattEodAlarm				: 1;
		USHORT  bUserSetErr					: 1;
		USHORT  bRelayErr					: 1;
		USHORT  bAuxFault					: 1;
		USHORT  bReserved2					: 1;
	}ACDC_N2002_N2402;

}RAMStatus;

/*
*	以下状态表实现对不同类型的PCS状态展开
*	该数据流从此处断开
*	只对235kW的PCS进行赋值和后续处理
*/
typedef union tagInverterStatus_1_1
{
	USHORT							uiInverterStatus;

	struct
	{
		USHORT	emRunningStatus				: 3;
		USHORT	emGridStatus				: 1;
		USHORT	bDischarge					: 1;
		USHORT  bCharge						: 1;
		USHORT  bOnOff						: 1;
		USHORT  bReserved1					: 1;
		USHORT  bOverBusVolt				: 1;
		USHORT  bVrtFault					: 1;
		USHORT  bBattDodAlarm				: 1;
		USHORT  bBattEodAlarm				: 1;
		USHORT  bUserSetErr					: 1;
		USHORT  bRelayErr					: 1;
		USHORT  bAuxFault					: 1;
		USHORT  bReserved2					: 1;
	}ACDC_N2002_N2402;

	struct
	{
		USHORT	bOutputSC					: 1;   // output short circuit
		USHORT	bReserved1					: 1;   
		USHORT	bReserved2					: 1;
		USHORT  bReserved3					: 1;
		USHORT  bModuSleep					: 1;
		USHORT  bModuDischargeAbnormal		: 1;
		USHORT  bReserved4					: 1;
		USHORT  bReserved5					: 1;
		USHORT  bModuDCSideOff				: 1;  // Module DC side in the off status
		USHORT  bModuFaultAlarm				: 1;
		USHORT  bModuProtectAlarm			: 1;
		USHORT  bFanFaultAlarm				: 1;
		USHORT  bOverTempAlarm				: 1;
		USHORT  bOutputOverVoltAlarm		: 1;
		USHORT  bWALK_IN					: 1;
		USHORT  bModuCommIntrpAlarm			: 1;
	}ACDC_N2101;

	struct
	{
		USHORT	bSCFault					: 1;   // short circuit fault
		USHORT	bBattSideUnderVolt			: 1;   
		USHORT	bSCICommFault				: 1;
		USHORT  bBusFault					: 1;
		USHORT  bSleep						: 1;
		USHORT  bDischargeFault				: 1;
		USHORT  bReserved1					: 1;
		USHORT  bReserved2					: 1;
		USHORT  emBattSideOnOff				: 1;
		USHORT  bFaultStatus				: 1;
		USHORT  bAlarmStatus				: 1;
		USHORT  bFanFault					: 1;
		USHORT  bOverTemp					: 1;
		USHORT  bBattSideOverVolt			: 1;
		USHORT  bWALK_IN					: 1;
		USHORT  bMonitorCommFault			: 1;
	}DCDC_N2302;

	struct
	{
		USHORT	bOutputSC					: 1;   // output short circuit
		USHORT	bReserved1					: 1;   
		USHORT	bCommIntrp					: 1;
		USHORT  bInputOrBusAbnormal			: 1;
		USHORT  bModuSleep					: 1;
		USHORT  bModuDischargeAbnormal		: 1;
		USHORT  bRestrictedAirflow			: 1;
		USHORT  bReserved2					: 1;
		USHORT  bModuDCSideOff				: 1;   // Module DC side in the off status
		USHORT  bModuFaultAlarm				: 1;
		USHORT  bModuProtectAlarm			: 1;
		USHORT  bFanFaultAlarm				: 1;
		USHORT  bOverTempAlarm				: 1;
		USHORT  bOutputOverVoltAlarm		: 1;
		USHORT  bWALK_IN					: 1;
		USHORT  bModuCommIntrpAlarm			: 1;
	}MPPT_30kW;
}InverterStatus_1_1;

typedef union tagInverterStatus_1_2
{
	USHORT							uiInverterStatus;

	struct
	{
		USHORT	bEOPFault					: 1;
		USHORT	IDRepeat					: 1;
		USHORT	bCurrUnbalance				: 1;
		USHORT  bFanFault					: 1;
		USHORT  bOverTempFault				: 1;
		USHORT  bPhaseLockFault				: 1;
		USHORT  bIslandingProtect			: 1;
		USHORT  bCommFault					: 1;
		USHORT  bOutDerating				: 1;
		USHORT  bOutOverload				: 1;
		USHORT  bOutShortCircuit			: 1;
		USHORT  bInnerOverCurr				: 1;
		USHORT  bBusFault					: 1;
		USHORT  bDcFault					: 1;
		USHORT  bAcFault					: 1;
		USHORT  bSysFault					: 1;
	}ACDC_N2002_N2402;

	struct
	{
		USHORT	bPowerLimit					: 1;
		USHORT	IDRepeat					: 1;
		USHORT	bCurrUnbalance				: 1;
		USHORT  bPhaseLoss					: 1;
		USHORT  bPhaseUnbalance				: 1;
		USHORT  bInputUnderVolt				: 1;
		USHORT  bInputOverVolt				: 1;
		USHORT  bPfcShutdown				: 1;
		USHORT  emSeriesParallelMode		: 2;
		USHORT  Reserved					: 6;
	}ACDC_N2101;

	struct
	{
		USHORT	bPowerLimit					: 1;   // short circuit fault
		USHORT	IDRepeat					: 1;   
		USHORT	bSysUnbalance				: 1;
		USHORT  Reserved3_4					: 2;
		USHORT  bBusUnderVolt				: 1;
		USHORT  bBusOverVolt				: 1;
		USHORT  bBusShutdown				: 1;
		USHORT  emSeriesParallelMode		: 2;
		USHORT  Reserved10_15				: 6;
	}DCDC_N2302;

	struct
	{
		USHORT	bPowerLimit					: 1;   // output short circuit
		USHORT	IDRepeat					: 1;   
		USHORT	bCurrUnbalance				: 1;
		USHORT  bPhaseLoss					: 1;
		USHORT  bPhaseUnbalance				: 1;
		USHORT  bInputUnderVolt				: 1;
		USHORT  bInputOverVolt				: 1;
		USHORT  bPfcShutdown				: 1;
		USHORT  emSeriesParallelMode		: 2;   // Module DC side in the off status
		USHORT  Reserved					: 6;
	}MPPT_30kW;

}InverterStatus_1_2;

typedef union tagInverterStatus_2_1
{
	USHORT							uiInverterStatus;

	struct
	{
		USHORT	bGridLoss					: 1;
		USHORT	bGridRevSeq					: 1;
		USHORT	bGridLossN					: 1;
		USHORT  bGridPhaseLoss				: 1;
		USHORT  bGridUnderFreq				: 1;
		USHORT  bGridOverFreq				: 1;
		USHORT  bGridUnderVolt				: 1;
		USHORT  bGridOverVolt				: 1;
		USHORT  bIslandingProtect			: 1;
		USHORT  bLvrtFault					: 1;
		USHORT  bHvrtFault					: 1;
		USHORT  Reserved					: 1;
		USHORT  bPhaseLockFault				: 1;
		USHORT  bRevGridUnbalance			: 1;
		USHORT  bRevGridTransOverVolt2		: 1;
		USHORT  bRevGridTransOverVolt1		: 1;
	}ACDC_N2002;

	struct
	{
		USHORT	bBusOverVolt				: 1;
		USHORT	bBusAbnormal				: 1;
		USHORT	bAcUnderFreq				: 1;
		USHORT  bAcOverFreq					: 1;
		USHORT  bAcUnderVolt				: 1;
		USHORT  bAcOverVolt					: 1;
		USHORT  bSpiCommFault				: 1;
		USHORT  bSciCommFault				: 1;
		USHORT  bPhaseSeqError				: 1;
		USHORT  bInputVoltUnbalance			: 1;
		USHORT  bActzOverCurr				: 1;
		USHORT  bSwSampleOverCurr			: 1;
		USHORT  bInputTransientDrop			: 1;
		USHORT  bInputPhaseLoss				: 1;
		USHORT  bInputNoPower				: 1;
		USHORT  bBusUnderVolt				: 1;
	}ACDC_N2402;

	struct
	{
		USHORT	bInverterMode				: 1;
		USHORT	bInvBattUnderVolt			: 1;
		USHORT	Reserved2_3					: 2;
		USHORT  bAcFault					: 1;
		USHORT  bPhaseLockErr				: 1;
		USHORT  bIslandingProtect			: 1;
		USHORT  Reserved7_15				: 9;
	}ACDC_N2101;

	struct
	{
		USHORT	bReverseMode				: 1;   // short circuit fault
		USHORT	bBattUnderVolt				: 1;   
		USHORT	Reserved2					: 1;
		USHORT  bBusInnerVoltAbnormal		: 1;
		USHORT  bBusAcDetected				: 1;
		USHORT  Reserved5_7					: 3;
		USHORT  bBusOverVolt				: 1;
		USHORT  Reserved9_10				: 2;
		USHORT  bDcOverCurrProtect			: 1;
		USHORT  Reserved12_14				: 3;
		USHORT  bBusVoltFault				: 1;
	}MPPT_30kW;

}InverterStatus_2_1;

typedef union tagInverterStatus_2_2
{
	USHORT							uiInverterStatus;

	struct
	{
		USHORT	bAcHigherThanDc				: 1;
		USHORT	bRevBusShortCircuit			: 1;
		USHORT	bBusSlowUnderVolt			: 1;
		USHORT  bBusTransUnderVolt			: 1;
		USHORT  bBusSlowOverVolt			: 1;
		USHORT  bBusTransOverVolt			: 1;
		USHORT  bBusSlowUnbalance			: 1;
		USHORT  bBusTransUnbalance			: 1;
		USHORT  Reserved8					: 1;
		USHORT  bBattEodShutdown			: 1;
		USHORT  Reserved10					: 1;
		USHORT  bBattChargeOverVolt			: 1;
		USHORT  bBattOverCurr				: 1;
		USHORT  bRevBattReverseConn			: 1;
		USHORT  bBattUnderVolt				: 1;
		USHORT  bBattOverVolt				: 1;
	}ACDC_N2002;

	struct
	{
		USHORT	bOutOverVolt				: 1;
		USHORT	bBattReverseConn			: 1;
		USHORT	bEStop						: 1;
		USHORT  bIslandingProtect			: 1;
		USHORT  bAcShortCircuit				: 1;
		USHORT  bAcRelayFault				: 1;
		USHORT  bPhaseLockErr1				: 1;
		USHORT  bPhaseLockErr2				: 1;
		USHORT  bFreqSyncAbnormal			: 1;
		USHORT  bDcRelayFault				: 1;
		USHORT  bInvRelayShort				: 1;
		USHORT  bInvRelayOpen				: 1;
		USHORT  bGridRelayShort				: 1;
		USHORT  bGridRelayOpen				: 1;
		USHORT  bOutShortAlarm				: 1;
		USHORT  bOutUnderVoltProtect		: 1;
	}ACDC_N2402;

	struct
	{
		USHORT	bOffGridMode				: 1;
		USHORT	bOffGridAcOverload			: 1;
		USHORT	Reserved2					: 1;
		USHORT  bGridConnInOffGrid			: 1;
		USHORT  bEStopAlarm					: 1;
		USHORT  Reserved5_15				: 11;
	}ACDC_N2101;

	struct
	{
		USHORT	Reserved0					: 1;   // short circuit fault
		USHORT	bOverload					: 1;   
		USHORT	Reserved2_3					: 2;
		USHORT  bEStopStatus				: 1;
		USHORT  bBusRelayFault				: 1;
		USHORT  bSpSwitchFail				: 1;
		USHORT  bBattReverseConn			: 1;
		USHORT  bFrequentOnOff				: 1;
		USHORT  bInternalUnbalance			: 1;
		USHORT  bInnerSpRelayFault			: 1;
		USHORT  Reserved11_15				: 5;
	}MPPT_30kW;

}InverterStatus_2_2;

typedef union tagInverterStatus_3_1
{
	USHORT							uiInverterStatus;

	struct
	{
		USHORT	bEfficiencyFault			: 1;
		USHORT	bOutCurrUnbalance			: 1;
		USHORT	bPhaseCCbcLimit				: 1;
		USHORT  bPhaseBCbcLimit				: 1;
		USHORT  bPhaseACbcLimit				: 1;
		USHORT  Reserved5					: 1;
		USHORT  bOutOverload				: 1;
		USHORT  bAcTransOverCurr			: 1;
		USHORT  bDcSideFault				: 1;
		USHORT  bDcRelayFault				: 1;
		USHORT  bInvRelayShort				: 1;
		USHORT  bInvRelayOpen				: 1;
		USHORT  bGridRelayShort				: 1;
		USHORT  bGridRelayOpen				: 1;
		USHORT  bInvSoftStartFail			: 1;
		USHORT  bBusSoftStartFail			: 1;
	}ACDC_N2002;

	struct
	{
		USHORT	Reserved0_2					: 1;
		USHORT	bOutCurrUnbalance			: 1;
		USHORT	bInvSoftStartFault			: 1;
		USHORT  bBusSoftStartFault			: 1;
		USHORT  bDcSoftStartRelayFault		: 1;
		USHORT  bParallelCommFault			: 1;
		USHORT  Reserved8_15				: 8;
	}ACDC_N2402;

}InverterStatus_3_1;

typedef union tagInverterStatus_3_2
{
	USHORT							uiInverterStatus;

	struct
	{
		USHORT	bParallelCommFault			: 1;
		USHORT	bDcSoftStartRelayFault		: 1;
		USHORT	bDcSoftStartOverload		: 1;
		USHORT  Reserved3_4					: 2;
		USHORT  bOutCurrSampleFault			: 1;
		USHORT  bInductorCurrSampleFault	: 1;
		USHORT  bDcCurrSampleFault			: 1;
		USHORT  bFreqSyncAbnormal			: 1;
		USHORT  bCarrierAbnormal			: 1;
		USHORT  bEpoFault					: 1;
		USHORT  bEepromFaultRev				: 1;
		USHORT  bCanCommFaultRev			: 1;
		USHORT  bAbuFaultRev				: 1;
		USHORT  bDspFaultRev				: 1;
		USHORT  bAcOutShortCircuit			: 1;
	}ACDC_N2002;

}InverterStatus_3_2;

typedef union tagInverterStatus_4_1
{
	USHORT							uiInverterStatus;

	struct
	{
		USHORT	bCanCommFault				: 1;
		USHORT	bSciCommFault				: 1;
		USHORT	bSpiCommFault				: 1;
		USHORT  bBalanceBridgeOverCurr		: 1;
		USHORT  bBusOverVolt				: 1;
		USHORT  bBusUnderVolt				: 1;
		USHORT  bHeatsinkOverTemp			: 1;
		USHORT  bAmbientOverTemp			: 1;
		USHORT  bAuxPowerFault				: 1;
		USHORT  bDspShootThrough			: 1;
		USHORT  bCpldShootThrough			: 1;
		USHORT  bFanFault					: 1;
		USHORT  bVoSetOverProtect			: 1;
		USHORT  bVoSetTooLow				: 1;
		USHORT  bDcShortCircuit				: 1;
		USHORT  bIdRepeat					: 1;
	}ACDC_N2002;

	struct
	{
		USHORT	bAmbientOverTemp			: 1;
		USHORT	bIpmDOverTemp				: 1;
		USHORT	bIpmCOverTemp				: 1;
		USHORT  bIpmBOverTemp				: 1;
		USHORT  bIpmAOverTemp				: 1;
		USHORT  bWaterOutletOverTemp		: 1;
		USHORT  bWaterInletOverTemp			: 1;
		USHORT  bCanCommFault				: 1;
		USHORT  bSoftStartResOverTemp		: 1;
		USHORT  bIdRepeat					: 1;
		USHORT  bVoSetTooLow				: 1;
		USHORT  bVoSetOverProtect			: 1;
		USHORT  bFanFault					: 1;
		USHORT  bLeakageCurrFault			: 1;
		USHORT  bInsulationFault			: 1;
		USHORT  bAuxPowerFault				: 1;
	}ACDC_N2402;

}InverterStatus_4_1;

typedef union tagInverterStatus_4_2
{
	USHORT							uiInverterStatus;
	

}InverterStatus_4_2;

typedef struct tagRectGrp_Data
{// RTIF
	//must always put first!!!
	BLOB_TYPE	           			emBlobType;   								  /*blob 类型*/

	MODULE_RUNNING_MODE  		    emModuleRunningMode;    					  /*模块运行模式*/
	MODULE_CTRL_MODE				emManuAutoMode;								  // 自动/手动模式
	CHG_DIDCHG_CTRL_FAIL_REASON		emChrgDischrgCtrlFailReason;  				  /*无法充放电原因*/
	
	MODULE_GRP_CFG_INFO				unioModuleCfgInfo;  						  /*模块组配置信息*/
	MODULE_GRP_ONLINE_INFO			unioModuleOnlineInfo;  						  /*模块组在线信息*/

	USHORT							usGrpID[MAX_RECT_TYPE]; 					  /*组号*/

	int					   			nOnlineRectNum[MAX_RECT_TYPE];   			  /*在线数量*/

	float		   		  		    fRatedPower[MAX_RECT_TYPE]; 				  /*额定功率*/
	float				   			fRatedCurr[MAX_RECT_TYPE];  				  /*额定电流*/

	float				   			fRealTime_MaxCurr[MAX_RECT_TYPE];  			  /*当前最大电流*/

	UINT 				   			emWorkMode[MAX_RECT_TYPE]; 					  //0-OFF|1-Rectifying Mode|2-Invert Mode|3-MPPT
	
	CTRL_STATUS 					emCtrlStatus[MAX_RECT_TYPE];  				  // 控制状态

	float 				   			fCtrlVolt[MAX_RECT_TYPE];     				  // 控制电压
	float				   			fCtrlCurr[MAX_RECT_TYPE];     				  // 控制电流
	float							fCtrlPower[MAX_RECT_TYPE];    				  // 控制功率

	RUNNING_STATUS					emRunningStatus[MAX_RECT_TYPE];   			  // 运行状态

	float			  	   			fRealTimeVolt[MAX_RECT_TYPE]; 				  // 当前电压
	float				   			fRealTimeCurr[MAX_RECT_TYPE]; 				  // 当前电流

	float				   			fCtrl_RealTime_Q[MAX_RECT_TYPE];   			  // 控制当前无功功率
	float							fRealTimeSystemPower[MAX_RECT_TYPE];  		  // 当前系统功率

	float							fCtrlBattSideCutoffVolt[MAX_RECT_TYPE]; 	  // 控制电池侧放电截止电压
	float							fCtrlBattSideCurrLmt[MAX_RECT_TYPE];  		  // 控制电池侧电流限制
	float							fCtrlBusSideCurrLmt[MAX_RECT_TYPE];   		  // 控制母线侧电流限制

	float							fRealTime_BattSideVolt[MAX_RECT_TYPE];        // 当前电池测电压
	float							fRealTime_BattSideTotalCurr[MAX_RECT_TYPE];   // 当前电池侧总电流
	float							fRealTime_BusSideVolt[MAX_RECT_TYPE];  		  // 当前母线侧电压
	float							fSystemVolt[MAX_RECT_TYPE];             	  // 系统电压
	float							fSystemTotalCurr[MAX_RECT_TYPE];         	  // 系统总电流

}A_BLOB_RECT_GRP_DATA;

typedef struct tagStrSingleRectData
{
	UINT 		nBelongGrp;    // 所属组号
 	
	BOOL 		bExist;

	BYTE 		byCommStatu; //0-Normal ,1-CommFail

	BYTE 		byRectErrCode[16];	
	BYTE 		byInvertErrCode[16];
	BYTE 		byPCSFaultCode[16];	

	BYTE		byPCSFaultcode_1110[4];
	BYTE		byPCSFaultcode_1111[4];
	BYTE 		byPCSFaultCode_1113[16];	
	BYTE 		byPCSFaultCode_1114[16];	
	BYTE 		byPCSFaultCode_1115[16];	

	UINT		uiPCSFaultCode_1113;
	UINT		uiPCSFaultCode_1114;
	UINT		uiPCSFaultCode_1115;
	
	//Rect Alarm Info//
	UINT 		emWorkMode; //1.Invert Mode; 0:Rectifying Mode; 2:Idle Mode

	BOOL		bPFC_Off;
	BOOL		bAC_OverVolt;
	BOOL		bDC_LowVolt;
	BOOL		bAC_NoBalance;
	BOOL		bAC_PhaseLack;
	BOOL		bCurr_NoBalace;
	BOOL		bID_Conflict;
	BOOL		bPowerLimited;

	BOOL		bRectCommFail;
	BOOL		bWalkIn_Enabled;
	BOOL		bDC_OverVolt;
	BOOL		bOverTemp;
	BOOL		bFanFault;
	BOOL		bProtected;
	BOOL		bRectFail;
	BOOL		bDC_Off;

	BOOL		bDisCh_Err;
	BOOL		bOn_Sleep;
	BOOL		bPFC_Err;
	BOOL		bInternalCommErr;
	BOOL		bInternalDC_LowVolt;
	BOOL		bDC_Shorted;

	float 		fRTOutputVolt;
	float 		fRTOutputCurr;

	float 		fRect_DCDCVer;   // DC软件版本号
	float 		fRect_PFCVer;    // PFC软件版本号

    BYTE        byBarCode[24];   // 条码信息 TODO:复用性待确认

	PCS_STATU_CODE      mapPCSCode;
	
	float 		fPh_InputVolt[NUM_GRID_PHASE];		// 交流三相电压
	float 		fPh_InputCurr[NUM_GRID_PHASE];		// 交流三相电流
 
	//Added For STS
	float		fPh_OutputVolt[NUM_GRID_PHASE];

 	float 		fAC_Freq;							// 交流频率
	float 		fAmbit_Temp;						// 模块环境温度
	
	float		fPtotalActive;						// 总有功功率
	float 		fPphase_Active[NUM_GRID_PHASE];		// 三相有功功率

	float		fPtotalReactive;					// 总无功功率
	float 		fPphase_Reactive[NUM_GRID_PHASE];	// 三相无功功率

	float		fPtotalFactor;						// 总功率因数
	float 		fPphase_Factor[NUM_GRID_PHASE];		// 三相功率因数

	float 		fphase_LineVolt[NUM_GRID_PHASE];	// AB BC CA  // 交流线电压

	float		fPTotalApparent;					// 总视在功率
	float		fPphase_Apparent[NUM_GRID_PHASE];	// 三相视在功率

	BYTE 		byCurrent_Schedulingmode;	// 1-交流调度  0-直流调度
	BYTE 		byCurrent_Onandoffstatus;	// 1-离网  0-并网逆变
	BYTE 		byCurrent_Systemstatus;		// 1-纯并网系统  0-并离网系统
	BYTE 		byCurrent_Chargingmode;		// 2-恒压 1-恒功率 0-恒流

	//235kw DC Curr/Volt
	float		fDCsideCurrent;						// 模块直流侧电流
	float		fDCsideVoltage;						// 模块直流侧电压

	float 		f35KW_PlateTemperature;
	float		f35KW_PowerDevicesperature;
	float		fMPPT_DCTemp;
	float		fMPPT_PFCTemp;
	float		fMPPT_DCTransTemp;
	float		fMPPT_PFCTransTemp;

	// D9 新增
	UINT							uModuID;     // 单模块号
	RAMStatus						unioRAMStatus;
	InverterStatus_1_1				unioInverterStatus_1_1;
	InverterStatus_1_2				unioInverterStatus_1_2;
	InverterStatus_2_1				unioInverterStatus_2_1;
	InverterStatus_2_2				unioInverterStatus_2_2;
	InverterStatus_3_1				unioInverterStatus_3_1;
	InverterStatus_3_2				unioInverterStatus_3_2;
	InverterStatus_4_1				unioInverterStatus_4_1;
	InverterStatus_4_2				unioInverterStatus_4_2;
	UINT							uiBarCode[4];				// 条码信息
	/*DCDC*/
	float							fModuBattSideVolt;			// 模块电池侧电压
	float							fModuBattSideCurr;			// 模块电池侧电流
	float							fModuBusSideVolt;			// 模块母线侧电压
	float							fModuBusSideCurr;			// 模块母线侧电流
	/*MPPT*/
	float							fModuVolt;					// 模块电压
	float							fModuCurr;					// 模块电流
}STU_SIGNLE_RECT_DATA;

typedef struct tagAllSingleRectInfo
{
	//must always put first!!!
	BLOB_TYPE	emBlobType;

	UINT 					nOnlineNum[MAX_RECT_TYPE];
	STU_SIGNLE_RECT_DATA 	arrRect[MAX_RECT_TYPE][MAX_SINGLE_RECT_NUM];
		

}A_BLOB_SINGLE_RECT_INFO;

typedef enum tagGrpRectCtrlCmdType
{
    //AC/DC Group Ctrl
    GRP_ACDC_RECTIFY_START= 0,
    GRP_ACDC_RECTIFY_ADJ_CURR,
	GRP_ACDC_INVERT_START,
 	GRP_ACDC_INVERT_PENDING,
	GRP_ACDC_STOP,
	
	//DC/DC Group Ctrl
	GRP_DCDC_RECTIFY_START,
	GRP_DCDC_RECTIFY_PENDING,
	GRP_DCDC_STOP,
	
	GRP_CTRL_MAX
}GRP_RECT_CTRL_CMDTYPE;

typedef enum tagBlobCtrlPcsCmdType
{
	// Stop
	CTRL_PCS_STOP,

	// ONGRID
	CTRL_PCS_ONGRID_RECTIFY,
	CTRL_PCS_ONGRID_INVERT,

	// OFFGRID
	CTRL_PCS_OFFGRID_INVERT,

	CTRL_PCS_CMD_MAX
}CTRL_PCS_CMDTYPE;

typedef struct tagBlobGrpRectCtrlCmd
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;
	
	int						nGrp;  
	GRP_RECT_CTRL_CMDTYPE	emCtrl_Type;

	float					fGrpVoltLmt;
	float					fGrpCurrLmt;
	float					fCtrlPower;
 
}A_BLOB_GRP_RECT_AUTOCTRL_CMD;

typedef enum tagEnergySchMode
{
	ENERGY_SCHMODE_DC = 0,
	ENERGY_SCHMODE_AC,

	ENERGY_SCHMODE_MAX
}ENERGY_SCH_MODE;

typedef enum tagChaOrDischMode
{
	CODM_CONSTANT_CURRENT_MODE = 0,
	CODM_CONSTANT_POWER_MODE,

	CODM_MAX
}CHAORDISCH_MODE;

typedef struct tagBlobCtrlPCSOngridCmd
{
	BLOB_TYPE				emBlobType;
	int						nPMIndex;
	CTRL_PCS_CMDTYPE		emCtrlType;

	ENERGY_SCH_MODE			emEnergySchMode;
	CHAORDISCH_MODE			emChaOrDischMode;
	float					fGrpCurrLmt;
	float					fGrpPwrLmt;
}A_BLOB_CTRL_PCS_ONGRID_CMD;

typedef struct tagBlobCtrlPCSOffgridCmd
{
	BLOB_TYPE				emBlobType;
	int						nPMIndex;
	CTRL_PCS_CMDTYPE		emCtrlType;

	float					fGrpVoltLmt;
	float					fGrpFreqLmt;
}A_BLOB_CTRL_PCS_OFFGRID_CMD;

typedef enum tagBackBattWorkMode
{
    BATT_WORKMODE_FIRST_DISCHRG = 0, // 优先放电:逆流自动启动充电使能,转充电
    BATT_WORKMODE_FIRST_CHRGING, // 优先充电:过载自动启动放电使能,转放电
	// LJH250930 加快自动充放响应,ChgDisOnly
	BATT_WORKMODE_DISCHRG_ONLY,  // 只放电
	BATT_WORKMODE_CHRGING_ONLY,  // 只充电
	BATT_WORKMODE_STANDBY,       // 待机
	BATT_WORKMODE_IDLE,          // 关机
	BATT_WORKMODE_MAX
}STBATT_PRI_WORKMODE;

typedef struct tagEMSBattPriSector
{
	time_t						tmStart;
    int							nInterval; //start + interval = end time
    FEERATE_ATTRI				emFeeRate;
	STBATT_PRI_WORKMODE         emBatt_WorkMode;
	UINT 						nStopSOC;
	UINT 						nActvPwrLmt; // 总功率限制
	int							nReActvPwrLmt;
	UINT						nBattGrpPwrLmt; // 单组电池组功率限制

}EMS_LIMIT_SECTOR; // !!!不可更改结构体!!!

typedef struct tagEMSBattPriSectorExt
{
	UINT						nGridInputPwrLmt; // 动态网侧输入限值
	time_t						tmEndTime; // 结束时间
	UINT						nres_1; // 保留
	UINT						nRes_2; // 保留
	UINT						nRes_3; // 保留
	UINT						nRes_4; // 保留
	UINT						nRes_5; // 保留

}EMS_LIMIT_SECTOR_EXT; // !!!不可更改结构体!!!

#define MAX_EMSMGMT_LMT_SETOR	96
typedef struct tagblobEMSMgmtPlan
{
	//must always put first!!!
	BLOB_TYPE					emBlobType;

	UINT 						nEMSMgmt_Ver;
	int 						nTotal_EmsLmt_Sectors;
    EMS_LIMIT_SECTOR            emsLmtSectors[MAX_EMSMGMT_LMT_SETOR];
	EMS_LIMIT_SECTOR_EXT        emsLmtSectors_ext[MAX_EMSMGMT_LMT_SETOR];

}A_BLOB_EMS_CONFIG_PLAN; // !!!不可更改结构体!!!

typedef struct tagBMSParmLimit
{
	UINT						nStopSOCStep1;
	UINT						nStopSOCStep3;
	UINT						nStopSOCStep5;
	UINT						nStopSOCStep7;

	UINT 						nPwrLmtStep1;
	UINT 						nPwrLmtStep3;
	UINT 						nPwrLmtStep5;
	UINT 						nPwrLmtStep7;

	// UINT 						nCurLmtStep1;
	// UINT 						nCurLmtStep3;
	// UINT 						nCurLmtStep5;
	// UINT 						nCurLmtStep7;

	int							nWaitTimeStep2;
	int							nWaitTimeStep4;
	int							nWaitTimeStep6;

}BMS_TESTMODE_PARM;

typedef struct tagBMSInitRange
{
	// Self Check Range
	// 1. 总压范围
	float						TH_nTotalVlot_Max;
	float						TH_nTotalVlot_Min;
	float						TH_nTotalAddVolt_Gap;
	// 2. 电压/温度总数
	float						TH_nVolCell_Num;
	float						TH_nTempCell_Num;
	// 3. 单体均值范围
	float						TH_nVolMean_Max;
	float						TH_nVolMean_Min;
	float						TH_nTempMean_Max;
	float						TH_nTempMean_Min;
	// 4. 单体最大值范围
	float						TH_nVolMax_Max;
	float						TH_nVolMax_Min;
	float						TH_nTempMax_Max;
	float						TH_nTempMax_Min;
	// 5. 单体最小值范围
	float						TH_nVolMin_Max;
	float						TH_nVolMin_Min;
	float						TH_nTempMin_Max;
	float						TH_nTempMin_Min;
	// 6. 单体极差范围
	float						TH_nVol_Gap;
	float						TH_nTemp_Gap;

	// Charge/Discharge Test Range
	// 1. Charge 
	float 						TH_nChargeEndTemp_Max;
	float 						TH_nChargeEndTemp_Min;
	float						TH_nChargeEndTemp_Gap;
	float						TH_nChargeEndVol_Max;
	float 						TH_nChargeEndVol_Gap;
	float 						TH_nChargeEnd_TotalVolMax;
	float						TH_nChargeEnd_Energy;
	float 						TH_nChargeEndPlateauVol_Gap;
	float						TH_fChargeEndRes_Gap;
	// 2. Discharge 
	float 						TH_nDischargeEndTemp_Max;
	float 						TH_nDischargeEndTemp_Min;
	float 						TH_nDischargeEndTemp_Gap;
	float 						TH_nDischargeEndVol_Min;
	float 						TH_nDischargeEndVol_Gap;
	float 						TH_nDischargeEnd_TotalVolMin;
	float						TH_nDischargeEnd_Energy;
	float 						TH_nDischargeEndPlateauVol_Gap;
	float						TH_fDischargeEndRes_Gap;
	// 3. Efficiency
	float						TH_nPower_Efficiency;
	float						TH_nSystem_Efficiency;

}BMS_INITIAL_RANGE;
typedef enum tagblobBMSTestState
{	
	STATE_NULL = 0,
	STATE_TESTSTART,
	STATE_SELFCHECK_END,
	STATE_STEP1,
	STATE_STEP2,
	STATE_STEP3,
	STATE_STEP4,
	STATE_STEP5,
	STATE_STEP6,
	STATE_STEP7,
	STATE_TEST_END,
	STATE_STOP,
	
}BMS_AUTOTEST_STATE;

typedef struct tagblobBMSTestSet
{	
	//must always put first!!!
	BLOB_TYPE					emBlobType;
	float						timeRemain[MAX_SINGLE_STBATT_NUM];
	BMS_AUTOTEST_STATE			testState[MAX_SINGLE_STBATT_NUM];
    BMS_TESTMODE_PARM			autoTestParm[MAX_SINGLE_STBATT_NUM];
    BMS_INITIAL_RANGE			initRange;

}A_BLOB_BMS_TESTMODE_SET;


typedef struct tagblobSEMS2SLAVE_6330DATA
{	
	//must always put first!!!
	BLOB_TYPE					emBlobType;

	S63Uctl_30WorkPwr	stRev6330;

}A_BLOB_SEMS2SLAVE_6330;

// LJH250612 planSector 增加EMS_LIMIT_SECTOR的参数
typedef struct tagblobSEMS_A6_USYS
{	
	//must always put first!!!
	BLOB_TYPE					emBlobType;
	SEMS_A6_USYS_DATA_INFO      A6_USYS;
}A_BLOB_SEMS_A6_USYS;

typedef struct tagblobBMSTestResult
{
	//must always put first!!!
	BLOB_TYPE					emBlobType;

	BMS_AUTOTEST_STATE			testState;

	float						timeRemain;

	//--- 0x1532ffxx ---//
	USHORT						BMS_V_Num;
	USHORT						BMS_T_Num;
	USHORT						BMS_Tot_V;
	USHORT						BMS_Add_V;


	//--- 0x1533ffxx ---//
	USHORT 						BMS_MaxVoltage;
	USHORT 						BMS_MinVoltage;
	USHORT 						BMS_AvgVolt;
    USHORT 						BMS_RangeVolt;

	//--- 0x1534ffxx ---//
	USHORT 						BMS_MaxTemperature;		// *0.1
	USHORT 						BMS_MinTemperature;		// *0.1
	USHORT 						BMS_AvgTemp;			// *0.1
	USHORT 						BMS_RangeTemperature;	// *0.1

	//--- 0x1535ffxx ---//
	USHORT						BMS_TotAdd_Gap;	// V		
	USHORT						BMS_RangeRes;	// *0.01

	//--- 0x1536ffxx ---//
	// Null

	//--- 0x1537ffxx ---//
	USHORT						BMS_LMaxTemperature;
	USHORT						BMS_LRangeTemperature;
	USHORT						BMS_LMaxVol;
	USHORT						BMS_LTermRangeVol;// terminal of the charge voltage range

	//--- 0x1538ffxx ---//
	USHORT						BMS_LMinTemperature;
	USHORT						BMS_Charge_Energy;
	USHORT						BMS_Charge_Capacity;
	USHORT						BMS_LPlateauRangeVol; // charge plateau vol range


	//--- 0x1539ffxx ---//
	USHORT						BMS_LRangeRes;

	//--- 0x1540ffxx ---//
	USHORT 						BMS_SMaxTemperature;
	USHORT 						BMS_SRangeTemperature;
	USHORT 						BMS_SMinVol;
	USHORT 						BMS_STermRangeVol; // terminal of the discharge voltage range

	//--- 0x1541ffxx ---//
	USHORT						BMS_SMinTemperature;
	USHORT 						BMS_Discharge_Energy;
	USHORT						BMS_Discharge_Capacity;
	USHORT						BMS_SPlateauRangeVol; // discharge plateau vol range

	//--- 0x1542ffxx ---//
	USHORT 						BMS_EnergyEfficiency;
	USHORT 						BMS_TotalSysEfficiency; // 系统总效率，通过电表计算

	//--- 0x1543ffxx ---//
	// int							BMS_testHeartBeat;
	USHORT  					BMS_Total_Result;
	USHORT 						BMS_Self_Result;
	USHORT						BMS_Charge_Result;
	USHORT 						BMS_DisCharge_Result;

	//--- 0x1544ffxx ---//
	USHORT						BMS_SRangeRes;

	// 存储电表充放电量，不上传上位机，只用于内部计算
	USHORT						BMS_Meter_Charge_KWh;
	USHORT 						BMS_Meter_Discharge_KWh;


}A_BLOB_BMS_TESTMODE_RESULT;

typedef struct tagblobBMSTestProcess
{	
	USHORT 							MaxTemp_inCharge;
	USHORT 							RangeTemp_inCharge;
	// int 							MaxVol_inCharge;
	USHORT 							RangeVol_inTerminalCharge;// 充电末端电压差
	USHORT 							RangeVol_inPlaCharge;// 充电平台期电压差
	USHORT							MinTemp_inCharge;
	float 							RangeRes_inCharge;
	float							StartEnergy_inCharge; // min
	float							EndEnergy_inCharge; // max
	float							StartKWh_inCharge;
	float							EndKWh_inCharge;

	USHORT 							MaxTemp_inDischarge;
	USHORT 							RangeTemp_inDischarge;
	// int 							MaxVol_inDisharge;
	USHORT 							RangeVol_inTerminalDischarge;// 放电末端电压差
	USHORT 							RangeVol_inPlaDischarge;// 放电平台期电压差
	USHORT							MinTemp_inDischarge;
	float 							RangeRes_inDischarge;
	float							StartEnergy_inDischarge; // min
	float							EndEnergy_inDischarge; // max
	float							StartKWh_inDischarge;
	float							EndKWh_inDischarge;

}A_BLOB_BMS_TESTMODE_PROCESS;

typedef enum tagTestResult
{	
	TEST_FAIL = -1,
	TEST_PASS = 0,

}STATE_TEST_RES;

typedef struct tagblobBranchNodeData
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;

	int						nNodeIdx; //John 20220413 Added For Bubble Sort
	float 					fChrgedEnergy;//For Bubble Sort. 

    //1. MPPT Manage By IMSU-D
    float 					fMPPT_Max_DisChrg_Pwr;  
    float 					fBatt_Max_DisChrg_Pwr;
    float 					fBatt_Max_Chrg_Pwr_Req;

    //2. MPPT PM
    float 					fMPPT_Distri_4_ACDC_Pwr;//Not Allocate
    float 					fMPPT_Distri_4_DCDC_Pwr;
    float 					fMPPT_Distri_4_Batt_Pwr;

    //3. Battery Cube
    float 					fBatt_Distri_4_ACDC_Pwr; // 分配给单组电池柜的放电功率
    float 					fBatt_Distri_4_DCDC_Pwr;
	UINT					nSOC;

    //4. PCS Max Ability
    float 					fACDC_Max_Ability_Pwr;
    float 					fDCDC_Max_Ability_Pwr;

    //5. Battery Cube Remain Power
    float 					fRemain_Batt_DisChrg_Pwr;
    float 					fRemain_MPPT_DisChrg_Pwr;
    float 					fRemain_Batt_Chrg_Pwr_Req;
 
    //6. Allocate For Battery Chrge Power
    float 					fACDC_Distri_4_BattChrg_Pwr; // 分配给单组电池柜的充电功率
    
	float 					fQoutput_Distri4ACDC;
 
	STBATT_PRI_WORKMODE     emBatt_WorkMode;

	float                   fAvg_ChrgLmtPower;
    float                   fAvg_DisChrgingLmtPower;

	float                   fAvg_QoutputLmt;


	time_t					tm_Last4BATTempLmt;
	UINT					nSysOverTempTimes;
	// LJH240911 AvgPwrFirst MIN(fACDC_Max_Ability_Pwr, fBatt_Max_Chrg_Pwr_Req) MAX:fActvPwr_PCSGrp MIN:fRatedPower
	float 					fMax_Chrg_PwrAbility0; // LJH250610 fPwrAbility0
	float 					fMax_DisChrg_PwrAbility0; // SEMS与系统状态/设置的截止充放电SOC相关 // LJH250930 加快自动充放响应,StopSOC_SetSEMS
	float                   fMax_Chrg_PwrAbilityA; // bSystemOK, bStopChrgBySoc_SetSEMS // LJH250930 加快自动充放响应,WorkASAP
	float                   fMax_DisChrg_PwrAbilityA;
	float 					fMax_Chrg_PwrAbility; // 业务相关:如管理计划SOC、单组电池组功率限制、系统故障、手动模式等 DisPatch_**
	float 					fMax_DisChrg_PwrAbility;
	// LJH241202 LJH250612可以删除，写入了管理计划 pCCU->emsMgmtData.planSectorNow.nStopSOC;
	UINT                    nStopSOCReq; // 可以删除
	double                  fTmLastSend6360Standby; // LJH250930 加快自动充放响应	
}A_BLOB_BRANCH_NODE_CALC_DATA;

/*
typedef struct tagBlobIMEU2DataInfo
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;

	//----------IMEU2 Data--------------//
	UINT 					emDI1_Fire;
	UINT 					emDI2_Smoke;
	UINT 					emDI3_DoorOpen;
	UINT 					emDI4_Flood;
	UINT 					emAirCond_CommFail;
	UINT 					emTempHumi_CommFail;
	
	//----------AC Air Condition---------//
	UINT					emMachineState; //1-Run,0-Stop
	UINT					emInterFanState;
	UINT					emExterFanState;
	UINT					emCompState;
	UINT					emHeaterState;
	UINT					emHydDischState;

	UINT					emRetAirTempFault; //1-Alarm,0-No Alarm
	UINT					emEvaTempFault;
	UINT					emCondTempFault;
	UINT					emInterFanFault;
	UINT					emExterFanFault;
	UINT					emRefLeakAlarm;
	UINT					emCabLowTempAlarm;
	UINT					emCabHighTempAlarm;
	
	BYTE 					byAirCondTotalAlm;

	float					fReturnAirTemp;
	float					fEvapTemp;
	float					fCondenserTemp;

	float 					fCompStartTemp;
	float					fCompRetDiffTemp;
	float					fHeaterStartTemp;
	float					fHeaterRetDiffTemp;
	float					fCabHighTempLimit;
	float					fCabLowTempLimit;

	//----------Temperature & Humi-------//
	float 					fHumidity;
	float 					fTemperature;

	//Version  Info
	BYTE 					szIMEUName[11];
    char					szHardwareVer[6];
    char 					szSoftwareVer[7];
    BYTE 					szSN[26];

	UINT 					emIMEU2_CommFail;

}A_BLOB_IMEU2_DATAINFO;
*/

typedef enum tagAirCondMachineState
{
	STAT_AIR_IDLE = 0,
	STAT_AIR_COOLON,
	STAT_AIR_HEATON,
	STAT_AIR_DEHUMIMITY,

}AIRCOND_MACHINE_STATE;

typedef struct tagBattCabIMEU2DataInfo
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;

	//----------IMEU2 Data--------------//

	//S2C_MSG1 0x14CByyxx
	float                   fPT1000Temperature[MAX_PT1000_NUM];

	//S2C_MSG2 0x14CCyyxx
	USHORT	uFloodFreq;		// Frequency of water immersion
	UINT	uPressureSen;		// pressure transmitter sensor
	UINT	uVibrationSen;		// Vibration sensor

	//S2C_MSG3 0x14CDyyxx
	float 					fHumidity[MAX_HUMITEMP_NUM];
	float 					fTemperature[MAX_HUMITEMP_NUM];
	float					fAirTemp;			//aircondition temperature
	float					fAirHum;			//aircondition humidity

	//S2C_MSG4  0x14CEyyxx
	float					fCombGasPotency[MAX_GASSENSOR_NUM];
	UINT 					nExhaustFanSpeed;

	//S2C_MSG6  0x14D0yyxx
	UINT 					emDI1_Fire;
	UINT 					emDI2_Smoke;
	UINT 					emDI3_DoorOpen;
	UINT 					emDI4_FlammableGas;
	UINT					emDI5Dump;
	UINT 					emDI6_ShutterClosedOver;
	UINT 					emFlood;
	UINT 					emTempHumi_CommFail[MAX_HUMITEMP_NUM];
	UINT 					emGasSensor_CommFail[MAX_GASSENSOR_NUM];
	UINT 					emShutterOpenedOver;
	BYTE					byCtrlDoLighting;	//Lighting on status
	BYTE					byCtrlDoZHSAlarm;	//audible and visual alarm
	BYTE					bySPDAlarm;			//Lightning protection alarm status

	//S2C_MSG11 0x14D5yyxx
	UINT					nAirCondiType_Cfg;	//reserve
	UINT					nHumiTempSensorNumb_Cfg;
	UINT					nCombGasSensorEnable_Cfg;
	UINT					nCombGasSensorType_Cfg;
	UINT					nCombGasSensorNumb_Cfg;
	UINT					nFireProtectType;
	BYTE					byHydrogenRemovalIntervalDay;
	BYTE					HydrogenRemovalIntervalMinute;

	//S2C_MSG12 0x14D6yyxx
	BYTE					byLouverActioTime;

	//S2C_MSG13	0x14D7yyxx
	char					szHardwareVer[16];
    char 					szSoftwareVer[16];

	UINT 					emIMEU2_CommFail;




	// RS 485 data
	UINT 					emAirCond_CommFail;
	UINT					emAirCondTotalAlm;
 	UINT                    emIMEU2BMSCommState;
	UINT					emAirCondHWAlarm;
	
	//Comb Gas Potency Detection
	UINT 					nGasSensorNum;
	
	//----------AC Air Condition---------//
	AIRCOND_MACHINE_STATE 	emAirMachineState;//LG
	AIRCOND_MACHINE_STATE 	emLast_AirMachineState;//LG

	UINT					emOnOrOffd;	//1-ON,0-OFF
	UINT					emStartRefrigMode;
	UINT					emStartSupplyAirMode;
	UINT					emStartStandbyMode;
	UINT					emStartHeatMode;
	UINT					emSensorFail;
	UINT					emHighOrLowVoltAlm;
	UINT					emHighOrLowTempAlm;
	UINT					emHighOrLowPressureAlm;
	UINT					emCompressorAlm;

	UINT					emWholeMachineStatus;  //20211203 Add For TongFei
	UINT					emInternalFanStatus;
	UINT					emOutsideFanStatus;
	UINT					emCompressorStatus;
	UINT					emElectricHeatStatus;
	UINT					emForceAutoMode;

	float 					fCoolSetPoint;		//(cooling on = cooling set point + cooling deviation)
	float 					fCoolDeviation;		//(cooling stop = cooling set point)
	float					fHighTempAlmValue;
	float					fLowTempAlmValue;
	float					fHeatSetPoint;		//(heating on = heating set point)
	float					fHeatDeviation; 	//(heating stop = heating set point + heating deviation)
	float					fRealTimeCabTemp;
	float					fRealTimeCabHumi;
	float					fDehumiStartTemp;
	float					fDehumiStopTemp;
	float					fDehumiStartHumi;
	float					fDehumiStopHumi;

	float					fDehumiRetDiffHumi;  //20211203 Add For TongFei
	UINT					emHighTempAlm;
	UINT					emLowTempAlm;
	UINT					emHighHumiAlm;
	UINT					emLowHumiAlm;
	UINT                    emPlateTubeTempSensorFail;
	UINT                    emCondensationTempSensorFail;
	UINT                    emInternalTempSensorFail;
	UINT                    emHighVoltAlm;
	UINT                    emLowVoltAlm;
	UINT                    emPhaseSequenceAlm;
	float					fPlateTubeTemp;
	float					fCondensationTemp;
	float					fHighHumiPoint;
	UINT					nTongFeiSW_Ver;

	//----------Temperature & Humi-------//
	UINT 					nHumiTempSensorNumber;
	
	//Version  Info
	BYTE 					szIMEUName[11];
    
    BYTE 					szSN[26];


}A_BLOB_BATTCAB_IMEU2_V20;


typedef struct tagIMEU2CANDataInfo
{
	//must always put first!!!
	BLOB_TYPE	emBlobType;

	//S2C_MSG1	0x14CByyxx
	float	fPT1000[MAX_PT1000_NUM];//PT1000 Temperature

	//S2C_MSG2	0x14CCyyxx
	USHORT	uFloodFreq;		// Frequency of water immersion
	UINT	uPressureSen;		// pressure transmitter sensor
	UINT	uVibrationSen;		// Vibration sensor
	
	//S2C_MSG3 0x14CDyyxx
	float	fTHSenTemp[MAX_TEMPERATURE_HUMIDITY_NUM];	//temperature sensor
	float	fTHSenHum[MAX_TEMPERATURE_HUMIDITY_NUM];	//humidity of sensor
	float	fAirTemp;			//aircondition temperature
	float	fAirHum;			//aircondition humidity

	//S2C_MSG4 0x14CEyyxx
	float	fGasConcent1H2;			//H2 gas
	float	fGasConcent2C0;			//CO gas
	float	fGasConcent3Other;		//other gas
	UINT	uGasFanSpeed;			//Gas Fan Speed
	
	//S2C_MSG6 0x14D0yyxx
	BYTE 	byDI1FireAlarm;
	BYTE 	byDI2SmokeAlarm;
	BYTE 	byDI3DoorOpenAlarm;
	BYTE 	byDI4FlammableGasAlarm;
	BYTE	byDI5DumpAlarm;
	BYTE 	byDI6ShutterClosedAlarm;	//close not in place
	BYTE 	reserve;	
	BYTE 	byFloodAlarm;
	BYTE	byTHSenCommuAlarm[MAX_TEMPERATURE_HUMIDITY_NUM];	//temperature humidity sensor communicaton alarm
	BYTE	byGasSenCommuAlarm[MAX_GAS_SENSOR_NUM];				//gas sensor communicaton alarm
	BYTE	byPT1000ShutterOpenedStatus;						//0:normal 1:open not in place
	BYTE	byCtrlDoLighting;									//Lighting on status
	BYTE	byCtrlDoZHSAlarm;									//audible and visual alarm
	BYTE	bySPDAlarm;											//Lightning protection alarm status				


	//S2C_MSG11	0x14D5yyxx
	BYTE 	reserve1;
	BYTE	byTempHumiNum;				
	BYTE	byComgasEnable;			//Combustiblegas enable
	BYTE	byComgasManu;			//Combustiblegas manufacturer
	BYTE	byComgasNum;			//Combustiblegas number
	BYTE	byFireType;
	BYTE	byHydrogenRemovalIntervalDay;
	BYTE	HydrogenRemovalIntervalMinute;

	//S2C_MSG12	0x14D6yyxx
	BYTE	byLouverActioTime;


	//S2C_MSG13	0x14D7yyxx
	BYTE	byHardwareVerH[MAX_MSG13_SUBID_NUM];
	BYTE	byHardwareVerL[MAX_MSG13_SUBID_NUM];
    BYTE	bySoftwareVerH[MAX_MSG13_SUBID_NUM];
	BYTE	bySoftwareVerL[MAX_MSG13_SUBID_NUM];

	BYTE 	byIMEU2CommFail;

}A_BLOB_IMEU2_CAN;

typedef union tagIFYBattDIDO_Info
{
	BYTE	szByDIDOStat[CAN_DATA_AREA_LEN];
	struct
	{
		//DI STATUS
		BYTE	DI_Breaker 				: 1;
		BYTE	DI_Fuse 				: 1;
		BYTE	DI_Smoke 				: 1;
		BYTE	DI_EPOButtom 			: 1;
		BYTE	DI_FireAlm 				: 1;
        BYTE	DI_GasAlm 				: 1;
		BYTE	DI_1 					: 1;
		BYTE	DI_2 					: 1;
        BYTE	DI_3 					: 1;
		BYTE	DI_4 					: 1;
        BYTE	DI_5 					: 1;
		BYTE	DI_6 					: 1;
        BYTE	DI_7 					: 1;
		BYTE	DI_MstPosContactorLS	: 1;
		BYTE	DI_MstNegContactorLS	: 1;
		BYTE	DI_FanContactorHS		: 1;
		
		//DO STATUS
		BYTE	DO_MainPosHighSideDrv 		: 1;
		BYTE	DO_PreChgHighSideDrv 		: 1;
		BYTE	DO_MainNegHighSideDrv 		: 1;
		BYTE	DO_PowerContactor 			: 1;
		BYTE	DO_RunnLEDPower				: 1;
		BYTE	DO_FaultLEDPower			: 1;
		BYTE	DO_SmokeSensorPower			: 1;
		BYTE	DO_HeatPowerRelay			: 1;
		BYTE	DO_FanPower 				: 1;
		BYTE	DO_Rsrve 					: 1;
		BYTE	DO_OutputRelay_1 			: 1;
		BYTE	DO_OutputRelay_2 			: 1;
		BYTE	DO_OutputRelay_3 			: 1;
		BYTE	DO_MainPosLowSideDrv 		: 1;
		BYTE	DO_MainNegLowSideDrv 		: 1;
		BYTE	DO_NegGroundRelay			: 1;

	}DIDO;

}IFY_BATT_DIDO_STAT;


typedef union tagIFYBattAlarmCode
{
	BYTE		szByIfyAlarm[CAN_DATA_AREA_LEN];
	UINT 		uFlag[2];
	struct
	{
		BYTE	CellVoltHighAlmLv2 : 1;
		BYTE	CellVoltLowAlmLv2 : 1;
		BYTE	BattTotalVoltHighAlmLv2 : 1;
		BYTE	BattTotalVoltLowAlmLv2 : 1;
        BYTE	ChgTempHighAlmLv2 : 1;
        BYTE	ChgTempLowAlmLv2 : 1;
		BYTE	DischgTempHighAlmLv2 : 1;
        BYTE	DischgTempLowAlmLv2 : 1;
		BYTE	ChgOverCurrAlmLv2 : 1;
        BYTE	DischgOverCurrAlmLv2 : 1;
		BYTE	PeripheralsTempHighAlmLv2 : 1;
		BYTE	BattInsulationAlmLv2 : 1;
		BYTE	CellVoltDiffBigAlmLv2 : 1;
		BYTE	CellTempDiffBigAlmLv2 : 1;
		BYTE	BattSocHighAlmLv2 : 1;
		BYTE	BattSocLowAlmLv2 : 1;

		BYTE	CellVoltHighAlmLv1 : 1;
		BYTE	CellVoltLowAlmLv1 : 1;
		BYTE	BattTotalVoltHighAlmLv1 : 1;
		BYTE	BattTotalVoltLowAlmLv1 : 1;
		BYTE	ChgTempHighAlmLv1 : 1;
        BYTE	ChgTempLowAlmLv1 : 1;
		BYTE	DischgTempHighAlmLv1 : 1;
        BYTE	DischgTempLowAlmLv1 : 1;
		BYTE	ChgOverCurrAlmLv1 : 1;
        BYTE	DischgOverCurrAlmLv1 : 1;
		BYTE	PeripheralsTempHighAlmLv1 : 1;
		BYTE	BattInsulationAlmLv1 : 1;
		BYTE	CellVoltDiffBigAlmLv1 : 1;
		BYTE	CellTempDiffBigAlmLv1 : 1;
		BYTE	BattSocHighAlmLv1 : 1;
		BYTE	BattSocLowAlmLv1 : 1;

		BYTE	BattSocDisplayAbnor : 1;
		BYTE	BattCapacityCheckAbnor : 1; 
		BYTE	PCSChgPwrRespTimeOut : 1;
		BYTE	PCSDischgPwrRespTimeOut : 1;
		BYTE	BalanceBoardTempAbnor : 1;
        BYTE	PowerSupplyBoardTempAbnor : 1;
        BYTE	Reserve1 : 1;
		BYTE	Reserve2 : 1;
        BYTE	Reserve3 : 1;
		BYTE	Reserve4 : 1;
        BYTE	Reserve5 : 1;
		BYTE	Reserve6 : 1;
		BYTE	Reserve7 : 1;
		BYTE	Reserve8 : 1;
		BYTE	Reserve9 : 1;
		BYTE	Reserve10 : 1;


	}Alarm;

}IFY_BATT_ALARM_CODE;

typedef union tagIFYBattFaultCode
{
	BYTE		szByIfyFault[CAN_DATA_AREA_LEN];
	UINT 		uFlag[2];
	struct
	{
		BYTE	SlaveBoardBackCheckFailed : 1;
		BYTE	SlaveBalanceCtrlFailed : 1;
		BYTE	SlaveReserveFaultBit1 : 1;
		BYTE	SlaveReserveFaultBit2 : 1;
		BYTE	SlaveSampleCircuitAbnor : 1;
        BYTE	SlaveVoltSampleDisconnect : 1;
		BYTE	SlaveTempSampleDisconnect : 1;
		BYTE	SlaveCellInfoMiss : 1;
        BYTE	SlaveCascadeAbnor : 1;
		BYTE	SlaveCommFailed : 1;
        BYTE	PeripheralsCommFailed : 1;
		BYTE	MasterCommFailed : 1;
        BYTE	MasterDevAddrConflict : 1;
		BYTE	MasterMainMCUAbnor : 1;
		BYTE	MasterSoftwareEPOCtrl : 1;
		BYTE	MasterInterAutoProtect : 1;

		BYTE	MasterRTCAbnor : 1;
		BYTE	MasterEEPROMAbnor : 1;
		BYTE	MasterFLASHAbnor : 1;
		BYTE	MasterCurrSampCircuitAbnor : 1;
		BYTE	MasterHighVoltSampCircAbnor : 1;
		BYTE	MasterLowVoltSampCircAbnor : 1;
		BYTE	MasterExternTempSampCircAbnor : 1;
		BYTE	MasterInsuCheckCircAbnor : 1;
		BYTE	MasterKM1BackCheckAbnor : 1;
		BYTE	MasterKM2BackCheckAbnor : 1;
		BYTE	MasterKM3BackCheckAbnor : 1;
		BYTE	MasterKM4BackCheckAbnor : 1;
		BYTE	MasterKM5To8BackCheckAbnor : 1;
		BYTE	MasterKM9BackCheckAbnor : 1;
		BYTE	MasterLS1BackCheckAbnor : 1;
		BYTE	MasterLS3BackCheckAbnor : 1;

		BYTE	BattMainPosContactorAbnor : 1;
		BYTE	BattPrechgContactorAbnor : 1;
		BYTE	BattNegContactorAbnor : 1;
		BYTE	BattFloodAlarm : 1;
		BYTE	BattFuseMelt : 1;
        BYTE	BattSmokeAlarm : 1;
		BYTE	BattPrechargeNotFull : 1;
		BYTE	BattPrechargeOverTemp : 1;
        BYTE	BattStartupFault : 1;
		BYTE	BattWithLoadTimeout : 1;
        BYTE	BattStopFault : 1;
		BYTE	BattPressEPOStop : 1;
        BYTE	BattBreakerNotClose : 1;
		BYTE	BattInterExternVoltDiffBig : 1;
		BYTE	ContactorWithLoadClose : 1;
		BYTE	ContactorWithLoadOpen : 1;

		BYTE	SOSFaultAlarmCode : 1;
		BYTE	CellVoltReachesExtreme : 1;
		BYTE	CellVoltDiffLarge : 1;
		BYTE	CellVoltDropLarge : 1;
		BYTE	CellTempReachesExtreme : 1;
		BYTE	CellTempDiffLarge : 1;
		BYTE	CellTempRiseFast : 1;
		BYTE	BalancePlateTempReachExtreme : 1;
		BYTE	PowerBoardTempReachExtreme : 1;
		BYTE	CurrInstantReachExtreme : 1;
		BYTE	StandbyCurrAbnormal : 1;
		BYTE	TotalBattCapDecay : 1;
		BYTE	SameCellChrgAndDischrgProt : 1;
		BYTE	HeatDissiptFailofSameCell : 1;
		BYTE	DODnotMetStd4LongTime : 1;
		BYTE	ChrgDischrgEffinotUpStd : 1;

	}Falut;

}IFY_BATT_FAULT_CODE;

//Bolow Status is reported by BMS
typedef enum tagYFYBattSystemState
{
	YFYBATT_SYSSTAT_HV_DOWN = 0,
	YFYBATT_SYSSTAT_HV_UP,
	YFYBATT_SYSSTAT_CHARGE_FULL,
	YFYBATT_SYSSTAT_DISCHARGE_FULL,
	YFYBATT_SYSSTAT_CHG_DISC_FORBID,
	YFYBATT_SYSSTAT_HV_DOWN_PROTECT,
	
 }YFY_BATT_SYS_STATE;

typedef enum tagYFYBattChargeState
{
	YFYBATT_CHGSTAT_IDLE = 0,
	YFYBATT_CHGSTAT_CHARGE,
	YFYBATT_CHGSTAT_DISCHARGE,
	
}YFY_BATT_CHG_STATE;

typedef enum tagYFYBattWorkState
{
	YFYBATT_WORK_PWRON = 0,
	YFYBATT_WORK_STANDBY,
	YFYBATT_WORK_STARTUP,
	YFYBATT_WORK_RUN,
	YFYBATT_WORK_SHUTDOWN,
	YFYBATT_WORK_FAULT,
	YFYBATT_WORK_PWROFF,

}YFY_BATT_WORK_STATE;

typedef enum tagYFYBattPowerOnStat
{
	YFYBATT_POWERON_NORMAL = 0,
	YFYBATT_POWERON_FAIL,
 	
}YFY_BATT_POWERON_STAT;

typedef enum tagBattFullyChgDischgState
{
	FULLY_TEST_IDLE = 0,
    // FULLY_WAIT_TEST,
    FULLY_CHG_START, // Start charging from soc 0%.
    FULLY_CHG_COMPLETE, // Stop charging at soc 100% && started from 0%! (Fully charged 0->100)
    FULLY_DISCHG_START, // Start discharging from soc 100%
    FULLY_DISCHG_COMPLETE, // Stop discharging at 0% %% started from 100% (Fully discharged 100->0)

}BATT_FULLY_CHGDISCHG_STATE;

typedef struct tagBlobIFYBMSInfo
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;	

	//--- 0x1432ffxx ---//
	YFY_BATT_SYS_STATE 		emBattSysState;
	YFY_BATT_CHG_STATE 		emBattChgState;
	YFY_BATT_POWERON_STAT 	emPowerONStatu;

	//--- 0x1433ffxx ---//
    float               	fAllow_MaxChargeCurr;
    float               	fAllow_MaxDisChargeCurr;
    float               	fAllow_MaxChargePow;
    float               	fAllow_MaxDisChargePow;
	
	//--- 0x1434ffxx  ---//
    float             		fAllow_LmtChargeVolt;
    float               	fAllow_LmtDisChargeVolt;

	//--- 0x1435ffxx  ---//
	IFY_BATT_ALARM_CODE 	unioBattAlm;

	//--- 0x1436ffxx  ---//
	IFY_BATT_FAULT_CODE 	unioBattFault;

	//--- 0x1438ffxx  ---//
 	IFY_BATT_DIDO_STAT		unioBattDIDOStat;

 
	//--- 0x143cffxx  ---//
	float               	fPackTotalVolt;
    float               	fPackTotalCurr;

	float					fOutputPower;
	float					fOutsideVolt;

	//--- 0x143dffxx  ---//
	float					fSOC;
	float					fSOE;
 	float					fSOH;
	float					fTotalCapacity;

	//--- 0x143effxx  ---//
	float					fCellAddTotalVolt;
	USHORT					usCellAverageVolt;  //mv
	SHORT					sCellAverageTemp;
	SHORT					sBattCabinetTemp;

	//--- 0x143fffxx  ---//
	USHORT					usMaxCellVlot_Cell; 
	USHORT					usMaxCellVlot_Volt; //mv
	USHORT					usMinCellVlot_Cell; 
	USHORT					usMinCellVlot_Volt; //mv

	//--- 0x1440ffxx  ---//
	USHORT					usMaxCellTemp_Cell;
	SHORT					sMaxCellTemp_Temp;
	USHORT					usMinCellTemp_Cell;
	SHORT					sMinCellTemp_Temp;

	//--- 0x1442ffxx ---//
 	float					fNegativeBusVolt;

	//--- 0x1443ffxx  --//
	float					fInsulation_P_R;
	float					fInsulation_N_R;

	//--- 0x1446ffxx ---//
	USHORT					usSystemRunTime;
	USHORT					usCanChargeTime;
	USHORT					usCanDischargeTime;
	USHORT					usCycleTimes;

	//--- 0x1447ffxx ---//
	USHORT					usMonAlarmNum;
	USHORT					usMonFaultNum;
	USHORT					usAccuAlarmNum;
	USHORT					usAccuFaultNum;

	//--- 0x1448ffxx  ---//
	float					fRemainEnergy;
	float					fTotalEnergy;
	float					fSingleChgCapacity;
	float					fSingleDischgCapacity;
	
	//---0x1449ffxx  -----//
	float					fAccuChgCapacity;
	float					fAccuDischgCapacity;
	
	//---0x144affxx  -----//
	float					fAccuChgEnergy;
	float					fAccuDischgEnergy;
	
	//---0x144bffxx  -----//
	float					fNowDOD;
	float					fDischgStartDOD;

	
	//---0x144cffxx  -----//
	USHORT					usDiagSOSLev;
	USHORT					usDiagSOSCode;

	//----- Cell Balance Status -----//
	//---0x16ppffxx 
	//USHORT					usCellTotalNum;  //can be used as BMS Cell Number
	BYTE                    byCellBalanceStat[A_CABINET_CELL_MAX_NUM]; //From 1~640 Of Battery Cell Unbalance
	BYTE                    byCellSafeStat[A_CABINET_CELL_MAX_NUM];
	
		//-----    Cell Voltage     -----//
	USHORT                  usCellVolt[A_CABINET_CELL_MAX_NUM]; //From 1~240 Of Battery Cell Volt

	//-----   Cell Temperature  -----//
	float                   cCellTemp[A_CABINET_CELL_MAX_NUM];//From 1~240 of Battery Cell Temperature

	float 					fCellRes[A_CABINET_CELL_MAX_NUM];
	USHORT 					usCellZrRes[A_CABINET_CELL_MAX_NUM];
	USHORT 					usCellZiRes[A_CABINET_CELL_MAX_NUM];

	//--- 0x0301ffxx  -----//
	BYTE					byBMS_Alive;

	//--- 0x1028ffxx  -----//
	USHORT 					usCellVoltTotalNum;
	USHORT 					usCellTempTotalNum;
	float					fStandTotalVoltage;
	float					fStandCapacity;
	float					fStandEnergy;
	BYTE					bySystemVersion[8];
	BYTE					bySoftwareVersion[8];
	BYTE					byHardwareVersion[8];
	
 	UINT					uiPermissionLevel;
	
	//--- 0x14e1ffxx  -----//
	float					fLeakageCurrent;//xsy
	float					nSoftVer;

	//edit by Kuangyue 20241202  S2C_LONGALARM  0x1035ffxx 
    UINT 						nAlarmInfo1;     //告警信息1
    UINT 						nAlarmInfo2;     //告警信息2
    UINT 						nAlarmInfo3;     //告警信息3

    UINT 						nAlarmInfo4;     //告警信息4
    UINT 						nAlarmInfo5;     //告警信息5
    UINT 						nAlarmInfo6;     //告警信息6

	UINT 						nAlarmInfo7;     //告警信息7

	//--- 0x16ssffxx  -----//
	BYTE					byCellAFEWorkMode[A_CABINET_CELL_MAX_NUM];//XSY250417
	BOOL					bAfeWorkModeAbnormal; // 0-normal, 1-abnormal
	//--- 0x1062ffxx  -----//
	USHORT					usNewCellAFESrStatus[A_CABINET_CELL_MAX_NUM];
	USHORT					usOldAFEAbnormalStatus[A_CABINET_CELL_MAX_NUM];


	// ---------------------------------- D9 Platform -----------------------------
	BYTE    byCellBalanceStat_836[BATT_CELL_MAX_NUM];      // 均充状态：0=空闲, 1=均充
    BYTE    byCellDischargeStat_836[BATT_CELL_MAX_NUM];    // 均放状态：0=空闲, 1=均放
    USHORT  usCellVolt_836[BATT_CELL_MAX_NUM];             // 单体电压，u16，单位1mV
    SHORT   sCellTemp_836[BATT_CELL_MAX_NUM];              // 单体温度，s16，单位0.1℃
    UINT   	uiCellResReal_836[BATT_CELL_MAX_NUM];          // 内阻实部，u32
    UINT  	uiCellResImag_836[BATT_CELL_MAX_NUM];          // 内阻虚部，u32
    BYTE    byCellIcStat[BATT_CELL_MAX_NUM];           // IC状态，u8 4bit 代表一个AFE IC状态

}A_BLOB_IFY_BMS_DATAINFO;

/*
typedef struct tagPCSSettParam
{
	UINT	emDCorACStartRun;
	UINT	emSetNoisyMode;
	float	fRatedPhaseVolt;
	float	fRatedfreq;
	float 	fPowerFactorAdj;
	UINT 	emWorkMode;

}A_BLOB_PCS_SETTINGPARAM;
*/


typedef struct tagCCU2PCS_CanMsg
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;	

	time_t 					tm_Recv;

	UINT 					nTotalMsgNum;

    BYTE 					byCanMsg[MAX_CAN_CYCLETIMES][MAX_CAN_MSGLEN];

}A_BLOB_CCU2PCSCAN_MSG;

//#pragma pack(1)
//move by lpl 240926 for event/rs485_whim_sampler_main/uart_server_main
typedef enum whMeter_Roughs
{
	//AC
	WHMETER_ROUGH_RTkWh_AC = 0,
	WHMETER_ROUGH_RTkvar,
	WHMETER_ROUGH_RTkvar2,
	
	WHMETER_ROUGH_RT_VOLT_A,
	WHMETER_ROUGH_RT_VOLT_B,
	WHMETER_ROUGH_RT_VOLT_C,
	WHMETER_ROUGH_RT_CURR_A,
	WHMETER_ROUGH_RT_CURR_B,
	WHMETER_ROUGH_RT_CURR_C,

	WHMETER_ROUGH_RTACTPower,
	WHMETER_ROUGH_ACTPower_PhA,
	WHMETER_ROUGH_ACTPower_PhB,
	WHMETER_ROUGH_ACTPower_PhC,

	WHMETER_ROUGH_RTReActPower,
	WHMETER_ROUGH_ReActPower_PhA,
	WHMETER_ROUGH_ReActPower_PhB,
	WHMETER_ROUGH_ReActPower_PhC,

	WHMETER_ROUGH_RT_RVSkWh_AC,
	WHMETER_ROUGH_RT_PowerFactor,
	WHMETER_ROUGH_PowerFactor_PhA,
	WHMETER_ROUGH_PowerFactor_PhB,
	WHMETER_ROUGH_PowerFactor_PhC,


	WHMETER_ROUGH_RT_Freq,
	WHMETER_ROUGH_PMAXDEMAND,

	WHMETER_ROUGH_LAST1_POS_SUM,
	WHMETER_ROUGH_LAST1_POS_SHARP,
	WHMETER_ROUGH_LAST1_POS_PEAK,
	WHMETER_ROUGH_LAST1_POS_FLAT,
	WHMETER_ROUGH_LAST1_POS_WAVE,

	WHMETER_ROUGH_LAST1_NEG_SUM,
	WHMETER_ROUGH_LAST1_NEG_SHARP,
	WHMETER_ROUGH_LAST1_NEG_PEAK,
	WHMETER_ROUGH_LAST1_NEG_FLAT,
	WHMETER_ROUGH_LAST1_NEG_WAVE,

	WHMETER_ROUGH_LAST2_POS_SUM,
	WHMETER_ROUGH_LAST2_POS_SHARP,
	WHMETER_ROUGH_LAST2_POS_PEAK,
	WHMETER_ROUGH_LAST2_POS_FLAT,
	WHMETER_ROUGH_LAST2_POS_WAVE,

	WHMETER_ROUGH_LAST2_NEG_SUM,
	WHMETER_ROUGH_LAST2_NEG_SHARP,
	WHMETER_ROUGH_LAST2_NEG_PEAK,
	WHMETER_ROUGH_LAST2_NEG_FLAT,
	WHMETER_ROUGH_LAST2_NEG_WAVE,

	WHMETER_ROUGH_SUM_RTkWh_AC ,
	WHMETER_ROUGH_SHARP_RTkWh_AC,
	WHMETER_ROUGH_PEAK_RTkWh_AC,
	WHMETER_ROUGH_FLAT_RTkWh_AC,
	WHMETER_ROUGH_WAVE_RTkWh_AC,
	// WHMETER_ROUGH_RTkWh_AC_5,
	// WHMETER_ROUGH_RTkWh_AC_6,
	// WHMETER_ROUGH_RTkWh_AC_7,
	// WHMETER_ROUGH_RTkWh_AC_8,

	WHMETER_ROUGH_RT_SUM_RVSkWh_AC,
	WHMETER_ROUGH_RT_SHARP_RVSkWh_AC,
	WHMETER_ROUGH_RT_PEAK_RVSkWh_AC,
	WHMETER_ROUGH_RT_FLAT_RVSkWh_AC,
	WHMETER_ROUGH_RT_WAVE_RVSkWh_AC,
	// WHMETER_ROUGH_RT_RVSkWh_AC_5,
	// WHMETER_ROUGH_RT_RVSkWh_AC_6,
	// WHMETER_ROUGH_RT_RVSkWh_AC_7,
	// WHMETER_ROUGH_RT_RVSkWh_AC_8,
	
	WHMETER_ROUGH_CURRENTPOSDEMAND,  

	WHMETER_ERR_WORD, //Received Msg that is not normal
	_MAX_WH_ROUGH
}WH_ROUGH_SIG;

typedef enum tagGridInputLimit_Mode
{
	MODE_DISABLE = 0,
	MODE_FIXED_LIMIT,
	MODE_DYNAMIC_LIMIT
}MODE_GRIDINPUT_LIMIT;

// LJH241202 SEMS
typedef struct tagBlobSEMS_ClientInfoSum
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;

	// 收到server数据; REMS下发后再上报给REMS(250225只开发了104协议-川崎项目)
	EMSCtrlMode                     emEMSCtrlMode_REMS; // EMS控制模式 1001, 3902,
	ACDC_WORKSTATU                  emACDC_WorkStatu_REMS; // LJH251028 REMS,增加ACDCWorkStatu 1001, 3907,
	float                           fActvPwrReq_REMS;   // 有功功率需求 chrg+ dischrg- 1001, 3901, // LJH251028 REMS,只有正数
	BYTE                            bREMS_CommStatus; // 上级EMS控制模式下，上级通信状态 0:正常 1:异常 // XSY20260105
	BYTE                            bStandby_BLL; // LJH250930 加快自动充放响应
	BYTE                            byWorkAsap_SEMS; // bCtrlByREMS || (bEnter_AutoChrg_SEMS | bEnter_AutoDisc_SEMS)
	// LJH251111 AEMS
	BYTE                            bStandby_AEMS;

	float                           fPacload4ChgSum;
	float                           fPacload4DischgSum;

	// B6_LSYS 上报到server数据
	float                   f_PcabMeterSum;
	float                   f_PgridMeterSum;
	
	// float                   fBatt_RealTime_VoltSum;
    // float                   fBatt_RealTime_CurrSum; // chrg+ disc-
	float                   fACDC_RealTime_CurrSum;
	float                   fACDC_RealTime_VoltSum;
	float                   fRealTime_MaxCurrSum;
	float 					fChrgedEnergySum;			//sys current total charged kwh
	
	float                   fMax_DisChrg_PwrAbilitySum; // 业务相关 sys current total max discharge power
	float                   fMax_Chrg_PwrAbilitySum;    //sys current total max charge power
	float                   fMax_DisChrg_PwrAbilitySumA;
	float                   fMax_Chrg_PwrAbilitySumA; // bSystemOK, bStopChrgBySoc_SetSEMS // LJH250930 加快自动充放响应,WorkASAP
	float                   fMax_DisChrg_PwrAbilitySum0;
	float                   fMax_Chrg_PwrAbilitySum0; // LJH250606 added:PCS BMS上报的原始值

	float 					fQ_PCSGrpSum;				//sys PCS Total output reactive power 
	float 					fActvPwr_PCSGrpSum;		//sys PCS Total output active power  （positive ：charge    negative：discharge）
	float 					fSOCSum;			//sys Total soc // LJH250606 fix:主控SOC因无小数部分放电时过早停止
	BYTE                    nSOCMaxSum;
	BYTE                    nSOCMinSum;
	
	BYTE                    byWorkAsapSum; // U6330WorkAsap
	BYTE                    bStandby_BLLSum;
	BYTE                    bStandby_SetSum; // LJH250930 加快自动充放响应,standbySetReq
	BYTE					emACDC_WorkStatuSum;  	//ACDC charge/discharge STATU_ACDC_IDLE = 0,
	BYTE  					emPMCtrlModeSum;     //从机-d运行模式 自动模式-手动模式 STAT_PM_CTRL_MODE
	BYTE  					emCCUStateSum;    	  	//从机系统状态 CCU_STATE
	// // LJH0222 for sems AutoCharging_IfReflux AutoDischarging_IfOverload
	// float                   fTotalPessPlan_ChrgReq;    // 管理计划总充电需求
	// float                   fTotalPessPlan_DischrgReq; // 管理计划总放电需求
} A_BLOB_SEMS_CLIENTINFO_SUM;

// LZY20260115 PID Parameters
typedef struct stPID_Params{
	float Kp;
	float Ki;
	// float Kd;
	float fDead_Band;
	float fProportional_Term_Sepa_Thres;    
	float fIntegral_Term_Sepa_Thres;       
} PID_Params;

// LZY20260115 PID Ctx
typedef struct stPID_Ctx {
	PID_Params stPIDChgParams;
	PID_Params stPIDDischgParams;

	float fClearCompensationThres;
	float fMaxOutput_Lmt;
	float fMinOutput_Lmt;
	float fCompensation_Lmt;
	float fInitCompCoef;

    // --- 运行时状态 (需初始化为0) ---
	float fErrorAvgFilter;
    float fPrevError;
    float fPrevPrevError;
    float fCompensation;
	float fPrevTarget;
	UINT nTargetStableCount_Thres;
	float fTargetStableThres;
} PID_Ctx;

// LZY20260126 PID Enable Status
typedef enum tagPowerCtrlEnableState
{
    POWER_CTRL_DISABLE = 0,
    POWER_CTRL_CHARGE,
	POWER_CTRL_DISCHARGE,
	POWER_CTRL_CAHARGE_DISCHARGE,
}POWER_CTRL_ENABLE_STATE;

typedef struct tagEMSMgmtData
{
    int 							nNodeNum;
	A_BLOB_BRANCH_NODE_CALC_DATA 	calc_Node[MAX_SEMS_CLIENTS_NUM];

	float							fTotalPM_Chrg_PwrAbility;
	float							fTotalPM_DisChrging_PwrAbility;
	// LJH240911 AvgPwrFirst add 累加所有组充放电功率
	float							fTotalPM_Chrg_PwrAbilityAll;
	float							fTotalPM_DisChrging_PwrAbilityAll;
	// 1.EmsPlan & PwrLmt 2.是否紧急放电 3.SEMS Ovld4Dischrg Rflx4Chrg
	STBATT_PRI_WORKMODE				emBatt_WorkModeReq; // 可以删除,使用 planSectorNow.emBatt_WorkModePlan

	BOOL 							bEnterUrgetDischrgMode;
	time_t							tm_LastEnterUrgetMode;
    //系统总的AC输入功率限制, AC与DC负载需求功率
	//float							fMaxRealTimeACMeterCurr;
	//float							fGridACMeter_RT_Power;
	

	float							fGridACInputPwr_Lmt;
	float 							fGridRemainACPwr2Chrg;
	// LJH241015 only for RunData
	float                           fRemainPwr4Chrg;    // 剩余充电功率
	float                           fRemainDemand4Chrg; // 剩余需量

	//float 							fTotal_ACInput_Pwr_Lmt;

	//float 							fOrg_ACLoad_Pwr_Req;
	//float 							fAvg_Org_ACLoad_Pwr_Req; 

    //float 							fTotal_ACLoad_Pwr_Req ;
    //float 							fTotal_DCLoad_Pwr_Req ;

    //系统剩余AC输入功率限制, AC与DC负载功率

   	float 							fAvg_Remain_ChrgingPwr_Lmt; 
	float 							fAvg_Remain_DisChrgPwr_Req; 

	float 							fRemain_ACChrgingPwr_Lmt;
    float 							fRemain_ACLoad_DisChrgPwr_Req; 

   	float 							fAvg_Remain_QoutputLmt; 
    float 							fRemain_QoutputLmt; 


	float 							fChrg_PwrRate;
	float 							fDisChrging_PwrRate;

	UINT 							nChrg_NodeNum;
	UINT 							nDisChrging_NodeNum;

	UINT							nLast_Chrg_NodeNum;
	UINT 							nLast_DisChrging_NodeNum;

	
	//float 							fRemain_DCLoad_Pwr_Req; 

	//------------Added Byg John 20230226------//
	float 							f_PgridMeter;
	float 							f_PcabMeter;
	float 							f_PauxMeter;
	float							f_PauxMeter_3;
	
	float 							f_PmaxGridDemand;// 当前电表最大需量

	float 							f_QgridMeter;
	float 							f_QcabMeter;
	float 							f_QauxMeter;
	float 							f_QauxMeter_3;

	UINT 							nPrio;
	
	float							fPacload4Chg; //This ACLoad Power is used for ESS Chg Calc(Avoid Overload);
	float							fPacload4Dischg;//This ACLoad Power is used for ESS Dischage Calc(Avoid Anti-reflux)
	
	//float 						fAvg_Pacload4Chg; 
	float 							fAvg_Pacload4Dischg; 

	float 							fTotal_PessDischgLmt;
	float 							fTotal_PessChgAvaliable;

	// LZY01508 20260127 PID Control
	float							fTotal_PessDischg_PidOut;
	float 							fTotal_PessChg_PidOut;
	int                             nTargetStableCount;
									 
	float							fQacload4Org;//2023-12-6 added. Origin QacLoad Power
	
	float 							fDirect;

	float 							fTotal_Qess_OutputLmt;//2023-12-6 added. Calc For PCS Allocate Q.
 
	int 							nTMSector;
	// LJH250315 管理计划的当前段 LJH250612 planSector
	EMS_LIMIT_SECTOR                planSectorOrg; // 原计划:planSectorOrg = pCCU->blobEmsCfgPlan.emsLmtSectors[pEMS->nTMSector]
	EMS_LIMIT_SECTOR                planSectorNow; // 跟业务相关
	EMS_LIMIT_SECTOR                planSector4USYSIfAutoChDis; // LJH250910 fix：美华智充-光伏逆流不充/过载不放
	
	//-----------Anti Reflux and OverLoad-----//
    BOOL							bEnterAnti_Reflux;
    time_t							tm_LastEnterReflux;

    BOOL 							bEnter_OverLoad;
    time_t							tm_LastEnterOverLoad;

	// LJH240705 AitiOverLoad HVMeter 240822:跟随高压表最大需量+OA(防超需量模式)
	BOOL 							bEnter_OverLoadHV;
    time_t							tm_LastEnterOverLoadHV;
	// LJH241007 AntiPhase 单相防逆流
	float                           fDiffPwr4Dischrg_SumPhs; // 差值，防逆流时(保护值-单相最小功率值)*3
	float                           fAtRflxMtr_Factor;
	float                           fAtRflxMtr_kWPhs[MAX_PHASE_NUM]; // 防逆流表单相功率值
	float                           fAtRflxMtr_kWPhMin;
	BOOL                            isAtRflxMtrHV; // 防逆流表是否是高压表,高压表只接了两相，默认不接B相
	BOOL							bEnterAnti_Reflux_Phase;
    time_t							tm_LastEnterReflux_Phase;
	// 单相防过载
	float                           fMaxPwr4Chrg_SumPhs; // 按设置的最大电流值计算出来的3相最大功率和
	float                           fAtOvlMtr_Factor;
	float                           fAtOvlMtr_VPhs[MAX_PHASE_NUM];
	float                           fAtOvlMtr_VPhMin; // 单相最小电压
	float                           fAtOvlMtr_APhs[MAX_PHASE_NUM]; // 防过载表的单相电流值
	float                           fAtOvlMtr_APhMax;
	BOOL 							bEnter_OverLoad_Phase;
    time_t							tm_LastEnterOverLoad_Phase;
	// LJH241015 ChrgOrDisChrg
	float                           fAtRflx_kW; // 防逆流功率值
	float                           fAtOvl_kW;  // 防过载功率值
	// ------------- SEMS相关 --------------//
	A_BLOB_SEMS2SLAVE_6330			blobRev6330;
	A_BLOB_SEMS_A6_USYS             blobA6_USYS;
	// 优化Sort_NodeOrderByChrgedEnergy, SortByEng and SortBySoc
	time_t							tm_LastSort;
	BOOL 							bSwapLastSort;
	BYTE                            idxsLastSort[MAX_SEMS_CLIENTS_NUM];
	
	BOOL                            changedBlobEmsCfgPlan;
	// LJH241202 SEMS AutoChrg---
	BOOL                            bAutoChrgEnable; // 自动充电使能 1001, 3301,
	float                           fTargetPwr_4Chrg_SEMS; // 自动充电目标功率值 1001, 3307,
	float                           fTargetPwr_4ChrgOffset_SEMS; // 自动充电目标功率修正值 1001, 3311,
	BOOL                            hasPCSDischrgLast_SEMS;
	BOOL                            bEnter_AutoChrg_SEMS;
	// LJH250930 加快自动充放响应,standbySetReq
	float                           fLtPwr2Standby_4Chrg; // 开始待机功率(小于) 1001, 3317,
	UINT                            uDelayStandby_4Chrg; // 待机延时(充电) 1001, 3318,

	// LJH241202 SEMS AutoDisc---
	BOOL                            bAutoDischrgEnable; // 自动放电使能 1001, 3302,
	float                           fTargetPwr_4Dischrg_SEMS; // 自动放电目标功率值 1001, 3308,
	float                           fTargetPwr_4DischrgOffset_SEMS; // 自动放电目标功率修正值 1001, 3312,
	BOOL                            hasPCSChrgLast_SEMS;
	BOOL                            bEnter_AutoDisc_SEMS;
	// LJH250930 加快自动充放响应,standbySetReq
	float                           fGtPwr2Standby_4Disc; // 开始待机功率(大于) 1001, 3319,
	UINT                            uDelayStandby_4Disc; // 待机延时(放电) 1001, 3320,
	time_t                          tmStandby_4Auto;
	BOOL                            bStandby_Set; // 待机-设置参数 1, 85,
	BOOL                            bStandby_Auto; // 待机-自动充放电
	BOOL                            bStandby_AEMS; // LJH251111 AEMS
	BOOL                            bStandby_BLL; // REMS || SEMS Plan || bStandby_Set || AEMS
	
	// LJH250205 REMS // LJH250313 ClientSum 使用 A_BLOB_SEMS_CLIENTINFO_SUM
	A_BLOB_SEMS_CLIENTINFO_SUM      clientInfoSum;
	time_t                          tmStart0sec;    // 修改管理计划的起始时间
	BOOL                            enableREMSCtrl; // 上级EMS控制使能(REMS) 1001, 3303,
	BOOL                            bCtrlByREMS;    // LJH250902 top#6+REMS 浙江金华-中能电科 祥宏&新电途&朗新网关
	BOOL                            enableAntiOvld_REMS; // 防过载使能(REMS) 1001, 3305,
	BOOL                            enableAntiRflx_REMS; // 防逆流使能(REMS) 1001, 3304,
	BYTE                            nStopSOC_ChrgSet_SEMS; // 充电截止SOC(REMS、自动充电) 1001, 3306,
	BYTE                            nStopSOC_DiscSet_SEMS; // 放电截止SOC(REMS、自动放电) 1001, 3309,
	// LJH250930 加快自动充放响应 bEnter_AutoChrg_SEMS bEnter_AutoDisc_SEMS bCtrlByREMS
	BOOL                            bChrgAsap; // TRUE:server使用fMax_Chrg_PwrAbilitySumA下发, ESS使用nStopSOC_ChrgSet_SEMS
	BOOL                            bDiscAsap;
	BYTE                            byWorkAsap; // 6330下发
	// LJH241202 SEMS cabMeter
	BOOL                            bCabMeter_SEMS; // 没有安装总柜内表，由客户端上报进行累加(如:川崎top#1)
	BOOL                            bGMeter_SEMS;   // 没有安装网侧表，由客户端上报进行累加(如:恒华top#7)
	// LJH250325 AntiOvldRflxByLC
	float 							fTotal_PessDischgLmt_Org;
	float 							fTotal_PessChgAvaliable_Org;

	SORT_METHOD						emSortMethod;// xsy20251220
	WORKMODE_REQ					emLastSocSortWm;
	float							fAllowedReverseValue;//lzx20260126 奥地利认证：允许逆流值设置

	// LZY01508 20260126 Power Ctrl Enable
	POWER_CTRL_ENABLE_STATE 	    emPIDEnableState;        // PID使能
	POWER_CTRL_ENABLE_STATE 	    emHighSensitivityEnable;   // 高灵敏度使能
	// LZY01508 20251226 PID Context
	PID_Ctx                         stPidCtx;                // PID上下文管理
	UINT                            nBattGrpPwrLmt;          // 单组电池限制功率
}EMS_MGMT_STU;

// LJH251106 同步主从参数
typedef union tagHasParamSetCFGS
{
	BYTE data;
	struct
	{
		BYTE bStopSOC_ChrgSet : 1;
		BYTE bStopSOC_DiscSet : 1;
		BYTE bSendCmdByLC1stOnlySet : 1;
		BYTE reserve2 : 5;
	};
} HasParamSetCFGS;

typedef struct tagSEMSPCSInfoData
{
	int 				  	nPCSNum;
	char					szVersion[MAX_PSC_NUM][MAX_BUFFER_LNE];
	char					szBar_code[MAX_PSC_NUM][MAX_BUFFER_LNE]; 
}SEMS_PCS_INFO_DATA; 

typedef struct tagBlobSEMS_ClientInfo
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;
	
	void               *pCCU;
    BOOL                bCommFailSemsClient;
	time_t				tmLastRcvHeatbeat; //last time received LSYS msg

	COMM_PEER_INFO      stClientInfo;
	UINT                idxClientRecv; // 客户端上报的序号
    int                 idxClientEms; // from 0
    HANDLE              hClientPort;
    HANDLE              hLock; //for comm.write

#define MMCLITNT_COMM_STAT_WAITCONNECT   0
#define MMCLITNT_COMM_STAT_NORMAL        1
#define MMCLITNT_COMM_STAT_FAILED        2
    int                 emRunStatus;
    //Notice exit thread
    BOOL                bNeedExitThread;
    HANDLE              hThread;
	BOOL				bClientFirstConn;
	BOOL				bNeedRegister;

	double 				tmLastSentCmd;
	BOOL	 			bDetected_ClientDisconnected;

	//need consider if registered by CFGG
#define CCU_REGISTERED_NONE					0
#define CCU_REGISTERED_OK					1
#define CCU_REGISTERED_ADDR_CONFLICT		2
	UINT				emClient_RegisteredStatus;

	//CCU upgrade flag
#define CCU_TCP_UPGRADESTATUS_IDLE 					0
#define CCU_TCP_UPGRADESTATUS_WAIT_FILETRANS 		1
#define CCU_TCP_UPGRADESTATUS_PENDING_FILETRANS 	2
#define CCU_TCP_UPGRADESTATUS_SUCCESS_FILETRANS 	3
#define CCU_TCP_UPGRADESTATUS_FAILED_FILETRANS 		4
#define CCU_TCP_UPGRADESTATUS_FINISHED		 		5
	BYTE				emCCUTCP_UpgradeStatus;

	//ccu upgrade bin file name
	char					szCCUBinFileName[64];	
	UINT					nBinFile_TotalBytes;
#define DEFAULT_FRMAESIZE_FOR_CCUUPDATE		700	//512 //1024
	UINT					nUsed_SingleFrameSize; //frame size for each frame
	BYTE*					pbyBinBuffer; //bin buffer from reading		
	BYTE					byLastUpdate_Percent;	
	UINT 					nTotal_Pkg2Transfer; //total pkgs that need transferred 
	UINT					nTransferred_Pkg; //already transferred pkgs
	//last time get UpdateCmd from CCU
	time_t					tmLastReceiveCCUUpgdadeCmd;
	double					tmLastSentUSYS;

	EMS_MGMT_STU                    sems;
	A_BLOB_BRANCH_GRP_RECTDATA	    branchNodeData_SEMS; // == pCCU->strLSYSInfo.branchNodeData
	// LJH250930 加快自动充放响应,StopSOC_SetSEMS
	SEMS_61_CFGG_DATA_INFO          CFGG;
	time_t					        tmSyncCFGG;
	SEMS_PCS_INFO_DATA				stPcsInfoData;  //从控下面的版本号和SN吗等信息
} A_BLOB_SEMS_CLIENTINFO;

typedef enum tagDevice_Type
{
	DEFAULT_DEVICE = 0,
	LIQUID_COOLING,
	LIQUID_COOLING_ADD_STS,
	LIQUID_COOLING_BACKUP,
	LIQUID_COOLING_836,

}ESS_DEVICE_TYPE;

typedef struct tagSubDevs_Upgrade_Info
{
	//need upgrade dev numbers
#define			 			UPGRADED_DVE_BatSide     5    //BSU BMU EU3 AIREU3 EU2
#define 					UPGRADED_DEV_ModSide	 7    //(PCS MPPT ABU)*2 IMSU
	BOOL 					bUpgradeFlag;
	UINT					nUpgradedCount;
	UINT 					nDevnums;
	char					szUpgradeFielPath[256];
	char                    file_name[64];
	UINT 					nUpdType;
}SUBDEVS_UPGRADE_INFO;

// LJH250428 LCCfg
typedef struct tagLCCfg
{
	SYSTEM_TYPE                     emSystemType;
	POS_AUXMETER                    emApplyScenery;
	UINT                            idxClientLC;
	LC_TYPE                         lcType; // 本机类型
	LC_TYPE                         lcType_Client; // 从机类型
	BOOL                            bSendCmdByLC1stOnly; // 只由一级主控上报 1001, 3313,
	UINT                            nESCNum;    //  储能柜配置数 1001, 3314,
	UINT                            nBranchNum; // 支路配置数 1001, 3315,

	BOOL                            bCabMeter; // 是否安装了电表
	BOOL                            bGridMeter;
} LCCfg;


// KYA20250609
typedef struct tagBlobCMD151
{
	//must always put first!!!
	BLOB_TYPE				          emBlobType;
	SC6LDEV_CMDHead                   cmdHead;
	SC6_LDEV_101_Data         		  branchDatas[MAX_NUM_BRANCH];
	SC6_LDEV_101_Data            	  SemsCMD101Data[MAX_NUM_ESC];
} A_BLOB_CMD101;
// KYA20250609
typedef struct tagBlobCMD152
{
	//must always put first!!!
	BLOB_TYPE				          emBlobType;
	SC6LDEV_CMDHead                   cmdHead;
	SC6_LDEV_102_Data         		  branchDatas[MAX_NUM_BRANCH];
	SC6_LDEV_102_Data            	  SemsCMD102Data[MAX_NUM_ESC];
} A_BLOB_CMD102;

typedef struct __SEMS_BMS_DATA
{
	int						iDevID;
	int						iBattGroupNum;
	SC6_LDEV_103_Data		bmsData[MAX_SINGLE_STBATT_NUM];
}SEMS_BMS_DATA;

typedef struct tagBlobSEMSCMD103
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;
	SEMS_BMS_DATA			clientBMS[MAX_NUM_ESC];		

} A_BLOB_SEMS_CMD103;


typedef struct __SEMS_POWER_CALC_DATA
{
	int						iDevID;
	int						iBattGroupNum;
	SC6_LDEV_104_Data		powerCalcData[MAX_SINGLE_STBATT_NUM];
}SEMS_POWER_CALC_DATA;

typedef struct tagBlobSEMSCMD104
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;
	SEMS_POWER_CALC_DATA	clientPowerCalc[MAX_NUM_ESC];		

} A_BLOB_SEMS_CMD104;

typedef struct tagBlobCMD156
{
	//must always put first!!!
	BLOB_TYPE				          emBlobType;
	SC6LDEV_CMDHead                   cmdHead;
	A_BLOB_SINGLE_RECT_INFO_106         	  branchDatas[MAX_NUM_BRANCH];
	A_BLOB_SINGLE_RECT_INFO_106            	  SemsCMD106Data[MAX_NUM_ESC];
} A_BLOB_CMD106;

// KYA20250609
typedef struct tagBlobCMD158
{
	//must always put first!!!
	BLOB_TYPE				          emBlobType;
	SC6LDEV_CMDHead                   cmdHead;
	SC6_LDEV_108_Data         		  branchDatas[MAX_NUM_BRANCH];
	SC6_LDEV_108_Data            	  SemsCMD108Data[MAX_NUM_ESC];
} A_BLOB_CMD108;
typedef struct _SESS_ACTIVE_ALARMS
{
	SC6_LDEV_109			activeAlms[MAX_QUERY_ACTV_ALM];
	int						iDevID;	//设备全ID， 如0001002
	int						iAlarmCount;
}SESS_ACTIVE_ALARMS;

typedef struct tagBlobSEMSCMD109
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;
	SESS_ACTIVE_ALARMS 		clientActiveAlarms[MAX_NUM_ESC];

} A_BLOB_SEMS_CMD109;


typedef struct __BATT_GROUP_CELL
{
	USHORT usCellNumPerGroup;
	SC6_LDEV_CELL_110 battCellInfo[A_CABINET_CELL_MAX_NUM];
}BATT_GROUP_CELL;

typedef struct __SEMS_BATT_CELL
{
	BATT_GROUP_CELL			battGroupCell[MAX_SINGLE_STBATT_NUM];
	int						iDevID;	//设备全ID， 如0001002
	int						iBattGroupNum;

}SEMS_BATT_CELL;

typedef struct tagBlobSEMSCMD110
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;
	SEMS_BATT_CELL			clientBattCell[MAX_NUM_ESC];

} A_BLOB_SEMS_CMD110;

typedef struct __SEMS_ELE_STATIST
{
	SC6_LDEV_202			eleStatist;
	int						iDevID;	//设备全ID， 如0001002
}SEMS_ELE_STATIST;

typedef struct tagBlobSEMSCMD202
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;
	SEMS_ELE_STATIST		semsEleStatist[MAX_NUM_ESC];
} A_BLOB_SEMS_CMD202;

typedef struct tagBlobCMD155
{// LJH250428 cmd155 BLOB_TYPE_CMD155
	//must always put first!!!
	BLOB_TYPE				          emBlobType;
	SC6LDEV_CMDHead                   cmdHead;
	SC6LDEV_CMD155_BranchData         branchDatas[MAX_NUM_BRANCH];
	SC6LDEV_CMD155_ESCData            escDatas[MAX_NUM_ESC];
} A_BLOB_CMD155;

typedef struct tagA_BLOB_CMD157_SUM
{// LJH250520 cmd157 BLOB_TYPE_CMD157
	//must always put first!!!
	BLOB_TYPE				          emBlobType;
	SC6LDEV_CMDHead                   cmdHead;
	SC6LDEV_CMD157                    branchDatas[MAX_NUM_BRANCH * 2]; // 二级主控柜内表+网侧表
	SC6LDEV_CMD157                    escDatas[MAX_NUM_ESC]; // ESC的柜内表
} A_BLOB_CMD157;


typedef enum tagPHY_INTER_TYPE
{
	RS485_1 = 0,
	RS485_2,
	UART_SERVER,
	UART_SERVER_2,
	UART_SERVER_3,
	UART_SERVER_4,
	_MAX_PHY_INTER_TYPE
}PHY_INTER_TYPE;

typedef enum tagPVRunStatus
{
	PV_STATUS_NOTINSTALL = 0,
	PV_STATUS_WAITING,
	PV_STATUS_RUNNING,
	PV_STATUS_FAULT,

}PV_RUN_STATUS;
typedef struct __PV_INVERTER_DATA
{
    char 				sn[MAX_SN_LEN];
    char 				szFwVer[MAX_SN_LEN];      //固件版本号
    char 				szCtrlVer[MAX_SN_LEN];     //控制版本号
    char 				szModbusVer[MAX_SN_LEN];  //逆变器modbus版本号
    int 				iRatedPower;                    //额定功率 W
	UINT				nOnOffStatus;				//当前开关机状态  0-关 1-开			
    PV_RUN_STATUS 		iWorkMode;            		//工作状态 0-未安装 1-等待，2-运行 3-故障
    UINT 				uErrCode;              		//故障码
    UINT 				uWarnCode;             		//告警码
   
    float fLineVoltAB;                  //电网线电压AB
    float fLineVoltBC;                  //电网线电压BC
    float fLineVoltCA;                  //电网线电压CA

    float fPhaseVoltA;                  //电网相电压A
    float fPhaseVoltB;                  //电网相电压B
    float fPhaseVoltC;                  //电网相电压C

    float fPhaseCurrA;                  //电网相电流A
    float fPhaseCurrB;                  //电网相电流B
    float fPhaseCurrC;                  //电网相电流C

    float fFreq;                        //电网频率
    float fOutputActivePower;           //逆变器当前输出交流有功功率 W
    int iOutputPowerFactor;             //逆变器当前输出功率因数 
    float fPVInputTotalPower;           //光伏直流输入总功率 W 

    float fInnerTemperature;            //逆变器内部环境温度

    float fPVGroupVolt[MAX_PV_NUM];     //PV组电压 V
    float fPVGroupCurr[MAX_PV_NUM * 2]; //PV组电流 A

    USHORT usYear;                      //本地系统时间  
    USHORT usMonth;
    USHORT usDay;
    USHORT usHour;
    USHORT usMin;
    USHORT usSecond;

    int 	iActivePowerRate;                   //最大输出有功功率百分比
	UINT	nReActivePowerPercen;				//最大输出无功功率百分比	  
    int 	iRealPowerPercent;                 //实际功率百分比  

    float fDailyPowerGeneration;                    //当日发电量 kWh  
    float fTotalPowerGeneration;                    //总共发电量 kWh 
    float fPvGrpDailyPowerGeneration[MAX_PV_NUM];   //PV组当日发电量 kWh  
    float fPvGrpTotalPowerGeneration[MAX_PV_NUM];   //PV组总共发电量 kWh   
    float fPVDCTotalPowerGeneration;                //直流输入总电量kWh

	float fRealTime_MaxActivePower;				//当前交流的最大能力输出 W（PV组串电压*电流）
	UINT  nParallelAntiBackflowEnable;			//多机防逆流开启使能位 1： 使能 0：不使能
	UINT  nAntiBackflowFailureResponseTime;     //防逆流失效响应时间
	UINT  nParallelAntiBackflowPowerLimit;  	//多机防逆流功率限制值设置
	
}PV_INVERTER_DATA;
typedef enum tagPVCtrlMode
{
	CTRL_BY_DATA_ACQUISITION,
	CTRL_BY_AEMS
}PV_CRTL_MODE;

typedef enum tagPVDOSwitchStatus
{
	DI_STATUS_RECYCLE = 0, //回收控制
	DI_STATUS_TRANSFER
}DO_SWITCH_STATUS;

typedef struct tagBolbPVInverterDataTotal
{	
	BLOB_TYPE					emBlobType;
	BOOL 						bEnableManualMode;				//手动控制使能 
	PV_CRTL_MODE				RecvCtrlMode;  					//接受- 0-PV本地数采控制  1-AEMS控制
	DO_SWITCH_STATUS            emDIstatus;						
	UINT						nRecvPVStatus;					//接受-控制光伏模块状态  开关
	float						fRecvActivePower;				//接受-有功功率下发
	float						fRecvReActivePower;				//接受-无功功率下发
	UINT						nAvailablePVNums;				//当前可用的设备数（已安装且没告警）
	float						fTotalPVPowerAbility;  			//当前总的交流最大输出能力(所有PV组串电压*电流) KW
	float						fTotalPVInput;					//光伏直流输入总功率 KW
	float						fTotalPVOutputActPower;			//逆变器当前输出交流有功功率 KW
	float 						fDailyPowerGenerationSum;		//当日发电量 kWh
	float 						fTotalPowerGenerationSum;		//总共发电量 kWh
	float 						fPVDCTotalPowerGenerationSum;   //直流输入总电量kWh
	int 						iTotalRatedPower;				//额定功率 kw
	PV_RUN_STATUS				emPvRunStatus;					//工作状态 0-未安装 1-等待，2-运行 3-故障

}A_BLOB_PV_INVERTER_DATA_TOTAL;
typedef struct tagBolbPVSettingByAEMS
{	
	BLOB_TYPE					emBlobType;
	PV_CRTL_MODE				CtrlMode;  					//0-PV本地数采控制  1-AEMS控制
	UINT						nPVStatus;					//控制光伏模块状态  0-关机  1-开机
	float						fActivePower;				//有功功率下发
	float						fReActivePower;				//无功功率下发

}A_BLOB_PV_CTRL_BY_AEMS;

typedef struct tagBolbPVInverterData
{	
	BLOB_TYPE					emBlobType;
	PV_INVERTER_DATA      		PVInverterData;
}A_BLOB_PV_INVERTER_DATA;

typedef enum tagCustom
{
	CUSTOM_NONE = 0,
	GH_CUSTOM,
}CUSTOM_TYPE;

typedef enum tagControlMode
{
	LOCAL_CTRL_MODE = 2, // Local Controller
	REMOTE_CTRL_MODE = 3, // Remote Controller
}COMTROL_MODE;

typedef enum tagTimeCalibration
{
	TIME__NONE = 0, 
	TIME_IOCPP,  
	TIME_MQTT,
	TIME_IFY,
	TIME_OCPP,  
	TIMT_MBE,
}TIME_CALIBRATION;

typedef struct tagBlobDevInstallConfig
{
	//must always put first!!!
	BLOB_TYPE		emBlobType;
	UINT			AIR[2];
	UINT			PACK[2];
	UINT			Bat_IMEU[MAX_BATTCABINET_NUM];
	UINT			Power_EU3[2];
	UINT			UartServer;
	UINT			TempControllType;
	UINT 			MulCube_EU3[2];
	UINT 			WaterChiller;
	// UINT			PVEnable;
} A_BLOB_DEVINSTALLCONFIG;


typedef struct tagES_DayStatData
{
	time_t			tmTimeStamp_D;
	//365 Days Remote AC Meter RealTime Reading
	float 			fAC_CurrKWH_Reading;
    //365 Days Remote AC Meter Total Cost KWH
    float 			fACTotalCostKWH_D;
	//365 Days Org ACLoad Cost KWH = Remote AC Meter Cost - Local Ac Meter Pos + Local AC Meter Neg
	float 			fOrgACLoad_CostKWH_D;
	//365 Days Remote AC Reverse Output KWH
	float 			fAC_RevrtOutPutKWH_D;
	//365 Days Local AC Meter Input KWH 
	float           fLocalAC_INPUTKWH_D;
	//365 Days Local AC Meter Output KWH 
	float           fLocalAC_OUTPUTKWH_D;
	//365 Days DC Load Cost KWH
	float			fDCLoad_CostKWH_D;	
    //365 Days MPPt Generate KWH
    float 			fMpptTotalGKWH_D[MAX_MPPT_GROUP_NUM];
    //365 Days Battery Grp Chrged KWH
    float 			fBattTotalChrgKWH_D[MAX_SINGLE_STBATT_NUM];
    //365 Days Battery Grp DisChrged KWH
    float 			fBattTotalDisChrgKWH_D[MAX_SINGLE_STBATT_NUM];
	//365 Days Battery Grp Chrged Times
	float 			fBattChrgTimes_D[MAX_SINGLE_STBATT_NUM];
    //365 Days Battery Grp DisChrged Times
    float 			fBattDisChrgTimes_D[MAX_SINGLE_STBATT_NUM];
	//365 Days Peak Flat Valley Chrged/DisChrged KWH
	float			fLocalAC_Sharp_InputKWH_D;
	float			fLocalAC_Peak_InputKWH_D;
	float			fLocalAC_Flat_InputKWH_D;
	float			fLocalAC_Valley_InputKWH_D;
	float			fLocalAC_Sharp_OutPutKWH_D;
	float			fLocalAC_Peak_OutPutKWH_D;
	float			fLocalAC_Flat_OutputKWH_D;
	float			fLocalAC_Valley_OutputKWH_D;
	float 			fOrgACLoad_Sharp_CostKWH_D;
	float 			fOrgACLoad_Peak_CostKWH_D;
	float 			fOrgACLoad_Flat_CostKWH_D;
	float 			fOrgACLoad_Valley_CostKWH_D;
	float 			fMppt_Sharp_GKWH_D[MAX_MPPT_GROUP_NUM];
	float 			fMppt_Peak_GKWH_D[MAX_MPPT_GROUP_NUM];
	float 			fMppt_Flat_GKWH_D[MAX_MPPT_GROUP_NUM];
	float 			fMppt_Valley_GKWH_D[MAX_MPPT_GROUP_NUM];
	float			fResv1_KWH_D;
	//365 Days Off-Grid Load Meter Cost KWH
	float 			fOffGridLoad_CostKWH_D;
	BOOL			bReportedOK;

}ES_DAY_STAT_DATA; 

#define GetFeeRateKWHDevNum 2
#define GetFeeRateKWHTimeNum 3  //current,last1,last2
typedef struct tagFeeRate
{
	float fPosSumKwh[GetFeeRateKWHTimeNum][10] ;
	float fNegSumKwh[GetFeeRateKWHTimeNum][10] ;
	float fPosSharpKwh[GetFeeRateKWHTimeNum][10] ;
	float fNegSharpKwh[GetFeeRateKWHTimeNum][10] ;
	float fPosPeakKwh[GetFeeRateKWHTimeNum][10] ;
	float fNegPeakKwh[GetFeeRateKWHTimeNum][10] ;
	float fPosFlatKwh[GetFeeRateKWHTimeNum][10] ;
	float fNegFlatKwh[GetFeeRateKWHTimeNum][10] ;
	float fPosWaveKwh[GetFeeRateKWHTimeNum][10] ;
	float fNegWaveKwh[GetFeeRateKWHTimeNum][10] ;
	
}FEE_RATE_KWH;
typedef struct tagBlobType_DataStorage
{
	BLOB_TYPE		emBlobType;
	//202
	//204
	time_t				 	t_SaveTime;
	ES_24HOUR_STAT_DATA     hours_Data;
	ES_DAY_STAT_DATA        days_Data;

	//2o6  
	FEE_RATE_KWH 		    feeRateKWH[GetFeeRateKWHDevNum]; //407, 401

}A_BLOB_TYPE_DATASTORAGE;

typedef struct tagCloudUpload
{
#define MAX_CLOUDUPLOAD_INTERNAL 2
	BOOL						    bUploadEnable;
	UINT						    nSaveCurrtNum;
	UINT						    nSaveTotalNum;
	time_t						    tmLastUpload;
	time_t						    tmSaveNumber[MAX_CLOUDSAVE_NUM];
}CLOUD_UPLOAD;

typedef struct tagBlobCellBarcode {
	BYTE byRawCellBarcode[15][52][24];      // 15个Pack × 52电芯 × 24字节
    char cASCIICellBarcode[15][52][25];     // 15个Pack × 52电芯 × 25字符

	BYTE byRawPackBarcode[15][20];      // 15个Pack × 20字节
    char cASCIIPackBarcode[15][21];     // 15个Pack × 21字符

    BYTE byRawBSUBarcode[15][23];      // 15个Pack × 23字节
    char cASCIIBSUBarcode[15][24];     // 15个Pack × 24字符

	BYTE byPermissionStatus;      // 0密码错误 1开启写入权限 2关闭写入权限
} A_BLOB_CELL_BARCODE;



/* ================================================================
 * 电量能量统计信息 - 整合主索引52、55、56、62
 * 来源: YFY_BMU02_核间通信通信协议表(03整体信息)
 * 单位规则: NA/整数单位→USHORT, 0.1等小数单位→float
 * ================================================================ */

/* 主索引52: 电量信息 */
typedef struct tagPowerInfo
{
    UINT  soc;                            /* 电池SOC值 [1E-3, 0-1000] 千分比 */
    UINT  soe;                            /* 电池SOE值 [1E-3] 千分比 */
    UINT  soh;                            /* 电池SOH值 [1E-3] 千分比 */
    float   total_capacity;                 /* 电池组总容量 [0.1AH] */
} A_POWER_INFO;

/* 主索引55: 当前能量统计 */
typedef struct tagEnergyCurrStat
{
    float   remaining_energy;               /* 剩余电能 [0.1kWh] */
    float   total_energy;                   /* 总电能 [0.1kWh] */
    UINT  single_charge_energy;           /* 单次充电电量 [1AH] */
    UINT  single_discharge_energy;        /* 单次放电电量 [1AH] */
} A_ENERGY_CURR_STAT;

/* 主索引56: 累计电量统计 */
typedef struct tagEnergyTotalStat
{
    ULONG   total_charge_energy;            /* 累计充电电量 [1AH] */
    ULONG   total_discharge_energy;         /* 累计放电电量 [1AH] */
    float   total_charge_power;             /* 累计充电电能 [0.1kWh] */
    float   total_discharge_power;          /* 累计放电电能 [0.1kWh] */
} A_ENERGY_TOTAL_STAT;

/* 主索引62: 放电深度统计 */
typedef struct tagDodStat
{
    UINT  current_dod;                    /* 当前最新DOD值 [1E-3] 千分比 */
    UINT  start_dod;                      /* 放电起始DOD值 [1E-3] 千分比 */
    UINT  history_max_dod;                /* 历史最大DOD值 [1E-3] 千分比 */
    UINT  history_avg_dod;                /* 历史平均DOD值 [1E-3] 千分比 */
} A_DOD_STAT;

/* 电量能量统计信息结构体 */
typedef struct tagBlobEnergyStat
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;
	
    A_POWER_INFO        power_info[MAX_BATT_GROUP_NUM];         /* 主索引52: 电量信息 */
    A_ENERGY_CURR_STAT  energy_curr_stat[MAX_BATT_GROUP_NUM];   /* 主索引55: 当前能量统计 */
    A_ENERGY_TOTAL_STAT energy_total_stat[MAX_BATT_GROUP_NUM];  /* 主索引56: 累计电量统计 */
    A_DOD_STAT          dod_stat[MAX_BATT_GROUP_NUM];           /* 主索引62: 放电深度统计 */
} A_BLOB_ENERGY_STAT;


/* ================================================================
 * 状态与安全统计信息 - 整合主索引50、51、53、61
 * 来源: YFY_BMU02_核间通信通信协议表(03整体信息)
 * 单位规则: NA/整数单位→USHORT, 0.1等小数单位→float
 * ================================================================ */

/* 主索引50: 状态信息 */
typedef struct tagStatusInfo
{
    UINT  protect_status;              /* 保护状态: 0-未就绪 1-正常 2-充电截止 3-放电截止 4-充放电限制 [NA] */
    UINT  work_status;                 /* 工作状态: 0-开机 1-待命 2-启动 3-运行 4-停机 5-故障 6-关机 [NA] */
    UINT  charge_discharge_status;     /* 充放电状态: 0-空闲 1-充电 2-放电 [NA] */
    UINT  work_mode;                   /* 工作模式: 0-自动 1-手动 2-紧急 5-调试 [NA] */
} A_STATUS_INFO;

/* 主索引51: 全局采样信息 */
typedef struct tagGlobalInfo
{
    float   total_voltage;               /* 总电压采样值 [0.1V] */
    float   total_current;               /* 总电流采样值 [0.1A], 负号表示充电 */
    float   output_power;                /* 输出功率值 [0.1kW], 负号表示充电 */
    float   precharge_voltage;           /* 预充电压值 [0.1V] */
} A_GLOBAL_INFO;

/* 主索引53: 限值信息 */
typedef struct tagLimitInfo
{
    float   max_charge_current;          /* 最大允许充电电流限值 [0.1A] */
    float   max_discharge_current;       /* 最大允许放电电流限值 [0.1A] */
    float   max_charge_power;            /* 最大允许充电功率限值 [0.1kW] */
    float   max_discharge_power;         /* 最大允许放电功率限值 [0.1kW] */
    float   max_charge_voltage;          /* 最大允许充电电压限值 [0.1V] */
    float   max_discharge_voltage;       /* 最大允许放电电压限值 [0.1V] */
} A_LIMIT_INFO;

/* 主索引61: 安全等级统计 */
typedef struct tagSosStat
{
    UINT  current_sos_value;           /* 当前安全等级SOS值 [1E-3] 千分比 */
    UINT  current_sos_reason;          /* 当前SOS等级异常原因, 按Bit位表示 [NA] */
    UINT  predict_sos_value;           /* 预测安全等级SOS值 [1E-3] 千分比 */
    UINT  predict_sos_reason;          /* 预测SOS等级异常原因, 按Bit位表示 [NA] */
} A_SOS_STAT;
typedef struct tagFeatureInfo
{
    float   cell_total_voltage;           /* 单体总电压值 [0.1V] */
    SHORT   cell_avg_voltage;              /* 单体平均电压值 [1mV] */
    float   cell_avg_temperature;          /* 单体平均温度值 [0.1℃] */
    float   hvbox_temperature;             /* 高压箱内温度值 [0.1℃] */
    UINT  max_voltage_index;             /* 单体最大电压序号 [NA] */
    SHORT   max_voltage_value;             /* 单体最大电压值 [1mV] */
    UINT  min_voltage_index;             /* 单体最小电压序号 [NA] */
    UINT   min_voltage_value;             /* 单体最小电压值 [1mV] */
    UINT  max_temperature_index;         /* 单体最高温度序号 [NA] */
    float   max_temperature_value;         /* 单体最高温度值 [0.1℃] */
    UINT  min_temperature_index;         /* 单体最低温度序号 [NA] */
    float   min_temperature_value;         /* 单体最低温度值 [0.1℃] */
} A_FEATURE_INFO;

/* 状态与安全统计信息结构体 */
typedef struct tagBlobStatusStat
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;

    A_STATUS_INFO   status_info[MAX_BATT_GROUP_NUM];     /* 主索引50: 状态信息 */
    A_GLOBAL_INFO   global_info[MAX_BATT_GROUP_NUM];     /* 主索引51: 全局采样信息 */
    A_LIMIT_INFO    limit_info[MAX_BATT_GROUP_NUM];      /* 主索引53: 限值信息 */
    A_FEATURE_INFO  feature_info[MAX_BATT_GROUP_NUM];    /* 主索引54: 特征信息 */
} A_BLOB_STATUS_STAT;


/* ================================================================
 * 特征与诊断统计信息 - 整合主索引54、59、60、63、70
 * 来源: YFY_BMU02_核间通信通信协议表(03整体信息)
 * 单位规则: NA/整数单位→USHORT, 0.1等小数单位→float
 * ================================================================ */

/* 主索引54: 特征信息 */

/* 主索引59: 时间统计 */
typedef struct tagTimeStat
{
    UINT  system_run_time;              /* 系统运行时长 [1Hour] */
    UINT  charge_available_time;        /* 可充电时间 [1Min] */
    UINT  discharge_available_time;     /* 可放电时间 [1Min] */
    UINT  cycle_count;                  /* 充放电循环次数 [1次] */
} A_TIME_STAT;

/* 主索引60: 故障统计 */
typedef struct tagFaultStat
{
    USHORT  monthly_alarm_count;          /* 单月告警数目 [NA] */
    USHORT  monthly_fault_count;          /* 单月故障数目 [NA] */
    USHORT  total_alarm_count;            /* 累计告警数目 [NA] */
    USHORT  total_fault_count;            /* 累计故障数目 [NA] */
} A_FAULT_STAT;

/* 主索引63: 权限状态 */
typedef struct tagAuthState
{
    USHORT  current_auth_level;           /* 当前权限级别: 0-普通 1-运维 2-高级 3-超级 [NA] */
    USHORT  current_auth_port;            /* 当前权限端口: 0-4:CAN 5-9:RS485 10:核间通信 [NA] */
} A_AUTH_STATE;

/* 主索引70: 控制反馈信息 */
typedef struct tagCtrlFeedback
{
    USHORT  insulation_detect_result;     /* 绝缘检测控制结果: 0-自动 1-正在检测 2-停止 3-完成 4-超时 [NA] */
    USHORT  reserved;                     /* 预留 [NA] */
} A_CTRL_FEEDBACK;

/* 主索引86: 不同频点内阻查询反馈信息 */
typedef struct tagResFeedback
{
    USHORT  current_res_freq;     /* 当前内阻频点/频率信息 */
    USHORT  query_stat;                     /* 查询是否完成 0-无 1-完成*/
    USHORT  reserved;                     /* 预留 [NA] */
} A_RES_FEEDBACK;

/* 特征与诊断统计信息结构体 */

typedef struct tagBlobDiagStat
{
	//must always put first!!!
	BLOB_TYPE				emBlobType;

    A_SOS_STAT       sos_stat[MAX_BATT_GROUP_NUM];       /* 主索引61: 安全等级统计 */
    A_TIME_STAT      time_stat[MAX_BATT_GROUP_NUM];      /* 主索引59: 时间统计 */
    A_FAULT_STAT     fault_stat[MAX_BATT_GROUP_NUM];     /* 主索引60: 故障统计 */             
} A_BLOB_DIAG_STAT;

typedef struct tagStatFeedback
{
    A_AUTH_STATE     auth_state;                         /* 主索引63: 权限状态(全局) */
    A_CTRL_FEEDBACK  ctrl_feedback;                      /* 主索引70: 控制反馈(全局) */
    A_RES_FEEDBACK   res_feedback;                       /* 主索引86: 不同频点内阻查询反馈(全局) */
} STAT_FEEDBACK;

typedef struct tagBmsAlarmFault
{
    USHORT alarm_info_1;
	USHORT alarm_info_2;  
	USHORT alarm_info_3;
    USHORT alarm_info_4;  
	USHORT alarm_info_5;  
	USHORT alarm_info_6;
    USHORT alarm_info_7;  
	USHORT alarm_info_8;  
	USHORT alarm_info_9;
    USHORT alarm_info_10;
    USHORT fault_info_1;  
	USHORT fault_info_2;  
	USHORT fault_info_3;
    USHORT fault_info_4;  
	USHORT fault_info_5;  
	USHORT fault_info_6;
    USHORT fault_info_7;  
	USHORT fault_info_8;
} BMS_ALARM_FAULT;

typedef struct tagImiu4Info
{
    int   battery_total_voltage; // todo
    int   link_voltage;
    int   hall_5v_sample;
    float   hall_signal_sample;
    int   hall_current_low;
    int   hall_current_high;
    float   shunt_current_low;
    int   shunt_current_high;
    int   shunt_temperature;
    float   hv_chip_5v_feedback;
    float   hv_chip_minus5v_feedback;
    float   positive_insulation_res;
    float   negative_insulation_res;
    float   ntc1_temperature;
    float   ntc2_temperature;
    int   dido_gpio_signal;
    int   board_5v_feedback;
    int   system_work_mode;
    int   alarm_info_1;
    int   alarm_info_2;
    int   alarm_info_3;
    int   fault_info_1;
    int   fault_info_2;
    int   fault_info_3;
} IMIU4_INFO;

typedef struct tagBsu02SampleInfo
{
    UINT  pt1000_selfcheck_temp_1;
    UINT  pt1000_selfcheck_temp_2;
    UINT  water_selfcheck_freq_1;
    UINT  water_selfcheck_freq_2;
    float   pt1000_5v_pullup_feedback;
    float   twelve_v_output_feedback;
    UINT  pt1000_temp_1;
    UINT  pt1000_temp_2;
    UINT  water_freq_1;
    UINT  water_freq_2;
    float   hv_interlock_pwm_amplitude;
    float   hv_interlock_pwm_duty;
    float   fg_pwm_amplitude;
    float   fg_pwm_duty;
    float   aerosol_concentration;
    UINT  aerosol_sensor_type;
    UINT  aerosol_status_fault;
    UINT  afe_bot_enum_count;
    UINT  afe_top_enum_count;
    UINT  reserved_19;
    UINT  reserved_20;
    UINT  reserved_21;
    UINT  reserved_22;
    UINT  reserved_23;
    UINT  reserved_24;
    UINT  reserved_25;
    UINT  reserved_26;
    UINT  reserved_27;
} BSU02_SAMPLE_INFO;

typedef struct tagHeartbeatInfo
{
    USHORT  heartbeat_hw_version;
    USHORT  device_id;
    USHORT  sw_version_l;
    USHORT  sw_version_h;
} HEARTBEAT_INFO;

typedef struct tagHisFaultRecord
{
    USHORT  fault_type;
    USHORT  fault_code;
    USHORT  time_year;
    USHORT  time_month_day;
    USHORT  time_hour_minute;
    USHORT  time_second;
    USHORT  aux_info_1;
    USHORT  aux_info_2;
    USHORT  aux_info_3;
    USHORT  aux_info_4;
    USHORT  aux_info_5;
} HIS_FAULT_RECORD;

typedef struct tagHisEventRecord
{
    USHORT  event_type;
    USHORT  event_code;
    USHORT  time_year;
    USHORT  time_month_day;
    USHORT  time_hour_minute;
    USHORT  time_second;
    USHORT  aux_info_1;
    USHORT  aux_info_2;
    USHORT  aux_info_3;
    USHORT  aux_info_4;
    USHORT  aux_info_5;
} HIS_EVENT_RECORD;

typedef struct tagHisDodRecord
{
    USHORT  seq_id;
    USHORT  time_year;
    USHORT  time_month_day;
    USHORT  time_hour_minute;
    USHORT  time_second;
    USHORT  dod;
    USHORT  dischg_qty;
    USHORT  chrg_qty;
    USHORT  dischg_energy;
    USHORT  chrg_energy;
} HIS_DOD_RECORD;

/* SOS安全等级历史记录(R核mIndex17/18推送,与DOD记录同构) */
typedef struct tagHisSosRecord
{
    USHORT  seq_id;             /* 序号(每簇自增,u16回绕) */
    USHORT  time_year;
    USHORT  time_month_day;
    USHORT  time_hour_minute;
    USHORT  time_second;
    USHORT  sos_level;          /* SOS安全等级[0,5] */
    USHORT  reason_bitmap;      /* 异常原因位图(按bit) */
    USHORT  event_type;         /* 事件类型(1:产生 2:消除 0:其它) */
    USHORT  reserved_1;
    USHORT  reserved_2;
} HIS_SOS_RECORD;

typedef struct tagBoardSampleInfo
{
    USHORT  vcc_1v8_feedback;
    USHORT  vcc_3v3_feedback;
    USHORT  v_4g_3v5_feedback;
    USHORT  vcc_5v_feedback;
    USHORT  vcch_5v_feedback;
    USHORT  vcc_12v_feedback;
    USHORT  temp_ad;
    USHORT  temp_ad1;
    USHORT  gpio_output;
    USHORT  gpio_input;
    USHORT  shiftreg_output;
    USHORT  shiftreg_input_1;
    USHORT  shiftreg_input_2;
} BOARD_SAMPLE_INFO;

typedef struct tagImeu3SampleInfo
{
    USHORT  ntc1_temp;
    USHORT  ntc2_temp;
    USHORT  ntc3_temp;
    USHORT  ntc4_temp;
    USHORT  ntc5_temp;
    USHORT  ntc6_temp;
    USHORT  ntc7_temp;
    USHORT  ntc_or_pt1000_8_temp;
    USHORT  ntc_or_pt1000_9_temp;
    USHORT  ntc_or_pt1000_10_temp;
    USHORT  ntc_or_pt1000_11_temp;
    USHORT  ntc_or_pt1000_12_temp;
	USHORT	temp_sor_temp;
	USHORT	humi_sor_humi;
	USHORT	temp_humi_sor_temp_485_1;
	USHORT	temp_humi_sor_humi_485_1;
	USHORT	temp_humi_sor_temp_485_2;
	USHORT	temp_humi_sor_humi_485_2;
	USHORT	temp_humi_sor_temp_485_3;
	USHORT	temp_humi_sor_humi_485_3;
	USHORT	temp_humi_sor_temp_485_4;
	USHORT	temp_humi_sor_humi_485_4;
	USHORT	dehumi_temp;
	USHORT	dehumi_humi;
	USHORT	dehumi_start;
	USHORT	dehumi_stop;
	USHORT	dehumi_workstat; // 除湿机工作状态 bit0温度负载 bit1湿度负载 0-未启动 1-启动 bit2resv bit3运行模式0-自动1-手动 bit4故障状态0-正常 1-故障
	USHORT	dehumi_addr;
	USHORT	di1_16;
	USHORT	di17_22;
	USHORT	do1_16;
	USHORT	do17_24;
	USHORT	x1_connect_stat;
	USHORT	combgas_h2;
	USHORT	combgas_co2;
	USHORT	resv1;
	USHORT	resv2;
	USHORT	flood_freq;
	USHORT	pre_sor_volt_1;
	USHORT	pre_sor_volt_2;
	USHORT	pre_sor_volt_3;
	USHORT	pre_sor_volt_4;
	USHORT	pre_sor_volt_5;
	USHORT	pre_sor_volt_6;
	USHORT	pwm_freq_1;
	USHORT	pwm_freq_2;
	USHORT	pwm_freq_3;
	USHORT	pwm_freq_4;
	USHORT	pwm_freq_5;
	USHORT	pwm_freq_6;
	USHORT	alarm_info1;
	USHORT	alarm_info2;
	USHORT	alarm_info3;

} IMEU3_SAMPLE_INFO;


#endif
