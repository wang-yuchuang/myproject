#ifndef __AIRCON_SAMPLER_MAIN_H__
#define __AIRCON_SAMPLER_MAIN_H__

/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**---------------------------------------File Information------------------------------------------
** @File Name    : aircon_sampler_main.h
** @Founder      : NJX01203
** @Date         : 2026.07.02
** @Function     : Periodic Modbus RTU sampler for liquid-cooling AC (EU3 + TF variants)
** @Instructions : 参考 0701 备份重写，符合 auxdev_sampler 架构。
**                 绑定 Sampler_ID=11, CommPort_ID=13 (RS485_AIRCON, ttyS9), Device 951 (DevModel 950)。
**                 支持 TF(同飞) + EU3 双协议 Modbus RTU 状态读取与控制下发。
**                 不包含 AC 自动热控（paramId 2110/2401-2406 被 PCS/LVRT 占用）。
**===============================================================================================*/

#include "../sampler_base.h"
#include "../pv_inverter_sampler/pv_modbus_rtu.h"
#include "aircond_regs.h"

/* Maximum registers we expect in a single Modbus read */
#define AIRCON_MAX_REG_READ        32

/* Comm-fault threshold: after N consecutive failures, mark comm lost */
#define AIRCON_COMM_FAULT_LIMIT    4   /* ×8s/block-timeout ≈ 32s comm-loss detection (was 10 ≈ 80s) */
#define AIRCON_COMM_RECOVER_LIMIT   3   /* consecutive good samples before comm-recover (was 5) */

/* CommPort ID for RS485_AIRCON (matches System.sql: Port 13, ttyS9) */
#define AIRCON_COMMPORT_ID         13

/* DB identities (Air Conditioner shares devId=951/modelId=950) */
#define AIRCON_DEV_ID              951
#define AIRCON_DEV_MODEL           950

/*===========================================================================
** TF variant StatusParameter IDs (1770-1810, matches System.sql)
**===========================================================================*/
/* Control readback block @0x1000 len 7 */
#define TF_PARAM_STARTUP_MODE         1770   /* ENUM, raw */
#define TF_PARAM_WORKING_MODE         1771   /* ENUM, raw */
#define TF_PARAM_CONTROL_OBJ          1772   /* ENUM, raw */
#define TF_PARAM_COLD_TEMP_SET        1773   /* FLOAT degC, integer */
#define TF_PARAM_COLD_DIFF            1774   /* FLOAT degC, raw*0.1 */
#define TF_PARAM_HEAT_TEMP_SET        1775   /* FLOAT degC, integer */
#define TF_PARAM_HEAT_DIFF            1776   /* FLOAT degC, raw*0.1 */
/* Status block @0x0000 len 6 */
#define TF_PARAM_UNIT_STATE           1777   /* ENUM, raw */
#define TF_PARAM_UNIT_MODE            1778   /* ENUM, raw */
#define TF_PARAM_LIQUID_SUPPLY_TEMP   1779   /* FLOAT degC, raw*0.1 */
#define TF_PARAM_RETURN_LIQUID_TEMP   1780   /* FLOAT degC, raw*0.1 */
#define TF_PARAM_STEAM_IN_TEMP        1781   /* FLOAT degC, raw*0.1 */
#define TF_PARAM_STEAM_OUT_TEMP       1782   /* FLOAT degC, raw*0.1 */
/* Status block @0x0007 len 3 */
#define TF_PARAM_AMBIENT_TEMP         1783   /* FLOAT degC, raw*0.1 */
#define TF_PARAM_CONDENSATION_TEMP    1784   /* FLOAT degC, raw*0.1 */
#define TF_PARAM_EEV_OPENING          1785   /* UINT Step, raw */
/* Status block @0x000B len 4 */
#define TF_PARAM_PUMP_SPEED           1786   /* UINT %, raw */
#define TF_PARAM_COMPRESSOR_SPEED     1787   /* UINT RPS, raw */
#define TF_PARAM_FAN_SPEED            1788   /* UINT RPS, raw */
#define TF_PARAM_WATER_SUPPLY_PRES    1789   /* FLOAT bar, raw*0.1 */
/* Status block @0x0012 len 2 */
#define TF_PARAM_RETURN_WATER_PRES    1790   /* FLOAT bar, raw*0.1 */
#define TF_PARAM_EXHAUST_PRES         1791   /* FLOAT bar, raw*0.1 */
/* Status block @0x0015 len 1 */
#define TF_PARAM_SYSTEM_HIGH_VOLT     1792   /* ENUM, raw */
/* Status block @0x0019 len 8 */
#define TF_PARAM_SYSTEM_LOW_VOLT      1793   /* ENUM, raw */
#define TF_PARAM_LIQUID_LEVEL_SW      1794   /* ENUM, raw */
#define TF_PARAM_PUMP_OVERLOAD        1795   /* ENUM, raw */
#define TF_PARAM_HEAT_TUBE_OVERLOAD   1796   /* ENUM, raw */
#define TF_PARAM_FAN_OVERLOAD         1797   /* ENUM, raw */
#define TF_PARAM_RELAY_OUTPUT         1798   /* ENUM, raw */
#define TF_PARAM_HEAT_TUBE            1799   /* ENUM, raw */
#define TF_PARAM_ALARM_RELAY          1800   /* ENUM, raw */
/* Status block @0x0022 len 5 */
#define TF_PARAM_HEATING_STRIP        1801   /* ENUM, raw */
#define TF_PARAM_PUMP_MALFUNCTION     1802   /* ENUM, raw */
#define TF_PARAM_PUMP_IDLE_PROTECT    1803   /* ENUM, raw */
#define TF_PARAM_PUMP_BLOCKAGE        1804   /* ENUM, raw */
#define TF_PARAM_PUMP_OVERCURRENT     1805   /* ENUM, raw */
/* Derived/extra fault flags (1806-1810) */
#define TF_PARAM_PUMP_UNDER_PRES      1806   /* ENUM, raw */
#define TF_PARAM_PUMP_OVER_PRES       1807   /* ENUM, raw */
#define TF_PARAM_PUMP_PHASE_LOSS      1808   /* ENUM, raw */
#define TF_PARAM_PUMP_DRIVE_OVERHEAT  1809   /* ENUM, raw */
#define TF_PARAM_WATER_PRES_SENSOR    1810   /* ENUM, raw */

/*===========================================================================
** TF fault-bit StatusParameter IDs (1811-1896, matches System.sql)
** Decoded from input regs 0x0019~0x0023 (TF/Tongfei fault bitmaps).
** Bits not present in the Modbus map (display/slave comm, fan-inverter faults)
** are declared here but pushed as 0 (no register source).
**===========================================================================*/
/* FaultStatus1 @0x0019 bit9-13 -> 1811-1815 */
#define TF_PARAM_MB_IIC_FAULT          1811   /* ENUM, raw, 0x0019 bit9   主板IIC故障 */
#define TF_PARAM_RETURN_WATER_LOW      1812   /* ENUM, raw, 0x0019 bit10  回水压力低故障 */
#define TF_PARAM_WATER_SUPPLY_HIGH     1813   /* ENUM, raw, 0x0019 bit11  供水压力过高 */
#define TF_PARAM_VOLT_HIGH             1814   /* ENUM, raw, 0x0019 bit12  电压过高 */
#define TF_PARAM_VOLT_LOW              1815   /* ENUM, raw, 0x0019 bit13  电压过低 */
/* FaultStatus2 @0x001A bit0-7 -> 1816-1824 (1823 from 0x001B bit5) */
#define TF_PARAM_SYS_LOW_PRES          1816   /* ENUM, raw, 0x001A bit0   系统低压 */
#define TF_PARAM_ELEC_HEAT_PROTECT     1817   /* ENUM, raw, 0x001A bit1   电加热保护 */
#define TF_PARAM_FAN_OVERLOAD_FAULT    1818   /* ENUM, raw, 0x001A bit2   风机过载 */
#define TF_PARAM_SYS_HIGH_PRES_FAULT   1819   /* ENUM, raw, 0x001A bit3   系统高压故障 */
#define TF_PARAM_EVAP_IN_PROBE         1820   /* ENUM, raw, 0x001A bit4   蒸发进口探头故障 */
#define TF_PARAM_EVAP_OUT_PROBE        1821   /* ENUM, raw, 0x001A bit5   蒸发出口探头故障 */
#define TF_PARAM_SYS_HIGH_PRES_LOCK    1822   /* ENUM, raw, 0x001A bit6   系统高压报警锁定 */
#define TF_PARAM_EXH_PRES_SENSOR       1823   /* ENUM, raw, 0x001B bit5   排气压力传感器故障 */
#define TF_PARAM_COMP_INV_COMM_FAULT   1824   /* ENUM, raw, 0x001A bit7   压缩机变频器通信故障 */
/* FaultStatus3 @0x001B bit0-8 -> 1825-1833 (1830/1834 no source) */
#define TF_PARAM_LIQUID_TEMP_PROBE     1825   /* ENUM, raw, 0x001B bit0   液温探头故障 */
#define TF_PARAM_RETURN_LIQ_PROBE      1826   /* ENUM, raw, 0x001B bit1   回液探头故障 */
#define TF_PARAM_AMBIENT_PROBE         1827   /* ENUM, raw, 0x001B bit2   环境探头故障 */
#define TF_PARAM_LIQUID_TEMP_HIGH      1828   /* ENUM, raw, 0x001B bit3   液温过高 */
#define TF_PARAM_RETURN_WATER_SENSOR   1829   /* ENUM, raw, 0x001B bit4   回水压力传感器故障 */
#define TF_PARAM_DISPLAY_COMM_TIMEOUT  1830   /* ENUM, raw, no source    屏幕与主板通信超时 */
#define TF_PARAM_LIQUID_TEMP_LOW       1831   /* ENUM, raw, 0x001B bit6   液温过低 */
#define TF_PARAM_WATER_SUPPLY_HIGH_WARN 1832  /* ENUM, raw, 0x001B bit7   供水压力过高预警 */
#define TF_PARAM_POWER_DROP_ALARM      1833   /* ENUM, raw, 0x001B bit8   掉电报警 */
#define TF_PARAM_SLAVE_COMM_FAULT      1834   /* ENUM, raw, no source    从板通讯故障 */
/* FaultStatus4 (compressor inverter) @0x001C bit0-15 -> 1835-1850 */
#define TF_PARAM_COMP_START_OVERCURR   1835   /* ENUM, raw, 0x001C bit0   压机启动瞬间过流 */
#define TF_PARAM_COMP_ACC_OVERCURR     1836   /* ENUM, raw, 0x001C bit1   压机加速运行过流 */
#define TF_PARAM_COMP_DEC_OVERCURR     1837   /* ENUM, raw, 0x001C bit2   压机减速运行过流 */
#define TF_PARAM_COMP_STEADY_OVERCURR  1838   /* ENUM, raw, 0x001C bit3   压机恒速运行过流 */
#define TF_PARAM_COMP_ACC_OVERVOLT     1839   /* ENUM, raw, 0x001C bit4   压机加速运行过压 */
#define TF_PARAM_COMP_DEC_OVERVOLT     1840   /* ENUM, raw, 0x001C bit5   压机减速运行过压 */
#define TF_PARAM_COMP_STEADY_OVERVOLT  1841   /* ENUM, raw, 0x001C bit6   压机恒速运行过压 */
#define TF_PARAM_COMP_STANDBY_OVERVOLT 1842   /* ENUM, raw, 0x001C bit7   压机待机时过压 */
#define TF_PARAM_COMP_RUN_UNDERVOLT    1843   /* ENUM, raw, 0x001C bit8   压机运行中欠压 */
#define TF_PARAM_COMP_OUTPUT_PHASE_LOSS 1844  /* ENUM, raw, 0x001C bit9   压机输出缺相 */
#define TF_PARAM_COMP_POWER_DEV_PROTECT 1845  /* ENUM, raw, 0x001C bit10  压机功率器件保护 */
#define TF_PARAM_COMP_OVERHEAT         1846   /* ENUM, raw, 0x001C bit11  压机过热 */
#define TF_PARAM_COMP_OVERLOAD         1847   /* ENUM, raw, 0x001C bit12  压机过载 */
#define TF_PARAM_COMP_DETECT_OVERLOAD  1848   /* ENUM, raw, 0x001C bit13  压机检测压机过载 */
#define TF_PARAM_COMP_LOAD_TOO_HIGH    1849   /* ENUM, raw, 0x001C bit14  压机负载过重 */
#define TF_PARAM_COMP_SPEED_TOO_HIGH   1850   /* ENUM, raw, 0x001C bit15  压机速度过大 */
/* FaultStatus5 (compressor inverter) @0x001D bit0-14 -> 1851-1865 */
#define TF_PARAM_COMP_D_AXIS_OVERCURR  1851   /* ENUM, raw, 0x001D bit0   压机D轴电流过大 */
#define TF_PARAM_COMP_Q_AXIS_OVERCURR  1852   /* ENUM, raw, 0x001D bit1   压机Q轴电流过大 */
#define TF_PARAM_COMP_PARAM_STORE_FAIL 1853   /* ENUM, raw, 0x001D bit2   压机参数存储失败 */
#define TF_PARAM_COMP_COMM_FAULT       1854   /* ENUM, raw, 0x001D bit3   压机通信故障 */
#define TF_PARAM_COMP_CURR_DETECT_FAULT 1855  /* ENUM, raw, 0x001D bit4   压机电流检测故障 */
#define TF_PARAM_COMP_START_STALL      1856   /* ENUM, raw, 0x001D bit5   压机启动中电机堵转 */
#define TF_PARAM_COMP_RUN_STALL        1857   /* ENUM, raw, 0x001D bit6   压机运行中电机堵转 */
#define TF_PARAM_COMP_LOST_SPEED       1858   /* ENUM, raw, 0x001D bit7   压机失速故障 */
#define TF_PARAM_COMP_INT_OVF1         1859   /* ENUM, raw, 0x001D bit8   压机中断溢出1 */
#define TF_PARAM_COMP_INT_OVF2         1860   /* ENUM, raw, 0x001D bit9   压机中断溢出2 */
#define TF_PARAM_COMP_START_JITTER     1861   /* ENUM, raw, 0x001D bit10  压机启动中转子抖动过大 */
#define TF_PARAM_COMP_RUN_JITTER       1862   /* ENUM, raw, 0x001D bit11  压机运行中转子抖动过大 */
#define TF_PARAM_PFC_OVERCURR          1863   /* ENUM, raw, 0x001D bit12  PFC过流 */
#define TF_PARAM_PFC_PEAK_OVERCURR     1864   /* ENUM, raw, 0x001D bit13  PFC峰值电流过大 */
#define TF_PARAM_PFC_RMS_OVERCURR      1865   /* ENUM, raw, 0x001D bit14  PFC有效值电流过大 */
/* Scalar fault codes (not bit fields) */
#define TF_PARAM_INV_FAULT_CODE        1866   /* UINT, raw, 0x0021       变频器故障代码(压机) */
#define TF_PARAM_SYS_TOTAL_FAULT_LEVEL 1867   /* UINT, raw, 0x0022       系统总故障等级 */
#define TF_PARAM_SYS_TOTAL_FAULT_CODE  1868   /* UINT, raw, 0x0023       系统总故障代码 */
/* Fan-inverter faults 1869-1896: no register source in 0x0019~0x0023, push 0 */
#define TF_PARAM_FAN_START_OVERCURR    1869   /* ENUM, raw, no source    风机启动瞬间过流 */
#define TF_PARAM_FAN_ACC_OVERCURR      1870   /* ENUM, raw, no source    风机加速运行过流 */
#define TF_PARAM_FAN_DEC_OVERCURR      1871   /* ENUM, raw, no source    风机减速运行过流 */
#define TF_PARAM_FAN_STEADY_OVERCURR   1872   /* ENUM, raw, no source    风机恒速运行过流 */
#define TF_PARAM_FAN_ACC_OVERVOLT      1873   /* ENUM, raw, no source    风机加速运行过压 */
#define TF_PARAM_FAN_DEC_OVERVOLT      1874   /* ENUM, raw, no source    风机减速运行过压 */
#define TF_PARAM_FAN_STEADY_OVERVOLT   1875   /* ENUM, raw, no source    风机恒速运行过压 */
#define TF_PARAM_FAN_STANDBY_OVERVOLT  1876   /* ENUM, raw, no source    风机待机时过压 */
#define TF_PARAM_FAN_RUN_UNDERVOLT     1877   /* ENUM, raw, no source    风机运行中欠压 */
#define TF_PARAM_FAN_INPUT_PHASE_LOSS  1878   /* ENUM, raw, no source    风机输入缺相 */
#define TF_PARAM_FAN_OUTPUT_PHASE_LOSS 1879   /* ENUM, raw, no source    风机输出缺相 */
#define TF_PARAM_FAN_POWER_DEV_PROTECT 1880   /* ENUM, raw, no source    风机功率器件保护 */
#define TF_PARAM_FAN_OVERHEAT          1881   /* ENUM, raw, no source    风机过热 */
#define TF_PARAM_INV_OVERLOAD          1882   /* ENUM, raw, no source    变频器过载 */
#define TF_PARAM_INV_FAN_OVERLOAD      1883   /* ENUM, raw, no source    变频风机过载 */
#define TF_PARAM_EXTERNAL_FAULT        1884   /* ENUM, raw, no source    外部故障 */
#define TF_PARAM_MOTOR_OVERLOAD        1885   /* ENUM, raw, no source    电机负载过重 */
#define TF_PARAM_INV_UNDERLOAD         1886   /* ENUM, raw, no source    变频器欠载 */
#define TF_PARAM_FAN_FAULT             1887   /* ENUM, raw, no source    风扇故障 */
#define TF_PARAM_USAGE_TERM_EXPIRED    1888   /* ENUM, raw, no source    用户使用期限禁止时间已到 */
#define TF_PARAM_FAN_PARAM_STORE_FAIL  1889   /* ENUM, raw, no source    参数存储失败 */
#define TF_PARAM_FAN_COMM_FAULT        1890   /* ENUM, raw, no source    通讯异常 */
#define TF_PARAM_FAN_CURR_DETECT_FAULT 1891   /* ENUM, raw, no source    电流检测故障 */
#define TF_PARAM_FAN_AUTOTUNE_FAULT    1892   /* ENUM, raw, no source    自整定不良 */
#define TF_PARAM_FAN_ANALOG_IN_DROP    1893   /* ENUM, raw, no source    模拟输入掉线 */
#define TF_PARAM_FAN_PG_DROP           1894   /* ENUM, raw, no source    PG断线 */
#define TF_PARAM_FAN_THERMISTOR_OPEN   1895   /* ENUM, raw, no source    热敏电阻开路 */
#define TF_PARAM_FAN_ABNORMAL_STOP     1896   /* ENUM, raw, no source    异常停机故障 */
/* New fault-bit params 3730-3735: sourced from tfSta5 registers 0x0019/0x001A */
#define TF_PARAM_FAN_DRIVE_OVERCURR   3730   /* ENUM, raw, 0x0019 bit14  风机驱动电流过载 */
#define TF_PARAM_DIFF_PRESS_ULTRALOW  3731   /* ENUM, raw, 0x0019 bit15  供回水压差超低报警 */
#define TF_PARAM_FAN_INV_COMM_FAULT   3732   /* ENUM, raw, 0x001A bit8   风机变频器通讯故障 */
#define TF_PARAM_COMP_INV_OTHER_FAULT 3733   /* ENUM, raw, 0x001A bit9   压机变频器其他故障 */
#define TF_PARAM_FAN_INV_OTHER_FAULT  3734   /* ENUM, raw, 0x001A bit10  风机变频器其他故障 */
#define TF_PARAM_DIFF_PRESS_LOW       3735   /* ENUM, raw, 0x001A bit11  供回水压差低报警 */

/*===========================================================================
** EU3 variant StatusParameter IDs (1521-1570, matches System.sql)
**===========================================================================*/
/* Control readback @501 len 7 + @703 len 4 */
#define EU3_PARAM_WORKING_MODE        1521   /* ENUM, raw (reg 501+2) */
#define EU3_PARAM_COLD_TEMP_SET       1522   /* FLOAT degC, reg 501+4 *0.1 */
#define EU3_PARAM_COLD_START_DIFF     1523   /* UINT degC, reg 703+0 *0.1 */
#define EU3_PARAM_COLD_STOP_DIFF      1524   /* UINT degC, reg 703+1 *0.1 */
#define EU3_PARAM_HEAT_TEMP_SET       1525   /* FLOAT degC, reg 501+5 *0.1 */
#define EU3_PARAM_HEAT_TEMP_DIFF      1526   /* FLOAT degC, reg 703+2 *0.1 */
#define EU3_PARAM_REFRIGERANT_TYPE    1527   /* ENUM, push 0 (config) */
#define EU3_PARAM_HOST_BAUD           1528   /* ENUM, push 0 (config) */
#define EU3_PARAM_HOST_MODBUS_ADDR    1529   /* UINT, push 0 (config) */
#define EU3_PARAM_OUTDOOR_SPEC        1530   /* ENUM, push 0 (config) */
/* Status @499 len 12 + @513 len 4 + @600 len 11 + @612 len 6 */
/* Status: @499(SWver/states), @513(compressor freq/power), @600(NTC temps), @612(pressures) */
#define EU3_PARAM_EEV_OPENING         1531   /* UINT, reg 508 EEV steps */
#define EU3_PARAM_COMP_TARGET_FREQ    1532   /* FLOAT Hz, reg 513 target */
#define EU3_PARAM_COMP_OPER_FREQ      1533   /* FLOAT Hz, reg 514 actual */
#define EU3_PARAM_FAN_TARGET_SPEED    1534   /* UINT %, not in current read */
#define EU3_PARAM_EXH_GAS_SUPERHEAT   1535   /* FLOAT degC, derived */
#define EU3_PARAM_EXT_LOOP_TEMP       1536   /* FLOAT degC, reg 602 cooling water inlet *0.1 */
#define EU3_PARAM_EXH_GAS_TEMP        1537   /* FLOAT degC, reg 600 排气温度 *0.1 */
#define EU3_PARAM_SUCCTION_TEMP       1538   /* FLOAT degC, reg 601 吸气温度 *0.1 */
#define EU3_PARAM_MOD_INLET_WATER     1539   /* FLOAT degC, reg 604 模块侧回水(进水) *0.1 */
#define EU3_PARAM_MOD_OUTLET_WATER    1540   /* FLOAT degC, reg 603 模块侧供水(出水) *0.1 */
#define EU3_PARAM_BATT_INLET_WATER    1541   /* FLOAT degC, reg 605 电池侧回水(进水) *0.1 */
#define EU3_PARAM_BATT_OUTLET_WATER   1542   /* FLOAT degC, reg 606 电池侧供水(出水) *0.1 */
#define EU3_PARAM_PUMP_ACTUAL_SPEED   1543   /* UINT %, not in current read */
#define EU3_PARAM_MOD_OUT_WATER_PRES  1544   /* FLOAT bar, reg 612 高压(排气) *0.1 */
#define EU3_PARAM_BATT_OUT_WATER_PRES 1545   /* FLOAT bar, reg 613 低压(吸气) *0.1 */
#define EU3_PARAM_HP_SATURATION_TEMP  1546   /* FLOAT degC, derived */
#define EU3_PARAM_LP_SATURATION_TEMP  1547   /* FLOAT degC, derived */
#define EU3_PARAM_SUC_SUPERHEAT_TEMP  1548   /* FLOAT degC, derived */
#define EU3_PARAM_LOW_PRESSURE        1549   /* FLOAT bar, reg 613 *0.1 */
#define EU3_PARAM_HIGH_PRESSURE       1550   /* FLOAT bar, reg 612 *0.1 */
#define EU3_PARAM_AC_INPUT_VOLT       1551   /* FLOAT V, not in current read */
#define EU3_PARAM_AC_INPUT_CURR       1552   /* FLOAT A, not in current read */
#define EU3_PARAM_COMP_PHASE_CURR     1553   /* FLOAT A, not in current read */
#define EU3_PARAM_DC_BUS_VOLT         1554   /* UINT V, not in current read */
#define EU3_PARAM_ELEC_HEAT_STATUS    1555   /* ENUM, reg 500 实际开机状态 */
#define EU3_PARAM_WATER_PUMP_STATUS   1556   /* ENUM, reg 501 实际运行模式 */
#define EU3_PARAM_CONDENSER_FAN       1557   /* ENUM, reg 502 系统运行阶段 */
#define EU3_PARAM_COMP_STATUS         1558   /* ENUM, reg 503 故障ASCII码 */
#define EU3_PARAM_CRANK_HEATER        1559   /* ENUM, not available */
#define EU3_PARAM_OPERATING_STATUS    1560   /* ENUM, derived */
#define EU3_PARAM_OIL_RETURN          1561   /* ENUM, push 0 */
#define EU3_PARAM_SW_VERSION          1562   /* UINT, reg 499 SW version */
#define EU3_PARAM_TOTAL_FAULT         1563   /* ENUM, reg 538 故障等级 (液路故障状态) */
#define EU3_PARAM_BATT_OUT_PRES_HI    1564   /* ENUM, derived from reg 705 */
#define EU3_PARAM_MOD_OUT_PRES_HI     1565   /* ENUM, derived */
#define EU3_PARAM_PTC_OVERLOAD        1566   /* ENUM, derived */
#define EU3_PARAM_RETURN_WATER_HI_LO  1567   /* ENUM, derived */
#define EU3_PARAM_OUTLET_TEMP_HI_LO   1568   /* ENUM, derived */
#define EU3_PARAM_FAN1_MALFUNCTION    1569   /* ENUM, reg 702 bit0 DI1 fan1 */
#define EU3_PARAM_FAN2_MALFUNCTION    1570   /* ENUM, reg 702 bit1 DI2 fan2 */

/*===========================================================================
** Legacy EU3 fault params (1571-1601, matches System.cfg alarm conditions)
** Originally pushed by the EU3 core (CAN) which aggregated aircon faults. The
** aircon is now direct RS485 (EU3 V3.00 Modbus); 21 of these map 1:1 to the
** 700~706 fault-bit params (3514-3641) above, 10 were EU3-core-only and no
** longer exist in V3.00 -> pushed 0 in pushEU3AirconParams.
**===========================================================================*/
#define EU3_PARAM_LEGACY_FAN3                     1571   /* ENUM, reg702 bit2  DI3 风机3 */
#define EU3_PARAM_LEGACY_FAN4                     1572   /* ENUM, reg702 bit3  DI4 风机4 */
#define EU3_PARAM_LEGACY_SYS1_ALARM_STATUS        1573   /* ENUM, push 0 (EU3 core only) */
#define EU3_PARAM_LEGACY_SYS1_LOCK_STATUS         1574   /* ENUM, push 0 (EU3 core only) */
#define EU3_PARAM_LEGACY_SUBCOOLING_TEMP_SENSOR   1575   /* ENUM, push 0 (no NTC in V3.00) */
#define EU3_PARAM_LEGACY_AMBIENT_TEMP_SENSOR      1576   /* ENUM, reg700 bit9  NTC10 环境 */
#define EU3_PARAM_LEGACY_EXHAUST_TEMP_SENSOR      1577   /* ENUM, reg700 bit0  NTC1 排气 */
#define EU3_PARAM_LEGACY_SUCTION_TEMP_SENSOR      1578   /* ENUM, reg700 bit1  NTC2 吸气 */
#define EU3_PARAM_LEGACY_MODULE_INLET_TEMP_SENSOR 1579   /* ENUM, reg700 bit4  NTC5 模块进水 */
#define EU3_PARAM_LEGACY_MODULE_OUTLET_TEMP_SENSOR 1580  /* ENUM, reg700 bit3  NTC4 模块出水 */
#define EU3_PARAM_LEGACY_FAN2                     1581   /* ENUM, reg702 bit1  DI2 风机2 */
#define EU3_PARAM_LEGACY_BATT_INLET_TEMP_SENSOR   1582   /* ENUM, reg700 bit5  NTC6 电池进水 */
#define EU3_PARAM_LEGACY_BATT_OUTLET_TEMP_SENSOR  1583   /* ENUM, reg700 bit6  NTC7 电池出水 */
#define EU3_PARAM_LEGACY_LOW_SUPERHEAT_ALARM      1584   /* ENUM, reg705 bit4  D4 过热度低 */
#define EU3_PARAM_LEGACY_HIGH_PRESSURE_SENSOR     1585   /* ENUM, reg700 bit12 PS1 排气压 */
#define EU3_PARAM_LEGACY_LOW_PRESSURE_SENSOR      1586   /* ENUM, reg700 bit13 PS2 吸气压 */
#define EU3_PARAM_LEGACY_DRIVER_COMM_FAULT        1587   /* ENUM, reg703 bit0  C0 压缩机通信 */
#define EU3_PARAM_LEGACY_WATER_TANK_SHORTAGE      1588   /* ENUM, reg705 bit7  D7 水箱缺液告警 */
#define EU3_PARAM_LEGACY_MODULE_SHORTAGE          1589   /* ENUM, reg705 bit8  D8 模块缺液 */
#define EU3_PARAM_LEGACY_FAN_ALARM_FAULT          1590   /* ENUM, reg702 bit15 B0 风机全故障 */
#define EU3_PARAM_LEGACY_HIGH_PRESSURE_PROTECT    1591   /* ENUM, reg705 bit5  D5 高压保护 */
#define EU3_PARAM_LEGACY_LOW_PRESSURE_PROTECT     1592   /* ENUM, reg705 bit6  D6 低压保护 */
#define EU3_PARAM_LEGACY_BATT_SHORTAGE            1593   /* ENUM, reg705 bit9  D9 电池缺液 */
#define EU3_PARAM_LEGACY_WINTER_ANTI_FREEZE_1     1594   /* ENUM, push 0 (no bit in V3.00) */
#define EU3_PARAM_LEGACY_WINTER_ANTI_FREEZE_2     1595   /* ENUM, push 0 (no bit in V3.00) */
#define EU3_PARAM_LEGACY_WATER_TEMP_ABNORMAL      1596   /* ENUM, push 0 (EU3 core only) */
#define EU3_PARAM_LEGACY_EXHAUST_OVER_TEMP        1597   /* ENUM, reg705 bit13 DD 排气高温 */
#define EU3_PARAM_LEGACY_COOLING_ANTI_FREEZE      1598   /* ENUM, push 0 (no bit in V3.00) */
#define EU3_PARAM_LEGACY_HY1_PUMP_FAULT           1599   /* ENUM, push 0 (EU3 core only) */
#define EU3_PARAM_LEGACY_HY3_PUMP_FAULT           1600   /* ENUM, push 0 (EU3 core only) */
#define EU3_PARAM_LEGACY_DC_FAN1_FAULT            1601   /* ENUM, push 0 (EU3 core only) */

/*===========================================================================
** EU3 fault-bit StatusParameter IDs (3514-3641, matches System.sql)
** Decoded from input regs 538 + 700~706 (EU3 V3.00 fault bitmaps).
**===========================================================================*/
/* Fluid-path fault status @538 (multi-value field: 0=OK, 1=alarm, 2=stop) */
#define EU3_PARAM_FLUID_BATT_OK        3514   /* ENUM, reg538 bit0-4 == 0 电池液路1无故障 */
#define EU3_PARAM_FLUID_BATT_ALARM     3515   /* ENUM, reg538 bit0-4 == 1 电池液路1告警 */
#define EU3_PARAM_FLUID_BATT_STOP_B0   3516   /* ENUM, reg538 bit0-4 == 2 电池液路1停机B0 */
#define EU3_PARAM_FLUID_BATT_STOP_B1   3517   /* ENUM, reg538 bit0-4 == 3 电池液路1停机B1 */
#define EU3_PARAM_FLUID_BATT_RSV       3518   /* ENUM, reg538 bit0-4 >= 4 电池液路1预留 */
#define EU3_PARAM_FLUID_MOD_OK         3519   /* ENUM, reg538 bit5-7 == 0 模块液路2正常 */
#define EU3_PARAM_FLUID_MOD_ALARM      3520   /* ENUM, reg538 bit5-7 == 1 模块液路2告警 */
#define EU3_PARAM_FLUID_MOD_STOP       3521   /* ENUM, reg538 bit5-7 == 2 模块液路2停机 */

/* Sensor faults @700 (bit0-15) + @701 (bit0-1) */
#define EU3_PARAM_NTC1_FAULT           3546   /* reg700 bit0  NTC1 排气 */
#define EU3_PARAM_NTC2_FAULT           3547   /* reg700 bit1  NTC2 吸气 */
#define EU3_PARAM_NTC3_FAULT           3548   /* reg700 bit2  NTC3 冷却水进水 */
#define EU3_PARAM_NTC4_FAULT           3549   /* reg700 bit3  NTC4 模块出水 */
#define EU3_PARAM_NTC5_FAULT           3550   /* reg700 bit4  NTC5 模块进水 */
#define EU3_PARAM_NTC6_FAULT           3551   /* reg700 bit5  NTC6 电池进水 */
#define EU3_PARAM_NTC7_FAULT           3552   /* reg700 bit6  NTC7 电池出水 */
#define EU3_PARAM_NTC8_FAULT           3553   /* reg700 bit7  NTC8 冷却水模块侧出水 */
#define EU3_PARAM_NTC9_FAULT           3554   /* reg700 bit8  NTC9 冷却水电池侧出水 */
#define EU3_PARAM_NTC10_FAULT          3555   /* reg700 bit9  NTC10 环境 */
#define EU3_PARAM_NTC11_FAULT          3556   /* reg700 bit10 NTC11 预留 */
#define EU3_PARAM_NTC12_FAULT          3557   /* reg700 bit11 NTC12 预留 */
#define EU3_PARAM_PS1_FAULT            3558   /* reg700 bit12 PS1 排气压 */
#define EU3_PARAM_PS2_FAULT            3559   /* reg700 bit13 PS2 吸气压 */
#define EU3_PARAM_PS3_FAULT            3560   /* reg700 bit14 PS3 电池出压 */
#define EU3_PARAM_PS4_FAULT            3561   /* reg700 bit15 PS4 模块出压 */
#define EU3_PARAM_PS5_FAULT            3562   /* reg701 bit0  PS5 进水压 */
#define EU3_PARAM_PS6_FAULT            3563   /* reg701 bit1  PS6 出水压 */

/* Component/DI faults @702 (bit0-15) */
#define EU3_PARAM_DI1_FAULT            3578   /* reg702 bit0  DI1 风机1 */
#define EU3_PARAM_DI2_FAULT            3579   /* reg702 bit1  DI2 风机2 */
#define EU3_PARAM_DI3_FAULT            3580   /* reg702 bit2  DI3 风机3 */
#define EU3_PARAM_DI4_FAULT            3581   /* reg702 bit3  DI4 风机4 */
#define EU3_PARAM_DI5_FAULT            3582   /* reg702 bit4  DI5 电热 */
#define EU3_PARAM_DI6_FAULT            3583   /* reg702 bit5  DI6 水箱缺液 */
#define EU3_PARAM_DI7_FAULT            3584   /* reg702 bit6  DI7 预留 */
#define EU3_PARAM_DI8_FAULT            3585   /* reg702 bit7  DI8 预留 */
#define EU3_PARAM_DI17_FAULT           3586   /* reg702 bit8  DI17 电池泵 */
#define EU3_PARAM_DI18_FAULT           3587   /* reg702 bit9  DI18 模块泵 */
#define EU3_PARAM_DI_B10_RSV           3588   /* reg702 bit10 预留 */
#define EU3_PARAM_DI_B11_RSV           3589   /* reg702 bit11 预留 */
#define EU3_PARAM_DI_B12_RSV           3590   /* reg702 bit12 预留 */
#define EU3_PARAM_DI_B13_RSV           3591   /* reg702 bit13 预留 */
#define EU3_PARAM_DI_B14_RSV           3592   /* reg702 bit14 预留 */
#define EU3_PARAM_FAN_ALL_FAULT        3593   /* reg702 bit15 风机全故障 */

/* Compressor faults @703 (bit0-15) + @704 (bit0-2) */
#define EU3_PARAM_COMP_COMM_FAULT      3594   /* reg703 bit0  C0 压缩机通信 */
#define EU3_PARAM_AC_OVERCURR          3595   /* reg703 bit1  C1 AC过流 */
#define EU3_PARAM_AC_INST_OVERCURR     3596   /* reg703 bit2  C2 AC瞬时过流 */
#define EU3_PARAM_PFC_OVERVOLT         3597   /* reg703 bit3  C3 PFC过压 */
#define EU3_PARAM_COMP_INV_FAULT       3598   /* reg703 bit4  C4 压机逆变 */
#define EU3_PARAM_COMP_INST_OVERCURR   3599   /* reg703 bit5  C5 压机瞬时过流 */
#define EU3_PARAM_COMP_LOST_SYNC       3600   /* reg703 bit6  C6 压机失步 */
#define EU3_PARAM_COMP_PHASE_LOSS      3601   /* reg703 bit7  C7 压机缺相 */
#define EU3_PARAM_INV_PCB_OVERHEAT     3602   /* reg703 bit8  C8 逆变/PCB过温 */
#define EU3_PARAM_IGBT_OVERHEAT        3603   /* reg703 bit9  C9 IGBT过温 */
#define EU3_PARAM_POWER_OVERLOAD       3604   /* reg703 bit10 CA 功率过载 */
#define EU3_PARAM_PFC_OVERCURR         3605   /* reg703 bit11 CB PFC过流 */
#define EU3_PARAM_AC_OVERVOLT          3606   /* reg703 bit12 CC AC过压 */
#define EU3_PARAM_DC_OVERVOLT          3607   /* reg703 bit13 CD DC过压 */
#define EU3_PARAM_COMP_PHASE_OVERCURR  3608   /* reg703 bit14 CE 压机相电流过流 */
#define EU3_PARAM_AC_UNDERVOLT         3609   /* reg703 bit15 CF AC欠压 */
#define EU3_PARAM_DC_UNDERVOLT         3610   /* reg704 bit0  CG DC欠压 */
#define EU3_PARAM_AC_PHASE_LOSS        3611   /* reg704 bit1  CH 交流缺相 */
#define EU3_PARAM_COMP_TOTAL_FAULT     3612   /* reg704 bit2  CI 压缩机总故障 */

/* System-state warning faults @705 (bit0-15) */
#define EU3_PARAM_WARN_BATT_OUT_PRES_HI   3626   /* reg705 bit0  D0 电池出水压力过高 */
#define EU3_PARAM_WARN_MOD_OUT_PRES_HI    3627   /* reg705 bit1  D1 模块出水压力过高 */
#define EU3_PARAM_WARN_BATT_WATER_TEMP    3628   /* reg705 bit2  D2 电池水温异常 */
#define EU3_PARAM_WARN_MOD_WATER_TEMP     3629   /* reg705 bit3  D3 模块水温异常 */
#define EU3_PARAM_WARN_LOW_SUPERHEAT      3630   /* reg705 bit4  D4 过热度低 */
#define EU3_PARAM_WARN_HP_PROTECT         3631   /* reg705 bit5  D5 高压保护 */
#define EU3_PARAM_WARN_LP_PROTECT         3632   /* reg705 bit6  D6 低压保护 */
#define EU3_PARAM_WARN_TANK_SHORTAGE      3633   /* reg705 bit7  D7 水箱缺液 */
#define EU3_PARAM_WARN_MOD_SHORTAGE       3634   /* reg705 bit8  D8 模块缺液 */
#define EU3_PARAM_WARN_BATT_SHORTAGE      3635   /* reg705 bit9  D9 电池缺液 */
#define EU3_PARAM_WARN_BATT_TEMP_DIFF     3636   /* reg705 bit10 DA 电池水温差 */
#define EU3_PARAM_WARN_MOD_TEMP_DIFF      3637   /* reg705 bit11 DB 模块水温差 */
#define EU3_PARAM_WARN_COOL_FLOW          3638   /* reg705 bit12 DC 冷却水流 */
#define EU3_PARAM_WARN_EXH_HIGH           3639   /* reg705 bit13 DD 排气高温 */
#define EU3_PARAM_WARN_COOL_TEMP          3640   /* reg705 bit14 DE 冷却水温异常 */
#define EU3_PARAM_WARN_BATT_DIFF_PROTECT  3641   /* reg705 bit15 DF 电池侧进出水温差保护 */
#define EU3_PARAM_NOISE_STATE_RD      3728   /* ENUM, reg 501+6 readback (matches System.cfg 'Noise Reduction Mode') */

/* TF SettingParameter IDs
 * 445      - Air Conditioner On/Off and Alarm Reset.
 * 2080-2095 - TF/EU3 aircon setpoints. SETTING branch dispatches the Modbus write;
 *             UI persists value via SETTYPE_RAW_BACK_SETT.
 */
#define TF_PARAM_AIR_POWER            445    /* Air Conditioner On/Off and Alarm Reset */

/* TF (Tongfei) aircon setpoint ControlParameter IDs - written to reg 0x1001..0x1006 (FC 0x06/0x10) */
#define TF_CTRL_WORKING_MODE          2080   /* ENUM, reg 0x1001 */
#define TF_CTRL_CONTROL_OBJ           2081   /* ENUM, reg 0x1002 */
#define TF_CTRL_COLD_TEMP             2082   /* INT degC, reg 0x1003 (integer C) */
#define TF_CTRL_COLD_DIFF             2083   /* INT degC, reg 0x1004 (integer C) */
#define TF_CTRL_HEAT_TEMP             2084   /* INT degC, reg 0x1005 (integer C) */
#define TF_CTRL_HEAT_DIFF             2085   /* INT degC, reg 0x1006 (integer C) */

/* EU3 (YFY) aircon setpoint ControlParameter IDs.
 * 501 block uses FC 0x10. 703 block uses FC 0x10 signed. Offsets below match
 * aircond_regs.h EU3_WR_IDX_* (note: 501+0 CELL_PWR_STATE is owned by param 445).
 */
#define EU3_CTRL_PCS_POWER            2086   /* ENUM, reg 501+1 (EU3_WR_IDX_PCS_PWR_STATE) */
#define EU3_CTRL_WORKING_MODE         2087   /* ENUM, reg 501+2 (EU3_WR_IDX_MODE) */
#define EU3_CTRL_CONTROL_OBJ          2088   /* ENUM, reg 501+3 (EU3_WR_IDX_CONTROL_OBJ) */
#define EU3_CTRL_COLD_TEMP            2089   /* FLOAT degC, reg 501+4 (EU3_WR_IDX_COLD_TEMP) *0.1 */
#define EU3_CTRL_HEAT_TEMP            2090   /* FLOAT degC, reg 501+5 (EU3_WR_IDX_HEAT_TEMP) *0.1 */
#define EU3_CTRL_NOISE_STATE          2091   /* ENUM, reg 501+6 (EU3_WR_IDX_NOISE_STATE) */
#define EU3_CTRL_COLD_OPEN_DIFF       2092   /* FLOAT degC signed, reg 703+0 *0.1 */
#define EU3_CTRL_COLD_CLOSE_DIFF      2093   /* FLOAT degC signed, reg 703+1 *0.1 */
#define EU3_CTRL_HEAT_OPEN_DIFF       2094   /* FLOAT degC signed, reg 703+2 *0.1 */
#define EU3_CTRL_HEAT_CLOSE_DIFF      2095   /* FLOAT degC signed, reg 703+3 *0.1 */

/*===========================================================================
** Per-device runtime state (expanded to hold both TF and EU3 fields)
**===========================================================================*/
typedef struct tagAirconDev
{
    int      nDevId;                              /* 951 */
    int      nDevModel;                           /* 950 */
    int      nAirType;                            /* AIR_TYPE_EU3 (1) or AIR_TYPE_TF (2) */
    int      nCommFailCnt;                        /* consecutive fail counter */
    int      nCommRecoverCnt;                     /* consecutive recovery confirm counter */
    BOOL     bCommOK;                             /* last known comm status */
    USHORT   rawBuf[AIRCON_MAX_REG_READ];         /* raw register scratch */

    /* Common parsed values (used by both variants where names overlap) */
    float    feedTemp;                            /* degC (EU3 FeedTemp / TF LiquidSupply) */
    float    exhTemp;                             /* degC (EU3 ExhTemp / TF ReturnLiquid) */
    float    sucTemp;                             /* degC */
    float    ambiTemp;                            /* degC */
    float    batTemp;                             /* degC */
    float    comRunFre;                           /* Hz */
    float    comTarFre;                           /* Hz */
    float    cellPumpHz;                          /* Hz */
    float    highPre;                             /* bar */
    float    lowPre;                              /* bar */
    float    tarTemp;                             /* degC */
    int      pwrState;
    int      runMode;
    int      runStep;
    int      errCode;
    int      errLevel;

    /* TF-specific raw register blocks (stored for full param push) */
    USHORT   tfCtrl[7];                           /* @0x1000 len 7 */
    USHORT   tfSta0[6];                           /* @0x0000 len 6 */
    USHORT   tfSta1[3];                           /* @0x0007 len 3 */
    USHORT   tfSta2[4];                           /* @0x000B len 4 */
    USHORT   tfSta3[2];                           /* @0x0012 len 2 */
    USHORT   tfSta4[1];                           /* @0x0015 len 1 */
    USHORT   tfSta5[9];                           /* @0x0019 len 9 (0x0019~0x0021) */
    USHORT   tfSta6[5];                           /* @0x0022 len 5 */

    /* EU3-specific raw register blocks */
    USHORT   eu3Ctrl[7];                          /* @501 len 7 */
    USHORT   eu3Ctrl1[4];                         /* @703 len 4 (signed) */
    USHORT   eu3Sta0[12];                         /* @499 len 12 */
    USHORT   eu3Sta1[4];                          /* @513 len 4 */
    USHORT   eu3Sta2[2];                          /* @538 len 2 */
    USHORT   eu3Sta3[11];                         /* @600 len 11 */
    USHORT   eu3Sta4[6];                          /* @612 len 6 */
    USHORT   eu3Sta5[7];                          /* @700 len 7 */
} AIRCON_DEV;

/* Per-sampler runtime state */
typedef struct tagAirconSampler
{
    HANDLE         hSampler;
    int            nSamplerId;
    AIRCON_DEV     dev;
    int            nSlaveAddr;
    int            nSamplePeriodMs;
    BOOL           bInited;
} AIRCON_SAMPLER;

#endif /* __AIRCON_SAMPLER_MAIN_H__ */
