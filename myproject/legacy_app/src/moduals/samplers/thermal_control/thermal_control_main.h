#ifndef _THERMAL_CONTROL_MAIN_H_
#define _THERMAL_CONTROL_MAIN_H_

/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**---------------------------------------File Information------------------------------------------
** @File Name    : thermal_control_main.h
** @Founder      : NJX01203
** @Date         : 2026.07.11
** @Function     : Formula-based BMS thermal management strategy (§2.1 aircon core).
**                 Replaces R-core 13-level if-else with T_decision formula + hysteresis
**                 state machine + time protection.
** @Instructions : Independent compute-only sampler (no CommPort, ID=13).
**                 Reads battery DPR params → computes T_decision → writes aircon
**                 control commands via Push_VarValue to devId=951 (aircon_sampler).
**                 Setting params 2408-2431, 2443-2451, 2452-2493 for formula coefficients & strategy data.  Mode switch is param 2203 on devId=951.
**===============================================================================================*/

#include "../sampler_base.h"

/* Sampler identity (matches System.sql Charger_Sampler entry) */
#define THM_SAMPLER_ID              13

/* DB identities for own device (compute-only, no physical hardware) */
#define THM_DEV_ID                  992
#define THM_DEV_MODEL               950

/*===========================================================================
** State machine
**===========================================================================*/
typedef enum tagTHM_STATE {
    THM_STATE_STANDBY  = 0,
    THM_STATE_COOLING  = 1,
    THM_STATE_DEEPCOOL = 2,
    THM_STATE_HEATING  = 3,
    THM_STATE_EMERGENCY_COOL = 4,
    THM_STATE_SELFCIRC = 5,
} THM_STATE;

/*===========================================================================
** New SettingParameter IDs (2408-2431) — formula coefficients & strategy data
** These belong to DeviceModel 950 (Air Conditioner).
**===========================================================================*/
#define THM_PARAM_I_OFFSET_L1       2408   /* FLOAT, I_offset when |I|/I_rate < 33% */
#define THM_PARAM_I_OFFSET_L2       2409   /* FLOAT, I_offset when 33% ≤ |I|/I_rate < 66% */
#define THM_PARAM_I_OFFSET_L3       2410   /* FLOAT, I_offset when 66% ≤ |I|/I_rate < 100% */
#define THM_PARAM_I_OFFSET_L4       2411   /* FLOAT, I_offset when |I|/I_rate ≥ 100% */
#define THM_PARAM_PCS_OFFSET_STBY   2412   /* FLOAT, PCS_offset when PCS in standby/idle */
#define THM_PARAM_PCS_OFFSET_CHG    2413   /* FLOAT, PCS_offset when PCS charging */
#define THM_PARAM_PCS_OFFSET_DCHG   2414   /* FLOAT, PCS_offset when PCS discharging */
#define THM_PARAM_T_TARGET_BASE     2415   /* FLOAT, base target temp (°C) for AC cooling setpoint */
#define THM_PARAM_MIN_RUN_SEC       2416   /* INT, minimum seconds in active state before STANDBY allowed */
#define THM_PARAM_MIN_STOP_SEC      2417   /* INT, minimum seconds in STANDBY before active state allowed */
#define THM_PARAM_COLD_DIFF         2418   /* FLOAT, cooling hysteresis differential (°C) */
#define THM_PARAM_COLD_DEEP_DELTA   2419   /* FLOAT, DEEPCOOL offset below base target (°C) */
#define THM_PARAM_COLD_DEEP_DIFF    2420   /* FLOAT, DEEPCOOL tighter hysteresis (°C) */
#define THM_PARAM_HEAT_TARGET       2421   /* FLOAT, heating target temp (°C) */
#define THM_PARAM_HEAT_DIFF         2422   /* FLOAT, heating hysteresis (°C) */
#define THM_PARAM_DEEPCOOL_ESC_SEC  2423   /* UINT, seconds in COOLING before DEEPCOOL upgrade */
#define THM_PARAM_EMERG_TRIGGER     2424   /* FLOAT, over-temp trigger (°C) */
#define THM_PARAM_EMERG_EXIT        2425   /* FLOAT, over-temp exit (°C) */
#define THM_PARAM_EMERG_TARGET      2426   /* FLOAT, over-temp target (°C) */
#define THM_PARAM_EMERG_EXIT_SEC    2427   /* UINT, over-temp exit confirm seconds */
#define THM_PARAM_I_RATIO_L1        2428   /* FLOAT, I_offset ratio threshold L1 */
#define THM_PARAM_I_RATIO_L2        2429   /* FLOAT, I_offset ratio threshold L2 */
#define THM_PARAM_I_RATIO_L3        2430   /* FLOAT, I_offset ratio threshold L3 */
#define THM_PARAM_STARTUP_PROT_SEC  2431   /* UINT, startup protection seconds */
#define THM_PARAM_COLD_ON_AVG        2432   /* FLOAT, cooling on: avg temp threshold (degC) */
#define THM_PARAM_COLD_OFF_AVG       2433   /* FLOAT, cooling off: avg temp threshold (degC) */
#define THM_PARAM_COLD_ON_MAX        2434   /* FLOAT, cooling on: max temp threshold (degC) */
#define THM_PARAM_COLD_OFF_MAX       2435   /* FLOAT, cooling off: max temp threshold (degC) */
#define THM_PARAM_COLD_EXIT_SEC      2436   /* UINT, cooling exit confirm seconds */
#define THM_PARAM_HEAT_ON_AVG        2437   /* FLOAT, heating on: avg temp threshold (degC) */
#define THM_PARAM_HEAT_ON_MIN        2438   /* FLOAT, heating on: min temp threshold (degC) */
#define THM_PARAM_HEAT_OFF_AVG       2439   /* FLOAT, heating off: avg temp threshold (degC) */
#define THM_PARAM_HEAT_OFF_MIN       2440   /* FLOAT, heating off: min temp threshold (degC) */
#define THM_PARAM_HEAT_EXIT_SEC      2441   /* UINT, heating exit confirm seconds */
#define THM_PARAM_MODE_SWITCH_SEC    2442   /* UINT, cool/heat switch full-stop buffer seconds */
#define THM_PARAM_CHARGE_RATED_CURR        2443   /* FLOAT, per-cluster charge rated current (A) */
#define THM_PARAM_DISCHARGE_RATED_CURR     2444   /* FLOAT, per-cluster discharge rated current (A) */
#define THM_PARAM_EVAL_CURR_WINDOW_SEC     2445   /* UINT, evaluation current window seconds */
#define THM_PARAM_SAME_CLUSTER_DIFF_ENTER  2446   /* FLOAT, same-cluster temp diff enter (degC) */
#define THM_PARAM_SAME_CLUSTER_DIFF_EXIT   2447   /* FLOAT, same-cluster temp diff exit (degC) */
#define THM_PARAM_CROSS_CLUSTER_DIFF_ENTER 2448   /* FLOAT, cross-cluster temp diff enter (degC) */
#define THM_PARAM_CROSS_CLUSTER_DIFF_EXIT  2449   /* FLOAT, cross-cluster temp diff exit (degC) */
#define THM_PARAM_SELF_CIRC_MAX_SEC        2450   /* UINT, self-circulation max duration seconds */
#define THM_PARAM_SELF_CIRC_INTERVAL_SEC   2451   /* UINT, self-circulation retrigger interval seconds */
#define THM_PARAM_SELF_CIRC_ENABLE        2494   /* UINT, self-circulation mode-3 enable gate (0=off until protocol confirmed) */

/*===========================================================================
** Phase-2 automatic-mode parameters (2452-2493) — feedforward curves, SOC/SOH
** correction, target bounds, cooling/heat boundaries (all have Parameter_Unit)
**===========================================================================*/
    /* A. current feedforward curves F(r) (sec 2.4) */
#define THM_PARAM_F_CHG_TOTAL_DT_03        2452   /* FLOAT, param 2452 */
#define THM_PARAM_F_CHG_TOTAL_DT_06        2453   /* FLOAT, param 2453 */
#define THM_PARAM_F_CHG_TOTAL_DT_08        2454   /* FLOAT, param 2454 */
#define THM_PARAM_F_CHG_TOTAL_DT_10        2455   /* FLOAT, param 2455 */
#define THM_PARAM_F_CHG_PEAK_DT_03         2456   /* FLOAT, param 2456 */
#define THM_PARAM_F_CHG_PEAK_DT_06         2457   /* FLOAT, param 2457 */
#define THM_PARAM_F_CHG_PEAK_DT_08         2458   /* FLOAT, param 2458 */
#define THM_PARAM_F_CHG_PEAK_DT_10         2459   /* FLOAT, param 2459 */
#define THM_PARAM_F_DCHG_TOTAL_DT_03       2460   /* FLOAT, param 2460 */
#define THM_PARAM_F_DCHG_TOTAL_DT_06       2461   /* FLOAT, param 2461 */
#define THM_PARAM_F_DCHG_TOTAL_DT_08       2462   /* FLOAT, param 2462 */
#define THM_PARAM_F_DCHG_TOTAL_DT_10       2463   /* FLOAT, param 2463 */
#define THM_PARAM_F_DCHG_PEAK_DT_03        2464   /* FLOAT, param 2464 */
#define THM_PARAM_F_DCHG_PEAK_DT_06        2465   /* FLOAT, param 2465 */
#define THM_PARAM_F_DCHG_PEAK_DT_08        2466   /* FLOAT, param 2466 */
#define THM_PARAM_F_DCHG_PEAK_DT_10        2467   /* FLOAT, param 2467 */

    /* B. SOC/SOH correction (sec 2.5) */
#define THM_PARAM_SOC_CHG_ENTER            2469   /* FLOAT, param 2469 */
#define THM_PARAM_SOC_CHG_EXIT             2470   /* FLOAT, param 2470 */
#define THM_PARAM_SOC_CHG_SLOPE            2471   /* FLOAT, param 2471 */
#define THM_PARAM_SOC_CHG_LIMIT            2472   /* FLOAT, param 2472 */
#define THM_PARAM_SOC_DCHG_ENTER           2474   /* FLOAT, param 2474 */
#define THM_PARAM_SOC_DCHG_EXIT            2475   /* FLOAT, param 2475 */
#define THM_PARAM_SOC_DCHG_SLOPE           2476   /* FLOAT, param 2476 */
#define THM_PARAM_SOC_DCHG_LIMIT           2477   /* FLOAT, param 2477 */
#define THM_PARAM_SOH_BASE                 2479   /* FLOAT, param 2479 */
#define THM_PARAM_SOH_SLOPE                2480   /* FLOAT, param 2480 */
#define THM_PARAM_SOH_LIMIT                2481   /* FLOAT, param 2481 */

    /* C. target water temp bounds (sec 2.6) */
#define THM_PARAM_T_TARGET_MIN             2482   /* FLOAT, param 2482 */
#define THM_PARAM_T_TARGET_MAX             2483   /* FLOAT, param 2483 */

    /* D. cooling intensity boundary + cool-exit load (sec 3.2/3.3) */
#define THM_PARAM_FORCE_COOL_T_TARGET      2484   /* FLOAT, param 2484 */
#define THM_PARAM_FORCE_COOL_R_TOTAL       2485   /* FLOAT, param 2485 */
#define THM_PARAM_FORCE_COOL_R_PEAK        2486   /* FLOAT, param 2486 */
#define THM_PARAM_FORCE_COOL_T_MAX         2487   /* FLOAT, param 2487 */
#define THM_PARAM_COOL_EXIT_R_TOTAL        2488   /* FLOAT, param 2488 */
#define THM_PARAM_COOL_EXIT_R_PEAK         2489   /* FLOAT, param 2489 */

    /* E. cold/heat interlock + low-temp charge (sec 4.2/4.3) */
#define THM_PARAM_HEAT_BLOCK_T_MAX         2490   /* FLOAT, param 2490 */
#define THM_PARAM_LOWT_CHG_LV1             2491   /* FLOAT, param 2491 */
#define THM_PARAM_LOWT_CHG_LV2             2492   /* FLOAT, param 2492 */
#define THM_PARAM_LOWT_CHG_LV3             2493   /* FLOAT, param 2493 */

/*===========================================================================
** Aircon control mode (param 2203 on devId=951, DeviceModel 950)
** This is the SINGLE source of truth for auto/manual mode.
**===========================================================================*/
#define THM_PARAM_AIRCON_CTRL_MODE  2203   /* on devId=951: ENUM 0=Auto, 1=Manual */
#define THM_AIRCON_MODE_AUTO        0
#define THM_AIRCON_MODE_MANUAL      1

/*===========================================================================
** Source (existing) parameter IDs — read from DPR
**===========================================================================*/
#define THM_SRC_T_CELL_MAX          558    /* FLOAT, battery cell max temp (°C) */
#define THM_SRC_T_CELL_MIN          559    /* FLOAT, battery cell min temp (°C) */
#define THM_SRC_HV_STATUS           553    /* ENUM, high-voltage contactor status */
#define THM_RPMSG_DEV_ID            500    /* RPMsg data aggregator (核间通信) */
#define THM_SRC_RPMSG_STATUS        510    /* BLOB, A_BLOB_STATUS_STAT (per-cluster: charge_discharge_status + total_current + cell temps) */
#define THM_SRC_RPMSG_ENERGY        509    /* BLOB, A_BLOB_ENERGY_STAT (per-cluster: SOC 千分比) */

/*===========================================================================
** Thermal threshold SettingParameter IDs (existing, DeviceModel 501)
**===========================================================================*/
#define THM_SRC_T_COLD_ON           8000   /* FLOAT, start cooling when T_decision > this */
#define THM_SRC_T_COLD_OFF          8001   /* FLOAT, stop cooling when T_decision ≤ this */
#define THM_SRC_T_HEAT_ON           8002   /* FLOAT, start heating when T_decision < this */
#define THM_SRC_T_HEAT_OFF          8003   /* FLOAT, stop heating when T_decision ≥ this */

/*===========================================================================
** Default threshold values (used before DPR values are loaded or if read fails)
**===========================================================================*/
#define THM_DEFAULT_T_COLD_ON       33.0f
#define THM_DEFAULT_T_COLD_OFF      28.0f
#define THM_DEFAULT_T_HEAT_ON       5.0f
#define THM_DEFAULT_T_HEAT_OFF      12.0f

/*===========================================================================
** Default formula coefficients (used before Control() receives Web settings)
**===========================================================================*/
#define THM_DEFAULT_I_OFFSET_L1     1.0f
#define THM_DEFAULT_I_OFFSET_L2     2.0f
#define THM_DEFAULT_I_OFFSET_L3     3.0f
#define THM_DEFAULT_I_OFFSET_L4     4.0f
#define THM_DEFAULT_PCS_OFFSET_STBY 0.0f
#define THM_DEFAULT_PCS_OFFSET_CHG  1.0f
#define THM_DEFAULT_PCS_OFFSET_DCHG 1.0f
#define THM_DEFAULT_T_TARGET_BASE   25.0f
#define THM_DEFAULT_MIN_RUN_SEC     600
#define THM_DEFAULT_MIN_STOP_SEC    300
#define THM_DEFAULT_COLD_DIFF          2.0f
#define THM_DEFAULT_COLD_DEEP_DELTA    5.0f
#define THM_DEFAULT_COLD_DEEP_DIFF     1.0f
#define THM_DEFAULT_HEAT_TARGET        10.0f
#define THM_DEFAULT_HEAT_DIFF          2.0f
#define THM_DEFAULT_DEEPCOOL_ESC_SEC   600
#define THM_DEFAULT_EMERG_TRIGGER      40.0f
#define THM_DEFAULT_EMERG_EXIT         38.0f
#define THM_DEFAULT_EMERG_TARGET       15.0f
#define THM_DEFAULT_EMERG_EXIT_SEC     300
#define THM_DEFAULT_I_RATIO_L1         0.33f
#define THM_DEFAULT_I_RATIO_L2         0.66f
#define THM_DEFAULT_I_RATIO_L3         1.00f
#define THM_DEFAULT_STARTUP_PROT_SEC   5
#define THM_DEFAULT_COLD_ON_AVG           32.0f
#define THM_DEFAULT_COLD_OFF_AVG          29.0f
#define THM_DEFAULT_COLD_ON_MAX           34.0f
#define THM_DEFAULT_COLD_OFF_MAX          30.0f
#define THM_DEFAULT_COLD_EXIT_SEC         30
#define THM_DEFAULT_HEAT_ON_AVG           17.0f
#define THM_DEFAULT_HEAT_ON_MIN           15.0f
#define THM_DEFAULT_HEAT_OFF_AVG          19.0f
#define THM_DEFAULT_HEAT_OFF_MIN          17.0f
#define THM_DEFAULT_HEAT_EXIT_SEC         60
#define THM_DEFAULT_MODE_SWITCH_SEC       60
#define THM_DEFAULT_CHARGE_RATED_CURR     157.0f
#define THM_DEFAULT_DISCHARGE_RATED_CURR  157.0f
#define THM_DEFAULT_EVAL_CURR_WINDOW_SEC  10
#define THM_DEFAULT_SAME_CLUSTER_DIFF_ENTER  6.0f
#define THM_DEFAULT_SAME_CLUSTER_DIFF_EXIT   4.0f
#define THM_DEFAULT_CROSS_CLUSTER_DIFF_ENTER 6.0f
#define THM_DEFAULT_CROSS_CLUSTER_DIFF_EXIT  4.0f
#define THM_DEFAULT_SELF_CIRC_MAX_SEC     1800
#define THM_DEFAULT_SELF_CIRC_INTERVAL_SEC 3600
#define THM_DEFAULT_SELF_CIRC_ENABLE     0

/*===========================================================================
** Default values for Phase-2 automatic-mode params (2452-2493)
**===========================================================================*/
/* A. current feedforward curves F(r) (sec 2.4) */
#define THM_DEFAULT_F_CHG_TOTAL_DT_03     0.0f
#define THM_DEFAULT_F_CHG_TOTAL_DT_06     3.0f
#define THM_DEFAULT_F_CHG_TOTAL_DT_08     6.5f
#define THM_DEFAULT_F_CHG_TOTAL_DT_10     10.0f
#define THM_DEFAULT_F_CHG_PEAK_DT_03      0.0f
#define THM_DEFAULT_F_CHG_PEAK_DT_06      0.0f
#define THM_DEFAULT_F_CHG_PEAK_DT_08      2.0f
#define THM_DEFAULT_F_CHG_PEAK_DT_10      4.0f
#define THM_DEFAULT_F_DCHG_TOTAL_DT_03    0.0f
#define THM_DEFAULT_F_DCHG_TOTAL_DT_06    3.0f
#define THM_DEFAULT_F_DCHG_TOTAL_DT_08    6.5f
#define THM_DEFAULT_F_DCHG_TOTAL_DT_10    10.0f
#define THM_DEFAULT_F_DCHG_PEAK_DT_03     0.0f
#define THM_DEFAULT_F_DCHG_PEAK_DT_06     0.0f
#define THM_DEFAULT_F_DCHG_PEAK_DT_08     2.0f
#define THM_DEFAULT_F_DCHG_PEAK_DT_10     4.0f

/* B. SOC/SOH correction (sec 2.5) */
#define THM_DEFAULT_SOC_CHG_ENTER         95.0f
#define THM_DEFAULT_SOC_CHG_EXIT          90.0f
#define THM_DEFAULT_SOC_CHG_SLOPE         0.0f
#define THM_DEFAULT_SOC_CHG_LIMIT         0.0f
#define THM_DEFAULT_SOC_DCHG_ENTER        20.0f
#define THM_DEFAULT_SOC_DCHG_EXIT         25.0f
#define THM_DEFAULT_SOC_DCHG_SLOPE        0.0f
#define THM_DEFAULT_SOC_DCHG_LIMIT        0.0f
#define THM_DEFAULT_SOH_BASE              90.0f
#define THM_DEFAULT_SOH_SLOPE             0.0f
#define THM_DEFAULT_SOH_LIMIT             2.0f

/* C. target water temp bounds (sec 2.6) */
/* No humidity sensor -> no dew-point calc; use conservative fixed lower limit
 * (spec 3.2): 12C avoids condensation on cold plates/pipes in typical indoor
 * environments. Adjustable via param 2482 after device-manual/prototype confirm. */
#define THM_DEFAULT_T_TARGET_MIN          12.0f
#define THM_DEFAULT_T_TARGET_MAX          35.0f

/* D. cooling intensity boundary + cool-exit load (sec 3.2/3.3) */
#define THM_DEFAULT_FORCE_COOL_T_TARGET   20.0f
#define THM_DEFAULT_FORCE_COOL_R_TOTAL    0.8f
#define THM_DEFAULT_FORCE_COOL_R_PEAK     0.9f
#define THM_DEFAULT_FORCE_COOL_T_MAX      38.0f
#define THM_DEFAULT_COOL_EXIT_R_TOTAL     0.6f
#define THM_DEFAULT_COOL_EXIT_R_PEAK      0.7f

/* E. cold/heat interlock + low-temp charge (sec 4.2/4.3) */
#define THM_DEFAULT_HEAT_BLOCK_T_MAX      25.0f
#define THM_DEFAULT_LOWT_CHG_LV1          10.0f
#define THM_DEFAULT_LOWT_CHG_LV2          5.0f
#define THM_DEFAULT_LOWT_CHG_LV3          0.0f

/*===========================================================================
** Destination: aircon control params (devId=951, Sampler=11 aircon_sampler)
** Pushed via Push_VarValue → framework routes to aircon_sampler::Control()
**===========================================================================*/
#define THM_DST_DEV_ID              951
#define THM_DST_AIR_POWER           445    /* ENUM: 0=stop, 2=cold, 4=heat */
#define THM_DST_TF_MODE             2080   /* ENUM: 1=cool, 2=heat */
#define THM_DST_TF_CTRL_OBJ         2081   /* ENUM: 1=T_out */
#define THM_DST_TF_COLD_TEMP        2082   /* UINT degC */
#define THM_DST_TF_COLD_DIFF        2083   /* INT degC */
#define THM_DST_TF_HEAT_TEMP        2084   /* INT degC */
#define THM_DST_TF_HEAT_DIFF        2085   /* INT degC */

/*===========================================================================
** EU3 destination param IDs (also devId=951, pushed to aircon_sampler)
** EU3 temps are FLOAT degC; aircon_sampler Control() scales x10.
**===========================================================================*/
#define THM_DST_EU3_MODE             2087   /* ENUM: 1=cool, 2=heat */
#define THM_DST_EU3_CTRL_OBJ         2088   /* ENUM: 1=T_out */
#define THM_DST_EU3_COLD_TEMP        2089   /* FLOAT degC (Control x10) */
#define THM_DST_EU3_HEAT_TEMP        2090   /* FLOAT degC (Control x10) */
#define THM_DST_EU3_COLD_OPEN_DIFF   2092   /* FLOAT degC (Control x10) */
#define THM_DST_EU3_HEAT_OPEN_DIFF   2094   /* FLOAT degC (Control x10) */
#define THM_DST_EU3_NOISE            2091   /* ENUM: 0-6, 6=no noise reduction (spec 6.2.2) */
#define THM_NOISE_LEVEL_NORMAL       6      /* noise reduction off = normal cooling */
#define THM_STAT_NOISE_OVERRIDE      3729   /* ENUM: 0=normal, 1=safety override active (EU3 noise) */

/* AC install type param (on devId=951, read to determine TF vs EU3 push path) */
#define THM_PARAM_AIR_TYPE           448    /* ENUM: 0=none, 1=EU3, 2=TF */

/* AC communication probe params (read from devId=951 to detect comm loss) */
#define THM_SRC_AC_STATE_TF          1777   /* TF UnitState (param on DM 950, devId=951) */
#define THM_SRC_AC_STATE_EU3         1560   /* EU3 OperatingStatus (param on DM 950, devId=951) */

/*===========================================================================
** PCS state categorization (maps STBATT_RUN_STATU → offset category)
**===========================================================================*/
#define THM_PCS_STBY     0
#define THM_PCS_CHARGING 1
#define THM_PCS_DISCHG   2

/* Per-cluster charge/discharge state (charge_discharge_status from R-core, spec 2.1) */
#define THM_CLUSTER_IDLE    0
#define THM_CLUSTER_CHG     1
#define THM_CLUSTER_DCHG    2

/*===========================================================================
** Base device ID for battery groups (devId = 501 + groupIndex)
**===========================================================================*/
#define THM_BATT_DEV_BASE           501

/*===========================================================================
** Maximum battery groups to scan
**===========================================================================*/
#define THM_MAX_BATT_GROUPS         8

/*===========================================================================
** Per-module runtime state
**===========================================================================*/
typedef struct tagThermalControl
{
    BOOL        bInited;
    HANDLE      hSampler;

    /* Current state machine state */
    THM_STATE   eState;
    long long    tStateEntered;       /* monotonic seconds (CLOCK_MONOTONIC) */
    long long    tStarted;            /* monotonic seconds at Initialize (startup protection) */
    int         nCycleCount;

    /* Cached input readings */
    float       fTCellMax;
    float       fTCellMin;
    float       fBattBusCurr;
    float       fAvgSOC;
    int         nPCSState;
    int         nHVFaultCount;       /* number of groups with HV fault */
    int         nAirType;            /* cached AC type: 0=none, 1=EU3, 2=TF */

    /* Per-cluster battery inputs (Phase 2.1: dual-cluster data access).
     * Indexed 0..MAX_BATT_GROUP_NUM-1; populated from RPMSG BLOBs (509/510)
     * which carry per-cluster arrays. */
    float       fClusterCurr[MAX_BATT_GROUP_NUM];      /* I_i: signed cluster current [A] (global_info[i].total_current, neg=charge) */
    float       fClusterSoc[MAX_BATT_GROUP_NUM];       /* SOC_i [%] (power_info[i].soc / 10) */
    float       fClusterSoh[MAX_BATT_GROUP_NUM];       /* SOH_i [%] (power_info[i].soh / 10) */
    int         nClusterChgDischg[MAX_BATT_GROUP_NUM]; /* per-cluster charge_discharge_status (0=idle,1=chg,2=dischg) */
    int         nClusterCount;                          /* number of installed clusters (2=836 dual-cluster, 1=单簇240) */
    float       fClusterVolt[MAX_BATT_GROUP_NUM];       /* per-cluster total_voltage [0.1V]; >0.1 => comm present */
    int         nClusterWorkStatus[MAX_BATT_GROUP_NUM]; /* per-cluster work_status (0-6, 5=fault) */
    BOOL        bStatusStatOk;                          /* status BLOB (510) read OK this cycle */
    BOOL        bEnergyStatOk;                          /* energy BLOB (509) read OK this cycle */
    float       fClusterTMax[MAX_BATT_GROUP_NUM];      /* per-cluster cell max temp [degC] (feature_info[i].max_temperature_value) */
    float       fClusterTMin[MAX_BATT_GROUP_NUM];      /* per-cluster cell min temp [degC] (feature_info[i].min_temperature_value) */
    float       fClusterTAvg[MAX_BATT_GROUP_NUM];      /* per-cluster cell avg temp [degC] (feature_info[i].cell_avg_temperature) */

    /* Phase 2.7 computed -- self-circulation temp diffs (spec 5.2/5.2.1) */
    float       fSameClusterDiff;                       /* max over valid clusters of (T_max,i - T_min,i) */
    float       fCrossClusterDiff;                      /* |T_avg,1 - T_avg,2| (dual-cluster only, else 0) */

    /* Phase 2 computed -- per-cluster load ratios (spec 2.3) */
    float       fClusterR[MAX_BATT_GROUP_NUM];       /* r_i per cluster */
    BOOL        bClusterValid[MAX_BATT_GROUP_NUM];   /* per-cluster data validity */
    float       fRPeak;                               /* r_peak = max(r_i) */
    float       fRTotal;                              /* r_total = Sum|I|/SumI_rated,dir */
    int         nDataDegrade;                         /* degraded data-point count this cycle */

    /* Phase 2 computed -- feedforward, corrections, target water temp (spec 2.4-2.6) */
    float       fClusterDtCurr[MAX_BATT_GROUP_NUM];   /* dT_current,i (current feedforward) */
    float       fClusterDtSoc[MAX_BATT_GROUP_NUM];    /* dT_SOC,i */
    float       fClusterDtSoh[MAX_BATT_GROUP_NUM];    /* dT_SOH,i */
    float       fClusterTTarget[MAX_BATT_GROUP_NUM];  /* T_target,i */
    float       fTTargetSystem;                       /* T_target,system = min over clusters */

    /* Computed outputs */
    float       fTDecision;
    float       fIOffsetApplied;
    float       fPCSOffsetApplied;
    BOOL        bThresholdsLoaded;   /* have we read 8000-8003 from DPR? */
    BOOL        bCoeffsLoaded;      /* have we read 2408-2417 from DPR? */

    /* Settings — formula coefficients (params 2408-2417) */
    float       fIOffsetL1;
    float       fIOffsetL2;
    float       fIOffsetL3;
    float       fIOffsetL4;
    float       fPCSOffsetStby;
    float       fPCSOffsetChg;
    float       fPCSOffsetDchg;
    float       fTTargetBase;
    int         nMinRunSec;
    int         nMinStopSec;

    /* Settings — newly parameterized strategy data (params 2418-2431) */
    float       fColdDiff;
    float       fColdDeepDelta;
    float       fColdDeepDiff;
    float       fHeatTarget;
    float       fHeatDiff;
    int         nDeepcoolEscSec;
    float       fEmergTrigger;
    float       fEmergExit;
    float       fEmergTarget;
    int         nEmergExitSec;
    float       fIRatioL1;
    float       fIRatioL2;
    float       fIRatioL3;
    int         nStartupProtectSec;

    /* Settings — cooling/heating decision & time protection (params 2432-2442) */
    float       fColdOnAvg;
    float       fColdOffAvg;
    float       fColdOnMax;
    float       fColdOffMax;
    int         nColdExitSec;
    float       fHeatOnAvg;
    float       fHeatOnMin;
    float       fHeatOffAvg;
    float       fHeatOffMin;
    int         nHeatExitSec;
    int         nModeSwitchSec;

    /* Settings — per-cluster current rating & self-circulation (params 2443-2451) */
    float       fChargeRatedCurr;
    float       fDischargeRatedCurr;
    int         nEvalCurrWindowSec;
    float       fSameClusterDiffEnter;
    float       fSameClusterDiffExit;
    float       fCrossClusterDiffEnter;
    float       fCrossClusterDiffExit;
    int         nSelfCircMaxSec;
    int         nSelfCircIntervalSec;
    int         nSelfCircEnable;      /* mode-3 push gate (0=off until protocol confirmed) */

    /* Settings — Phase-2 automatic-mode params (2452-2493) */
    /* A. current feedforward curves F(r) (sec 2.4) */
    float      fFChgTotalDt03;
    float      fFChgTotalDt06;
    float      fFChgTotalDt08;
    float      fFChgTotalDt10;
    float      fFChgPeakDt03;
    float      fFChgPeakDt06;
    float      fFChgPeakDt08;
    float      fFChgPeakDt10;
    float      fFDchgTotalDt03;
    float      fFDchgTotalDt06;
    float      fFDchgTotalDt08;
    float      fFDchgTotalDt10;
    float      fFDchgPeakDt03;
    float      fFDchgPeakDt06;
    float      fFDchgPeakDt08;
    float      fFDchgPeakDt10;

    /* B. SOC/SOH correction (sec 2.5) */
    float      fSocChgEnter;
    float      fSocChgExit;
    float      fSocChgSlope;
    float      fSocChgLimit;
    float      fSocDchgEnter;
    float      fSocDchgExit;
    float      fSocDchgSlope;
    float      fSocDchgLimit;
    float      fSohBase;
    float      fSohSlope;
    float      fSohLimit;

    /* C. target water temp bounds (sec 2.6) */
    float      fTTargetMin;
    float      fTTargetMax;

    /* D. cooling intensity boundary + cool-exit load (sec 3.2/3.3) */
    float      fForceCoolTTarget;
    float      fForceCoolRTotal;
    float      fForceCoolRPeak;
    float      fForceCoolTMax;
    float      fCoolExitRTotal;
    float      fCoolExitRPeak;

    /* E. cold/heat interlock + low-temp charge (sec 4.2/4.3) */
    float      fHeatBlockTMax;
    float      fLowTChgLv1;
    float      fLowTChgLv2;
    float      fLowTChgLv3;

    /* Phase 2.7 self-circulation runtime tracking */
    long long  tSelfCircLastOff;                  /* monotonic sec of last self-circulation exit (negative sentinel = never ran) */

    /* Phase 2.8 cold/heat interlock runtime tracking (spec 4.2) */
    float       fSysClusterTMax;       /* max valid cluster T_max (degC); -999 if none valid */
    int         nCoolDemandClusters;   /* valid clusters with cooling demand */
    int         nHeatDemandClusters;   /* valid clusters with heating demand */
    BOOL        bColdHeatConflict;     /* cooling + heating demand simultaneously */
    BOOL        bPrevColdHeatConflict; /* previous-cycle conflict (one-shot report) */
    long long   tLastCoolEnd;          /* monotonic sec last exited cooling-family state */
    long long   tLastHeatEnd;          /* monotonic sec last exited heating state */

    /* Phase 2.9 AC comm-loss tracking (spec 7.1/7.2): never pretend standby */
    BOOL        bACCommLost;           /* AC comm currently lost */
    long long   tACLostAt;             /* monotonic sec AC comm was lost */
    long long   tLastCommandAt;        /* monotonic sec last AC command was pushed */

    /* Phase 2.10 same-state target resend (spec 2.6/2.10) */
    float       fLastPushedCoolTarget; /* last cooling target pushed (degC); -999 sentinel */
    /* Phase 3.4 noise safety override (spec 6.2.2) */
    int         nNoiseLevelBeforeOverride; /* cached 2091 value when override engaged */
    BOOL        bNoiseOverrideActive;      /* TRUE while forcing noise=6 (normal) */

    /* Settings — thermal thresholds (params 8000-8003, cache only) */
    float       fTColdOn;
    float       fTColdOff;
    float       fTHeatOn;
    float       fTHeatOff;

    /* Last pushed state (avoid redundant pushes) */
    THM_STATE   eLastPushedState;
    long long    tEmergencyBelowExit;   /* monotonic sec: when T first dropped below exit threshold */
} THERMAL_CONTROL;

#endif /* _THERMAL_CONTROL_MAIN_H_ */
