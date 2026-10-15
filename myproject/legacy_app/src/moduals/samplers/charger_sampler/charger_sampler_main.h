#ifndef __CHARGER_SAMPLER_MAIN_H
#define __CHARGER_SAMPLER_MAIN_H

#include "modbus-tcp.h"
#include "../sampler_base.h"
#include "../../cJSON.h"

typedef enum {
    CONN_TYPE_RTU,
    CONN_TYPE_TCP
}MODBUS_CONN_TYPE;

// 点位类型枚举
typedef enum {
    POINT_TYPE_COIL,           // 线圈 (可读写)
    POINT_TYPE_DISCRETE_INPUT, // 离散输入 (只读)
    POINT_TYPE_HOLDING_REG,    // 保持寄存器 (可读写)
    POINT_TYPE_INPUT_REG       // 输入寄存器 (只读)
}MODBUS_POINT_TYPE;

// 数据类型枚举
typedef enum {
    TRANS_TYPE_BOOL,      // 布尔
    TRANS_TYPE_INT16,     // 16位整数
    TRANS_TYPE_UINT16,    // 16位无符号整数
    TRANS_TYPE_INT32,     // 32位整数 (占2个寄存器)
    TRANS_TYPE_UINT32,    // 32位无符号整数
    TRANS_TYPE_FLOAT,     // 浮点数 (占2个寄存器)
    TRANS_TYPE_DOUBLE,    // 双精度浮点数 (占4个寄存器)
    TRANS_TYPE_STRING     // 字符串 (占多个寄存器)
}MODBUS_TRANS_TYPE;

typedef enum {
    DATA_VALUE_BOOL,    // 布尔
    DATA_VALUE_INT,     // 整数
    DATA_VALUE_UINT,    // 无符号整数
    DATA_VALUE_FLOAT,   // 浮点数
    DATA_VALUE_STRING   // 字符串
}MODBUS_VALUE_TYPE;

typedef enum
{
    // 2.2.1.Charging Data
    MID_SysAvaStu,
    MID_Connector1AvaStu,
    MID_Connector2AvaStu,
    MID_Connector3AvaStu,
    MID_Con1_L1_N_VolExport,
    MID_Con1_L2_N_VolExport,
    MID_Con1_L3_N_VolExport,
    MID_Con2_L1_N_VolExport,
    MID_Con2_L2_N_VolExport,
    MID_Con2_L3_N_VolExport,
    MID_Con3_L1_N_VolExport,
    MID_Con3_L2_N_VolExport,
    MID_Con3_L3_N_VolExport,
    MID_Con1_L1_N_CurExport,
    MID_Con1_L2_N_CurExport,
    MID_Con1_L3_N_CurExport,
    MID_Con2_L1_N_CurExport,
    MID_Con2_L2_N_CurExport,
    MID_Con2_L3_N_CurExport,
    MID_Con3_L1_N_CurExport,
    MID_Con3_L2_N_CurExport,
    MID_Con3_L3_N_CurExport,
    MID_Con1CurrentSOC,
    MID_Con2CurrentSOC,
    MID_Con3CurrentSOC,
    MID_Con1RTEnergyCost,
    MID_Con2RTEnergyCost,
    MID_Con3RTEnergyCost,
    MID_Con1PredictRemaintime,
    MID_Con2PredictRemaintime,
    MID_Con3PredictRemaintime,
    MID_Conn1PlugStatus,
    MID_Conn2PlugStatus,
    MID_Conn3PlugStatus,
    MID_Con1ChargingStatus,
    MID_Con2ChargingStatus,
    MID_Con3ChargingStatus,
    MID_Connector4AvaStu,
    MID_Connector5AvaStu,
    MID_Connector6AvaStu,
    MID_Con4_L1_N_VolExport,
    MID_Con4_L2_N_VolExport,
    MID_Con4_L3_N_VolExport,
    MID_Con5_L1_N_VolExport,
    MID_Con5_L2_N_VolExport,
    MID_Con5_L3_N_VolExport,
    MID_Con6_L1_N_VolExport,
    MID_Con6_L2_N_VolExport,
    MID_Con6_L3_N_VolExport,
    MID_Con4_L1_N_CurExport,
    MID_Con4_L2_N_CurExport,
    MID_Con4_L3_N_CurExport,
    MID_Con5_L1_N_CurExport,
    MID_Con5_L2_N_CurExport,
    MID_Con5_L3_N_CurExport,
    MID_Con6_L1_N_CurExport,
    MID_Con6_L2_N_CurExport,
    MID_Con6_L3_N_CurExport,
    MID_Con4CurrentSOC,
    MID_Con5CurrentSOC,
    MID_Con6CurrentSOC,
    MID_Con4RTEnergyCost,
    MID_Con5RTEnergyCost,
    MID_Con6RTEnergyCost,
    MID_Con4PredictRemaintime,
    MID_Con5PredictRemaintime,
    MID_Con6PredictRemaintime,
    MID_Conn4PlugStatus,
    MID_Conn5PlugStatus,
    MID_Conn6PlugStatus,
    MID_Con4ChargingStatus,
    MID_Con5ChargingStatus,
    MID_Con6ChargingStatus,
    MID_Con1MeterTotalEnergy,
    MID_Con2MeterTotalEnergy,
    MID_Con3MeterTotalEnergy,
    MID_RdChgerInputPowerLimitStatic,
    MID_RdChgerInputPowerLimitDynamic,

    // 2.2.2.Charging Data(Dev Info)
    MID_SerialNumber,
    MID_InputPowerlimit,
    MID_EV_RTTotalPower,

    // 2.2.3 ESS Data
    MID_MaxDischgAbility2ACload,
    MID_RTPowerDischg4ACload,

    // 2.3.ES Battery Data
    MID_TotalBattGroupsNum,
    // 1# Battery Information
    MID_Batt1_RunStatus,
    MID_Batt1_Voltage,
    MID_Batt1_Current,
    MID_Batt1_SOH,
    MID_Batt1_SOC,
    MID_Batt1_MaxCellVoltage,
    MID_Batt1_MinCellVoltage,
    MID_Batt1_AvgCellVoltage,
    MID_Batt1_MaxCellTemp,
    MID_Batt1_MinCellTemp,
    MID_Batt1_AvgCellTemp,
    MID_Batt1_ChgDischgEnable,
    MID_Batt1_AlarmStatus,
    // 2# Battery Information
    MID_Batt2_RunStatus,
    MID_Batt2_Voltage,
    MID_Batt2_Current,
    MID_Batt2_SOH,
    MID_Batt2_SOC,
    MID_Batt2_MaxCellVoltage,
    MID_Batt2_MinCellVoltage,
    MID_Batt2_AvgCellVoltage,
    MID_Batt2_MaxCellTemp,
    MID_Batt2_MinCellTemp,
    MID_Batt2_AvgCellTemp,
    MID_Batt2_ChgDischgEnable,
    MID_Batt2_AlarmStatus,
    // 3# Battery Information
    MID_Batt3_RunStatus,
    MID_Batt3_Voltage,
    MID_Batt3_Current,
    MID_Batt3_SOH,
    MID_Batt3_SOC,
    MID_Batt3_MaxCellVoltage,
    MID_Batt3_MinCellVoltage,
    MID_Batt3_AvgCellVoltage,
    MID_Batt3_MaxCellTemp,
    MID_Batt3_MinCellTemp,
    MID_Batt3_AvgCellTemp,
    MID_Batt3_ChgDischgEnable,
    MID_Batt3_AlarmStatus,
    // 4# Battery Information
    MID_Batt4_RunStatus,
    MID_Batt4_Voltage,
    MID_Batt4_Current,
    MID_Batt4_SOH,
    MID_Batt4_SOC,
    MID_Batt4_MaxCellVoltage,
    MID_Batt4_MinCellVoltage,
    MID_Batt4_AvgCellVoltage,
    MID_Batt4_MaxCellTemp,
    MID_Batt4_MinCellTemp,
    MID_Batt4_AvgCellTemp,
    MID_Batt4_ChgDischgEnable,
    MID_Batt4_AlarmStatus,

    // 2.4.1.Remote Start/Stop Charging (idTag not provided)
    MID_ChgCtrl_Connector1,
    MID_ChgCtrl_Connector2,
    MID_ChgCtrl_Connector3,
    MID_ChgCtrl_Connector4,
    MID_ChgCtrl_Connector5,
    MID_ChgCtrl_Connector6,

    // 2.4.2.Remote Start/ Stop Charging (idTag provided)
    MID_StartChg_Conn1_IdTag,
    MID_StopChg_Conn1_IdTag,
    MID_StartChg_Conn2_IdTag,
    MID_StopChg_Conn2_IdTag,
    MID_StartChg_Conn3_IdTag,
    MID_StopChg_Conn3_IdTag,

    // 2.4.3.System Parameter Ctrl
    // System Parameter Set/Ctrl Addresses  
    MID_ChgerInputPowerLimit_Static,
    MID_ChargerSwitch,
    MID_Conn1_MaxOutputCurrent,
    MID_Conn2_MaxOutputCurrent,
    MID_Conn3_MaxOutputCurrent,
    MID_Conn1_MaxOutputPower,
    MID_Conn2_MaxOutputPower,
    MID_Conn3_MaxOutputPower,
    MID_ChgerInputPowerLimit_Dynamic,
    MID_Conn4_MaxOutputCurrent,
    MID_Conn5_MaxOutputCurrent,
    MID_Conn6_MaxOutputCurrent,
    MID_Conn4_MaxOutputPower,
    MID_Conn5_MaxOutputPower,
    MID_Conn6_MaxOutputPower,
    MID_ACLoadDemandPower,
    MID_OfflineSafeMode_Enable,
    MID_OfflineSafeMode_Timeout,
    MID_OfflineSafeMode_MaxPowerLimit,

    // 2.5.Energy management plan setting
    // (pass)

    MID_MAX
}MODBUS_DATA_ID;

typedef struct tagModbusDataMap
{
	MODBUS_DATA_ID	    emModbusDataId;
    MODBUS_POINT_TYPE   emModbusPointType;

	// BOOL				bNeedSpecialPro;

	int					nDevId;     // 数据库设备ID
	int					nSigType;   // 数据库表类型
	int					nSigId;     // 数据库参数ID

	MODBUS_TRANS_TYPE	emDataType;
    MODBUS_VALUE_TYPE   emValueType;

	int					nRegAddrStart;
	int					nRegNum;

	float				fScale;
	float			    fOffset;

	void				*ptrValue;
    void				*ptrValueReal;
}MODBUS_DATA_MAP;

typedef struct tagModbusDataRead
{
    // 2.2.1.Charging Data
    // 系统可用状态
    uint16_t usSysAvaStu;

    // 枪1-3可用状态
    uint16_t usConnector1AvaStu;
    uint16_t usConnector2AvaStu;
    uint16_t usConnector3AvaStu;

    // 枪1三相电压
    int16_t sCon1_L1_N_VolExport;
    int16_t sCon1_L2_N_VolExport;
    int16_t sCon1_L3_N_VolExport;

    // 枪2三相电压
    int16_t sCon2_L1_N_VolExport;
    int16_t sCon2_L2_N_VolExport;
    int16_t sCon2_L3_N_VolExport;

    // 枪3三相电压
    int16_t sCon3_L1_N_VolExport;
    int16_t sCon3_L2_N_VolExport;
    int16_t sCon3_L3_N_VolExport;

    // 枪1三相电流
    int16_t sCon1_L1_N_CurExport;
    int16_t sCon1_L2_N_CurExport;
    int16_t sCon1_L3_N_CurExport;

    // 枪2三相电流
    int16_t sCon2_L1_N_CurExport;
    int16_t sCon2_L2_N_CurExport;
    int16_t sCon2_L3_N_CurExport;

    // 枪3三相电流
    int16_t sCon3_L1_N_CurExport;
    int16_t sCon3_L2_N_CurExport;
    int16_t sCon3_L3_N_CurExport;

    // 枪1-3当前SOC
    uint16_t usCon1CurrentSOC;
    uint16_t usCon2CurrentSOC;
    uint16_t usCon3CurrentSOC;

    // 枪1-3实时能量成本
    uint16_t usCon1RTEnergyCost;
    uint16_t usCon2RTEnergyCost;
    uint16_t usCon3RTEnergyCost;

    // 枪1-3预计剩余时间
    uint16_t usCon1PredictRemaintime;
    uint16_t usCon2PredictRemaintime;
    uint16_t usCon3PredictRemaintime;

    // 枪1-3插头状态
    uint16_t usConn1PlugStatus;
    uint16_t usConn2PlugStatus;
    uint16_t usConn3PlugStatus;

    // 枪1-3充电状态
    uint16_t usCon1ChargingStatus;
    uint16_t usCon2ChargingStatus;
    uint16_t usCon3ChargingStatus;

    // 枪4-6可用状态
    uint16_t usConnector4AvaStu;
    uint16_t usConnector5AvaStu;
    uint16_t usConnector6AvaStu;

    // 枪4三相电压
    int16_t sCon4_L1_N_VolExport;
    int16_t sCon4_L2_N_VolExport;
    int16_t sCon4_L3_N_VolExport;

    // 枪5三相电压
    int16_t sCon5_L1_N_VolExport;
    int16_t sCon5_L2_N_VolExport;
    int16_t sCon5_L3_N_VolExport;

    // 枪6三相电压
    int16_t sCon6_L1_N_VolExport;
    int16_t sCon6_L2_N_VolExport;
    int16_t sCon6_L3_N_VolExport;

    // 枪4三相电流
    int16_t sCon4_L1_N_CurExport;
    int16_t sCon4_L2_N_CurExport;
    int16_t sCon4_L3_N_CurExport;

    // 枪5三相电流
    int16_t sCon5_L1_N_CurExport;
    int16_t sCon5_L2_N_CurExport;
    int16_t sCon5_L3_N_CurExport;

    // 枪6三相电流
    int16_t sCon6_L1_N_CurExport;
    int16_t sCon6_L2_N_CurExport;
    int16_t sCon6_L3_N_CurExport;

    // 枪4-6当前SOC
    uint16_t usCon4CurrentSOC;
    uint16_t usCon5CurrentSOC;
    uint16_t usCon6CurrentSOC;

    // 枪4-6实时能量成本
    uint16_t usCon4RTEnergyCost;
    uint16_t usCon5RTEnergyCost;
    uint16_t usCon6RTEnergyCost;

    // 枪4-6预计剩余时间
    uint16_t usCon4PredictRemaintime;
    uint16_t usCon5PredictRemaintime;
    uint16_t usCon6PredictRemaintime;

    // 枪4-6插头状态
    uint16_t usConn4PlugStatus;
    uint16_t usConn5PlugStatus;
    uint16_t usConn6PlugStatus;

    // 枪4-6充电状态
    uint16_t usCon4ChargingStatus;
    uint16_t usCon5ChargingStatus;
    uint16_t usCon6ChargingStatus;

    // 枪电表能量
    uint16_t usCon1MeterTotalEnergy;
    uint16_t usCon2MeterTotalEnergy;
    uint16_t usCon3MeterTotalEnergy;
    
    // 充电桩输入功率限制
    uint16_t usRdChgerInputPowerLimitStatic;
    uint16_t usRdChgerInputPowerLimitDynamic;

    // 2.2.2.Charging Data(Dev Info)
    char        szSerialNumber[26];
    uint16_t    usInputPowerlimit;
    uint16_t    usEV_RTTotalPower;

    // 2.2.3 ESS Data
    uint16_t usMaxDischgAbility2ACload;
    uint16_t usRTPowerDischg4ACload;

    // 2.3.ES Battery Data
    uint16_t usTotalBattGroupsNum;
    // 1# Battery Information
    uint16_t usBatt1_RunStatus;
    uint16_t usBatt1_Voltage;
    uint16_t usBatt1_Current;
    uint16_t usBatt1_SOH;
    uint16_t usBatt1_SOC;
    uint16_t usBatt1_MaxCellVoltage;
    uint16_t usBatt1_MinCellVoltage;
    uint16_t usBatt1_AvgCellVoltage;
    uint16_t usBatt1_MaxCellTemp;
    uint16_t usBatt1_MinCellTemp;
    uint16_t usBatt1_AvgCellTemp;
    uint16_t usBatt1_ChgDischgEnable;
    uint16_t usBatt1_AlarmStatus;
    // 2# Battery Information
    uint16_t usBatt2_RunStatus;
    uint16_t usBatt2_Voltage;
    uint16_t usBatt2_Current;
    uint16_t usBatt2_SOH;
    uint16_t usBatt2_SOC;
    uint16_t usBatt2_MaxCellVoltage;
    uint16_t usBatt2_MinCellVoltage;
    uint16_t usBatt2_AvgCellVoltage;
    uint16_t usBatt2_MaxCellTemp;
    uint16_t usBatt2_MinCellTemp;
    uint16_t usBatt2_AvgCellTemp;
    uint16_t usBatt2_ChgDischgEnable;
    uint16_t usBatt2_AlarmStatus;
    // 3# Battery Information
    uint16_t usBatt3_RunStatus;
    uint16_t usBatt3_Voltage;
    uint16_t usBatt3_Current;
    uint16_t usBatt3_SOH;
    uint16_t usBatt3_SOC;
    uint16_t usBatt3_MaxCellVoltage;
    uint16_t usBatt3_MinCellVoltage;
    uint16_t usBatt3_AvgCellVoltage;
    uint16_t usBatt3_MaxCellTemp;
    uint16_t usBatt3_MinCellTemp;
    uint16_t usBatt3_AvgCellTemp;
    uint16_t usBatt3_ChgDischgEnable;
    uint16_t usBatt3_AlarmStatus;
    // 4# Battery Information
    uint16_t usBatt4_RunStatus;
    uint16_t usBatt4_Voltage;
    uint16_t usBatt4_Current;
    uint16_t usBatt4_SOH;
    uint16_t usBatt4_SOC;
    uint16_t usBatt4_MaxCellVoltage;
    uint16_t usBatt4_MinCellVoltage;
    uint16_t usBatt4_AvgCellVoltage;
    uint16_t usBatt4_MaxCellTemp;
    uint16_t usBatt4_MinCellTemp;
    uint16_t usBatt4_AvgCellTemp;
    uint16_t usBatt4_ChgDischgEnable;
    uint16_t usBatt4_AlarmStatus;

    // 2.4.1.Remote Start/Stop Charging (idTag not provided)
    uint16_t usChgCtrl_Connector1;
    uint16_t usChgCtrl_Connector2;
    uint16_t usChgCtrl_Connector3;
    uint16_t usChgCtrl_Connector4;
    uint16_t usChgCtrl_Connector5;
    uint16_t usChgCtrl_Connector6;

    // 2.4.2.Remote Start/ Stop Charging (idTag provided)
    char szStartChg_Conn1_IdTag[32];
    char szStopChg_Conn1_IdTag[32];
    char szStartChg_Conn2_IdTag[32];
    char szStopChg_Conn2_IdTag[32];
    char szStartChg_Conn3_IdTag[32];
    char szStopChg_Conn3_IdTag[32];

    // 2.4.3.System Parameter Ctrl
    // System Parameter Set/Ctrl Addresses  
    uint16_t usChgerInputPowerLimit_Static;
    uint16_t usChargerSwitch;
    uint16_t usConn1_MaxOutputCurrent;
    uint16_t usConn2_MaxOutputCurrent;
    uint16_t usConn3_MaxOutputCurrent;
    uint16_t usConn1_MaxOutputPower;
    uint16_t usConn2_MaxOutputPower;
    uint16_t usConn3_MaxOutputPower;
    uint16_t usChgerInputPowerLimit_Dynamic;
    uint16_t usConn4_MaxOutputCurrent;
    uint16_t usConn5_MaxOutputCurrent;
    uint16_t usConn6_MaxOutputCurrent;
    uint16_t usConn4_MaxOutputPower;
    uint16_t usConn5_MaxOutputPower;
    uint16_t usConn6_MaxOutputPower;
    uint16_t usACLoadDemandPower;
    uint16_t usOfflineSafeMode_Enable;
    uint16_t usOfflineSafeMode_Timeout;
    uint16_t usOfflineSafeMode_MaxPowerLimit;

    // 2.5.Energy management plan setting(pass)

}MODBUS_DATA_READ;

typedef struct stModbusDataValue
{
    // 2.2.1.Charging Data
    // 系统可用状态
    unsigned int nSysAvaStu;

    // 枪1-3可用状态
    unsigned int nConnector1AvaStu;
    unsigned int nConnector2AvaStu;
    unsigned int nConnector3AvaStu;

    // 枪1三相电压
    float fCon1_L1_N_VolExport;
    float fCon1_L2_N_VolExport;
    float fCon1_L3_N_VolExport;

    // 枪2三相电压
    float fCon2_L1_N_VolExport;
    float fCon2_L2_N_VolExport;
    float fCon2_L3_N_VolExport;

    // 枪3三相电压
    float fCon3_L1_N_VolExport;
    float fCon3_L2_N_VolExport;
    float fCon3_L3_N_VolExport;

    // 枪1三相电流
    float fCon1_L1_N_CurExport;
    float fCon1_L2_N_CurExport;
    float fCon1_L3_N_CurExport;

    // 枪2三相电流
    float fCon2_L1_N_CurExport;
    float fCon2_L2_N_CurExport;
    float fCon2_L3_N_CurExport;

    // 枪3三相电流
    float fCon3_L1_N_CurExport;
    float fCon3_L2_N_CurExport;
    float fCon3_L3_N_CurExport;

    // 枪1-3当前SOC
    unsigned int nCon1CurrentSOC;
    unsigned int nCon2CurrentSOC;
    unsigned int nCon3CurrentSOC;

    // 枪1-3实时能量成本
    float fCon1RTEnergyCost;
    float fCon2RTEnergyCost;
    float fCon3RTEnergyCost;

    // 枪1-3预计剩余时间
    float fCon1PredictRemaintime;
    float fCon2PredictRemaintime;
    float fCon3PredictRemaintime;

    // 枪1-3插头状态
    unsigned int nConn1PlugStatus;
    unsigned int nConn2PlugStatus;
    unsigned int nConn3PlugStatus;

    // 枪1-3充电状态
    unsigned int nCon1ChargingStatus;
    unsigned int nCon2ChargingStatus;
    unsigned int nCon3ChargingStatus;

    // 枪4-6可用状态
    unsigned int nConnector4AvaStu;
    unsigned int nConnector5AvaStu;
    unsigned int nConnector6AvaStu;

    // 枪4三相电压
    float fCon4_L1_N_VolExport;
    float fCon4_L2_N_VolExport;
    float fCon4_L3_N_VolExport;

    // 枪5三相电压
    float fCon5_L1_N_VolExport;
    float fCon5_L2_N_VolExport;
    float fCon5_L3_N_VolExport;

    // 枪6三相电压
    float fCon6_L1_N_VolExport;
    float fCon6_L2_N_VolExport;
    float fCon6_L3_N_VolExport;

    // 枪4三相电流
    float fCon4_L1_N_CurExport;
    float fCon4_L2_N_CurExport;
    float fCon4_L3_N_CurExport;

    // 枪5三相电流
    float fCon5_L1_N_CurExport;
    float fCon5_L2_N_CurExport;
    float fCon5_L3_N_CurExport;

    // 枪6三相电流
    float fCon6_L1_N_CurExport;
    float fCon6_L2_N_CurExport;
    float fCon6_L3_N_CurExport;

    // 枪4-6当前SOC
    unsigned int nCon4CurrentSOC;
    unsigned int nCon5CurrentSOC;
    unsigned int nCon6CurrentSOC;

    // 枪4-6实时能量成本
    float fCon4RTEnergyCost;
    float fCon5RTEnergyCost;
    float fCon6RTEnergyCost;

    // 枪4-6预计剩余时间
    float fCon4PredictRemaintime;
    float fCon5PredictRemaintime;
    float fCon6PredictRemaintime;

    // 枪4-6插头状态
    unsigned int nConn4PlugStatus;
    unsigned int nConn5PlugStatus;
    unsigned int nConn6PlugStatus;

    // 枪4-6充电状态
    unsigned int nCon4ChargingStatus;
    unsigned int nCon5ChargingStatus;
    unsigned int nCon6ChargingStatus;

    // 枪电表能量
    float fCon1MeterTotalEnergy;
    float fCon2MeterTotalEnergy;
    float fCon3MeterTotalEnergy;

    // 充电桩输入功率限制
    float fRdChgerInputPowerLimitStatic;
    float fRdChgerInputPowerLimitDynamic;

    // 2.2.2.Charging Data(Dev Info)
    char     szSerialNumber[26];
    float    fInputPowerlimit;
    float    fEV_RTTotalPower;

    // 2.2.3 ESS Data
    float fMaxDischgAbility2ACload;
    float fRTPowerDischg4ACload;

    // 2.3.ES Battery Data
    int nTotalBattGroupsNum;
    // 1# Battery Information
    unsigned int     nBatt1_RunStatus;
    float   fBatt1_Voltage;
    float   fBatt1_Current;
    unsigned int     nBatt1_SOH;
    unsigned int     nBatt1_SOC;
    float   fBatt1_MaxCellVoltage;
    float   fBatt1_MinCellVoltage;
    float   fBatt1_AvgCellVoltage;
    float   fBatt1_MaxCellTemp;
    float   fBatt1_MinCellTemp;
    float   fBatt1_AvgCellTemp;
    unsigned int     nBatt1_ChgDischgEnable;
    unsigned int     nBatt1_AlarmStatus;
    // 2# Battery Information
    unsigned int     nBatt2_RunStatus;
    float   fBatt2_Voltage;
    float   fBatt2_Current;
    unsigned int     nBatt2_SOH;
    unsigned int     nBatt2_SOC;
    float   fBatt2_MaxCellVoltage;
    float   fBatt2_MinCellVoltage;
    float   fBatt2_AvgCellVoltage;
    float   fBatt2_MaxCellTemp;
    float   fBatt2_MinCellTemp;
    float   fBatt2_AvgCellTemp;
    unsigned int     nBatt2_ChgDischgEnable;
    unsigned int     nBatt2_AlarmStatus;
    // 3# Battery Information
    unsigned int     nBatt3_RunStatus;
    float   fBatt3_Voltage;
    float   fBatt3_Current;
    unsigned int     nBatt3_SOH;
    unsigned int     nBatt3_SOC;
    float   fBatt3_MaxCellVoltage;
    float   fBatt3_MinCellVoltage;
    float   fBatt3_AvgCellVoltage;
    float   fBatt3_MaxCellTemp;
    float   fBatt3_MinCellTemp;
    float   fBatt3_AvgCellTemp;
    unsigned int     nBatt3_ChgDischgEnable;
    unsigned int     nBatt3_AlarmStatus;
    // 4# Battery Information
    unsigned int     nBatt4_RunStatus;
    float   fBatt4_Voltage;
    float   fBatt4_Current;
    unsigned int     nBatt4_SOH;
    unsigned int     nBatt4_SOC;
    float   fBatt4_MaxCellVoltage;
    float   fBatt4_MinCellVoltage;
    float   fBatt4_AvgCellVoltage;
    float   fBatt4_MaxCellTemp;
    float   fBatt4_MinCellTemp;
    float   fBatt4_AvgCellTemp;
    unsigned int     nBatt4_ChgDischgEnable;
    unsigned int     nBatt4_AlarmStatus;

    // 2.4.1.Remote Start/Stop Charging (idTag not provided)
    unsigned int nChgCtrl_Connector1;
    unsigned int nChgCtrl_Connector2;
    unsigned int nChgCtrl_Connector3;
    unsigned int nChgCtrl_Connector4;
    unsigned int nChgCtrl_Connector5;
    unsigned int nChgCtrl_Connector6;

    // 2.4.2.Remote Start/ Stop Charging (idTag provided)
    char szStartChg_Conn1_IdTag[32];
    char szStopChg_Conn1_IdTag[32];
    char szStartChg_Conn2_IdTag[32];
    char szStopChg_Conn2_IdTag[32];
    char szStartChg_Conn3_IdTag[32];
    char szStopChg_Conn3_IdTag[32];

    // 2.4.3.System Parameter Ctrl
    // System Parameter Set/Ctrl Addresses  
    unsigned int nChgerInputPowerLimit_Static;
    unsigned int nChargerSwitch;
    unsigned int nConn1_MaxOutputCurrent;
    unsigned int nConn2_MaxOutputCurrent;
    unsigned int nConn3_MaxOutputCurrent;
    unsigned int nConn1_MaxOutputPower;
    unsigned int nConn2_MaxOutputPower;
    unsigned int nConn3_MaxOutputPower;
    unsigned int nChgerInputPowerLimit_Dynamic;
    unsigned int nConn4_MaxOutputCurrent;
    unsigned int nConn5_MaxOutputCurrent;
    unsigned int nConn6_MaxOutputCurrent;
    unsigned int nConn4_MaxOutputPower;
    unsigned int nConn5_MaxOutputPower;
    unsigned int nConn6_MaxOutputPower;
    unsigned int nACLoadDemandPower;
    unsigned int nOfflineSafeMode_Enable;
    unsigned int nOfflineSafeMode_Timeout;
    unsigned int nOfflineSafeMode_MaxPowerLimit;

    // 2.5 Energy Management Plan Setting(pass)

}MODBUS_DATA_VALUE;

typedef struct stModbusConnInfo
{
    char                szIp[32];
    int		            nPort;          // TCP is 502
    int		            nSlaveId;       // both TCP and RTU is 1
    int		            nRspTimeout;
    int		            nByteTimeout;
}MODBUS_CONN_INFO;

typedef struct stRoughChargerData
{
	HANDLE		                        hSampler;       //pointer to sampler
	int			                        nSamplerId;     //copy from sampler
	HANDLE		                        hSamplerLock;
    DATA_PROCESSOR*		                pDPR;

	modbus_t*							ctx;

    MODBUS_DATA_READ                    rdModbus;       // Modbus读取的原始数据

    MODBUS_DATA_VALUE                   DataValue;      // 根据偏移和比例调整解析后的数据值

    MODBUS_CONN_INFO                    DevInfo;        // Modbus连接信息

    MODBUS_DATA_MAP*                    ModbusDataMap;  // Modbus数据图
    int                                 nModbusDataMapNum;  // Modbus数据图数量

#define MAX_READ_SEG 8     // 最大分段读取数量
    int                                 nReadItemNum[MAX_READ_SEG];         // 数据图ModbusDataId每个分段数量
    int                                 nReadRegNum[MAX_READ_SEG];          // 各分段读寄存器的数量
    int                                 MapReadStartReg[MAX_READ_SEG];      // 各分段读寄存器的起始地址
    uint16_t                            *tab_rp_registers[MAX_READ_SEG];    // 各分段读寄存器缓存

}ROUGH_CHARGER_DATA;

BOOL ChargerModbusTCPInitialize(ROUGH_CHARGER_DATA *pCharger_Modbus_Rough);
int ChargerModbusTCPSample(SAMPLER* pSampler);
BOOL InitModbusDataMap(ROUGH_CHARGER_DATA* pModbusRough);

#endif