/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**---------------------------------------File Information------------------------------------------
** @File Name    : thermal_control_main.c
** @Founder      : NJX01203
** @Date         : 2026.07.11
** @Function     : Formula-based BMS thermal management strategy (§2.1 aircon core).
**                 Decision formula: T_decision = T_cellMax + I_offset(|I|/I_rate) + PCS_offset(state)
**                 State machine: STANDBY ↔ COOLING/DEEPCOOL/HEATING with hysteresis + time protection.
**                 Output: Push_VarValue → aircon_sampler::Control() → Modbus RTU.
**===============================================================================================*/

#include "thermal_control_main.h"
#include <string.h>
#include <time.h>

/*=================================================================================================
** @Name      : getMonotonicSec
** @Function  : Return monotonic time in seconds (immune to system clock jumps).
**===============================================================================================*/
static long long getMonotonicSec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec;
}

/*=================================================================================================
** @Name      : pushParamEnum / pushParamInt
** @Function  : Push_VarValue type-convenience wrappers (matching aircon_sampler pattern).
**===============================================================================================*/
/*=================================================================================================
** @Name      : pushParamFloat
** @Function  : Push_VarValue type-convenience wrapper for FLOAT values (needed for EU3 temps).
**===============================================================================================*/
static void pushParamFloat(HANDLE hSampler, int nDevId, int nParamId, float fVal)
{
    VAR_VALUE varV;
    memset(&varV, 0, sizeof(varV));
    varV.fValue = fVal;
    Push_VarValue(SETTYPE_INTERN_SETT, hSampler, nDevId, nParamId, varV);
}

static void pushParamEnum(HANDLE hSampler, int nDevId, int nParamId, long emVal)
{
    VAR_VALUE varV;
    memset(&varV, 0, sizeof(varV));
    varV.emValue = (_ENUM)emVal;
    Push_VarValue(SETTYPE_INTERN_SETT, hSampler, nDevId, nParamId, varV);
}

static void pushParamInt(HANDLE hSampler, int nDevId, int nParamId, long lVal)
{
    VAR_VALUE varV;
    memset(&varV, 0, sizeof(varV));
    varV.lValue = lVal;
    Push_VarValue(SETTYPE_INTERN_SETT, hSampler, nDevId, nParamId, varV);
}

static void pushParamStatus(HANDLE hSampler, int nDevId, int nParamId, long emVal)
{
    VAR_VALUE varV;
    memset(&varV, 0, sizeof(varV));
    varV.emValue = (_ENUM)emVal;
    Push_VarValue(SETTYPE_RAW_SP, hSampler, nDevId, nParamId, varV);
}

/*=================================================================================================
** @Name      : ComputeIOffset
** @Function  : 4-level I_offset lookup based on |I|/I_rate ratio.
**              Level 1: |I|/I_rate < 33%  → fIOffsetL1
**              Level 2: 33% ≤ ratio < 66% → fIOffsetL2
**              Level 3: 66% ≤ ratio < 100%→ fIOffsetL3
**              Level 4: ratio ≥ 100%       → fIOffsetL4
**===============================================================================================*/
static float ComputeIOffset(float fCurrRatio, THERMAL_CONTROL *pThm)
{
    if(fCurrRatio < pThm->fIRatioL1)
        return pThm->fIOffsetL1;
    else if(fCurrRatio < pThm->fIRatioL2)
        return pThm->fIOffsetL2;
    else if(fCurrRatio < pThm->fIRatioL3)
        return pThm->fIOffsetL3;
    else
        return pThm->fIOffsetL4;
}

/*=================================================================================================
** @Name      : ComputePCSOffset
** @Function  : Map STBATT_RUN_STATU → PCS_offset category, return corresponding coefficient.
**              Standby (0/2/3) → fPCSOffsetStby
**              Charge  (4/6)   → fPCSOffsetChg
**              Discharge (5/7) → fPCSOffsetDchg
**              Fault/unexpected → 0.0f
**              Note: ReadBattInputs only maps charge_discharge_status 0/1/2 to
**              IDLE/CHARGE/DISCHARGE; the NOT_INSTALLED/SLEEP/FULL_CHARGE/
**              FULL_DISCHARGE cases below are currently unreachable (kept defensively).
**===============================================================================================*/
static float ComputePCSOffset(int nPCSState, THERMAL_CONTROL *pThm)
{
    switch(nPCSState)
    {
        case STBATT_RUNSTAT_NOT_INSTALLED:
        case STBATT_RUNSTAT_SLEEP:
        case STBATT_RUNSTAT_IDLE:
            return pThm->fPCSOffsetStby;

        case STBATT_RUNSTAT_CHARGE:
        case STBATT_RUNSTAT_FULL_CHARGE:
            return pThm->fPCSOffsetChg;

        case STBATT_RUNSTAT_DISCHARGE:
        case STBATT_RUNSTAT_FULL_DISCHARGE:
            return pThm->fPCSOffsetDchg;

        default: /* COMM_FAIL or unknown */
            TRACED("[thermal] ComputePCSOffset: unexpected PCS state=%d, using 0", nPCSState);
            return 0.0f;
    }
}

/*=================================================================================================
** @Name      : IsValidTemp
** @Function  : Return TRUE if a sampled temperature is a valid reading (spec 7.3).
**              The BMU reports sentinel values -41.0C and 126.0C when a sensor is
**              invalid/out-of-range; these must be excluded from max/min/avg statistics
**              so they cannot trigger spurious cooling/heating.
**===============================================================================================*/
static BOOL IsValidTemp(float fT)
{
    return (fT > -41.0f && fT < 126.0f);
}

/*=================================================================================================
** @Name      : ReadBattInputs
** @Function  : Read all battery DPR inputs for this cycle.
**              Returns number of valid battery groups found (0 = skip this cycle).
**              Populates pThm->fTCellMax, fTCellMin, fBattBusCurr, fAvgSOC,
**              nPCSState, nHVFaultCount.
**===============================================================================================*/
static int ReadBattInputs(THERMAL_CONTROL *pThm)
{
    SAMPLER       *pSampler = (SAMPLER *)pThm->hSampler;
    DATA_PROCESSOR *pDPR     = (DATA_PROCESSOR *)pSampler->hDPR;
    VAR_VALUE varV;
    int n;
    int nValidGroups = 0;
    float fCellMax = -999.0f;
    float fCellMin =  999.0f;
    int   nHVFaults = 0;

    pThm->fBattBusCurr = 0.0f;
    pThm->fAvgSOC      = 0.0f;

    /* --- Read all battery groups --- */
    for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
    {
        int nDevId = THM_BATT_DEV_BASE + n;

        /* Read T_cellMax (558) */
        memset(&varV, 0, sizeof(varV));
        if(pDPR->Get(pThm->hSampler, GETTYPE_SP_VAR, nDevId, THM_SRC_T_CELL_MAX, &varV, 4) == ERR_DPR_OK)
        {
            if(IsValidTemp(varV.fValue) && varV.fValue > fCellMax)
                fCellMax = varV.fValue;
        }

        /* Read T_cellMin (559) */
        memset(&varV, 0, sizeof(varV));
        if(pDPR->Get(pThm->hSampler, GETTYPE_SP_VAR, nDevId, THM_SRC_T_CELL_MIN, &varV, 4) == ERR_DPR_OK)
        {
            if(IsValidTemp(varV.fValue) && varV.fValue < fCellMin)
                fCellMin = varV.fValue;
        }

        /* Check if this group is present (T_cellMax > -50°C is a valid battery) */
        if(varV.fValue > -50.0f)
        {
            nValidGroups++;

            /* Read HV status (553) */
            memset(&varV, 0, sizeof(varV));
            if(pDPR->Get(pThm->hSampler, GETTYPE_SP_VAR, nDevId, THM_SRC_HV_STATUS, &varV, 4) == ERR_DPR_OK)
            {
                if(varV.emValue == 3)  /* 3 = fault */
                    nHVFaults++;
            }
        }
    }

    /* --- Read RPMsg BLOBs from devId=500 (核间通信 aggregated per-cluster data) ---
     * Phase 2.1: read BOTH clusters (index 0 and 1) instead of only cluster 0. */
    {
        A_BLOB_STATUS_STAT statusStat;
        A_BLOB_ENERGY_STAT energyStat;
        int i;

        ZERO_POBJS(&statusStat, 1);
        memset(&varV, 0, sizeof(varV));
        varV.blobValue = (BYTE *)&statusStat;
        pThm->bStatusStatOk = FALSE;
        if(pDPR->Get(pThm->hSampler, GETTYPE_SP_VAR, THM_RPMSG_DEV_ID,
                      THM_SRC_RPMSG_STATUS, &varV, sizeof(A_BLOB_STATUS_STAT)) == ERR_DPR_OK)
        {
            pThm->bStatusStatOk = TRUE;
            for(i = 0; i < MAX_BATT_GROUP_NUM; i++)
            {
                /* Per-cluster signed current (neg=charge) and charge/discharge state */
                pThm->fClusterCurr[i]      = statusStat.global_info[i].total_current;
                pThm->nClusterChgDischg[i] = (int)statusStat.status_info[i].charge_discharge_status;
                /* Per-cluster voltage (comm presence) and work status (fault detection) */
                pThm->fClusterVolt[i]       = statusStat.global_info[i].total_voltage;
                pThm->nClusterWorkStatus[i] = (int)statusStat.status_info[i].work_status;
                /* Per-cluster cell temps (spec 5.2.1: same/cross-cluster diff);
                 * already in degC (BLOB source multiplies raw 0.1C wire value by 0.1). */
                pThm->fClusterTMax[i] = statusStat.feature_info[i].max_temperature_value;
                pThm->fClusterTMin[i] = statusStat.feature_info[i].min_temperature_value;
                pThm->fClusterTAvg[i] = statusStat.feature_info[i].cell_avg_temperature;
            }

            /* Backward-compat: cluster 0 still drives the legacy scalar fields until
             * Sample() is migrated to per-cluster in Phase 2.3. */
            pThm->fBattBusCurr = pThm->fClusterCurr[0];
            switch(pThm->nClusterChgDischg[0])
            {
                case 1:  /* charging */
                    pThm->nPCSState = STBATT_RUNSTAT_CHARGE;
                    break;
                case 2:  /* discharging */
                    pThm->nPCSState = STBATT_RUNSTAT_DISCHARGE;
                    break;
                default: /* 0 = idle */
                    pThm->nPCSState = STBATT_RUNSTAT_IDLE;
                    break;
            }
        }

        /* Also read energy blob for per-cluster SOC/SOH (param 509) */
        ZERO_POBJS(&energyStat, 1);
        memset(&varV, 0, sizeof(varV));
        varV.blobValue = (BYTE *)&energyStat;
        pThm->bEnergyStatOk = FALSE;
        if(pDPR->Get(pThm->hSampler, GETTYPE_SP_VAR, THM_RPMSG_DEV_ID,
                      THM_SRC_RPMSG_ENERGY, &varV, sizeof(A_BLOB_ENERGY_STAT)) == ERR_DPR_OK)
        {
            pThm->bEnergyStatOk = TRUE;
            for(i = 0; i < MAX_BATT_GROUP_NUM; i++)
            {
                /* SOC/SOH stored as 千分比 (0-1000), divide by 10 for percent */
                pThm->fClusterSoc[i] = (float)energyStat.power_info[i].soc / 10.0f;
                pThm->fClusterSoh[i] = (float)energyStat.power_info[i].soh / 10.0f;
            }
            pThm->fAvgSOC = pThm->fClusterSoc[0];
        }
    }

    pThm->fTCellMax      = (fCellMax > -999.0f) ? fCellMax : 0.0f;
    pThm->fTCellMin      = (fCellMin <  999.0f) ? fCellMin : 0.0f;
    pThm->nHVFaultCount  = nHVFaults;
    /* Installed cluster count from comm presence (836=2 clusters, 单簇240=1).
     * Uses last-good voltage; bStatusStatOk=FALSE zeroes load ratios this cycle anyway. */
    pThm->nClusterCount = 0;
    for(n = 0; n < MAX_BATT_GROUP_NUM; n++)
    {
        if(pThm->fClusterVolt[n] > 0.1f)
            pThm->nClusterCount++;
    }

    return nValidGroups;
}

/*=================================================================================================
** @Name      : ComputeLoadRatios
** @Function  : Compute per-cluster load ratios from this cycle's battery inputs (spec 2.3).
**              r_i      = |I_i| / I_rated,dir,i   (direction-specific rated current)
**              r_peak   = max(r_i) over valid clusters
**              r_total  = Sum|I_i| / Sum I_rated,dir,i  (denominator fixed by topology,
**                         NOT scaled by instantaneous online cluster count)
**              Invalid clusters (comm lost / fault / stale BLOB) are NOT substituted as 0 A;
**              they increment nDataDegrade and are excluded from numerator & peak.
**===============================================================================================*/
static void ComputeLoadRatios(THERMAL_CONTROL *pThm)
{
    int   i;
    float fSumAbsI  = 0.0f;   /* Sum|I_i| over valid clusters */
    float fSumRated = 0.0f;   /* Sum I_rated,dir,i over topology clusters */
    float fRPeak    = 0.0f;

    pThm->nDataDegrade = 0;

    for(i = 0; i < MAX_BATT_GROUP_NUM; i++)
    {
        float fAbsI;
        float fRated;

        /* Direction-specific rated current: charge vs discharge/idle */
        fRated = (pThm->nClusterChgDischg[i] == THM_CLUSTER_CHG)
                 ? pThm->fChargeRatedCurr
                 : pThm->fDischargeRatedCurr;

        /* Per-cluster validity: status BLOB fresh, comm present, no fault */
        if(pThm->bStatusStatOk
           && pThm->fClusterVolt[i] > 0.1f
           && pThm->nClusterWorkStatus[i] != 5)
        {
            fAbsI = (pThm->fClusterCurr[i] >= 0.0f)
                    ? pThm->fClusterCurr[i]
                    : -pThm->fClusterCurr[i];

            pThm->fClusterR[i]    = (fRated > 0.01f) ? (fAbsI / fRated) : 0.0f;
            pThm->bClusterValid[i] = TRUE;

            if(pThm->fClusterR[i] > fRPeak)
                fRPeak = pThm->fClusterR[i];

            fSumAbsI += fAbsI;
        }
        else
        {
            /* Invalid branch: do NOT substitute 0 A. Exclude from numerator/peak,
             * record degradation so higher layers can fall back to temp protection. */
            pThm->fClusterR[i]     = 0.0f;
            pThm->bClusterValid[i] = FALSE;
            pThm->nDataDegrade++;
        }
    }

    pThm->fRPeak = fRPeak;

    /* Denominator = topology-fixed Sum I_rated,dir,i (836=2x157, single-cluster 240=1x157). */
    for(i = 0; i < pThm->nClusterCount; i++)
    {
        fSumRated += (pThm->nClusterChgDischg[i] == THM_CLUSTER_CHG)
                     ? pThm->fChargeRatedCurr
                     : pThm->fDischargeRatedCurr;
    }

    pThm->fRTotal = (fSumRated > 0.01f) ? (fSumAbsI / fSumRated) : 0.0f;
}

/*=================================================================================================
** @Name      : PiecewiseLinear
** @Function  : 4-segment piecewise-linear interpolation over breakpoints r={0.3,0.6,0.8,1.0}.
**              d03/d06/d08/d10 are the curve values at those breakpoints; r>1.0 is capped at d10.
**===============================================================================================*/
static float PiecewiseLinear(float fR, float fD03, float fD06, float fD08, float fD10)
{
    if(fR <= 0.3f)
        return fD03;
    else if(fR <= 0.6f)
        return fD03 + (fR - 0.3f) * (fD06 - fD03) / 0.3f;
    else if(fR <= 0.8f)
        return fD06 + (fR - 0.6f) * (fD08 - fD06) / 0.2f;
    else if(fR <= 1.0f)
        return fD08 + (fR - 0.8f) * (fD10 - fD08) / 0.2f;
    else
        return fD10;
}

/*=================================================================================================
** @Name      : ComputeFeedforward
** @Function  : Per-cluster current feedforward dT_current,i (spec 2.4 / 2.5).
**              dT_current,i = max( F_dir,total(r_total), F_dir,peak(r_peak) )
**              F_dir is the charge or discharge curve selected by that cluster's R-core
**              charge_discharge_status. Idle/invalid clusters contribute 0 (no cooling demand).
**              Multi-cluster direction conflict: each cluster uses its own curve, then the
**              shared cooler takes the strongest demand (spec: 各簇按自身方向计算，取更强需求).
**===============================================================================================*/
static void ComputeFeedforward(THERMAL_CONTROL *pThm)
{
    int i;

    for(i = 0; i < MAX_BATT_GROUP_NUM; i++)
    {
        float fDTotal;
        float fDPeak;

        /* Idle clusters emit no heat and must not create cooling demand;
         * invalid clusters are excluded (data degradation, not 0-substitution). */
        if(!pThm->bClusterValid[i] || pThm->nClusterChgDischg[i] == THM_CLUSTER_IDLE)
        {
            pThm->fClusterDtCurr[i] = 0.0f;
            continue;
        }

        /* Select charge vs discharge curve per cluster direction */
        if(pThm->nClusterChgDischg[i] == THM_CLUSTER_CHG)
        {
            fDTotal = PiecewiseLinear(pThm->fRTotal,
                                       pThm->fFChgTotalDt03, pThm->fFChgTotalDt06,
                                       pThm->fFChgTotalDt08, pThm->fFChgTotalDt10);
            fDPeak  = PiecewiseLinear(pThm->fRPeak,
                                       pThm->fFChgPeakDt03, pThm->fFChgPeakDt06,
                                       pThm->fFChgPeakDt08, pThm->fFChgPeakDt10);
        }
        else
        {
            fDTotal = PiecewiseLinear(pThm->fRTotal,
                                       pThm->fFDchgTotalDt03, pThm->fFDchgTotalDt06,
                                       pThm->fFDchgTotalDt08, pThm->fFDchgTotalDt10);
            fDPeak  = PiecewiseLinear(pThm->fRPeak,
                                       pThm->fFDchgPeakDt03, pThm->fFDchgPeakDt06,
                                       pThm->fFDchgPeakDt08, pThm->fFDchgPeakDt10);
        }

        pThm->fClusterDtCurr[i] = (fDTotal > fDPeak) ? fDTotal : fDPeak;
    }
}

/*=================================================================================================
** @Name      : ComputeSocSohCorrection
** @Function  : Per-cluster SOC/SOH cooling correction (spec 2.5).
**              dT_SOC,i: charge = high-SOC extra heat; discharge = low-SOC inner-resistance risk.
**                F_SOC,chg = clamp((SOC_i - Enter) * Slope, 0, Limit)  when SOC_i > Enter
**                F_SOC,dchg= clamp((Enter - SOC_i) * Slope, 0, Limit) when SOC_i < Enter
**              dT_SOH,i: all conditions, below SOH base rises linearly (spec: SOH<90% ramps up,
**                +1 degC per 5% drop, capped 2 degC) => clamp((Base - SOH_i)/10 * Slope, 0, Limit).
**              Enter/Exit thresholds form hysteresis at the state-machine level (Phase 3); Exit is
**              NOT consumed here. Stale/missing energy BLOB => zero correction + data degradation,
**              never substitute another cluster or treat missing SOH as 0%.
**===============================================================================================*/
static void ComputeSocSohCorrection(THERMAL_CONTROL *pThm)
{
    int i;

    for(i = 0; i < MAX_BATT_GROUP_NUM; i++)
    {
        float fDtSoc = 0.0f;
        float fDtSoh = 0.0f;

        /* Invalid cluster: zero correction (degradation already counted in ComputeLoadRatios) */
        if(!pThm->bClusterValid[i])
        {
            pThm->fClusterDtSoc[i] = 0.0f;
            pThm->fClusterDtSoh[i] = 0.0f;
            continue;
        }

        /* --- SOC/SOH need a fresh energy BLOB; stale values must not drive cooling --- */
        if(pThm->bEnergyStatOk)
        {
            if(pThm->nClusterChgDischg[i] == THM_CLUSTER_CHG)
            {
                if(pThm->fClusterSoc[i] > pThm->fSocChgEnter)
                {
                    fDtSoc = (pThm->fClusterSoc[i] - pThm->fSocChgEnter) * pThm->fSocChgSlope;
                    if(fDtSoc > pThm->fSocChgLimit) fDtSoc = pThm->fSocChgLimit;
                    if(fDtSoc < 0.0f)           fDtSoc = 0.0f;
                }
            }
            else if(pThm->nClusterChgDischg[i] == THM_CLUSTER_DCHG)
            {
                if(pThm->fClusterSoc[i] < pThm->fSocDchgEnter)
                {
                    fDtSoc = (pThm->fSocDchgEnter - pThm->fClusterSoc[i]) * pThm->fSocDchgSlope;
                    if(fDtSoc > pThm->fSocDchgLimit) fDtSoc = pThm->fSocDchgLimit;
                    if(fDtSoc < 0.0f)             fDtSoc = 0.0f;
                }
            }
            /* IDLE clusters: no SOC direction, no correction */

            if(pThm->fClusterSoh[i] < pThm->fSohBase)
            {
                fDtSoh = (pThm->fSohBase - pThm->fClusterSoh[i]) / 10.0f * pThm->fSohSlope;
                if(fDtSoh > pThm->fSohLimit) fDtSoh = pThm->fSohLimit;
                if(fDtSoh < 0.0f)          fDtSoh = 0.0f;
            }
        }
        else
        {
            /* Energy BLOB stale/missing: zero both corrections, count one degradation */
            pThm->nDataDegrade++;
        }

        pThm->fClusterDtSoc[i] = fDtSoc;
        pThm->fClusterDtSoh[i] = fDtSoh;
    }
}

/*=================================================================================================
** @Name      : ComputeTargetTemperatures
** @Function  : Compute per-cluster and system target water temperature (spec 2.6).
**              T_prelim,i = T_base - dT_current,i - dT_SOC,i - dT_SOH,i
**              T_target,i = clamp(T_prelim,i, [T_target_min, T_target_max])
**              T_target,system = min(T_target,i) over valid clusters.
**              Invalid clusters hold T_base (never used in the min).
**===============================================================================================*/
static void ComputeTargetTemperatures(THERMAL_CONTROL *pThm)
{
    int   i;
    float fTMin     = 999.0f;
    BOOL  bAnyValid = FALSE;

    for(i = 0; i < MAX_BATT_GROUP_NUM; i++)
    {
        float fT;

        if(!pThm->bClusterValid[i])
        {
            pThm->fClusterTTarget[i] = pThm->fTTargetBase;
            continue;
        }

        fT = pThm->fTTargetBase
           - pThm->fClusterDtCurr[i]
           - pThm->fClusterDtSoc[i]
           - pThm->fClusterDtSoh[i];

        if(fT < pThm->fTTargetMin) fT = pThm->fTTargetMin;
        if(fT > pThm->fTTargetMax) fT = pThm->fTTargetMax;

        pThm->fClusterTTarget[i] = fT;
        bAnyValid = TRUE;
        if(fT < fTMin)
            fTMin = fT;
    }

    pThm->fTTargetSystem = bAnyValid ? fTMin : pThm->fTTargetBase;
}

/*=================================================================================================
** @Name      : ComputeSelfCircDiffs
** @Function  : Compute same-cluster and cross-cluster temperature diffs (spec 5.2/5.2.1).
**              dT_same,i = T_max,i - T_min,i  (per cluster, valid temps only)
**              fSameClusterDiff = max over valid clusters
**              dT_cross = |T_avg,1 - T_avg,2|  (dual-cluster projects only)
**              Single-cluster projects (240) never compute cross-cluster diff.
**              Invalid temps (sentinel -41/126) are excluded — they never trigger
**              spurious self-circulation.
**===============================================================================================*/
static void ComputeSelfCircDiffs(THERMAL_CONTROL *pThm)
{
    int   i;
    float fSameMax = 0.0f;
    float fCross   = 0.0f;
    BOOL  bDual    = (pThm->nClusterCount >= 2);

    /* --- same-cluster diff: max over valid clusters of (T_max - T_min) --- */
    for(i = 0; i < MAX_BATT_GROUP_NUM; i++)
    {
        float fTMax = pThm->fClusterTMax[i];
        float fTMin = pThm->fClusterTMin[i];

        if(!pThm->bClusterValid[i])
            continue;
        if(!IsValidTemp(fTMax) || !IsValidTemp(fTMin))
            continue;

        {
            float fDiff = fTMax - fTMin;
            if(fDiff > fSameMax)
                fSameMax = fDiff;
        }
    }
    pThm->fSameClusterDiff = fSameMax;

    /* --- cross-cluster diff: |T_avg,1 - T_avg,2|, dual-cluster only --- */
    if(bDual
       && pThm->bClusterValid[0] && pThm->bClusterValid[1]
       && IsValidTemp(pThm->fClusterTAvg[0]) && IsValidTemp(pThm->fClusterTAvg[1]))
    {
        float fDiff = pThm->fClusterTAvg[0] - pThm->fClusterTAvg[1];
        fCross = (fDiff >= 0.0f) ? fDiff : -fDiff;
    }
    pThm->fCrossClusterDiff = fCross;
}

/*=================================================================================================
** @Name      : ComputeColdHeatDemand
** @Function  : Per-cluster cooling/heating demand + cold/heat interlock inputs (spec 4.1/4.2).
**              Cooling demand,i : T_max,i >= fColdOnMax  OR  T_avg,i >= fColdOnAvg
**              Heating demand,i : T_avg,i <= fHeatOnAvg  OR  T_min,i <= fHeatOnMin
**              fSysClusterTMax   : max valid cluster T_max (degC); -999 if none valid.
**              bColdHeatConflict : cooling and heating demand coexist -> cooling wins (spec 4.2).
**              One-shot conflict report via bPrevColdHeatConflict rising edge.
**===============================================================================================*/
static void ComputeColdHeatDemand(THERMAL_CONTROL *pThm)
{
    int   i;
    float fSysMax = -999.0f;
    int   nCool = 0;
    int   nHeat = 0;

    for(i = 0; i < MAX_BATT_GROUP_NUM; i++)
    {
        float fTMax = pThm->fClusterTMax[i];
        float fTMin = pThm->fClusterTMin[i];
        float fTAvg = pThm->fClusterTAvg[i];

        if(!pThm->bClusterValid[i])
            continue;
        if(!IsValidTemp(fTMax))
            continue;

        if(fTMax > fSysMax)
            fSysMax = fTMax;

        /* Cooling demand (spec 4.1): max temp or avg temp exceeds cooling-on threshold */
        if(fTMax >= pThm->fColdOnMax
           || (IsValidTemp(fTAvg) && fTAvg >= pThm->fColdOnAvg))
            nCool++;

        /* Heating demand (spec 4.1): avg temp <= heatOnAvg OR min temp <= heatOnMin */
        if((IsValidTemp(fTAvg) && fTAvg <= pThm->fHeatOnAvg)
           || (IsValidTemp(fTMin) && fTMin <= pThm->fHeatOnMin))
            nHeat++;
    }

    pThm->fSysClusterTMax     = fSysMax;
    pThm->nCoolDemandClusters = nCool;
    pThm->nHeatDemandClusters = nHeat;
    pThm->bColdHeatConflict   = (nCool > 0 && nHeat > 0) ? TRUE : FALSE;

    /* One-shot conflict report (spec 4.2: 优先制冷并上报冷热需求冲突) */
    if(pThm->bColdHeatConflict && !pThm->bPrevColdHeatConflict)
    {
        TRACED("[thermal] cold/heat demand conflict: cool=%d clusters, heat=%d clusters; prioritizing cooling",
               nCool, nHeat);
    }
    pThm->bPrevColdHeatConflict = pThm->bColdHeatConflict;
}





/*=================================================================================================
** @Name      : LoadThresholds
** @Function  : Read thermal thresholds (8000-8003) from DPR setting branch.
**              Called once after init and whenever thresholds change.
**===============================================================================================*/
static void LoadThresholds(THERMAL_CONTROL *pThm)
{
    SAMPLER       *pSampler = (SAMPLER *)pThm->hSampler;
    DATA_PROCESSOR *pDPR     = (DATA_PROCESSOR *)pSampler->hDPR;
    VAR_VALUE varV;

    /* Try reading from DPR Setting branch.  Use battery devId (501 base).
     * These are SettingParameters on DeviceModel 501 (Battery). */
    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_BATT_DEV_BASE,
                  THM_SRC_T_COLD_ON, &varV, 4) == ERR_DPR_OK)
        pThm->fTColdOn = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_BATT_DEV_BASE,
                  THM_SRC_T_COLD_OFF, &varV, 4) == ERR_DPR_OK)
        pThm->fTColdOff = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_BATT_DEV_BASE,
                  THM_SRC_T_HEAT_ON, &varV, 4) == ERR_DPR_OK)
        pThm->fTHeatOn = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_BATT_DEV_BASE,
                  THM_SRC_T_HEAT_OFF, &varV, 4) == ERR_DPR_OK)
        pThm->fTHeatOff = varV.fValue;

    pThm->bThresholdsLoaded = TRUE;
    TRACED("[thermal] Thresholds loaded: coldOn=%.1f coldOff=%.1f heatOn=%.1f heatOff=%.1f",
           pThm->fTColdOn, pThm->fTColdOff, pThm->fTHeatOn, pThm->fTHeatOff);
}

/*=================================================================================================
** @Name      : LoadCoefficients
** @Function  : Read formula coefficients (2408-2417) from DPR setting branch.
**              Mirrors LoadThresholds but reads from devId=951 (aircon) instead of devId=501 (battery).
**              Called every Sample() cycle so Web edits take effect without restart.
**===============================================================================================*/
static void LoadCoefficients(THERMAL_CONTROL *pThm)
{
    SAMPLER        *pSampler = (SAMPLER *)pThm->hSampler;
    DATA_PROCESSOR *pDPR     = (DATA_PROCESSOR *)pSampler->hDPR;
    VAR_VALUE varV;

    /* Read formula coefficients (params 2408-2417) from DPR setting branch.
     * These are SettingParameters on DeviceModel 950 (Air Conditioner), devId=951. */

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_I_OFFSET_L1, &varV, 4) == ERR_DPR_OK)
        pThm->fIOffsetL1 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_I_OFFSET_L2, &varV, 4) == ERR_DPR_OK)
        pThm->fIOffsetL2 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_I_OFFSET_L3, &varV, 4) == ERR_DPR_OK)
        pThm->fIOffsetL3 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_I_OFFSET_L4, &varV, 4) == ERR_DPR_OK)
        pThm->fIOffsetL4 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_PCS_OFFSET_STBY, &varV, 4) == ERR_DPR_OK)
        pThm->fPCSOffsetStby = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_PCS_OFFSET_CHG, &varV, 4) == ERR_DPR_OK)
        pThm->fPCSOffsetChg = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_PCS_OFFSET_DCHG, &varV, 4) == ERR_DPR_OK)
        pThm->fPCSOffsetDchg = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_T_TARGET_BASE, &varV, 4) == ERR_DPR_OK)
        pThm->fTTargetBase = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_MIN_RUN_SEC, &varV, 4) == ERR_DPR_OK)
        pThm->nMinRunSec = (int)varV.lValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_MIN_STOP_SEC, &varV, 4) == ERR_DPR_OK)
    pThm->nMinStopSec = (int)varV.lValue;

    /* Read AC install type (param 448) from devId=951 to select TF/EU3 push path */
    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_AIR_TYPE, &varV, 4) == ERR_DPR_OK)
        pThm->nAirType = (int)varV.emValue;

    /* Read newly parameterized strategy data (params 2418-2431) */
    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_COLD_DIFF, &varV, 4) == ERR_DPR_OK)
        pThm->fColdDiff = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_COLD_DEEP_DELTA, &varV, 4) == ERR_DPR_OK)
        pThm->fColdDeepDelta = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_COLD_DEEP_DIFF, &varV, 4) == ERR_DPR_OK)
        pThm->fColdDeepDiff = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_HEAT_TARGET, &varV, 4) == ERR_DPR_OK)
        pThm->fHeatTarget = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_HEAT_DIFF, &varV, 4) == ERR_DPR_OK)
        pThm->fHeatDiff = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_DEEPCOOL_ESC_SEC, &varV, 4) == ERR_DPR_OK)
        pThm->nDeepcoolEscSec = (int)varV.lValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_EMERG_TRIGGER, &varV, 4) == ERR_DPR_OK)
        pThm->fEmergTrigger = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_EMERG_EXIT, &varV, 4) == ERR_DPR_OK)
        pThm->fEmergExit = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_EMERG_TARGET, &varV, 4) == ERR_DPR_OK)
        pThm->fEmergTarget = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_EMERG_EXIT_SEC, &varV, 4) == ERR_DPR_OK)
        pThm->nEmergExitSec = (int)varV.lValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_I_RATIO_L1, &varV, 4) == ERR_DPR_OK)
        pThm->fIRatioL1 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_I_RATIO_L2, &varV, 4) == ERR_DPR_OK)
        pThm->fIRatioL2 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_I_RATIO_L3, &varV, 4) == ERR_DPR_OK)
        pThm->fIRatioL3 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_STARTUP_PROT_SEC, &varV, 4) == ERR_DPR_OK)
        pThm->nStartupProtectSec = (int)varV.lValue;

    /* Read Phase 1 decision & time-protection params (2432-2442) */
    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_COLD_ON_AVG, &varV, 4) == ERR_DPR_OK)
        pThm->fColdOnAvg = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_COLD_OFF_AVG, &varV, 4) == ERR_DPR_OK)
        pThm->fColdOffAvg = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_COLD_ON_MAX, &varV, 4) == ERR_DPR_OK)
        pThm->fColdOnMax = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_COLD_OFF_MAX, &varV, 4) == ERR_DPR_OK)
        pThm->fColdOffMax = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_COLD_EXIT_SEC, &varV, 4) == ERR_DPR_OK)
        pThm->nColdExitSec = (int)varV.lValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_HEAT_ON_AVG, &varV, 4) == ERR_DPR_OK)
        pThm->fHeatOnAvg = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_HEAT_ON_MIN, &varV, 4) == ERR_DPR_OK)
        pThm->fHeatOnMin = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_HEAT_OFF_AVG, &varV, 4) == ERR_DPR_OK)
        pThm->fHeatOffAvg = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_HEAT_OFF_MIN, &varV, 4) == ERR_DPR_OK)
        pThm->fHeatOffMin = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_HEAT_EXIT_SEC, &varV, 4) == ERR_DPR_OK)
        pThm->nHeatExitSec = (int)varV.lValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_MODE_SWITCH_SEC, &varV, 4) == ERR_DPR_OK)
        pThm->nModeSwitchSec = (int)varV.lValue;

    /* Read Phase 1 current rating & self-circulation params (2443-2451) */
    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_CHARGE_RATED_CURR, &varV, 4) == ERR_DPR_OK)
        pThm->fChargeRatedCurr = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_DISCHARGE_RATED_CURR, &varV, 4) == ERR_DPR_OK)
        pThm->fDischargeRatedCurr = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_EVAL_CURR_WINDOW_SEC, &varV, 4) == ERR_DPR_OK)
        pThm->nEvalCurrWindowSec = (int)varV.lValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_SAME_CLUSTER_DIFF_ENTER, &varV, 4) == ERR_DPR_OK)
        pThm->fSameClusterDiffEnter = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_SAME_CLUSTER_DIFF_EXIT, &varV, 4) == ERR_DPR_OK)
        pThm->fSameClusterDiffExit = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_CROSS_CLUSTER_DIFF_ENTER, &varV, 4) == ERR_DPR_OK)
        pThm->fCrossClusterDiffEnter = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_CROSS_CLUSTER_DIFF_EXIT, &varV, 4) == ERR_DPR_OK)
        pThm->fCrossClusterDiffExit = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_SELF_CIRC_MAX_SEC, &varV, 4) == ERR_DPR_OK)
        pThm->nSelfCircMaxSec = (int)varV.lValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_SELF_CIRC_INTERVAL_SEC, &varV, 4) == ERR_DPR_OK)
        pThm->nSelfCircIntervalSec = (int)varV.lValue;

    /* Self-circulation mode-3 enable gate (param 2494): 0=off until TF/EU3 protocol
     * mode 3 is confirmed on real hardware (spec 3.3 / 5.2). */
    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_SELF_CIRC_ENABLE, &varV, 4) == ERR_DPR_OK)
        pThm->nSelfCircEnable = (int)varV.lValue;

    /* Read Phase 2 automatic-mode params (2452-2493) */
    /* A. current feedforward curves F(r) (sec 2.4) */
    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_F_CHG_TOTAL_DT_03, &varV, 4) == ERR_DPR_OK)
        pThm->fFChgTotalDt03 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_F_CHG_TOTAL_DT_06, &varV, 4) == ERR_DPR_OK)
        pThm->fFChgTotalDt06 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_F_CHG_TOTAL_DT_08, &varV, 4) == ERR_DPR_OK)
        pThm->fFChgTotalDt08 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_F_CHG_TOTAL_DT_10, &varV, 4) == ERR_DPR_OK)
        pThm->fFChgTotalDt10 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_F_CHG_PEAK_DT_03, &varV, 4) == ERR_DPR_OK)
        pThm->fFChgPeakDt03 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_F_CHG_PEAK_DT_06, &varV, 4) == ERR_DPR_OK)
        pThm->fFChgPeakDt06 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_F_CHG_PEAK_DT_08, &varV, 4) == ERR_DPR_OK)
        pThm->fFChgPeakDt08 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_F_CHG_PEAK_DT_10, &varV, 4) == ERR_DPR_OK)
        pThm->fFChgPeakDt10 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_F_DCHG_TOTAL_DT_03, &varV, 4) == ERR_DPR_OK)
        pThm->fFDchgTotalDt03 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_F_DCHG_TOTAL_DT_06, &varV, 4) == ERR_DPR_OK)
        pThm->fFDchgTotalDt06 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_F_DCHG_TOTAL_DT_08, &varV, 4) == ERR_DPR_OK)
        pThm->fFDchgTotalDt08 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_F_DCHG_TOTAL_DT_10, &varV, 4) == ERR_DPR_OK)
        pThm->fFDchgTotalDt10 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_F_DCHG_PEAK_DT_03, &varV, 4) == ERR_DPR_OK)
        pThm->fFDchgPeakDt03 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_F_DCHG_PEAK_DT_06, &varV, 4) == ERR_DPR_OK)
        pThm->fFDchgPeakDt06 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_F_DCHG_PEAK_DT_08, &varV, 4) == ERR_DPR_OK)
        pThm->fFDchgPeakDt08 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_F_DCHG_PEAK_DT_10, &varV, 4) == ERR_DPR_OK)
        pThm->fFDchgPeakDt10 = varV.fValue;
    /* B. SOC/SOH correction (sec 2.5) */

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_SOC_CHG_ENTER, &varV, 4) == ERR_DPR_OK)
        pThm->fSocChgEnter = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_SOC_CHG_EXIT, &varV, 4) == ERR_DPR_OK)
        pThm->fSocChgExit = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_SOC_CHG_SLOPE, &varV, 4) == ERR_DPR_OK)
        pThm->fSocChgSlope = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_SOC_CHG_LIMIT, &varV, 4) == ERR_DPR_OK)
        pThm->fSocChgLimit = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_SOC_DCHG_ENTER, &varV, 4) == ERR_DPR_OK)
        pThm->fSocDchgEnter = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_SOC_DCHG_EXIT, &varV, 4) == ERR_DPR_OK)
        pThm->fSocDchgExit = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_SOC_DCHG_SLOPE, &varV, 4) == ERR_DPR_OK)
        pThm->fSocDchgSlope = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_SOC_DCHG_LIMIT, &varV, 4) == ERR_DPR_OK)
        pThm->fSocDchgLimit = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_SOH_BASE, &varV, 4) == ERR_DPR_OK)
        pThm->fSohBase = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_SOH_SLOPE, &varV, 4) == ERR_DPR_OK)
        pThm->fSohSlope = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_SOH_LIMIT, &varV, 4) == ERR_DPR_OK)
        pThm->fSohLimit = varV.fValue;
    /* C. target water temp bounds (sec 2.6) */
    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_T_TARGET_MIN, &varV, 4) == ERR_DPR_OK)
        pThm->fTTargetMin = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_T_TARGET_MAX, &varV, 4) == ERR_DPR_OK)
        pThm->fTTargetMax = varV.fValue;
    /* D. cooling intensity boundary + cool-exit load (sec 3.2/3.3) */
    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_FORCE_COOL_T_TARGET, &varV, 4) == ERR_DPR_OK)
        pThm->fForceCoolTTarget = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_FORCE_COOL_R_TOTAL, &varV, 4) == ERR_DPR_OK)
        pThm->fForceCoolRTotal = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_FORCE_COOL_R_PEAK, &varV, 4) == ERR_DPR_OK)
        pThm->fForceCoolRPeak = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_FORCE_COOL_T_MAX, &varV, 4) == ERR_DPR_OK)
        pThm->fForceCoolTMax = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_COOL_EXIT_R_TOTAL, &varV, 4) == ERR_DPR_OK)
        pThm->fCoolExitRTotal = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_COOL_EXIT_R_PEAK, &varV, 4) == ERR_DPR_OK)
        pThm->fCoolExitRPeak = varV.fValue;
    /* E. cold/heat interlock + low-temp charge (sec 4.2/4.3) */
    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_HEAT_BLOCK_T_MAX, &varV, 4) == ERR_DPR_OK)
        pThm->fHeatBlockTMax = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_LOWT_CHG_LV1, &varV, 4) == ERR_DPR_OK)
        pThm->fLowTChgLv1 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_LOWT_CHG_LV2, &varV, 4) == ERR_DPR_OK)
        pThm->fLowTChgLv2 = varV.fValue;

    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_LOWT_CHG_LV3, &varV, 4) == ERR_DPR_OK)
        pThm->fLowTChgLv3 = varV.fValue;


    TRACED("[thermal] Phase1 decision params: coldOn[avg=%.1f/max=%.1f] coldOff[avg=%.1f/max=%.1f] coldExit=%d "
           "heatOn[avg=%.1f/min=%.1f] heatOff[avg=%.1f/min=%.1f] heatExit=%d switchBuf=%d "
           "rated[chg=%.1f/dchg=%.1f] evalWin=%d "
           "diff[same=%.1f/%.1f cross=%.1f/%.1f] selfCirc[max=%d/int=%d]",
           pThm->fColdOnAvg, pThm->fColdOnMax, pThm->fColdOffAvg, pThm->fColdOffMax, pThm->nColdExitSec,
           pThm->fHeatOnAvg, pThm->fHeatOnMin, pThm->fHeatOffAvg, pThm->fHeatOffMin, pThm->nHeatExitSec,
           pThm->nModeSwitchSec,
           pThm->fChargeRatedCurr, pThm->fDischargeRatedCurr, pThm->nEvalCurrWindowSec,
           pThm->fSameClusterDiffEnter, pThm->fSameClusterDiffExit, pThm->fCrossClusterDiffEnter, pThm->fCrossClusterDiffExit,
           pThm->nSelfCircMaxSec, pThm->nSelfCircIntervalSec);

    TRACED("[thermal] Strategy params: coldDiff=%.1f deepDelta=%.1f deepDiff=%.1f "
           "heatTgt=%.1f heatDiff=%.1f deepEsc=%d emerg[%.1f/%.1f/%.1f/%d] "
           "ratio[%.2f/%.2f/%.2f] startupProt=%d",
           pThm->fColdDiff, pThm->fColdDeepDelta, pThm->fColdDeepDiff,
           pThm->fHeatTarget, pThm->fHeatDiff, pThm->nDeepcoolEscSec,
           pThm->fEmergTrigger, pThm->fEmergExit, pThm->fEmergTarget, pThm->nEmergExitSec,
           pThm->fIRatioL1, pThm->fIRatioL2, pThm->fIRatioL3, pThm->nStartupProtectSec);

    TRACED("[thermal] Coefficients loaded: L1=%.1f L2=%.1f L3=%.1f L4=%.1f "
           "PCS[s/c/d]=%.1f/%.1f/%.1f T_base=%.1f run=%d stop=%d airType=%d",
           pThm->fIOffsetL1, pThm->fIOffsetL2,
           pThm->fIOffsetL3, pThm->fIOffsetL4,
           pThm->fPCSOffsetStby, pThm->fPCSOffsetChg, pThm->fPCSOffsetDchg,
           pThm->fTTargetBase, pThm->nMinRunSec, pThm->nMinStopSec, pThm->nAirType);
}

/*=================================================================================================
** @Name      : PushACCommand
** @Function  : Push aircon control commands for the current state via Push_VarValue.
**              Writes to devId=951 (Air Conditioner#1), which triggers
**              aircon_sampler::Control() to write the corresponding Modbus registers.
**              Push order: setpoints → mode → power ON/OFF (last).
**===============================================================================================*/
static void PushACCommand(THERMAL_CONTROL *pThm)
{
    HANDLE hSamp = pThm->hSampler;
    int nDstDev  = THM_DST_DEV_ID;
    int nColdTarget    = (int)pThm->fTTargetSystem;
    int nColdDeepTgt   = (int)(pThm->fTTargetSystem - pThm->fColdDeepDelta);
    float fColdTarget  = pThm->fTTargetSystem;
    float fColdDeepTgt = pThm->fTTargetSystem - pThm->fColdDeepDelta;

    switch(pThm->eState)
    {
        case THM_STATE_STANDBY:
            pushParamEnum(hSamp, nDstDev, THM_DST_AIR_POWER, 0);
            break;

        case THM_STATE_COOLING:
            if(pThm->nAirType == 1) /* EU3 */
            {
                pushParamFloat(hSamp, nDstDev, THM_DST_EU3_COLD_TEMP,      fColdTarget);
                pushParamFloat(hSamp, nDstDev, THM_DST_EU3_COLD_OPEN_DIFF, pThm->fColdDiff);
                pushParamEnum (hSamp, nDstDev, THM_DST_EU3_CTRL_OBJ,       1);
                pushParamEnum (hSamp, nDstDev, THM_DST_EU3_MODE,           1);
                pushParamEnum (hSamp, nDstDev, THM_DST_AIR_POWER,          1);
            }
            else /* TF (default, also covers nAirType==0 unconfigured) */
            {
                pushParamInt (hSamp, nDstDev, THM_DST_TF_COLD_TEMP, nColdTarget);
                pushParamInt (hSamp, nDstDev, THM_DST_TF_COLD_DIFF, (int)pThm->fColdDiff);
                pushParamEnum(hSamp, nDstDev, THM_DST_TF_CTRL_OBJ,  1);
                pushParamEnum(hSamp, nDstDev, THM_DST_TF_MODE,      1);
                pushParamEnum(hSamp, nDstDev, THM_DST_AIR_POWER,    2);
            }
            break;

        case THM_STATE_DEEPCOOL:
            if(pThm->nAirType == 1) /* EU3 */
            {
                pushParamFloat(hSamp, nDstDev, THM_DST_EU3_COLD_TEMP,      fColdDeepTgt);
                pushParamFloat(hSamp, nDstDev, THM_DST_EU3_COLD_OPEN_DIFF, pThm->fColdDeepDiff);
                pushParamEnum (hSamp, nDstDev, THM_DST_EU3_CTRL_OBJ,       1);
                pushParamEnum (hSamp, nDstDev, THM_DST_EU3_MODE,           1);
                pushParamEnum (hSamp, nDstDev, THM_DST_AIR_POWER,          1);
            }
            else /* TF */
            {
                pushParamInt (hSamp, nDstDev, THM_DST_TF_COLD_TEMP, nColdDeepTgt);
                pushParamInt (hSamp, nDstDev, THM_DST_TF_COLD_DIFF, (int)pThm->fColdDeepDiff);
                pushParamEnum(hSamp, nDstDev, THM_DST_TF_CTRL_OBJ,  1);
                pushParamEnum(hSamp, nDstDev, THM_DST_TF_MODE,      1);
                pushParamEnum(hSamp, nDstDev, THM_DST_AIR_POWER,    2);
            }
            break;

        case THM_STATE_HEATING:
            if(pThm->nAirType == 1) /* EU3 */
            {
                pushParamFloat(hSamp, nDstDev, THM_DST_EU3_HEAT_TEMP,      pThm->fHeatTarget);
                pushParamFloat(hSamp, nDstDev, THM_DST_EU3_HEAT_OPEN_DIFF, pThm->fHeatDiff);
                pushParamEnum (hSamp, nDstDev, THM_DST_EU3_CTRL_OBJ,       1);
                pushParamEnum (hSamp, nDstDev, THM_DST_EU3_MODE,           2);
                pushParamEnum (hSamp, nDstDev, THM_DST_AIR_POWER,          1);
            }
            else /* TF */
            {
                pushParamInt (hSamp, nDstDev, THM_DST_TF_HEAT_TEMP, (int)pThm->fHeatTarget);
                pushParamInt (hSamp, nDstDev, THM_DST_TF_HEAT_DIFF, (int)pThm->fHeatDiff);
                pushParamEnum(hSamp, nDstDev, THM_DST_TF_CTRL_OBJ,  1);
                pushParamEnum(hSamp, nDstDev, THM_DST_TF_MODE,      2);
                pushParamEnum(hSamp, nDstDev, THM_DST_AIR_POWER,    4);
            }
            break;

        case THM_STATE_SELFCIRC:
            /* Mode 3 self-circulation (spec 5.2): circulate water without
             * compressor.  Gate: reached only when nSelfCircEnable != 0.
             * 参数448 chooses the protocol adapter; no compressor, no
             * unverified pump-speed writes. */
            if(pThm->nAirType == 1) /* EU3 */
            {
                pushParamEnum(hSamp, nDstDev, THM_DST_EU3_CTRL_OBJ, 1);
                pushParamEnum(hSamp, nDstDev, THM_DST_EU3_MODE,     3);
                pushParamEnum(hSamp, nDstDev, THM_DST_AIR_POWER,    0);
            }
            else /* TF */
            {
                pushParamEnum(hSamp, nDstDev, THM_DST_TF_CTRL_OBJ, 1);
                pushParamEnum(hSamp, nDstDev, THM_DST_TF_MODE,     3);
                pushParamEnum(hSamp, nDstDev, THM_DST_AIR_POWER,   0);
            }
            break;

        case THM_STATE_EMERGENCY_COOL:
            /* 15C target, no hysteresis, max aggression */
            if(pThm->nAirType == 1) /* EU3 */
            {
                pushParamFloat(hSamp, nDstDev, THM_DST_EU3_COLD_TEMP,      pThm->fEmergTarget);
                pushParamFloat(hSamp, nDstDev, THM_DST_EU3_COLD_OPEN_DIFF, 0.0f);
                pushParamEnum (hSamp, nDstDev, THM_DST_EU3_CTRL_OBJ,       1);
                pushParamEnum (hSamp, nDstDev, THM_DST_EU3_MODE,           1);
                pushParamEnum (hSamp, nDstDev, THM_DST_AIR_POWER,          1);
            }
            else /* TF */
            {
                pushParamInt (hSamp, nDstDev, THM_DST_TF_COLD_TEMP, (int)pThm->fEmergTarget);
                pushParamInt (hSamp, nDstDev, THM_DST_TF_COLD_DIFF, 0);
                pushParamEnum(hSamp, nDstDev, THM_DST_TF_CTRL_OBJ,  1);
                pushParamEnum(hSamp, nDstDev, THM_DST_TF_MODE,      1);
                pushParamEnum(hSamp, nDstDev, THM_DST_AIR_POWER,    2);
            }
            break;

        default:
            break;
    }

    /* Record last pushed cooling target for same-state resend detection (spec 2.10) */
    if(pThm->eState == THM_STATE_COOLING)
        pThm->fLastPushedCoolTarget = fColdTarget;
    else if(pThm->eState == THM_STATE_DEEPCOOL)
        pThm->fLastPushedCoolTarget = fColdDeepTgt;

    pThm->eLastPushedState = pThm->eState;
    pThm->tLastCommandAt   = getMonotonicSec();  /* spec 7.1: track last-command age */
}

/*=================================================================================================
** @Name      : RunStateMachine
** @Function  : Evaluate T_decision against hysteresis thresholds and time protection,
**              transitioning between STANDBY, COOLING, DEEPCOOL, and HEATING.
**
**              Hysteresis:
**                Cooling on:  T_decision >  fTColdOn (default 33°C)
**                Cooling off: T_decision ≤  fTColdOff (default 28°C)
**                Heating on:  T_decision <  fTHeatOn (default 5°C)
**                Heating off: T_decision ≥  fTHeatOff (default 12°C)
**
**              Time protection:
**                min_run:  minimum seconds in active state before STANDBY allowed
**                min_stop: minimum seconds in STANDBY before active state allowed
**
**              Escalation:
**                COOLING → DEEPCOOL after nDeepcoolEscSec (default 600s)
**                if T_decision still > T_coldOff
**                DEEPCOOL → COOLING when T_decision ≤ T_coldOn (down-escalate)
**
**              Returns TRUE if state changed, FALSE otherwise.
**===============================================================================================*/
static BOOL RunStateMachine(THERMAL_CONTROL *pThm)
{
    THM_STATE eOldState = pThm->eState;
    long long tNow     = getMonotonicSec();
    long long tElapsed = tNow - pThm->tStateEntered;
    float fTDec = pThm->fTDecision;

    switch(eOldState)
    {
        case THM_STATE_STANDBY:
            /* Enter COOLING if T too high AND min_stop expired (or first run) */
            if(fTDec > pThm->fTColdOn && tElapsed >= (time_t)pThm->nMinStopSec
               && (tNow - pThm->tLastHeatEnd) >= (long long)pThm->nModeSwitchSec)
            {
                pThm->eState = THM_STATE_COOLING;
            }
            /* Enter HEATING if T too low AND min_stop expired */
            else if(fTDec < pThm->fTHeatOn && tElapsed >= (time_t)pThm->nMinStopSec
                    && (tNow - pThm->tLastCoolEnd) >= (long long)pThm->nModeSwitchSec
                    && pThm->fSysClusterTMax <= pThm->fHeatBlockTMax
                    && !pThm->bColdHeatConflict)
            {
                pThm->eState = THM_STATE_HEATING;
            }
            /* Enter SELFCIRC (mode 3) if no cooling/heating demand, temp diff
             * reaches threshold, protocol confirmed, and re-trigger interval
             * elapsed (spec 5.2). Lower priority than cooling/heating. */
            else if(pThm->nSelfCircEnable != 0
                    && tElapsed >= (time_t)pThm->nMinStopSec
                    && (tNow - pThm->tSelfCircLastOff) >= (long long)pThm->nSelfCircIntervalSec
                    && ((pThm->fSameClusterDiff >= pThm->fSameClusterDiffEnter)
                        || (pThm->nClusterCount >= 2
                            && pThm->fCrossClusterDiff >= pThm->fCrossClusterDiffEnter)))
            {
                pThm->eState = THM_STATE_SELFCIRC;
            }
            break;

        case THM_STATE_COOLING:
            /* Exit to STANDBY: T is cool enough AND min_run expired */
            if(fTDec <= pThm->fTColdOff && tElapsed >= (time_t)pThm->nMinRunSec)
            {
                pThm->tLastCoolEnd = tNow;
                pThm->eState = THM_STATE_STANDBY;
            }
            /* Escalate to DEEPCOOL: been cooling too long and temp still high */
            else if(tElapsed >= (time_t)pThm->nDeepcoolEscSec && fTDec > pThm->fTColdOff)
            {
                pThm->eState = THM_STATE_DEEPCOOL;
            }
            break;

        case THM_STATE_DEEPCOOL:
            /* Exit to STANDBY: T is cool enough AND min_run expired */
            if(fTDec <= pThm->fTColdOff && tElapsed >= (time_t)pThm->nMinRunSec)
            {
                pThm->tLastCoolEnd = tNow;
                pThm->eState = THM_STATE_STANDBY;
            }
            /* Down-escalate: cooling is working, return to standard COOLING */
            else if(fTDec <= pThm->fTColdOn)
            {
                pThm->eState = THM_STATE_COOLING;
            }
            break;

        case THM_STATE_HEATING:
            /* Exit to STANDBY: T is warm enough AND min_run expired */
            if(fTDec >= pThm->fTHeatOff && tElapsed >= (time_t)pThm->nMinRunSec)
            {
                pThm->tLastHeatEnd = tNow;
                pThm->eState = THM_STATE_STANDBY;
            }
            else if(fTDec > pThm->fTColdOn || pThm->fSysClusterTMax > pThm->fHeatBlockTMax)
            {
                /* Cooling demand or hot-cluster block overrides heating (spec 4.2/5.1) */
                pThm->tLastHeatEnd = tNow;
                pThm->eState = THM_STATE_STANDBY;
            }
            break;

        case THM_STATE_SELFCIRC:
            /* Immediate exit on cooling/heating demand (spec 5.2: 出现冷热需求时立即退出) */
            if(fTDec > pThm->fTColdOn)
            {
                pThm->eState = THM_STATE_COOLING;
                pThm->tSelfCircLastOff = tNow;
            }
            else if(fTDec < pThm->fTHeatOn
                    && pThm->fSysClusterTMax <= pThm->fHeatBlockTMax
                    && !pThm->bColdHeatConflict)
            {
                pThm->eState = THM_STATE_HEATING;
                pThm->tSelfCircLastOff = tNow;
            }
            /* Exit: temp diff subsided OR max runtime reached (spec 5.2) */
            else if(tElapsed >= (long long)pThm->nSelfCircMaxSec
                    || (pThm->fSameClusterDiff < pThm->fSameClusterDiffExit
                        && (pThm->nClusterCount < 2
                            || pThm->fCrossClusterDiff < pThm->fCrossClusterDiffExit)))
            {
                pThm->eState = THM_STATE_STANDBY;
                pThm->tSelfCircLastOff = tNow;
            }
            break;

        default:
            pThm->eState = THM_STATE_STANDBY;
            break;
    }

    if(eOldState != pThm->eState)
    {
        pThm->tStateEntered = tNow;
        TRACED("[thermal] State transition: %d → %d (T_dec=%.1f, elapsed=%lds)",
               eOldState, pThm->eState, fTDec, (long)tElapsed);
        return TRUE;
    }
    return FALSE;
}

/*=================================================================================================
** @Name      : Initialize
** @Function  : Framework entry point. Alloc THERMAL_CONTROL, sets defaults.
**              No CommPort needed — this is a compute-only module.
**===============================================================================================*/
DLLExport BOOL Initialize(HANDLE hSamplerSelf)
{
    SAMPLER        *pSampler = (SAMPLER *)hSamplerSelf;
    THERMAL_CONTROL *pThm;

    pSampler->pDataHeap = (void *)malloc(sizeof(THERMAL_CONTROL));
    if(pSampler->pDataHeap == NULL)
    {
        TRACE("[thermal] Initialize: malloc failed\n");
        return FALSE;
    }
    memset(pSampler->pDataHeap, 0, sizeof(THERMAL_CONTROL));
    pThm = (THERMAL_CONTROL *)pSampler->pDataHeap;

    pThm->hSampler = hSamplerSelf;

    /* Default state: STANDBY, assume just entered */
    pThm->eState        = THM_STATE_STANDBY;
    pThm->tStateEntered = getMonotonicSec();
    pThm->nCycleCount   = 0;

    /* Default thresholds (overridden when DPR values loaded) */
    pThm->fTColdOn      = THM_DEFAULT_T_COLD_ON;
    pThm->fTColdOff     = THM_DEFAULT_T_COLD_OFF;
    pThm->fTHeatOn      = THM_DEFAULT_T_HEAT_ON;
    pThm->fTHeatOff     = THM_DEFAULT_T_HEAT_OFF;
    pThm->bThresholdsLoaded = FALSE;

    /* Default formula coefficients (overridden by LoadCoefficients on first cycle
     * or when Control() receives Web settings) */
    pThm->fIOffsetL1      = THM_DEFAULT_I_OFFSET_L1;
    pThm->fIOffsetL2      = THM_DEFAULT_I_OFFSET_L2;
    pThm->fIOffsetL3      = THM_DEFAULT_I_OFFSET_L3;
    pThm->fIOffsetL4      = THM_DEFAULT_I_OFFSET_L4;
    pThm->fPCSOffsetStby  = THM_DEFAULT_PCS_OFFSET_STBY;
    pThm->fPCSOffsetChg   = THM_DEFAULT_PCS_OFFSET_CHG;
    pThm->fPCSOffsetDchg  = THM_DEFAULT_PCS_OFFSET_DCHG;
    pThm->fTTargetBase    = THM_DEFAULT_T_TARGET_BASE;
    pThm->nMinRunSec      = THM_DEFAULT_MIN_RUN_SEC;
    pThm->nMinStopSec     = THM_DEFAULT_MIN_STOP_SEC;

    /* Default newly parameterized strategy data (params 2418-2431) */
    pThm->fColdDiff        = THM_DEFAULT_COLD_DIFF;
    pThm->fColdDeepDelta   = THM_DEFAULT_COLD_DEEP_DELTA;
    pThm->fColdDeepDiff    = THM_DEFAULT_COLD_DEEP_DIFF;
    pThm->fHeatTarget      = THM_DEFAULT_HEAT_TARGET;
    pThm->fHeatDiff        = THM_DEFAULT_HEAT_DIFF;
    pThm->nDeepcoolEscSec  = THM_DEFAULT_DEEPCOOL_ESC_SEC;
    pThm->fEmergTrigger    = THM_DEFAULT_EMERG_TRIGGER;
    pThm->fEmergExit       = THM_DEFAULT_EMERG_EXIT;
    pThm->fEmergTarget     = THM_DEFAULT_EMERG_TARGET;
    pThm->nEmergExitSec    = THM_DEFAULT_EMERG_EXIT_SEC;
    pThm->fIRatioL1        = THM_DEFAULT_I_RATIO_L1;
    pThm->fIRatioL2        = THM_DEFAULT_I_RATIO_L2;
    pThm->fIRatioL3        = THM_DEFAULT_I_RATIO_L3;
    pThm->nStartupProtectSec = THM_DEFAULT_STARTUP_PROT_SEC;

    /* Default Phase 1 decision & time-protection params (2432-2442) */
    pThm->fColdOnAvg          = THM_DEFAULT_COLD_ON_AVG;
    pThm->fColdOffAvg         = THM_DEFAULT_COLD_OFF_AVG;
    pThm->fColdOnMax          = THM_DEFAULT_COLD_ON_MAX;
    pThm->fColdOffMax         = THM_DEFAULT_COLD_OFF_MAX;
    pThm->nColdExitSec        = THM_DEFAULT_COLD_EXIT_SEC;
    pThm->fHeatOnAvg          = THM_DEFAULT_HEAT_ON_AVG;
    pThm->fHeatOnMin          = THM_DEFAULT_HEAT_ON_MIN;
    pThm->fHeatOffAvg         = THM_DEFAULT_HEAT_OFF_AVG;
    pThm->fHeatOffMin         = THM_DEFAULT_HEAT_OFF_MIN;
    pThm->nHeatExitSec        = THM_DEFAULT_HEAT_EXIT_SEC;
    pThm->nModeSwitchSec      = THM_DEFAULT_MODE_SWITCH_SEC;

    /* Default Phase 1 current rating & self-circulation params (2443-2451) */
    pThm->fChargeRatedCurr      = THM_DEFAULT_CHARGE_RATED_CURR;
    pThm->fDischargeRatedCurr   = THM_DEFAULT_DISCHARGE_RATED_CURR;
    pThm->nEvalCurrWindowSec    = THM_DEFAULT_EVAL_CURR_WINDOW_SEC;
    pThm->fSameClusterDiffEnter = THM_DEFAULT_SAME_CLUSTER_DIFF_ENTER;
    pThm->fSameClusterDiffExit  = THM_DEFAULT_SAME_CLUSTER_DIFF_EXIT;
    pThm->fCrossClusterDiffEnter= THM_DEFAULT_CROSS_CLUSTER_DIFF_ENTER;
    pThm->fCrossClusterDiffExit = THM_DEFAULT_CROSS_CLUSTER_DIFF_EXIT;
    pThm->nSelfCircMaxSec       = THM_DEFAULT_SELF_CIRC_MAX_SEC;
    pThm->nSelfCircIntervalSec  = THM_DEFAULT_SELF_CIRC_INTERVAL_SEC;
    pThm->nSelfCircEnable      = THM_DEFAULT_SELF_CIRC_ENABLE;
    pThm->tSelfCircLastOff    = -THM_DEFAULT_SELF_CIRC_INTERVAL_SEC; /* negative sentinel: never ran, so first trigger not gated by re-trigger interval */

    /* Phase 2.8 cold/heat interlock runtime defaults (spec 4.2).
     * Sentinels: fSysClusterTMax=-999 = no valid cluster yet; tLastCoolEnd/tLastHeatEnd
     * negative = never ran, so cool<->heat full-stop buffer is satisfied on first cycle. */
    pThm->fSysClusterTMax       = -999.0f;
    pThm->nCoolDemandClusters   = 0;
    pThm->nHeatDemandClusters   = 0;
    pThm->bColdHeatConflict     = FALSE;
    pThm->bPrevColdHeatConflict = FALSE;
    pThm->tLastCoolEnd          = -THM_DEFAULT_MODE_SWITCH_SEC;
    pThm->tLastHeatEnd          = -THM_DEFAULT_MODE_SWITCH_SEC;
    pThm->fLastPushedCoolTarget = -999.0f;  /* sentinel: never pushed (spec 2.10) */

    /* Default Phase 2 automatic-mode params (2452-2493) */
    /* A. current feedforward curves F(r) (sec 2.4) */
    pThm->fFChgTotalDt03           = THM_DEFAULT_F_CHG_TOTAL_DT_03;
    pThm->fFChgTotalDt06           = THM_DEFAULT_F_CHG_TOTAL_DT_06;
    pThm->fFChgTotalDt08           = THM_DEFAULT_F_CHG_TOTAL_DT_08;
    pThm->fFChgTotalDt10           = THM_DEFAULT_F_CHG_TOTAL_DT_10;
    pThm->fFChgPeakDt03            = THM_DEFAULT_F_CHG_PEAK_DT_03;
    pThm->fFChgPeakDt06            = THM_DEFAULT_F_CHG_PEAK_DT_06;
    pThm->fFChgPeakDt08            = THM_DEFAULT_F_CHG_PEAK_DT_08;
    pThm->fFChgPeakDt10            = THM_DEFAULT_F_CHG_PEAK_DT_10;
    pThm->fFDchgTotalDt03          = THM_DEFAULT_F_DCHG_TOTAL_DT_03;
    pThm->fFDchgTotalDt06          = THM_DEFAULT_F_DCHG_TOTAL_DT_06;
    pThm->fFDchgTotalDt08          = THM_DEFAULT_F_DCHG_TOTAL_DT_08;
    pThm->fFDchgTotalDt10          = THM_DEFAULT_F_DCHG_TOTAL_DT_10;
    pThm->fFDchgPeakDt03           = THM_DEFAULT_F_DCHG_PEAK_DT_03;
    pThm->fFDchgPeakDt06           = THM_DEFAULT_F_DCHG_PEAK_DT_06;
    pThm->fFDchgPeakDt08           = THM_DEFAULT_F_DCHG_PEAK_DT_08;
    pThm->fFDchgPeakDt10           = THM_DEFAULT_F_DCHG_PEAK_DT_10;

    /* B. SOC/SOH correction (sec 2.5) */
    pThm->fSocChgEnter             = THM_DEFAULT_SOC_CHG_ENTER;
    pThm->fSocChgExit              = THM_DEFAULT_SOC_CHG_EXIT;
    pThm->fSocChgSlope             = THM_DEFAULT_SOC_CHG_SLOPE;
    pThm->fSocChgLimit             = THM_DEFAULT_SOC_CHG_LIMIT;
    pThm->fSocDchgEnter            = THM_DEFAULT_SOC_DCHG_ENTER;
    pThm->fSocDchgExit             = THM_DEFAULT_SOC_DCHG_EXIT;
    pThm->fSocDchgSlope            = THM_DEFAULT_SOC_DCHG_SLOPE;
    pThm->fSocDchgLimit            = THM_DEFAULT_SOC_DCHG_LIMIT;
    pThm->fSohBase                 = THM_DEFAULT_SOH_BASE;
    pThm->fSohSlope                = THM_DEFAULT_SOH_SLOPE;
    pThm->fSohLimit                = THM_DEFAULT_SOH_LIMIT;

    /* C. target water temp bounds (sec 2.6) */
    pThm->fTTargetMin              = THM_DEFAULT_T_TARGET_MIN;
    pThm->fTTargetMax              = THM_DEFAULT_T_TARGET_MAX;

    /* D. cooling intensity boundary + cool-exit load (sec 3.2/3.3) */
    pThm->fForceCoolTTarget        = THM_DEFAULT_FORCE_COOL_T_TARGET;
    pThm->fForceCoolRTotal         = THM_DEFAULT_FORCE_COOL_R_TOTAL;
    pThm->fForceCoolRPeak          = THM_DEFAULT_FORCE_COOL_R_PEAK;
    pThm->fForceCoolTMax           = THM_DEFAULT_FORCE_COOL_T_MAX;
    pThm->fCoolExitRTotal          = THM_DEFAULT_COOL_EXIT_R_TOTAL;
    pThm->fCoolExitRPeak           = THM_DEFAULT_COOL_EXIT_R_PEAK;

    /* E. cold/heat interlock + low-temp charge (sec 4.2/4.3) */
    pThm->fHeatBlockTMax           = THM_DEFAULT_HEAT_BLOCK_T_MAX;
    pThm->fLowTChgLv1              = THM_DEFAULT_LOWT_CHG_LV1;
    pThm->fLowTChgLv2              = THM_DEFAULT_LOWT_CHG_LV2;
    pThm->fLowTChgLv3              = THM_DEFAULT_LOWT_CHG_LV3;


    pThm->tStarted         = getMonotonicSec();

    pThm->bInited = TRUE;

    TRACED("[thermal] Initialize OK (sampler=%d, dev=%d, model=%d)",
           pSampler->info.nSamplerID, THM_DEV_ID, THM_DEV_MODEL);
    return TRUE;
}

/*=================================================================================================
** @Name      : ApplyNoiseSafetyOverride
** @Function  : Spec 6.2.2 safety override for EU3 noise-reduction mode.
**              Noise reduction (param 2091, ENUM 0-6, 6=no reduction) limits compressor/fan
**              capability and may delay cooling. On a safety threshold (EMERGENCY_COOL over-temp)
**              the A-core must exit noise reduction and resume normal cooling, then restore the
**              user's requested level once the safety condition clears.
**              bEngage=TRUE  -> cache current level, force 2091=6 (normal cooling).
**              bEngage=FALSE -> restore cached level.
**===============================================================================================*/
static void ApplyNoiseSafetyOverride(THERMAL_CONTROL *pThm, BOOL bEngage)
{
    SAMPLER        *pSampler;
    DATA_PROCESSOR *pDPR;
    VAR_VALUE       varV;

    /* Noise reduction is an EU3-only feature (TF water chiller has no such mode) */
    if(pThm->nAirType != 1)
        return;

    pSampler = (SAMPLER *)pThm->hSampler;
    pDPR     = (DATA_PROCESSOR *)pSampler->hDPR;

    if(bEngage)
    {
        int nCurLevel = THM_NOISE_LEVEL_NORMAL;

        if(pThm->bNoiseOverrideActive)
            return;  /* already overriding */

        /* Cache the user's requested level before overwriting it */
        memset(&varV, 0, sizeof(varV));
        if(pDPR->Get(pThm->hSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                     THM_DST_EU3_NOISE, &varV, 4) == ERR_DPR_OK)
            nCurLevel = (int)varV.emValue;

        pThm->nNoiseLevelBeforeOverride = nCurLevel;
        pushParamEnum(pThm->hSampler, THM_DST_DEV_ID, THM_DST_EU3_NOISE, THM_NOISE_LEVEL_NORMAL);
        pThm->bNoiseOverrideActive = TRUE;
        TRACED("[thermal] noise safety override: forced level %d (was %d)",
               THM_NOISE_LEVEL_NORMAL, nCurLevel);
    }
    else
    {
        if(!pThm->bNoiseOverrideActive)
            return;

        pushParamEnum(pThm->hSampler, THM_DST_DEV_ID, THM_DST_EU3_NOISE,
                      pThm->nNoiseLevelBeforeOverride);
        pThm->bNoiseOverrideActive = FALSE;
        TRACED("[thermal] noise safety override released: restored level %d",
               pThm->nNoiseLevelBeforeOverride);
    }
}

/*=================================================================================================
** @Name      : Sample
** @Function  : Framework periodic entry point (1000ms cycle).
**              0. EMERGENCY_COOL check (overrides Manual, 15C target)
**              1. Load thresholds/coefficients on first cycle
**              2. Read all battery DPR inputs
**              3. Compute T_decision = T_cellMax + I_offset + PCS_offset
**              4. Run state machine
**              5. Push AC commands if state changed
**===============================================================================================*/
DLLExport int Sample(HANDLE hSamplerThis)
{
    SAMPLER         *pSampler = (SAMPLER *)hSamplerThis;
    DATA_PROCESSOR  *pDPR;
    THERMAL_CONTROL *pThm;
    int              nValidGroups;
    float            fCurrRatio;
    BOOL             bStateChanged;
    BOOL             bForcePush = FALSE;
    VAR_VALUE        varV;

    if(pSampler == NULL || pSampler->pDataHeap == NULL)
        return 0;
    pThm = (THERMAL_CONTROL *)pSampler->pDataHeap;

    if(!pThm->bInited)
        return 0;

    pThm->nCycleCount++;

    /* --- Load thresholds once on first cycle --- */
    if(!pThm->bThresholdsLoaded)
        LoadThresholds(pThm);

    /* Reload coefficients every cycle so Web edits take effect without restart */
    LoadCoefficients(pThm);

    /* --- Read all battery inputs (before mode check, needed for EMERGENCY_COOL) --- */
    nValidGroups = ReadBattInputs(pThm);

    /* --- Compute per-cluster load ratios (spec 2.3) --- */
    ComputeLoadRatios(pThm);

    /* --- Compute per-cluster current feedforward dT_current (spec 2.4) --- */
    ComputeFeedforward(pThm);

    /* --- Compute per-cluster SOC/SOH cooling correction (spec 2.5) --- */
    ComputeSocSohCorrection(pThm);

    /* --- Compute per-cluster + system target water temperature (spec 2.6) --- */
    ComputeTargetTemperatures(pThm);

    /* --- Compute same/cross-cluster temp diffs for self-circulation (spec 2.7 / 5.2) --- */
    ComputeSelfCircDiffs(pThm);

    /* --- Compute per-cluster cooling/heating demand + interlock inputs (spec 4.1/4.2) --- */
    ComputeColdHeatDemand(pThm);

    /* === EMERGENCY_COOL: highest-priority over-temp protection ===
     * Overrides Manual mode. Pushes AC command every cycle for robustness. */
    if(pThm->eState == THM_STATE_EMERGENCY_COOL
       || (nValidGroups > 0 && pThm->fTCellMax >= pThm->fEmergTrigger))
    {
        if(pThm->eState != THM_STATE_EMERGENCY_COOL)
        {
            TRACED("[thermal] EMERGENCY_COOL triggered: T_max=%.1f >= %.0fC",
                   pThm->fTCellMax, pThm->fEmergTrigger);
            pThm->eState = THM_STATE_EMERGENCY_COOL;
            pThm->tStateEntered = getMonotonicSec();
            pThm->tEmergencyBelowExit = 0;
            ApplyNoiseSafetyOverride(pThm, TRUE);
        }
        PushACCommand(pThm);  /* push every cycle */
        return 0;
    }

    /* Exit EMERGENCY_COOL when T stays below exit threshold for required duration */
    if(pThm->eState == THM_STATE_EMERGENCY_COOL)
    {
        if(pThm->fTCellMax < pThm->fEmergExit)
        {
            long long tNow = getMonotonicSec();
            if(pThm->tEmergencyBelowExit == 0)
                pThm->tEmergencyBelowExit = tNow;
            else if(tNow - pThm->tEmergencyBelowExit >= (long long)pThm->nEmergExitSec)
            {
                TRACED("[thermal] EMERGENCY_COOL exit: T_max=%.1f < %.0fC for %llds",
                       pThm->fTCellMax, pThm->fEmergExit,
                       tNow - pThm->tEmergencyBelowExit);
                pThm->eState = THM_STATE_STANDBY;
                pThm->tStateEntered = tNow;
                pThm->tEmergencyBelowExit = 0;
                PushACCommand(pThm);
                ApplyNoiseSafetyOverride(pThm, FALSE);
            }
        }
        else
        {
            pThm->tEmergencyBelowExit = 0;  /* temp rose, reset timer */
        }
        return 0;  /* stay in emergency, skip normal cycle */
    }

    /* --- Check AC communication status (read AC state param from devId=951) ---
     * Spec 7.1/7.2: on comm loss aircon_sampler pushes SAMP_INVALID_VALUE.
     * Do NOT pretend the device is in STANDBY — it may still be executing its
     * last command. Mark actual state unknown, record last-command age, report
     * capability loss, and skip this cycle. On recovery, rebuild timing and
     * force a re-push of the desired state (spec 7.2: 恢复后重建计时再控制). */
    pDPR = (DATA_PROCESSOR *)pSampler->hDPR;
    {
        int nProbeParam = (pThm->nAirType == 1) ? THM_SRC_AC_STATE_EU3 : THM_SRC_AC_STATE_TF;
        memset(&varV, 0, sizeof(varV));
        if(pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, THM_DST_DEV_ID,
                      nProbeParam, &varV, 4) == ERR_DPR_OK)
        {
            if(varV.lValue == SAMP_INVALID_VALUE || varV.emValue == (_ENUM)SAMP_INVALID_VALUE)
            {
                long long tNow = getMonotonicSec();
                if(!pThm->bACCommLost)
                {
                    pThm->bACCommLost = TRUE;
                    pThm->tACLostAt   = tNow;
                    TRACED("[thermal] AC comm lost (param %d invalid). Actual state unknown; "
                           "last cmd age=%llds. Reporting capability loss.",
                           nProbeParam, (tNow - pThm->tLastCommandAt));
                }
                return 0;  /* skip cycle; keep eState (do NOT pretend standby) */
            }
            else if(pThm->bACCommLost)
            {
                TRACED("[thermal] AC comm recovered after %llds. Rebuilding timing, re-push state=%d.",
                       getMonotonicSec() - pThm->tACLostAt, pThm->eState);
                pThm->bACCommLost   = FALSE;
                pThm->tACLostAt     = 0;
                pThm->tStateEntered = getMonotonicSec();  /* rebuild min_run/min_stop timer */
                bForcePush          = TRUE;                /* re-issue desired command */
            }
        }
    }

    /* --- Check control mode: read param 2203 (Aircon Ctrl Mode) from devId=951.
     * 2203 is the SINGLE source of truth: 0=Auto (run thermal strategy),
     * 1=Manual (skip - user controls AC directly via Web UI).
     * NOTE: EMERGENCY_COOL overrides Manual (checked above). */
    memset(&varV, 0, sizeof(varV));
    if(pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, THM_DST_DEV_ID,
                  THM_PARAM_AIRCON_CTRL_MODE, &varV, 4) == ERR_DPR_OK)
    {
        if(varV.emValue == THM_AIRCON_MODE_MANUAL)  /* 2203 == 1 */
        {
            /* Manual mode: thermal strategy suspended; user controls AC via Web.
             * Periodic hint so operators see why AC ignores the auto strategy.
             * (spec 3.6: no 900s timeout, just a status indicator) */
            if((pThm->nCycleCount % 10) == 0)
                TRACED("[thermal] AC in MANUAL mode (2203=1): auto strategy suspended");
            return 0;
        }
    }

    /* --- Startup protection: after Initialize, hold for nStartupProtectSec before
     *     entering normal control (read actual device state first). EMERGENCY_COOL
     *     and AC comm-loss checks above still run for safety. --- */
    if(pThm->nStartupProtectSec > 0 &&
       (getMonotonicSec() - pThm->tStarted) < (long long)pThm->nStartupProtectSec)
    {
        return 0;
    }

    if(nValidGroups == 0)
    {
        TRACED("[thermal] Sample: no valid battery groups, skipping");
        return 0;
    }

    /* --- Compute |I|/I_rated ratio (discharge rated current as denominator) --- */
    if(pThm->fDischargeRatedCurr > 0.01f)
    {
        float fAbsCurr = (pThm->fBattBusCurr >= 0.0f) ?
                          pThm->fBattBusCurr : -pThm->fBattBusCurr;
        fCurrRatio = fAbsCurr / pThm->fDischargeRatedCurr;
    }
    else
    {
        fCurrRatio = 0.0f;
    }

    /* --- Compute I_offset and PCS_offset --- */
    pThm->fIOffsetApplied  = ComputeIOffset(fCurrRatio, pThm);
    pThm->fPCSOffsetApplied = ComputePCSOffset(pThm->nPCSState, pThm);

    /* --- Compute T_decision --- */
    pThm->fTDecision = pThm->fTCellMax +
                       pThm->fIOffsetApplied +
                       pThm->fPCSOffsetApplied;

    /* --- Run state machine --- */
    bStateChanged = RunStateMachine(pThm);

    /* --- Same-state target resend (spec 2.10): cooling/deepcool target drifts with
     *     load/SOC/SOH even when state is unchanged; re-push so the AC follows the
     *     new setpoint. 0.5C deadband avoids flutter (TF pushes integer degC). --- */
    if(!bStateChanged && !bForcePush
       && (pThm->eState == THM_STATE_COOLING || pThm->eState == THM_STATE_DEEPCOOL))
    {
        float fTarget = (pThm->eState == THM_STATE_COOLING)
                        ? pThm->fTTargetSystem
                        : (pThm->fTTargetSystem - pThm->fColdDeepDelta);
        float fDiff   = fTarget - pThm->fLastPushedCoolTarget;
        if(fDiff < 0.0f)
            fDiff = -fDiff;
        if(fDiff >= 0.5f)
            bForcePush = TRUE;
    }

    /* --- Push AC commands on state change --- */
    if(bStateChanged || bForcePush)
        PushACCommand(pThm);

    /* --- Push noise safety override status flag (spec 6.2.2 read-only diagnostic) --- */
    pushParamStatus(pThm->hSampler, THM_DST_DEV_ID, THM_STAT_NOISE_OVERRIDE,
                    pThm->bNoiseOverrideActive ? 1 : 0);

    /* --- Periodic debug trace (~every 10s) --- */
    if((pThm->nCycleCount % 10) == 0)
    {
        TRACED("[thermal] S=%d Tmax=%.1f I=%.1f SOC=%.1f%% PCS=%d "
               "T_dec=%.1f T_on/off=%.1f/%.1f I_off=%.1f PCS_off=%.1f groups=%d "
               "r_total=%.2f r_peak=%.2f r0=%.2f r1=%.2f valid[%d/%d] degrade=%d "
               "dt_cur[0]=%.1f dt_cur[1]=%.1f dt_soc[0]=%.1f dt_soc[1]=%.1f dt_soh[0]=%.1f dt_soh[1]=%.1f "
               "T_tgt[0]=%.1f T_tgt[1]=%.1f T_tgt_sys=%.1f",
               pThm->eState, pThm->fTCellMax, pThm->fBattBusCurr, pThm->fAvgSOC,
               pThm->nPCSState, pThm->fTDecision,
               pThm->fTColdOn, pThm->fTColdOff,
               pThm->fIOffsetApplied, pThm->fPCSOffsetApplied,
               nValidGroups,
               pThm->fRTotal, pThm->fRPeak, pThm->fClusterR[0], pThm->fClusterR[1],
               pThm->bClusterValid[0] ? 1 : 0, pThm->bClusterValid[1] ? 1 : 0, pThm->nDataDegrade,
               pThm->fClusterDtCurr[0], pThm->fClusterDtCurr[1],
               pThm->fClusterDtSoc[0], pThm->fClusterDtSoc[1],
               pThm->fClusterDtSoh[0], pThm->fClusterDtSoh[1],
               pThm->fClusterTTarget[0], pThm->fClusterTTarget[1], pThm->fTTargetSystem);
    }

    return 0;
}

/*=================================================================================================
** @Name      : Control
** @Function  : Framework sync entry point (IsNeedSync_Sampler=1). No-op for this compute-only
**              sampler: formula coefficients (2408-2417) live on devId=951 (Air Conditioner#1)
**              and are read fresh every cycle by LoadCoefficients(). DPR routes those Web writes
**              to aircon_sampler (sampler 11), never to this module.
**===============================================================================================*/
DLLExport int Control(HANDLE hSamplerThis,
                      PARAM_TYPE emParaType,
                      int nDevId,
                      int nParamId,
                      VAR_VALUE *pVarVal,
                      int nValSize,
                      int nTimeOut)
{
    (void)hSamplerThis; (void)emParaType; (void)nDevId; (void)nParamId;
    (void)pVarVal; (void)nValSize; (void)nTimeOut;
    return 0;
}

/*=================================================================================================
** @Name      : JigTest
** @Function  : Production test interface (empty — no hardware to test).
**===============================================================================================*/
DLLExport int JigTest(HANDLE hSamplerThis, void *pJigData)
{
    (void)hSamplerThis; (void)pJigData;
    return 0;
}
