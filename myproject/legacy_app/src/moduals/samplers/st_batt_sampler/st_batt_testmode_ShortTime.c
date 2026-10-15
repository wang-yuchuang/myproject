#include "st_batt_sampler.h"

//#define TRACE	 TRACE("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), TRACE
//#define TRACEX	 TRACE("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), TRACE

#define INIT_TEST_STATE_MACHINE(p, _emState, _pfnOnAct, _failFrom) \
 					((p)->emState = (_emState), \
					(p)->pfnAction = (_pfnOnAct), \
					(p)->emFailFrom = (_failFrom), \
					(p)++)

/*
static BOOL Send_StopCmd(STBATT_SAMPLER_ROUGH *pSTBatt, int nBattIdx, STATE_MANUL_CTRL emCtrlState)
{
    VAR_VALUE varV;
	double tmNow = GetCurrentTime();
    A_BLOB_SINGLE_STBATT_RUNINFO *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];

    pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx] = tmNow;

    float fSet_Volt = 0.0, fSet_Curr = 0.0;
    float fLimitCurrent = 0.0;

    switch (emCtrlState)
    {
        case STAT_MANUAL_STOP:
            {
                fSet_Volt = 0.0; 
                fSet_Curr = 0.0;
                pSTBatt->tm_LastStopChg_4STBatt[nBattIdx] = tmNow;
            }
            break;
        default:
            break;
    }

	//Manual Ctlr Mode Protect Logic
	pSTBatt->blobManualCtrlPM.emState   = emCtrlState; 		//0-means PM Stop Work 
	pSTBatt->blobManualCtrlPM.nGrp      = nBattIdx + 1; 	//0-means All PM Grp
	pSTBatt->blobManualCtrlPM.fGrpVolt  = fSet_Volt;
	pSTBatt->blobManualCtrlPM.fGrpCurr  = fSet_Curr;
	varV.blobValue = (BYTE *)&pSTBatt->blobManualCtrlPM;

    //Push_STBatt_AllData

	Push_BlobValue(SETTYPE_INTERN_CTRL, pSTBatt->hSampler, 
		200, 214, (void*)varV.blobValue, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), BLOB_TYPE_MANUALCTRL_RECT);
    
    // TRACE("after push blobvalue.\n");
    
    pSTBatt->fLastSendReqVolt_2CCU[nBattIdx] = fSet_Volt;
    pSTBatt->fLastSendReqCurr_2CCU[nBattIdx] = fSet_Curr;

	return TRUE;
}

static BOOL Send_CtrlCmd(STBATT_SAMPLER_ROUGH *pSTBatt, int nBattIdx, STATE_MANUL_CTRL emCtrlState, float inLimitCurrent)
{
    VAR_VALUE varV;
	double tmNow = GetCurrentTime();
    A_BLOB_SINGLE_STBATT_RUNINFO *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];

    pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx] = tmNow;

    float fSet_Volt = 0.0, fSet_Curr = 0.0;
    float fLimitCurrent = 0.0;

    switch (emCtrlState)
    {
        case STAT_MANUAL_STOP:
            {
                fSet_Volt = 0.0; 
                fSet_Curr = 0.0;
                pSTBatt->tm_LastStopChg_4STBatt[nBattIdx] = tmNow;
            }
            break;
        case STAT_MANUAL_START_CHG:
        case STAT_MANUAL_ADJ_CHG:
            {
                fSet_Volt = MIN(1000, pSiglBatt->fAllow_LmtChargeVolt);
                // TRACE("fPackTotalVolt: %f\n",pSTBatt->blobSingleBattInfo[nBattIdx].fPackTotalVolt);
                if(pSTBatt->blobSingleBattInfo[nBattIdx].fPackTotalVolt > 50.0)
                {
                    // fLimitCurrent = (pSTBatt->calcNodeData[nBattIdx].fACDC_Distri_4_BattChrg_Pwr*1000)/pSiglBatt->fPackTotalVolt;
                    fLimitCurrent = inLimitCurrent;
                }
                fSet_Curr = MIN(fLimitCurrent, pSiglBatt->fAllow_MaxChargeCurr);
                // TRACE("fLimitCurrent: %f\n", fLimitCurrent);
                // TRACE("fset Volt: %f\n", fSet_Volt);
                // TRACE("fset Curr: %f\n", fSet_Curr);

                TRACE("Test Mode:Rectify Mode Ctrl nBattIdx=[%d]:fLimitCurrent=%.1f fAllow_MaxChargeCurr=%.1f\n", \
                    nBattIdx, fLimitCurrent,pSiglBatt->fAllow_MaxChargeCurr);
                TRACE("Test Mode:Rectify Mode Ctrl nBattIdx=[%d]:fLimitCurrent=%.1f fAllow_MaxChargeCurr=%.1f\n", \
                    nBattIdx, fLimitCurrent,pSiglBatt->fAllow_MaxChargeCurr);
            }
            break;
        case STAT_MANUAL_START_DISCHG:
        case STAT_MANUAL_ADJ_DISCHG:
            {
                fSet_Volt = MIN(1000, pSiglBatt->fAllow_LmtChargeVolt);
                // TRACE("fPackTotalVolt: %f\n",pSTBatt->blobSingleBattInfo[nBattIdx].fPackTotalVolt);
                if(pSiglBatt->fPackTotalVolt > 50.0)
                {
                    fLimitCurrent = inLimitCurrent;
                    // fLimitCurrent = (pSTBatt->calcNodeData[nBattIdx].fBatt_Distri_4_ACDC_Pwr*1000)/pSiglBatt->fPackTotalVolt;
                }

                fSet_Curr = MIN(fLimitCurrent, pSiglBatt->fAllow_MaxDisChargeCurr);
                // TRACE("fLimitCurrent: %f\n", fLimitCurrent);
                // TRACE("fset Volt: %f\n", fSet_Volt);
                // TRACE("fset Curr: %f\n", fSet_Curr);

                TRACE("Test Mode:Invert Mode Ctrl nBattIdx=[%d]:emCmdType=[%d] fLimitCurrent=%.1f fAllow_MaxDisChargeCurr=%.1f\n", \
                    nBattIdx, fLimitCurrent, pSiglBatt->fAllow_MaxDisChargeCurr);

                TRACE("Test Mode:Invert Mode Ctrl nBattIdx=[%d]:emCmdType=[%d] fLimitCurrent=%.1f fAllow_MaxDisChargeCurr=%.1f\n", \
                    nBattIdx, fLimitCurrent, pSiglBatt->fAllow_MaxDisChargeCurr);
            }
            break;

        default:
            break;
    }

	//Manual Ctlr Mode Protect Logic
	pSTBatt->blobManualCtrlPM.emState   = emCtrlState; 		//0-means PM Stop Work 
	pSTBatt->blobManualCtrlPM.nGrp      = nBattIdx + 1; 	//0-means All PM Grp
	pSTBatt->blobManualCtrlPM.fGrpVolt  = fSet_Volt;
	pSTBatt->blobManualCtrlPM.fGrpCurr  = fSet_Curr;
	varV.blobValue = (BYTE *)&pSTBatt->blobManualCtrlPM;

    //Push_STBatt_AllData

	Push_BlobValue(SETTYPE_INTERN_CTRL, pSTBatt->hSampler, 
		200, 214, (void*)varV.blobValue, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), BLOB_TYPE_MANUALCTRL_RECT);
    
    TRACE("after push blobvalue.\n");
    
    pSTBatt->fLastSendReqVolt_2CCU[nBattIdx] = fSet_Volt;
    pSTBatt->fLastSendReqCurr_2CCU[nBattIdx] = fSet_Curr;

	return TRUE;
}
*/

STATE_TEST_RES SelfCheckTest_ShortTime(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{   
    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];

    float fOutRelay_Volt = pBranchGrp->fACDCOutRelayVolt;
    float fPM_RealtimeVolt = pBranchGrp->fACDC_RealTime_Volt;

    if(fOutRelay_Volt < MIN_PM_OUTPUT_VOLT)
    {   
        TRACE("fOutRelay_Volt too small: %.1f\n");
        pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_RELAY_VOLT_ABNORMAL;
		
        return TEST_FAIL; 
    }

    if(fPM_RealtimeVolt > MIN_PM_OUTPUT_VOLT)
    {   
        TRACE("fPM_RealtimeVolt too large: %.1f\n");
        //Need Send Stop PM Cmd
        Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);

        return TEST_FAIL;
    }

    STATE_TEST_RES res_all = TEST_PASS;
    unsigned int res_byte = 0;
    // int res_1 = 0; // 0x1532ffxx
    // int res_2 = 0; // 0x1533ffxx
    // int res_3 = 0; // 0x1534ffxx

    // Voltage Cell info
    float VolCell_TotalVolt = pSTBatt->blobIfyBMS[nBattIdx].fPackTotalVolt;
    float VolCell_TotalAddVolt = pSTBatt->blobIfyBMS[nBattIdx].fCellAddTotalVolt;
    float VolCell_Num = pSTBatt->blobIfyBMS[nBattIdx].usCellVoltTotalNum;
    float VolCell_Mean = pSTBatt->blobIfyBMS[nBattIdx].usCellAverageVolt;
    float VolCell_Max = pSTBatt->blobIfyBMS[nBattIdx].usMaxCellVlot_Volt;
    float VolCell_Min = pSTBatt->blobIfyBMS[nBattIdx].usMinCellVlot_Volt;
    // Temp Cell info
    int TempCell_Num  = pSTBatt->blobIfyBMS[nBattIdx].usCellTempTotalNum;
    int TempCell_Mean = pSTBatt->blobIfyBMS[nBattIdx].sCellAverageTemp;
    int TempCell_Max  = pSTBatt->blobIfyBMS[nBattIdx].sMaxCellTemp_Temp;
    int TempCell_Min  = pSTBatt->blobIfyBMS[nBattIdx].sMinCellTemp_Temp;


    // save self check result
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_V_Num = (int)VolCell_Num;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_T_Num = (int)TempCell_Num;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Tot_V = (int)VolCell_TotalVolt;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_TotAdd_Gap = (int)abs(VolCell_TotalVolt-VolCell_TotalAddVolt);

    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_MaxVoltage = (int)VolCell_Max;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_MinVoltage = (int)VolCell_Min;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_AvgVolt = (int)VolCell_Mean;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_RangeVolt = (int)(VolCell_Max-VolCell_Min);

    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_MaxTemperature = -TempCell_Max;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_MinTemperature = -TempCell_Min;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_AvgTemp = -TempCell_Mean;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_RangeTemperature = -(TempCell_Max-TempCell_Min);

    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Add_V = (int)VolCell_TotalAddVolt;

    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Self_Result = (int)res_byte;
    pSTBatt->bmsAutoTestResult[nBattIdx].testState = STATE_SELFCHECK_END;
    
    return res_all;
}

STATE_TEST_RES ChargeEndTest_ShortTime(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx) // 0: Originial test
{   
    STATE_TEST_RES res_all = TEST_PASS;
    unsigned int res_byte = 0;
    // Voltage Cell info
    int VolCell_Max = pSTBatt->blobIfyBMS[nBattIdx].usMaxCellVlot_Volt;
    int VolCell_Min = pSTBatt->blobIfyBMS[nBattIdx].usMinCellVlot_Volt;
    int VollCell_TerminalRange = pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inTerminalCharge;
    int VollCell_PlaRange = pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inPlaCharge;
    // Temp Cell info
    int TempCell_Max    = pSTBatt->bmsTestProcessRecord[nBattIdx].MaxTemp_inCharge;
    int TempCell_Min    = pSTBatt->bmsTestProcessRecord[nBattIdx].MinTemp_inCharge;
    int TempCell_Range  = pSTBatt->bmsTestProcessRecord[nBattIdx].RangeTemp_inCharge;

    // Charge amount info
    float Charge_Energy = pSTBatt->bmsTestProcessRecord[nBattIdx].EndEnergy_inCharge - pSTBatt->bmsTestProcessRecord[nBattIdx].StartEnergy_inCharge;
    float Charge_Capacity = pSTBatt->blobIfyBMS[nBattIdx].fSingleChgCapacity;
    // System Charge kWhinfo
    float Meter_Charge_KWh = pSTBatt->bmsTestProcessRecord[nBattIdx].EndKWh_inCharge - pSTBatt->bmsTestProcessRecord[nBattIdx].StartKWh_inCharge;

    // Total add voltage
    float VolCell_TotalAddMax = pSTBatt->blobIfyBMS[nBattIdx].fPackTotalVolt; // 充电截止总电压 >= 852V 则认为充电完成

    
    float TH_nVolCell_Num           =   pSTBatt->bmsTestModeParm.initRange.TH_nVolCell_Num;
    float TH_nChargeEndTemp_Max     =   pSTBatt->bmsTestModeParm.initRange.TH_nChargeEndTemp_Max;
    float TH_nChargeEndTemp_Min     =   pSTBatt->bmsTestModeParm.initRange.TH_nChargeEndTemp_Min;
    float TH_nChargeEndTemp_Gap     =   pSTBatt->bmsTestModeParm.initRange.TH_nChargeEndTemp_Gap;
    float TH_nChargeEndVol_Max      =   pSTBatt->bmsTestModeParm.initRange.TH_nChargeEndVol_Max;
    float TH_nChargeEnd_TotalVolMax =   pSTBatt->bmsTestModeParm.initRange.TH_nChargeEnd_TotalVolMax;
    float TH_nChargeEndVol_Gap      =   pSTBatt->bmsTestModeParm.initRange.TH_nChargeEndVol_Gap;
    float TH_nChargeEnd_Energy      =   0.0;
    if(pSTBatt->lcCfg.emSystemType == SYSTYPE_ESS_AIR)
    {
        TH_nChargeEnd_Energy = TH_nVolCell_Num * 3.2 * AIR_CELL_CAPACITY * 0.001;
    }
    else
    {
        TH_nChargeEnd_Energy = TH_nVolCell_Num * 3.2 * LIQUIDED_CELL_CAPACITY * 0.001;
    }
    float TH_nChargeEndPlateauVol_Gap      =   pSTBatt->bmsTestModeParm.initRange.TH_nChargeEndPlateauVol_Gap;

    if(TempCell_Max > (TH_nChargeEndTemp_Max * 10))
    {
        TRACE("Temp cell max: %d out of range!\n", TempCell_Max);
        res_byte |= (1 << 0);
        res_all = TEST_FAIL;
    }
    if(TempCell_Range > (TH_nChargeEndTemp_Gap * 10))
    {
        TRACE("Temp cell gap: %d out of range!\n", TempCell_Max-TempCell_Min);
        res_byte |= (1 << 1);
        res_all = TEST_FAIL;
    }
    // if((VolCell_Max < TH_nChargeEndVol_Max) && (VolCell_TotalAddMax < TH_nChargeEnd_TotalVolMax))//单体电压 >= 3600mv / 总压 >= 852v
    // {
    //     TRACE("Vol cell max: %d out of range!\n", VolCell_Max);
    //     res_byte |= (1 << 2);
    //     res_all = TEST_FAIL;
    // }
    if(VollCell_TerminalRange > TH_nChargeEndVol_Gap)
    {
        TRACE("Vol cell gap: %d out of range!\n", VolCell_Max-VolCell_Min);
        res_byte |= (1 << 3);
        res_all = TEST_FAIL;
    }
    if(TempCell_Min < (TH_nChargeEndTemp_Min * 10))
    {
        TRACE("Temp cell min: %d out of range!\n", TempCell_Min);
        res_byte |= (1 << 4);
        res_all = TEST_FAIL;
    }
    // if(Charge_Energy < TH_nChargeEnd_Energy)
    // {
    //     TRACE("Charge amount: %.1f out of range!\n", Charge_Energy);
    //     res_byte |= (1 << 5);
    //     res_all = TEST_FAIL;    
    // }
    if(VollCell_PlaRange > TH_nChargeEndPlateauVol_Gap)
    {
        TRACE("Charge plateau vol: %.1f out of range!\n", VollCell_PlaRange);
        res_byte |= (1 << 7);
        res_all = TEST_FAIL;
    }

    // save charge end result
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_LMaxTemperature = -(int)TempCell_Max;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_LRangeTemperature = -(int)TempCell_Range;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_LMaxVol = (int)VolCell_Max;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_LTermRangeVol = (int)VollCell_TerminalRange;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_LPlateauRangeVol = (int)VollCell_PlaRange;//xxxsy

    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_LMinTemperature = -(int)TempCell_Min;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Charge_Energy = (int)(Charge_Energy * 10);  // Master Computer has a decimal
    // pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Charge_Energy = (int)(Meter_Charge_KWh * 10);  // Master Computer has a decimal, now need to get data from meter
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Charge_Capacity = (int)(Charge_Capacity * 10);  // Master Computer has a decimal;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Meter_Charge_KWh = (int)(Meter_Charge_KWh * 10);  // Master Computer has a decimal

    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Charge_Result = (int)res_byte;

    // TRACE("saved res!!!\n");
    // TRACE("BMS_LMaxTemperature: %d\n", pSTBatt->bmsAutoTestResult[nBattIdx].BMS_LMaxTemperature);
    // TRACE("BMS_LRangeTemperature: %d\n", pSTBatt->bmsAutoTestResult[nBattIdx].BMS_LRangeTemperature);
    // TRACE("BMS_LMaxVol: %d\n", pSTBatt->bmsAutoTestResult[nBattIdx].BMS_LMaxVol);
    // TRACE("BMS_LEndRangeVoltage: %d\n", pSTBatt->bmsAutoTestResult[nBattIdx].BMS_LEndRangeVol);
    // TRACE("BMS_LMinTemperature: %d\n", pSTBatt->bmsAutoTestResult[nBattIdx].BMS_LMinTemperature);
    // TRACE("BMS_Charge_Energy: %d\n", pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Charge_Energy);
    // TRACE("BMS_Charge_Result: %d\n", pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Charge_Result);


    return res_all;
}

STATE_TEST_RES DischargeEndTest_ShortTime(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx) // 0: Originial test
{   
    STATE_TEST_RES res_all = TEST_PASS;
    unsigned int res_byte = 0;
    // Voltage Cell info
    float VolCell_Max = pSTBatt->blobIfyBMS[nBattIdx].usMaxCellVlot_Volt;
    float VolCell_Min = pSTBatt->blobIfyBMS[nBattIdx].usMinCellVlot_Volt;
    int VollCell_TerminalRange = pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inTerminalDischarge;
    int VollCell_PlaRange = pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inPlaDischarge;

    // Temp Cell info
    int TempCell_Max    = pSTBatt->bmsTestProcessRecord[nBattIdx].MaxTemp_inDischarge;
    int TempCell_Min    = pSTBatt->bmsTestProcessRecord[nBattIdx].MinTemp_inDischarge;
    int TempCell_Range  = pSTBatt->bmsTestProcessRecord[nBattIdx].RangeTemp_inDischarge;

    // Discharge amount info
    float DisCharge_Energy = pSTBatt->bmsTestProcessRecord[nBattIdx].EndEnergy_inDischarge - pSTBatt->bmsTestProcessRecord[nBattIdx].StartEnergy_inDischarge;
    float DisCharge_Capacity = pSTBatt->blobIfyBMS[nBattIdx].fSingleDischgCapacity;
    // System Discharge kWhinfo
    float Meter_Discharge_KWh = pSTBatt->bmsTestProcessRecord[nBattIdx].EndKWh_inDischarge - pSTBatt->bmsTestProcessRecord[nBattIdx].StartKWh_inDischarge;

    // Total add voltage
    float VolCell_TotalAddMin = pSTBatt->blobIfyBMS[nBattIdx].fPackTotalVolt; // 充电截止总电压 <= 696V 则认为充电完成

    
    // Vol/Temp init range
    float TH_nVolCell_Num               =   pSTBatt->bmsTestModeParm.initRange.TH_nVolCell_Num;
    float TH_nDischargeEndTemp_Max      =   pSTBatt->bmsTestModeParm.initRange.TH_nDischargeEndTemp_Max;
    float TH_nDischargeEndTemp_Min      =   pSTBatt->bmsTestModeParm.initRange.TH_nDischargeEndTemp_Min;
    float TH_nDischargeEndTemp_Gap      =   pSTBatt->bmsTestModeParm.initRange.TH_nDischargeEndTemp_Gap;
    float TH_nDischargeEndVol_Min       =   pSTBatt->bmsTestModeParm.initRange.TH_nDischargeEndVol_Min;
    float TH_nDischargeEnd_TotalVolMin  =   pSTBatt->bmsTestModeParm.initRange.TH_nDischargeEnd_TotalVolMin;
    float TH_nDischargeEndVol_Gap       =   pSTBatt->bmsTestModeParm.initRange.TH_nDischargeEndVol_Gap;
    float TH_nDischargeEnd_Energy       =   0.0;
    if(pSTBatt->lcCfg.emSystemType == SYSTYPE_ESS_AIR)
    {
        TH_nDischargeEnd_Energy = TH_nVolCell_Num * 3.2 * AIR_CELL_CAPACITY * 0.92 * 0.001;
    }
    else
    {
        TH_nDischargeEnd_Energy = TH_nVolCell_Num * 3.2 * LIQUIDED_CELL_CAPACITY * 0.92 * 0.001;
    }
    float TH_nDischargeEndPlateauVol_Gap      =   pSTBatt->bmsTestModeParm.initRange.TH_nDischargeEndPlateauVol_Gap;

    if(TempCell_Max > (TH_nDischargeEndTemp_Max * 10))
    {
        TRACE("Temp cell max: %.1f out of range!\n", TempCell_Max);
        res_byte |= (1 << 0);
        res_all = TEST_FAIL;
    }
    if(TempCell_Range > (TH_nDischargeEndTemp_Gap * 10))
    {
        TRACE("Temp cell gap: %.1f out of range!\n", TempCell_Max-TempCell_Min);
        res_byte |= (1 << 1);
        res_all = TEST_FAIL;
    }
    // if((VolCell_Min > TH_nDischargeEndVol_Min) && (VolCell_TotalAddMin > TH_nDischargeEnd_TotalVolMin))//单体电压 < 2900mv / 总压 < 696V
    // {
    //     TRACE("Vol cell min: %.1f out of range!\n", VolCell_Min);
    //     res_byte |= (1 << 2);
    //     res_all = TEST_FAIL;
    // }
    if(VollCell_TerminalRange > TH_nDischargeEndVol_Gap)
    {
        TRACE("Vol cell gap: %.1f out of range!\n", VolCell_Max-VolCell_Min);
        res_byte |= (1 << 3);
        res_all = TEST_FAIL;
    }
    
    if(TempCell_Min < (TH_nDischargeEndTemp_Min * 10))
    {
        TRACE("Temp cell min: %.1f out of range!\n", TempCell_Min);
        res_byte |= (1 << 4);
        res_all = TEST_FAIL;
    }
    // if(DisCharge_Energy < TH_nDischargeEnd_Energy)
    // {
    //     TRACE("Discharge amount: %.1f out of range!\n");
    //     res_byte |= (1 << 5);
    //     res_all = TEST_FAIL;
    // }
    if(VollCell_PlaRange > TH_nDischargeEndPlateauVol_Gap)
    {
        TRACE("Disharge plateau vol: %.1f out of range!\n", VollCell_PlaRange);
        res_byte |= (1 << 7);
        res_all = TEST_FAIL;
    }

    // save self check result
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_SMaxTemperature = -(int)TempCell_Max;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_SRangeTemperature = -(int)TempCell_Range;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_SMinVol = (int)VolCell_Min;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_STermRangeVol = (int)VollCell_TerminalRange;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_SPlateauRangeVol = (int)VollCell_PlaRange;//xxxsy

    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_SMinTemperature = -(int)TempCell_Min;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Discharge_Energy = (int)DisCharge_Energy * 10;  // Master Computer has a decimal
    // pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Discharge_Energy = (int)(Meter_Discharge_KWh * 10);  // Master Computer has a decimal, now need to get data from meter
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Discharge_Capacity = (int)(DisCharge_Capacity * 10);  // Master Computer has a decimal;
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Meter_Discharge_KWh = (int)(Meter_Discharge_KWh * 10);  // Master Computer has a decimal

    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_DisCharge_Result = (int)res_byte;

    return res_all;
}

STATE_TEST_RES TotalTest_ShortTime(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{   
    STATE_TEST_RES res_all = TEST_PASS;
    unsigned int res_byte = 0;

    // // Discharge amount info
    // float DisCharge_Energy = pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Discharge_Energy;
    // // Charge amount info
    // float Charge_Energy = pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Charge_Energy;
    // // Energy Efficiency
    // float Power_Eff = (DisCharge_Energy * 100.0) / Charge_Energy;

    // // System Charge kWh
    // float Meter_Charge_KWh = pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Meter_Charge_KWh;
    // // System Discharge kWh
    // float Meter_Discharge_KWh = pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Meter_Discharge_KWh;
    // // System Efficiency
    // float System_Eff = (Meter_Discharge_KWh * 100.00) / Meter_Charge_KWh;

    // // Test init range
    // float TH_nPower_Efficiency = pSTBatt->bmsTestModeParm.initRange.TH_nPower_Efficiency;
    // float TH_nSystem_Efficiency = pSTBatt->bmsTestModeParm.initRange.TH_nSystem_Efficiency;

    // if(Power_Eff<TH_nPower_Efficiency)
    // {
    //     TRACE("Power Efficiency: %.1f out of range!\n", Power_Eff);
    //     res_byte |= (1 << 0);
    //     res_all = TEST_FAIL;
    // }
    // if(System_Eff<TH_nSystem_Efficiency)
    // {
    //     TRACE("System Efficiency: %.1f out of range!\n", System_Eff);
    //     res_byte |= (1 << 1);
    //     res_all = TEST_FAIL;
    // }
    // pSTBatt->bmsAutoTestResult[nBattIdx].BMS_EnergyEfficiency = (int)(Power_Eff * 10);// Master Computer has a decimal
    // pSTBatt->bmsAutoTestResult[nBattIdx].BMS_TotalSysEfficiency = (int)(System_Eff * 10);// Master Computer has a decimal
    pSTBatt->bmsAutoTestResult[nBattIdx].BMS_Total_Result = (int)res_byte;

    return res_all;
}

static void Update_ChargeData(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{   
    //Debug
    // TRACE("IN PROCESS Charge! n: %d, tempmax: %d, tempmin: %d, temprange: %d, volrange: %d\n", nBattIdx,\
    pSTBatt->bmsTestProcessRecord[nBattIdx].MaxTemp_inCharge, pSTBatt->bmsTestProcessRecord[nBattIdx].MinTemp_inCharge, \
    pSTBatt->bmsTestProcessRecord[nBattIdx].RangeTemp_inCharge, pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inTerminalCharge);
    // Voltage Cell info
    int VolCell_Max = pSTBatt->blobIfyBMS[nBattIdx].usMaxCellVlot_Volt;
    int VolCell_Min = pSTBatt->blobIfyBMS[nBattIdx].usMinCellVlot_Volt;
    int VolCell_Avg = pSTBatt->blobIfyBMS[nBattIdx].usCellAverageVolt; // 平均电压用于判断是否进入充电末端
    // printf("VolCell_Avg:%d\n", VolCell_Avg);//xxxsy
    // Temp Cell info
    int TempCell_Max = pSTBatt->blobIfyBMS[nBattIdx].sMaxCellTemp_Temp;
    int TempCell_Min = pSTBatt->blobIfyBMS[nBattIdx].sMinCellTemp_Temp;
    // Charge Energy info
    float ChargeAddAmount = pSTBatt->blobIfyBMS[nBattIdx].fAccuChgEnergy;
    // System Charge kWh info
    float SystemChargeAddAmount = pSTBatt->fCabMeter_Power[0];//交流实时电量

    if(TempCell_Max>pSTBatt->bmsTestProcessRecord[nBattIdx].MaxTemp_inCharge)
    {
        pSTBatt->bmsTestProcessRecord[nBattIdx].MaxTemp_inCharge = TempCell_Max;
    }
    if(TempCell_Min<pSTBatt->bmsTestProcessRecord[nBattIdx].MinTemp_inCharge)
    {
        pSTBatt->bmsTestProcessRecord[nBattIdx].MinTemp_inCharge = TempCell_Min;
    }
    if((TempCell_Max-TempCell_Min)>pSTBatt->bmsTestProcessRecord[nBattIdx].RangeTemp_inCharge)
    {
        pSTBatt->bmsTestProcessRecord[nBattIdx].RangeTemp_inCharge = TempCell_Max-TempCell_Min;
    }
    if((3250 < VolCell_Avg && VolCell_Avg < 3430) && ((VolCell_Max-VolCell_Min)>pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inPlaCharge))
    {// avg vol in 3300~3430
        pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inPlaCharge = VolCell_Max-VolCell_Min;
    }
    else if((VolCell_Avg >= 3430) && ((VolCell_Max-VolCell_Min)>pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inTerminalCharge))
    {// avg vol >= 3430 
        pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inTerminalCharge = VolCell_Max-VolCell_Min;
    }
    // printf("RangeVol_inPlaCharge:%d\n", pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inPlaCharge);
    // printf("RangeVol_inTerminalCharge:%d\n", pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inTerminalCharge);
    if(pSTBatt->bmsTestProcessRecord[nBattIdx].StartEnergy_inCharge == 0)
    {
        pSTBatt->bmsTestProcessRecord[nBattIdx].StartEnergy_inCharge = ChargeAddAmount;
    }
    if(pSTBatt->bmsTestProcessRecord[nBattIdx].EndEnergy_inCharge < ChargeAddAmount)
    {
        pSTBatt->bmsTestProcessRecord[nBattIdx].EndEnergy_inCharge = ChargeAddAmount;
    }
    if(pSTBatt->bmsTestProcessRecord[nBattIdx].StartKWh_inCharge == 0)
    {
        pSTBatt->bmsTestProcessRecord[nBattIdx].StartKWh_inCharge = SystemChargeAddAmount;
    }
    if(pSTBatt->bmsTestProcessRecord[nBattIdx].EndKWh_inCharge < SystemChargeAddAmount)
    {
        pSTBatt->bmsTestProcessRecord[nBattIdx].EndKWh_inCharge = SystemChargeAddAmount;
    }
}

static void Update_DischargeData(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    //Debug
    // TRACE("IN PROCESS Discharge! n: %d, tempmax: %d, tempmin: %d, temprange: %d, volrange: %d\n", nBattIdx,\
    pSTBatt->bmsTestProcessRecord[nBattIdx].MaxTemp_inDischarge, pSTBatt->bmsTestProcessRecord[nBattIdx].MinTemp_inDischarge, \
    pSTBatt->bmsTestProcessRecord[nBattIdx].RangeTemp_inDischarge, pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inTerminalDischarge);
    // Voltage Cell info
    int VolCell_Max = pSTBatt->blobIfyBMS[nBattIdx].usMaxCellVlot_Volt;
    int VolCell_Min = pSTBatt->blobIfyBMS[nBattIdx].usMinCellVlot_Volt;
    int VolCell_Avg = pSTBatt->blobIfyBMS[nBattIdx].usCellAverageVolt; // 平均电压用于判断是否进入放电末端
    // printf("VolCell_Avg:%d\n", VolCell_Avg);//xxxsy
    // Temp Cell info
    int TempCell_Max = pSTBatt->blobIfyBMS[nBattIdx].sMaxCellTemp_Temp;
    int TempCell_Min = pSTBatt->blobIfyBMS[nBattIdx].sMinCellTemp_Temp;
    // Discharge Energy info
    float DischargeAddAmount = pSTBatt->blobIfyBMS[nBattIdx].fAccuDischgEnergy;
    // System Discharge kWh info
    float SystemDischargeAddAmount = pSTBatt->fCabMeter_Power[2];// 反向有功总电能

    if(TempCell_Max>pSTBatt->bmsTestProcessRecord[nBattIdx].MaxTemp_inDischarge)
    {
        pSTBatt->bmsTestProcessRecord[nBattIdx].MaxTemp_inDischarge = TempCell_Max;
    }
    if(TempCell_Min<pSTBatt->bmsTestProcessRecord[nBattIdx].MinTemp_inDischarge)
    {
        pSTBatt->bmsTestProcessRecord[nBattIdx].MinTemp_inDischarge = TempCell_Min;
    }
    if((TempCell_Max-TempCell_Min)>pSTBatt->bmsTestProcessRecord[nBattIdx].RangeTemp_inDischarge)
    {
        pSTBatt->bmsTestProcessRecord[nBattIdx].RangeTemp_inDischarge = TempCell_Max-TempCell_Min;
    }
    if((3100 < VolCell_Avg && VolCell_Avg < 3230) && ((VolCell_Max-VolCell_Min)>pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inPlaDischarge))
    {// avg vol in 3100~3230
        pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inPlaDischarge = VolCell_Max-VolCell_Min;
    }
    if((VolCell_Avg <= 3100) && ((VolCell_Max-VolCell_Min)>pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inTerminalDischarge))
    {// avg vol <= 3100 
        pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inTerminalDischarge = VolCell_Max-VolCell_Min;
    }
    // printf("RangeVol_inPlaDischarge:%d\n", pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inPlaDischarge);//xxxsy
    // printf("RangeVol_inTerminalDischarge:%d\n", pSTBatt->bmsTestProcessRecord[nBattIdx].RangeVol_inTerminalDischarge);
    if(pSTBatt->bmsTestProcessRecord[nBattIdx].StartEnergy_inDischarge == 0)
    {
        pSTBatt->bmsTestProcessRecord[nBattIdx].StartEnergy_inDischarge = DischargeAddAmount;
    }
    if(pSTBatt->bmsTestProcessRecord[nBattIdx].EndEnergy_inDischarge < DischargeAddAmount)
    {
        pSTBatt->bmsTestProcessRecord[nBattIdx].EndEnergy_inDischarge = DischargeAddAmount;
    }
    if(pSTBatt->bmsTestProcessRecord[nBattIdx].StartKWh_inDischarge == 0)
    {
        pSTBatt->bmsTestProcessRecord[nBattIdx].StartKWh_inDischarge = SystemDischargeAddAmount;
    }
    if(pSTBatt->bmsTestProcessRecord[nBattIdx].EndKWh_inDischarge < SystemDischargeAddAmount)
    {
        pSTBatt->bmsTestProcessRecord[nBattIdx].EndKWh_inDischarge = SystemDischargeAddAmount;
    }
}

static void Init_ProcessRecord(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    A_BLOB_BMS_TESTMODE_PROCESS initData = 
    {
    .MaxTemp_inCharge = 0,
    .RangeTemp_inCharge = 0,
    .RangeVol_inTerminalCharge = 0,
    .RangeVol_inPlaCharge = 0,
    .MinTemp_inCharge = 1000,

    .MaxTemp_inDischarge = 0,
    .RangeTemp_inDischarge = 0,
    .RangeVol_inTerminalDischarge = 0,
    .RangeVol_inPlaDischarge = 0,
    .MinTemp_inDischarge = 1000
    };

    pSTBatt->bmsTestProcessRecord[nBattIdx] = initData; 

    TRACE("n: %d, init process record!\n", nBattIdx);
    
}

static void Push_TestState(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    pSTBatt->bmsTestModeParm.testState[nBattIdx] = pSTBatt->bmsAutoTestResult[nBattIdx].testState;
    pSTBatt->bmsTestModeParm.timeRemain[nBattIdx] = pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain;
    VAR_VALUE varV;
    varV.blobValue = (BYTE *)&pSTBatt->bmsTestModeParm;
    // TRACE("----------------before push----------------\n");
    Push_BlobValue(SETTYPE_INTERN_SETT, pSTBatt->hSampler, 
		500, 202, (void*)varV.blobValue, sizeof(A_BLOB_BMS_TESTMODE_SET), BLOB_TYPE_BMS_TESTMODE_SET);
}

static BOOL Initial_TestMode_MachineState_Tow(STBATT_SAMPLER_ROUGH *pSTBatt)
{   
    TRACE("Enter init TestMode successfully!\n");
    STATE_MACHINE_ITEM_TEST  *p = &pSTBatt->stuTestMode.stateMachineLoop[0];

    INIT_TEST_STATE_MACHINE(p,  STATE_IDLE,                 OnTest_Idle,             FAIL_FROM_NONE);
    INIT_TEST_STATE_MACHINE(p,  STATE_SELF_CHECK,           OnTest_Self_Check,       FAIL_FROM_NONE);
    INIT_TEST_STATE_MACHINE(p,  STATE_FULLDSICHARGE_STEP1,  OnTest_FullDischg_Step1, FAIL_FROM_NONE);
    INIT_TEST_STATE_MACHINE(p,  STATE_SLEEP_STEP2,          OnTest_Sleep_Step2,      FAIL_FROM_NONE);
    INIT_TEST_STATE_MACHINE(p,  STATE_FULLCHARGING_STEP3,   OnTest_FullChg_Step3,    FAIL_FROM_NONE);
    INIT_TEST_STATE_MACHINE(p,  STATE_SLEEP_STEP4,          OnTest_Sleep_Step4,      FAIL_FROM_NONE);
    INIT_TEST_STATE_MACHINE(p,  STATE_FULLDSICHARGE_STEP5,  OnTest_FullDischg_Step5, FAIL_FROM_NONE);
    INIT_TEST_STATE_MACHINE(p,  STATE_SLEEP_STEP6,          OnTest_Sleep_Step6,      FAIL_FROM_NONE);
    INIT_TEST_STATE_MACHINE(p,  STATE_CHARGE_HALF_STEP7,    OnTest_Chg_Half_Step7,   FAIL_FROM_NONE);
    INIT_TEST_STATE_MACHINE(p,  STATE_TEST_FINISHED,        OnTest_Finished,    	 FAIL_FROM_NONE);
    INIT_TEST_STATE_MACHINE(p,  STATE_ON_PAUSED,            OnTest_Paused,    	     FAIL_FROM_NONE);

	
    return TRUE;
}

static void Prepare_Test_StateMachine_Tow(STBATT_SAMPLER_ROUGH *pSTBatt)
{   
    TRACE("Enter prepare TestMode successfully!\n");
    //Init state
    int n = 0;
    for(n =0; n < MAX_SINGLE_STBATT_NUM; n++)
    {
        pSTBatt->stuTestMode.pfnOnFail[n] = OnFail;

        pSTBatt->stuTestMode.pCurrState[n] = &pSTBatt->stuTestMode.stateMachineLoop[STATE_IDLE];
        pSTBatt->stuTestMode.pLastState[n] = pSTBatt->stuTestMode.pCurrState;
        pSTBatt->stuTestMode.nFailTimes[n] = 0;
        pSTBatt->stuTestMode.emTrigState[n] = FALSE;

        // TRACE("-------- n=%d --------\n",n);
        // TRACE("pfnOnFail:%d\n",pSTBatt->stuTestMode.pfnOnFail[n]);
        // TRACE("pCurrState:%d\n",pSTBatt->stuTestMode.pCurrState[n]);
        // TRACE("pLastState:%d\n",pSTBatt->stuTestMode.pLastState[n]);
        // TRACE("nFailClass:%d\n",pSTBatt->stuTestMode.nFailClass[n]);
        // TRACE("emTrigState:%d\n",pSTBatt->stuTestMode.emTrigState[n]);

    }
    return;
}
/*
static BOOL Charge_Control_STEP1(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP1;

    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];
    A_BLOB_SINGLE_STBATT_RUNINFO *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];

	double tmNow = GetCurrentTime();

    // if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 12*60*60)  
    // {
 	//     pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_FULLCHG_STEP3;
	// 	return STATE_FAIL; 
    // }

    //Adjust BMS Output Volt And Current 
    
    //Calc_CostEnergy(pSTBatt, nBattIdx);

    //Predict_RemainTime(pSTBatt, nBattIdx);

    //Adjust BMS RealTime Output Volt/Current
    float fLimitCurrent = 0.0;
    float fACDC_Distri_4_BattChrg_Pwr = (float)pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nPwrLmtStep1;
   
    if(pSiglBatt->fPackTotalVolt > 50.0)
	{
		fLimitCurrent = (fACDC_Distri_4_BattChrg_Pwr*1000)/pSiglBatt->fPackTotalVolt;
	}

    float fVoltReqDiff = fabs(pSTBatt->fLastSendReqVolt_2CCU[nBattIdx] - MIN(pSiglBatt->fAllow_LmtChargeVolt, 1000));
	float fCurrReqDiff = fabs(pSTBatt->fLastSendReqCurr_2CCU[nBattIdx] - MIN(pSiglBatt->fAllow_MaxChargeCurr, fLimitCurrent));
 	float fCurrOutDiff = pSiglBatt->fPackTotalCurr - pSiglBatt->fAllow_MaxChargeCurr;
	//2021-11-17,MPPt Output Current will Cause PackCurr Over MAx_AllowCurr
	TRACEX("tmNow=%f nBattIdx=[%d] fVoltReqDiff=[%.1f], fCurrReqDiff=[%.1f]fCurrOutDiff=%f,\
		fPackTotalCurr=%f,fLastSendReqCurr_2CCU=[%.1f] fAllow_MaxChargeCurr=[%.1f] fACDC_Distri_4_BattChrg_Pwr=[%.1f]\n",\
        tmNow, nBattIdx, fVoltReqDiff, fCurrReqDiff, fCurrOutDiff, pSiglBatt->fPackTotalCurr,\
        pSTBatt->fLastSendReqCurr_2CCU[nBattIdx], pSiglBatt->fAllow_MaxChargeCurr,\
        fACDC_Distri_4_BattChrg_Pwr);

     if((fabs(fVoltReqDiff) > 2.0) || 
        (fabs(fCurrReqDiff) > 2.0) ||
        (fCurrOutDiff > 2.0)) 
	{
		if((fabs(tmNow - pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD))//CATL Need 3S Adjust to 0A,Otherwise BMS will Be OverCurrent Alarm
		{	
			TRACEX("Test Mode:Charge Adjust Cmd:tmNow=%f - tm_fLastSendCmd_2CCU[%d]=%lf\n", \
			tmNow , nBattIdx, pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]);
			if((pSiglBatt->fAllow_MaxChargeCurr < 20.0) || (fabs(fCurrReqDiff) > 10))
			{
				LogOut(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
					"Adj:Idx=[%d]IPack[%.1f]Imax[%.0f]Ilmt[%.0f]Ilast[%.0f]Irect[%.1f]SOC[%d]!", \
					nBattIdx,pSiglBatt->fPackTotalCurr,pSiglBatt->fAllow_MaxChargeCurr,\
					fLimitCurrent,pSTBatt->fLastSendReqCurr_2CCU[nBattIdx],\
					pSTBatt->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSiglBatt->nSOC);
			}
            Send_CtrlCmd(pSTBatt, nBattIdx, STAT_MANUAL_START_CHG, fLimitCurrent);

		}		
	}

    float   fSOC = (float)pSiglBatt->nSOC;
    BOOL    bFull_Chged = pSiglBatt->bIsBatt_ChargeForbidden;
    float   fAllow_chg_Current = pSiglBatt->fAllow_MaxChargeCurr;

    // float stopSOC = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nCurLmtStep1;
    float stopSOC = 100;
    // float minCur = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nCurLmtStep3;
    
    // Update_ChargeData(pSTBatt, nBattIdx);
    float Charge_Energy = pSTBatt->blobIfyBMS[nBattIdx].fSingleChgCapacity;
    // TRACE("!!! n: %d, chargeamount: %.1f\n", nBattIdx, Charge_Energy);

    float VolCell_Max = pSTBatt->blobIfyBMS[nBattIdx].usMaxCellVlot_Volt;
    float TH_nChargeEndVol_Max = pSTBatt->bmsTestModeParm.initRange.TH_nChargeEndVol_Max;

    // TRACE("vcell max: %.1f ? setting: %.1f\n", VolCell_Max, TH_nChargeEndVol_Max);
    // if((VolCell_Max>=stopVol) || (fAllow_chg_Current < minCur))
    // if((fSOC >= stopSOC) || (VolCell_Max>=3600) || (bFull_Chged==TRUE)) 
    // if(bFull_Chged==TRUE)
    // Short Time Test
    // if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 5*60)
    // {   
    //     // STATE_TEST_RES result;
    //     // TRACE("before charge test\n");
    //     // result = ChargeEndTest(pSTBatt, nBattIdx);
    //     // // if(result == TEST_FAIL)
    //     // // {
    //     // //     return STATE_FAIL;
    //     // // }
    //     // Push_TestState(pSTBatt, nBattIdx);
    //     TRACE("over stop soc, send stop cmd!!!\n");
    //     pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP2;
    //     Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
    //     return TRUE;
    // }

    // Calculate remaining time
    float RemainToCharge_Energy = pSTBatt->blobIfyBMS[nBattIdx].fTotalEnergy - pSTBatt->blobIfyBMS[nBattIdx].fRemainEnergy;
    float Remain_Time = 60 * ((RemainToCharge_Energy * 1000) / (pSiglBatt->fPackTotalVolt * pSiglBatt->fPackTotalCurr));
    pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
    Push_TestState(pSTBatt, nBattIdx);

    return FALSE; 
}

static BOOL Discharge_Control_STEP1(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP1;

    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];
    A_BLOB_SINGLE_STBATT_RUNINFO *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];

	double tmNow = GetCurrentTime();
    // if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 12*60*60)
    // {
    //     pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_FULLDISCHG_STEP1;
	// 	return STATE_FAIL; 
    // }

    //Adjust BMS RealTime Output Volt/Current
    float fCalc_Batt_Distri_4_ACDC_Pwr = (float)pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nPwrLmtStep1;
    TRACE("set power limit: %f\n", fCalc_Batt_Distri_4_ACDC_Pwr);
    float fLimitCurrent = 0.0;

    if(pSiglBatt->fPackTotalVolt > 1.0)
	{
		fLimitCurrent = (fCalc_Batt_Distri_4_ACDC_Pwr*1000)/pSiglBatt->fPackTotalVolt;
	}

    float fCurrReqDiff = fabs(fabs(pSTBatt->fLastSendReqCurr_2CCU[nBattIdx]) - MIN(fLimitCurrent, fabs(pSiglBatt->fAllow_MaxDisChargeCurr)));
	float fCurrOutDiff = fabs(fabs(pSiglBatt->fPackTotalCurr) - MIN(fLimitCurrent, fabs(pSiglBatt->fAllow_MaxDisChargeCurr)));

    if((fabs(fCurrReqDiff) > 2.0) || ((fCurrOutDiff > 3.0) && (pSiglBatt->fPackTotalCurr < -3.0)))
		{
			if((fabs(tmNow - pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD))//Step Speed to 1.0 Cycle to Adjust the Power 
			{			
                Send_CtrlCmd(pSTBatt, nBattIdx, STAT_MANUAL_START_DISCHG, fLimitCurrent);
                
                TRACE("Test Mode : DisCharg Adjust Cmd: nBattIdx=[%d] Adjust ChargePower fCurrReqDiff=%f fCurrOutDiff=%f fLimitCurrent=%.1f fLastSendReqCurr_2CCU=%.1f fPackTotalCurr=%.1f\n",\
				nBattIdx, fCurrReqDiff, fCurrOutDiff ,fLimitCurrent ,\
				pSTBatt->fLastSendReqCurr_2CCU[nBattIdx], pSiglBatt->fPackTotalCurr);

			}		
		}	

    float   fSOC = (float)pSiglBatt->nSOC;
    BOOL    bFull_DisChged = pSiglBatt->bIsBatt_DisChargeForbidden;
    float   fAllow_Dischg_Current = pSiglBatt->fAllow_MaxDisChargeCurr;

    Update_DischargeData(pSTBatt, nBattIdx);
    // float stopSOC = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nStopSOCStep1;
    float stopSOC = 0;
    // float minCur = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nCurLmtStep1;
    
    float VolCell_Min = pSTBatt->blobIfyBMS[nBattIdx].usMinCellVlot_Volt;
    float TH_nDischargeEndVol_Min = pSTBatt->bmsTestModeParm.initRange.TH_nDischargeEndVol_Min;

    // if((fSOC <= stopSOC) || (VolCell_Min <= 2850) || (bFull_DisChged==TRUE))
    // if(bFull_DisChged==TRUE)
    // Short Time Test
    // if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 5*60)
    // {   
    //     pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP2;
    //     Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
    //     return TRUE; 
    // }

    // Calculate remaining time
    float Remain_Energy = pSTBatt->blobIfyBMS[nBattIdx].fRemainEnergy;
    float Remain_Time = 60 * ((Remain_Energy * 1000) / (pSiglBatt->fPackTotalVolt * pSiglBatt->fPackTotalCurr));
    pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
    Push_TestState(pSTBatt, nBattIdx);

    return FALSE;
}

static BOOL Charge_Control_STEP3(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP3;

    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];
    A_BLOB_SINGLE_STBATT_RUNINFO *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];

	double tmNow = GetCurrentTime();

    // if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 12*60*60)  
    // {
 	//     pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_FULLCHG_STEP3;
	// 	return STATE_FAIL; 
    // }

    //Adjust BMS Output Volt And Current 
    
    //Calc_CostEnergy(pSTBatt, nBattIdx);

    //Predict_RemainTime(pSTBatt, nBattIdx);

    //Adjust BMS RealTime Output Volt/Current
    float fLimitCurrent = 0.0;
    float fACDC_Distri_4_BattChrg_Pwr = (float)pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nPwrLmtStep3;
   
    if(pSiglBatt->fPackTotalVolt > 50.0)
	{
		fLimitCurrent = (fACDC_Distri_4_BattChrg_Pwr*1000)/pSiglBatt->fPackTotalVolt;
	}

    float fVoltReqDiff = fabs(pSTBatt->fLastSendReqVolt_2CCU[nBattIdx] - MIN(pSiglBatt->fAllow_LmtChargeVolt, 1000));
	float fCurrReqDiff = fabs(pSTBatt->fLastSendReqCurr_2CCU[nBattIdx] - MIN(pSiglBatt->fAllow_MaxChargeCurr, fLimitCurrent));
 	float fCurrOutDiff = pSiglBatt->fPackTotalCurr - pSiglBatt->fAllow_MaxChargeCurr;
	//2021-11-17,MPPt Output Current will Cause PackCurr Over MAx_AllowCurr
	TRACEX("tmNow=%f nBattIdx=[%d] fVoltReqDiff=[%.1f], fCurrReqDiff=[%.1f]fCurrOutDiff=%f,\
		fPackTotalCurr=%f,fLastSendReqCurr_2CCU=[%.1f] fAllow_MaxChargeCurr=[%.1f] fACDC_Distri_4_BattChrg_Pwr=[%.1f]\n",\
        tmNow, nBattIdx, fVoltReqDiff, fCurrReqDiff, fCurrOutDiff, pSiglBatt->fPackTotalCurr,\
        pSTBatt->fLastSendReqCurr_2CCU[nBattIdx], pSiglBatt->fAllow_MaxChargeCurr,\
        fACDC_Distri_4_BattChrg_Pwr);

     if((fabs(fVoltReqDiff) > 2.0) || 
        (fabs(fCurrReqDiff) > 2.0) ||
        (fCurrOutDiff > 2.0)) 
	{
		if((fabs(tmNow - pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD))//CATL Need 3S Adjust to 0A,Otherwise BMS will Be OverCurrent Alarm
		{	
			TRACEX("Test Mode:Charge Adjust Cmd:tmNow=%f - tm_fLastSendCmd_2CCU[%d]=%lf\n", \
			tmNow , nBattIdx, pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]);
			if((pSiglBatt->fAllow_MaxChargeCurr < 20.0) || (fabs(fCurrReqDiff) > 10))
			{
				LogOut(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
					"Adj:Idx=[%d]IPack[%.1f]Imax[%.0f]Ilmt[%.0f]Ilast[%.0f]Irect[%.1f]SOC[%d]!", \
					nBattIdx,pSiglBatt->fPackTotalCurr,pSiglBatt->fAllow_MaxChargeCurr,\
					fLimitCurrent,pSTBatt->fLastSendReqCurr_2CCU[nBattIdx],\
					pSTBatt->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSiglBatt->nSOC);
			}
            Send_CtrlCmd(pSTBatt, nBattIdx, STAT_MANUAL_START_CHG, fLimitCurrent);

		}		
	}

    float   fSOC = (float)pSiglBatt->nSOC;
    BOOL    bFull_Chged = pSiglBatt->bIsBatt_ChargeForbidden;
    float   fAllow_chg_Current = pSiglBatt->fAllow_MaxChargeCurr;

    // float stopSOC = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nStopSOCStep3;
    float stopSOC = 100;
    // float minCur = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nCurLmtStep3;
    
    Update_ChargeData(pSTBatt, nBattIdx);
    float Charge_Energy = pSTBatt->blobIfyBMS[nBattIdx].fSingleChgCapacity;
    // TRACE("!!! n: %d, chargeamount: %.1f\n", nBattIdx, Charge_Energy);

    float VolCell_Max = pSTBatt->blobIfyBMS[nBattIdx].usMaxCellVlot_Volt;
    float TH_nChargeEndVol_Max = pSTBatt->bmsTestModeParm.initRange.TH_nChargeEndVol_Max;

    // TRACE("vcell max: %.1f ? setting: %.1f\n", VolCell_Max, TH_nChargeEndVol_Max);
    // if((VolCell_Max>=stopVol) || (fAllow_chg_Current < minCur))
    // if((fSOC >= stopSOC) || (VolCell_Max>=3600) || (bFull_Chged==TRUE)) 
    // Short Time Test
    // if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 5*60)
    // {   
    //     STATE_TEST_RES result;
    //     TRACE("before charge test\n");
    //     result = ChargeEndTest(pSTBatt, nBattIdx);
    //     // // if(result == TEST_FAIL)
    //     // // {
    //     // //     return STATE_FAIL;
    //     // // }
    //     // Push_TestState(pSTBatt, nBattIdx);
    //     // TRACE("send stop cmd!!!\n");
    //     pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP4;
    //     Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
    //     return TRUE;
    // }

    // Calculate remaining time
    float RemainToCharge_Energy = pSTBatt->blobIfyBMS[nBattIdx].fTotalEnergy - pSTBatt->blobIfyBMS[nBattIdx].fRemainEnergy;
    float Remain_Time = 60 * ((RemainToCharge_Energy * 1000) / (pSiglBatt->fPackTotalVolt * pSiglBatt->fPackTotalCurr));
    pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
    Push_TestState(pSTBatt, nBattIdx);

    return FALSE; 
}

static BOOL Discharge_Control_STEP3(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP3;

    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];
    A_BLOB_SINGLE_STBATT_RUNINFO *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];

    time_t tmNow = time(NULL);

    // if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 12*60*60) //Over 6Hour,Exit
    // {
    //     pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_FULLDISCHG_STEP5;
	// 	return STATE_FAIL; 
    // }

    //Calc_CostEnergy();

    //Predict_RemainTime();

    //Adjust BMS RealTime Output Volt/Current
    float fCalc_Batt_Distri_4_ACDC_Pwr = (float)pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nPwrLmtStep3;
    float fLimitCurrent = 0.0;

    if(pSiglBatt->fPackTotalVolt > 1.0)
	{
		fLimitCurrent = (fCalc_Batt_Distri_4_ACDC_Pwr*1000)/pSiglBatt->fPackTotalVolt;
	}

    float fCurrReqDiff = fabs(fabs(pSTBatt->fLastSendReqCurr_2CCU[nBattIdx]) - MIN(fLimitCurrent, fabs(pSiglBatt->fAllow_MaxDisChargeCurr)));
	float fCurrOutDiff = fabs(fabs(pSiglBatt->fPackTotalCurr) - MIN(fLimitCurrent, fabs(pSiglBatt->fAllow_MaxDisChargeCurr)));

    if((fabs(fCurrReqDiff) > 2.0) || ((fCurrOutDiff > 3.0) && (pSiglBatt->fPackTotalCurr < -3.0)))
		{
			if((fabs(tmNow - pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD))//Step Speed to 1.0 Cycle to Adjust the Power 
			{			
                Send_CtrlCmd(pSTBatt, nBattIdx, STAT_MANUAL_START_DISCHG, fLimitCurrent);
				
				TRACEX("Test Mode : DisCharg Adjust Cmd: nBattIdx=[%d] Adjust ChargePower fCurrReqDiff=%f fCurrOutDiff=%f\
				fLimitCurrent=%.1f fLastSendReqCurr_2CCU=%.1f fPackTotalCurr=%.1f\n",\
				nBattIdx, fCurrReqDiff, fCurrOutDiff ,fLimitCurrent ,\
				pSTBatt->fLastSendReqCurr_2CCU[nBattIdx], pSiglBatt->fPackTotalCurr);
                
                // TRACE("Test Mode : DisCharg Adjust Cmd: nBattIdx=[%d] Adjust ChargePower fCurrReqDiff=%f fCurrOutDiff=%f fLimitCurrent=%.1f fLastSendReqCurr_2CCU=%.1f fPackTotalCurr=%.1f\n",\
				// nBattIdx, fCurrReqDiff, fCurrOutDiff ,fLimitCurrent ,\
				// pSTBatt->fLastSendReqCurr_2CCU[nBattIdx], pSiglBatt->fPackTotalCurr);

			}		
		}	

    float   fSOC = (float)pSiglBatt->nSOC;
    BOOL    bFull_DisChged = pSiglBatt->bIsBatt_DisChargeForbidden;
    float   fAllow_Dischg_Current = pSiglBatt->fAllow_MaxDisChargeCurr;

    // float stopSOC = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nCurLmtStep3;
    float stopSOC = 0;
    // float minCur = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nCurLmtStep5;

    // Update_DischargeData(pSTBatt, nBattIdx);
    float DisCharge_Energy = pSTBatt->blobIfyBMS[nBattIdx].fSingleDischgCapacity;

    float VolCell_Min = pSTBatt->blobIfyBMS[nBattIdx].usMinCellVlot_Volt;
    // TRACE("volcell min: %f\n", VolCell_Min);
    float TH_nDischargeEndVol_Min = pSTBatt->bmsTestModeParm.initRange.TH_nDischargeEndVol_Min;

    float   fAllow_chg_Current = pSiglBatt->fAllow_MaxChargeCurr;
    // TRACE("fallowcur: %f\n", fAllow_chg_Current);
    // if((VolCell_Min<=stopVol)|| (fAllow_chg_Current < minCur))
    // if(VolCell_Min<=stopVol)
    // if((fSOC <= stopSOC) || (VolCell_Min <= 2850) || (bFull_DisChged==TRUE))
    // Short Time Test
    // if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 5*60)
    // {   
    //     TRACE("enter to send stopcmd!!!!\n");
    //     STATE_TEST_RES result;
    //     result = DischargeEndTest(pSTBatt, nBattIdx);
    //     // if(result == TEST_FAIL)
    //     // {
    //     //     return STATE_FAIL;
    //     // }
    //     // Push_TestState(pSTBatt, nBattIdx);
    //     pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP4;
    //     Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
    //     return TRUE;
    // }

    // Calculate remaining time
    float Remain_Energy = pSTBatt->blobIfyBMS[nBattIdx].fRemainEnergy;
    float Remain_Time = 60 * ((Remain_Energy * 1000) / (pSiglBatt->fPackTotalVolt * pSiglBatt->fPackTotalCurr));
    pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
    Push_TestState(pSTBatt, nBattIdx);

    return FALSE;
} 

static BOOL Charge_Control_STEP5(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP5;

    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];
    A_BLOB_SINGLE_STBATT_RUNINFO *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];

	double tmNow = GetCurrentTime();

    // if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 12*60*60)  
    // {
 	//     pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_FULLCHG_STEP3;
	// 	return STATE_FAIL; 
    // }

    //Adjust BMS Output Volt And Current 
    
    //Calc_CostEnergy(pSTBatt, nBattIdx);

    //Predict_RemainTime(pSTBatt, nBattIdx);

    //Adjust BMS RealTime Output Volt/Current
    float fLimitCurrent = 0.0;
    float fACDC_Distri_4_BattChrg_Pwr = (float)pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nPwrLmtStep5;
   
    if(pSiglBatt->fPackTotalVolt > 50.0)
	{
		fLimitCurrent = (fACDC_Distri_4_BattChrg_Pwr*1000)/pSiglBatt->fPackTotalVolt;
	}

    float fVoltReqDiff = fabs(pSTBatt->fLastSendReqVolt_2CCU[nBattIdx] - MIN(pSiglBatt->fAllow_LmtChargeVolt, 1000));
	float fCurrReqDiff = fabs(pSTBatt->fLastSendReqCurr_2CCU[nBattIdx] - MIN(pSiglBatt->fAllow_MaxChargeCurr, fLimitCurrent));
 	float fCurrOutDiff = pSiglBatt->fPackTotalCurr - pSiglBatt->fAllow_MaxChargeCurr;
	//2021-11-17,MPPt Output Current will Cause PackCurr Over MAx_AllowCurr
	TRACEX("tmNow=%f nBattIdx=[%d] fVoltReqDiff=[%.1f], fCurrReqDiff=[%.1f]fCurrOutDiff=%f,\
		fPackTotalCurr=%f,fLastSendReqCurr_2CCU=[%.1f] fAllow_MaxChargeCurr=[%.1f] fACDC_Distri_4_BattChrg_Pwr=[%.1f]\n",\
        tmNow, nBattIdx, fVoltReqDiff, fCurrReqDiff, fCurrOutDiff, pSiglBatt->fPackTotalCurr,\
        pSTBatt->fLastSendReqCurr_2CCU[nBattIdx], pSiglBatt->fAllow_MaxChargeCurr,\
        fACDC_Distri_4_BattChrg_Pwr);

     if((fabs(fVoltReqDiff) > 2.0) || 
        (fabs(fCurrReqDiff) > 2.0) ||
        (fCurrOutDiff > 2.0)) 
	{
		if((fabs(tmNow - pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD))//CATL Need 3S Adjust to 0A,Otherwise BMS will Be OverCurrent Alarm
		{	
			TRACEX("Test Mode:Charge Adjust Cmd:tmNow=%f - tm_fLastSendCmd_2CCU[%d]=%lf\n", \
			tmNow , nBattIdx, pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]);
			if((pSiglBatt->fAllow_MaxChargeCurr < 20.0) || (fabs(fCurrReqDiff) > 10))
			{
				LogOut(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
					"Adj:Idx=[%d]IPack[%.1f]Imax[%.0f]Ilmt[%.0f]Ilast[%.0f]Irect[%.1f]SOC[%d]!", \
					nBattIdx,pSiglBatt->fPackTotalCurr,pSiglBatt->fAllow_MaxChargeCurr,\
					fLimitCurrent,pSTBatt->fLastSendReqCurr_2CCU[nBattIdx],\
					pSTBatt->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSiglBatt->nSOC);
			}
            Send_CtrlCmd(pSTBatt, nBattIdx, STAT_MANUAL_START_CHG, fLimitCurrent);

		}		
	}

    float   fSOC = (float)pSiglBatt->nSOC;
    BOOL    bFull_Chged = pSiglBatt->bIsBatt_ChargeForbidden;
    float   fAllow_chg_Current = pSiglBatt->fAllow_MaxChargeCurr;

    // float stopSOC = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nCurLmtStep5;
    float stopSOC = 100;
    // float minCur = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nCurLmtStep3;
    
    // Update_ChargeData(pSTBatt, nBattIdx);
    float Charge_Energy = pSTBatt->blobIfyBMS[nBattIdx].fSingleChgCapacity;
    // TRACE("!!! n: %d, chargeamount: %.1f\n", nBattIdx, Charge_Energy);

    float VolCell_Max = pSTBatt->blobIfyBMS[nBattIdx].usMaxCellVlot_Volt;
    float TH_nChargeEndVol_Max = pSTBatt->bmsTestModeParm.initRange.TH_nChargeEndVol_Max;

    // TRACE("vcell max: %.1f ? setting: %.1f\n", VolCell_Max, TH_nChargeEndVol_Max);
    // if((VolCell_Max>=stopVol) || (fAllow_chg_Current < minCur))
    // if((fSOC >= stopSOC) || (VolCell_Max>=3600) || (bFull_Chged==TRUE)) 
    // Short Time Test
    // if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 5*60) 
    // {   
    //     STATE_TEST_RES result;
    //     TRACE("before charge test\n");
    //     result = ChargeEndTest(pSTBatt, nBattIdx);
    //     TotalTest(pSTBatt, nBattIdx);
    //     // // if(result == TEST_FAIL)
    //     // // {
    //     // //     return STATE_FAIL;
    //     // // }
    //     // Push_TestState(pSTBatt, nBattIdx);
    //     // TRACE("send stop cmd!!!\n");
    //     pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP6;
    //     Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
    //     return TRUE;
    // }

    // Calculate remaining time
    float RemainToCharge_Energy = pSTBatt->blobIfyBMS[nBattIdx].fTotalEnergy - pSTBatt->blobIfyBMS[nBattIdx].fRemainEnergy;
    float Remain_Time = 60 * ((RemainToCharge_Energy * 1000) / (pSiglBatt->fPackTotalVolt * pSiglBatt->fPackTotalCurr));
    pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
    Push_TestState(pSTBatt, nBattIdx);

    return FALSE; 
}

static BOOL Discharge_Control_STEP5(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP5;

    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];
    A_BLOB_SINGLE_STBATT_RUNINFO *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];

    time_t tmNow = time(NULL);

    // if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 12*60*60) //Over 6Hour,Exit
    // {
    //     pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_FULLDISCHG_STEP5;
	// 	return STATE_FAIL; 
    // }

    //Calc_CostEnergy();

    //Predict_RemainTime();

    //Adjust BMS RealTime Output Volt/Current
    float fCalc_Batt_Distri_4_ACDC_Pwr = (float)pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nPwrLmtStep5;
    float fLimitCurrent = 0.0;

    if(pSiglBatt->fPackTotalVolt > 1.0)
	{
		fLimitCurrent = (fCalc_Batt_Distri_4_ACDC_Pwr*1000)/pSiglBatt->fPackTotalVolt;
	}

    float fCurrReqDiff = fabs(fabs(pSTBatt->fLastSendReqCurr_2CCU[nBattIdx]) - MIN(fLimitCurrent, fabs(pSiglBatt->fAllow_MaxDisChargeCurr)));
	float fCurrOutDiff = fabs(fabs(pSiglBatt->fPackTotalCurr) - MIN(fLimitCurrent, fabs(pSiglBatt->fAllow_MaxDisChargeCurr)));

    if((fabs(fCurrReqDiff) > 2.0) || ((fCurrOutDiff > 3.0) && (pSiglBatt->fPackTotalCurr < -3.0)))
		{
			if((fabs(tmNow - pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD))//Step Speed to 1.0 Cycle to Adjust the Power 
			{			
                Send_CtrlCmd(pSTBatt, nBattIdx, STAT_MANUAL_START_DISCHG, fLimitCurrent);
				
				TRACEX("Test Mode : DisCharg Adjust Cmd: nBattIdx=[%d] Adjust ChargePower fCurrReqDiff=%f fCurrOutDiff=%f\
				fLimitCurrent=%.1f fLastSendReqCurr_2CCU=%.1f fPackTotalCurr=%.1f\n",\
				nBattIdx, fCurrReqDiff, fCurrOutDiff ,fLimitCurrent ,\
				pSTBatt->fLastSendReqCurr_2CCU[nBattIdx], pSiglBatt->fPackTotalCurr);
                
                TRACE("Test Mode : DisCharg Adjust Cmd: nBattIdx=[%d] Adjust ChargePower fCurrReqDiff=%f fCurrOutDiff=%f fLimitCurrent=%.1f fLastSendReqCurr_2CCU=%.1f fPackTotalCurr=%.1f\n",\
				nBattIdx, fCurrReqDiff, fCurrOutDiff ,fLimitCurrent ,\
				pSTBatt->fLastSendReqCurr_2CCU[nBattIdx], pSiglBatt->fPackTotalCurr);

			}		
		}	

    float   fSOC = (float)pSiglBatt->nSOC;
    BOOL    bFull_DisChged = pSiglBatt->bIsBatt_DisChargeForbidden;
    float   fAllow_Dischg_Current = pSiglBatt->fAllow_MaxDisChargeCurr;

    // float stopSOC = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nStopSOCStep5;
    float stopSOC = 0;
    // float minCur = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nCurLmtStep5;

    Update_DischargeData(pSTBatt, nBattIdx);
    float DisCharge_Energy = pSTBatt->blobIfyBMS[nBattIdx].fSingleDischgCapacity;

    float VolCell_Min = pSTBatt->blobIfyBMS[nBattIdx].usMinCellVlot_Volt;
    // TRACE("volcell min: %f\n", VolCell_Min);
    float TH_nDischargeEndVol_Min = pSTBatt->bmsTestModeParm.initRange.TH_nDischargeEndVol_Min;

    float   fAllow_chg_Current = pSiglBatt->fAllow_MaxChargeCurr;
    // TRACE("fallowcur: %f\n", fAllow_chg_Current);
    // if((VolCell_Min<=stopVol)|| (fAllow_chg_Current < minCur))
    // if(VolCell_Min<=stopVol)
    // if((fSOC <= stopSOC) || (VolCell_Min <= 2850) || (bFull_DisChged==TRUE))
    // Short Time Test
    // if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 5*60)
    // {   
    //     TRACE("enter to send stopcmd!!!!\n");
    //     STATE_TEST_RES result;
    //     result = DischargeEndTest(pSTBatt, nBattIdx);
    //     TotalTest(pSTBatt, nBattIdx);
    //     // if(result == TEST_FAIL)
    //     // {
    //     //     return STATE_FAIL;
    //     // }
    //     // Push_TestState(pSTBatt, nBattIdx);
    //     pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP6;
    //     Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
    //     return TRUE;
    // }

    // Calculate remaining time
    float Remain_Energy = pSTBatt->blobIfyBMS[nBattIdx].fRemainEnergy;
    float Remain_Time = 60 * ((Remain_Energy * 1000) / (pSiglBatt->fPackTotalVolt * pSiglBatt->fPackTotalCurr));
    pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
    Push_TestState(pSTBatt, nBattIdx);

    return FALSE;
} 

static BOOL Charge_Control_STEP7(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP7;

    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];
    A_BLOB_SINGLE_STBATT_RUNINFO *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];

	double tmNow = GetCurrentTime();

    // if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 12*60*60)  
    // {
 	//     pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_FULLCHG_STEP3;
	// 	return STATE_FAIL; 
    // }

    //Adjust BMS Output Volt And Current 
    
    //Calc_CostEnergy(pSTBatt, nBattIdx);

    //Predict_RemainTime(pSTBatt, nBattIdx);

    //Adjust BMS RealTime Output Volt/Current
    float fLimitCurrent = 0.0;
    float fACDC_Distri_4_BattChrg_Pwr = (float)pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nPwrLmtStep7;
   
    if(pSiglBatt->fPackTotalVolt > 50.0)
	{
		fLimitCurrent = (fACDC_Distri_4_BattChrg_Pwr*1000)/pSiglBatt->fPackTotalVolt;
	}

    float fVoltReqDiff = fabs(pSTBatt->fLastSendReqVolt_2CCU[nBattIdx] - MIN(pSiglBatt->fAllow_LmtChargeVolt, 1000));
	float fCurrReqDiff = fabs(pSTBatt->fLastSendReqCurr_2CCU[nBattIdx] - MIN(pSiglBatt->fAllow_MaxChargeCurr, fLimitCurrent));
 	float fCurrOutDiff = pSiglBatt->fPackTotalCurr - pSiglBatt->fAllow_MaxChargeCurr;
	//2021-11-17,MPPt Output Current will Cause PackCurr Over MAx_AllowCurr
	TRACEX("tmNow=%f nBattIdx=[%d] fVoltReqDiff=[%.1f], fCurrReqDiff=[%.1f]fCurrOutDiff=%f,\
		fPackTotalCurr=%f,fLastSendReqCurr_2CCU=[%.1f] fAllow_MaxChargeCurr=[%.1f] fACDC_Distri_4_BattChrg_Pwr=[%.1f]\n",\
        tmNow, nBattIdx, fVoltReqDiff, fCurrReqDiff, fCurrOutDiff, pSiglBatt->fPackTotalCurr,\
        pSTBatt->fLastSendReqCurr_2CCU[nBattIdx], pSiglBatt->fAllow_MaxChargeCurr,\
        fACDC_Distri_4_BattChrg_Pwr);

     if((fabs(fVoltReqDiff) > 2.0) || 
        (fabs(fCurrReqDiff) > 2.0) ||
        (fCurrOutDiff > 2.0)) 
	{
		if((fabs(tmNow - pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD))//CATL Need 3S Adjust to 0A,Otherwise BMS will Be OverCurrent Alarm
		{	
			TRACEX("Test Mode:Charge Adjust Cmd:tmNow=%f - tm_fLastSendCmd_2CCU[%d]=%lf\n", \
			tmNow , nBattIdx, pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]);
			if((pSiglBatt->fAllow_MaxChargeCurr < 20.0) || (fabs(fCurrReqDiff) > 10))
			{
				LogOut(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
					"Adj:Idx=[%d]IPack[%.1f]Imax[%.0f]Ilmt[%.0f]Ilast[%.0f]Irect[%.1f]SOC[%d]!", \
					nBattIdx,pSiglBatt->fPackTotalCurr,pSiglBatt->fAllow_MaxChargeCurr,\
					fLimitCurrent,pSTBatt->fLastSendReqCurr_2CCU[nBattIdx],\
					pSTBatt->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSiglBatt->nSOC);
			}
            Send_CtrlCmd(pSTBatt, nBattIdx, STAT_MANUAL_START_CHG, fLimitCurrent);

		}		
	}

    float   fSOC = (float)pSiglBatt->nSOC;
    BOOL    bFull_Chged = pSiglBatt->bIsBatt_ChargeForbidden;
    float   fAllow_chg_Current = pSiglBatt->fAllow_MaxChargeCurr;

    // float stopSOC = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nStopSOCStep7;
    float stopSOC = 50;
    // float minCur = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nCurLmtStep3;
    
    Update_ChargeData(pSTBatt, nBattIdx);
    float Charge_Energy = pSTBatt->blobIfyBMS[nBattIdx].fSingleChgCapacity;
    // TRACE("!!! n: %d, chargeamount: %.1f\n", nBattIdx, Charge_Energy);

    float VolCell_Max = pSTBatt->blobIfyBMS[nBattIdx].usMaxCellVlot_Volt;
    float TH_nChargeEndVol_Max = pSTBatt->bmsTestModeParm.initRange.TH_nChargeEndVol_Max;

    // TRACE("vcell max: %.1f ? setting: %.1f\n", VolCell_Max, TH_nChargeEndVol_Max);
    // if((VolCell_Max>=stopVol) || (fAllow_chg_Current < minCur))
    // Short Time Test
    // if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 5*60)
    // {   
    //     STATE_TEST_RES result;
    //     // TRACE("before charge test\n");
    //     // result = ChargeEndTest(pSTBatt, nBattIdx);
    //     // TotalTest(pSTBatt, nBattIdx);
    //     // // if(result == TEST_FAIL)
    //     // // {
    //     // //     return STATE_FAIL;
    //     // // }
    //     // Push_TestState(pSTBatt, nBattIdx);
    //     // TRACE("send stop cmd!!!\n");
    //     pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_TEST_END;
    //     Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
    //     return TRUE;
    // }

    // Calculate remaining time
    float RemainToCharge_Energy = (pSTBatt->blobIfyBMS[nBattIdx].fTotalEnergy * stopSOC/100)  - pSTBatt->blobIfyBMS[nBattIdx].fRemainEnergy;
    float Remain_Time = 60 * ((RemainToCharge_Energy * 1000) / (pSiglBatt->fPackTotalVolt * pSiglBatt->fPackTotalCurr));
    pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
    Push_TestState(pSTBatt, nBattIdx);

    return FALSE; 
}

static BOOL Discharge_Control_STEP7(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    TRACE("---777 OnFull Dischg Step5, Goto Sleep Step6 nBattIdx=[%d]!\n", nBattIdx);
    pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP7;

    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];
    A_BLOB_SINGLE_STBATT_RUNINFO *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];

    time_t tmNow = time(NULL);

    // if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 12*60*60) //Over 6Hour,Exit
    // {
    //     pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_FULLDISCHG_STEP5;
	// 	return STATE_FAIL; 
    // }

    //Calc_CostEnergy();

    //Predict_RemainTime();

    //Adjust BMS RealTime Output Volt/Current
    float fCalc_Batt_Distri_4_ACDC_Pwr = (float)pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nPwrLmtStep7;
    float fLimitCurrent = 0.0;

    if(pSiglBatt->fPackTotalVolt > 1.0)
	{
		fLimitCurrent = (fCalc_Batt_Distri_4_ACDC_Pwr*1000)/pSiglBatt->fPackTotalVolt;
	}

    float fCurrReqDiff = fabs(fabs(pSTBatt->fLastSendReqCurr_2CCU[nBattIdx]) - MIN(fLimitCurrent, fabs(pSiglBatt->fAllow_MaxDisChargeCurr)));
	float fCurrOutDiff = fabs(fabs(pSiglBatt->fPackTotalCurr) - MIN(fLimitCurrent, fabs(pSiglBatt->fAllow_MaxDisChargeCurr)));

    if((fabs(fCurrReqDiff) > 2.0) || ((fCurrOutDiff > 3.0) && (pSiglBatt->fPackTotalCurr < -3.0)))
		{
			if((fabs(tmNow - pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD))//Step Speed to 1.0 Cycle to Adjust the Power 
			{			
                Send_CtrlCmd(pSTBatt, nBattIdx, STAT_MANUAL_START_DISCHG, fLimitCurrent);
				
				TRACEX("Test Mode : DisCharg Adjust Cmd: nBattIdx=[%d] Adjust ChargePower fCurrReqDiff=%f fCurrOutDiff=%f\
				fLimitCurrent=%.1f fLastSendReqCurr_2CCU=%.1f fPackTotalCurr=%.1f\n",\
				nBattIdx, fCurrReqDiff, fCurrOutDiff ,fLimitCurrent ,\
				pSTBatt->fLastSendReqCurr_2CCU[nBattIdx], pSiglBatt->fPackTotalCurr);
                
                TRACE("Test Mode : DisCharg Adjust Cmd: nBattIdx=[%d] Adjust ChargePower fCurrReqDiff=%f fCurrOutDiff=%f fLimitCurrent=%.1f fLastSendReqCurr_2CCU=%.1f fPackTotalCurr=%.1f\n",\
				nBattIdx, fCurrReqDiff, fCurrOutDiff ,fLimitCurrent ,\
				pSTBatt->fLastSendReqCurr_2CCU[nBattIdx], pSiglBatt->fPackTotalCurr);

			}		
		}	

    float   fSOC = (float)pSiglBatt->nSOC;
    BOOL    bFull_DisChged = pSiglBatt->bIsBatt_DisChargeForbidden;
    float   fAllow_Dischg_Current = pSiglBatt->fAllow_MaxDisChargeCurr;

    // float stopSOC = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nCurLmtStep7;
    float stopSOC = 50;
    // float minCur = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nCurLmtStep5;

    // Update_DischargeData(pSTBatt, nBattIdx);
    float DisCharge_Energy = pSTBatt->blobIfyBMS[nBattIdx].fSingleDischgCapacity;

    float VolCell_Min = pSTBatt->blobIfyBMS[nBattIdx].usMinCellVlot_Volt;
    TRACE("volcell min: %f\n", VolCell_Min);
    float TH_nDischargeEndVol_Min = pSTBatt->bmsTestModeParm.initRange.TH_nDischargeEndVol_Min;

    float   fAllow_chg_Current = pSiglBatt->fAllow_MaxChargeCurr;
    TRACE("fallowcur: %f\n", fAllow_chg_Current);
    // if((VolCell_Min<=stopVol)|| (fAllow_chg_Current < minCur))
    // if(VolCell_Min<=stopVol)
    // Short Time Test
    // if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 5*60)
    // {   
    //     TRACE("enter to send stopcmd!!!!\n");
    //     // STATE_TEST_RES result;
    //     // result = DischargeEndTest(pSTBatt, nBattIdx);
    //     // TotalTest(pSTBatt, nBattIdx);
    //     // if(result == TEST_FAIL)
    //     // {
    //     //     return STATE_FAIL;
    //     // }
    //     Push_TestState(pSTBatt, nBattIdx);
    //     pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_TEST_END;
    //     Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
    //     return TRUE;
    // }

    // Calculate remaining time
    float RemainToDischarge_Energy = pSTBatt->blobIfyBMS[nBattIdx].fRemainEnergy - (pSTBatt->blobIfyBMS[nBattIdx].fTotalEnergy * stopSOC/100);
    float Remain_Time = 60 * ((RemainToDischarge_Energy * 1000) / (pSiglBatt->fPackTotalVolt * pSiglBatt->fPackTotalCurr));
    pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
    Push_TestState(pSTBatt, nBattIdx);

    return FALSE;
} 
*/


/*********************************************************************************
*  
*  FUNCTION NAME : GetSystemFailCode
*  CREATOR       :  
*  CREATED DATE  : 2022-07-20 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static TESTMODE_FAIL_CODE GetSystemFailCode(STBATT_SAMPLER_ROUGH *pSTBatt, int nBattIdx)
{
	A_BLOB_SINGLE_STBATT_RUNINFO  *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];
   
    if(pSTBatt->ccuStatu != CCU_STATE_IDLE)
	{   
        TRACE("ccuStatu:%d\n",pSTBatt->ccuStatu);
        return FAIL_SYSSTATU_FAIL;
    }

    if(pSiglBatt->emBattStatus_Run <= STBATT_RUNSTAT_COMM_FAIL)
    {
        return FAIL_BAT_COMMFAIL;
    }
	
    if(pSiglBatt->emBattStatus_Alm == STBATT_ALMSTAT_FAULT)
    {
        return FAIL_BAT_FAULT;
    }
	

    return FAIL_NO_FAIL;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Push_RackTestValue
*  CREATOR       :  
*  CREATED DATE  : 2022-07-22 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Push_RackTestValue(STBATT_SAMPLER_ROUGH *pSTBatt, int nBattIdx)
{   
	VAR_VALUE varV;

	varV.emValue = pSTBatt->stuTestMode.emRealTimeState[nBattIdx];
	Push_VarValue(SETTYPE_LOGIC_SP, pSTBatt->hSampler, 501 + nBattIdx, 507, varV);

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Enter_BMSTestMode_ShortTime
*  CREATOR       :  
*  CREATED DATE  : 2022-07-20 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Enter_BMSTestMode_ShortTime(STBATT_SAMPLER_ROUGH *pSTBatt, int nBattIdx)
{   
    if(nBattIdx !=0 && nBattIdx != 2)
    {
        return;
    }

    //When Exchge PM Ctrl Mode, Need Initial StateMachine !!!!
    //When Exit RackTest Mode, Need Off All Group PM.
    if (pSTBatt->bmsAutoTestResult[nBattIdx].testState==STATE_NULL)
    {   
        if (nBattIdx==0)
        {
            if(pSTBatt->bmsAutoTestResult[0].testState==STATE_NULL 
            && pSTBatt->bmsAutoTestResult[1].testState==STATE_NULL 
            && pSTBatt->bmsAutoTestResult[2].testState==STATE_NULL 
            && pSTBatt->bmsAutoTestResult[3].testState==STATE_NULL)
            {
                // TRACE("Ready to enter init TestMode.\n");
	            Initial_TestMode_MachineState_Tow(pSTBatt);

	            // TRACE("Ready to enter prepare TestMode.\n");
	            Prepare_Test_StateMachine_Tow(pSTBatt);
            }
        }
        Push_TestState(pSTBatt, nBattIdx);
        return; 
    }
    else if (pSTBatt->bmsAutoTestResult[nBattIdx].testState==STATE_STOP)
    {   
        Push_TestState(pSTBatt, nBattIdx);
        return;
    }

    STATE_MACHINE_STAT_TEST emNewState;
	STATE_MACHINE_STAT_TEST emLastState;
    TESTMODE_FAIL_CODE emFailCode = FAIL_NO_FAIL;

    time_t tmNow = time(NULL);

    //Calc_Power_Allocate(pSTBatt); //Chg Power Lmt + Dsichg Power Lmt

    // TRACE("-nBattIdx=[%d]-Start Initial BMS TestMode StateMachine-\n", nBattIdx);

    emLastState = pSTBatt->stuTestMode.pCurrState[nBattIdx]->emState;
	emNewState = pSTBatt->stuTestMode.pCurrState[nBattIdx]->pfnAction(pSTBatt, nBattIdx);
	TRACE("-nBattIdx=[%d]-Now on state[%d], new state[%d]-\n", nBattIdx, emLastState, emNewState);

	pSTBatt->stuTestMode.pLastState[nBattIdx] = &pSTBatt->stuTestMode.stateMachineLoop[emLastState];
	
    pSTBatt->stuTestMode.emRealTimeState[nBattIdx] = emNewState;


	if(emNewState == STATE_FAIL)
	{
	    pSTBatt->stuTestMode.pfnOnFail[nBattIdx](pSTBatt,
	        pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailFrom,
	        pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode,
	        nBattIdx);
	    
	    return;
	}
	else
	{
		emFailCode = GetSystemFailCode(pSTBatt, nBattIdx);
		if(emFailCode != FAIL_NO_FAIL)
		{
			pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = emFailCode;
			pSTBatt->stuTestMode.pfnOnFail[nBattIdx](pSTBatt, pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailFrom, emFailCode, nBattIdx);
			
			pSTBatt->stuTestMode.emTrigState[nBattIdx] = STATE_IDLE; //Incase Sometimes Triiged when Testing

			TRACE("Err: nBattIdx=[%d] emFailCode=[%d] Has Fault!!!\n",  nBattIdx, emFailCode);
			return;
		}
	}

	if(emNewState != emLastState) //State Machine Changed, Need Log The Info
	{
		if(emNewState == STATE_SELF_CHECK)
		{
		    LogOut(MAIN_MODULE_SAMPLER, "BAT TEST", LOG_TYPE_INFO,\
				"Idx[%d]NewState[%d]LastState[%d] Start Test!", nBattIdx,emNewState,emLastState);
 		}
		else if(emNewState == STATE_TEST_FINISHED)
		{
            pSTBatt->stuTestMode.emTrigState[nBattIdx] = STATE_IDLE; //Incase Sometimes Triiged when Testing
		    LogOut(MAIN_MODULE_SAMPLER, "BAT TEST", LOG_TYPE_INFO,\
				"Idx[%d]NewState[%d]LastState[%d] End Test!", nBattIdx,emNewState,emLastState);
		}

		pSTBatt->stuTestMode.pCurrState[nBattIdx] = &pSTBatt->stuTestMode.stateMachineLoop[emNewState];
		pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter = time(NULL);
		TRACE("STATE Changed !!!! n: %d, emNewState=[%d] tm_Enter=[%ld]\n",\
		nBattIdx, emNewState, pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter);
	}

    Push_RackTestValue(pSTBatt, nBattIdx);
	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Init_TestParam
*  CREATOR       :  
*  CREATED DATE  : 2022-07-20 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Init_TestParam(STBATT_SAMPLER_ROUGH *pSTBatt, int nBattIdx)
{
    //pSTBatt->stuTestMode.emTrigState[nBattIdx] = STATE_IDLE;

    pSTBatt->stuTestMode.emFailCode[nBattIdx] = FAIL_NO_FAIL;

    pSTBatt->stuTestMode.nFailTimes[nBattIdx] = 0;


    //GetTest_CfgParams(pBMSRough_Data);

    TRACE("---000 Init Test Param\n");

    return;
}

// /*********************************************************************************
// *  
// *  FUNCTION NAME : OnTest_Idle
// *  CREATOR       :  
// *  CREATED DATE  : 2022-07-20 11:13:53
// *  DESCRIPTION   :  
// *  
// *  CHANGE HISTORY: 
// *  
// ***********************************************************************************/
// static STATE_MACHINE_STAT_TEST OnTest_Idle(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
// {
//     TRACE("---111 On_Idle, Goto Self_Check nBattIdx=[%d]!\n", nBattIdx);
//     pSTBatt->bmsAutoTestResult[nBattIdx].testState = STATE_TESTSTART;
//     pSTBatt->bmsAutoTestResult[nBattIdx + Grp_gap].testState = STATE_TESTSTART;
//     Init_ProcessRecord(pSTBatt, nBattIdx);
//     Init_ProcessRecord(pSTBatt, nBattIdx + Grp_gap);

//     STATE_MACHINE_STAT_TEST emTrigState = pSTBatt->stuTestMode.emTrigState[nBattIdx];
 
//     if(emTrigState != STATE_IDLE)
//     {
//         Init_TestParam(pSTBatt, nBattIdx); //Init Param/Clear Fail/Get Parame Cfg 
//     }

//     Push_TestState(pSTBatt, nBattIdx);
//     return STATE_SELF_CHECK;
// }

// /*********************************************************************************
// *  
// *  FUNCTION NAME : OnTest_Self_Check
// *  CREATOR       :  
// *  CREATED DATE  : 2022-07-20 11:13:53
// *  DESCRIPTION   :  
// *  
// *  CHANGE HISTORY: 
// *  
// ***********************************************************************************/
// static STATE_MACHINE_STAT_TEST OnTest_Self_Check(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
// {   
//     time_t tmNow = time(NULL);
//     // A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];

//     TRACE("---222 nBattIdx=[%d] OnSelf_Check,tmNow=[%ld] tm_Enter=[%ld] !\n",\
//         nBattIdx, tmNow, pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter);

//     pSTBatt->bmsAutoTestResult[nBattIdx].testState = STATE_TESTSTART;
//     Push_TestState(pSTBatt, nBattIdx);
//     pSTBatt->bmsAutoTestResult[nBattIdx + Grp_gap].testState = STATE_TESTSTART;
//     Push_TestState(pSTBatt, nBattIdx + Grp_gap);

//     int nOffset = ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter);
//     if(nOffset > 10)
//     {   
//         TRACE("nOffset: %d\n", nOffset);//调试用
    
//         TRACE("---222 Goto Fail !!! TimeOut! tmNow=[%ld] tm_Enter=[%ld]\n", tmNow , pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter);

//         pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_SELFCHECK;
// 		return STATE_FAIL; 
//     }
    
//     // float fOutRelay_Volt = pBranchGrp->fACDCOutRelayVolt;
//     // float fPM_RealtimeVolt = pBranchGrp->fACDC_RealTime_Volt;

//     STATE_TEST_RES result_0;
//     STATE_TEST_RES result_1;
//     // STATE_TEST_RES result_3;
//     // STATE_TEST_RES result_4;

//     result_0 = SelfCheckTest_ShortTime(pSTBatt, nBattIdx);
//     result_1 = SelfCheckTest_ShortTime(pSTBatt, nBattIdx + Grp_gap);
//     // result_3 = SelfCheckTest(pSTBatt, nBattIdx + Grp_gap);
//     // result_4 = SelfCheckTest(pSTBatt, nBattIdx+3);
//     TRACE("res0: %d, res1: %d\n", result_0, result_1);

//     // if(result_0 == TEST_FAIL)
//     // {   
//     //     pSTBatt->bmsAutoTestResult[nBattIdx].testState = STATE_SELFCHECK_END;
//     //     Push_TestState(pSTBatt, nBattIdx);
//     //     return STATE_FAIL;
//     // }
//     // if(result_1 == TEST_FAIL)
//     // {   
//     //     pSTBatt->bmsAutoTestResult[nBattIdx + Grp_gap].testState = STATE_SELFCHECK_END;
//     //     Push_TestState(pSTBatt, nBattIdx + Grp_gap);
//     //     return STATE_FAIL;
//     // }
//     // if(result_3 == TEST_FAIL)
//     // {   
//     //     pSTBatt->bmsAutoTestResult[nBattIdx + Grp_gap].testState = STATE_SELFCHECK_END;
//     //     Push_TestState(pSTBatt, nBattIdx + Grp_gap);
//     //     return STATE_FAIL;
//     // }
//     // if(result_4 == TEST_FAIL)
//     // {   
//     //     pSTBatt->bmsAutoTestResult[nBattIdx+3].testState = STATE_SELFCHECK_END;
//     //     Push_TestState(pSTBatt, nBattIdx+3);
//     //     return STATE_FAIL;
//     // }

//     // if(fOutRelay_Volt < MIN_PM_OUTPUT_VOLT)
//     // {   
//     //     TRACE("fOutRelay_Volt too small: %.1f\n");
//     //     pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_RELAY_VOLT_ABNORMAL;
		
//     //     return STATE_SELF_CHECK; 
// 	// 	// return STATE_FAIL; 
//     // }

//     // if(fPM_RealtimeVolt > MIN_PM_OUTPUT_VOLT)
//     // {   
//     //     TRACE("fPM_RealtimeVolt too large: %.1f\n");
//     //     //Need Send Stop PM Cmd
//     //     Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);

//     //     return STATE_SELF_CHECK;
//     // }

//     pSTBatt->bmsAutoTestResult[nBattIdx].testState = STATE_SELFCHECK_END;
//     Push_TestState(pSTBatt, nBattIdx);
//     pSTBatt->bmsAutoTestResult[nBattIdx + Grp_gap].testState = STATE_SELFCHECK_END;
//     Push_TestState(pSTBatt, nBattIdx + Grp_gap);
//     return STATE_FULLDSICHARGE_STEP1;
// }

// /*********************************************************************************
// *  
// *  FUNCTION NAME : OnTest_FullDischg_Step1
// *  CREATOR       :  
// *  CREATED DATE  : 2022-07-20 11:13:53
// *  DESCRIPTION   :  
// *  
// *  CHANGE HISTORY: 
// *  
// ***********************************************************************************/
// static STATE_MACHINE_STAT_TEST OnTest_FullDischg_Step1(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
// {
//     TRACE("---333 Dischg&Chg Step1, Goto Sleep Step2---\n");

//     BOOL batt_0, batt_1;
//     if(pSTBatt->bmsAutoTestResult[nBattIdx].testState !=  STATE_STEP2)
//     {
//         batt_0 = Discharge_Control_STEP1(pSTBatt, nBattIdx); // Batt 0 to discharge.    
//     }
//     else
//     {
//         batt_0 = TRUE;
//     }
    
//     if(pSTBatt->bmsAutoTestResult[nBattIdx + Grp_gap].testState !=  STATE_STEP2)
//     {
//         batt_1 = Charge_Control_STEP1(pSTBatt, nBattIdx + Grp_gap); // Batt 1 to charge.
//     }
//     else
//     {
//         batt_1 = TRUE;
//     }

// 	double tmNow = GetCurrentTime();
//     if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 5*60)
//     {   
//         pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP2;
//         Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
//         Send_StopCmd(pSTBatt, nBattIdx+Grp_gap, STAT_MANUAL_STOP);
//         batt_0 = TRUE;
//         batt_1 = TRUE;
//     }

//     TRACE("batt0: %d, batt1: %d\n", batt_0, batt_1);

//     // Discharge_Control(pSTBatt, nBattIdx + Grp_gap);
//     // Charge_Control(pSTBatt, nBattIdx+3);

//     if (batt_0 == TRUE && batt_1 == TRUE)
//     {
//         return STATE_SLEEP_STEP2;
//     }

//     return STATE_FULLDSICHARGE_STEP1;
// }

// /*********************************************************************************
// *  
// *  FUNCTION NAME : OnTest_Sleep_Step2
// *  CREATOR       :  
// *  CREATED DATE  : 2022-07-20 11:13:53
// *  DESCRIPTION   :  
// *  
// *  CHANGE HISTORY: 
// *  
// ***********************************************************************************/
// static STATE_MACHINE_STAT_TEST OnTest_Sleep_Step2(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
// {
//     TRACE("---444 On Sleep Step2, Goto Full Chging Step3\n");
//     pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP2;
//     pSTBatt->bmsAutoTestResult[nBattIdx + Grp_gap].testState =  STATE_STEP2;

//     time_t tmNow = time(NULL);
    
//     int sleepTime = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nWaitTimeStep2; // Count in minute!
//     float timeGap = (ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter)/60);
//     // TRACE("tmNow: %lld\n", (long long)tmNow);
//     // TRACE("tmEnter: %lld\n", (long long)pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter);
//     // TRACE("time gap: %f min\n", timeGap);
    
//     if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > sleepTime*60) 
//     {   
//         TRACE("-----!!!!!!!!!!!-----\n");
//         TRACE("sleep ends!!!\n");
//  		return STATE_FULLCHARGING_STEP3; 
//     }

//     float Remain_Time = sleepTime - timeGap;
//     pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
//     pSTBatt->bmsAutoTestResult[nBattIdx + Grp_gap].timeRemain =  fabs(Remain_Time);
//     Push_TestState(pSTBatt, nBattIdx);
//     Push_TestState(pSTBatt, nBattIdx + Grp_gap);

//     return STATE_SLEEP_STEP2;
// }

// /*********************************************************************************
// *  
// *  FUNCTION NAME : OnTest_FullChg_Step3
// *  CREATOR       :  
// *  CREATED DATE  : 2022-07-20 11:13:53
// *  DESCRIPTION   :  
// *  
// *  CHANGE HISTORY: 
// *  
// ***********************************************************************************/
// static STATE_MACHINE_STAT_TEST OnTest_FullChg_Step3(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
// {
//     TRACE("---555 OnFull Full Chg&Dischg Step3, Goto Sleep Step4\n");

//     BOOL batt_0, batt_1;
//     if(pSTBatt->bmsAutoTestResult[nBattIdx].testState !=  STATE_STEP4)
//     {
//         batt_0 = Charge_Control_STEP3(pSTBatt, nBattIdx); // Batt 0 to charge.    
//     }
//     else
//     {
//         batt_0 = TRUE;
//     }
    
//     if(pSTBatt->bmsAutoTestResult[nBattIdx + Grp_gap].testState !=  STATE_STEP4)
//     {
//         batt_1 = Discharge_Control_STEP3(pSTBatt, nBattIdx + Grp_gap); // Batt 1 to discharge.
//     }
//     else
//     {
//         batt_1 = TRUE;
//     }

//     double tmNow = GetCurrentTime();
//     if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 5*60)
//     {   
//         // ChargeEndTest_ShortTime(pSTBatt, nBattIdx);
//         pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP4;
//         Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
//         Send_StopCmd(pSTBatt, nBattIdx+Grp_gap, STAT_MANUAL_STOP);
//         batt_0 = TRUE;
//         batt_1 = TRUE;
//     }

//     TRACE("batt0: %d, batt1: %d\n", batt_0, batt_1);

//     // Discharge_Control(pSTBatt, nBattIdx + Grp_gap);
//     // Charge_Control(pSTBatt, nBattIdx+3);

//     if (batt_0 == TRUE && batt_1 == TRUE)
//     {
//         return STATE_SLEEP_STEP4;
//     }

//     return STATE_FULLCHARGING_STEP3;
// }
 
// /*********************************************************************************
// *  
// *  FUNCTION NAME :  
// *  CREATOR       :  
// *  CREATED DATE  : 2022-07-20 11:13:53
// *  DESCRIPTION   :  
// *  
// *  CHANGE HISTORY: 
// *  
// ***********************************************************************************/
// static STATE_MACHINE_STAT_TEST OnTest_Sleep_Step4(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
// {
//     TRACE("---666 OnSleep Step2, Goto Full Dischg Step5\n");
//     pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP4;
//     pSTBatt->bmsAutoTestResult[nBattIdx + Grp_gap].testState =  STATE_STEP4;

//     time_t tmNow = time(NULL);

//     int sleepTime = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nWaitTimeStep4; // Count in minute!
//     float timeGap = (ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter)/60);

//     if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > sleepTime*60) //Sleep 5 Min
//     {
//  		return STATE_FULLDSICHARGE_STEP5;
//     }

//     float Remain_Time = sleepTime - timeGap;
//     pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
//     pSTBatt->bmsAutoTestResult[nBattIdx + Grp_gap].timeRemain =  fabs(Remain_Time);
//     Push_TestState(pSTBatt, nBattIdx);
//     Push_TestState(pSTBatt, nBattIdx + Grp_gap);

//     return STATE_SLEEP_STEP4;
// }

// /*********************************************************************************
// *  
// *  FUNCTION NAME :  
// *  CREATOR       :  
// *  CREATED DATE  : 2022-07-20 11:13:53
// *  DESCRIPTION   :  
// *  
// *  CHANGE HISTORY: 
// *  
// ***********************************************************************************/
// static STATE_MACHINE_STAT_TEST OnTest_FullDischg_Step5(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
// {
//     TRACE("---777 OnFull Full Chg&Dischg Step5, Goto Sleep Step6\n");

//     BOOL batt_0, batt_1;
//     if(pSTBatt->bmsAutoTestResult[nBattIdx].testState !=  STATE_STEP6)
//     {
//         batt_0 = Discharge_Control_STEP5(pSTBatt, nBattIdx); // Batt 0 to discharge.    
//     }
//     else
//     {
//         batt_0 = TRUE;
//     }
    
//     if(pSTBatt->bmsAutoTestResult[nBattIdx + Grp_gap].testState !=  STATE_STEP6)
//     {
//         batt_1 = Charge_Control_STEP5(pSTBatt, nBattIdx + Grp_gap); // Batt 1 to charge.
//     }
//     else
//     {
//         batt_1 = TRUE;
//     }

//     double tmNow = GetCurrentTime();
//     if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 5*60)
//     {   
//         ChargeEndTest_ShortTime(pSTBatt, nBattIdx);
//         DischargeEndTest_ShortTime(pSTBatt, nBattIdx+Grp_gap);
//         TotalTest_ShortTime(pSTBatt, nBattIdx);
//         pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP6;
//         Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
//         Send_StopCmd(pSTBatt, nBattIdx+Grp_gap, STAT_MANUAL_STOP);
//         batt_0 = TRUE;
//         batt_1 = TRUE;
//     }

//     TRACE("batt0: %d, batt1: %d\n", batt_0, batt_1);

//     // Discharge_Control(pSTBatt, nBattIdx + Grp_gap);
//     // Charge_Control(pSTBatt, nBattIdx+3);

//     if (batt_0 == TRUE && batt_1 == TRUE)
//     {
//         return STATE_SLEEP_STEP6;
//     }

//     return STATE_FULLDSICHARGE_STEP5;

// }

// /*********************************************************************************
// *  
// *  FUNCTION NAME :  
// *  CREATOR       :  
// *  CREATED DATE  : 2022-07-20 11:13:53
// *  DESCRIPTION   :  
// *  
// *  CHANGE HISTORY: 
// *  
// ***********************************************************************************/
// static STATE_MACHINE_STAT_TEST OnTest_Sleep_Step6(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
// {
//     TRACE("---888 OnSleep Step3, Goto Chg Half Step7 nBattIdx=[%d]!\n", nBattIdx);
//     pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP6;
//     pSTBatt->bmsAutoTestResult[nBattIdx + Grp_gap].testState =  STATE_STEP6;

//     // Push_TestState(pSTBatt, nBattIdx);

//     time_t tmNow = time(NULL);

//     int sleepTime = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nWaitTimeStep6; // Count in minute!
//     float timeGap = (ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter)/60);

//     if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > sleepTime*60) //Sleep 5 Min
//     {
//  		return STATE_CHARGE_HALF_STEP7;
//     }

//     float Remain_Time = sleepTime - timeGap;
//     pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
//     pSTBatt->bmsAutoTestResult[nBattIdx + Grp_gap].timeRemain =  fabs(Remain_Time);
//     Push_TestState(pSTBatt, nBattIdx);
//     Push_TestState(pSTBatt, nBattIdx + Grp_gap);

//     return STATE_SLEEP_STEP6;
// }

// /*********************************************************************************
// *  
// *  FUNCTION NAME :  
// *  CREATOR       :  
// *  CREATED DATE  : 2022-07-20 11:13:53
// *  DESCRIPTION   :  
// *  
// *  CHANGE HISTORY: 
// *  
// ***********************************************************************************/
// static STATE_MACHINE_STAT_TEST OnTest_Chg_Half_Step7(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
// {
//     TRACE("---999 OnChg Half Step7, Goto Test Finished nBattIdx=[%d]!\n", nBattIdx);

//     BOOL batt_0, batt_1;
//     if(pSTBatt->bmsAutoTestResult[nBattIdx].testState != STATE_TEST_END)
//     {
//         batt_0 = Charge_Control_STEP7(pSTBatt, nBattIdx); // Batt 0 to charge.    
//     }
//     else
//     {
//         batt_0 = TRUE;
//     }
    
//     if(pSTBatt->bmsAutoTestResult[nBattIdx + Grp_gap].testState !=  STATE_TEST_END)
//     {
//         batt_1 = Discharge_Control_STEP7(pSTBatt, nBattIdx + Grp_gap); // Batt 1 to discharge.
//     }
//     else
//     {
//         batt_1 = TRUE;
//     }

//     double tmNow = GetCurrentTime();
//     if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 5*60)
//     {   
//         ChargeEndTest_ShortTime(pSTBatt, nBattIdx);
//         DischargeEndTest_ShortTime(pSTBatt, nBattIdx+Grp_gap);
//         TotalTest_ShortTime(pSTBatt, nBattIdx);
//         // TotalTest_ShortTime(pSTBatt, nBattIdx+Grp_gap);
//         pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_TEST_END;
//         pSTBatt->bmsAutoTestResult[nBattIdx+Grp_gap].testState =  STATE_TEST_END;
//         Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
//         Send_StopCmd(pSTBatt, nBattIdx+Grp_gap, STAT_MANUAL_STOP);
//         batt_0 = TRUE;
//         batt_1 = TRUE;
//     }

//     TRACE("batt0: %d, batt1: %d\n", batt_0, batt_1);

//     // Discharge_Control(pSTBatt, nBattIdx + Grp_gap);
//     // Charge_Control(pSTBatt, nBattIdx+3);

//     if (batt_0 == TRUE && batt_1 == TRUE)
//     {
//         // return STATE_TEST_FINISHED;
//         return STATE_TEST_FINISHED; // Debug not to quit when finish.
//     }

//     return STATE_CHARGE_HALF_STEP7;
// }

// /*********************************************************************************
// *  
// *  FUNCTION NAME :  
// *  CREATOR       :  
// *  CREATED DATE  : 2022-07-20 11:13:53
// *  DESCRIPTION   :  
// *  
// *  CHANGE HISTORY: 
// *  
// ***********************************************************************************/
// static STATE_MACHINE_STAT_TEST OnTest_Finished(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
// {
//     TRACE("---aaa OnTest Finished, Goto Idle. nBattIdx=[%d]!\n", nBattIdx);
//     // pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_TEST_END;

//     A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];
//     A_BLOB_SINGLE_STBATT_RUNINFO *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];

//     time_t tmNow = time(NULL);
//     if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 1*10) //Over 10S
//     {   
//         pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_NULL;
//         pSTBatt->bmsAutoTestResult[nBattIdx+Grp_gap].testState =  STATE_NULL;
//  		return STATE_IDLE;
//     }

//     //Judge PM Volt /Current, If Abnormal ,Need Send Stop Cmd
//     float fPM_Volt = pBranchGrp->fACDC_RealTime_Volt;
//     if(fPM_Volt > MIN_PM_OUTPUT_VOLT)
//     {
//         Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
//         Send_StopCmd(pSTBatt, nBattIdx+Grp_gap, STAT_MANUAL_STOP);
//         pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_NULL;
//         pSTBatt->bmsAutoTestResult[nBattIdx+Grp_gap].testState =  STATE_NULL;
//         return STATE_TEST_FINISHED;
//     }

//     pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_NULL;
//     pSTBatt->bmsAutoTestResult[nBattIdx+Grp_gap].testState =  STATE_NULL;
//     Push_TestState(pSTBatt, nBattIdx);
//     Push_TestState(pSTBatt, nBattIdx + Grp_gap);

//     return STATE_IDLE;
// }

// static STATE_MACHINE_STAT_TEST OnTest_Paused(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
// {
//     TRACE("---bbb On Paused [%d]---!\n", nBattIdx);
    
//     A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];

//     time_t tmNow = time(NULL);
//     if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 1*60) //Wait 1 Min
//     {
//  		return STATE_IDLE;
//     }

//     //Judge PM Volt /Current, If Abnormal ,Need Send Stop Cmd
//     float fPM_Volt = pBranchGrp->fACDC_RealTime_Volt;
//     if(fPM_Volt > MIN_PM_OUTPUT_VOLT)
//     {
//         Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
//         return STATE_ON_PAUSED;
//     }

//     return STATE_IDLE;
// }



/*********************************************************************************
*  
*  FUNCTION NAME : OnTest_Idle
*  CREATOR       :  
*  CREATED DATE  : 2022-07-20 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static STATE_MACHINE_STAT_TEST OnTest_Idle(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    TRACE("---111 On_Idle, Goto Self_Check nBattIdx=[%d]!\n", nBattIdx);
    pSTBatt->bmsAutoTestResult[nBattIdx].testState = STATE_TESTSTART;
    Init_ProcessRecord(pSTBatt, nBattIdx);

    STATE_MACHINE_STAT_TEST emTrigState = pSTBatt->stuTestMode.emTrigState[nBattIdx];
 
    if(emTrigState != STATE_IDLE)
    {
        Init_TestParam(pSTBatt, nBattIdx); //Init Param/Clear Fail/Get Parame Cfg 
    }

    TRACE("emTrigState:%d\n",emTrigState);

    Push_TestState(pSTBatt, nBattIdx);
    return STATE_SELF_CHECK;
}

/*********************************************************************************
*  
*  FUNCTION NAME : OnTest_Self_Check
*  CREATOR       :  
*  CREATED DATE  : 2022-07-20 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static STATE_MACHINE_STAT_TEST OnTest_Self_Check(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{   
    time_t tmNow = time(NULL);
    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];

    TRACE("---222 nBattIdx=[%d] OnSelf_Check,tmNow=[%ld] tm_Enter=[%ld] !\n",\
        nBattIdx, tmNow, pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter);

    int nOffset = ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter);
    if(nOffset > 10)
    {   
        // TRACE("nOffset: %d\n", nOffset);//调试用
    
        TRACE("---222 Goto Fail !!! TimeOut! tmNow=[%ld] tm_Enter=[%ld]\n", tmNow , pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter);

        pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_SELFCHECK;
		return STATE_FAIL; 
    }
    
    float fOutRelay_Volt = pBranchGrp->fACDCOutRelayVolt;
    float fPM_RealtimeVolt = pBranchGrp->fACDC_RealTime_Volt;

    STATE_TEST_RES result;
    result = SelfCheckTest(pSTBatt, nBattIdx);
    if(result == TEST_FAIL)
    {   
        pSTBatt->bmsAutoTestResult[nBattIdx].testState = STATE_SELFCHECK_END;
        Push_TestState(pSTBatt, nBattIdx);
        return STATE_FAIL;
    }

    if(fOutRelay_Volt < MIN_PM_OUTPUT_VOLT)
    {   
        // TRACE("fOutRelay_Volt too small\n");
        pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_RELAY_VOLT_ABNORMAL;
        return STATE_FAIL;
    }
    if(fPM_RealtimeVolt > MIN_PM_OUTPUT_VOLT)
    {   
        // TRACE("fPM_RealtimeVolt too large\n");
        //Need Send Stop PM Cmd
        Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);

        return STATE_SELF_CHECK;
    }

    pSTBatt->bmsAutoTestResult[nBattIdx].testState = STATE_SELFCHECK_END;
    Push_TestState(pSTBatt, nBattIdx);
    return STATE_FULLDSICHARGE_STEP1;
}

/*********************************************************************************
*  
*  FUNCTION NAME : OnTest_FullDischg_Step1
*  CREATOR       :  
*  CREATED DATE  : 2022-07-20 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static STATE_MACHINE_STAT_TEST OnTest_FullDischg_Step1(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    TRACE("---333 OnFull Dischg Step1, Goto Sleep Step2 nBattIdx=[%d]!\n", nBattIdx);
    pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP1;

    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];
    A_BLOB_SINGLE_STBATT_RUNINFO *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];

	double tmNow = GetCurrentTime();
    if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 12*60*60)
    {
        pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_FULLDISCHG_STEP1;
		return STATE_FAIL; 
    }

    //Calc_CostEnergy();

    //Predict_RemainTime();

    //Adjust BMS RealTime Output Volt/Current
    float fCalc_Batt_Distri_4_ACDC_Pwr = (float)pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nPwrLmtStep1;
    TRACE("set power limit: %f\n", fCalc_Batt_Distri_4_ACDC_Pwr);
    float fLimitCurrent = 0.0;

    if(pSiglBatt->fPackTotalVolt > 1.0)
	{
		fLimitCurrent = (fCalc_Batt_Distri_4_ACDC_Pwr*1000)/pSiglBatt->fPackTotalVolt;
    }

    float fCurrReqDiff = fabs(fabs(pSTBatt->fLastSendReqCurr_2CCU[nBattIdx]) - MIN(fLimitCurrent, fabs(pSiglBatt->fAllow_MaxDisChargeCurr)));
	float fCurrOutDiff = fabs(fabs(pSiglBatt->fPackTotalCurr) - MIN(fLimitCurrent, fabs(pSiglBatt->fAllow_MaxDisChargeCurr)));

    if((fabs(fCurrReqDiff) > 2.0) || ((fCurrOutDiff > 3.0) && (pSiglBatt->fPackTotalCurr < -3.0)))
		{
			if((fabs(tmNow - pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD))//Step Speed to 1.0 Cycle to Adjust the Power 
			{			
                Send_CtrlCmd(pSTBatt, nBattIdx, STAT_MANUAL_START_DISCHG, fLimitCurrent);
				
				TRACEX("Test Mode : DisCharg Adjust Cmd: nBattIdx=[%d] Adjust ChargePower fCurrReqDiff=%f fCurrOutDiff=%f\
				fLimitCurrent=%.1f fLastSendReqCurr_2CCU=%.1f fPackTotalCurr=%.1f\n",\
				nBattIdx, fCurrReqDiff, fCurrOutDiff ,fLimitCurrent ,\
				pSTBatt->fLastSendReqCurr_2CCU[nBattIdx], pSiglBatt->fPackTotalCurr);
			}		
		}	

    float   fSOC = (float)pSiglBatt->nSOC;
    BOOL    bFull_DisChged = pSiglBatt->bIsBatt_DisChargeForbidden;
    float   fAllow_Dischg_Current = pSiglBatt->fAllow_MaxDisChargeCurr;

    float stopSOC = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nStopSOCStep1;
    // float minCur = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nCurLmtStep1;

    float VolCell_Min = pSTBatt->blobIfyBMS[nBattIdx].usMinCellVlot_Volt;
    float TH_nDischargeEndVol_Min = pSTBatt->bmsTestModeParm.initRange.TH_nDischargeEndVol_Min;

    // TRACE("vcell min: %.1f ? setting: %.1f\n", VolCell_Min, stopVol);

    // if((VolCell_Min < stopVol) || (fAllow_Dischg_Current < minCur))
    if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 5*60)
    {   
        Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
        return STATE_SLEEP_STEP2;
    }

    // Calculate remaining time
    float Remain_Energy = pSTBatt->blobIfyBMS[nBattIdx].fRemainEnergy;
    float Remain_Time = 60 * ((Remain_Energy * 1000) / (pSiglBatt->fPackTotalVolt * pSiglBatt->fPackTotalCurr));
    pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
    Push_TestState(pSTBatt, nBattIdx);

    return STATE_FULLDSICHARGE_STEP1;
}

/*********************************************************************************
*  
*  FUNCTION NAME : OnTest_Sleep_Step2
*  CREATOR       :  
*  CREATED DATE  : 2022-07-20 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static STATE_MACHINE_STAT_TEST OnTest_Sleep_Step2(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    TRACE("---444 On Sleep Step1, Goto Full Chging Step3 nBattIdx=[%d]!\n", nBattIdx);
    pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP2;

    time_t tmNow = time(NULL);
    
    int sleepTime = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nWaitTimeStep2; // Count in minute!
    float timeGap = (ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter)/60);
    // TRACE("tmNow: %lld\n", (long long)tmNow);
    // TRACE("tmEnter: %lld\n", (long long)pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter);
    // TRACE("time gap: %f min\n", timeGap);
    
    if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > sleepTime*60) 
    {   
 		return STATE_FULLCHARGING_STEP3; 
    }

    float Remain_Time = sleepTime - timeGap;
    pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
    Push_TestState(pSTBatt, nBattIdx);

    return STATE_SLEEP_STEP2;
}

/*********************************************************************************
*  
*  FUNCTION NAME : OnTest_FullChg_Step3
*  CREATOR       :  
*  CREATED DATE  : 2022-07-20 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static STATE_MACHINE_STAT_TEST OnTest_FullChg_Step3(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    TRACE("---555 OnFull Full Chging Step3, Goto Sleep Step4 nBattIdx=[%d]!\n", nBattIdx);
    pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP3;

    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];
    A_BLOB_SINGLE_STBATT_RUNINFO *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];

	double tmNow = GetCurrentTime();

    if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 12*60*60)  
    {
 	    pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_FULLCHG_STEP3;
		return STATE_FAIL; 
    }

    //Adjust BMS Output Volt And Current 
    
    //Calc_CostEnergy(pSTBatt, nBattIdx);

    //Predict_RemainTime(pSTBatt, nBattIdx);

    //Adjust BMS RealTime Output Volt/Current
    float fLimitCurrent = 0.0;
    float fACDC_Distri_4_BattChrg_Pwr = (float)pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nPwrLmtStep3;
   
    if(pSiglBatt->fPackTotalVolt > 50.0)
	{
		fLimitCurrent = (fACDC_Distri_4_BattChrg_Pwr*1000)/pSiglBatt->fPackTotalVolt;
	}

    float fVoltReqDiff = fabs(pSTBatt->fLastSendReqVolt_2CCU[nBattIdx] - MIN(pSiglBatt->fAllow_LmtChargeVolt, 1000));
	float fCurrReqDiff = fabs(pSTBatt->fLastSendReqCurr_2CCU[nBattIdx] - MIN(pSiglBatt->fAllow_MaxChargeCurr, fLimitCurrent));
 	float fCurrOutDiff = pSiglBatt->fPackTotalCurr - pSiglBatt->fAllow_MaxChargeCurr;
	//2021-11-17,MPPt Output Current will Cause PackCurr Over MAx_AllowCurr
	TRACEX("tmNow=%f nBattIdx=[%d] fVoltReqDiff=[%.1f], fCurrReqDiff=[%.1f]fCurrOutDiff=%f,\
		fPackTotalCurr=%f,fLastSendReqCurr_2CCU=[%.1f] fAllow_MaxChargeCurr=[%.1f] fACDC_Distri_4_BattChrg_Pwr=[%.1f]\n",\
        tmNow, nBattIdx, fVoltReqDiff, fCurrReqDiff, fCurrOutDiff, pSiglBatt->fPackTotalCurr,\
        pSTBatt->fLastSendReqCurr_2CCU[nBattIdx], pSiglBatt->fAllow_MaxChargeCurr,\
        fACDC_Distri_4_BattChrg_Pwr);

     if((fabs(fVoltReqDiff) > 2.0) || 
        (fabs(fCurrReqDiff) > 2.0) ||
        (fCurrOutDiff > 2.0)) 
	{
		if((fabs(tmNow - pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD))//CATL Need 3S Adjust to 0A,Otherwise BMS will Be OverCurrent Alarm
		{	
			TRACEX("Test Mode:Charge Adjust Cmd:tmNow=%f - tm_fLastSendCmd_2CCU[%d]=%lf\n", \
			tmNow , nBattIdx, pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]);
			if((pSiglBatt->fAllow_MaxChargeCurr < 20.0) || (fabs(fCurrReqDiff) > 10))
			{
				LogOut(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
					"Adj:Idx=[%d]IPack[%.1f]Imax[%.0f]Ilmt[%.0f]Ilast[%.0f]Irect[%.1f]SOC[%d]!", \
					nBattIdx,pSiglBatt->fPackTotalCurr,pSiglBatt->fAllow_MaxChargeCurr,\
					fLimitCurrent,pSTBatt->fLastSendReqCurr_2CCU[nBattIdx],\
					pSTBatt->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSiglBatt->nSOC);
			}
            Send_CtrlCmd(pSTBatt, nBattIdx, STAT_MANUAL_START_CHG, fLimitCurrent);

		}		
	}

    float   fSOC = (float)pSiglBatt->nSOC;
    BOOL    bFull_Chged = pSiglBatt->bIsBatt_ChargeForbidden;
    float   fAllow_chg_Current = pSiglBatt->fAllow_MaxChargeCurr;

    float stopSOC = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nStopSOCStep3;
    // float minCur = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nCurLmtStep3;
    
    if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 60) // sample data after 20s
    {
        Update_ChargeData(pSTBatt, nBattIdx);
    }
    float Charge_Energy = pSTBatt->blobIfyBMS[nBattIdx].fSingleChgCapacity;
    // TRACE("!!! n: %d, chargeamount: %.1f\n", nBattIdx, Charge_Energy);

    float VolCell_Max = pSTBatt->blobIfyBMS[nBattIdx].usMaxCellVlot_Volt;
    float TH_nChargeEndVol_Max = pSTBatt->bmsTestModeParm.initRange.TH_nChargeEndVol_Max;

    // TRACE("vcell max: %.1f ? setting: %.1f\n", VolCell_Max, TH_nChargeEndVol_Max);
    // if((VolCell_Max>=stopVol) || (fAllow_chg_Current < minCur))

    if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 5*60)
    {   
        STATE_TEST_RES result;
        TRACE("before charge test\n");
        result = ChargeEndTest_ShortTime(pSTBatt, nBattIdx);

        Push_TestState(pSTBatt, nBattIdx);

        Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
        return STATE_SLEEP_STEP4;
    }

    // Calculate remaining time
    float RemainToCharge_Energy = pSTBatt->blobIfyBMS[nBattIdx].fTotalEnergy - pSTBatt->blobIfyBMS[nBattIdx].fRemainEnergy;
    float Remain_Time = 60 * ((RemainToCharge_Energy * 1000) / (pSiglBatt->fPackTotalVolt * pSiglBatt->fPackTotalCurr));
    pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
    Push_TestState(pSTBatt, nBattIdx);

    return STATE_FULLCHARGING_STEP3;
}
 
/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-07-20 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static STATE_MACHINE_STAT_TEST OnTest_Sleep_Step4(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    TRACE("---666 OnSleep Step2, Goto Full Dischg Step5 nBattIdx=[%d]!\n", nBattIdx);
    pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP4;

    time_t tmNow = time(NULL);

    int sleepTime = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nWaitTimeStep4; // Count in minute!
    float timeGap = (ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter)/60);

    if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > sleepTime*60) //Sleep 5 Min
    {
 		return STATE_FULLDSICHARGE_STEP5;
    }

    float Remain_Time = sleepTime - timeGap;
    pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
    Push_TestState(pSTBatt, nBattIdx);

    return STATE_SLEEP_STEP4;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-07-20 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static STATE_MACHINE_STAT_TEST OnTest_FullDischg_Step5(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    TRACE("---777 OnFull Dischg Step5, Goto Sleep Step6 nBattIdx=[%d]!\n", nBattIdx);
    pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP5;

    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];
    A_BLOB_SINGLE_STBATT_RUNINFO *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];

    time_t tmNow = time(NULL);

    if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 12*60*60) //Over 6Hour,Exit
    {
        pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_FULLDISCHG_STEP5;
		return STATE_FAIL; 
    }

    //Calc_CostEnergy();

    //Predict_RemainTime();

    //Adjust BMS RealTime Output Volt/Current
    float fCalc_Batt_Distri_4_ACDC_Pwr = (float)pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nPwrLmtStep5;
    float fLimitCurrent = 0.0;

    if(pSiglBatt->fPackTotalVolt > 1.0)
	{
		fLimitCurrent = (fCalc_Batt_Distri_4_ACDC_Pwr*1000)/pSiglBatt->fPackTotalVolt;
	}

    float fCurrReqDiff = fabs(fabs(pSTBatt->fLastSendReqCurr_2CCU[nBattIdx]) - MIN(fLimitCurrent, fabs(pSiglBatt->fAllow_MaxDisChargeCurr)));
	float fCurrOutDiff = fabs(fabs(pSiglBatt->fPackTotalCurr) - MIN(fLimitCurrent, fabs(pSiglBatt->fAllow_MaxDisChargeCurr)));

    if((fabs(fCurrReqDiff) > 2.0) || ((fCurrOutDiff > 3.0) && (pSiglBatt->fPackTotalCurr < -3.0)))
    {
			if((fabs(tmNow - pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD))//Step Speed to 1.0 Cycle to Adjust the Power 
			{			
                Send_CtrlCmd(pSTBatt, nBattIdx, STAT_MANUAL_START_DISCHG, fLimitCurrent);
				
				TRACEX("Test Mode : DisCharg Adjust Cmd: nBattIdx=[%d] Adjust ChargePower fCurrReqDiff=%f fCurrOutDiff=%f\
				fLimitCurrent=%.1f fLastSendReqCurr_2CCU=%.1f fPackTotalCurr=%.1f\n",\
				nBattIdx, fCurrReqDiff, fCurrOutDiff ,fLimitCurrent ,\
				pSTBatt->fLastSendReqCurr_2CCU[nBattIdx], pSiglBatt->fPackTotalCurr);
    }
		}	

    float   fSOC = (float)pSiglBatt->nSOC;
    BOOL    bFull_DisChged = pSiglBatt->bIsBatt_DisChargeForbidden;
    float   fAllow_Dischg_Current = pSiglBatt->fAllow_MaxDisChargeCurr;

    float stopSOC = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nStopSOCStep5;
    // float minCur = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nCurLmtStep5;

    if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 60) // sample data after 20s
    {
        Update_DischargeData(pSTBatt, nBattIdx);
    }
    float DisCharge_Energy = pSTBatt->blobIfyBMS[nBattIdx].fSingleDischgCapacity;

    float VolCell_Min = pSTBatt->blobIfyBMS[nBattIdx].usMinCellVlot_Volt;
     TRACE("volcell min: %f\n", VolCell_Min);
    float TH_nDischargeEndVol_Min = pSTBatt->bmsTestModeParm.initRange.TH_nDischargeEndVol_Min;

    float   fAllow_chg_Current = pSiglBatt->fAllow_MaxChargeCurr;
    TRACE("fallowcur: %f\n", fAllow_chg_Current);
    // if((VolCell_Min<=stopVol)|| (fAllow_chg_Current < minCur))
    // if(VolCell_Min<=stopVol)
    if((tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 5*60)
    {   
        TRACE("enter to send stopcmd!!!!\n");
        STATE_TEST_RES result;
        result = DischargeEndTest_ShortTime(pSTBatt, nBattIdx);
        result = TotalTest_ShortTime(pSTBatt, nBattIdx);

        Push_TestState(pSTBatt, nBattIdx);
        Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
        return STATE_SLEEP_STEP6;
    }

    // Calculate remaining time
    float Remain_Energy = pSTBatt->blobIfyBMS[nBattIdx].fRemainEnergy;
    float Remain_Time = 60 * ((Remain_Energy * 1000) / (pSiglBatt->fPackTotalVolt * pSiglBatt->fPackTotalCurr));
    pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
    Push_TestState(pSTBatt, nBattIdx);

    return STATE_FULLDSICHARGE_STEP5 ;

}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-07-20 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static STATE_MACHINE_STAT_TEST OnTest_Sleep_Step6(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    TRACE("---888 OnSleep Step3, Goto Chg Half Step7 nBattIdx=[%d]!\n", nBattIdx);
    pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP6;

    // Push_TestState(pSTBatt, nBattIdx);

    time_t tmNow = time(NULL);

    int sleepTime = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nWaitTimeStep6; // Count in minute!
    float timeGap = (ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter)/60);

    if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > sleepTime*60) //Sleep 5 Min
    {
 		return STATE_CHARGE_HALF_STEP7;
    }

    float Remain_Time = sleepTime - timeGap;
    pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
    Push_TestState(pSTBatt, nBattIdx);

    return STATE_SLEEP_STEP6;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-07-20 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static STATE_MACHINE_STAT_TEST OnTest_Chg_Half_Step7(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    TRACE("---999 OnChg Half Step7, Goto Test Finished nBattIdx=[%d]!\n", nBattIdx);
    pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_STEP7;

    // Push_TestState(pSTBatt, nBattIdx);

    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];
    A_BLOB_SINGLE_STBATT_RUNINFO *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];

    time_t tmNow = time(NULL);

    if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 12*60*60)
    {
        pSTBatt->stuTestMode.pCurrState[nBattIdx]->emFailCode = FAIL_HALFCHG_STEP7;
		return STATE_FAIL; 
    }

    float   fSOC = (float)pSiglBatt->nSOC;

    //Adjust BMS RealTime Output Volt/Current
    float fLimitCurrent = 0.0;
    float fACDC_Distri_4_BattChrg_Pwr = (float)pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nPwrLmtStep7;
   
    if(pSiglBatt->fPackTotalVolt > 50.0)
	{
		fLimitCurrent = (fACDC_Distri_4_BattChrg_Pwr*1000)/pSiglBatt->fPackTotalVolt;
	}
    
    float fVoltReqDiff = fabs(pSTBatt->fLastSendReqVolt_2CCU[nBattIdx] - MIN(pSiglBatt->fAllow_LmtChargeVolt, 1000));
	float fCurrReqDiff = fabs(pSTBatt->fLastSendReqCurr_2CCU[nBattIdx] - MIN(pSiglBatt->fAllow_MaxChargeCurr, fLimitCurrent));
 	float fCurrOutDiff = pSiglBatt->fPackTotalCurr - pSiglBatt->fAllow_MaxChargeCurr;
	// TRACEX("tmNow=%f nBattIdx=[%d] fVoltReqDiff=[%.1f], fCurrReqDiff=[%.1f]fCurrOutDiff=%f,\
	// 	fPackTotalCurr=%f,fLastSendReqCurr_2CCU=[%.1f] fAllow_MaxChargeCurr=[%.1f] fACDC_Distri_4_BattChrg_Pwr=[%.1f]\n",\
    //     tmNow, nBattIdx, fVoltReqDiff, fCurrReqDiff, fCurrOutDiff, pSiglBatt->fPackTotalCurr,\
    //     pSTBatt->fLastSendReqCurr_2CCU[nBattIdx], pSiglBatt->fAllow_MaxChargeCurr,\
    //     fACDC_Distri_4_BattChrg_Pwr);

     if((fabs(fVoltReqDiff) > 2.0) || 
        (fabs(fCurrReqDiff) > 2.0) ||
        (fCurrOutDiff > 2.0)) 
	{
		if((fabs(tmNow - pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD))//CATL Need 3S Adjust to 0A,Otherwise BMS will Be OverCurrent Alarm
		{	
			TRACEX("Test Mode:Charge Adjust Cmd:tmNow=%f - tm_fLastSendCmd_2CCU[%d]=%lf\n", \
			tmNow , nBattIdx, pSTBatt->tm_fLastSendCmd_2CCU[nBattIdx]);
			if((pSiglBatt->fAllow_MaxChargeCurr < 20.0) || (fabs(fCurrReqDiff) > 10))
			{   
				LogOut(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
					"Adj:Idx=[%d]IPack[%.1f]Imax[%.0f]Ilmt[%.0f]Ilast[%.0f]Irect[%.1f]SOC[%d]!", \
					nBattIdx,pSiglBatt->fPackTotalCurr,pSiglBatt->fAllow_MaxChargeCurr,\
					fLimitCurrent,pSTBatt->fLastSendReqCurr_2CCU[nBattIdx],\
					pSTBatt->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSiglBatt->nSOC);
			}
            Send_CtrlCmd(pSTBatt, nBattIdx, STAT_MANUAL_START_CHG, fLimitCurrent);
		}		
	}

    float stopSOC = pSTBatt->bmsTestModeParm.autoTestParm[nBattIdx].nStopSOCStep7;

    float VolCell_Max = pSTBatt->blobIfyBMS[nBattIdx].usMaxCellVlot_Volt;    
    
    if((tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 5*60)
    {   
        Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
        return STATE_TEST_FINISHED;
    }

    // Calculate remaining time
    float RemainToCharge_Energy = (pSTBatt->blobIfyBMS[nBattIdx].fTotalEnergy *stopSOC/100)  - pSTBatt->blobIfyBMS[nBattIdx].fRemainEnergy;
    float Remain_Time = 60 * ((RemainToCharge_Energy * 1000) / (pSiglBatt->fPackTotalVolt * pSiglBatt->fPackTotalCurr));
    pSTBatt->bmsAutoTestResult[nBattIdx].timeRemain =  fabs(Remain_Time);
    Push_TestState(pSTBatt, nBattIdx);

    return STATE_CHARGE_HALF_STEP7;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-07-20 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static STATE_MACHINE_STAT_TEST OnTest_Finished(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    TRACE("---aaa OnTest Finished, Goto Idle. nBattIdx=[%d]!\n", nBattIdx);
    // pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_TEST_END;

    ClearTestHisData(pSTBatt); // clear test history data

    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];

    time_t tmNow = time(NULL);
    if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 1*10) //Over 10S
    {   
        pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_NULL;
 		return STATE_IDLE;
    }

    //Judge PM Volt /Current, If Abnormal ,Need Send Stop Cmd
    float fPM_Volt = pBranchGrp->fACDC_RealTime_Volt;
    if(fPM_Volt > MIN_PM_OUTPUT_VOLT)
    {
        Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
        pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_NULL;
        return STATE_TEST_FINISHED;
    }

    pSTBatt->bmsAutoTestResult[nBattIdx].testState =  STATE_NULL;
    Push_TestState(pSTBatt, nBattIdx);

    return STATE_IDLE;
}

static STATE_MACHINE_STAT_TEST OnTest_Paused(STBATT_SAMPLER_ROUGH* pSTBatt, int nBattIdx)
{
    TRACE("---bbb On Paused [%d]---!\n", nBattIdx);
    
    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];

    time_t tmNow = time(NULL);
    if(ABS(tmNow - pSTBatt->stuTestMode.pCurrState[nBattIdx]->tm_Enter) > 1*60) //Wait 1 Min
    {
 		return STATE_IDLE;
    }

    //Judge PM Volt /Current, If Abnormal ,Need Send Stop Cmd
    float fPM_Volt = pBranchGrp->fACDC_RealTime_Volt;
    if(fPM_Volt > MIN_PM_OUTPUT_VOLT)
    {
        Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
        return STATE_ON_PAUSED;
    }

    return STATE_IDLE;
}


/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-07-20 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void OnFail(STBATT_SAMPLER_ROUGH* pSTBatt, IN FAIL_FROM emFailFrom, IN TESTMODE_FAIL_CODE emFailCode, int nBattIdx)
{
    TRACE("---bbb !!! Oh, Test Failed nBattIdx=[%d]!\n", nBattIdx);

    A_BLOB_BRANCH_GRP_RECTDATA *pBranchGrp = &pSTBatt->branchGrpData[nBattIdx];
    A_BLOB_SINGLE_STBATT_RUNINFO *pSiglBatt = &pSTBatt->blobSingleBattInfo[nBattIdx];


    //Step.1 LogCat
    
    switch (emFailCode)
	{
        case FAIL_RELAY_VOLT_ABNORMAL:
        case FAIL_SELFCHECK:
        case FAIL_FULLDISCHG_STEP1:
        case FAIL_FULLCHG_STEP3:
        case FAIL_FULLDISCHG_STEP5:
        case FAIL_HALFCHG_STEP7:
        {

        }
        break;

        default :break;
    }

    //Judge PM Running Status.
    float fPM_Volt = pBranchGrp->fACDC_RealTime_Volt;
    if(fPM_Volt > MIN_PM_OUTPUT_VOLT)
    {
        Send_StopCmd(pSTBatt, nBattIdx, STAT_MANUAL_STOP);
    }

    return;
}
