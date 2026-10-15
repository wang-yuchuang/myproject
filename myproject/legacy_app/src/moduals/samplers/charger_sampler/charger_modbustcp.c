#include "charger_sampler_main.h"

int InitModbusDataMap(ROUGH_CHARGER_DATA* pModbusRough)
{
    printf("InitModbusDataMap...\n");
    ROUGH_CHARGER_DATA* pModbus = pModbusRough;

    MODBUS_DATA_MAP ChargerModbusDataItem[] = 
    {
        // 2.2.1.Charging Data
        // 系统可用状态
        {MID_SysAvaStu,                 POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT,    0x0001, 1, 1, 0, (void*)&pModbus->rdModbus.usSysAvaStu, (void*)&pModbus->DataValue.nSysAvaStu},
        {MID_Connector1AvaStu,          POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT,    0x0002, 1, 1, 0, (void*)&pModbus->rdModbus.usConnector1AvaStu, (void*)&pModbus->DataValue.nConnector1AvaStu},
        {MID_Connector2AvaStu,          POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT,    0x0003, 1, 1, 0, (void*)&pModbus->rdModbus.usConnector2AvaStu, (void*)&pModbus->DataValue.nConnector2AvaStu},
        {MID_Connector3AvaStu,          POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT,    0x0004, 1, 1, 0, (void*)&pModbus->rdModbus.usConnector3AvaStu, (void*)&pModbus->DataValue.nConnector3AvaStu},

        // 枪1三相电压
        {MID_Con1_L1_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,   0x0005, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon1_L1_N_VolExport, (void*)&pModbus->DataValue.fCon1_L1_N_VolExport},
        {MID_Con1_L2_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,   0x0006, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon1_L2_N_VolExport, (void*)&pModbus->DataValue.fCon1_L2_N_VolExport},
        {MID_Con1_L3_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,   0x0007, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon1_L3_N_VolExport, (void*)&pModbus->DataValue.fCon1_L3_N_VolExport},

        // 枪2三相电压
        {MID_Con2_L1_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,   0x0008, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon2_L1_N_VolExport, (void*)&pModbus->DataValue.fCon2_L1_N_VolExport},
        {MID_Con2_L2_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,   0x0009, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon2_L2_N_VolExport, (void*)&pModbus->DataValue.fCon2_L2_N_VolExport},
        {MID_Con2_L3_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,   0x000A, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon2_L3_N_VolExport, (void*)&pModbus->DataValue.fCon2_L3_N_VolExport},

        // 枪3三相电压
        {MID_Con3_L1_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,   0x000B, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon3_L1_N_VolExport, (void*)&pModbus->DataValue.fCon3_L1_N_VolExport},
        {MID_Con3_L2_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,   0x000C, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon3_L2_N_VolExport, (void*)&pModbus->DataValue.fCon3_L2_N_VolExport},
        {MID_Con3_L3_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,   0x000D, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon3_L3_N_VolExport, (void*)&pModbus->DataValue.fCon3_L3_N_VolExport},

        // 枪1三相电流
        {MID_Con1_L1_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x000E, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon1_L1_N_CurExport, (void*)&pModbus->DataValue.fCon1_L1_N_CurExport},
        {MID_Con1_L2_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x000F, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon1_L2_N_CurExport, (void*)&pModbus->DataValue.fCon1_L2_N_CurExport},
        {MID_Con1_L3_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0010, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon1_L3_N_CurExport, (void*)&pModbus->DataValue.fCon1_L3_N_CurExport},

        // 枪2三相电流
        {MID_Con2_L1_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0011, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon2_L1_N_CurExport, (void*)&pModbus->DataValue.fCon2_L1_N_CurExport},
        {MID_Con2_L2_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0012, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon2_L2_N_CurExport, (void*)&pModbus->DataValue.fCon2_L2_N_CurExport},
        {MID_Con2_L3_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0013, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon2_L3_N_CurExport, (void*)&pModbus->DataValue.fCon2_L3_N_CurExport},

        // 枪3三相电流
        {MID_Con3_L1_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0014, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon3_L1_N_CurExport, (void*)&pModbus->DataValue.fCon3_L1_N_CurExport},
        {MID_Con3_L2_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0015, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon3_L2_N_CurExport, (void*)&pModbus->DataValue.fCon3_L2_N_CurExport},
        {MID_Con3_L3_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0016, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon3_L3_N_CurExport, (void*)&pModbus->DataValue.fCon3_L3_N_CurExport},

        // 枪1-3当前SOC
        {MID_Con1CurrentSOC,            POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0017, 1, 1, 0, (void*)&pModbus->rdModbus.usCon1CurrentSOC, (void*)&pModbus->DataValue.nCon1CurrentSOC},
        {MID_Con2CurrentSOC,            POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0018, 1, 1, 0, (void*)&pModbus->rdModbus.usCon2CurrentSOC, (void*)&pModbus->DataValue.nCon2CurrentSOC},
        {MID_Con3CurrentSOC,            POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0019, 1, 1, 0, (void*)&pModbus->rdModbus.usCon3CurrentSOC, (void*)&pModbus->DataValue.nCon3CurrentSOC},

        // 枪1-3实时能量成本
        {MID_Con1RTEnergyCost,          POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x001A, 1, 0.01, 0, (void*)&pModbus->rdModbus.usCon1RTEnergyCost, (void*)&pModbus->DataValue.fCon1RTEnergyCost},
        {MID_Con2RTEnergyCost,          POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x001B, 1, 0.01, 0, (void*)&pModbus->rdModbus.usCon2RTEnergyCost, (void*)&pModbus->DataValue.fCon2RTEnergyCost},
        {MID_Con3RTEnergyCost,          POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x001C, 1, 0.01, 0, (void*)&pModbus->rdModbus.usCon3RTEnergyCost, (void*)&pModbus->DataValue.fCon3RTEnergyCost},

        // 枪1-3预计剩余时间
        {MID_Con1PredictRemaintime,     POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x001D, 1, 0.1, 0, (void*)&pModbus->rdModbus.usCon1PredictRemaintime, (void*)&pModbus->DataValue.fCon1PredictRemaintime},
        {MID_Con2PredictRemaintime,     POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x001E, 1, 0.1, 0, (void*)&pModbus->rdModbus.usCon2PredictRemaintime, (void*)&pModbus->DataValue.fCon2PredictRemaintime},
        {MID_Con3PredictRemaintime,     POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x001F, 1, 0.1, 0, (void*)&pModbus->rdModbus.usCon3PredictRemaintime, (void*)&pModbus->DataValue.fCon3PredictRemaintime},

        // 枪1-3插头状态
        {MID_Conn1PlugStatus,           POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0020, 1, 1, 0, (void*)&pModbus->rdModbus.usConn1PlugStatus, (void*)&pModbus->DataValue.nConn1PlugStatus},
        {MID_Conn2PlugStatus,           POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0021, 1, 1, 0, (void*)&pModbus->rdModbus.usConn2PlugStatus, (void*)&pModbus->DataValue.nConn2PlugStatus},
        {MID_Conn3PlugStatus,           POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0022, 1, 1, 0, (void*)&pModbus->rdModbus.usConn3PlugStatus, (void*)&pModbus->DataValue.nConn2PlugStatus},

        // 枪1-3充电状态
        {MID_Con1ChargingStatus,        POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0023, 1, 1, 0, (void*)&pModbus->rdModbus.usCon1ChargingStatus, (void*)&pModbus->DataValue.nCon1ChargingStatus},
        {MID_Con2ChargingStatus,        POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0024, 1, 1, 0, (void*)&pModbus->rdModbus.usCon2ChargingStatus, (void*)&pModbus->DataValue.nCon2ChargingStatus},
        {MID_Con3ChargingStatus,        POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0025, 1, 1, 0, (void*)&pModbus->rdModbus.usCon3ChargingStatus, (void*)&pModbus->DataValue.nCon3ChargingStatus},

        // 枪4-6可用状态
        {MID_Connector4AvaStu,          POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0026, 1, 1, 0, (void*)&pModbus->rdModbus.usConnector4AvaStu, (void*)&pModbus->DataValue.nConnector4AvaStu},
        {MID_Connector5AvaStu,          POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0027, 1, 1, 0, (void*)&pModbus->rdModbus.usConnector5AvaStu, (void*)&pModbus->DataValue.nConnector5AvaStu},
        {MID_Connector6AvaStu,          POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0028, 1, 1, 0, (void*)&pModbus->rdModbus.usConnector6AvaStu, (void*)&pModbus->DataValue.nConnector6AvaStu},

        // 枪4三相电压
        {MID_Con4_L1_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0029, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon4_L1_N_VolExport, (void*)&pModbus->DataValue.fCon4_L1_N_VolExport},
        {MID_Con4_L2_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x002A, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon4_L2_N_VolExport, (void*)&pModbus->DataValue.fCon4_L2_N_VolExport},
        {MID_Con4_L3_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x002B, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon4_L3_N_VolExport, (void*)&pModbus->DataValue.fCon4_L3_N_VolExport},

        // 枪5三相电压
        {MID_Con5_L1_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x002C, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon5_L1_N_VolExport, (void*)&pModbus->DataValue.fCon5_L1_N_VolExport},
        {MID_Con5_L2_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x002D, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon5_L2_N_VolExport, (void*)&pModbus->DataValue.fCon5_L2_N_VolExport},
        {MID_Con5_L3_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x002E, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon5_L3_N_VolExport, (void*)&pModbus->DataValue.fCon5_L3_N_VolExport},

        // 枪6三相电压
        {MID_Con6_L1_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x002F, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon6_L1_N_VolExport, (void*)&pModbus->DataValue.fCon6_L1_N_VolExport},
        {MID_Con6_L2_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0030, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon6_L2_N_VolExport, (void*)&pModbus->DataValue.fCon6_L2_N_VolExport},
        {MID_Con6_L3_N_VolExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0031, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon6_L3_N_VolExport, (void*)&pModbus->DataValue.fCon6_L3_N_VolExport},

        // 枪4三相电流
        {MID_Con4_L1_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0032, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon4_L1_N_CurExport, (void*)&pModbus->DataValue.fCon4_L1_N_CurExport},
        {MID_Con4_L2_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0033, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon4_L2_N_CurExport, (void*)&pModbus->DataValue.fCon4_L2_N_CurExport},
        {MID_Con4_L3_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0034, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon4_L3_N_CurExport, (void*)&pModbus->DataValue.fCon4_L3_N_CurExport},

        // 枪5三相电流
        {MID_Con5_L1_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0035, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon5_L1_N_CurExport, (void*)&pModbus->DataValue.fCon5_L1_N_CurExport},
        {MID_Con5_L2_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0036, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon5_L2_N_CurExport, (void*)&pModbus->DataValue.fCon5_L2_N_CurExport},
        {MID_Con5_L3_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0037, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon5_L3_N_CurExport, (void*)&pModbus->DataValue.fCon5_L3_N_CurExport},

        // 枪6三相电流
        {MID_Con6_L1_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0038, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon6_L1_N_CurExport, (void*)&pModbus->DataValue.fCon6_L1_N_CurExport},
        {MID_Con6_L2_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x0039, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon6_L2_N_CurExport, (void*)&pModbus->DataValue.fCon6_L2_N_CurExport},
        {MID_Con6_L3_N_CurExport,       POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_INT16, DATA_VALUE_FLOAT,  0x003A, 1, 0.1, 0, (void*)&pModbus->rdModbus.sCon6_L3_N_CurExport, (void*)&pModbus->DataValue.fCon6_L3_N_CurExport},

        // 枪4-6当前SOC
        {MID_Con4CurrentSOC,            POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x003B, 1, 1, 0, (void*)&pModbus->rdModbus.usCon4CurrentSOC, (void*)&pModbus->DataValue.nCon4CurrentSOC},
        {MID_Con5CurrentSOC,            POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x003C, 1, 1, 0, (void*)&pModbus->rdModbus.usCon5CurrentSOC, (void*)&pModbus->DataValue.nCon5CurrentSOC},
        {MID_Con6CurrentSOC,            POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x003D, 1, 1, 0, (void*)&pModbus->rdModbus.usCon6CurrentSOC, (void*)&pModbus->DataValue.nCon6CurrentSOC},

        // 枪4-6实时能量成本
        {MID_Con4RTEnergyCost,          POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x003E, 1, 0.01, 0, (void*)&pModbus->rdModbus.usCon4RTEnergyCost, (void*)&pModbus->DataValue.fCon4RTEnergyCost},
        {MID_Con5RTEnergyCost,          POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x003F, 1, 0.01, 0, (void*)&pModbus->rdModbus.usCon5RTEnergyCost, (void*)&pModbus->DataValue.fCon5RTEnergyCost},
        {MID_Con6RTEnergyCost,          POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x0040, 1, 0.01, 0, (void*)&pModbus->rdModbus.usCon6RTEnergyCost, (void*)&pModbus->DataValue.fCon6RTEnergyCost},

        // 枪4-6预计剩余时间
        {MID_Con4PredictRemaintime,     POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x0041, 1, 0.1, 0, (void*)&pModbus->rdModbus.usCon4PredictRemaintime, (void*)&pModbus->DataValue.fCon4PredictRemaintime},
        {MID_Con5PredictRemaintime,     POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x0042, 1, 0.1, 0, (void*)&pModbus->rdModbus.usCon5PredictRemaintime, (void*)&pModbus->DataValue.fCon5PredictRemaintime},
        {MID_Con6PredictRemaintime,     POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x0043, 1, 0.1, 0, (void*)&pModbus->rdModbus.usCon6PredictRemaintime, (void*)&pModbus->DataValue.fCon6PredictRemaintime},

        // 枪4-6插头状态
        {MID_Conn4PlugStatus,           POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0044, 1, 1, 0, (void*)&pModbus->rdModbus.usConn4PlugStatus, (void*)&pModbus->DataValue.nConn4PlugStatus},
        {MID_Conn5PlugStatus,           POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0045, 1, 1, 0, (void*)&pModbus->rdModbus.usConn5PlugStatus, (void*)&pModbus->DataValue.nConn5PlugStatus},
        {MID_Conn6PlugStatus,           POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0046, 1, 1, 0, (void*)&pModbus->rdModbus.usConn6PlugStatus, (void*)&pModbus->DataValue.nConn6PlugStatus},

        // 枪4-6充电状态
        {MID_Con4ChargingStatus,        POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0047, 1, 1, 0, (void*)&pModbus->rdModbus.usCon4ChargingStatus, (void*)&pModbus->DataValue.nCon4ChargingStatus},
        {MID_Con5ChargingStatus,        POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0048, 1, 1, 0, (void*)&pModbus->rdModbus.usCon5ChargingStatus, (void*)&pModbus->DataValue.nCon5ChargingStatus},
        {MID_Con6ChargingStatus,        POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x0049, 1, 1, 0, (void*)&pModbus->rdModbus.usCon6ChargingStatus, (void*)&pModbus->DataValue.nCon6ChargingStatus},

        // 枪对应电表能量
        {MID_Con1MeterTotalEnergy,      POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x004A, 2, 0.01, 0, (void*)&pModbus->rdModbus.usCon1MeterTotalEnergy, (void*)&pModbus->DataValue.fCon1MeterTotalEnergy},
        {MID_Con2MeterTotalEnergy,      POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x004C, 2, 0.01, 0, (void*)&pModbus->rdModbus.usCon2MeterTotalEnergy, (void*)&pModbus->DataValue.fCon2MeterTotalEnergy},
        {MID_Con3MeterTotalEnergy,      POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x004E, 2, 0.01, 0, (void*)&pModbus->rdModbus.usCon3MeterTotalEnergy, (void*)&pModbus->DataValue.fCon3MeterTotalEnergy},

        // 充电桩输入功率限制
        {MID_RdChgerInputPowerLimitStatic,  POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x0050, 1, 0.1, 0, (void*)&pModbus->rdModbus.usRdChgerInputPowerLimitStatic, (void*)&pModbus->DataValue.fRdChgerInputPowerLimitStatic},
        {MID_RdChgerInputPowerLimitDynamic, POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x0051, 1, 0.1, 0, (void*)&pModbus->rdModbus.usRdChgerInputPowerLimitDynamic, (void*)&pModbus->DataValue.fRdChgerInputPowerLimitDynamic},

        // 2.2.2 Charging Data (Dev Info)
        {MID_SerialNumber,              POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_STRING, DATA_VALUE_STRING, 0x1001, 13, 1, 0, (void*)&pModbus->rdModbus.szSerialNumber, (void*)&pModbus->DataValue.szSerialNumber},
        {MID_InputPowerlimit,           POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x100E, 1, 0.1, 0, (void*)&pModbus->rdModbus.usInputPowerlimit, (void*)&pModbus->DataValue.fInputPowerlimit},
        {MID_EV_RTTotalPower,           POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x100F, 1, 0.1, 0, (void*)&pModbus->rdModbus.usEV_RTTotalPower, (void*)&pModbus->DataValue.fEV_RTTotalPower},

        // 2.2.3 ESS Data
        {MID_MaxDischgAbility2ACload,   POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x1101, 1, 0.1, 0, (void*)&pModbus->rdModbus.usMaxDischgAbility2ACload, (void*)&pModbus->DataValue.fMaxDischgAbility2ACload},
        {MID_RTPowerDischg4ACload,      POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x1102, 1, 0.1, 0, (void*)&pModbus->rdModbus.usRTPowerDischg4ACload, (void*)&pModbus->DataValue.fRTPowerDischg4ACload},

        // 2.3 ES Battery Data - Total Battery Groups
        {MID_TotalBattGroupsNum,        POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x2001, 1, 1, 0, (void*)&pModbus->rdModbus.usTotalBattGroupsNum, (void*)&pModbus->DataValue.nTotalBattGroupsNum},
        // 1# Battery Information (0x2002 ~ 0x200E)
        {MID_Batt1_RunStatus,           POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x2002, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt1_RunStatus, (void*)&pModbus->DataValue.nBatt1_RunStatus},
        {MID_Batt1_Voltage,             POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2003, 1, 0.1, 0, (void*)&pModbus->rdModbus.usBatt1_Voltage, (void*)&pModbus->DataValue.fBatt1_Voltage},
        {MID_Batt1_Current,             POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2004, 1, 0.1, 0, (void*)&pModbus->rdModbus.usBatt1_Current, (void*)&pModbus->DataValue.fBatt1_Current},
        {MID_Batt1_SOH,                 POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x2005, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt1_SOH, (void*)&pModbus->DataValue.nBatt1_SOH},
        {MID_Batt1_SOC,                 POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x2006, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt1_SOC, (void*)&pModbus->DataValue.nBatt1_SOC},
        {MID_Batt1_MaxCellVoltage,      POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2007, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt1_MaxCellVoltage, (void*)&pModbus->DataValue.fBatt1_MaxCellVoltage},
        {MID_Batt1_MinCellVoltage,      POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2008, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt1_MinCellVoltage, (void*)&pModbus->DataValue.fBatt1_MinCellVoltage},
        {MID_Batt1_AvgCellVoltage,      POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2009, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt1_AvgCellVoltage, (void*)&pModbus->DataValue.fBatt1_AvgCellVoltage},
        {MID_Batt1_MaxCellTemp,         POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x200A, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt1_MaxCellTemp, (void*)&pModbus->DataValue.fBatt1_MaxCellTemp},
        {MID_Batt1_MinCellTemp,         POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x200B, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt1_MinCellTemp, (void*)&pModbus->DataValue.fBatt1_MinCellTemp},
        {MID_Batt1_AvgCellTemp,         POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x200C, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt1_AvgCellTemp, (void*)&pModbus->DataValue.fBatt1_AvgCellTemp},
        {MID_Batt1_ChgDischgEnable,     POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x200D, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt1_ChgDischgEnable, (void*)&pModbus->DataValue.nBatt1_ChgDischgEnable},
        {MID_Batt1_AlarmStatus,         POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x200E, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt1_AlarmStatus, (void*)&pModbus->DataValue.nBatt1_AlarmStatus},

        // 2# Battery Information (0x200F ~ 0x201B)
        {MID_Batt2_RunStatus,           POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x200F, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt2_RunStatus, (void*)&pModbus->DataValue.nBatt2_RunStatus},
        {MID_Batt2_Voltage,             POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2010, 1, 0.1, 0, (void*)&pModbus->rdModbus.usBatt2_Voltage, (void*)&pModbus->DataValue.fBatt2_Voltage},
        {MID_Batt2_Current,             POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2011, 1, 0.1, 0, (void*)&pModbus->rdModbus.usBatt2_Current, (void*)&pModbus->DataValue.fBatt2_Current},
        {MID_Batt2_SOH,                 POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x2012, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt2_SOH, (void*)&pModbus->DataValue.nBatt2_SOH},
        {MID_Batt2_SOC,                 POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x2013, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt2_SOC, (void*)&pModbus->DataValue.nBatt2_SOC},
        {MID_Batt2_MaxCellVoltage,      POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2014, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt2_MaxCellVoltage, (void*)&pModbus->DataValue.fBatt2_MaxCellVoltage},
        {MID_Batt2_MinCellVoltage,      POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2015, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt2_MinCellVoltage, (void*)&pModbus->DataValue.fBatt2_MinCellVoltage},
        {MID_Batt2_AvgCellVoltage,      POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2016, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt2_AvgCellVoltage, (void*)&pModbus->DataValue.fBatt2_AvgCellVoltage},
        {MID_Batt2_MaxCellTemp,         POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2017, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt2_MaxCellTemp, (void*)&pModbus->DataValue.fBatt2_MaxCellTemp},
        {MID_Batt2_MinCellTemp,         POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2018, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt2_MinCellTemp, (void*)&pModbus->DataValue.fBatt2_MinCellTemp},
        {MID_Batt2_AvgCellTemp,         POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2019, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt2_AvgCellTemp, (void*)&pModbus->DataValue.fBatt2_AvgCellTemp},
        {MID_Batt2_ChgDischgEnable,     POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x201A, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt2_ChgDischgEnable, (void*)&pModbus->DataValue.nBatt2_ChgDischgEnable},
        {MID_Batt2_AlarmStatus,         POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x201B, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt2_AlarmStatus, (void*)&pModbus->DataValue.nBatt2_AlarmStatus},

        // 3# Battery Information (0x201C ~ 0x2028)
        {MID_Batt3_RunStatus,           POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x201C, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt3_RunStatus, (void*)&pModbus->DataValue.nBatt3_RunStatus},
        {MID_Batt3_Voltage,             POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x201D, 1, 0.1, 0, (void*)&pModbus->rdModbus.usBatt3_Voltage, (void*)&pModbus->DataValue.fBatt3_Voltage},
        {MID_Batt3_Current,             POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x201E, 1, 0.1, 0, (void*)&pModbus->rdModbus.usBatt3_Current, (void*)&pModbus->DataValue.fBatt3_Current},
        {MID_Batt3_SOH,                 POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x201F, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt3_SOH, (void*)&pModbus->DataValue.nBatt3_SOH},
        {MID_Batt3_SOC,                 POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x2020, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt3_SOC, (void*)&pModbus->DataValue.nBatt3_SOC},
        {MID_Batt3_MaxCellVoltage,      POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2021, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt3_MaxCellVoltage, (void*)&pModbus->DataValue.fBatt3_MaxCellVoltage},
        {MID_Batt3_MinCellVoltage,      POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2022, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt3_MinCellVoltage, (void*)&pModbus->DataValue.fBatt3_MinCellVoltage},
        {MID_Batt3_AvgCellVoltage,      POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2023, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt3_AvgCellVoltage, (void*)&pModbus->DataValue.fBatt3_AvgCellVoltage},
        {MID_Batt3_MaxCellTemp,         POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2024, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt3_MaxCellTemp, (void*)&pModbus->DataValue.fBatt3_MaxCellTemp},
        {MID_Batt3_MinCellTemp,         POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2025, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt3_MinCellTemp, (void*)&pModbus->DataValue.fBatt3_MinCellTemp},
        {MID_Batt3_AvgCellTemp,         POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2026, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt3_AvgCellTemp, (void*)&pModbus->DataValue.fBatt3_AvgCellTemp},
        {MID_Batt3_ChgDischgEnable,     POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x2027, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt3_ChgDischgEnable, (void*)&pModbus->DataValue.nBatt3_ChgDischgEnable},
        {MID_Batt3_AlarmStatus,         POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x2028, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt3_AlarmStatus, (void*)&pModbus->DataValue.nBatt3_AlarmStatus},

        // 4# Battery Information (0x2029 ~ 0x2035)
        {MID_Batt4_RunStatus,           POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x2029, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt4_RunStatus, (void*)&pModbus->DataValue.nBatt4_RunStatus},
        {MID_Batt4_Voltage,             POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x202A, 1, 0.1, 0, (void*)&pModbus->rdModbus.usBatt4_Voltage, (void*)&pModbus->DataValue.fBatt4_Voltage},
        {MID_Batt4_Current,             POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x202B, 1, 0.1, 0, (void*)&pModbus->rdModbus.usBatt4_Current, (void*)&pModbus->DataValue.fBatt4_Current},
        {MID_Batt4_SOH,                 POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x202C, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt4_SOH, (void*)&pModbus->DataValue.nBatt4_SOH},
        {MID_Batt4_SOC,                 POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x202D, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt4_SOC, (void*)&pModbus->DataValue.nBatt4_SOC},
        {MID_Batt4_MaxCellVoltage,      POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x202E, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt4_MaxCellVoltage, (void*)&pModbus->DataValue.fBatt4_MaxCellVoltage},
        {MID_Batt4_MinCellVoltage,      POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x202F, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt4_MinCellVoltage, (void*)&pModbus->DataValue.fBatt4_MinCellVoltage},
        {MID_Batt4_AvgCellVoltage,      POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2030, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt4_AvgCellVoltage, (void*)&pModbus->DataValue.fBatt4_AvgCellVoltage},
        {MID_Batt4_MaxCellTemp,         POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2031, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt4_MaxCellTemp, (void*)&pModbus->DataValue.fBatt4_MaxCellTemp},
        {MID_Batt4_MinCellTemp,         POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2032, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt4_MinCellTemp, (void*)&pModbus->DataValue.fBatt4_MinCellTemp},
        {MID_Batt4_AvgCellTemp,         POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_FLOAT, 0x2033, 1, 0.01, 0, (void*)&pModbus->rdModbus.usBatt4_AvgCellTemp, (void*)&pModbus->DataValue.fBatt4_AvgCellTemp},
        {MID_Batt4_ChgDischgEnable,     POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x2034, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt4_ChgDischgEnable, (void*)&pModbus->DataValue.nBatt4_ChgDischgEnable},
        {MID_Batt4_AlarmStatus,         POINT_TYPE_INPUT_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0x2035, 1, 1, 0, (void*)&pModbus->rdModbus.usBatt4_AlarmStatus, (void*)&pModbus->DataValue.nBatt4_AlarmStatus},

        // 2.4.1.Remote Start/Stop Charging (idTag not provided)
        {MID_ChgCtrl_Connector1,        POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xA000, 1, 1, 0, (void*)&pModbus->rdModbus.usChgCtrl_Connector1, (void*)&pModbus->DataValue.nChgCtrl_Connector1},
        {MID_ChgCtrl_Connector2,        POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xA001, 1, 1, 0, (void*)&pModbus->rdModbus.usChgCtrl_Connector2, (void*)&pModbus->DataValue.nChgCtrl_Connector2},
        {MID_ChgCtrl_Connector3,        POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xA002, 1, 1, 0, (void*)&pModbus->rdModbus.usChgCtrl_Connector3, (void*)&pModbus->DataValue.nChgCtrl_Connector3},
        {MID_ChgCtrl_Connector4,        POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xA003, 1, 1, 0, (void*)&pModbus->rdModbus.usChgCtrl_Connector4, (void*)&pModbus->DataValue.nChgCtrl_Connector4},
        {MID_ChgCtrl_Connector5,        POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xA004, 1, 1, 0, (void*)&pModbus->rdModbus.usChgCtrl_Connector5, (void*)&pModbus->DataValue.nChgCtrl_Connector5},
        {MID_ChgCtrl_Connector6,        POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xA005, 1, 1, 0, (void*)&pModbus->rdModbus.usChgCtrl_Connector6, (void*)&pModbus->DataValue.nChgCtrl_Connector6},

        // Connector 1
        {MID_StartChg_Conn1_IdTag,      POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_STRING, DATA_VALUE_STRING, 0xB000, 16, 1, 0, (void*)&pModbus->rdModbus.szStartChg_Conn1_IdTag, (void*)&pModbus->DataValue.szStartChg_Conn1_IdTag},
        {MID_StopChg_Conn1_IdTag,       POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_STRING, DATA_VALUE_STRING, 0xB010, 16, 1, 0, (void*)&pModbus->rdModbus.szStopChg_Conn1_IdTag, (void*)&pModbus->DataValue.szStopChg_Conn1_IdTag},
        {MID_StartChg_Conn2_IdTag,      POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_STRING, DATA_VALUE_STRING, 0xB020, 16, 1, 0, (void*)&pModbus->rdModbus.szStartChg_Conn2_IdTag, (void*)&pModbus->DataValue.szStartChg_Conn2_IdTag},
        {MID_StopChg_Conn2_IdTag,       POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_STRING, DATA_VALUE_STRING, 0xB030, 16, 1, 0, (void*)&pModbus->rdModbus.szStopChg_Conn2_IdTag, (void*)&pModbus->DataValue.szStopChg_Conn2_IdTag},
        {MID_StartChg_Conn3_IdTag,      POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_STRING, DATA_VALUE_STRING, 0xB040, 16, 1, 0, (void*)&pModbus->rdModbus.szStartChg_Conn3_IdTag, (void*)&pModbus->DataValue.szStartChg_Conn3_IdTag},
        {MID_StopChg_Conn3_IdTag,       POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_STRING, DATA_VALUE_STRING, 0xB050, 16, 1, 0, (void*)&pModbus->rdModbus.szStopChg_Conn3_IdTag, (void*)&pModbus->DataValue.szStopChg_Conn3_IdTag},

        // System Parameter Ctrl
        {MID_ChgerInputPowerLimit_Static,   POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC000, 1, 1, 0, (void*)&pModbus->rdModbus.usChgerInputPowerLimit_Static, (void*)&pModbus->DataValue.nChgerInputPowerLimit_Static},
        {MID_ChargerSwitch,                 POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC001, 1, 1, 0, (void*)&pModbus->rdModbus.usChargerSwitch, (void*)&pModbus->DataValue.nChargerSwitch},
        {MID_Conn1_MaxOutputCurrent,        POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC002, 1, 1, 0, (void*)&pModbus->rdModbus.usConn1_MaxOutputCurrent, (void*)&pModbus->DataValue.nConn1_MaxOutputCurrent},
        {MID_Conn2_MaxOutputCurrent,        POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC003, 1, 1, 0, (void*)&pModbus->rdModbus.usConn2_MaxOutputCurrent, (void*)&pModbus->DataValue.nConn2_MaxOutputCurrent},
        {MID_Conn3_MaxOutputCurrent,        POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC004, 1, 1, 0, (void*)&pModbus->rdModbus.usConn3_MaxOutputCurrent, (void*)&pModbus->DataValue.nConn3_MaxOutputCurrent},
        {MID_Conn1_MaxOutputPower,          POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC005, 1, 1, 0, (void*)&pModbus->rdModbus.usConn1_MaxOutputPower, (void*)&pModbus->DataValue.nConn1_MaxOutputPower},
        {MID_Conn2_MaxOutputPower,          POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC006, 1, 1, 0, (void*)&pModbus->rdModbus.usConn2_MaxOutputPower, (void*)&pModbus->DataValue.nConn2_MaxOutputPower},
        {MID_Conn3_MaxOutputPower,          POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC007, 1, 1, 0, (void*)&pModbus->rdModbus.usConn3_MaxOutputPower, (void*)&pModbus->DataValue.nConn3_MaxOutputPower},
        {MID_ChgerInputPowerLimit_Dynamic,  POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC008, 1, 1, 0, (void*)&pModbus->rdModbus.usChgerInputPowerLimit_Dynamic, (void*)&pModbus->DataValue.nChgerInputPowerLimit_Dynamic},
        {MID_Conn4_MaxOutputCurrent,        POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC009, 1, 1, 0, (void*)&pModbus->rdModbus.usConn4_MaxOutputCurrent, (void*)&pModbus->DataValue.nConn4_MaxOutputCurrent},
        {MID_Conn5_MaxOutputCurrent,        POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC00A, 1, 1, 0, (void*)&pModbus->rdModbus.usConn5_MaxOutputCurrent, (void*)&pModbus->DataValue.nConn5_MaxOutputCurrent},
        {MID_Conn6_MaxOutputCurrent,        POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC00B, 1, 1, 0, (void*)&pModbus->rdModbus.usConn6_MaxOutputCurrent, (void*)&pModbus->DataValue.nConn6_MaxOutputCurrent},
        {MID_Conn4_MaxOutputPower,          POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC00C, 1, 1, 0, (void*)&pModbus->rdModbus.usConn4_MaxOutputPower, (void*)&pModbus->DataValue.nConn4_MaxOutputPower},
        {MID_Conn5_MaxOutputPower,          POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC00D, 1, 1, 0, (void*)&pModbus->rdModbus.usConn5_MaxOutputPower, (void*)&pModbus->DataValue.nConn5_MaxOutputPower},
        {MID_Conn6_MaxOutputPower,          POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC00E, 1, 1, 0, (void*)&pModbus->rdModbus.usConn6_MaxOutputPower, (void*)&pModbus->DataValue.nConn6_MaxOutputPower},
        {MID_ACLoadDemandPower,             POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC00F, 1, 1, 0, (void*)&pModbus->rdModbus.usACLoadDemandPower, (void*)&pModbus->DataValue.nACLoadDemandPower},
        {MID_OfflineSafeMode_Enable,        POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC010, 1, 1, 0, (void*)&pModbus->rdModbus.usOfflineSafeMode_Enable, (void*)&pModbus->DataValue.nOfflineSafeMode_Enable},
        {MID_OfflineSafeMode_Timeout,       POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC011, 1, 1, 0, (void*)&pModbus->rdModbus.usOfflineSafeMode_Timeout, (void*)&pModbus->DataValue.nOfflineSafeMode_Timeout},
        {MID_OfflineSafeMode_MaxPowerLimit, POINT_TYPE_HOLDING_REG, 0, 0, 0, TRANS_TYPE_UINT16, DATA_VALUE_UINT, 0xC012, 1, 1, 0, (void*)&pModbus->rdModbus.usOfflineSafeMode_MaxPowerLimit, (void*)&pModbus->DataValue.nOfflineSafeMode_MaxPowerLimit},
    };

    pModbus->ModbusDataMap = NEW(MODBUS_DATA_MAP,ITEM_OF(ChargerModbusDataItem));
	if(pModbus->ModbusDataMap == NULL)
	{
        printf("InitModbusDataMap malloc failed!\n");
		return FALSE;
	}
    memcpy(pModbus->ModbusDataMap, ChargerModbusDataItem, sizeof(ChargerModbusDataItem));
    pModbus->nModbusDataMapNum = ITEM_OF(ChargerModbusDataItem);
    printf("Modbus Data Map Num: %d\n", pModbus->nModbusDataMapNum);
}

BOOL ChargerModbusTCPInitialize(ROUGH_CHARGER_DATA *pModbusRough)
{
    printf("Charger[ModbusTCP][Initialize]\n");

    strcpy(pModbusRough->DevInfo.szIp, "192.168.100.101");
    pModbusRough->DevInfo.nPort = 502;
    pModbusRough->DevInfo.nSlaveId = 1;
    pModbusRough->DevInfo.nRspTimeout = 5;    // 3s
    pModbusRough->DevInfo.nByteTimeout = 2;   // 2s

    pModbusRough->ctx = modbus_new_tcp(pModbusRough->DevInfo.szIp, pModbusRough->DevInfo.nPort);
    modbus_set_debug(pModbusRough->ctx, TRUE);
    // modbus_set_debug(pModbusRough->ctx, FALSE);
    if (pModbusRough->ctx == NULL) 
    {
        printf("Unable to allocate libmodbus context\n");
        return FALSE;
    }
    if (modbus_connect(pModbusRough->ctx) == -1) {
        printf("Connection failed: %s\n", modbus_strerror(errno));
        modbus_close(pModbusRough->ctx);
        modbus_free(pModbusRough->ctx);
        pModbusRough->ctx = NULL;
        return -1;
    }

    // 设置从机地址（设备号）
    // slave_id 范围通常是 0-247，具体看设备文档
    int rc = modbus_set_slave(pModbusRough->ctx, pModbusRough->DevInfo.nSlaveId);
    if (rc == -1) 
    {
        fprintf(stderr, "Set slave address failed: %s\n", modbus_strerror(errno));
        modbus_close(pModbusRough->ctx);
        modbus_free(pModbusRough->ctx);
        pModbusRough->ctx = NULL;
        return FALSE;
    }

    // 设置错误恢复模式
    modbus_set_error_recovery(pModbusRough->ctx, MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL);

    // 设置 500 毫秒响应超时
    modbus_set_response_timeout(pModbusRough->ctx, pModbusRough->DevInfo.nRspTimeout, 0);

    // 设置 200 毫秒字节超时
    modbus_set_byte_timeout(pModbusRough->ctx, pModbusRough->DevInfo.nByteTimeout, 0);

    // 初始化读取映射
    memset(pModbusRough->nReadRegNum, 0, sizeof(pModbusRough->nReadRegNum));
    int tempMapReadStartReg[MAX_READ_SEG] = {0x0001, 0x1001, 0x1101, 0x2001, 0xA000, 0xB000, 0xC000, 0/*pass*/};
    memcpy(pModbusRough->MapReadStartReg, tempMapReadStartReg, sizeof(tempMapReadStartReg));
    int i;
    for (i = 0; i < MAX_READ_SEG; i++) 
    {
        pModbusRough->tab_rp_registers[i] = (uint16_t *)malloc(255 * sizeof(uint16_t));
        if (pModbusRough->tab_rp_registers[i] == NULL) 
        {
            // 处理内存分配失败,释放已分配的内存
            int j;
            for (j = 0; j < i; ++j)
            {
                free(pModbusRough->tab_rp_registers[j]);
            }
            return FALSE;
        }
        memset(pModbusRough->tab_rp_registers[i], 0, 255 * sizeof(uint16_t));
    }

    // 分段统计需要读取的寄存器数量
    CountRegNum(pModbusRough);

    uint32_t to_sec, to_usec;
    // 获取响应超时
    modbus_get_response_timeout(pModbusRough->ctx, &to_sec, &to_usec);
    // printf("响应超时: %u.%06u 秒\n", to_sec, to_usec);
    // 获取字节超时
    modbus_get_byte_timeout(pModbusRough->ctx, &to_sec, &to_usec);
    // printf("字节超时: %u.%06u 秒\n", to_sec, to_usec);

    return TRUE;
}

// 人为划分读取的段，统计每个分段需要读取的寄存器数量
int CountRegNum(ROUGH_CHARGER_DATA  *pModbusRough)
{
    int i;
    // 2.2.1.Charging Data 
    for (i = MID_SysAvaStu; i <= MID_RdChgerInputPowerLimitDynamic; ++i)
    {
        pModbusRough->nReadRegNum[0] += pModbusRough->ModbusDataMap[i].nRegNum;
    }
    pModbusRough->nReadItemNum[0] = i - MID_SysAvaStu;

    // 2.2.2 Charging Data (Dev Info)
    for (i = MID_SerialNumber; i <= MID_EV_RTTotalPower; ++i)
    {
        pModbusRough->nReadRegNum[1] += pModbusRough->ModbusDataMap[i].nRegNum;
    }
    pModbusRough->nReadItemNum[1] = i - MID_SerialNumber;


    // 2.2.3 ESS Data
    for (i = MID_MaxDischgAbility2ACload; i <= MID_RTPowerDischg4ACload; ++i)
    {
        pModbusRough->nReadRegNum[2] += pModbusRough->ModbusDataMap[i].nRegNum;
    }
    pModbusRough->nReadItemNum[2] = i - MID_MaxDischgAbility2ACload;

    // 2.3 ES Battery Data - Total Battery Groups
    for (i = MID_TotalBattGroupsNum; i <= MID_Batt4_AlarmStatus; ++i)
    {
        pModbusRough->nReadRegNum[3] += pModbusRough->ModbusDataMap[i].nRegNum;
    }
    pModbusRough->nReadItemNum[3] = i - MID_TotalBattGroupsNum;

    // 2.4.1.Remote Start/Stop Charging (idTag not provided)
    for (i = MID_ChgCtrl_Connector1; i <= MID_ChgCtrl_Connector6; ++i)
    {
        pModbusRough->nReadRegNum[4] += pModbusRough->ModbusDataMap[i].nRegNum;
    }
    pModbusRough->nReadItemNum[4] = i - MID_ChgCtrl_Connector1;

    // 2.4.2.Remote Start/ Stop Charging (idTag provided)
    for (i = MID_StartChg_Conn1_IdTag; i <= MID_StopChg_Conn3_IdTag; ++i)
    {
        pModbusRough->nReadRegNum[5] += pModbusRough->ModbusDataMap[i].nRegNum;
    }
    pModbusRough->nReadItemNum[5] = i - MID_StartChg_Conn1_IdTag;

    // 2.4.3.System Parameter Ctrl
    for (i = MID_ChgerInputPowerLimit_Static; i <= MID_OfflineSafeMode_MaxPowerLimit; ++i)
    {
        pModbusRough->nReadRegNum[6] += pModbusRough->ModbusDataMap[i].nRegNum;
    }
    pModbusRough->nReadItemNum[6] = i - MID_ChgerInputPowerLimit_Static;

    // 2.5 Energy Management Plan Setting(pass)

    return 0;
}

// 创建 modbus context
BOOL CtxIsReady(ROUGH_CHARGER_DATA  *pModbusRough)
{
    printf("ModbusRough->ctx == NULL = [%d]\n", pModbusRough->ctx == NULL);
    if (pModbusRough->ctx == NULL)
    {
        printf("modbus context is NULL\n");
        // 创建 modbus context
        int i = 0;
        for (i = 0; i < 3; ++i)
        {
            pModbusRough->ctx = modbus_new_tcp(pModbusRough->DevInfo.szIp, pModbusRough->DevInfo.nPort);
            if (pModbusRough->ctx != NULL)
            {
                if (modbus_connect(pModbusRough->ctx) == 0)
                {
                    break;
                }
                else
                {
                    printf("Connection failed: %s\n", modbus_strerror(errno));
                    modbus_close(pModbusRough->ctx);
                    modbus_free(pModbusRough->ctx);
                    pModbusRough->ctx = NULL;
                    // sleep(3);
                    continue;
                }
            }
        }
        if (pModbusRough->ctx == NULL)
        {
            printf("modbus_new_tcp retry [%d] times failed\n", i);
            return FALSE;
        }
    }
    else
    {
        return TRUE;
    }
}

// 根据上面的统计结果，分段读取数据
static void SegmentReadData(ROUGH_CHARGER_DATA  *pModbusRough)
{
    int rc;
    int i;

    // memset(pModbusRough->tab_rp_registers, 0, sizeof(pModbusRough->tab_rp_registers));

    for (i = 0; i < MAX_READ_SEG; ++i)
    {
        if (pModbusRough->nReadRegNum[i] == 0)
        {
            printf("skip segment [%d]", i);
            continue;
        }
        // printf("nReadRegNum[%d] = %d\n", i, pModbusRough->nReadRegNum[i]);

        rc = modbus_read_registers(pModbusRough->ctx, pModbusRough->MapReadStartReg[i], 
            pModbusRough->nReadRegNum[i], pModbusRough->tab_rp_registers[i]);
        printf("rc[%d] = %d\n", i, rc);
        if (rc == -1)
        {
            printf("Reading registers segment[%d] failed: %s\n", i, modbus_strerror(errno));
            modbus_close(pModbusRough->ctx);
            modbus_free(pModbusRough->ctx);
            pModbusRough->ctx = NULL;
            return ERR_SAMPLE_FAILED;
        }
        else
        {
            // if (i == 6)
            // {
            //     printf("Reading registers succeed:\n");
            //     // 打印读取到的寄存器值
            //     printf("modbus_read_registers: ");
            //     int j;
            //     for (j = 0; j < rc; j++)
            //     {
            //         printf("0x%04x ", pModbusRough->tab_rp_registers[i][j]);
            //     }
            //     printf("\n");
            // }
        }
    }
}

// 解析读取到的原始数据，存放到对应的数据结构中
static void ParseModbusData(ROUGH_CHARGER_DATA  *pModbusRough)
{
    int i;
    for (i = 0; i < MAX_READ_SEG; ++i)
    {
        if (pModbusRough->nReadRegNum[i] == 0)
        {
            continue;
        }
        // printf("Parsing Read Map Index[%d]: 0x%04x\n", i, pModbusRough->MapReadStartReg[i]);
        int nDataIdStart = 0;
        int j;
        for (j = 0; j < i; ++j)
        {
            nDataIdStart += pModbusRough->nReadItemNum[j];
        }
        // printf("nDataIdStart: [%d]\n", nDataIdStart);
        // 从分段 DataId 起始的地方开始解析数据
        for (j = 0; j < pModbusRough->nReadItemNum[i]; ++j)
        {
            MODBUS_DATA_MAP* DataItem = &pModbusRough->ModbusDataMap[nDataIdStart + j];
            // printf("DataItem->nRegAddrStart = [%04x]\n", DataItem->nRegAddrStart);
            switch (DataItem->emDataType)
            {
                case TRANS_TYPE_UINT16:
                {
                    *(uint16_t*)DataItem->ptrValue = pModbusRough->tab_rp_registers[i][j];
                    break;
                }
                case TRANS_TYPE_INT16:
                {
                    *(int16_t*)DataItem->ptrValue = (int16_t)pModbusRough->tab_rp_registers[i][j];
                    break;
                }
                case TRANS_TYPE_STRING:
                {
                    // 假设字符串长度为 nRegNum 个寄存器，每个寄存器 2 字节
                    int strLen = DataItem->nRegNum * 2;
                    char* strPtr = (char*)DataItem->ptrValue;
                    int k;
                    for (k = 0; k < DataItem->nRegNum; ++k)
                    {
                        strPtr[k * 2] = (pModbusRough->tab_rp_registers[i][j + k] >> 8) & 0xFF;
                        strPtr[k * 2 + 1] = pModbusRough->tab_rp_registers[i][j + k] & 0xFF;
                    }
                    strPtr[strLen] = '\0'; // 确保字符串以 null 结尾
                    // printf("Parsed String: %s\n", strPtr);
                    break;
                }
                default:
                    printf("Unsupported data type!\n");
                    break;
            }
        }
    }
    // 打印解析后的数据以验证
    printf("Parsed Data:\n");
    printf("System Available Status: [%u]\n", pModbusRough->rdModbus.usSysAvaStu);
    printf("Connector1AvaStu: [%u]\n", pModbusRough->rdModbus.usConnector1AvaStu);
    printf("Connector2AvaStu: [%u]\n", pModbusRough->rdModbus.usConnector2AvaStu);
    printf("Connector3AvaStu: [%u]\n", pModbusRough->rdModbus.usConnector3AvaStu);
    printf("sCon1_L1_N_VolExport: [%d]\n", pModbusRough->rdModbus.sCon1_L1_N_VolExport);
    printf("sCon1_L2_N_VolExport: [%d]\n", pModbusRough->rdModbus.sCon1_L2_N_VolExport);
    printf("sCon1_L3_N_VolExport: [%d]\n", pModbusRough->rdModbus.sCon1_L3_N_VolExport);
    printf("Serial Number: [%s]\n", pModbusRough->rdModbus.szSerialNumber);
}

// 根据比例和偏移信息，将原始数据转换成可用的真实值
static BOOL ConvertData(ROUGH_CHARGER_DATA  *pModbusRough)
{
    printf("Converting Data...\n");
    int i;
    for (i = 0; i <= MID_Batt4_AlarmStatus; ++i)
    {
        MODBUS_DATA_MAP* DataItem = &pModbusRough->ModbusDataMap[i];
        if (DataItem == NULL)
        {
            printf("DataItem[%d] is NULL\n", i);
            continue;
        }
        if (DataItem->ptrValue == NULL)
        {
            printf("DataItem[%d]->ptrValue is NULL\n", i);
            continue;
        }
        // printf("DataItem[%d]->emValueType: [%d]\n", i, DataItem->emValueType);
        switch (DataItem->emValueType)
        {
        case DATA_VALUE_INT:
            {
                *(int*)DataItem->ptrValueReal = (int)(*(int16_t*)DataItem->ptrValue * DataItem->fScale + DataItem->fOffset);
                // printf("[INT] Converted Value: %d\n", *(int*)DataItem->ptrValueReal);
            }
            break;
        case DATA_VALUE_UINT:
            {
                *(unsigned int*)DataItem->ptrValueReal = (unsigned int)(*(uint16_t*)DataItem->ptrValue * DataItem->fScale + DataItem->fOffset);
                // printf("[UINT] Converted Value: %u\n", *(unsigned int*)DataItem->ptrValueReal);
            }
            break;
        case DATA_VALUE_FLOAT:
            {
                if (DataItem->emDataType == TRANS_TYPE_INT16)
                {
                    *(float*)DataItem->ptrValueReal = (float)(*(int16_t*)DataItem->ptrValue * DataItem->fScale + DataItem->fOffset);
                }
                else if (DataItem->emDataType == TRANS_TYPE_UINT16)
                {
                    *(float*)DataItem->ptrValueReal = (float)(*(uint16_t*)DataItem->ptrValue * DataItem->fScale + DataItem->fOffset);
                }
                else
                {
                    printf("Unsupported data type![DATA_VALUE_FLOAT]\n");
                }
                // printf("[FLOAT] Converted Value: %f\n", *(float*)DataItem->ptrValueReal);
            }
            break;
        case DATA_VALUE_STRING:
            {
                // 复制固定长度的数据块
                memcpy(DataItem->ptrValueReal, 
                    DataItem->ptrValue, 
                    DataItem->nRegNum * 2);
            }
            break;
        default:
            printf("Unsupported data type![DATA_VALUE_UNKNOWN]\n");
            break;
        }
    }

    

    return TRUE;
}

int ChargerModbusTCPSample(SAMPLER* pSampler)
{
    // printf("Charger[ModbusTCP][Sample]\n");
    ROUGH_CHARGER_DATA  *pModbusRough = (ROUGH_CHARGER_DATA*)pSampler->pDataHeap;
    
    // 包含任何因为连接断开而需要重新连接的逻辑
    if (CtxIsReady(pModbusRough) != TRUE)
    {
        printf("Charger ModbusTCP disconnected\n");
        return ERR_SAMPLE_FAILED;
    }
    
    // 分段读取寄存器
    SegmentReadData(pModbusRough);

    // 通过 Map 解析原始数据到内存变量（原始协议值）
    ParseModbusData(pModbusRough);

    // 根据 Map 将原始数据转换成可用的真实值
    ConvertData(pModbusRough);

    // 打印最终转换的数据以验证
    printf("Data Values:\n");
    printf("System Available Status: [%d]\n", pModbusRough->DataValue.nSysAvaStu);
    printf("Connector1AvaStu: [%d]\n", pModbusRough->DataValue.nConnector1AvaStu);
    printf("Connector2AvaStu: [%d]\n", pModbusRough->DataValue.nConnector2AvaStu);
    printf("Connector3AvaStu: [%d]\n", pModbusRough->DataValue.nConnector3AvaStu);
    printf("sCon1_L1_N_VolExport: [%f]\n", pModbusRough->DataValue.fCon1_L1_N_VolExport);
    printf("sCon1_L2_N_VolExport: [%f]\n", pModbusRough->DataValue.fCon1_L2_N_VolExport);
    printf("sCon1_L3_N_VolExport: [%f]\n", pModbusRough->DataValue.fCon1_L3_N_VolExport);
    if (pModbusRough->DataValue.szSerialNumber != NULL)
        printf("Serial Number: [%s]\n", pModbusRough->DataValue.szSerialNumber);

    printf("ChgerInputPowerLimitStatic: [%f]\n", pModbusRough->DataValue.fRdChgerInputPowerLimitStatic);
    printf("ChgerInputPowerLimitDynamic: [%f]\n", pModbusRough->DataValue.fRdChgerInputPowerLimitDynamic);

    return ERR_SAMPLE_OK;
}