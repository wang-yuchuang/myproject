/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**---------------------------------------File Information------------------------------------------
** @File Name    : aircon_sampler_main.c
** @Founder      : NJX01203
** @Date         : 2026.07.02
** @Function     : Periodic Modbus RTU sampler for liquid-cooling AC (EU3 + TF variants)
** @Instructions : 参考 0701 备份重写，符合 auxdev_sampler 架构。
**                 绑定 Sampler_ID=11, CommPort_ID=13 (RS485_AIRCON, ttyS9), Device 951 (DevModel 950)。
**                 使用 pv_modbus_rtu.c 进行 Modbus RTU 通信（无 libmodbus 依赖）。
**                 TF params: 1770-1810, EU3 params: 1521-1570 (per System.sql)。
**                 不包含 AC 自动热控（paramId 2110/2401-2406 被 PCS/LVRT 占用）。
**===============================================================================================*/

#include "aircon_sampler_main.h"
#include <stdio.h>
#include <string.h>

/*=================================================================================================
** @Name      : pushParamFloat / pushParamInt / pushParamEnum
** @Function  : Push_VarValue 的类型便捷封装
**===============================================================================================*/
static void pushParamFloat(HANDLE hSampler, int nDevId, int nParamId, float fVal)
{
    VAR_VALUE varV;
    memset(&varV, 0, sizeof(varV));
    varV.fValue = fVal;
    Push_VarValue(SETTYPE_RAW_SP, hSampler, nDevId, nParamId, varV);
}

static void pushParamInt(HANDLE hSampler, int nDevId, int nParamId, long lVal)
{
    VAR_VALUE varV;
    memset(&varV, 0, sizeof(varV));
    varV.lValue = lVal;
    Push_VarValue(SETTYPE_RAW_SP, hSampler, nDevId, nParamId, varV);
}

static void pushParamEnum(HANDLE hSampler, int nDevId, int nParamId, long emVal)
{
    VAR_VALUE varV;
    memset(&varV, 0, sizeof(varV));
    varV.emValue = (_ENUM)emVal;
    Push_VarValue(SETTYPE_RAW_SP, hSampler, nDevId, nParamId, varV);
}

/*=================================================================================================
** @Name      : pushParamInvalid
** @Function  : 推送无效值 (SAMP_INVALID_VALUE)，框架据此自动置 bRawInvalid=TRUE
**===============================================================================================*/
static void pushParamInvalid(HANDLE hSampler, int nDevId, int nParamId)
{
    VAR_VALUE varV;
    memset(&varV, 0, sizeof(varV));
    varV.lValue = SAMP_INVALID_VALUE;
    Push_VarValue(SETTYPE_RAW_SP, hSampler, nDevId, nParamId, varV);
}

/*=================================================================================================
** @Name      : beToHostArr
** @Function  : Read N big-endian USHORT registers from a Modbus RTU byte stream.
**              Modbus is big-endian; ARM is little-endian. Direct memcpy or USHORT*
**              reinterpretation would byte-swap every register (0x00FA -> 0xFA00).
**              Mirrors the per-register read pattern in pv_modbus_rtu.c getTwoBytes().
**===============================================================================================*/
static void beToHostArr(USHORT *pDst, const BYTE *pSrc, int nCount)
{
    int i;
    for(i = 0; i < nCount; i++)
        pDst[i] = (USHORT)((pSrc[i * 2] << 8) | pSrc[i * 2 + 1]);
}

/*=================================================================================================
**                         TF variant parse callbacks
**===============================================================================================*/
/* TF Control readback @0x1000 len 7 */
static BOOL parseTFCb_ctrl(int iStartAddr, BYTE *pDataIn, void *pDataOut)
{
    AIRCON_DEV *pDev = (AIRCON_DEV *)pDataOut;
    (void)iStartAddr;
    beToHostArr(pDev->tfCtrl, pDataIn, (int)(sizeof(pDev->tfCtrl) / sizeof(USHORT)));
    return TRUE;
}

/* TF Status @0x0000 len 6 */
static BOOL parseTFCb_0x0000(int iStartAddr, BYTE *pDataIn, void *pDataOut)
{
    AIRCON_DEV *pDev = (AIRCON_DEV *)pDataOut;
    (void)iStartAddr;
    beToHostArr(pDev->tfSta0, pDataIn, (int)(sizeof(pDev->tfSta0) / sizeof(USHORT)));
    return TRUE;
}

/* TF Status @0x0007 len 3 */
static BOOL parseTFCb_0x0007(int iStartAddr, BYTE *pDataIn, void *pDataOut)
{
    AIRCON_DEV *pDev = (AIRCON_DEV *)pDataOut;
    (void)iStartAddr;
    beToHostArr(pDev->tfSta1, pDataIn, (int)(sizeof(pDev->tfSta1) / sizeof(USHORT)));
    return TRUE;
}

/* TF Status @0x000B len 4 */
static BOOL parseTFCb_0x000B(int iStartAddr, BYTE *pDataIn, void *pDataOut)
{
    AIRCON_DEV *pDev = (AIRCON_DEV *)pDataOut;
    (void)iStartAddr;
    beToHostArr(pDev->tfSta2, pDataIn, (int)(sizeof(pDev->tfSta2) / sizeof(USHORT)));
    return TRUE;
}

/* TF Status @0x0012 len 2 */
static BOOL parseTFCb_0x0012(int iStartAddr, BYTE *pDataIn, void *pDataOut)
{
    AIRCON_DEV *pDev = (AIRCON_DEV *)pDataOut;
    (void)iStartAddr;
    beToHostArr(pDev->tfSta3, pDataIn, (int)(sizeof(pDev->tfSta3) / sizeof(USHORT)));
    return TRUE;
}

/* TF Status @0x0015 len 1 */
static BOOL parseTFCb_0x0015(int iStartAddr, BYTE *pDataIn, void *pDataOut)
{
    AIRCON_DEV *pDev = (AIRCON_DEV *)pDataOut;
    (void)iStartAddr;
    beToHostArr(pDev->tfSta4, pDataIn, (int)(sizeof(pDev->tfSta4) / sizeof(USHORT)));
    return TRUE;
}

/* TF Status @0x0019 len 9 (0x0019~0x0021) */
static BOOL parseTFCb_0x0019(int iStartAddr, BYTE *pDataIn, void *pDataOut)
{
    AIRCON_DEV *pDev = (AIRCON_DEV *)pDataOut;
    (void)iStartAddr;
    beToHostArr(pDev->tfSta5, pDataIn, (int)(sizeof(pDev->tfSta5) / sizeof(USHORT)));
    return TRUE;
}

/* TF Status @0x0022 len 5 */
static BOOL parseTFCb_0x0022(int iStartAddr, BYTE *pDataIn, void *pDataOut)
{
    AIRCON_DEV *pDev = (AIRCON_DEV *)pDataOut;
    (void)iStartAddr;
    beToHostArr(pDev->tfSta6, pDataIn, (int)(sizeof(pDev->tfSta6) / sizeof(USHORT)));
    return TRUE;
}

/*=================================================================================================
** @Name      : pushTFAirconParams
** @Function  : Push all TF StatusParameters (1770-1896) to the framework.
**              Field indexing follows R-core gTFAirStateInfo_213 / gTFAirUserParaInfo_212 layout.
**===============================================================================================*/
static void pushTFAirconParams(HANDLE hSampler, const AIRCON_DEV *pDev)
{
    const int nDevId = pDev->nDevId;

    /* Control readback @0x1000 (7 regs: StartMode, Mode, ControlObj, ColdTemp, ColdDiff, HeatTemp, HeatDiff) */
    pushParamEnum (hSampler, nDevId, TF_PARAM_STARTUP_MODE,        (long)pDev->tfCtrl[0]);
    pushParamEnum (hSampler, nDevId, TF_PARAM_WORKING_MODE,        (long)pDev->tfCtrl[1]);
    pushParamEnum (hSampler, nDevId, TF_PARAM_CONTROL_OBJ,         (long)pDev->tfCtrl[2]);
    pushParamFloat(hSampler, nDevId, TF_PARAM_COLD_TEMP_SET,       (float)(short)pDev->tfCtrl[3]);
    pushParamFloat(hSampler, nDevId, TF_PARAM_COLD_DIFF,           (float)(short)pDev->tfCtrl[4] * 0.1f);
    pushParamFloat(hSampler, nDevId, TF_PARAM_HEAT_TEMP_SET,       (float)(short)pDev->tfCtrl[5]);
    pushParamFloat(hSampler, nDevId, TF_PARAM_HEAT_DIFF,           (float)(short)pDev->tfCtrl[6] * 0.1f);

    /* Status @0x0000 len 6 (UnitState, UnitMode, [rsv], LiquidSupply, ReturnLiquid, SteamIn) */
    pushParamEnum (hSampler, nDevId, TF_PARAM_UNIT_STATE,          (long)pDev->tfSta0[0]);
    pushParamEnum (hSampler, nDevId, TF_PARAM_UNIT_MODE,           (long)pDev->tfSta0[1]);
    pushParamFloat(hSampler, nDevId, TF_PARAM_LIQUID_SUPPLY_TEMP,  (float)(short)pDev->tfSta0[3] * 0.1f);
    pushParamFloat(hSampler, nDevId, TF_PARAM_RETURN_LIQUID_TEMP,  (float)(short)pDev->tfSta0[4] * 0.1f);
    pushParamFloat(hSampler, nDevId, TF_PARAM_STEAM_IN_TEMP,       (float)(short)pDev->tfSta0[5] * 0.1f);

    /* Status @0x0007 len 3 (SteamOutTemp, AmbientTemp, CondensationTemp) */
    pushParamFloat(hSampler, nDevId, TF_PARAM_STEAM_OUT_TEMP,      (float)(short)pDev->tfSta1[0] * 0.1f);
    pushParamFloat(hSampler, nDevId, TF_PARAM_AMBIENT_TEMP,        (float)(short)pDev->tfSta1[1] * 0.1f);
    pushParamFloat(hSampler, nDevId, TF_PARAM_CONDENSATION_TEMP,   (float)(short)pDev->tfSta1[2] * 0.1f);

    /* Status @0x000B len 4 (EEVOpening, PumpSpeed, CompSpeed, FanSpeed) */
    pushParamInt  (hSampler, nDevId, TF_PARAM_EEV_OPENING,         (long)pDev->tfSta2[0]);
    pushParamInt  (hSampler, nDevId, TF_PARAM_PUMP_SPEED,          (long)pDev->tfSta2[1]);
    pushParamInt  (hSampler, nDevId, TF_PARAM_COMPRESSOR_SPEED,    (long)pDev->tfSta2[2]);
    pushParamInt  (hSampler, nDevId, TF_PARAM_FAN_SPEED,           (long)pDev->tfSta2[3]);

    /* Status @0x0012 len 2 (WaterSupplyPres, ReturnWaterPres) */
    pushParamFloat(hSampler, nDevId, TF_PARAM_WATER_SUPPLY_PRES,   (float)(short)pDev->tfSta3[0] * 0.1f);
    pushParamFloat(hSampler, nDevId, TF_PARAM_RETURN_WATER_PRES,   (float)(short)pDev->tfSta3[1] * 0.1f);

    /* Status @0x0015 len 1 (ExhaustPres) */
    pushParamFloat(hSampler, nDevId, TF_PARAM_EXHAUST_PRES,        (float)(short)pDev->tfSta4[0] * 0.1f);

    /* FIXME: DI/relay 状态参数 1792-1801 (系统高压/系统低压/液位开关/泵过载/加热管过载/
     *        风机过载/继电器输出/加热管/报警继电器/加热带) 在 TF 协议 0x0000~0x0026 中
     *        无独立 DI 寄存器来源, 暂推 0 (避免从故障字误读导致假告警)。 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_SYSTEM_HIGH_VOLT,   0);
    pushParamEnum (hSampler, nDevId, TF_PARAM_SYSTEM_LOW_VOLT,    0);
    pushParamEnum (hSampler, nDevId, TF_PARAM_LIQUID_LEVEL_SW,    0);
    pushParamEnum (hSampler, nDevId, TF_PARAM_PUMP_OVERLOAD,      0);
    pushParamEnum (hSampler, nDevId, TF_PARAM_HEAT_TUBE_OVERLOAD, 0);
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_OVERLOAD,       0);
    pushParamEnum (hSampler, nDevId, TF_PARAM_RELAY_OUTPUT,       0);
    pushParamEnum (hSampler, nDevId, TF_PARAM_HEAT_TUBE,          0);
    pushParamEnum (hSampler, nDevId, TF_PARAM_ALARM_RELAY,        0);
    pushParamEnum (hSampler, nDevId, TF_PARAM_HEATING_STRIP,      0);

    /* ===== TF 故障位图 @0x0019~0x001D -> Param_ID 1802-1865 ===== 
     * 故障位语义: 0=无故障, 1=有故障; 直接推位值 (DB 告警条件为 ">0", 与 EU3 一致)。
     * 注: 0x001E/0x001F(故障状态6/7)全部备用, 不推。 */
    /* 0x0019 故障状态1 (tfSta5[0]): 泵/主板/电压故障 bit0-13 -> 1802-1815; bit14-15 -> 3730-3731 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_PUMP_MALFUNCTION,    (long)((pDev->tfSta5[0] >> 0) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_PUMP_IDLE_PROTECT,   (long)((pDev->tfSta5[0] >> 1) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_PUMP_BLOCKAGE,       (long)((pDev->tfSta5[0] >> 2) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_PUMP_OVERCURRENT,    (long)((pDev->tfSta5[0] >> 3) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_PUMP_UNDER_PRES,     (long)((pDev->tfSta5[0] >> 4) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_PUMP_OVER_PRES,      (long)((pDev->tfSta5[0] >> 5) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_PUMP_PHASE_LOSS,     (long)((pDev->tfSta5[0] >> 6) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_PUMP_DRIVE_OVERHEAT, (long)((pDev->tfSta5[0] >> 7) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_WATER_PRES_SENSOR,   (long)((pDev->tfSta5[0] >> 8) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_MB_IIC_FAULT,        (long)((pDev->tfSta5[0] >> 9) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_RETURN_WATER_LOW,    (long)((pDev->tfSta5[0] >> 10) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_WATER_SUPPLY_HIGH,   (long)((pDev->tfSta5[0] >> 11) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_VOLT_HIGH,           (long)((pDev->tfSta5[0] >> 12) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_VOLT_LOW,            (long)((pDev->tfSta5[0] >> 13) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_DRIVE_OVERCURR, (long)((pDev->tfSta5[0] >> 14) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_DIFF_PRESS_ULTRALOW,(long)((pDev->tfSta5[0] >> 15) & 1));

    /* 0x001A 故障状态2 (tfSta5[1]): 系统压/探头/压机变频器通讯故障 bit0-7 -> 1816-1824; bit8-11 -> 3732-3735 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_SYS_LOW_PRES,        (long)((pDev->tfSta5[1] >> 0) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_ELEC_HEAT_PROTECT,   (long)((pDev->tfSta5[1] >> 1) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_OVERLOAD_FAULT,  (long)((pDev->tfSta5[1] >> 2) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_SYS_HIGH_PRES_FAULT, (long)((pDev->tfSta5[1] >> 3) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_EVAP_IN_PROBE,       (long)((pDev->tfSta5[1] >> 4) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_EVAP_OUT_PROBE,      (long)((pDev->tfSta5[1] >> 5) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_SYS_HIGH_PRES_LOCK,  (long)((pDev->tfSta5[1] >> 6) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_INV_COMM_FAULT, (long)((pDev->tfSta5[1] >> 7) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_INV_COMM_FAULT,   (long)((pDev->tfSta5[1] >> 8) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_INV_OTHER_FAULT, (long)((pDev->tfSta5[1] >> 9) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_INV_OTHER_FAULT,  (long)((pDev->tfSta5[1] >> 10) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_DIFF_PRESS_LOW,       (long)((pDev->tfSta5[1] >> 11) & 1));

    /* 0x001B 故障状态3 (tfSta5[2]): 液温/探头/压力/掉电故障 bit0-8 (1823 在 bit5) */
    pushParamEnum (hSampler, nDevId, TF_PARAM_LIQUID_TEMP_PROBE,   (long)((pDev->tfSta5[2] >> 0) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_RETURN_LIQ_PROBE,    (long)((pDev->tfSta5[2] >> 1) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_AMBIENT_PROBE,       (long)((pDev->tfSta5[2] >> 2) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_LIQUID_TEMP_HIGH,    (long)((pDev->tfSta5[2] >> 3) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_RETURN_WATER_SENSOR, (long)((pDev->tfSta5[2] >> 4) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_EXH_PRES_SENSOR,     (long)((pDev->tfSta5[2] >> 5) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_LIQUID_TEMP_LOW,     (long)((pDev->tfSta5[2] >> 6) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_WATER_SUPPLY_HIGH_WARN, (long)((pDev->tfSta5[2] >> 7) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_POWER_DROP_ALARM,    (long)((pDev->tfSta5[2] >> 8) & 1));

    /* 0x001C 故障状态4 (tfSta5[3]): 压机变频器故障 bit0-15 -> 1835-1850 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_START_OVERCURR,   (long)((pDev->tfSta5[3] >> 0) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_ACC_OVERCURR,     (long)((pDev->tfSta5[3] >> 1) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_DEC_OVERCURR,     (long)((pDev->tfSta5[3] >> 2) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_STEADY_OVERCURR,  (long)((pDev->tfSta5[3] >> 3) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_ACC_OVERVOLT,     (long)((pDev->tfSta5[3] >> 4) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_DEC_OVERVOLT,     (long)((pDev->tfSta5[3] >> 5) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_STEADY_OVERVOLT,  (long)((pDev->tfSta5[3] >> 6) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_STANDBY_OVERVOLT, (long)((pDev->tfSta5[3] >> 7) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_RUN_UNDERVOLT,    (long)((pDev->tfSta5[3] >> 8) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_OUTPUT_PHASE_LOSS,(long)((pDev->tfSta5[3] >> 9) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_POWER_DEV_PROTECT,(long)((pDev->tfSta5[3] >> 10) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_OVERHEAT,         (long)((pDev->tfSta5[3] >> 11) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_OVERLOAD,         (long)((pDev->tfSta5[3] >> 12) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_DETECT_OVERLOAD,  (long)((pDev->tfSta5[3] >> 13) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_LOAD_TOO_HIGH,    (long)((pDev->tfSta5[3] >> 14) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_SPEED_TOO_HIGH,   (long)((pDev->tfSta5[3] >> 15) & 1));

    /* 0x001D 故障状态5 (tfSta5[4]): 压机变频器故障 bit0-14 -> 1851-1865 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_D_AXIS_OVERCURR,  (long)((pDev->tfSta5[4] >> 0) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_Q_AXIS_OVERCURR,  (long)((pDev->tfSta5[4] >> 1) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_PARAM_STORE_FAIL, (long)((pDev->tfSta5[4] >> 2) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_COMM_FAULT,       (long)((pDev->tfSta5[4] >> 3) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_CURR_DETECT_FAULT,(long)((pDev->tfSta5[4] >> 4) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_START_STALL,      (long)((pDev->tfSta5[4] >> 5) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_RUN_STALL,        (long)((pDev->tfSta5[4] >> 6) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_LOST_SPEED,       (long)((pDev->tfSta5[4] >> 7) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_INT_OVF1,         (long)((pDev->tfSta5[4] >> 8) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_INT_OVF2,         (long)((pDev->tfSta5[4] >> 9) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_START_JITTER,     (long)((pDev->tfSta5[4] >> 10) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_COMP_RUN_JITTER,       (long)((pDev->tfSta5[4] >> 11) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_PFC_OVERCURR,          (long)((pDev->tfSta5[4] >> 12) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_PFC_PEAK_OVERCURR,     (long)((pDev->tfSta5[4] >> 13) & 1));
    pushParamEnum (hSampler, nDevId, TF_PARAM_PFC_RMS_OVERCURR,      (long)((pDev->tfSta5[4] >> 14) & 1));

    /* 标量故障码 (非位域) */
    pushParamInt  (hSampler, nDevId, TF_PARAM_INV_FAULT_CODE,        (long)pDev->tfSta5[8]);   /* 0x0021 压机故障代码 */
    pushParamInt  (hSampler, nDevId, TF_PARAM_SYS_TOTAL_FAULT_LEVEL, (long)pDev->tfSta6[0]);   /* 0x0022 系统总故障等级 */
    pushParamInt  (hSampler, nDevId, TF_PARAM_SYS_TOTAL_FAULT_CODE,  (long)pDev->tfSta6[1]);   /* 0x0023 系统总故障代码 */

    /* 无 Modbus 来源参数 -> 推 0 (不在 0x0019~0x0023 位表中的告警参数) */
    pushParamEnum (hSampler, nDevId, TF_PARAM_DISPLAY_COMM_TIMEOUT, 0);   /* 1830 屏幕与主板通信超时 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_SLAVE_COMM_FAULT,     0);   /* 1834 从板通讯故障 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_START_OVERCURR,   0);   /* 1869 风机启动瞬间过流 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_ACC_OVERCURR,     0);   /* 1870 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_DEC_OVERCURR,     0);   /* 1871 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_STEADY_OVERCURR,  0);   /* 1872 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_ACC_OVERVOLT,     0);   /* 1873 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_DEC_OVERVOLT,     0);   /* 1874 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_STEADY_OVERVOLT,  0);   /* 1875 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_STANDBY_OVERVOLT, 0);   /* 1876 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_RUN_UNDERVOLT,    0);   /* 1877 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_INPUT_PHASE_LOSS, 0);   /* 1878 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_OUTPUT_PHASE_LOSS,0);   /* 1879 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_POWER_DEV_PROTECT,0);   /* 1880 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_OVERHEAT,         0);   /* 1881 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_INV_OVERLOAD,         0);   /* 1882 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_INV_FAN_OVERLOAD,     0);   /* 1883 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_EXTERNAL_FAULT,       0);   /* 1884 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_MOTOR_OVERLOAD,       0);   /* 1885 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_INV_UNDERLOAD,        0);   /* 1886 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_FAULT,            0);   /* 1887 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_USAGE_TERM_EXPIRED,   0);   /* 1888 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_PARAM_STORE_FAIL, 0);   /* 1889 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_COMM_FAULT,       0);   /* 1890 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_CURR_DETECT_FAULT,0);   /* 1891 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_AUTOTUNE_FAULT,   0);   /* 1892 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_ANALOG_IN_DROP,   0);   /* 1893 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_PG_DROP,          0);   /* 1894 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_THERMISTOR_OPEN,  0);   /* 1895 */
    pushParamEnum (hSampler, nDevId, TF_PARAM_FAN_ABNORMAL_STOP,    0);   /* 1896 */
}

/*=================================================================================================
**                         EU3 variant parse callbacks & push
**===============================================================================================*/
/* EU3 Control readback @501 len 7 */
static BOOL parseEU3Cb_501(int iStartAddr, BYTE *pDataIn, void *pDataOut)
{
    AIRCON_DEV *pDev = (AIRCON_DEV *)pDataOut;
    (void)iStartAddr;
    beToHostArr(pDev->eu3Ctrl, pDataIn, (int)(sizeof(pDev->eu3Ctrl) / sizeof(USHORT)));
    return TRUE;
}

/* EU3 Control readback @703 len 4 (signed) */
static BOOL parseEU3Cb_703(int iStartAddr, BYTE *pDataIn, void *pDataOut)
{
    AIRCON_DEV *pDev = (AIRCON_DEV *)pDataOut;
    (void)iStartAddr;
    beToHostArr(pDev->eu3Ctrl1, pDataIn, (int)(sizeof(pDev->eu3Ctrl1) / sizeof(USHORT)));
    return TRUE;
}

/* EU3 Status @499 len 12 */
static BOOL parseEU3Cb_499(int iStartAddr, BYTE *pDataIn, void *pDataOut)
{
    AIRCON_DEV *pDev = (AIRCON_DEV *)pDataOut;
    (void)iStartAddr;
    beToHostArr(pDev->eu3Sta0, pDataIn, (int)(sizeof(pDev->eu3Sta0) / sizeof(USHORT)));
    /* EU3 V3.00 Input regs @499 len 12:
     *   [0]=499 SW ver  [1]=500 power on/off  [2]=501 run mode  [3]=502 run stage
     *   [4]=503 fault ASCII [5]=504 target temp×10 [6]=505 feedback water×10
     *   [7]=506 suction superheat  [8]=507 LP sat temp
     *   [9]=508 EEV steps  [10]=509 capacity %  [11]=510 PID-P %
     * NOTE: Temperature struct fields (feedTemp/exhTemp/batTemp/tarTemp/ambiTemp/sucTemp)
     *       are populated from @600 NTC block in parseEU3Cb_600, NOT here. */
    pDev->pwrState = (int)pDev->eu3Sta0[1];   /* reg 500 = actual power on/off */
    pDev->runMode  = (int)pDev->eu3Sta0[2];   /* reg 501 = actual run mode */
    pDev->runStep  = (int)pDev->eu3Sta0[3];   /* reg 502 = system run stage */
    pDev->errCode  = (int)pDev->eu3Sta0[4];   /* reg 503 = fault ASCII code */
    return TRUE;
}

/* EU3 Status @513 len 4 */
static BOOL parseEU3Cb_513(int iStartAddr, BYTE *pDataIn, void *pDataOut)
{
    AIRCON_DEV *pDev = (AIRCON_DEV *)pDataOut;
    (void)iStartAddr;
    beToHostArr(pDev->eu3Sta1, pDataIn, (int)(sizeof(pDev->eu3Sta1) / sizeof(USHORT)));
    /* EU3 V3.00 Input regs @513 len 4:
     *   [0]=513 comp TARGET freq Hz  [1]=514 comp ACTUAL freq Hz
     *   [2]=515 comp running stage    [3]=516 comp power W */
    pDev->comTarFre = (float)pDev->eu3Sta1[0];  /* reg 513 = target freq */
    pDev->comRunFre = (float)pDev->eu3Sta1[1];  /* reg 514 = actual freq */
    return TRUE;
}

/* EU3 Status @538 len 2 (故障等级: fluid-path fault status + compressor limit state) */
static BOOL parseEU3Cb_538(int iStartAddr, BYTE *pDataIn, void *pDataOut)
{
    AIRCON_DEV *pDev = (AIRCON_DEV *)pDataOut;
    (void)iStartAddr;
    beToHostArr(pDev->eu3Sta2, pDataIn, (int)(sizeof(pDev->eu3Sta2) / sizeof(USHORT)));
    return TRUE;
}

/* EU3 Status @600 len 11 (NTC temperatures) */
static BOOL parseEU3Cb_600(int iStartAddr, BYTE *pDataIn, void *pDataOut)
{
    AIRCON_DEV *pDev = (AIRCON_DEV *)pDataOut;
    (void)iStartAddr;
    beToHostArr(pDev->eu3Sta3, pDataIn, (int)(sizeof(pDev->eu3Sta3) / sizeof(USHORT)));
    /* EU3 V3.00 Input regs @600 len 11, all int16 ×0.1°C:
     *   [0]=600 NITC1 排气温度          → exhTemp
     *   [1]=601 NITC2 吸气温度          → sucTemp
     *   [2]=602 NITC3 冷却水进水温度
     *   [3]=603 NITC4 模块侧供水(出水)   → feedTemp (MOD_OUTLET_WATER)
     *   [4]=604 NITC5 模块侧回水(进水)
     *   [5]=605 NITC6 电池侧回水(进水)   → batTemp (BATT_INLET_WATER)
     *   [6]=606 NITC7 电池侧供水(出水)   → tarTemp (BATT_OUTLET_WATER)
     *   [7]=607 NITC8 冷却水模块出水
     *   [8]=608 NITC9 冷却水电池出水
     *   [9]=609 NITC10 环境温度          → ambiTemp
     *   [10]=610 NITC11 */
    pDev->exhTemp  = (float)((short)pDev->eu3Sta3[0]) * 0.1f;  /* reg 600 排气温度 */
    pDev->sucTemp  = (float)((short)pDev->eu3Sta3[1]) * 0.1f;  /* reg 601 吸气温度 */
    pDev->feedTemp = (float)((short)pDev->eu3Sta3[3]) * 0.1f;  /* reg 603 模块出水温度 */
    pDev->batTemp  = (float)((short)pDev->eu3Sta3[5]) * 0.1f;  /* reg 605 电池进水温度 */
    pDev->tarTemp  = (float)((short)pDev->eu3Sta3[6]) * 0.1f;  /* reg 606 电池出水温度 */
    pDev->ambiTemp = (float)((short)pDev->eu3Sta3[9]) * 0.1f;  /* reg 609 环境温度 */
    return TRUE;
}

/* EU3 Status @612 len 6 (pressures) */
static BOOL parseEU3Cb_612(int iStartAddr, BYTE *pDataIn, void *pDataOut)
{
    AIRCON_DEV *pDev = (AIRCON_DEV *)pDataOut;
    (void)iStartAddr;
    beToHostArr(pDev->eu3Sta4, pDataIn, (int)(sizeof(pDev->eu3Sta4) / sizeof(USHORT)));
    pDev->highPre = (float)((short)pDev->eu3Sta4[0]) * 0.1f;
    pDev->lowPre  = (float)((short)pDev->eu3Sta4[1]) * 0.1f;
    return TRUE;
}

/* EU3 Status @700 len 7 (fault bitmaps)
 *   [0]=700 传感器故障 (NTC/PS sensor bits)
 *   [1]=701 传感器故障 cont'd
 *   [2]=702 系统部件故障 (DI1-6 fan/heat/liquid + DI17-18 pump)
 *   [3]=703 压缩机故障
 *   [4]=704 压缩机故障 cont'd
 *   [5]=705 系统状态故障
 *   [6]=706 系统状态故障 cont'd */
static BOOL parseEU3Cb_700(int iStartAddr, BYTE *pDataIn, void *pDataOut)
{
    AIRCON_DEV *pDev = (AIRCON_DEV *)pDataOut;
    (void)iStartAddr;
    beToHostArr(pDev->eu3Sta5, pDataIn, (int)(sizeof(pDev->eu3Sta5) / sizeof(USHORT)));
    return TRUE;
}

/*=================================================================================================
** @Name      : pushEU3AirconParams
** @Function  : Push all 50 EU3 StatusParameters (1521-1570) to the framework.
**===============================================================================================*/
static void pushEU3AirconParams(HANDLE hSampler, const AIRCON_DEV *pDev)
{
    const int nDevId = pDev->nDevId;


    /* Control readback @501 + @703 */
    pushParamEnum (hSampler, nDevId, EU3_PARAM_WORKING_MODE,       (long)pDev->eu3Ctrl[2]);    /* Mode */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_COLD_TEMP_SET,      (float)(short)pDev->eu3Ctrl[4] * 0.1f);
    pushParamFloat(hSampler, nDevId, EU3_PARAM_COLD_START_DIFF,    (float)(short)pDev->eu3Ctrl1[0] * 0.1f);
    pushParamFloat(hSampler, nDevId, EU3_PARAM_COLD_STOP_DIFF,     (float)(short)pDev->eu3Ctrl1[1] * 0.1f);
    pushParamFloat(hSampler, nDevId, EU3_PARAM_HEAT_TEMP_SET,      (float)(short)pDev->eu3Ctrl[5] * 0.1f);
    pushParamFloat(hSampler, nDevId, EU3_PARAM_HEAT_TEMP_DIFF,     (float)(short)pDev->eu3Ctrl1[2] * 0.1f);
    pushParamEnum (hSampler, nDevId, EU3_PARAM_NOISE_STATE_RD,     (long)pDev->eu3Ctrl[6]);   /* reg 507 noise state readback */
    pushParamEnum (hSampler, nDevId, EU3_PARAM_REFRIGERANT_TYPE,   0);   /* config, not readable */
    pushParamEnum (hSampler, nDevId, EU3_PARAM_HOST_BAUD,          0);
    pushParamInt  (hSampler, nDevId, EU3_PARAM_HOST_MODBUS_ADDR,   0);
    pushParamEnum (hSampler, nDevId, EU3_PARAM_OUTDOOR_SPEC,       0);

    /* Status @513 (compressor freq/power) + @499 (EEV) */
    pushParamInt  (hSampler, nDevId, EU3_PARAM_EEV_OPENING,        (long)pDev->eu3Sta0[9]);   /* reg 508 EEV steps */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_COMP_TARGET_FREQ,   (float)pDev->eu3Sta1[0]);  /* reg 513 target freq Hz */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_COMP_OPER_FREQ,     (float)pDev->eu3Sta1[1]);  /* reg 514 actual freq Hz */
    pushParamInt  (hSampler, nDevId, EU3_PARAM_FAN_TARGET_SPEED,   0);  /* not in current read blocks */

    /* Status @600 NTC temperatures (all int16 x0.1 degC) */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_EXH_GAS_SUPERHEAT,  0.0f);  /* derived */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_EXT_LOOP_TEMP,      (float)(short)pDev->eu3Sta3[2] * 0.1f); /* reg 602 cooling water inlet */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_EXH_GAS_TEMP,       pDev->exhTemp);   /* reg 600 排气温度 */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_SUCCTION_TEMP,      pDev->sucTemp);   /* reg 601 吸气温度 */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_MOD_INLET_WATER,    (float)(short)pDev->eu3Sta3[4] * 0.1f); /* reg 604 模块侧回水(进水) */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_MOD_OUTLET_WATER,   pDev->feedTemp);  /* reg 603 模块侧供水(出水) */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_BATT_INLET_WATER,   pDev->batTemp);   /* reg 605 电池侧回水(进水) */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_BATT_OUTLET_WATER,  pDev->tarTemp);   /* reg 606 电池侧供水(出水) */
    pushParamInt  (hSampler, nDevId, EU3_PARAM_PUMP_ACTUAL_SPEED,  0);  /* not in current read blocks */

    /* Status @612 (pressures, int16 x0.1 bar) */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_MOD_OUT_WATER_PRES, pDev->highPre);   /* reg 612 高压(排气) */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_BATT_OUT_WATER_PRES,pDev->lowPre);    /* reg 613 低压(吸气) */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_HP_SATURATION_TEMP, 0.0f);  /* derived */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_LP_SATURATION_TEMP, 0.0f);
    pushParamFloat(hSampler, nDevId, EU3_PARAM_SUC_SUPERHEAT_TEMP, 0.0f);
    pushParamFloat(hSampler, nDevId, EU3_PARAM_LOW_PRESSURE,       pDev->lowPre);    /* reg 613 */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_HIGH_PRESSURE,      pDev->highPre);   /* reg 612 */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_AC_INPUT_VOLT,      0.0f);  /* not in current read blocks */
    pushParamFloat(hSampler, nDevId, EU3_PARAM_AC_INPUT_CURR,      0.0f);
    pushParamFloat(hSampler, nDevId, EU3_PARAM_COMP_PHASE_CURR,    0.0f);
    pushParamInt  (hSampler, nDevId, EU3_PARAM_DC_BUS_VOLT,        0);

    /* Status states from @499 (input regs) */
    pushParamEnum (hSampler, nDevId, EU3_PARAM_ELEC_HEAT_STATUS,   (long)pDev->pwrState);  /* reg 500 实际开机状态 */
    pushParamEnum (hSampler, nDevId, EU3_PARAM_WATER_PUMP_STATUS,  (long)pDev->runMode);   /* reg 501 实际运行模式 */
    pushParamEnum (hSampler, nDevId, EU3_PARAM_CONDENSER_FAN,      (long)pDev->runStep);   /* reg 502 系统运行阶段 */
    pushParamEnum (hSampler, nDevId, EU3_PARAM_COMP_STATUS,        (long)pDev->errCode);   /* reg 503 故障ASCII码 */
    pushParamEnum (hSampler, nDevId, EU3_PARAM_CRANK_HEATER,       0);  /* not available */
    pushParamEnum (hSampler, nDevId, EU3_PARAM_OPERATING_STATUS,   0);  /* derived */
    pushParamEnum (hSampler, nDevId, EU3_PARAM_OIL_RETURN,         0);
    pushParamInt  (hSampler, nDevId, EU3_PARAM_SW_VERSION,         (long)pDev->eu3Sta0[0]); /* reg 499 SW version */

    /* Fault flags from @538 (fluid-path fault level) + @700~@706 (fault bitmaps) */
    pushParamEnum (hSampler, nDevId, EU3_PARAM_TOTAL_FAULT,        (long)pDev->eu3Sta2[0]); /* reg 538 故障等级 */
    /* Old fault params 1564-1570 (older firmware gen; fan bits map cleanly, rest 0) */
    pushParamEnum (hSampler, nDevId, EU3_PARAM_BATT_OUT_PRES_HI,   0);  /* derived from reg 705 */
    pushParamEnum (hSampler, nDevId, EU3_PARAM_MOD_OUT_PRES_HI,    0);
    pushParamEnum (hSampler, nDevId, EU3_PARAM_PTC_OVERLOAD,       0);
    pushParamEnum (hSampler, nDevId, EU3_PARAM_RETURN_WATER_HI_LO, 0);
    pushParamEnum (hSampler, nDevId, EU3_PARAM_OUTLET_TEMP_HI_LO,  0);
    pushParamEnum (hSampler, nDevId, EU3_PARAM_FAN1_MALFUNCTION,   (long)((pDev->eu3Sta5[2] >> 0) & 0x01)); /* reg 702 bit0 DI1 fan1 */
    pushParamEnum (hSampler, nDevId, EU3_PARAM_FAN2_MALFUNCTION,   (long)((pDev->eu3Sta5[2] >> 1) & 0x01)); /* reg 702 bit1 DI2 fan2 */

    /* ---- NEW EU3 V3.00 fault-bit params (3514-3641) ---- */
    {
        const USHORT sensor0 = pDev->eu3Sta5[0];   /* reg 700 传感器故障 */
        const USHORT sensor1 = pDev->eu3Sta5[1];   /* reg 701 传感器故障 cont'd */
        const USHORT compon  = pDev->eu3Sta5[2];   /* reg 702 系统部件故障 */
        const USHORT comp0   = pDev->eu3Sta5[3];   /* reg 703 压缩机故障 */
        const USHORT comp1   = pDev->eu3Sta5[4];   /* reg 704 压缩机故障 cont'd */
        const USHORT syssta0 = pDev->eu3Sta5[5];   /* reg 705 系统状态故障 */
        const USHORT fluid   = pDev->eu3Sta2[0];   /* reg 538: bit0-4 batt, bit5-7 module */
        const int    battPath = (int)(fluid & 0x1F);        /* 0=OK 1=alarm 2=stop */
        const int    modPath  = (int)((fluid >> 5) & 0x07); /* 0=OK 1=alarm 2=stop */

        /* Fluid-path fault status @538 (multi-value field) */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_FLUID_BATT_OK,      (long)(battPath == 0));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_FLUID_BATT_ALARM,   (long)(battPath == 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_FLUID_BATT_STOP_B0, (long)(battPath == 2));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_FLUID_BATT_STOP_B1, (long)(battPath == 3));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_FLUID_BATT_RSV,     (long)(battPath >= 4));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_FLUID_MOD_OK,       (long)(modPath == 0));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_FLUID_MOD_ALARM,    (long)(modPath == 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_FLUID_MOD_STOP,     (long)(modPath == 2));

        /* Sensor faults @700/701 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_NTC1_FAULT,  (long)((sensor0 >> 0)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_NTC2_FAULT,  (long)((sensor0 >> 1)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_NTC3_FAULT,  (long)((sensor0 >> 2)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_NTC4_FAULT,  (long)((sensor0 >> 3)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_NTC5_FAULT,  (long)((sensor0 >> 4)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_NTC6_FAULT,  (long)((sensor0 >> 5)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_NTC7_FAULT,  (long)((sensor0 >> 6)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_NTC8_FAULT,  (long)((sensor0 >> 7)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_NTC9_FAULT,  (long)((sensor0 >> 8)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_NTC10_FAULT, (long)((sensor0 >> 9)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_NTC11_FAULT, (long)((sensor0 >> 10) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_NTC12_FAULT, (long)((sensor0 >> 11) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_PS1_FAULT,   (long)((sensor0 >> 12) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_PS2_FAULT,   (long)((sensor0 >> 13) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_PS3_FAULT,   (long)((sensor0 >> 14) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_PS4_FAULT,   (long)((sensor0 >> 15) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_PS5_FAULT,   (long)((sensor1 >> 0)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_PS6_FAULT,   (long)((sensor1 >> 1)  & 1));

        /* Component/DI faults @702 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DI1_FAULT,    (long)((compon >> 0)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DI2_FAULT,    (long)((compon >> 1)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DI3_FAULT,    (long)((compon >> 2)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DI4_FAULT,    (long)((compon >> 3)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DI5_FAULT,    (long)((compon >> 4)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DI6_FAULT,    (long)((compon >> 5)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DI7_FAULT,    (long)((compon >> 6)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DI8_FAULT,    (long)((compon >> 7)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DI17_FAULT,   (long)((compon >> 8)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DI18_FAULT,   (long)((compon >> 9)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DI_B10_RSV,   (long)((compon >> 10) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DI_B11_RSV,   (long)((compon >> 11) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DI_B12_RSV,   (long)((compon >> 12) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DI_B13_RSV,   (long)((compon >> 13) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DI_B14_RSV,   (long)((compon >> 14) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_FAN_ALL_FAULT,(long)((compon >> 15) & 1));

        /* Compressor faults @703/704 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_COMP_COMM_FAULT,     (long)((comp0 >> 0)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_AC_OVERCURR,         (long)((comp0 >> 1)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_AC_INST_OVERCURR,    (long)((comp0 >> 2)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_PFC_OVERVOLT,        (long)((comp0 >> 3)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_COMP_INV_FAULT,      (long)((comp0 >> 4)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_COMP_INST_OVERCURR,  (long)((comp0 >> 5)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_COMP_LOST_SYNC,      (long)((comp0 >> 6)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_COMP_PHASE_LOSS,     (long)((comp0 >> 7)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_INV_PCB_OVERHEAT,    (long)((comp0 >> 8)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_IGBT_OVERHEAT,       (long)((comp0 >> 9)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_POWER_OVERLOAD,      (long)((comp0 >> 10) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_PFC_OVERCURR,        (long)((comp0 >> 11) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_AC_OVERVOLT,         (long)((comp0 >> 12) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DC_OVERVOLT,         (long)((comp0 >> 13) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_COMP_PHASE_OVERCURR, (long)((comp0 >> 14) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_AC_UNDERVOLT,        (long)((comp0 >> 15) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_DC_UNDERVOLT,        (long)((comp1 >> 0)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_AC_PHASE_LOSS,       (long)((comp1 >> 1)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_COMP_TOTAL_FAULT,    (long)((comp1 >> 2)  & 1));

        /* System-state warning faults @705 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_WARN_BATT_OUT_PRES_HI,  (long)((syssta0 >> 0)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_WARN_MOD_OUT_PRES_HI,   (long)((syssta0 >> 1)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_WARN_BATT_WATER_TEMP,   (long)((syssta0 >> 2)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_WARN_MOD_WATER_TEMP,    (long)((syssta0 >> 3)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_WARN_LOW_SUPERHEAT,     (long)((syssta0 >> 4)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_WARN_HP_PROTECT,        (long)((syssta0 >> 5)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_WARN_LP_PROTECT,        (long)((syssta0 >> 6)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_WARN_TANK_SHORTAGE,     (long)((syssta0 >> 7)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_WARN_MOD_SHORTAGE,      (long)((syssta0 >> 8)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_WARN_BATT_SHORTAGE,     (long)((syssta0 >> 9)  & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_WARN_BATT_TEMP_DIFF,    (long)((syssta0 >> 10) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_WARN_MOD_TEMP_DIFF,     (long)((syssta0 >> 11) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_WARN_COOL_FLOW,         (long)((syssta0 >> 12) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_WARN_EXH_HIGH,          (long)((syssta0 >> 13) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_WARN_COOL_TEMP,         (long)((syssta0 >> 14) & 1));
        pushParamEnum (hSampler, nDevId, EU3_PARAM_WARN_BATT_DIFF_PROTECT, (long)((syssta0 >> 15) & 1));

        /* ---- Legacy EU3 fault params 1571-1601 (old CAN-era numbering) ----
         * Alarms 1509-1539 reference these via Alarm_OccurCondition [$,0,1571..1601]==1.
         * Aircon now direct RS485 (EU3 V3.00): 21 map 1:1 to fault bits above, 10 were
         * EU3-core-only (no V3.00 bit) -> pushed 0. */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_FAN3,                 (long)((compon  >> 2)  & 1)); /* 1571 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_FAN4,                 (long)((compon  >> 3)  & 1)); /* 1572 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_SYS1_ALARM_STATUS,    0);   /* 1573 EU3 core only */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_SYS1_LOCK_STATUS,     0);   /* 1574 EU3 core only */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_SUBCOOLING_TEMP_SENSOR,0);   /* 1575 no NTC in V3.00 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_AMBIENT_TEMP_SENSOR,  (long)((sensor0 >> 9)  & 1)); /* 1576 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_EXHAUST_TEMP_SENSOR,  (long)((sensor0 >> 0)  & 1)); /* 1577 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_SUCTION_TEMP_SENSOR,  (long)((sensor0 >> 1)  & 1)); /* 1578 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_MODULE_INLET_TEMP_SENSOR, (long)((sensor0 >> 4) & 1)); /* 1579 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_MODULE_OUTLET_TEMP_SENSOR,(long)((sensor0 >> 3) & 1)); /* 1580 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_FAN2,                 (long)((compon  >> 1)  & 1)); /* 1581 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_BATT_INLET_TEMP_SENSOR, (long)((sensor0 >> 5) & 1)); /* 1582 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_BATT_OUTLET_TEMP_SENSOR,(long)((sensor0 >> 6) & 1)); /* 1583 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_LOW_SUPERHEAT_ALARM,  (long)((syssta0 >> 4)  & 1)); /* 1584 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_HIGH_PRESSURE_SENSOR, (long)((sensor0 >> 12) & 1)); /* 1585 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_LOW_PRESSURE_SENSOR,  (long)((sensor0 >> 13) & 1)); /* 1586 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_DRIVER_COMM_FAULT,    (long)((comp0   >> 0)  & 1)); /* 1587 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_WATER_TANK_SHORTAGE,  (long)((syssta0 >> 7)  & 1)); /* 1588 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_MODULE_SHORTAGE,      (long)((syssta0 >> 8)  & 1)); /* 1589 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_FAN_ALARM_FAULT,      (long)((compon  >> 15) & 1)); /* 1590 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_HIGH_PRESSURE_PROTECT,(long)((syssta0 >> 5)  & 1)); /* 1591 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_LOW_PRESSURE_PROTECT, (long)((syssta0 >> 6)  & 1)); /* 1592 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_BATT_SHORTAGE,        (long)((syssta0 >> 9)  & 1)); /* 1593 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_WINTER_ANTI_FREEZE_1, 0);   /* 1594 no bit in V3.00 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_WINTER_ANTI_FREEZE_2, 0);   /* 1595 no bit in V3.00 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_WATER_TEMP_ABNORMAL,  0);   /* 1596 EU3 core only */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_EXHAUST_OVER_TEMP,    (long)((syssta0 >> 13) & 1)); /* 1597 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_COOLING_ANTI_FREEZE,  0);   /* 1598 no bit in V3.00 */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_HY1_PUMP_FAULT,       0);   /* 1599 EU3 core only */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_HY3_PUMP_FAULT,       0);   /* 1600 EU3 core only */
        pushParamEnum (hSampler, nDevId, EU3_PARAM_LEGACY_DC_FAN1_FAULT,        0);   /* 1601 EU3 core only */
    }
}

/*=================================================================================================
** @Name      : invalidateAirconParams
** @Function  : 通信故障时将空调数据参数标记为无效 (按 TF/EU3 变体选择范围)
**===============================================================================================*/
static void invalidateAirconParams(HANDLE hSampler, const AIRCON_DEV *pDev)
{
    const int nDevId = pDev->nDevId;
    int i, iStart, iEnd;
    if(pDev->nAirType == AIR_TYPE_TF)
    {
        iStart = TF_PARAM_STARTUP_MODE;      /* 1770 */
        iEnd   = TF_PARAM_FAN_ABNORMAL_STOP;  /* 1896 */
        for(i = iStart; i <= iEnd; i++)
            pushParamInvalid(hSampler, nDevId, i);
    }
    else
    {
        iStart = EU3_PARAM_WORKING_MODE;      /* 1521 */
        iEnd   = EU3_PARAM_FAN2_MALFUNCTION;  /* 1570 */
        for(i = iStart; i <= iEnd; i++)
            pushParamInvalid(hSampler, nDevId, i);
        /* new EU3 fault-bit params (non-contiguous ID ranges) */
        for(i = EU3_PARAM_FLUID_BATT_OK;         i <= EU3_PARAM_FLUID_MOD_STOP;        i++)
            pushParamInvalid(hSampler, nDevId, i);
        for(i = EU3_PARAM_NTC1_FAULT;            i <= EU3_PARAM_PS6_FAULT;             i++)
            pushParamInvalid(hSampler, nDevId, i);
        for(i = EU3_PARAM_DI1_FAULT;             i <= EU3_PARAM_FAN_ALL_FAULT;         i++)
            pushParamInvalid(hSampler, nDevId, i);
        for(i = EU3_PARAM_COMP_COMM_FAULT;       i <= EU3_PARAM_COMP_TOTAL_FAULT;      i++)
            pushParamInvalid(hSampler, nDevId, i);
        for(i = EU3_PARAM_WARN_BATT_OUT_PRES_HI; i <= EU3_PARAM_WARN_BATT_DIFF_PROTECT; i++)
            pushParamInvalid(hSampler, nDevId, i);
        /* legacy EU3 fault params (1571-1601, old CAN-era numbering) */
        for(i = EU3_PARAM_LEGACY_FAN3; i <= EU3_PARAM_LEGACY_DC_FAN1_FAULT; i++)
            pushParamInvalid(hSampler, nDevId, i);
    }
    pushParamInvalid(hSampler, nDevId, EU3_PARAM_NOISE_STATE_RD); /* 3728 noise readback */
}

/*=================================================================================================
** @Name      : sampleTFAircon / sampleEu3Aircon
**===============================================================================================*/
static BOOL sampleTFAircon(AIRCON_SAMPLER *pSamp, COMMPORT *pComm, HANDLE hPort)
{
    MODBUS_REQUEST_INFO reqTab[] = {
        { MODBUS_GETDATA_CMD, MB_TF_AIR_WR_BASE_ADDR,  7, parseTFCb_ctrl   },  /* @0x1000 control readback */
        { MODBUS_GETDATA_CMD, MB_TF_AIR_RO_BASE_ADDR,  6, parseTFCb_0x0000 },
        { MODBUS_GETDATA_CMD, MB_TF_AIR_RO_BASE1_ADDR, 3, parseTFCb_0x0007 },
        { MODBUS_GETDATA_CMD, MB_TF_AIR_RO_BASE2_ADDR, 4, parseTFCb_0x000B },
        { MODBUS_GETDATA_CMD, MB_TF_AIR_RO_BASE3_ADDR, 2, parseTFCb_0x0012 },
        { MODBUS_GETDATA_CMD, MB_TF_AIR_RO_BASE4_ADDR, 1, parseTFCb_0x0015 },
        { MODBUS_GETDATA_CMD, MB_TF_AIR_RO_BASE5_ADDR, 9, parseTFCb_0x0019 },  /* 0x0019~0x0021 */
        { MODBUS_GETDATA_CMD, MB_TF_AIR_RO_BASE6_ADDR, 5, parseTFCb_0x0022 },
    };
    int i;
    int nSuccessCnt = 0;

    for(i = 0; i < (int)(sizeof(reqTab) / sizeof(reqTab[0])); i++)
    {
        BOOL bRet = sampleModbusRtuData(pSamp->nSlaveAddr, &reqTab[i], pComm, hPort, &pSamp->dev);
        if(bRet)
            nSuccessCnt++;
        /* 不再 break: 单块失败不影响其他块，已读到的数据仍然有效 */
    }
    /* 至少 1 块成功即认为本轮采样有效，避免偶发超时导致全部参数失效 */
    return (nSuccessCnt > 0) ? TRUE : FALSE;
}

static BOOL sampleEu3Aircon(AIRCON_SAMPLER *pSamp, COMMPORT *pComm, HANDLE hPort)
{
    MODBUS_REQUEST_INFO reqTab[] = {
        { MODBUS_GETDATA_CMD,    MB_EU3_AIR_WR_BASE_ADDR,  7, parseEU3Cb_501 },
        { MODBUS_GETDATA_CMD,    MB_EU3_AIR_WR_BASE1_ADDR, 4, parseEU3Cb_703 },
        { MODBUS_GETDATA_CMD_04, MB_EU3_AIR_RO_BASE_ADDR, 12, parseEU3Cb_499 },
        { MODBUS_GETDATA_CMD_04, MB_EU3_AIR_RO_BASE1_ADDR, 4, parseEU3Cb_513 },
        { MODBUS_GETDATA_CMD_04, MB_EU3_AIR_RO_BASE2_ADDR, 2, parseEU3Cb_538 },
        { MODBUS_GETDATA_CMD_04, MB_EU3_AIR_RO_BASE3_ADDR,11, parseEU3Cb_600 },
        { MODBUS_GETDATA_CMD_04, MB_EU3_AIR_RO_BASE4_ADDR, 6, parseEU3Cb_612 },
        { MODBUS_GETDATA_CMD_04, MB_EU3_AIR_RO_BASE5_ADDR, 7, parseEU3Cb_700 },
    };
    int i;
    int nSuccessCnt = 0;

    for(i = 0; i < (int)(sizeof(reqTab) / sizeof(reqTab[0])); i++)
    {
        BOOL bRet = sampleModbusRtuData(pSamp->nSlaveAddr, &reqTab[i], pComm, hPort, &pSamp->dev);
        if(bRet)
            nSuccessCnt++;
        /* 不再 break: 单块失败不影响其他块 */
    }
    return (nSuccessCnt > 0) ? TRUE : FALSE;
}

/*=================================================================================================
** @Name      : Initialize
** @Function  : sampler 框架入口，初始化数据结构并打开 CommPort
**===============================================================================================*/
DLLExport BOOL Initialize(HANDLE hSamplerSelf)
{
    SAMPLER         *pSampler = (SAMPLER *)hSamplerSelf;
    AIRCON_SAMPLER  *pSamp;

    pSampler->pDataHeap = (void *)malloc(sizeof(AIRCON_SAMPLER));
    if(pSampler->pDataHeap == NULL)
    {
        TRACE("[aircon] Initialize: malloc failed\n");
        return FALSE;
    }
    memset(pSampler->pDataHeap, 0, sizeof(AIRCON_SAMPLER));
    pSamp = (AIRCON_SAMPLER *)pSampler->pDataHeap;

    pSamp->hSampler        = hSamplerSelf;
    pSamp->nSamplerId      = pSampler->info.nSamplerID;
    pSamp->dev.nDevId      = AIRCON_DEV_ID;
    pSamp->dev.nDevModel   = AIRCON_DEV_MODEL;
    /* 空调类型: 从 DPR 读 param 448 (网页“设备安装”切换后存 DB; 启动时 DPR 已在步骤 2
     * LoadConfigAndData 完成恢复, 此时 Get 直接返回库值), 运行时切换走 Control(448)。
     * 取不到或值无效 (0=未安装 / 3/4=不支持) 时回退 TF — 实测 2026-08-13 真实设备为同飞协议
     * (TF 寄存器 0x0000-0x0022 正常应答, EU3 寄存器 499+ 全部异常 02)。
     * 注: 不能硬编码 — DPR 同值去重 (dpr_loader.c IsSettVal_SameAsOld) 会导致
     * “网页下发成功但 Control 不执行”, 硬编码与库值不一致时无法通过重下发修复 */
    pSamp->dev.nAirType    = AIR_TYPE_TF;
    BOOL bInstalled        = FALSE;   /* 0=未安装, 1/2/3/4=已安装 */
    {
        DATA_PROCESSOR *pDPR = (DATA_PROCESSOR *)pSampler->hDPR;
        if(pDPR != NULL && pDPR->Get != NULL)
        {
            VAR_VALUE varV;
            memset(&varV, 0, sizeof(varV));
            if(ERR_DPR_OK == pDPR->Get(hSamplerSelf, GETTYPE_ST_VAR, AIRCON_DEV_ID, 448,
                                       &varV, sizeof(varV)))
            {
                bInstalled = (varV.emValue != 0);
                if(varV.emValue == AIR_TYPE_EU3 || varV.emValue == AIR_TYPE_TF)
                {
                    pSamp->dev.nAirType = (int)varV.emValue;
                }
            }
        }
    }
    pSamp->dev.bCommOK     = bInstalled;   /* 已安装才监控通信, 未安装不报通信中断 */
    pSamp->nSlaveAddr      = MODBUS_SLAVE_ADDR_AIR;
    pSamp->nSamplePeriodMs = 1000;

    /* 初始状态推送: 通信未建立，标记数据无效
     * 防止 UI 因数据库默认值 0 而误显示"通信正常" */
invalidateAirconParams(hSamplerSelf, &pSamp->dev);

    /* Acquire comm port — NJX: failure must NOT return FALSE, otherwise
     * Init_Load_AllSamplers() aborts the entire nccu_app (sampler_loader.c:352).
     * Instead mark bInited=FALSE; Sample() will skip silently. */
    if(GetCommPort_ById(hSamplerSelf, AIRCON_COMMPORT_ID, pSampler->pUsedCommPort) == NULL)
    {
        TRACE("[aircon] Initialize WARNING: CommPort %d not available, sampler idle\n", AIRCON_COMMPORT_ID);
        pSamp->bInited = FALSE;
        return TRUE;
    }

    /* Open CommPort (sampler owns the serial device open) */
    int nErrCode = 0;
    pSampler->hCommPort = pSampler->pUsedCommPort->Open(
        pSampler->pUsedCommPort->info.szPortAttri,
        pSampler->info.szSamplerSetting,
        COMM_CLIENT_MODE, 1000, &nErrCode);
    if(pSampler->hCommPort == NULL || nErrCode != ERR_COMM_OK)
    {
        TRACE("[aircon] Initialize WARNING: Open(%s/%s) failed err=%d, sampler idle\n",
               pSampler->pUsedCommPort->info.szPortAttri,
               pSampler->info.szSamplerSetting,
               nErrCode);
        pSamp->bInited = FALSE;
        return TRUE;
    }
    /* Clear RX/TX buffers */
    pSampler->pUsedCommPort->Control(pSampler->hCommPort, COMM_PURGE_RXCLEAR, NULL, 0);
    pSampler->pUsedCommPort->Control(pSampler->hCommPort, COMM_PURGE_TXCLEAR, NULL, 0);

    pSamp->bInited = TRUE;

    return TRUE;
}

/*=================================================================================================
** @Name      : Sample
** @Function  : sampler 框架周期调用入口 (1000ms 周期)
**              根据 nAirType 分支 TF/EU3 读取，推送参数，处理通信故障
**===============================================================================================*/
DLLExport int Sample(HANDLE hSamplerThis)
{
    SAMPLER         *pSampler = (SAMPLER *)hSamplerThis;
    AIRCON_SAMPLER  *pSamp;
    COMMPORT        *pComm;
    HANDLE           hPort;
    BOOL             bReadOk = FALSE;
    static int       s_nSampleTick = 0;

    if(pSampler == NULL || pSampler->pDataHeap == NULL)
        return 0;
    pSamp = (AIRCON_SAMPLER *)pSampler->pDataHeap;
    pComm = pSampler->pUsedCommPort;
    hPort = pSampler->hCommPort;

    if(!pSamp->bInited)
    {
        /* 端口未打开 (无硬件): 周期性重推故障状态,
         * 防止 DPR 数据库默认值 (COMMSTAT=0 "通信正常") 覆盖 Initialize 的推送 */
        if((++s_nSampleTick % 10) == 1)   /* 每 ~10 秒重推一次 */
        {
            invalidateAirconParams(hSamplerThis, &pSamp->dev);
        }
        return 0;
    }


    /* Flush any stale RX data */
    if(pComm != NULL && pComm->Control != NULL)
    {
        pComm->Control(hPort, COMM_PURGE_RXCLEAR, NULL, 0);
    }

    /* Branch on AC type */
    switch(pSamp->dev.nAirType)
    {
        case AIR_TYPE_EU3:
            bReadOk = sampleEu3Aircon(pSamp, pComm, hPort);
            break;
        case AIR_TYPE_TF:
            bReadOk = sampleTFAircon(pSamp, pComm, hPort);
            break;
        default:
            break;
    }

    if(bReadOk)
    {
        pSamp->dev.nCommFailCnt = 0;
        if(!pSamp->dev.bCommOK)
        {
            pSamp->dev.nCommRecoverCnt++;
            if(pSamp->dev.nCommRecoverCnt >= AIRCON_COMM_RECOVER_LIMIT)
            {
                pSamp->dev.nCommRecoverCnt = 0;
                SetCommStatus(hSamplerThis, pSamp->dev.nDevId, TRUE);
                pSamp->dev.bCommOK = TRUE;
            }
        }
        /* Variant-specific push */
        if(pSamp->dev.nAirType == AIR_TYPE_TF)
            pushTFAirconParams(hSamplerThis, &pSamp->dev);
        else
            pushEU3AirconParams(hSamplerThis, &pSamp->dev);
    }
    else
    {
        pSamp->dev.nCommFailCnt++;
        pSamp->dev.nCommRecoverCnt = 0;
        if(pSamp->dev.nCommFailCnt >= AIRCON_COMM_FAULT_LIMIT && pSamp->dev.bCommOK)
        {
            pSamp->dev.bCommOK = FALSE;
            SetCommStatus(hSamplerThis, pSamp->dev.nDevId, FALSE);
            invalidateAirconParams(hSamplerThis, &pSamp->dev);
        }
    }

    /* Cap nCommFailCnt to prevent unbounded growth (signed int overflow ~68yr at 1s tick) */
    if(pSamp->dev.nCommFailCnt > AIRCON_COMM_FAULT_LIMIT * 2)
    {
        pSamp->dev.nCommFailCnt = AIRCON_COMM_FAULT_LIMIT;
    }
    return 0;
}

/*=================================================================================================
** @Name      : Control
** @Function  : Framework entry for SettingParameters and ControlParameters.
**              - PARAM_TYPE_SETTING:
**                448                  - AirCond Install Type (0=none, 1=EU3, 2=TF)
**                445                  - AirCond On/Off and Alarm Reset (write Modbus)
**                2080-2085            - TF setpoints (write Modbus 0x1001..0x1006)
**                2086-2095            - EU3 setpoints (write Modbus 501/703 blocks)
**                default/444          - pure save settings, just return 0
**===============================================================================================*/
DLLExport int Control(HANDLE hSamplerThis,
                      PARAM_TYPE emParaType,
                      int nDevId,
                      int nParamId,
                      VAR_VALUE *pVarVal,
                      int nValSize,
                      int nTimeOut)
{
    SAMPLER         *pSampler = (SAMPLER *)hSamplerThis;
    AIRCON_SAMPLER  *pSamp;

    (void)nValSize; (void)nTimeOut;
    if(pSampler == NULL || pSampler->pDataHeap == NULL || pVarVal == NULL)
        return -1;
    pSamp = (AIRCON_SAMPLER *)pSampler->pDataHeap;
    if(!pSamp->bInited)
        return -1;
    if(nDevId != AIRCON_DEV_ID)
        return -1;

    /* param 448 (aircon install type switch) does NOT depend on Modbus comm.
     * Must be handled BEFORE bCommOK guard to avoid deadlock:
     * bCommOK starts FALSE, so without this early bypass the type
     * switch request is always rejected and the sampler stays in default TF mode. */
    if(emParaType == PARAM_TYPE_SETTING && nParamId == 448)
    {
        int nNewType = (int)pVarVal->emValue;
        pSamp->dev.nAirType       = nNewType;
        pSamp->dev.nCommFailCnt    = 0;
        pSamp->dev.nCommRecoverCnt = 0;
        /* 安装类型切换: 清残留通信告警, 让状态机重新检测。
         * 未安装 (0) -> 停止监控, bCommOK=FALSE 避免 Sample 误报;
         * 已安装 (1/2) -> 假定通信正常, 轮询失败后重新触发告警 (与 Initialize 一致) */
        SetCommStatus(hSamplerThis, pSamp->dev.nDevId, TRUE);
        pSamp->dev.bCommOK = (nNewType != 0);
        if(nNewType == 0)
        {
            invalidateAirconParams(hSamplerThis, &pSamp->dev);
        }
        TRACE("[aircon] install type -> %d: SetCommStatus(951, TRUE)\n", nNewType);
        return 0;
    }

    if(!pSamp->dev.bCommOK)
    {
        return -1;
    }

    if(emParaType == PARAM_TYPE_SETTING)
    {
        switch(nParamId)
        {

            /* ===== 445 + 2080-2095: write Modbus; return -1 on failure so DAI
             * does NOT persist the value and web UI receives errorCode. ===== */
            case TF_PARAM_AIR_POWER:    /* 445: On/Off and Alarm Reset */
                if(pSamp->dev.nAirType == AIR_TYPE_TF)
                {
                    /* TF 0x300 is a bit-command register (FC 0x05/0x06):
                     *   B0 (0x0001) = 开机命令 (power on)
                     *   B1 (0x0002) = 关机命令 (power off)
                     *   B2 (0x0004) = 报警复位 (alarm reset)
                     * thermal_control pushes 445 = 0 (off) / 2 (cooling) / 4 (heating),
                     * so non-zero => power ON. Working mode (0x1001) is handled by 2080.
                     * Start mode (0x1000) is config-only, default 0=manual; NOT written here. */
                    SHORT nOnOffCmd = (pVarVal->emValue == 0) ? (SHORT)0x0002 : (SHORT)0x0001;
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr,
                                                          MB_TF_AIR_ONOFF_ADDR,
                                                          nOnOffCmd,
                                                          pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                else if(pSamp->dev.nAirType == AIR_TYPE_EU3)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr,
                                                          MB_EU3_AIR_WR_BASE_ADDR + EU3_WR_IDX_CELL_PWR_STATE,
                                                          (SHORT)(long)pVarVal->emValue,
                                                          pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;

            /* ===== TF (Tongfei) aircon setpoints - reg 0x1001..0x1006 ===== */
            case TF_CTRL_WORKING_MODE:    /* 2080: ENUM */
                if(pSamp->dev.nAirType == AIR_TYPE_TF)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr, MB_TF_AIR_WR_BASE_ADDR + TF_WR_IDX_MODE,
                                                       (SHORT)(long)pVarVal->emValue, pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;
            case TF_CTRL_CONTROL_OBJ:     /* 2081: ENUM */
                if(pSamp->dev.nAirType == AIR_TYPE_TF)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr, MB_TF_AIR_WR_BASE_ADDR + TF_WR_IDX_CONTROL_OBJ,
                                                       (SHORT)(long)pVarVal->emValue, pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;
            case TF_CTRL_COLD_TEMP:       /* 2082: INT degC, written raw */
                if(pSamp->dev.nAirType == AIR_TYPE_TF)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr, MB_TF_AIR_WR_BASE_ADDR + TF_WR_IDX_COLD_TEMP,
                                                       (SHORT)(long)pVarVal->emValue, pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;
            case TF_CTRL_COLD_DIFF:       /* 2083: INT degC */
                if(pSamp->dev.nAirType == AIR_TYPE_TF)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr, MB_TF_AIR_WR_BASE_ADDR + TF_WR_IDX_COLD_DIFF,
                                                       (SHORT)(long)pVarVal->emValue, pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;
            case TF_CTRL_HEAT_TEMP:       /* 2084: INT degC */
                if(pSamp->dev.nAirType == AIR_TYPE_TF)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr, MB_TF_AIR_WR_BASE_ADDR + TF_WR_IDX_HEAT_TEMP,
                                                       (SHORT)(long)pVarVal->emValue, pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;
            case TF_CTRL_HEAT_DIFF:       /* 2085: INT degC */
                if(pSamp->dev.nAirType == AIR_TYPE_TF)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr, MB_TF_AIR_WR_BASE_ADDR + TF_WR_IDX_HEAT_DIFF,
                                                       (SHORT)(long)pVarVal->emValue, pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;

            /* ===== EU3 (YFY) aircon setpoints - 501 block ===== */
            case EU3_CTRL_PCS_POWER:      /* 2086: ENUM @501+1 */
                if(pSamp->dev.nAirType == AIR_TYPE_EU3)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr, MB_EU3_AIR_WR_BASE_ADDR + EU3_WR_IDX_PCS_PWR_STATE,
                                                       (SHORT)(long)pVarVal->emValue, pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;
            case EU3_CTRL_WORKING_MODE:   /* 2087: ENUM @501+2 */
                if(pSamp->dev.nAirType == AIR_TYPE_EU3)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr, MB_EU3_AIR_WR_BASE_ADDR + EU3_WR_IDX_MODE,
                                                       (SHORT)(long)pVarVal->emValue, pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;
            case EU3_CTRL_CONTROL_OBJ:    /* 2088: ENUM @501+3 */
                if(pSamp->dev.nAirType == AIR_TYPE_EU3)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr, MB_EU3_AIR_WR_BASE_ADDR + EU3_WR_IDX_CONTROL_OBJ,
                                                       (SHORT)(long)pVarVal->emValue, pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;
            case EU3_CTRL_COLD_TEMP:      /* 2089: FLOAT degC, scale x10 @501+4 */
                if(pSamp->dev.nAirType == AIR_TYPE_EU3)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr, MB_EU3_AIR_WR_BASE_ADDR + EU3_WR_IDX_COLD_TEMP,
                                                       (SHORT)(pVarVal->fValue * 10.0f), pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;
            case EU3_CTRL_HEAT_TEMP:      /* 2090: FLOAT degC, scale x10 @501+5 */
                if(pSamp->dev.nAirType == AIR_TYPE_EU3)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr, MB_EU3_AIR_WR_BASE_ADDR + EU3_WR_IDX_HEAT_TEMP,
                                                       (SHORT)(pVarVal->fValue * 10.0f), pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;
            case EU3_CTRL_NOISE_STATE:    /* 2091: ENUM @501+6 */
                if(pSamp->dev.nAirType == AIR_TYPE_EU3)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr, MB_EU3_AIR_WR_BASE_ADDR + EU3_WR_IDX_NOISE_STATE,
                                                       (SHORT)(long)pVarVal->emValue, pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;

            /* ===== EU3 703 block - signed differential temperatures, scale x10 ===== */
            case EU3_CTRL_COLD_OPEN_DIFF:  /* 2092: FLOAT signed @703+0 */
                if(pSamp->dev.nAirType == AIR_TYPE_EU3)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr, MB_EU3_AIR_WR_BASE1_ADDR + EU3_WR_IDX_COLD_OPEN_DIFF,
                                                       (SHORT)(pVarVal->fValue * 10.0f), pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;
            case EU3_CTRL_COLD_CLOSE_DIFF: /* 2093: FLOAT signed @703+1 */
                if(pSamp->dev.nAirType == AIR_TYPE_EU3)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr, MB_EU3_AIR_WR_BASE1_ADDR + EU3_WR_IDX_COLD_CLOSE_DIFF,
                                                       (SHORT)(pVarVal->fValue * 10.0f), pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;
            case EU3_CTRL_HEAT_OPEN_DIFF:  /* 2094: FLOAT signed @703+2 */
                if(pSamp->dev.nAirType == AIR_TYPE_EU3)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr, MB_EU3_AIR_WR_BASE1_ADDR + EU3_WR_IDX_HEAT_OPEN_DIFF,
                                                       (SHORT)(pVarVal->fValue * 10.0f), pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;
            case EU3_CTRL_HEAT_CLOSE_DIFF: /* 2095: FLOAT signed @703+3 */
                if(pSamp->dev.nAirType == AIR_TYPE_EU3)
                {
                    if(!writeModbusRtuSignalRegister(pSamp->nSlaveAddr, MB_EU3_AIR_WR_BASE1_ADDR + EU3_WR_IDX_HEAT_CLOSE_DIFF,
                                                       (SHORT)(pVarVal->fValue * 10.0f), pSampler->pUsedCommPort, pSampler->hCommPort))
                        return -1;
                }
                return 0;

            case 444:  /* Air-condi mode dif - pure save */
            default:
                return 0;
        }
    }

    /* CONTROLS - no aircon params in ControlParameters, anything arriving here is unknown. */
    (void)pVarVal;
    return -1;
}

/*=================================================================================================
** @Name      : JigTest
** @Function  : 生产测试接口 (空实现)
**===============================================================================================*/
DLLExport int JigTest(HANDLE hSamplerThis, void *pJigData)
{
    (void)hSamplerThis; (void)pJigData;
    return 0;
}
