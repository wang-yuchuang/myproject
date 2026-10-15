#ifndef __AIRCOND_REGS_H__
#define __AIRCOND_REGS_H__

/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**---------------------------------------File Information------------------------------------------
** @File Name    : aircond_regs.h
** @Founder      : NJX01203
** @Date         : 2026.07.02
** @Function     : Modbus register map for liquid-cooling air conditioner (EU3/YFY + TF/Tongfei)
** @Instructions : Migrated from R-core AuxDevTactic.c; both variants supported at runtime.
**===============================================================================================*/

/* Modbus slave address (air conditioner is always slave 1) */
#define MODBUS_SLAVE_ADDR_AIR       1

/* ===== EU3 / YFY self-developed liquid-cooling AC register map ===== */
/* Control parameters - read with FC 0x03, write with FC 0x10 */
#define MB_EU3_AIR_WR_BASE_ADDR     501   /* len 7: CellPwrState, PcsPwrState, Mode, ControlObj, ColdTemp, HeatTemp, NoiseState */
#define MB_EU3_AIR_WR_BASE1_ADDR    703   /* len 4 (signed): ColdOpenDiff, ColdCloseDiff, HeatOpenDiff, HeatCloseDiff */

/* Status parameters - read with FC 0x04 (input registers) */
#define MB_EU3_AIR_RO_BASE_ADDR     499   /* len 12: PwrState, RunMode, Runstep, ErrCode, ErrLevel, FeedTemp, ExhTemp, SucTemp, AmbiTemp, BatTemp, ... */
#define MB_EU3_AIR_RO_BASE1_ADDR    513   /* len 4: ComRunFre, ComTarFre, ComRunStep, CellPumpHz */
#define MB_EU3_AIR_RO_BASE2_ADDR    538   /* len 2: failure level (fluid-path fault status), comp limit state */
#define MB_EU3_AIR_RO_BASE3_ADDR    600   /* len 11: NTC temperatures */
#define MB_EU3_AIR_RO_BASE4_ADDR    612   /* len 6: pressures HighPre, LowPre, ... */
#define MB_EU3_AIR_RO_BASE5_ADDR    700   /* len 7: failure bits SensorErr1/2, ComponErr1/2, CompErr1/2, SysStaErr2 */

/* EU3 WR 501 field offsets */
#define EU3_WR_IDX_CELL_PWR_STATE   0
#define EU3_WR_IDX_PCS_PWR_STATE    1
#define EU3_WR_IDX_MODE             2
#define EU3_WR_IDX_CONTROL_OBJ      3
#define EU3_WR_IDX_COLD_TEMP        4   /* 0.1 degC */
#define EU3_WR_IDX_HEAT_TEMP        5   /* 0.1 degC */
#define EU3_WR_IDX_NOISE_STATE      6

/* EU3 WR 703 field offsets (signed) */
#define EU3_WR_IDX_COLD_OPEN_DIFF   0   /* [10,100] = 1.0~10.0 degC */
#define EU3_WR_IDX_COLD_CLOSE_DIFF  1   /* [-100,-10] */
#define EU3_WR_IDX_HEAT_OPEN_DIFF   2
#define EU3_WR_IDX_HEAT_CLOSE_DIFF  3

/* ===== TF / Tongfei liquid-cooling AC register map ===== */
/* On/Off command - write with FC 0x05/0x06 (bit command register) */
#define MB_TF_AIR_ONOFF_ADDR        0x300  /* 开关机命令: B0=开机命令, B1=关机命令, B2=报警复位 */

/* Control parameters - read with FC 0x03, write with FC 0x06 (single) / 0x10 (multi) */
#define MB_TF_AIR_WR_BASE_ADDR      0x1000  /* len 7: StartMode, mode, controlObj, coldTemp, coldDiff, heatTemp, heatDiff */

/* Status parameters - read with FC 0x03 (holding registers) */
#define MB_TF_AIR_RO_BASE_ADDR      0x0000  /* len 6 */
#define MB_TF_AIR_RO_BASE1_ADDR     0x0007  /* len 3 */
#define MB_TF_AIR_RO_BASE2_ADDR     0x000B  /* len 4 */
#define MB_TF_AIR_RO_BASE3_ADDR     0x0012  /* len 2 */
#define MB_TF_AIR_RO_BASE4_ADDR     0x0015  /* len 1 */
#define MB_TF_AIR_RO_BASE5_ADDR     0x0019  /* len 9 (0x0019~0x0021: 故障状态1-7 + 压机故障等级/代码) */
#define MB_TF_AIR_RO_BASE6_ADDR     0x0022  /* len 5 */

/* TF WR 0x1000 field offsets */
#define TF_WR_IDX_START_MODE        0
#define TF_WR_IDX_MODE              1
#define TF_WR_IDX_CONTROL_OBJ       2
#define TF_WR_IDX_COLD_TEMP         3   /* integer degC */
#define TF_WR_IDX_COLD_DIFF         4
#define TF_WR_IDX_HEAT_TEMP         5
#define TF_WR_IDX_HEAT_DIFF         6

/* Runtime variant selection */
#define AIR_TYPE_NONE               0
#define AIR_TYPE_EU3                1
#define AIR_TYPE_TF                 2

#endif /* __AIRCOND_REGS_H__ */
