#ifndef __ST_BATT_SAMPLER_MAIN_H
#define __ST_BATT_SAMPLER_MAIN_H

#include <semaphore.h>
#include "../sampler_base.h"
#include "imeu2_can.h"
#include "imeu3_can.h"

//#define TRACE	printf("\n[%s:%d] [%ld]--",__FUNCTION__, __LINE__, time(NULL)), printf //1 ? (void)0 : (void)printf //
//#define TRACEX	printf("\n[%s:%d] [%ld]--",__FUNCTION__, __LINE__, time(NULL)), printf //1 ? (void)0 : (void)printf //

//#pragma pack(push)
//#pragma pack(n) n=1,2,4
//#pragma pack(pop)

#define CAN1_READ_TIMEOUT				50
#define RPMSG_READ_TIMEOUT              10   // 无用

#define MAX_RECORD_OCVCALIBRATIONDATA    10000
#define CURRENT_THRESHOLD 			40.0f   // 40A电流阈值
#define CURRENT_THRESHOLD2			15.0f   // 15A电流阈值
#define CHARGE_DISCHARGE_TIME   	2400  	// 小电流充放电时间40min
#define CHARGE_DISCHARGE_TIME2      1200	// 大电流充放电时间20min
#define IDLE_TIME 					2400    // 静置40min
#define IDLE_WAIT_TIMEOUT_TIME  	1800	// 等待空闲状态持续30min
#define OCVCOMPLETE_DURATION_TIME  	3600	//OCV标志完成持续时长 60min
#define MAX_OCV_REC_NUM  			6*60*24	//十秒记录一次，记录24h的记录
#define PCS_HOTSTANDBY_MAX_CURRENT 	2.0f	// PCS热待机最大电流2A
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))


#define ONE_MSG_LEN                     64

#define MAX_PWRONPCS_SEC                60

#define MAX_READ_BUFF_SIZE	            200

#define  CAN_SAMPLE_OK		            0
#define  CAN_WRITE_ERROR	            -5
#define  CAN_READ_ERROR	                -6

#define BATT_COMM_FAIL_TIMES	        40 //2021-09-28 SBK Suggest 4S,BMS Erase Falsh Need over 500ms

#define IMEU_COMM_FAIL_TIMES	        40

#define CAN_CMD_BROADCAST	            0
#define MAX_CAN_SAMPLER_LOCK_WAIT	    100
#define MAX_READ_RETRY_TIMES            3

#define STBATT_PUSH2CCU_PERIOD         10
#define URGET_STBATT_PUSH2CCU_PERIOD	1

#define MAX_CYCLE_SEND_TIMES	        180 //BMS one Times MAX Recv 170 MsgID==1K
#define MAX_BMS_SINGLE_MSGLEN 	        6

#define MAX_BMS_ABNORMAL_TIMES          20

#define STAT_RELAY_ON  	1
#define STAT_RELAY_OFF 	0

#define STBATT_V33_HOLDSTAT_CMD		0x01
#define STBATT_V33_POWERON_CMD 		0x02
#define STBATT_V33_POWEROFF_CMD 	0x03

#define MIN_BMS_BINSIZE 			4*1024
#define MAX_BMS_BINSIZE				10*1024*1024 

#define BMS_RETRY_POWERON_TIMES 	3

#define MAX_RECSAVE_NUM			    100
#define MAX_SUPPORTTED_VAR_FILES    100

#define MAX_IMEU_NUM 4

#define MAX_COOLINGPACK_NUM 5
#define COOLINGPACK_PARAMETER_NUM 10  //Used for liquid cooled PACK push parameter array

#define FLOAT_EQU_ZERO(fValue)  	(fabs((fValue)) <= 1e-6)

#define AirCondi_Interval_Time 3

typedef enum Freq_Code
{
	NORUNNING = 0,
    RUNNING = 1,
    FINISHED = 2, // added by hepingfu 
}Freq_Flag;

typedef unsigned long long ULLONG;//edit by kuangyue 20250306

typedef enum tagERR_Code
{
	ERR_CAN_R_NO_ERR	= 0x00,
    ERR_CAN_R_COMM_FAIL = 0x01,	
    ERR_CAN_R_NO_LOCK             /*by control Cmd Locked, need wait to sample */

}ERR_CODE_CAN_R;

typedef enum tagSTBatt_Ctrl_CmdType
{
    CTRL_STBATT_WAKE_UP,
    CTRL_STBATT_GO_SLEEP,
    CTRL_STBATT_MASK_ERR, //!!!!!!be carefull to use this cmd!!!!!!!!!
    CTRL_STBATT_REBOOT,
    CTRL_STBATT_SUPER_USER,
    CTRL_STBATT_CLEAR_FAULT,
        CTRL_STBATT_WORK_MODE,
    CTRL_STBATT_EU3_AIRNOISE,
    CTRL_STBATT_KEEP_POWER_MODE,
    // LJH240715 aircond
    CTRL_STBATT_RDAIRCOND,
    CTRL_STBATT_SETAIRCOND_BATT_COOLON,
    CTRL_STBATT_SETAIRCOND_BATT_COOLOFF,
    CTRL_STBATT_SETAIRCOND_BATT_HEATON,
    CTRL_STBATT_SETAIRCOND_BATT_HEATOFF,
    CTRL_STBATT_SETAIRCOND_AMBIT_COOLON,
    CTRL_STBATT_SETAIRCOND_AMBIT_COOLOFF,
    CTRL_STBATT_SETAIRCOND_AMBIT_HEATON,
    CTRL_STBATT_SETAIRCOND_AMBIT_HEATOFF,
    CTRL_STBATT_RINT_POINT,
    CTRL_STBATT_AFE_RESET,   /* INFY C2S_CMD_AfeReset: 0x0556xxyy C1 1F 01 [PACK] 00 00 00 00 */
    CTRL_STBATT_BSU_RESET    /* INFY C2S_CMD_BsuReset: 0x0556xxyy C1 20 01 [PACK] 00 00 00 00 */
}STBATT_CTRL_CMDTYPE;

typedef enum tag_STBATT_STATE_MACHINE_STATE
{
    STBATT_STATE_IDLE = 0,
    STBATT_STATE_WAIT_CHARGE,
    STBATT_STATE_IN_CHARGING,
    //STBATT_STATE_WAIT_DISCHARGE,
    STBATT_STATE_IN_DISCHARGING,
    STBATT_STATE_FAULTED,
    STBATT_STATE_OFFGRID,
    STBATT_STATE_MAX_ST,
}STBATT_STATEMACHINE_STATE;



//-------------Start----------------//
typedef enum tagstNDBattCmd_Idx
{ 
	_CAN_R_CMD_B2C_STATU = 0,
	_CAN_R_CMD_B2C_SUMDATA1, 
	_CAN_R_CMD_B2C_SUMDATA2, 
	_CAN_R_CMD_B2C_SUMDATA3, 
	_CAN_R_CMD_B2C_LIMIT, 

	_CAN_R_CMD_S2A_ALARM,
	_CAN_R_CMD_S2A_SUMDATA3,
	_CAN_R_CMD_S2A_SUMDATA4,
    
	_CAN_R_CMD_B2C_INFYALARM,
	_CAN_R_CMD_B2C_INFYFAULT,
	_CAN_R_CMD_B2C_INFYGPIO,
	_CAN_R_CMD_B2C_UPGRADESTATU,
	 

	_CAN_S_CMD_C2B_STATU,   

	_CAN_S_CMD_C2B_HANDSHAKE,

    _CAN_S_CMD_C2B_UNLOCK,

    _CAN_S_CMD_C2B_EXCHANGE_KEY,

    _CAN_S_CMD_C2B_WRITE_REQUEST,

     _CAN_S_CMD_C2B_WRITE_REQUEST_2,

     _CAN_S_CMD_C2B_ROUTINE_CONTROL,

     _CAN_S_CMD_C2B_DOWNLOAD_REQUSET,

     _CAN_S_CMD_C2B_DOWNLOAD_FILE,

     _CAN_S_CMD_C2B_DOWNLOAD_EXIT,

     _CAN_S_CMD_C2B_CHECK_FILE,
 
    _CAN_S_CMD_C2B_CHECK_COMPATIBILITY,

    _CAN_S_CMD_C2B_RESET_ECU,

	_CAN_S_CMD_C2B_SECTORDATA,

	_CAN_S_CMD_C2B_DATAFINISH, //Old Ver Need, New Don't Need

	_CAN_S_CMD_C2B_INSTALL,

	_CAN_S_CMD_C2B_STOPUPGRADE,

	_MAX_ND_CAN_CMD,

}ID_CMD_ND_BATT;


typedef struct tagA_ND_BattFrame
{
	BYTE 			byRawData[CAN_FRAME_LEN];

	/*CAN Protocal Head Info,Total 29 bits,extend frame*/  //0x040E8F(91-9A)
	BYTE			byErrCode;
 	BYTE			byResv1;
	BYTE			byCmdType;

 	int				nSubCmd;
    BYTE    		byBMSGrpAddr;   

	UINT    		nBMSGrpMapIdx;
	
	BYTE 			byPCSAddr;

	ID_CMD_ND_BATT  emFuncCmd;

	UINT 			uiFrmID;
	UINT 			uiDataLen;
	BYTE   	 		byData[CAN_DATA_AREA_LEN + 1];      /* Data Area */

	BOOL    bIsCtrlCmd;     /* if true, need send urgently */

}A_CAN_ND_BATT_FRAME;
//-----------------------End----------------------//


typedef union tagSTBattSysFaultByte
{
	UINT	uiFlag;
	struct
	{
		UINT	IMEU_CommFail : 1;
		UINT	Flood_Alarm : 1;
		UINT	DoorOpen_Alarm : 1;
		UINT	Smoke_Alarm : 1;
        UINT	Fire_Alarm : 1;
		UINT	PowerON_Fail : 1;
		UINT    AirCond_CommFail : 1;
		UINT	AirCond_Alarm : 1;
		UINT	Flammable_Gas : 1;
		UINT	CabSafe_AbnormalState : 1;
        UINT	BattGrp_ForbidRun : 1;
        UINT	BattGrp_InputContactor : 1; 
        UINT	BattGrp_IsolationSwitch : 1;
 	}Fault;

}STBATT_SYS_FAULT_MAP;

typedef union tagSTSysStatusByte
{
    USHORT uiFlag;  

    struct
    {
        USHORT ElectricHeatingStatus : 1; // BIT0: 电辅热状态
        USHORT CirculationPumpStatus : 1; // BIT1: 循环水泵状态
        USHORT CondenserFanStatus    : 1; // BIT2: 冷凝风机状态
        USHORT Comp1Status           : 1; // BIT3: 系统1压缩机状态
        USHORT CrankHeat1Status      : 1; // BIT4: 系统1曲轴加热带状态
        USHORT Sys1RunningStatus     : 1; // BIT5: 系统1运行状态
        USHORT OilReturn1Status      : 1; // BIT6: 系统1回油状态
        USHORT TotalFaultFlag        : 1; // BIT7: 系统总故障标志
    } Status;

}STSYS_FAULT_MAP_TATE1BIT;

typedef union tagSTSysAlarmByte1
{
    USHORT uiFlag;  

    struct
    {
        USHORT HighBatteryOutletPressure : 1; // BIT0: 电池出水压力过高告警
        USHORT HighModuleOutletPressure  : 1; // BIT1: 模块出水压力过高告警
        USHORT PTCOverload               : 1; // BIT2: PTC过载告警
        USHORT HighLowReturnWaterTemp    : 1; // BIT3: 回水温度过高过低
        USHORT HighLowOutletWaterTemp    : 1; // BIT4: 出水温度过高过低
        USHORT Fan1Fault                 : 1; // BIT5: 风机1故障
        USHORT Fan2Fault                 : 1; // BIT6: 风机2故障
        USHORT Fan3Fault                 : 1; // BIT7: 风机3故障
    } Alarm;

} STSYS_FAULT_MAP_TATE2BIT;

typedef union tagSTSysFaultByte1
{
    USHORT uiFlag;  

    struct
    {
        USHORT Fan4Fault                     : 1; // BIT0: 风机4故障
        USHORT Sys1AlarmStatus               : 1; // BIT1: 系统1故障报警状态
        USHORT Sys1LockStatus                : 1; // BIT2: 系统1故障锁机状态
        USHORT SubcoolingTempSensorFault     : 1; // BIT3: 过冷温度传感器故障
        USHORT Sys1AmbientTempSensorFault    : 1; // BIT4: 系统1外环温度传感器故障
        USHORT Sys1ExhaustTempSensorFault    : 1; // BIT5: 系统1排气温度传感器故障
        USHORT Sys1SuctionTempSensorFault    : 1; // BIT6: 系统1吸气温度传感器故障
        USHORT ModuleInletTempSensorFault    : 1; // BIT7: 模块进水温度传感器故障
    } Fault;

} STSYS_FAULT_MAP_TATE3BIT;

typedef union tagSTSysFaultByte2
{
    USHORT uiFlag; 

    struct
    {
        USHORT ModuleOutletTempSensorFault   : 1; // BIT0: 模块出水温度传感器故障
        USHORT Fan2FaultReserved             : 1; // BIT1: 风机2故障(预留)
        USHORT BatteryInletTempSensorFault   : 1; // BIT2: 电池进水温度传感器故障
        USHORT BatteryOutletTempSensorFault  : 1; // BIT3: 电池出水温度传感器故障
        USHORT LowSuperheatAlarm             : 1; // BIT4: 过热度过低告警
        USHORT Sys1HighPressureSensorFault   : 1; // BIT5: 系统1高压压力传感器故障
        USHORT Sys1LowPressureSensorFault    : 1; // BIT6: 系统1低压压力传感器故障
        USHORT Sys1DriverCommFault           : 1; // BIT7: 系统1与驱动器通讯故障
    } Fault;

} STSYS_FAULT_MAP_TATE4BIT;

typedef union tagSTSysAlarmByte2
{
    USHORT uiFlag;  

    struct
    {
        USHORT WaterTankLowLevelAlarm       : 1; // BIT0: 水箱缺液告警
        USHORT ModuleSystemLowLevelAlarm    : 1; // BIT1: 模块系统缺液告警
        USHORT FanAlarmFault                : 1; // BIT2: 风机告警故障
        USHORT Sys1HighPressureProtection   : 1; // BIT3: 系统1高压保护
        USHORT Sys1LowPressureProtection    : 1; // BIT4: 系统1低压保护
        USHORT BatterySystemLowLevelAlarm   : 1; // BIT5: 电池系统缺液告警
        USHORT Sys1WinterAntiFreeze1        : 1; // BIT6: 系统1冬季防冻结1
        USHORT Sys1WinterAntiFreeze2        : 1; // BIT7: 系统1冬季防冻结2
    } Alarm;

} STSYS_FAULT_MAP_TATE5BIT;

typedef union tagSTSysProtectionByte
{
    USHORT uiFlag;  

    struct
    {
        USHORT Sys1WaterTempAbnormalProtection : 1; // BIT0: 系统1水温异常保护
        USHORT Sys1ExhaustOverTempProtection   : 1; // BIT1: 系统1排气过高保护
        USHORT Sys1CoolingAntiFreezeProtection : 1; // BIT2: 系统1制冷防冻保护
        USHORT HY1PumpFaultFeedback            : 1; // BIT3: HY1水泵故障反馈
        USHORT HY3PumpFaultFeedback            : 1; // BIT4: HY3水泵故障反馈
        USHORT DCFan1Fault                     : 1; // BIT5: 直流风机1故障
        USHORT Reserved6                       : 1; // BIT6: 预留位
        USHORT Reserved7                       : 1; // BIT7: 预留位
    } Protection;

} STSYS_FAULT_MAP_TATE6BIT;

typedef struct tagAirConditionStatus
{
    //---liquid AIR status01---//
	USHORT						nPowerStatus;		//开关机状态
	USHORT						nWorkMode;			//工作模式
	float						fColdTemp;    		//制冷温度

	USHORT						nColdOpenDiff; 		//制冷启动温差
	USHORT						nColdStopDiff;		//制冷停止温差
	float						fHeatTemp;			//制热温度

	float						fHeatDiff;			//制热温差
	USHORT						nRefrigerantType;	//冷媒类型
	USHORT						nUpMachineBaud;		//上位机波特率

	USHORT						nContactAddr;		//通信地址

	//---liquid AIR status02---//
	USHORT						nOutsideMachinAbility;	//室外机规格能力
	//USHORT						nSoftVer;				//软件版本

	USHORT						nPLS;    				//系统1主电子膨胀阀开度

	float 						fSys1TargetCompFreq;    // 系统1变频压缩机目标频率
	float 						fSys1CurrentCompFreq;   // 系统1变频压缩机运行频率
	USHORT 						nFanTargetSpeed;       		// 风机目标转速

	float 						fExhaustSuperheat;        // 排气过热度
	float 						fSys1AmbientTemp;         // 系统1外环温度
	float 						fSys1ExhaustTemp;         // 系统1排气温度

	float 						fSys1SuctionTemp;         // 系统1吸气温度
	float 						fModuleInletTemp;         // 模块进水温度
	float 						fModuleOutletTemp;        // 模块出水温度

	float 						fBatteryInletTemp;        // 电池进水温度
	float 						fBatteryOutletTemp;       // 电池出水温度
	USHORT						 nPumpActualSpeed;        // 水泵实际转速

	float 						fModuleOutletPressure;    // 模块出水压力 0.01
	float 						fBatteryOutletPressure;   // 电池出水压力 0.01
	float 						fSys1HighSatTemp;         // 系统1高压饱和温度

	float 						fSys1LowSatTemp;          // 系统1低压饱和温度
	float 						fSuctionSuperheat;        // 吸气过热度
	float 						fSys1LowPressure;         // 系统1低压压力 0.01

	float 						fSys1HighPressure;        // 系统1高压压力 0.01
	float 						fSys1ACInputVoltage;      // 系统1AC输入电压
	float 						fSys1ACInputCurrent;      // 系统1AC输入电流

	USHORT 						nSys1CompressorCurrent;  // 系统1压缩机相电流值
	float 						fSys1DCBusVoltage;        // 系统1直流母线电压

    BYTE						byBMSAirSoftVersion[16];   //软件版本

    //---liquid AIR status03---//
	STSYS_FAULT_MAP_TATE1BIT		AirSysFault1;
	STSYS_FAULT_MAP_TATE2BIT		AirSysFault2;
	STSYS_FAULT_MAP_TATE3BIT		AirSysFault3;
	STSYS_FAULT_MAP_TATE4BIT		AirSysFault4;
	STSYS_FAULT_MAP_TATE5BIT		AirSysFault5;
	STSYS_FAULT_MAP_TATE6BIT		AirSysFault6;

}AIR_STATUS_ALARM_FAULT;

typedef struct tagTongFeiAirConditionStatus
{
    //S2C_TFAIRPARAINFO   0x10d4ffxx
    USHORT nStartupMode;    //启动模式
    USHORT nWorkingMode;    //工作模式
    USHORT nSuplyWater;     //供回水控制目标

    float  fRefrigerationPresetTemperature; //制冷预设温度
    float  fCoolingTemperatureTifference;   //制冷温差
    float  fHeatingTresetTemperature;   //制热预设温度

    float  fHeatingTemperatureTifference;   //制热温差

    //S2C_TFAIRSTAINFO  0x10d5ffxx
    USHORT  nUnitOperatingStatus;   //机组运行状态
    USHORT  nUnitOperatingMode;     //机组运行模式
    USHORT  nSwitchingInput;        //开关量输入
    USHORT  nRelayInput;            //继电器输出

    float   fLiquidSupplyTemperature;   //供液温度
    float   fReturnLiquidTemperature;   //回液温度
    float   fSystemSteamingIn;          //系统蒸进

    float   fSteamOutTemperature;       //蒸出温度
    float   fAmbientTemperature;        //环境温度
    float   fSystemCondensationTemperature; //系统冷凝温度

    USHORT  nCurrentOpeningDegree;      //节流电膨当前开度
    USHORT  nCurrentSpeedWaterPump;     //水泵当前转速
    USHORT  nCurrentSpeedCompressor;    //压缩机当前转速

    USHORT  nFanSpeed;                //风机转速
    float  nWaterSupplyPressure;       //供水压力
    float  nReturnWaterPressure;       //回水压力

    float  nExhaustPressure;           //排气压力

    //S2C_TFAIRERRINFO  0x10d6ffxx
    USHORT nFaultStatus1;     //故障状态1
    USHORT nFaultStatus2;     //故障状态2
    USHORT nFaultStatus3;     //故障状态3

    USHORT nFaultStatus4;     //故障状态4
    USHORT nFaultStatus5;     //故障状态5
    USHORT nFaultStatus6;    //备用

    USHORT nFaultStatus7;    //备用
    USHORT nVariableFrequencyFault1;  //变频风机故障1
    USHORT nVariableFrequencyFault2;   //变频风机故障1

    UINT nThreePhasePFCFault;        //三相PF值故障  预留

}TONGFEI_AIR_STATUS_ALARM_FAULT;

typedef struct tagIMEU3AirConditionControlStatus
{
    //S2C_EU3AIRCONTROLINFO   0x10caffxx
    //edit by kuangyue 20250811
    BYTE byCellPwrState;                  // 电池测开关机
    BYTE byPcsPwrState;                   // 模块侧开关机
    USHORT usBatteryMode;                 // 电池侧工作模式

    BYTE byControlObj;                   // 进出水控制目标：0：控制出水温度，1：控制进水温度
    float fCoolingTemp;                  // 设置制冷温度
    float fHeatingTemp;                  // 设置制热温度

    BYTE  byNoiseReductionMode;          // 降噪模式：0：自动降噪模式，1：静音模式1，2：静音模式2，3：降噪模式3，4：降噪模式4，5：降噪模式5，6：全开放模式
    float fCoolingTempDiff;              // 设置制冷启动温差
    float fCoolingStopTempDiff;          // 设置制冷停止温差

    float fHeatingTempDiff;              // 设置制热启动温差
    float fHeatingStopTempDiff;          // 设置制热停止温差

}IMEU3_AIR_STATUS_CONTROL_INFO;
typedef struct tagIMEU3AirConditionStatus
{
    //S2C_EU3AIRSTATEINFO   0x10cbffxx
    //edit by kuangyue 20250217
    UINT bSoftVer;              // 软件版本
    USHORT uPowerStat;              // 开机状态
    USHORT uOpMode;                 // 运行模式 1:制冷 2:制热 3:自循环 5:自然冷

    USHORT uOpStep;                 // 运行阶段
    USHORT uErrCode;                // 故障码
    float fTgtTemp;                 // 目标温度

    float fRetTemp;                 // 反馈水温
    float fOverheatTemp;            // 吸气过热温度
    float fSatTemp;                 // 饱和温度

    USHORT uExpValveOpen;           // 电子膨胀阀当前开度
    USHORT uTotalCap;               // 能力总输出
    USHORT uCompTgtHz;              // 压缩机目标频率

    USHORT uCompOpHz;               // 压缩机运行频率
    USHORT uCompOpPhase;            // 压缩机运行阶段
    USHORT nFaultLv;                 // 故障等级

    float fexhaustTempFault;              // 排气温度
    float fEvapInTemp;               // 吸气温度
    float fCoolInTemp;               // 空调冷却水进水温度

    float fModuleOutTemp;              // 模块出水温度
    float fModuleInTemp;             // 模块进水温度
    float fBattInTemp;               // 电池进水温度

    float fBattOutTemp;              // 电池出水温度
    float fCoolModOutTemp;           // 空调冷却水模块出水温度
    float fCoolBattOutTemp;          // 空调冷却水电池出水温度

    float fAmbTemp;                  // 环境温度
    float fHighPress;                // 高压压力
    float fLowPress;                 // 低压压力

    float fBattOutPress;             // 电池出水压力
    float fModuleOutPress;           // 模块出水压力
    float fCoolInPress;             //空调冷却水进水压力

    float fCoolOutPress;            //空调冷却水出水压力
    USHORT nFaultLvl;                  //传感器故障1
    USHORT nVfFault2;                  //传感器故障2

    USHORT nPfcFault;                  //系统部件故障
    USHORT nCompFault1;                // 压缩机故障1
    USHORT nCompFault2;                // 压缩机故障2

    USHORT nSysStatFault;              // 系统状态故障
    float fBattPumpHz;               // 电池水泵频率
    float fBattPumpDuty;              // 电池水泵占空比

    float fModulePumpHz;              // 模块水泵频率
    float fModulePumpDuty;            // 模块水泵占空比
    float fDrvFanHz;                  // 驱动器散热风机输出频率

    float fDrvFanDuty;                // 驱动器散热风机输出占空比
    float fFanHz;                     // 风机频率
    float fFanDuty;                   // 风机占空比



}IMEU3_AIR_STATUS_ALARM_FAULT;

// edit by kuangyue 20250306
typedef union tagXinMiaoAirStatusByte
{
    ULLONG uiFlag; // 64 位标志位

    struct 
    {
        ULLONG noFault         : 1; 
        ULLONG paramErr        : 1;
        ULLONG phaseSeqFault   : 1;
        ULLONG phaseLoss       : 1;
        ULLONG overSpeed       : 1;
        ULLONG retTempFault    : 1;
        ULLONG outTempFault    : 1;
        ULLONG envTempFaultFault    : 1;
        ULLONG exhaustTempFaultFault: 1;
        ULLONG suctionTempFaultFault: 1;
        ULLONG comm485FaultFault    : 1;
        ULLONG boardCommFaultFault  : 1;
        ULLONG outPressFault   : 1;
        ULLONG inPressFault    : 1;
        ULLONG compOverCurr    : 1;
        ULLONG pressDiffLow    : 1;
        ULLONG coolTempLow     : 1;
        ULLONG outTempHigh     : 1;
        ULLONG exhaustTempHigh : 1;
        ULLONG highPressFault  : 1;
        ULLONG lowPressFault   : 1;
        ULLONG highPressProt   : 1;
        ULLONG lowPressProt    : 1;
        ULLONG waterLow        : 1;
        ULLONG levelSwitch     : 1;
        ULLONG fan1Fault       : 1;
        ULLONG outPressHigh    : 1;
        ULLONG drvModuleFault  : 1;
        ULLONG busVoltAbn      : 1;
        ULLONG drvTempHigh     : 1;
        ULLONG compStartFault  : 1;
        ULLONG compStepLoss    : 1;
        ULLONG freqOverCurr    : 1;
        ULLONG rectSoftOver    : 1;
        ULLONG rectHardOver    : 1;
        ULLONG fan2Fault       : 1;
        ULLONG pumpFault       : 1;
        ULLONG freqInstOver    : 1;
        ULLONG Reserved        : 26;
    } Faults;

} XINMIAO_AIR_FAULT_STATUS_BITS;

typedef struct tagXINMIAOAirConditionStatus
{
    //S2C_XINMIAOAIRSTATEINFO   0x10d7ffxx
    //edit by kuangyue 20250217
    UINT nPumpStat;             // 水泵运行状态
    UINT nComp1Stat;            // 1#压缩机运行状态
    UINT nComp2Stat;            // 2#压缩机运行状态

    UINT nHeatStat;             // 电加热运行状态
    UINT nCoolFanStat;          // 冷凝风机运行状态
    float fOutTemp;             // 机组出水温度

    float fRetTemp;             // 机组回水温度
    float fOutPress;            // 机组出水压力
    float fRetPress;            // 机组回水压力

    float fenvTempFault;             // 机组外环境温度
    UINT nMakeUpPump;           // 补液水泵状态
    USHORT nFaultCode;            // 故障告警代码

    UINT nWorkMode;             // 液冷机组控制模式
    UINT nPowerStat;            // 开机状态
    float fTgtTemp;             // 机组设定温度

    XINMIAO_AIR_FAULT_STATUS_BITS faultBits;    // 故障状态位域//edit by kuangyue 20250306

}XINMIAO_AIR_STATUS_ALARM_FAULT;

typedef struct tagLiquidCoolingPACK
{
    //S2C_PACKINFO1   0x10d1ffxx
    float  fPositiveTemp[MAX_COOLINGPACK_NUM];    //Positive electrode copper bar temperature
    float  fNegativeTemp[MAX_COOLINGPACK_NUM];     //Negative copper bar temperature
    UINT   uImmersionFreq[MAX_COOLINGPACK_NUM];   //Immersion frequency

    //S2C_PACKINFO2   0x10d2ffxx
    float  fHighVoltageValue[MAX_COOLINGPACK_NUM];      //High voltage interlock amplitude
    UINT   uHighVoltageDuty[MAX_COOLINGPACK_NUM];       //High voltage interlock duty cycle

    //S2C_PACKINFO3   0x10d3ffxx
    UINT   uSol[MAX_COOLINGPACK_NUM];               //Sol concentration
    UINT   uH2[MAX_COOLINGPACK_NUM];               //H2 concentration
    UINT   uAerosolStatusAndFault[MAX_COOLINGPACK_NUM]; //Aerosol status and fault information

    UINT   uReserve1[MAX_COOLINGPACK_NUM];    //液冷PACK预留参数1
    UINT   uReserve2[MAX_COOLINGPACK_NUM];    //液冷PACK预留参数2

    UINT usBsu1BotNum;  // PACK1 AFE BOT端枚举数量
    UINT usBsu1TopNum;  // PACK1 AFE TOP端枚举数量
    UINT usBsu2BotNum;  // PACK2 AFE BOT端枚举数量

    UINT usBsu2TopNum;  // PACK2 AFE TOP端枚举数量
    UINT usBsu3BotNum;  // PACK3 AFE BOT端枚举数量
    UINT usBsu3TopNum;  // PACK3 AFE TOP端枚举数量

    UINT usBsu4BotNum;  // PACK4 AFE BOT端枚举数量
    UINT usBsu4TopNum;  // PACK4 AFE TOP端枚举数量
    UINT usBsu5BotNum;  // PACK5 AFE BOT端枚举数量

    UINT usBsu5TopNum;  // PACK5 AFE TOP端枚举数量
}LIQUID_COOLING_PACK_STATUS;


typedef STBATT_STATEMACHINE_STATE	(*ON_STSTATE_PROC)(void* pArg, int nBattIdx);
typedef struct tagSTStateMachineItem
{
	//char				        szStateName[16];
	STBATT_STATEMACHINE_STATE	emState;
	ON_STSTATE_PROC		        pfnAction;
	double				        tm_Enter[MAX_SINGLE_STBATT_NUM];
}STSTATE_MACHINE_ITEM;

typedef struct tagSingleBattData
{

    STSTATE_MACHINE_ITEM*                   pCurrState;
    STSTATE_MACHINE_ITEM*                   pLastState;

}SINGLE_BATT_DATA;


//typedef enum tag_StatBattUpdate
//{
//
//}STAT_BATT_SW_UPDATE;


#define UPGRADE_UDS_RECEIVE_TIMEOUT     6

#define UPGRADE_FRAME_ID_UDS            0x1bf00000
#define UPGRADE_SEED_LEN                4
#define UPGRADE_WRITE_REQUEST_LEN       16
#define UPGRADE_ROUNTINE_CONTROL_LEN    9
#define UPGRADE_DOWNLOAD_REQUEST_LEN    8
#define UPGRADE_CRC32_DATA_LEN          4

#define UDS_SERVICE_HANDSHAKE_10          0x10
#define UDS_SERVICE_UNLOCK_27              0x27
#define UDS_SERVICE_EXCHANGE_KEY_27        0x27
#define UDS_SERVICE_WRITE_REQUEST_2E       0x2e
#define UDS_SERVICE_ROUNTINE_CONTROL_31    0x31
#define UDS_SERVICE_DOWNLOAD_REQUEST_34    0x34
#define UDS_SERVICE_DOWNLOAD_36            0x36
#define UDS_SERVICE_DOWNLOAD_EXIT_37       0x37
#define UDS_SERVICE_RESET_ECU_11           0x11

#define UDS_FRAME_TYPE_SINGLE               0
#define UDS_FRAME_TYPE_FLOW_CONTROL         3
#define UDS_REPLY_POSITIVE                  0x40
#define UDS_REPLY_NEGATIVE                  0x7f
#define UDS_REPLY_PENDING                   0x78

#define BMU_MIN_ADDRESS                 0x32
#define BMU_MAX_ADDRESS                 0x35
#define BSU_MIN_ADDRESS                 0x00
#define BSU_MAX_ADDRESS                 0x0B
#define BMU_AUX_MIN_ADDRESS             0xC8
#define BMU_AUX_MAX_ADDRESS             0xCB
#define IMEU2_MIN_ADDRESS               0x28
#define IMEU2_MAX_ADDRESS               0x2B
#define IMEU3_MIN_ADDRESS               0x1E
#define IMEU3_MAX_ADDRESS               0x21
#define IMEU3_STSCAbinet_ADDRESS        0x2D
#define Air_IMEU3_MIN_ADDRESS           0x2B
#define Air_IMEU3_MAX_ADDRESS           0x2C


#define BMU_PROGRAM_ADDRESS             0x014000
#define BSU_PROGRAM_ADDRESS             0X0800A000
//#define BMU_AUX_PROGRAM_ADDRESS         0X0800A000
#define BMU_AUX_PROGRAM_ADDRESS         0X0000A000
#define IMEU2_PROGRAM_ADDRESS           0x0009800
#define IMEU3_PROGRAM_ADDRESS           0x0800C800
#define IMEU3_AIR_ADDRESS               0x08008000
typedef enum _UDS_RESPOND
{
    SERVICE_NEGATIVE_FAILED = 0,
    SERVICE_POSITIVE_OK = 1,
    SERVICE_NEGATIVE_BUSY = 2,
    SERVICE_NOT_RESPOND = 3
}UDS_RESPOND;

typedef enum _UPGRADE_DEV_TYPE
{
    DEV_NONE = 0,
    DEV_BSU,
    DEV_BMU,
    DEV_BMU_AUX = 4,
    DEV_IMEU2 = 6,
    DEV_IMEU3 = 6

}UPGRADE_DEV_TYPE;

typedef struct tagBattUpgradeParam
{
    time_t                      tm_StartUpgrade;
    
    SW_UPGRADE_STATUS		    emUpgradeStatus;

    UPDOWN_FILE_INFO 		    stUpdownFileInfo;

    char				        szFilePath[256];

    UINT						nLast_StartMsgId;

    UINT						nLocalSendMsgId;

    UINT                        nTotalMsgNum;

    UINT                        nSingleMsgLen;

    //---BMS Upload Upgrade Status----//
    int							nRemtDownIndex[MAX_SINGLE_STBATT_NUM];
	BMS_REPORT_STATE			emRemtReptStat[MAX_SINGLE_STBATT_NUM];

 	int							nRemtRunModel[MAX_SINGLE_STBATT_NUM];
	int							nRemtRunState[MAX_SINGLE_STBATT_NUM];
	int							nRemtVersion[MAX_SINGLE_STBATT_NUM];
    BYTE   	 					byData[MAX_SINGLE_STBATT_NUM][CAN_DATA_AREA_LEN];
    BYTE                        bySeed[UPGRADE_SEED_LEN];
    UINT                        uStartAddr;
    UINT                        uMaxDownloadBlockLen;
    BYTE                        byFrameStatus;
    BYTE                        byDataWaiteRespone;//wait for response after send byDataWaitResponse bytes of data
    BYTE                        byFrameInterval;//Time interval between reply of long frames
    BYTE                        byDevAddr;
    UPGRADE_DEV_TYPE            eDevType;
    BOOL                        bUpgradeResult;

}BATT_UPGRADE_PARAM;

 
typedef enum emBMSCanMsgSaveStatu
{
    STAT_ON_START = 0,
    STAT_ON_PENDING,
    STAT_ON_END

}STATE_BMS_SAVE;

typedef enum emTriggerMsgType
{
    TP_TRIG_SUSPEND = 0,
    TP_TRIG_MANUAL,
    TP_TRIG_FAULT,
    TP_TRIG_LOG, 
    TP_TRIG_WAVE,  
    TP_TRIG_DOD,  


}TP_TRIGGER_MSG;

typedef struct tagBMSCANMsgSave
{
	time_t					tm_LastStatEnter;
	
	STATE_BMS_SAVE			emSaveState;


 }STU_BMS_CANMSG_STATMACH;


typedef struct tagBMSMsgFileInfo
{
	time_t			tmFileGenerated;
	UINT			nBMSIdx;
	ULONG			nTotalMsgNum;  
	char			szFileName[64];

}BMSMSG_FILEINFO;

//#define MAX_WRITEMSG_NUM            100
#define MIN_DATA_FRAME              8
#define SIZE_WRITE_BUFF             1024

typedef struct tagCanDataSave
{
    HANDLE						hThread;
    BOOL						bNeedExitThread;

    HANDLE						hMutex;
    sem_t						hSemWrite[MAX_SINGLE_STBATT_NUM];
 
    BOOL						bIsStartedWrite[MAX_SINGLE_STBATT_NUM];
    BOOL						bNeedWriteMsg2File[MAX_SINGLE_STBATT_NUM];
    BOOL						bNeedPost[MAX_SINGLE_STBATT_NUM];

    FILE						*hFile[MAX_SINGLE_STBATT_NUM];

	char						szWriteBuff[MAX_SINGLE_STBATT_NUM][SIZE_WRITE_BUFF];

    UINT                        nTotalMsgNum[MAX_SINGLE_STBATT_NUM];

    //time_t                      tmFileGenerated[MAX_SINGLE_STBATT_NUM];

    UINT						nUsedTotalFiles;
	BMSMSG_FILEINFO				usedFileInfo[MAX_SUPPORTTED_VAR_FILES];
	BOOL						bIsUSBDisk_Mounted;

	BMSMSG_FILEINFO*			pFileInfo;


    //BYTE						szFrame[MIN_DATA_FRAME*MAX_WRITEMSG_NUM+1];

    STU_BMS_CANMSG_STATMACH		stuStatMach[MAX_SINGLE_STBATT_NUM];

    TP_TRIGGER_MSG		        emTrigMsgTp[MAX_SINGLE_STBATT_NUM];


}STU_CANDATA_SAVE;

//-----------------------------BMS Test Mode Rough Data----------------//

#define     MIN_PM_OUTPUT_VOLT     250
typedef enum tagFailFrom
{
	FAIL_FROM_NONE = 0,
	FAIL_FROM_MANULSTOP,
	FAIL_FROM_ABNORMAL

}FAIL_FROM;

typedef enum emTestModeFailCode
{
    FAIL_NO_FAIL = 0,
    FAIL_RELAY_VOLT_ABNORMAL,
    FAIL_SELFCHECK,
    FAIL_FULLDISCHG_STEP1,
    FAIL_FULLCHG_STEP3,
    FAIL_FULLDISCHG_STEP5,
    FAIL_HALFCHG_STEP7,

    FAIL_SYSSTATU_FAIL,
    FAIL_BAT_FAULT,
    FAIL_BAT_COMMFAIL,




}TESTMODE_FAIL_CODE;


typedef STATE_MACHINE_STAT_TEST     (*ON_STATE_PROC)(void* pArg, int nBattIdx);
typedef void (*ON_FAIL_ACTION)(void *pArg, FAIL_FROM emFailFrom, TESTMODE_FAIL_CODE emFailCode, int nBattIdx);

typedef struct tagStateMachineItem
{    
    STATE_MACHINE_STAT_TEST     emState;
    ON_STATE_PROC           pfnAction;
 
    time_t                  tm_Enter;
    FAIL_FROM               emFailFrom;
    TESTMODE_FAIL_CODE      emFailCode;
    time_t                  tm_PredictRemTM;

}STATE_MACHINE_ITEM_TEST;

typedef struct stRoughBMSTestMode
{ 

    STATE_MACHINE_ITEM_TEST		stateMachineLoop[STATE_MAX_ST];

    STATE_MACHINE_ITEM_TEST		*pLastState[MAX_SINGLE_STBATT_NUM];
    STATE_MACHINE_ITEM_TEST		*pCurrState[MAX_SINGLE_STBATT_NUM];

    ON_FAIL_ACTION				pfnOnFail[MAX_SINGLE_STBATT_NUM];
	TESTMODE_FAIL_CODE			emFailCode[MAX_SINGLE_STBATT_NUM];
	int				    		nFailClass[MAX_SINGLE_STBATT_NUM];
    int                 		nFailTimes[MAX_SINGLE_STBATT_NUM];

    STATE_MACHINE_STAT_TEST     emTrigState[MAX_SINGLE_STBATT_NUM];

    STATE_MACHINE_STAT_TEST     emRealTimeState[MAX_SINGLE_STBATT_NUM];

    #define TEST_STOP_NONE  	0
    #define TEST_STOP_NORMAL	1
    #define TEST_STOP_URGET 	2
    #define TEST_STOP_BMS   	3

    int							nStopCond[MAX_SINGLE_STBATT_NUM];

}BMS_TESTMODE_ROUGH;


typedef struct stRpmsginfo
{
    int datalen;
    int dataType;
}DATA_INFO;

//-----------------------------------End--------------------------------//
 
// 用于记录最新200条数据的环形缓冲区
typedef struct {
    BYTE buffer[200][CAN_FRAME_LEN];
    UINT head;  // 当前位置
    UINT tail;  // 末尾位置
    UINT count;
} CanRecCirBuffer;

typedef struct {
    BYTE buffer[200][CAN_FRAME_LEN];
    UINT count;
    BOOL isActive;
} CanRecBuffer;

typedef enum tagSOCCalibrationMethod
{
    SOCChargingCalibration = 0,
    SOCDischargingCalibration,

}SOCCalibrationMethod;

typedef struct tagCalibratedSOCData
{
    UINT                                id;
    time_t                              tmUTCTime;
	STBATT_RUN_STATU                    emBattStatus_Run;
    UINT                                nBattStandbyTime;
    float                               fPackVolt;
    float                               fPackMaxVolt;
    float                               fPackMinVolt;
    float                               fPackCurr;
    float                               fPackTemp;
    int                                 nSOC;
    int                                 nOCVResult;
    int                                 nCalibratedSOC;

}CalibratedSOCData;
typedef struct stBattRun_Rough
{
	HANDLE		                        hSampler; //pointer to sampler
	int			                        nSamplerId; //copy from sampler
	HANDLE		                        hSamplerLock;
    DATA_PROCESSOR*		                pDPR;

    CCU_STATE                           ccuStatu;

    STAT_PM_CTRL_MODE                   emPMCtrlMode;
    STAT_PM_CTRL_MODE                   emLast_PMCtrlMode;

    STSTATE_MACHINE_ITEM                stateMachs[STBATT_STATE_MAX_ST];
    
    SINGLE_BATT_DATA                    singleBattData[MAX_SINGLE_STBATT_NUM];

	HANDLE				                hCanDevHanle;   /* when can opened, get the handle */
	HANDLE                              hCAN1Write_Mutex;
    COMMPORT*			                pComm;

    HANDLE				                RpmsgHanle_cycle;   
    COMMPORT			                Rpmsg_cycle_info;

	HANDLE				                RpmsgHanle_faq;   
    COMMPORT			                Rpmsg_faq_info;

    BYTE				                abyRcvBuf[MAX_READ_BUFF_SIZE * CAN_FRAME_LEN];

    BYTE                                emSysBattInstallType; //this will be gotton from bootUp setting
    UINT                                nSystemCfgBattNum;

    BOOL                                bIsSysRunAsMaster; //if run as slave, do not sample
    
    BOOL                                bCtrlOutRelayClose[MAX_SINGLE_STBATT_NUM];
    time_t                              ulLastCtrlTime[MAX_SINGLE_STBATT_NUM];
	int                                 nCtrlTimes[MAX_SINGLE_STBATT_NUM];
    int                                 nPowerONTimes[MAX_SINGLE_STBATT_NUM];

	//A_BLOB_BATT_MGMT_PLAN               stESMgmtPlan; //ST battery management plan settings
    A_BLOB_EMS_CONFIG_PLAN 			    blobEmsCfgPlan;

    //A_BLOB_ES_iOCPP_CONFIGURAION		blobESiOCPPCfgs;

    double                              tm_fLastCalcBattEnergyCost;

    //BattGrp sett/status
    BATT_MANUFACTURER                   emBattMFG;

    UINT                                 nProtectSOC;
    UINT                                 emDisChgByStopSOC;
    UINT                                 emFullyChgDischgMode;
    UINT                                 nBackupProtectSOC;

    UINT                                 nPVSysBattRetSOC; 
    UINT                                 nStopSOC[MAX_SINGLE_STBATT_NUM];

    HIS_BMS_RUNTIMEDATA_REC             hisBmsRunDataRec;

	HIS_HEALTH_STATIC_DAILY			    hisHealthRec; //note this pointer should never be deleted!!!


    A_BLOB_SINGLE_STBATT_RUNINFO        blobSingleBattInfo[MAX_SINGLE_STBATT_NUM];
    double                              fSingle_Last_ChrgedEnergyReading[MAX_SINGLE_STBATT_NUM];
    double                              fSingle_Last_DisChrgedEnergyReading[MAX_SINGLE_STBATT_NUM];

    A_BLOB_GROUP_STBATT_RUNINFO         blobGroupBattInfo;
    double                              fGroup_Last_ChrgedEnergyReading;
    double                              fGroup_Last_DisChrgedEnergyReading;
    
    A_BLOB_ES_RUN_DATA                  esRunData;
    
    A_BLOB_IFY_BMS_DATAINFO             blobIfyBMS[MAX_SINGLE_STBATT_NUM];

    A_BLOB_BRANCH_GRP_RECTDATA          branchGrpData[MAX_SINGLE_STBATT_NUM];

    A_BLOB_BRANCH_NODE_CALC_DATA        calcNodeData[MAX_SINGLE_STBATT_NUM];

    A_BLOB_STATUS_STAT              StatusStat;
    A_BLOB_ENERGY_STAT              EnergyStatus;
    A_BLOB_DIAG_STAT                DiagStat;
    STAT_FEEDBACK                   StatFeedback;
    BMS_ALARM_FAULT                 stBmsAlarmFault[MAX_BATT_GROUP_NUM];
    IMIU4_INFO                      stImiu4Info[MAX_BATT_GROUP_NUM];
    BSU02_SAMPLE_INFO               stBsu02Info;
    HEARTBEAT_INFO                  stHeartbeatInfo;
    HIS_FAULT_RECORD                stHisFaultRecord[MAX_BATT_GROUP_NUM];
    HIS_EVENT_RECORD                stHisEventRecord[MAX_BATT_GROUP_NUM];
    HIS_DOD_RECORD                 stHisDODRecord[MAX_BATT_GROUP_NUM];
    HIS_SOS_RECORD                 stHisSOSRecord[MAX_BATT_GROUP_NUM];
    BOARD_SAMPLE_INFO               stBoardSampleInfo;
    IMEU3_SAMPLE_INFO               stimeu3SampleInfo;

    A_BLOB_MANUALCTRL_RECT_CMD	        blobManualCtrlPM;

	STS_AUTOMANUAL_ONOFFGRID		    emSTSAutoManual;

    float                               fLastSendReqVolt_2CCU[MAX_SINGLE_STBATT_NUM];
    float                               fLastSendReqCurr_2CCU[MAX_SINGLE_STBATT_NUM];
    GRP_RECT_CTRL_CMDTYPE               emLastCtrlPcsCmd[MAX_SINGLE_STBATT_NUM];

    double                              tm_fLastSendCmd_2CCU[MAX_SINGLE_STBATT_NUM];
    time_t                              tm_LastStopChg_4STBatt[MAX_SINGLE_STBATT_NUM];
    time_t                              tm_LastStartChg_4STBatt[MAX_SINGLE_STBATT_NUM];
    time_t 				                t_FirstBattBoot;

    double                              t_LastTrigIsoStartCmd[MAX_SINGLE_STBATT_NUM];


    double                              fCalc_ChargedEnergy[MAX_SINGLE_STBATT_NUM];
    double                              fCalc_DisChargedEnergy[MAX_SINGLE_STBATT_NUM];

	double                              fLastCalc_ChargedEnergy[MAX_SINGLE_STBATT_NUM];
    double                              fLastCalc_DisChargedEnergy[MAX_SINGLE_STBATT_NUM];
	
    int                                 nCCUAbnormalTimes;
    UINT                                nInsolutionAlmTimes;
    
    UINT                                nPCSFaultTimes;
    UINT                                nLeakageCurrAlmTimes;

	BATT_UPGRADE_PARAM                  stUpgradeParam;

    //User Grid ACMeter
    float				                fGridMeter_Volt[3];
	float				                fGridMeter_Curr[3];
    float                               fGridMeter_Power[3];
    float 				                f_QgridMeter;
	float 				                f_QcabMeter;                                
	//Cabinet ACMeter               
	float				                fCabMeter_Volt[3]; 
	float				                fCabMeter_Curr[3]; 
	float				                fCabMeter_Power[3]; 

    float                                fPowerCubeTemp[3];

    IMEU_TYPE			                emIMEUType[MAX_IMEU_NUM];
    IMEU_TYPE			                emPOWER_IMEUType[MAX_IMEU_NUM];
    A_BLOB_BATTCAB_IMEU2_V20            blobIMEU2Data[MAX_SINGLE_STBATT_NUM];
    IMEU2_CAN_ROUGH_DATA                IMEU2CANRoughData[MAX_IMEU_NUM];

    IMEU3_CAN_ROUGH_DATA                IMEU3CANRoughData[MAX_IMEU_NUM];
    IMEU3_CAN_ROUGH_DATA                PowerEU3_OtherEU3_Data[MAX_IMEU_NUM];

    BATT_CELL_TEMP_PRECISION            eBattCellTempPrecison;

    UINT                                nStuffTimes[MAX_SINGLE_STBATT_NUM];

    //----- CAN Frame Save Function -----//
    STU_CANDATA_SAVE                    stuCanSave;

    //-----------Test Mode DataRough -------//
    BMS_TESTMODE_ROUGH                  stuTestMode;

    //------------YFY ISO Detection Trigger -----------------//
    TRIGGER_ISO_STATUS 			        emISOTriggerStatu[MAX_SINGLE_STBATT_NUM];
    float                               fISO_PeriodMin_Sett;

    //---------- Upgrade IMEU-D Bin --------------//
	//CCU_UPGRADE_PARAM						tagCCUUpgradeParam;
	A_BLOB_UPDT_INFO_4CCU					blobUpdCmd;

    //Add 20230704 YFY BMS CMD Send
    UINT					            emEnterSuperuser[MAX_SINGLE_STBATT_NUM];
  	UINT					            emClearFaultType[MAX_SINGLE_STBATT_NUM];
    UINT					            emWorkMode[MAX_SINGLE_STBATT_NUM];

    UINT					            emAirNoise[MAX_SINGLE_STBATT_NUM];

    BOOL                                bEnterReboot[MAX_SINGLE_STBATT_NUM];
    UINT					            Afe_Reset_PackNum[MAX_SINGLE_STBATT_NUM];
    UINT					            Bus_Reset_PackNum[MAX_SINGLE_STBATT_NUM];

    //add by lpl 20240807 for air condition Status & alarm & fault 
    AIR_STATUS_ALARM_FAULT               stAirStatusAlarm[MAX_SINGLE_STBATT_NUM];
    TONGFEI_AIR_STATUS_ALARM_FAULT       stTongFeiAirStatusAlarm[MAX_SINGLE_STBATT_NUM];
    AIR_TYPE                             emAirType[MAX_SINGLE_STBATT_NUM];
    //edit by kuangyue 20250217
    IMEU3_AIR_STATUS_ALARM_FAULT         stIMEU3AirStatusFault[MAX_SINGLE_STBATT_NUM];
    IMEU3_AIR_STATUS_CONTROL_INFO        stIMEU3AirControlInfo[MAX_SINGLE_STBATT_NUM];//edit by kuangyue 20250811
    XINMIAO_AIR_STATUS_ALARM_FAULT         stXinMiaoAirStatusFault[MAX_SINGLE_STBATT_NUM];

    LIQUID_COOLING_PACK_STATUS           stLiquidCoolingPack[MAX_SINGLE_STBATT_NUM];

    //Calculate charge or discharge energy by Jeffery 20240715
    double                              fSingleStartChargeEnergy[MAX_SINGLE_STBATT_NUM];
    float                               fSingleChargeEnergy[MAX_SINGLE_STBATT_NUM];
    double                              fSingleStartDischargeEnergy[MAX_SINGLE_STBATT_NUM];
    float                               fSingleDischargeEnergy[MAX_SINGLE_STBATT_NUM];  
    double                              tm_fLastCalcBattEnergyCostbyBMS;
    BATT_FULLY_CHGDISCHG_STATE          emFullyChgDischgState[MAX_SINGLE_STBATT_NUM];
    // LJH240715 aircond
    int                                 nSetTemp4Aircond;
    ESS_DEVICE_TYPE                     emESSDevType;
    //---------- BMS TestMode Parm ----------//
    A_BLOB_BMS_TESTMODE_SET             bmsTestModeParm;
    // BMS_TESTMODE_PARM			        autoTestParm[MAX_SINGLE_STBATT_NUM];
    A_BLOB_BMS_TESTMODE_RESULT          bmsAutoTestResult[MAX_SINGLE_STBATT_NUM];
    A_BLOB_BMS_TESTMODE_PROCESS         bmsTestProcessRecord[MAX_SINGLE_STBATT_NUM];
    // LJH250408 CtrlByREMS
    BOOL                                enableREMSCtrl;
    BOOL                                executeChargingWhenReflux;
    //WLD01206 IMEU3 control
    UINT                                nAirCondi_StartUp_Num;
    time_t                              t_AirCondi_StartUp_Time;
    UINT                                nSmokeDetector_num;
    time_t                              t_Smoke_time;
    BOOL                                bSmokeFlag;
    // Jeffery250325 ​Internal Resistance Control
    int                                 nRintFreqIdx[MAX_SINGLE_STBATT_NUM];
    float                               f35KW_OpenTemp;
    float                               f35KW_ShutdownTemp;

    // manu record bms-mu2 can log
    BOOL                                bNeedRecBmsMu2CanLog;
    double                              tm_fLastStartRecBmsMu2canlog;
    // auto record realtime 200 bmu canlogs, and 200 canlogs after alm active 
    CanRecCirBuffer                     bmuCanRecCirBuffer;
    CanRecBuffer                        bmuAlmCanRecCirBuffer;
    double                              tm_fLastRecBmsAlmcanlog;

    // hot-wait flag
    BOOL                                bHotWaitFlag;

    char                                szBsuVersion[16];
    LCCfg                               lcCfg; // LJH250508 LCCfg
    time_t                              t_QF3_triger; 
    UINT                                QF3_Enable;
    // LJH250930 加快自动充放响应,StopSOC_SetSEMS
    BYTE                                nStopSOC_ChrgSet_SEMS; // 充电截止SOC(REMS、自动充电) 1001, 3306,
	BYTE                                nStopSOC_DiscSet_SEMS; // 放电截止SOC(REMS、自动放电) 1001, 3309,
    BOOL                                bStandby_Set; // 1, 85,

    BOOL bAir537NeedClear[MAX_SINGLE_STBATT_NUM]; //记录537参数的待清除状态（0=无需清除，1=待清除）
    
    // XSY251208 电池防欠压(必需)防过充(可选)保护
    BOOL                                bAntiOverChg;
    BOOL                                bBattProt;

    // LZY01508 20260126 Power Ctrl Enable
	POWER_CTRL_ENABLE_STATE 	        emPIDEnableState;        // PID使能
	POWER_CTRL_ENABLE_STATE 	        emHighSensitivityEnable;   // 高灵敏度使能
    BOOL                                bKeepPowerMode[MAX_SINGLE_STBATT_NUM]; // 保供电模式 0：正常 1：保供电

    SOCCalibrationMethod                emSOCCalibration;
    UINT                                nCalibratedSOC;//校准后的SOC值，单位：千分比
    BOOL                                bIsCalibrated;//是否校准完成

    BOOL                                bIsBattStandingStable;          // 是否处于稳定静置状态
    CalibratedSOCData                   stCalibratedSOCData[MAX_RECORD_OCVCALIBRATIONDATA]; // 校准后的SOC数据
    int                                 nToTalRecNum;
    BOOL                                bOCVFlag;     //判断是否完成OCV校准
    int                                 nRunRecordInterval;

    // RPMSG 核间通信端口（与 CAN 并存）
    HANDLE                              hRpmsgDev;          // RPMSG 端口句柄
    COMMPORT                            pRpmsgComm;        // RPMSG 端口驱动
    BOOL                                bRpmsgPortReady;   // RPMSG 端口准备就绪标志
}STBATT_SAMPLER_ROUGH;

typedef enum tagOCV_CAL_STAGE
{
    STAGE_CHARGE_DISCHARGE = 0,  // 充放电阶段
    STAGE_WAIT_IDLE,             // 充放电到空闲之间的等待阶段
    STAGE_IDLE,                  // 静置阶段
    STAGE_COMPLETED              // 所有阶段完成
} OCV_CAL_STAGE;

typedef struct tagVoltageSOCMap
{
    UINT    nVoltage;  // 电压值，单位：毫伏(mV)
    UINT    nSOC;      // SOC值，单位：千分比
} VoltageSOCMap;

typedef struct stUDS_UPGRADE_PARAMETER
{
    BYTE byServiceID;
    BYTE bySubServiceID[3];
    BYTE bySubServiceLen;

}STUDS_UPGRADE_PARAMETER;

typedef struct stHEX2BIN_INFO
{
    int iBinDataLen;
    unsigned int uStartAddr;
    int iErrLine;
}HEX2BIN_INFO;

//-----   Public Function   s-----//
ssize_t Write_CAN1(IN STBATT_SAMPLER_ROUGH *pBattData, IN void * buf,IN size_t count);
void    CAN_ClearReadBuf(STBATT_SAMPLER_ROUGH *pBattData);
void    CAN_Filter_Conf(STBATT_SAMPLER_ROUGH *pBattData);

void    CtrlBattCabinetContactor(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int n, BOOL bCtrlDOClose);
BOOL    isBMSRunAbnormal(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, float fPackTotalCurr);
BOOL    JudgeIfNeedEnterOffGrid(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx);
BOOL    JudgeIfPCS_OnHotWaiting(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx);

TRIGGER_ISO_STATUS Judge_BMSIsoTriggerStatus(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx);

/*
//-----   for PL batt    -----//
void PL_Batt_Init(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);
void PL_Batt_Sample(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);
BOOL PL_Batt_Control(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, STBATT_CTRL_CMDTYPE emCmdTp, int nWhichBatt);

//For ND Batt
void ND_Batt_Init(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);
void ND_Batt_Sample(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);
BOOL ND_Batt_Control(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, STBATT_CTRL_CMDTYPE emCmdTp, int nWhichBatt);
*/

//For ND V33 Protocal
void ND_V33_Batt_Init(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);
void ND_V33_Batt_Sample(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);
BOOL ND_V33_Batt_Control(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, STBATT_CTRL_CMDTYPE emCmdTp, int nWhichBatt);
void ND_V33_Init_RoughData(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int n);

//For st-batt State-Machine and mgmt
STBATT_STATEMACHINE_STATE OnIdle(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx);
STBATT_STATEMACHINE_STATE OnFaulted(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx);
STBATT_STATEMACHINE_STATE OnWaitCharge(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx);
STBATT_STATEMACHINE_STATE OnCharging(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx);
STBATT_STATEMACHINE_STATE OnDischarging(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx);
STBATT_STATEMACHINE_STATE OnOffGrid(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx);

BOOL Send_Ctrl_STBatt(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, STBATT_CTRL_CMDTYPE emCmdTp, int nWhichBatt);
BOOL Send_Ctrl_CCU(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, GRP_RECT_CTRL_CMDTYPE emCmdType, int nBattIdx);
BOOL BattProtect(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx);
BOOL Send_Ctrl_V2B_Stop(STBATT_SAMPLER_ROUGH *pSTBatt_Rough,  int nBattIdx);


void Enter_ManualCtrlPMMode(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx);
void Enter_BMSTestMode(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx);
void Enter_BMSTestMode_FirstCharge(STBATT_SAMPLER_ROUGH *pSTBatt, int nBattIdx);

//----- For Infy BMS Protocal -----//
void YFY_Batt_Init(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);
void YFY_Batt_Sample(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);
BOOL YFY_Batt_Control(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, STBATT_CTRL_CMDTYPE emCmdTp, int nBattIdx);
void YFY_Init_RoughData(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int n);

//----- For Infy BMS Rpmsg Protocal -----//
void YFY_Batt_Rpmsg_Init(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);
void YFY_Batt_Rpmsg_Sample(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);
void MapRawData_2_InfyBMSBlob(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);

//-----Fengchao Battery Function -----//
void FengCH_Batt_Sample(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);
BOOL FengCH_Batt_Control(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, STBATT_CTRL_CMDTYPE emCmdTp, int nWhichBatt);
void FengCH_Batt_Init(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);

void* _Thread_BMSCanDataSave(void* pArgs);
BOOL Start_WriteMsg2USB(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, UINT nBMSIdx, TP_TRIGGER_MSG emMsgType);
BOOL Pending_WriteMsg2USB(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, UINT nBMSIdx, BYTE* pMsgData, UINT nMsgNum);
BOOL End_WriteMsg2USB(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, UINT nBMSIdx);
char *GetMsgTypeStrByMsgTPId(TP_TRIGGER_MSG emMsgType, char *szMsgType);

void Update_BMSHealthData2DB(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx);

//---------------BMS Test Mode ----------------//
static STATE_MACHINE_STAT_TEST OnTest_Idle(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);
static STATE_MACHINE_STAT_TEST OnTest_Self_Check(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);
static STATE_MACHINE_STAT_TEST OnTest_FullDischg_Step1(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);
static STATE_MACHINE_STAT_TEST OnTest_FullChg_Step1(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);

static STATE_MACHINE_STAT_TEST OnTest_Sleep_Step2(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);
static STATE_MACHINE_STAT_TEST OnTest_FullChg_Step3(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);
static STATE_MACHINE_STAT_TEST OnTest_FullDischg_Step3(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);

static STATE_MACHINE_STAT_TEST OnTest_Sleep_Step4(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);;
static STATE_MACHINE_STAT_TEST OnTest_FullDischg_Step5(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);
static STATE_MACHINE_STAT_TEST OnTest_FullChg_Step5(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);

static STATE_MACHINE_STAT_TEST OnTest_Sleep_Step6(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);
static STATE_MACHINE_STAT_TEST OnTest_Chg_Half_Step7(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);
static STATE_MACHINE_STAT_TEST OnTest_Dischg_Half_Step7(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);

static STATE_MACHINE_STAT_TEST OnTest_Finished(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);
static STATE_MACHINE_STAT_TEST OnTest_Paused(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);
//
////Fail handler
static void OnFail(STBATT_SAMPLER_ROUGH* pSTBatt, IN FAIL_FROM emFailFrom, IN TESTMODE_FAIL_CODE emFailCode, int nBattIdx);
static BOOL Initial_TestMode_MachineState(STBATT_SAMPLER_ROUGH *pSTBatt);
static BOOL Initial_TestMode_MachineState_FirstCharge(STBATT_SAMPLER_ROUGH *pSTBatt); //先充 再放，用于不同机柜测试时的充放电对拖
static void Prepare_Test_StateMachine(STBATT_SAMPLER_ROUGH *pSTBatt);
static BOOL Initial_TestMode_MachineState_Tow(STBATT_SAMPLER_ROUGH *pSTBatt);
static void Prepare_Test_StateMachine_Tow(STBATT_SAMPLER_ROUGH *pSTBatt);

#define Grp_gap 1    // Setting for batt0 tows with batt1, batt2 tows with batt3, so fix Grp_gap == 1

BOOL Send_StopCmd(STBATT_SAMPLER_ROUGH *pSTBatt, int nBattIdx, STATE_MANUL_CTRL emCtrlState);
BOOL Send_CtrlCmd(STBATT_SAMPLER_ROUGH *pSTBatt, int nBattIdx, STATE_MANUL_CTRL emCtrlState, float inLimitCurrent);
STATE_TEST_RES SelfCheckTest(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);
STATE_TEST_RES ChargeEndTest(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);
STATE_TEST_RES DischargeEndTest(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);
STATE_TEST_RES TotalTest(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);

STATE_TEST_RES SelfCheckTest_ShortTime(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);
STATE_TEST_RES ChargeEndTest_ShortTime(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);
STATE_TEST_RES DischargeEndTest_ShortTime(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);
STATE_TEST_RES TotalTest_ShortTime(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx);

time_t tm_LastSaveData; // for save dynamic test data
void ClearTestHisData(STBATT_SAMPLER_ROUGH* pSTBatt);
void Save_CellInfo(STBATT_SAMPLER_ROUGH *pSTBatt, int nBattIdx, time_t tmNow);


//---BMS .Txt Upgrade---//
void HandleBatteryUpgrading(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);
void Make_SendCmd_4CANFrame(IN A_CAN_ND_BATT_FRAME *pFrm);
//--------------------End-----------------------//

int IMEU3SendDOCtlCAN(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int BatIdx, BYTE WhichDo, BYTE Value);
void QueryBSUVersion(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);
void YFY_C2S_Query_Barcode(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int ntype, int nPackIdx, int nCellIdx);
void YFY_C2S_Write_Barcode(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int ntype, int nPackIdx, int nCellIdx, char *szBarcode);

void RecBmsMu2Log(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, BYTE* pbySendBuf, ssize_t uFrameSize);
void RecBmsMu2AlarmLog(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, BYTE* pbySendBuf, ssize_t uFrameSize);

int BMU02_SendFrameByArFrame(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, const AR_FRAME* pArFrame);
BOOL GetSetParameter(AR_FRAME* ar_Frame, BYTE FunCode, int nParamId, const BYTE* tx_data, int len);
void ReadAndParseBmsStatus(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);
void ReadAndParseConfigParameters(STBATT_SAMPLER_ROUGH *pSTBatt_Rough);


#endif


