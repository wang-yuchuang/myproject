#include "../fcgi_defs.h"

typedef enum tagChgQueryDateCond
{
    CHGREC_QUERYCOND_DATE_ALL,
    CHGREC_QUERYCOND_DATE_DAY,
    CHGREC_QUERYCOND_DATE_WEEK,
    CHGREC_QUERYCOND_DATE_MONTH
}CHGREC_QUERYCOND_DATETP;

typedef enum tagChgQueryGunCond
{
    CHGREC_QUERYCOND_GUN_ALL,
    CHGREC_QUERYCOND_GUN_A,
    CHGREC_QUERYCOND_GUN_B,
    CHGREC_QUERYCOND_GUN_C
}CHGREC_QUERYCOND_GUNTP;

typedef enum tagChgQueryStartMethodCond
{
    CHGREC_QUERYCOND_STMETHOD_ALL,
    CHGREC_QUERYCOND_STMETHOD_RFID,
    CHGREC_QUERYCOND_STMETHOD_OTP,
    CHGREC_QUERYCOND_STMETHOD_BACKEND,
    CHGREC_QUERYCOND_STMETHOD_LOCALADMIN,
    CHGREC_QUERYCOND_STMETHOD_RESERVE,
    CHGREC_QUERYCOND_STMETHOD_NOCARD,
    CHGREC_QUERYCOND_STMETHOD_PLUGNCHG
}CHGREC_QUERYCOND_STMETHODTP;

typedef struct tagAPIQueryConds
{
    int                         nPageId;
    int                         nPageVolume;
    CHGREC_QUERYCOND_DATETP     emCondDate;
    CHGREC_QUERYCOND_GUNTP      emCondGun;
    CHGREC_QUERYCOND_STMETHODTP emCondStMethod;
    char                        szSortKey[64];
    //following is for export only
    //char                        szExportName[64];
}CHRREC_API_GETCOND;

static BOOL IsSortKeyCond_TheSame(CHRREC_API_GETCOND* pCondNew, CHRREC_API_GETCOND* pCondOld)
{
    if(pCondNew->szSortKey == pCondOld->szSortKey)
    {
        return TRUE;
    }
    else if((pCondNew->szSortKey == NULL) || (pCondOld->szSortKey == NULL))
    {
        return FALSE;
    }
    else
    {
        return (strncmp(pCondNew->szSortKey, pCondOld->szSortKey, 64) == 0) ? TRUE : FALSE;
    }
}

/*
static char* Make_StopReasonStr(HIS_CHARGE_RECORD* pRec, OUT char* pOutStr)
{
    if(pRec->extraRecData.runData.emChargeEndReason >= 401)
    {
        //user canceled
        snprintf(pOutStr, 64, "%s(%d)", LoadLangText_ById(223),
            pRec->extraRecData.runData.emChargeEndReason);
    }
    else if(pRec->extraRecData.runData.emChargeEndReason <= 2)
    {
        //normal stop
        snprintf(pOutStr, 64, "%s(%d)", LoadLangText_ById(381),
            pRec->extraRecData.runData.emChargeEndReason);
    }
    else
    {
        //error code
        snprintf(pOutStr, 64, "%s(%d)", LoadLangText_ById(30),
            pRec->extraRecData.runData.emChargeEndReason);
    }  
    
    return pOutStr;
}
*/

 
/*
static char* CreateFileName_4ExportChgRecs(CHRREC_API_GETCOND *pCond, char* pszExport)
{
    // snprintf(pszExport, 64, "ChargeRec_%s.csv", 
    //         TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT_EX, szTemp, 32));
    sprintf(pszExport, "ChargeRec");
    //BOOL bIsAll = FALSE;
    char szTemp[32];
    //cond-date
    memset(szTemp, 0, 32);   
    if(pCond->emCondDate == CHGREC_QUERYCOND_DATE_DAY)
    {
        sprintf(szTemp, "_By-Day");
    }
    else if(pCond->emCondDate == CHGREC_QUERYCOND_DATE_MONTH)
    {
        sprintf(szTemp, "_By-Month");
    }
    else if(pCond->emCondDate == CHGREC_QUERYCOND_DATE_WEEK)
    {
        sprintf(szTemp, "_By-Week");
    }
    // else //(pCond->emCondDate == CHGREC_QUERYCOND_DATE_ALL)
    // {
    //     //sprintf(szTemp, "_Date-All");
    //     bIsAll = TRUE;
    // }
    strcat(pszExport, szTemp);
    //cond-gun
    memset(szTemp, 0, 32);   
    if(pCond->emCondGun == CHGREC_QUERYCOND_GUN_A)
    {
        sprintf(szTemp, "_Gun-A");
    }
    else if(pCond->emCondGun == CHGREC_QUERYCOND_GUN_B)
    {
        sprintf(szTemp, "_Gun-B");
    }
    else if(pCond->emCondGun == CHGREC_QUERYCOND_GUN_C)
    {
        sprintf(szTemp, "_Gun-C");
    }
    else //(pCond->emCondGun == CHGREC_QUERYCOND_GUN_All)
    {
        sprintf(szTemp, "_Gun-All");
    }
    strcat(pszExport, szTemp);
    
    //cond-startMethod
    memset(szTemp, 0, 32);   
    if(pCond->emCondStMethod == CHGREC_QUERYCOND_STMETHOD_RFID)
    {
        sprintf(szTemp, "_Via-RFID");
    }
    else if(pCond->emCondStMethod == CHGREC_QUERYCOND_STMETHOD_OTP)
    {
        sprintf(szTemp, "_Via-OTP");
    }
    else if(pCond->emCondStMethod == CHGREC_QUERYCOND_STMETHOD_BACKEND)
    {
        sprintf(szTemp, "_Via-Backend");
    }
    else if(pCond->emCondStMethod == CHGREC_QUERYCOND_STMETHOD_LOCALADMIN)
    {
        sprintf(szTemp, "_Via-LocalAdmin");
    }
    else if(pCond->emCondStMethod == CHGREC_QUERYCOND_STMETHOD_RESERVE)
    {
        sprintf(szTemp, "_Via-Reserve");
    }
    else if(pCond->emCondStMethod == CHGREC_QUERYCOND_STMETHOD_NOCARD)
    {
        sprintf(szTemp, "_Via-NoCard");
    }
    else if(pCond->emCondStMethod == CHGREC_QUERYCOND_STMETHOD_PLUGNCHG)
    {
        sprintf(szTemp, "_Via-PlugNChg");
    }
    // else //(pCond->emCondStMethod == CHGREC_QUERYCOND_GUN_All)
    // {
    //     sprintf(szTemp, "_Gun-All");
    // }
    strcat(pszExport, szTemp);
    //sorkey
    if(pCond->szSortKey != NULL)
    {
        if(strlen(pCond->szSortKey) > 2)
        {
            memset(szTemp, 0, 16);   
            sprintf(szTemp, "_SortKey-%s", pCond->szSortKey);
            strcat(pszExport, szTemp);          
        }
    }

    memset(szTemp, 0, 32);   
    char szTTTT[64];
    sprintf(szTTTT, "_%s.csv", TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT_EX, szTemp, 32));
    strcat(pszExport, szTTTT);

    return pszExport;
}*/

/*
static BOOL Make_ExportChgRec_andUpload(FCGI_ROUGH_DATA* pFCGI_Rough, CHRREC_API_GETCOND *pCond, BOOL bHasSortKey)
{
    TRACEX("--- Make_ExportChgRec_andUpload ---\n");

    //now judge if need re-query the record
    //Judge_IfNeedRequery(pFCGI_Rough, pCond, bHasSortKey);
    //now make the output -> /opt/appdata/data/chg_rec.csv
    #define CHGREC_EXPORT_LOCALFILE "/opt/appdata/data/bms_rec.csv"
    FILE* fp = fopen(CHGREC_EXPORT_LOCALFILE, "w+");
    if(fp == NULL)
    {
        Make_Web_OutPut(104, "Can NOT create local file for export!", NULL, NULL);
        return FALSE;          
    }

    fclose(fp);
    char szExportName[128] = "file";    
    if(!UploadFile_2Client(CHGREC_EXPORT_LOCALFILE, CreateFileName_4ExportChgRecs(pCond, szExportName)))
    {
        unlink(CHGREC_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload process is FAILED!", NULL, NULL);
        return FALSE;      
    }

    unlink(CHGREC_EXPORT_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}
*/

static char* MakeMeterValItem_2StrLine(int nIdx, A_CHARGE_METERVALUE_ITEM* pItem, OUT char* pOutStr)
{
    *pOutStr = 0;
    if(pItem->byValidElements < 1)
    {
        return NULL;
    }
    //id
    char szIdx[8];
    if(nIdx == -1)
    {
        snprintf(szIdx, 8, "Begin\t");
    }
    else if(nIdx == -2)
    {
        snprintf(szIdx, 8, "End\t");
    }
    else
    {
        snprintf(szIdx, 8, "%d\t", nIdx);
    }
    strcat(pOutStr, szIdx);
    char szTTTT[64];
    char szTime[32];
    //tmSampled
    snprintf(szTTTT, 64, "%s\t", 
        AcurTimeToString(Conv_UTC_2LocalTime_D(pItem->tmSampled), TIME_CHN_FMT, szTime, 32));
    strcat(pOutStr, szTTTT);
    //Voltage/Current/Power/Engery/SOC
    int n;
    METERVALUE_ELEMENT* pEle = pItem->meterValues;
    //volt   
    BOOL bFound = FALSE; 
    pEle = pItem->meterValues;
    for(n = 0; n < MIN(pItem->byValidElements, MAX_SUPPORT_METER_KEYS_CHARGING); n++, pEle++)
    {  
        if(pEle->tp == METER_ELETYPE_Voltage)
        {
            snprintf(szTTTT, 64, "%.1f\t", pEle->val.fVal);
            strcat(pOutStr, szTTTT);
            bFound = TRUE; 
            break;
        }
        else if(pEle->tp == METER_ELETYPE_Voltage_ABC)
        {
            snprintf(szTTTT, 64, "%.1f,%.1f,%.1f\t", (float)pEle->val.sVal_ABS[0]/10.0, 
                (float)pEle->val.sVal_ABS[1]/10.0, (float)pEle->val.sVal_ABS[2]/10.0);
            strcat(pOutStr, szTTTT);
            bFound = TRUE; 
            break;
        }
    }
    if(!bFound)
    {
        snprintf(szTTTT, 64, "--\t");
        strcat(pOutStr, szTTTT);
    }
    //current
    bFound = FALSE; 
    pEle = pItem->meterValues;
    for(n = 0; n < MIN(pItem->byValidElements, MAX_SUPPORT_METER_KEYS_CHARGING); n++, pEle++)
    {        
        if(pEle->tp == METER_ELETYPE_Current_Import)
        {
            snprintf(szTTTT, 64, "%.1f\t", pEle->val.fVal);
            strcat(pOutStr, szTTTT);
            bFound = TRUE; 
            break;
        }
        else if(pEle->tp == METER_ELETYPE_Current_Import_ABC)
        {
            snprintf(szTTTT, 64, "%.1f,%.1f,%.1f\t", (float)pEle->val.sVal_ABS[0]/10.0, 
                (float)pEle->val.sVal_ABS[1]/10.0, (float)pEle->val.sVal_ABS[2]/10.0);
            strcat(pOutStr, szTTTT);
            bFound = TRUE; 
            break;
        }    
    }
    if(!bFound)
    {
        snprintf(szTTTT, 64, "--\t");
        strcat(pOutStr, szTTTT);
    }
    //power
    bFound = FALSE; 
    pEle = pItem->meterValues;
    for(n = 0; n < MIN(pItem->byValidElements, MAX_SUPPORT_METER_KEYS_CHARGING); n++, pEle++)
    {        
        if(pEle->tp == METER_ELETYPE_Power_Active_Import)
        {
            snprintf(szTTTT, 64, "%.1f\t", pEle->val.fVal);
            strcat(pOutStr, szTTTT);
            bFound = TRUE; 
            break;
        }   
    }
    if(!bFound)
    {
        snprintf(szTTTT, 64, "--\t");
        strcat(pOutStr, szTTTT);
    }
    //Energy
    bFound = FALSE; 
    pEle = pItem->meterValues;
    for(n = 0; n < MIN(pItem->byValidElements, MAX_SUPPORT_METER_KEYS_CHARGING); n++, pEle++)
    {        
        if(pEle->tp == METER_ELETYPE_Engergy_Active_Import_Register)
        {
            snprintf(szTTTT, 64, "%.2f\t", pEle->val.fVal);
            strcat(pOutStr, szTTTT);
            bFound = TRUE; 
            break;
        }       
    }
    if(!bFound)
    {
        snprintf(szTTTT, 64, "--\t");
        strcat(pOutStr, szTTTT);
    }
    //SOC
    bFound = FALSE; 
    pEle = pItem->meterValues;
    for(n = 0; n < MIN(pItem->byValidElements, MAX_SUPPORT_METER_KEYS_CHARGING); n++, pEle++)
    {        
        if(pEle->tp == METER_ELETYPE_Battery_SOC)
        {
            snprintf(szTTTT, 64, "%.0f%%\t", pEle->val.fVal);
            strcat(pOutStr, szTTTT);
            bFound = TRUE; 
            break;
        }    
    }
    if(!bFound)
    {
        snprintf(szTTTT, 64, "--\t");
        strcat(pOutStr, szTTTT);
    }
    //Report OK?
    snprintf(szTTTT, 64, "%s\t", pItem->bIsReportedOK ? "TRUE" : "FALSE");
    strcat(pOutStr, szTTTT);

    return pOutStr;
}

/*
static BOOL Make_ExportMeterVal_andUpload(FCGI_ROUGH_DATA* pFCGI_Rough, QUERYCOND_HISMETERVAL* pCond)
{
    if(!QueryMeterVal_byCond(pCond))
    {
        Make_Web_OutPut(104, "Did not find such meterval record!", NULL, NULL);
        return FALSE;     
    }

    // TRACEX("----The get totalValItem=%d----\n", 
    //     pFCGI_Rough->chargerDataRepo.merterVal_Rec.meterData.nMeterValTotal);
    // return TRUE;

    //now make the output -> /opt/appdata/data/meter_val.csv
#define METERVAL_EXPORT_LOCALFILE "/opt/appdata/data/meter_val.csv"
    FILE* fp = fopen(METERVAL_EXPORT_LOCALFILE, "w+");
    if(fp == NULL)
    {
        Make_Web_OutPut(104, "Can NOT create local file for export!", NULL, NULL);
        return FALSE;          
    }
    char szRecLine[512];
    int n;
    sprintf(szRecLine, "%s", "Index \t Time-Sampled \t Voltage(V) \t Current(A) \t Power(kW) \t Energy.Register(kWh) \t SOC \t Reported-2-Backend\n");
    fputs(szRecLine, fp);    
    int nTotal = MIN(pFCGI_Rough->chargerDataRepo.merterVal_Rec.meterData.nMeterValTotal,
            MAX_HIS_STORE_METER_VALUES);    
    //start/stop
    A_CHARGE_METERVALUE_ITEM* pRec = &pFCGI_Rough->chargerDataRepo.merterVal_Rec.meterData.stMeterVal_Start;
    if(MakeMeterValItem_2StrLine(-1, pRec, szRecLine) != NULL)
    {
        fputs(szRecLine, fp);
    }
    pRec = &pFCGI_Rough->chargerDataRepo.merterVal_Rec.meterData.stMeterVal_Stop;
    if(MakeMeterValItem_2StrLine(-2, pRec, szRecLine) != NULL)
    {
        fputs(szRecLine, fp);
    }
    //now each 
    pRec = pFCGI_Rough->chargerDataRepo.merterVal_Rec.meterData.stMeterValList;
    for(n = 0; n < nTotal; n++, pRec++)
    {        
        if(MakeMeterValItem_2StrLine(n + 1, pRec, szRecLine) != NULL)
        {
            fputs(szRecLine, fp);
        }
    }
    
    fclose(fp);
    char szRemoteName[64];
    char szTemp[32];
    char szTemp1[32];
    snprintf(szRemoteName, 64, "meterval_tmstart-%s_gun-%d_%s.csv", 
            TimeToString(Conv_UTC_2LocalTime(pCond->tmChargeStart), TIME_CHN_FMT_EX, szTemp, 32),
            pCond->nGunIdx,
            TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT_EX, szTemp1, 32));
    if(!UploadFile_2Client(METERVAL_EXPORT_LOCALFILE, szRemoteName))
    {
        Make_Web_OutPut(104, "Upload process is FAILED!", NULL, NULL);
        return FALSE;      
    }

    unlink(METERVAL_EXPORT_LOCALFILE);

    return FALSE; //shoud return FALSE!!!
}
*/


char *BmsStatusToString(IN STBATT_RUN_STATU emBattStatus_Run, OUT char *strBmsStatus, IN int nLenStrSize)
{
    switch(emBattStatus_Run)
    {
        case STBATT_RUNSTAT_NOT_INSTALLED :
            snprintf(strBmsStatus, nLenStrSize, "%s", "Not Install");  
            break;
        case STBATT_RUNSTAT_COMM_FAIL :
            snprintf(strBmsStatus, nLenStrSize, "%s", "Comm Fail");  
            break;
        case STBATT_RUNSTAT_SLEEP :
            snprintf(strBmsStatus, nLenStrSize, "%s", "Sleep");  
            break;
        case STBATT_RUNSTAT_IDLE :
            snprintf(strBmsStatus, nLenStrSize, "%s", "Idle");  
            break;
        case STBATT_RUNSTAT_CHARGE :
            snprintf(strBmsStatus, nLenStrSize, "%s", "Charging");  
            break;
        case STBATT_RUNSTAT_DISCHARGE :
            snprintf(strBmsStatus, nLenStrSize, "%s", "DisCharging");  
            break;
        case STBATT_RUNSTAT_FULL_CHARGE :
            snprintf(strBmsStatus, nLenStrSize, "%s", "Full Chrg");  
            break;
        case STBATT_RUNSTAT_FULL_DISCHARGE :
            snprintf(strBmsStatus, nLenStrSize, "%s", "Full Dischrg");  
            break;
        default:
            snprintf(strBmsStatus, nLenStrSize, "%s", "unknown");  
            break;
    }
 
	return strBmsStatus;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Make BmsRunDataItem_2StrLine()
*  CREATOR       : 
*  CREATED DATE  : 2021-12-23 22:13:53
*  DESCRIPTION   : 
*      
*  
*  CHANGE HISTORY:    
*  
***********************************************************************************/
static char* Make_BmsRunDataItem_2StrLine(FCGI_ROUGH_DATA* pFCGI_Rough, HIS_BMS_RUNTIMEDATA_REC* pRec, int nIdx, OUT char* pOutStr, UINT* Index)
{
    A_BMS_RUNTIME_DATA_ITEM* pItem = &pRec->bmsRunData.stBmsRuntimeDataList[nIdx];
    *pOutStr = 0;
    char szIdx[8];
    char szTime[32];
    char szTTTT[64];
    char szBmsStatus[16];
    //id
    snprintf(szIdx, 8, "%d,", *Index + 1);
    (*Index)++;

    strcat(pOutStr, szIdx);
    // Time,
    snprintf(szTTTT, 64, "%s,", TimeToString(pItem->tmSaved, "%H:%M:%S", szTime, sizeof(szTime)));
    strcat(pOutStr, szTTTT);
    // GridMeter_V,
    snprintf(szTTTT, 64, "%.1f|%.1f|%.1f,", pItem->fGridMeter_Volt[0],pItem->fGridMeter_Volt[1],pItem->fGridMeter_Volt[2]);
    strcat(pOutStr, szTTTT);
    // GridMeter_A,
    snprintf(szTTTT, 64, "%.1f|%.1f|%.1f,", pItem->fGridMeter_Curr[0],pItem->fGridMeter_Curr[1],pItem->fGridMeter_Curr[2]);
    strcat(pOutStr, szTTTT);
    // GridMeter_kW,GridMeter_kWh(In),GridMeter_kWh(Out),
    snprintf(szTTTT, 64, "%.1f,%.1f,%.1f,", pItem->fGridMeter_Power[1],pItem->fGridMeter_Power[0],pItem->fGridMeter_Power[2]); 
    strcat(pOutStr, szTTTT);
    // CabMeter_V,
    snprintf(szTTTT, 64, "%.1f|%.1f|%.1f,", pItem->fCabMeter_Volt[0],pItem->fCabMeter_Volt[1],pItem->fCabMeter_Volt[2]);
    strcat(pOutStr, szTTTT);
    // CabMeter_A,
    snprintf(szTTTT, 64, "%.1f|%.1f|%.1f,", pItem->fCabMeter_Curr[0],pItem->fCabMeter_Curr[1],pItem->fCabMeter_Curr[2]);
    strcat(pOutStr, szTTTT);
    // CabMeter_kW,CabMeter_kWh(In),CabMeter_kWh(Out),GrpId,
    snprintf(szTTTT, 64, "%.1f,%.1f,%.1f,%d,", pItem->fCabMeter_Power[1],pItem->fCabMeter_Power[0],pItem->fCabMeter_Power[2],pItem->nBmsIdx); 
    strcat(pOutStr, szTTTT);
    if (pFCGI_Rough->emSystemType == SYSTYPE_EMS_STATION)
    {
        // diff1
        // CabMeter_Client,ComFail,SOC,REMS|Ctrl,RPwr,
        snprintf(szTTTT, 64, "%.1f,%d,%d,%d|%d,%.1f,",
            pItem->f_PcabMeter_Client,
            pItem->bCommFailSemsClient,
            pItem->nSOC,
            pItem->emEMSCtrlMode_REMS,
            pItem->emACDC_WorkStatu_REMS,
            pItem->fActvPwrReq_REMS);
        strcat(pOutStr, szTTTT);
    }
    else
    {
        // diff1
        // BMS_Status,Alm_Status,SOC,SOH,REMS|Ctrl,RPwr,Vout|Vnb,PR|NR,OrgBMS,
        snprintf(szTTTT, 64, "%s,%d,%d,%d,"
                "%d|%d,%.1f,%.1f|%.1f,%.1f|%.1f,%d,", 
            BmsStatusToString(pItem->emBattStatus_Run,szBmsStatus,sizeof(szBmsStatus)), 
            pItem->emBattStatus_Alm, 
            pItem->nSOC,
            pItem->nSOH,

            pItem->emEMSCtrlMode_REMS,
            pItem->emACDC_WorkStatu_REMS,
            pItem->fActvPwrReq_REMS,
            pItem->fOutsideVolt, 
            pItem->fNegativeBusVolt,
            pItem->fInsulation_P_R,
            pItem->fInsulation_N_R,
            pItem->usBattWorkState);
        strcat(pOutStr, szTTTT);
        // BMS_V,BMS_A,Ctrl_V,Ctrl_A,
        snprintf(szTTTT, 64, "%.1f,%.1f,%.1f,%.1f,", 
            pItem->fPackTotalVolt,
            pItem->fPackTotalCurr,
            pItem->fPMCtrl_Volt, 
            pItem->fPMCtrl_Curr);
        strcat(pOutStr, szTTTT);
    }
    // same1 LJH250610 add:PCSOut
    // Ctrl_CMD,PCS_Status,PCSOut,PCSOut_V,PCSOut_A,
    static char s_ctrlCmds[GRP_DCDC_STOP+1][15] = {"Rect", "RectAdj", "Invt", "InvtPend", "Stop", "Rect", "RectPend", "Stop"};
    BYTE emCtrlPcsCmd = pItem->emCtrlPcsCmd;
    if (emCtrlPcsCmd > GRP_DCDC_STOP)
    {
        snprintf(szTTTT, 64, "%d,", emCtrlPcsCmd);
    }
    else
    {
        snprintf(szTTTT, 64, "%s,", s_ctrlCmds[emCtrlPcsCmd]);
    }
    strcat(pOutStr, szTTTT);

    static char s_pcsStatus[STATU_ACDC_STANDBY+1][10] = {"Idle", "Rect", "Invt", "Stand"};
    BYTE byPCSStatu = pItem->byPCSStatu;
    if (byPCSStatu > STATU_ACDC_STANDBY)
    {
        snprintf(szTTTT, 64, "%d,", byPCSStatu);
    }
    else
    {
        snprintf(szTTTT, 64, "%s,", s_pcsStatus[byPCSStatu]);
    }
    strcat(pOutStr, szTTTT);
    // PCSOut,PCSOut_V,PCSOut_A LJH250610 add:PCSOut 
    snprintf(szTTTT, 64, "%.1f,%.1f,%.1f,", 
        pItem->fActvPwr_PCSGrp,
        pItem->fPMOutput_Volt,
        pItem->fPMOutput_Curr);
    strcat(pOutStr, szTTTT);
    if (pFCGI_Rough->emSystemType == SYSTYPE_EMS_STATION)
    {
        // diff2
    }
    else
    {
        // diff2
        // MPPTOut_V,MPPTOut_A,
        snprintf(szTTTT, 64, "%.1f,%.1f,", 
            pItem->fMPPTOutPut_Volt,
            pItem->fMPPTOutPut_Curr);
        strcat(pOutStr, szTTTT);
    }
    // same2
    // CalcChg,CalcAvgChg,CalcDisChg,CalcAvgDisChg,
    snprintf(szTTTT, 64, "%.1f,%.1f,%.1f,%.1f,", 
	    pItem->fCalc_Chg_Power,
        pItem->fAvg_Calc_Chg_Power,
        pItem->fCalc_DisChg_Power,
        pItem->fAvg_Calc_DisChg_Power);
    strcat(pOutStr, szTTTT);
    // LJH241017 add Rem4Chg
    // ESSChg_kW,ESSDisChg_kW,Load4Chg,Load4DisChg,Rem4Chg,
    snprintf(szTTTT, 64, "%.1f,%.1f,%.1f,%.1f,%.1f %.1f|%.1f,", 
        pItem->fTotal_PessChgAvaliable,
        pItem->fTotal_PessDischgLmt,
        pItem->fPacload4Chg,
	    pItem->fPacload4Dischg,

        pItem->fGridRemainACPwr2Chrg,
        pItem->fRemainPwr4Chrg, // pEMS->fRemainPwr4Chrg = pEMS->fGridACInputPwr_Lmt - pEMS->fPacload4Chg; // 剩余充电功率
        pItem->fRemainDemand4Chrg);
    strcat(pOutStr, szTTTT);
    if (pFCGI_Rough->emSystemType == SYSTYPE_EMS_STATION)
    {
        // diff3 LJH250610 fPwrAbility0
        // MaxChg|DisChg,MaxChg0|DisChg0,AntiMode,AutoChDis,
        snprintf(szTTTT, 64, "%.1f|%.1f,%.1f|%.1f,%d|%d|%d %d|%d,%d|%d,", 
            pItem->fMax_Chrg_PwrAbility,
            pItem->fMax_DisChrg_PwrAbility,
            pItem->fMax_Chrg_PwrAbility0,
            pItem->fMax_DisChrg_PwrAbility0,

            pItem->antiMode[0], pItem->antiMode[1], pItem->antiMode[2],
            pItem->antiMode_Phase[0], pItem->antiMode_Phase[1],
            pItem->autoChrgDischrg[0], pItem->autoChrgDischrg[1]);
        strcat(pOutStr, szTTTT);
        // MaxRT_A, // AuxMeter1_kW,AuxMeter3_kW, // Rflx_kW,Ovl_kW,Demand
        snprintf(szTTTT, 64, "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",
            pItem->fAllow_MaxChargeCurr, // Update_RuntimeData_2DB_SEMS:fRealTime_MaxCurrSum
            pItem->f_PauxMeter,
            pItem->f_PauxMeter_3,
            pItem->fAtRflx_kW,
            pItem->fAtOvl_kW,
            pItem->f_PmaxGridDemand);
        strcat(pOutStr, szTTTT);
    }
    else
    {
        // diff3 MaxChg|DisChg,Rated|PwrLmt,BattChg|DisChg,MaxPCS,AntiMode,
        snprintf(szTTTT, 64, "%.1f|%.1f,%.1f|%u,%.1f|%.1f,%.1f,%d|%d|%d %d|%d,", 
            pItem->fMax_Chrg_PwrAbility,
            pItem->fMax_DisChrg_PwrAbility,
            pItem->fRatedPower,
            pItem->nBattGrpPwrLmt,
            pItem->fBatt_Max_Chrg_Pwr_Req,
            pItem->fBatt_Max_DisChrg_Pwr,
            pItem->fACDC_Max_Output_Pwr_Ability,

            pItem->antiMode[0], pItem->antiMode[1], pItem->antiMode[2],
            pItem->antiMode_Phase[0], pItem->antiMode_Phase[1]);
        strcat(pOutStr, szTTTT);
        // LmtChg_V,LmtDisChg_V,LmtChg_A,LmtDisChg_A,
        // Cell_Max_V,Cell_Min_V,Cell_Gap_V,Cell_Avg_V,
        float fUCell_Diff = pItem->fMax_CellVolt - pItem->fMin_CellVolt;
        snprintf(szTTTT, 64, "%.1f,%.1f,%.1f,%.1f,%.3f,%.3f,%.3f,%.3f,", 
            pItem->fAllow_LmtChargeVolt,
            pItem->fAllow_LmtDisChargeVolt,
            pItem->fAllow_MaxChargeCurr,
            pItem->fAllow_MaxDisChargeCurr,

            pItem->fMax_CellVolt, 
            pItem->fMin_CellVolt,
            fUCell_Diff,
            pItem->fAvg_CellVolt);
        strcat(pOutStr, szTTTT);

        // PackNum(Max_V),CellNum(Max_V),PackNum(Min_V),CellNum(Min_V),
        // Cell_Max_T,Cell_Min_T,Cell_Gap_T,Cell_Avg_T,
        float fBAT_T_Diff = pItem->fMax_CellTemp - pItem->fMin_CellTemp;
        snprintf(szTTTT, 64, "%d,%d,%d,%d,"
            "%.1f,%.1f,%.1f,%.1f,", 
            pItem->uiBAT_Ucell_Max_CSC,
            pItem->uiBAT_Ucell_Max_CELL,
            pItem->uiBAT_Ucell_Min_CSC,
            pItem->uiBAT_Ucell_Min_CELL,

            pItem->fMax_CellTemp,
            pItem->fMin_CellTemp,
            fBAT_T_Diff,
            pItem->fAvg_CellTemp);
        strcat(pOutStr, szTTTT);
        // PackNum(Max_T),PackNum(Min_T),
        // BAT_T_Max_CSC,BAT_T_Min_CSC,Chrg_Energy,DisChrging_Energy,SingleChrgCapacity,SingleDischrgCapacity,
        snprintf(szTTTT, 64, "%d,%d,%.1f,%.1f,%.1f,%.1f,", 
            pItem->nBAT_T_Max_CSC,
            pItem->nBAT_T_Min_CSC,
            pItem->fChrg_Energy,
            pItem->fDisChrging_Energy,
            pItem->fSingleChrgCapacity,
            pItem->fSingleDischrgCapacity
            );
        strcat(pOutStr, szTTTT);
        // AccuChrgCapacity,AccuDischrgCapacity,AccuChrgEnergy,AccuDischrgEnergy,
        // SingleChargeEnergy,SingleDischargeEnergy,OnOrOffd,Air_Temp1,Air_Temp2,Air_Humidity,
        snprintf(szTTTT, 64, "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%d,%.1f,%.1f,%.1f,", 
            pItem->AccuChrgCapacity,
            pItem->fAccuDischrgCapacity,
            pItem->fAccuChrgEnergy,
            pItem->fAccuDischrgEnergy,
            pItem->fSingleChargeEnergy,
            pItem->fSingleDischargeEnergy,
            pItem->byOnOrOffd,
            pItem->fAir_Temp1,
            pItem->fAir_Temp2,
            pItem->fAir_Humidity);
        strcat(pOutStr, szTTTT);
        // AuxMeter1_kW,AuxMeter3_kW,
        snprintf(szTTTT, 64, "%.1f,%.1f,",
            pItem->f_PauxMeter,
            pItem->f_PauxMeter_3);
        strcat(pOutStr, szTTTT);
        // Rflx_kW,Ovl_kW, // LJH241007 AntiPhase RunData
        snprintf(szTTTT, 64, "%.1f %.1f|%.1f|%.1f %.2f,%.1f,",
            pItem->fAtRflx_kW,
            pItem->fAtRflxMtr_kWPhs[0], pItem->fAtRflxMtr_kWPhs[1], pItem->fAtRflxMtr_kWPhs[2], pItem->fAtRflxMtr_Factor,
            pItem->fAtOvl_kW);
        strcat(pOutStr, szTTTT);
        /* // PhsDiffDis,PhOvl_A,PhsMaxChrg, PhsDiffDis:比预保护值(单相防逆流)差值的功率和 PhOvl_A:单相电流值 PhsMaxChrg:最大单相功率和
        snprintf(szTTTT, 64, "%.1f,%.1f|%.1f|%.1f %.2f,%.1f,",
            pItem->fDiffPwr4Dischrg_SumPhs,
            pItem->fAtOvlMtr_APhs[0], pItem->fAtOvlMtr_APhs[1], pItem->fAtOvlMtr_APhs[2], pItem->fAtOvlMtr_Factor,
            pItem->fMaxPwr4Chrg_SumPhs);
        strcat(pOutStr, szTTTT);*/
        // LiqColAirCondition
        snprintf(szTTTT, 64, "%d,%d,%d,%d,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,",
            pItem->byLCAC_Type,
            pItem->byLCAC_WorkMode,
            pItem->byLCAC_BmsSetWM,
            pItem->byLCAC_CompState,
            pItem->fLCAC_CompSpeed,
            pItem->fLCAC_ColTemp,
            pItem->fLCAC_ColOpenDiff,
            pItem->fLCAC_ColStopDiff,
            pItem->fLCAC_HeatTemp,
            pItem->fLCAC_HeatOpenDiff,
            pItem->fLCAC_HeatStopDiff);
        strcat(pOutStr, szTTTT);
        snprintf(szTTTT, 64, "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,",
            pItem->fLCAC_XmSetTemp,
            pItem->fLCAC_AmbiT,
            pItem->fLCAC_FeedT,
            pItem->fLCAC_ReturnT,
            pItem->fLCAC_ModFeedT,
            pItem->fLCAC_ModReturnT,
            pItem->fLCAC_ExPre,
            pItem->fLCAC_FeedPre,
            pItem->fLCAC_ReturnPre);
        strcat(pOutStr, szTTTT);
        snprintf(szTTTT, 64, "%04X|%04X|%04X|%04X|%04X|%04X,",
            pItem->usLCAC_FaultCode[0],
            pItem->usLCAC_FaultCode[1],
            pItem->usLCAC_FaultCode[2],
            pItem->usLCAC_FaultCode[3],
            pItem->usLCAC_FaultCode[4],
            pItem->usLCAC_FaultCode[5]);
        strcat(pOutStr, szTTTT);
        // SysT1,SysT2,SysT3,Demand,
        snprintf(szTTTT, 64, "%.1f,%.1f,%.1f,%.1f,", 
            pItem->fPowerCubeTemp[0],
            pItem->fPowerCubeTemp[1],
            pItem->fPowerCubeTemp[2],
            pItem->f_PmaxGridDemand);
        strcat(pOutStr, szTTTT);
        // VpmOut,PCS_kVar,GridMeter_kVar,CabMeter_kVar,Ctrl_kVar,
        snprintf(szTTTT, 64, "%.1f,%.1f,%.1f,%.1f,%.1f,", 
            pItem->fACDCOutRelayVolt,
            pItem->fQ_PCSGrp,
            pItem->f_QgridMeter,
            pItem->f_QcabMeter,
            pItem->fQ_PCSGrpCtrl);
        strcat(pOutStr, szTTTT);

        // STS_Status,PCSSys#1|#2|#|#,
        snprintf(szTTTT, 64, "%02X,%02X|%02X|%02X|%02X,", 
            pItem->bySTSSysStatu ,    // 24 ABUAlmStatus0.
            pItem->byPCSSysStatu[0] , // 20 PCSGrp#1 Idx:1 RunStatu   pCCU->strLSYSInfo.mapPCSSysStatus[n].byPCSSysStatu = (BYTE)pData[0];
            pItem->byPCSSysStatu[1] , // 16 PCS Grp#1 Idx:2 RunSTatus pCCU->strLSYSInfo.mapPCSSysStatusBelong2[n].byPCSSysStatu = (BYTE)pData[0];
            pItem->byPCSSysStatu[2] ,
            pItem->byPCSSysStatu[3]);
        strcat(pOutStr, szTTTT);
        // BMSFail,
        snprintf(szTTTT, 64, "%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X,", 
            pItem->szFault[0] ,
            pItem->szFault[1] ,
            pItem->szFault[2] ,
            pItem->szFault[3] ,
            pItem->szFault[4] ,
            pItem->szFault[5] ,
            pItem->szFault[6] ,
            pItem->szFault[7]);
        strcat(pOutStr, szTTTT);
        // Alm,
        snprintf(szTTTT, 64, "%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X,", 
            pItem->szAlarm[0] ,
            pItem->szAlarm[1] ,
            pItem->szAlarm[2] ,
            pItem->szAlarm[3] ,
            pItem->szAlarm[4] ,
            pItem->szAlarm[5] ,
            pItem->szAlarm[6] ,
            pItem->szAlarm[7]);
        strcat(pOutStr, szTTTT);
        // LongAlm_info1-7,
        snprintf(szTTTT, 64, "%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X,", 
            pItem->szLONGALARM[0] ,
            pItem->szLONGALARM[1] ,
            pItem->szLONGALARM[2] ,
            pItem->szLONGALARM[3] ,
            pItem->szLONGALARM[4] ,
            pItem->szLONGALARM[5] ,
            pItem->szLONGALARM[6] ,
            pItem->szLONGALARM[7] ,
            pItem->szLONGALARM[8] ,
            pItem->szLONGALARM[9] ,
            pItem->szLONGALARM[10] ,
            pItem->szLONGALARM[11] ,
            pItem->szLONGALARM[12] ,
            pItem->szLONGALARM[13]);
        strcat(pOutStr, szTTTT);
        // DIDO
        snprintf(szTTTT, 64, "%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X,", 
            pItem->szDIDOStatu[0] ,
            pItem->szDIDOStatu[1] ,
            pItem->szDIDOStatu[2] ,
            pItem->szDIDOStatu[3] ,
            pItem->szDIDOStatu[4] ,
            pItem->szDIDOStatu[5] ,
            pItem->szDIDOStatu[6] ,
            pItem->szDIDOStatu[7]);
        strcat(pOutStr, szTTTT);
        //BatEu3_DI,BatEu3_DO
        snprintf(szTTTT, 64, "%02X|%02X|%02X,%02X|%02X|%02X,",
            pItem->by_BatEu3_DIStatus[2],
            pItem->by_BatEu3_DIStatus[1],
            pItem->by_BatEu3_DIStatus[0],
            pItem->by_BatEu3_DOStatus[2],
            pItem->by_BatEu3_DOStatus[1],
            pItem->by_BatEu3_DOStatus[0]);
        strcat(pOutStr, szTTTT);
        //NTC_BatEu3_Temp[4],TemperatureSen,HumiditySen,GasConcent1H2,GasConcent2C0,Humidity,
        snprintf(szTTTT, 64, "%.1f|%.1f|%.1f|%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,",
            pItem->fNTC_BatEu3_Temp[0],
            pItem->fNTC_BatEu3_Temp[1],
            pItem->fNTC_BatEu3_Temp[2],
            pItem->fNTC_BatEu3_Temp[3],
            pItem->fTemperatureSen,
            pItem->fHumiditySen,
            pItem->fGasConcent1H2,
            pItem->fGasConcent2C0,
            pItem->fHumidity);
        strcat(pOutStr, szTTTT);
        //PowerEu3_DI,PowerEu3_DO
        snprintf(szTTTT, 64, "%02X|%02X|%02X,%02X|%02X|%02X,",
            pItem->by_PowerEu3_DIStatus[2],
            pItem->by_PowerEu3_DIStatus[1],
            pItem->by_PowerEu3_DIStatus[0],
            pItem->by_PowerEu3_DOStatus[2],
            pItem->by_PowerEu3_DOStatus[1],
            pItem->by_PowerEu3_DOStatus[0]);
        strcat(pOutStr, szTTTT);
        //NTC_BatEu3_Temp[4],
        snprintf(szTTTT, 64, "%.1f|%.1f|%.1f|%.1f,\n",
            pItem->fNTC_PowerEu3_Temp[0],
            pItem->fNTC_PowerEu3_Temp[1],
            pItem->fNTC_PowerEu3_Temp[2],
            pItem->fNTC_PowerEu3_Temp[3]);
        strcat(pOutStr, szTTTT);


    }

    return pOutStr;
}

void Sort_Array(UINT* pArray, UINT nCount)
{
    UINT i, j, temp;
    for(i = 0; i < nCount - 1; ++i)
    {
        for(j = 0; j < nCount - i - 1; ++j)
        {
            if(pArray[j] > pArray[j + 1])
            {
                temp = pArray[j];
                pArray[j] = pArray[j + 1];
                pArray[j + 1] = temp;
            }
        }
    }
}

/*********************************************************************************
*  
*  FUNCTION NAME : Make_Export_ChgRunData_andUpload()
*  CREATOR       : 
*  CREATED DATE  : 2021-12-21 22:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:    
*  
***********************************************************************************/
static BOOL Make_Export_BmsRunData_andUpload(FCGI_ROUGH_DATA* pFCGI_Rough, QUERYCOND_HISMETERVAL* pCond)
{
    sqlite3 *db;
    int rc = sqlite3_open_v2( "/data/BmsRunData.data",
            &db, /*SQLITE_OPEN_READWRITE*/SQLITE_OPEN_READONLY, NULL); //has to be read-only
    if(rc != SQLITE_OK)
    {
        printf("----can NOT open %s db---\n", "DATA");
        return FALSE;
    }

    sqlite3_busy_timeout(db, 5000);

    UINT nCount = 0;
    UINT tTimeSum[MAX_BMS_RECORDS_NUM] = {0};
    
    sqlite3_stmt * stat;
    char szSql[255];
    sprintf(szSql, "SELECT * FROM Data_RealTimeBmsData Where Day = %d", pCond->nGunIdx);
    sqlite3_prepare_v2(db, szSql, -1, &stat, NULL);

	while(sqlite3_step(stat) == SQLITE_ROW)
    {
        if(nCount >= MAX_BMS_RECORDS_NUM)
        {
            break;
        }
        tTimeSum[nCount] = sqlite3_column_int(stat, 2);
        nCount++;
    }   
    sqlite3_finalize(stat);

    Sort_Array(tTimeSum, nCount);

    //printf("--- 222 ---\n");
    char szRecLine[1536];
    int n;
    // int nTotal = MIN(pFCGI_Rough->chargerDataRepo.bmsRunData_Rec.bmsRunData.nBmsRuntimeDataTotal,
    //         MAX_HIS_BMS_RUNTIME_DATALIST);    
    // if(nTotal < 1)
    // {
    //     Make_Web_OutPut(104, "The record is empty!", NULL, NULL);
    //     return FALSE;      
    // }
    //printf("--- 333 --- nTotal=[%d]\n", nTotal);

    //now make the output -> /opt/appdata/data/chg_rundata.csv
#define CHGRUNDATA_EXPORT_LOCALFILE "/opt/appdata/data/"
#define CHGRUNDATA_EXPORT_TARFILE "/opt/appdata/data/bms_rundata.tar.gz"

    char szBmsName[128];
    char szbmsTemp[32];
    char szbmsTemp1[32];
    snprintf(szBmsName, 128, "%s_bms_rundata[%s].csv", 
            pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
            TimeToString(Conv_UTC_2LocalTime(pCond->nGunIdx), ISO8601_FMT_2, szbmsTemp, 32));

    char szBmsRunPath[256];
    snprintf(szBmsRunPath, 256, "%s%s", CHGRUNDATA_EXPORT_LOCALFILE, szBmsName);
   
    FILE* fp = fopen(szBmsRunPath, "w+");
    if(fp == NULL)
    {
        Make_Web_OutPut(104, "Can NOT create local file for export!", NULL, NULL);
        return FALSE;          
    }
    //printf("--- 444 ---\n");

    //now for each item, we print it one by one
    //A_CHARGE_RUNTIME_DATA_ITEM* pItem = pFCGI_Rough->chargerDataRepo.bmsRunData_Rec.chgRunData.stChgRuntimeDataList;
    HIS_BMS_RUNTIMEDATA_REC* pRec = &pFCGI_Rough->chargerDataRepo.bmsRunData_Rec;    
    //title for the data
    memset(szRecLine, '\0', 1536);
    snprintf(szRecLine, 1536,
            "Id,Time,"
            "GridMeter_V,GridMeter_A,GridMeter_kW,GridMeter_kWh(In),GridMeter_kWh(Out),"
            "CabMeter_V,CabMeter_A,"
            "CabMeter_kW,CabMeter_kWh(In),CabMeter_kWh(Out),GrpId,");
    int nUsed = strlen(szRecLine);
    // LJH241202 SEMS rundata
    if (pFCGI_Rough->emSystemType == SYSTYPE_EMS_STATION)
    {
        snprintf(szRecLine + nUsed, 1536-nUsed,
            // diff1
            "CabMeter_Client,ComFail,SOC,EMSMode,PwrReq,"
            // same1 LJH250610 add:PCSOut
            "Ctrl_CMD,PCS_Status,PCSOut,PCSOut_V,PCSOut_A,"
            // diff2
            // same2
            "CalcChg,CalcAvgChg,CalcDisChg,CalcAvgDisChg,"
            "ESSChg_kW,ESSDisChg_kW,Load4Chg,Load4DisChg,Rem4Chg,"
            // diff3
            "MaxChg|DisChg,MaxChg0|DisChg0,AntiMode,AutoChDis,"
            "MaxRT_A,"
            "AuxMeter1_kW,AuxMeter3_kW,"
            "Rflx_kW,Ovl_kW,Demand" //防逆流表,防过载表,最大需量
            "\n");
    }
    else
    {
        snprintf(szRecLine + nUsed, 1536-nUsed,
            // diff1
            "BMS_Status,Alm_Status,SOC,SOH,REMS|Ctrl,RPwr,Vout|Vnb,PR|NR,OrgBMS,"
            "BMS_V,BMS_A,Ctrl_V,Ctrl_A,"
            // same1 LJH250610 add:PCSOut
            "Ctrl_CMD,PCS_Status,PCSOut,PCSOut_V,PCSOut_A,"
            // diff2
            "MPPTOut_V,MPPTOut_A,"
            // same2
            "CalcChg,CalcAvgChg,CalcDisChg,CalcAvgDisChg,"
            "ESSChg_kW,ESSDisChg_kW,Load4Chg,Load4DisChg,Rem4Chg,"
            // diff3
            "MaxChg|DisChg,Rated|PwrLmt,BattChg|DisChg,MaxPCS,AntiMode,"
            "LmtChg_V,LmtDisChg_V,LmtChg_A,LmtDisChg_A,"
            "Cell_Max_V,Cell_Min_V,Cell_Gap_V,Cell_Avg_V,"
            "PackNum(Max_V),CellNum(Max_V),PackNum(Min_V),CellNum(Min_V),"
            "Cell_Max_T,Cell_Min_T,Cell_Gap_T,Cell_Avg_T,"
            "PackNum(Max_T),PackNum(Min_T),"
            "RemChgkWh,RemDisChgkWh,"
            "SingleChrgCapacity, SingleDischrgCapacity,"
            "AccuChrgCapacity, AccuDischrgCapacity,"
            "AccuChrgEnergy, AccuDischrgEnergy,"
            "SingleChgkWh, SingleDischgkWh,"
            "Air_Status,Cab_Temp,AirCool_Temp,Cab_Humi,"

            "AuxMeter1_kW,AuxMeter3_kW,"
            "Rflx_kW,Ovl_kW," //防逆流表,防过载表

            // LiqColAirCondition
            "LCAC_Type,LCAC_WorkMode,LCAC_BmsSetWM,LCAC_CompState,LCAC_CompSpeed,"
            "LCAC_ColTemp,LCAC_ColOpenDiff,LCAC_ColStopDiff,LCAC_HeatTemp,LCAC_HeatOpenDiff,LCAC_HeatStopDiff,LCAC_XmSetTemp,"
	        "LCAC_AmbiT,LCAC_FeedT,LCAC_ReturnT,LCAC_ModFeedT,LCAC_ModReturnT,"
            "LCAC_ExPress,LCAC_FeedPress,LCAC_ReturnPress,LCAC_FaulCode,"

            /* //"PhsDiffDis,PhOvl_A,PhsMaxChrg," // 单相数据先不导出 */
            "SysT1,SysT2,SysT3,Demand,"
            "VpmOut,PCS_kVar,GridMeter_kVar,CabMeter_kVar,Ctrl_kVar,"
            "STS_Status,PCSSys#1|#2|#|#,BMSFail,Alm,LongAlm_info1-7,DIDO,"
            "BatEu3_DI,BatEu3_DO,BatEu3_NTC1|2|3|4,TemperatureSen,HumiditySen,H2,CO,Humidity,"
            "PowerEu3_DI,PowerEu3_DO,PowerEu3_NTC1|2|3|4"
            "\n");
    }
    fputs(szRecLine, fp);  

    UINT j;
    int Index = 0;
    for(n = 0; n < nCount; ++n)
    {
        sqlite3_stmt * stat1 = NULL;
        sprintf(szSql, "SELECT * FROM Data_RealTimeBmsData Where Log_Time = %d", tTimeSum[n]);
        rc = sqlite3_prepare_v2(db, szSql, -1, &stat1, NULL);
        if(rc != SQLITE_OK)
        {
            // printf("SQL 执行错误\n");
            continue;
        }
        if(sqlite3_step(stat1) == SQLITE_ROW)
        {
            int nRecSize = sqlite3_column_bytes(stat1, 3);
            int nCompressSize = sqlite3_sizeof_blob_uncompressed(stat1, 3);
            if(nCompressSize > 0)
            {
                HIS_BMS_RUNTIMEDATA_REC pBmsRunData_Rec;
                memset(&pBmsRunData_Rec, 0, sizeof(HIS_BMS_RUNTIMEDATA_REC));
                pBmsRunData_Rec.nDay = sqlite3_column_int(stat1, 1);
                pBmsRunData_Rec.tmLogTime = (time_t)sqlite3_column_int(stat1, 2);
                if (nCompressSize == sizeof(BMS_RUNTIMEDATA_LIST))
                {
                    sqlite3_parse_blob_uncompressed(stat1, 3, 
                        (void*)&pBmsRunData_Rec.bmsRunData, sizeof(BMS_RUNTIMEDATA_LIST));
                }
                else
                {
                    // printf("Data size mismatch: %d vs %d\n", nCompressSize, sizeof(BMS_RUNTIMEDATA_LIST));
                     sqlite3_finalize(stat1);
                    continue;
                }
                // printf("Export BmsRunData Day=%d, LogTime=%d, TotalRec=%d\n",
                //     pBmsRunData_Rec.nDay,
                //     pBmsRunData_Rec.tmLogTime,
                //     pBmsRunData_Rec.bmsRunData.nBmsRuntimeDataTotal);
                UINT Sum = MAX(0, pBmsRunData_Rec.bmsRunData.nBmsRuntimeDataTotal);
                for(j = 0; j < Sum; j++)
                {   
                    if(j >= MAX_HIS_BMS_RUNTIME_DATALIST)
                    {
                        break;
                    }
                    if(Make_BmsRunDataItem_2StrLine(pFCGI_Rough, &pBmsRunData_Rec, j, szRecLine, &Index) != NULL)
                    {
                        fputs(szRecLine, fp);
                    }
                }
            }
        }
        sqlite3_finalize(stat1);
    }
   
    sqlite3_close(db);

    // for(n = 0; n < nTotal; n++)
    // {        

    //     if(Make_BmsRunDataItem_2StrLine(pFCGI_Rough, pRec, n, szRecLine) != NULL)
    //     {
    //         fputs(szRecLine, fp);
    //     }
    // }
    fclose(fp);
    
    char tar_cmd[512];
    snprintf(tar_cmd, 512, "tar -zcf %s -C %s '%s'", CHGRUNDATA_EXPORT_TARFILE, "/opt/appdata/data", szBmsName);
    int tar_ret = _SYSTEM(tar_cmd);
    if(tar_ret != 0)
    {
        unlink(szBmsRunPath);
        unlink(CHGRUNDATA_EXPORT_TARFILE);
        Make_Web_OutPut(104, "Can NOT create tar file for export!", NULL, NULL);
        return FALSE;      
    }
  
    char szRemoteName[150];
    char szTemp[32];
    char szTemp1[32];
    snprintf(szRemoteName, 150, "%s_RunData_Day[%s]_%s.tar.gz", 
            pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
            TimeToString(Conv_UTC_2LocalTime(pCond->nGunIdx), TIME_CHN_FMT, szTemp, 32),
            TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp1, 32));
    
    if(!UploadFile_2Client(CHGRUNDATA_EXPORT_TARFILE, szRemoteName))
    {
        unlink(szBmsRunPath);
        unlink(CHGRUNDATA_EXPORT_TARFILE);
        Make_Web_OutPut(104, "Upload process is FAILED!", NULL, NULL);
        return FALSE;      
    }
    //printf("--- 555 ---\n");
    unlink(szBmsRunPath);
    unlink(CHGRUNDATA_EXPORT_TARFILE);
    
    return FALSE; //shoud return FALSE!!!
}

static BOOL Make_QueryBmsRecResult_BySubPage(FCGI_ROUGH_DATA* pFCGI_Rough, int cond_system, int bHasSortKey)
{
    WEB_BMSREC_DATA* pRec; 
    //now make the output
    int nTotalPage = 1;
    int nBmsRecTotal = pFCGI_Rough->chargerDataRepo.nBmsRecTotal;
    //now add general info    
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", nBmsRecTotal);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", nTotalPage);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", 1);
    
    cJSON* pJRecList = cJSON_CreateArray();

    TRACED("Make_QueryBmsRecResult_BySubPage nBmsRecTotal=%d, nPageId=%d, nPageVolume=%d\n",
        nBmsRecTotal, pCond->nPageId, pCond->nPageVolume);

    int n = 0;
    char szTemp[64];
  
    for(n = 0; n < nBmsRecTotal; n++)
    {
        pRec = &pFCGI_Rough->chargerDataRepo.bmsRecData[n];

        cJSON* pJ_ARec = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJ_ARec, "idx", n);

        char szId[32];
        memset(szId, '\0', 32);
        snprintf(szId, 32, "0-1-%d", pFCGI_Rough->chargerDataRepo.tTimeSum[n]);
        cJSON_AddStringToObject(pJ_ARec, "id", szId);

        //cJSON_AddNumberToObject(pJ_ARec, "sn", pRec->nDay);   
        cJSON_AddStringToObject(pJ_ARec, "date", TimeToString(Conv_UTC_2LocalTime(pFCGI_Rough->chargerDataRepo.tTimeSum[n]), TIME_CHN_FMT, szTemp, 32));

        char szRemoteName[150];
        char szTemp[32];
        char szTemp1[32];
        snprintf(szRemoteName, 150, "%s_RunData_Day[%s].tar.gz", 
                pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
                TimeToString(Conv_UTC_2LocalTime(pFCGI_Rough->chargerDataRepo.tTimeSum[n]), TIME_CHN_FMT, szTemp, 32));

        cJSON_AddStringToObject(pJ_ARec, "filename", szRemoteName); 
        cJSON_AddStringToObject(pJ_ARec, "sn", 
            TimeToString(Conv_UTC_2LocalTime(pFCGI_Rough->chargerDataRepo.tTimeSum[n]), TIME_CHN_FMT, szTemp, 32));
        cJSON_AddNumberToObject(pJ_ARec, "gun", pFCGI_Rough->chargerDataRepo.tTimeSum[n]); 
        cJSON_AddStringToObject(pJ_ARec, "stmeth",  "--");
        cJSON_AddStringToObject(pJ_ARec, "chgmode", "--");
    
        //cJSON_AddFloatToObject(pJ_ARec, "soc_start", 100, 0);
        //cJSON_AddFloatToObject(pJ_ARec, "soc_end", 80, 0);  
        
        cJSON_AddStringToObject(pJ_ARec, "tm_start", "--");
        
        cJSON_AddStringToObject(pJ_ARec, "kwh_used", "--");
        cJSON_AddStringToObject(pJ_ARec, "min_used", "--");
        cJSON_AddStringToObject(pJ_ARec, "stop_reason", "--");
        cJSON_AddStringToObject(pJ_ARec, "tm_end", "--");
    
        //add back the item
        cJSON_AddItemToArray(pJRecList, pJ_ARec);
    }
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
    return TRUE;
}

static BOOL Make_1_BySubPage(FCGI_ROUGH_DATA* pFCGI_Rough, int sub_id)
{
 
    cJSON* pJRecList = cJSON_CreateArray();
    int n = 0;
    char szTemp[32];
  
    cJSON* pJ_ARec = cJSON_CreateObject();
    cJSON_AddNumberToObject(pJ_ARec, "idx", n);
    switch(sub_id)
    {
        case 2:
            cJSON_AddStringToObject(pJ_ARec, "id", "0-2");
            cJSON_AddStringToObject(pJ_ARec, "filename", "factory_test.tar");
            break;
        case 3:
            cJSON_AddStringToObject(pJ_ARec, "id", "0-3");
            cJSON_AddStringToObject(pJ_ARec, "filename", "SingleRectData");
            break;
        case 4:
            cJSON_AddStringToObject(pJ_ARec, "id", "0-4");
            cJSON_AddStringToObject(pJ_ARec, "filename", "BranchPCSData");
            break;
        case 5:
            cJSON_AddStringToObject(pJ_ARec, "id", "0-5");
            cJSON_AddStringToObject(pJ_ARec, "filename", "BranchPwrCalcData");
            break;
        case 6:
            cJSON_AddStringToObject(pJ_ARec, "id", "0-6");
            cJSON_AddStringToObject(pJ_ARec, "filename", "PCSGrpData");
            break;
        case 7:
            cJSON_AddStringToObject(pJ_ARec, "id", "0-7");
            cJSON_AddStringToObject(pJ_ARec, "filename", "SingleBattData");
            break;
        case 8:
            cJSON_AddStringToObject(pJ_ARec, "id", "0-8");
            cJSON_AddStringToObject(pJ_ARec, "filename", "SingleIfyBMSData");
            break;
        case 9:
            cJSON_AddStringToObject(pJ_ARec, "id", "0-9");
            cJSON_AddStringToObject(pJ_ARec, "filename", "EMSMgmtPlanData");
            break;
        case 10:
            cJSON_AddStringToObject(pJ_ARec, "id", "0-10");
            cJSON_AddStringToObject(pJ_ARec, "filename", "SingleCellData");
            break;
        case 11:
            cJSON_AddStringToObject(pJ_ARec, "id", "0-11");
            cJSON_AddStringToObject(pJ_ARec, "filename", "CCU2PCS_CanMsg");
            break;
        case 12:
            cJSON_AddStringToObject(pJ_ARec, "id", "0-12");
            cJSON_AddStringToObject(pJ_ARec, "filename", "Meter_Log");
            break;
        default:
            return FALSE;
    }

    cJSON_AddStringToObject(pJ_ARec, "date", TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
     
    cJSON_AddItemToArray(pJRecList, pJ_ARec);
    
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
    return TRUE;
}

static BOOL Make_2_BySubPage(FCGI_ROUGH_DATA* pFCGI_Rough, int sub_id)
{
 
    cJSON* pJRecList = cJSON_CreateArray();
    int n = 0;
    char szTemp[32];
  
    cJSON* pJ_ARec = cJSON_CreateObject();
    cJSON_AddNumberToObject(pJ_ARec, "idx", n);
    switch(sub_id)
    {
        case 1:
            cJSON_AddStringToObject(pJ_ARec, "id", "1-1");
            cJSON_AddStringToObject(pJ_ARec, "filename", "HisStatistic_KWH");
            break;
        case 2:
            cJSON_AddStringToObject(pJ_ARec, "id", "1-2");
            cJSON_AddStringToObject(pJ_ARec, "filename", "Bms_CanMsg");
            break;
        case 3:
            cJSON_AddStringToObject(pJ_ARec, "id", "1-3");
            cJSON_AddStringToObject(pJ_ARec, "filename", "AfeSrStatus_Data");
            break;
        case 4:
            cJSON_AddStringToObject(pJ_ARec, "id", "1-4");
            cJSON_AddStringToObject(pJ_ARec, "filename", "BMS_SOC_Calibration_Data");
            break;
        case 5:
            cJSON_AddStringToObject(pJ_ARec, "id", "1-5");
            cJSON_AddStringToObject(pJ_ARec, "filename", "bmsAlmHisLog");
            break;
        default:
            cJSON_AddStringToObject(pJ_ARec, "id", "1-1");
            cJSON_AddStringToObject(pJ_ARec, "filename", "HisStatistic_KWH");
            break;
    }

    cJSON_AddStringToObject(pJ_ARec, "date", TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
     
    cJSON_AddItemToArray(pJRecList, pJ_ARec);
    
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
    return TRUE;
}

static BOOL Make_DODHistory_BySubPage(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    sqlite3 *db;
    int rc = sqlite3_open_v2("/data/BmsRunData.data", &db, SQLITE_OPEN_READONLY, NULL); //has to be read-only
    if(rc != SQLITE_OK)
    {
        TRACE("----can NOT open BmsRunData db for DOD history---\n");
        Make_Web_OutPut(104, "Can NOT open BMS data db!", NULL, NULL);
        return FALSE;
    }
    sqlite3_busy_timeout(db, 5000);

    const char* szSql =
        "SELECT Idx, Time_Year, Time_MonthDay, Time_HourMinute, Time_Second "
        "FROM Data_DODHistory ORDER BY Idx";

    sqlite3_stmt* stat;
    sqlite3_prepare_v2(db, szSql, -1, &stat, NULL);

    int nTotal = 0;
    while(sqlite3_step(stat) == SQLITE_ROW)
    {
        nTotal++;
        if(nTotal >= MAX_BMS_RECORDS_NUM)
            break;
    }
    sqlite3_finalize(stat);

    cJSON* pJRecList = cJSON_CreateArray();
    char szTemp[64];
    int n = 0;

    sqlite3_prepare_v2(db, szSql, -1, &stat, NULL);
    while(sqlite3_step(stat) == SQLITE_ROW)
    {
        if(n >= MAX_BMS_RECORDS_NUM)
            break;

        int idx        = sqlite3_column_int(stat, 0);
        int year       = sqlite3_column_int(stat, 1);
        int monthday   = sqlite3_column_int(stat, 2);
        int hourminute = sqlite3_column_int(stat, 3);
        int second     = sqlite3_column_int(stat, 4);

        cJSON* pJ_ARec = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJ_ARec, "idx", n);

        char szId[32];
        memset(szId, '\0', 32);
        snprintf(szId, 32, "1-6-%d", idx);
        cJSON_AddStringToObject(pJ_ARec, "id", szId);

        snprintf(szTemp, 64, "%04d-%02d-%02d %02d:%02d:%02d",
                 year, (monthday >> 8) & 0xFF, monthday & 0xFF,
                 (hourminute >> 8) & 0xFF, hourminute & 0xFF, second);
        cJSON_AddStringToObject(pJ_ARec, "date", szTemp);

        cJSON_AddStringToObject(pJ_ARec, "filename", "DOD_History");

        cJSON_AddItemToArray(pJRecList, pJ_ARec);
        n++;
    }
    sqlite3_finalize(stat);
    sqlite3_close(db);

    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", nTotal);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", 1);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", 1);
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
    return TRUE;
}

static BOOL Make_SOSHistory_BySubPage(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    sqlite3 *db;
    int rc = sqlite3_open_v2("/data/BmsRunData.data", &db, SQLITE_OPEN_READONLY, NULL); //has to be read-only
    if(rc != SQLITE_OK)
    {
        TRACE("----can NOT open BmsRunData db for SOS history---\n");
        Make_Web_OutPut(104, "Can NOT open BMS data db!", NULL, NULL);
        return FALSE;
    }
    sqlite3_busy_timeout(db, 5000);

    const char* szSql =
        "SELECT Idx, Time_Year, Time_MonthDay, Time_HourMinute, Time_Second "
        "FROM Data_SOSHistory ORDER BY Idx";

    sqlite3_stmt* stat;
    sqlite3_prepare_v2(db, szSql, -1, &stat, NULL);

    int nTotal = 0;
    while(sqlite3_step(stat) == SQLITE_ROW)
    {
        nTotal++;
        if(nTotal >= MAX_BMS_RECORDS_NUM)
            break;
    }
    sqlite3_finalize(stat);

    cJSON* pJRecList = cJSON_CreateArray();
    char szTemp[64];
    int n = 0;

    sqlite3_prepare_v2(db, szSql, -1, &stat, NULL);
    while(sqlite3_step(stat) == SQLITE_ROW)
    {
        if(n >= MAX_BMS_RECORDS_NUM)
            break;

        int idx        = sqlite3_column_int(stat, 0);
        int year       = sqlite3_column_int(stat, 1);
        int monthday   = sqlite3_column_int(stat, 2);
        int hourminute = sqlite3_column_int(stat, 3);
        int second     = sqlite3_column_int(stat, 4);

        cJSON* pJ_ARec = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJ_ARec, "idx", n);

        char szId[32];
        memset(szId, '\0', 32);
        snprintf(szId, 32, "1-7-%d", idx);
        cJSON_AddStringToObject(pJ_ARec, "id", szId);

        snprintf(szTemp, 64, "%04d-%02d-%02d %02d:%02u:%02d",
                 year, (monthday >> 8) & 0xFF, monthday & 0xFF,
                 (hourminute >> 8) & 0xFF, hourminute & 0xFF, second);
        cJSON_AddStringToObject(pJ_ARec, "date", szTemp);

        cJSON_AddStringToObject(pJ_ARec, "filename", "SOS_History");

        cJSON_AddItemToArray(pJRecList, pJ_ARec);
        n++;
    }
    sqlite3_finalize(stat);
    sqlite3_close(db);

    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", nTotal);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", 1);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", 1);
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
    return TRUE;
}


static BOOL Make_BMS_BySubPage(FCGI_ROUGH_DATA* pFCGI_Rough, int Main_id, int Sub_id)
{
    switch(Main_id)
    {
        case 0: 
            switch(Sub_id)
            {
                case 0: 
                    return  Make_QueryBmsRecResult_BySubPage(pFCGI_Rough, Main_id, Sub_id);
                    break;
                case 1: 
                    return  Make_QueryBmsRecResult_BySubPage(pFCGI_Rough, Main_id, Sub_id);
                    break;
                case 2: 
                    return Make_1_BySubPage(pFCGI_Rough, Sub_id);
                    break;
                default:
                    return Make_1_BySubPage(pFCGI_Rough, Sub_id);
                    break;
            }
           
        case 1: 
            if(Sub_id == 6)
                return Make_DODHistory_BySubPage(pFCGI_Rough);
            if(Sub_id == 7)
                return Make_SOSHistory_BySubPage(pFCGI_Rough);
            return Make_2_BySubPage(pFCGI_Rough, Sub_id);
            break;
        default:
            break;
    }

    return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Make_Export_StatisticKWH_andUpload()
*  CREATOR       : 
*  CREATED DATE  : 2022-01-18 17:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:    
*  
***********************************************************************************/
#define MAX_SPRINTF_LINE_LEN	512


static void WriteHeader2Table(FCGI_ROUGH_DATA* pFCGI_Rough, FILE *fp, HISDATA_TABLETYPE emTableType)
{
	char szTableName[][20]={{"Hour Statistic KWH"}, {"Daily Statistic KWH"}, {"Yearly Statistic Kwh"}};
    char szFileTitle[MAX_SPRINTF_LINE_LEN];
    memset(szFileTitle, '\0', MAX_SPRINTF_LINE_LEN);
    UINT nFileLen = 0;

	//2 new a table for Chagrge log
	if(emTableType == TP_HISDATA_WEEKHOURS)
	{
		nFileLen = snprintf(szFileTitle, MAX_SPRINTF_LINE_LEN,"\n%30s,\n", szTableName[emTableType]);
        
        snprintf(szFileTitle + nFileLen, MAX_SPRINTF_LINE_LEN,"%s,\n",\
		    "Date,Time,User ACLoad(Used),User ACMeter IN,User ACMeter OUT,Cab ACMeter IN, Cab ACMeter OUT,PV#1,PV#2,PV#3,PV#4,BAT#1 Chrg,BAT#1 Dischrg,"
		    "BAT#2 Chrg,BAT#2 Dischrg,BAT#3 Chrg,BAT#3 Dischrg,BAT#4 Chrg,BAT#4 Dischrg,1KV DCLoad(Used)KWH,User ACMeter RealTime,");
    }
	else if((emTableType == TP_HISDATA_DAILY) || (emTableType == TP_HISDATA_YEARLY))
	{
	 	nFileLen = snprintf(szFileTitle, MAX_SPRINTF_LINE_LEN,"\n%30s,\n", szTableName[emTableType]);
         
        snprintf(szFileTitle + nFileLen, MAX_SPRINTF_LINE_LEN,"%s,\n",\
		    "Date,User ACLoad(Used),User ACMeter IN,User ACMeter OUT,Cab ACMeter IN,Cab ACMeter OUT,PV#1,PV#2,PV#3,PV#4,"
            "IN Cab ACMeter Sharp,IN Cab ACMeter Peak,IN Cab ACMeter Flat,IN Cab ACMeter Valley,"
		    "OUT Cab ACMeter Sharp,OUT Cab ACMeter Peak,OUT Cab ACMeter Flat,OUT Cab ACMeter Valley,"
            "BAT#1 Chrg, BAT#1 Dischrg,BAT#2 Chrg,BAT#2 Dischrg,BAT#3 Chrg,BAT#3 Dischrg,BAT#4 Chrg,BAT#4 Dischrg,1KV DCLoad(Used)KWH,User ACMeter RealTime,");
 	}

    if(fp != NULL)
    {
        fputs(szFileTitle, fp);
    }
	//TRACE("Write Content To Table szTableName[%d]=%s\n", emTableType, szTableName[emTableType]);
	return ;
}
 
static void WriteContent2Table(FCGI_ROUGH_DATA* pFCGI_Rough, FILE *fp, HISDATA_TABLETYPE emTableType)
{
	VAR_VALUE 	varV;

	char szTime[32]={0};
	char szFileTitle[MAX_SPRINTF_LINE_LEN];
	memset(szFileTitle, 0, sizeof(szFileTitle));
	
    //----- Step1.Get ES Statistic History Data -----//
    A_BLOB_ES_STATISTIC_DATA    *pHisStatKwh;
    WEB_PARAM_VALUE             *pWebParaVal;
    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 200, 79)) != NULL)
    {
        if(pWebParaVal->pParamInfo->nValueSize == sizeof(A_BLOB_ES_STATISTIC_DATA))
        {
            pHisStatKwh = (A_BLOB_ES_STATISTIC_DATA*)pWebParaVal->currValue.blobValue;
        }
    }

    //----- Step2. Write Content Data To fp -----//
 	int n = 0, m = 0;
	int nDailyCount  = 0 ;
	int nYearlyCount = 0;

	int nHourSetor = 0,         nMinSector = 0;
    int nHourAdd1_Sector = 0,   nMinAdd1_Sector = 0;

	if(emTableType == TP_HISDATA_WEEKHOURS)
	{
        for(m = 0; m < MAX_HOURS_SECTOR; m++)
        {
            time_t saveTime = pHisStatKwh->es_Data.tmTimeSave;
			if(saveTime != 0)
			{
				TimeToString(saveTime, "%Y-%m-%d ", szTime, sizeof(szTime));		

				//TRACE("n=%d m=%d szTime=%s nStepCount=%d nFileLen=%d\n", n, m, szTime, nStepCount, nFileLen);
				nHourSetor = m/2; 
				nMinSector = m%2;
    			
				nHourAdd1_Sector = (m+1)/2; 
				nMinAdd1_Sector = (m+1)%2;

				snprintf(szFileTitle, MAX_SPRINTF_LINE_LEN, \
                "%s,%02d:%02d~%02d:%02d,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,\n", \
				szTime,\
				nHourSetor,\
				nMinSector*30,\
				nHourAdd1_Sector,\
				nMinAdd1_Sector*30,\
				pHisStatKwh->es_Data.hours_Data.fOrgACLoad_CostKWH_Sector[m],\
 				pHisStatKwh->es_Data.hours_Data.fAC_CostKWH_Sector[m],\
				 pHisStatKwh->es_Data.hours_Data.fAC_RevrtOutPutKWH_Sector[m],\
				pHisStatKwh->es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[m],\
				pHisStatKwh->es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[m],\
				pHisStatKwh->es_Data.hours_Data.fMppt_GKWH_Sector[0][m],\
				pHisStatKwh->es_Data.hours_Data.fMppt_GKWH_Sector[1][m],\
				pHisStatKwh->es_Data.hours_Data.fMppt_GKWH_Sector[2][m],\
				pHisStatKwh->es_Data.hours_Data.fMppt_GKWH_Sector[3][m],\
				pHisStatKwh->es_Data.hours_Data.fBatt_ChrgKWH_Sector[0][m],\
				pHisStatKwh->es_Data.hours_Data.fBatt_DisChrgKWH_Sector[0][m],\
				pHisStatKwh->es_Data.hours_Data.fBatt_ChrgKWH_Sector[1][m],\
				pHisStatKwh->es_Data.hours_Data.fBatt_DisChrgKWH_Sector[1][m],\
				pHisStatKwh->es_Data.hours_Data.fBatt_ChrgKWH_Sector[2][m],\
				pHisStatKwh->es_Data.hours_Data.fBatt_DisChrgKWH_Sector[2][m],\
				pHisStatKwh->es_Data.hours_Data.fBatt_ChrgKWH_Sector[3][m],\
				pHisStatKwh->es_Data.hours_Data.fBatt_DisChrgKWH_Sector[3][m],\
				pHisStatKwh->es_Data.hours_Data.fDCLoad_CostKWH_Sector[m],\					
				pHisStatKwh->es_Data.hours_Data.fAC_CurrKWH_Reading[m]);
	
                fputs(szFileTitle, fp);
				memset(szFileTitle, 0, sizeof(szFileTitle));
			}
		}
 	}
	else if(emTableType == TP_HISDATA_DAILY)
	{
		for(n = 0; n < MAX_DAYS_SECTOR; n++)
        {
            time_t saveTime = pHisStatKwh->es_Data.days_Data.tmTimeStamp_D[n];
			if(saveTime != 0)
			{
				TimeToString(saveTime, TIME_CHN_FMT, szTime, sizeof(szTime));

				//TRACE("n=%d m=%d szTime=%s nDailyCount=%d nStepCount=%d nFileLen=%d\n",  n, m, szTime, nDailyCount, nStepCount, nFileLen);

				snprintf(szFileTitle, MAX_SPRINTF_LINE_LEN, \
                "%s,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,\n", \
				szTime,\
				pHisStatKwh->es_Data.days_Data.fOrgACLoad_CostKWH_D[n],\
 				pHisStatKwh->es_Data.days_Data.fACTotalCostKWH_D[n],\
				pHisStatKwh->es_Data.days_Data.fAC_RevrtOutPutKWH_D[n],\
				pHisStatKwh->es_Data.days_Data.fLocalAC_INPUTKWH_D[n],\
				pHisStatKwh->es_Data.days_Data.fLocalAC_OUTPUTKWH_D[n],\
				pHisStatKwh->es_Data.days_Data.fMpptTotalGKWH_D[0][n],\
				pHisStatKwh->es_Data.days_Data.fMpptTotalGKWH_D[1][n],\
				pHisStatKwh->es_Data.days_Data.fMpptTotalGKWH_D[2][n],\
				pHisStatKwh->es_Data.days_Data.fMpptTotalGKWH_D[3][n],\
				pHisStatKwh->es_Data.days_Data.fLocalAC_Sharp_InputKWH_D[n],\
				pHisStatKwh->es_Data.days_Data.fLocalAC_Peak_InputKWH_D[n],\
				pHisStatKwh->es_Data.days_Data.fLocalAC_Flat_InputKWH_D[n],\
				pHisStatKwh->es_Data.days_Data.fLocalAC_Valley_InputKWH_D[n],\
				pHisStatKwh->es_Data.days_Data.fLocalAC_Sharp_OutPutKWH_D[n],\
				pHisStatKwh->es_Data.days_Data.fLocalAC_Peak_OutPutKWH_D[n],\
				pHisStatKwh->es_Data.days_Data.fLocalAC_Flat_OutputKWH_D[n],\
				pHisStatKwh->es_Data.days_Data.fLocalAC_Valley_OutputKWH_D[n],\
				pHisStatKwh->es_Data.days_Data.fBattTotalChrgKWH_D[0][n],\
				pHisStatKwh->es_Data.days_Data.fBattTotalDisChrgKWH_D[0][n],\
				pHisStatKwh->es_Data.days_Data.fBattTotalChrgKWH_D[1][n],\
				pHisStatKwh->es_Data.days_Data.fBattTotalDisChrgKWH_D[1][n],\
				pHisStatKwh->es_Data.days_Data.fBattTotalChrgKWH_D[2][n],\
				pHisStatKwh->es_Data.days_Data.fBattTotalDisChrgKWH_D[2][n],\
				pHisStatKwh->es_Data.days_Data.fBattTotalChrgKWH_D[3][n],\
				pHisStatKwh->es_Data.days_Data.fBattTotalDisChrgKWH_D[3][n],\
				pHisStatKwh->es_Data.days_Data.fDCLoad_CostKWH_D[n],\
				pHisStatKwh->es_Data.days_Data.fAC_CurrKWH_Reading[n]);

                fputs(szFileTitle, fp);
				memset(szFileTitle, 0, sizeof(szFileTitle));	
			}	
		}
	}
	else if(emTableType == TP_HISDATA_YEARLY)
	{
		for(n = 0; n < MAX_YEARS_SECTOR; n++)
        {
            time_t saveTime = pHisStatKwh->es_Data.years_Data.tmTimeStamp_Y[n];
			if(saveTime != 0)
			{
				TimeToString(saveTime, TIME_CHN_FMT, szTime, sizeof(szTime));
				
				//TRACE("n=%d m=%d szTime=%s nYearlyCount=%d nStepCount=%d nFileLen=%d\n",  n,m, szTime, nYearlyCount, nStepCount, nFileLen);

				snprintf(szFileTitle, MAX_SPRINTF_LINE_LEN, "%s,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,\n", \
				szTime,\
				pHisStatKwh->es_Data.years_Data.fOrgACLoad_CostKWH_Y[n],\
 				pHisStatKwh->es_Data.years_Data.fACTotalCostKWH_Y[n],\
				pHisStatKwh->es_Data.years_Data.fAC_RevrtOutPutKWH_Y[n],\
				pHisStatKwh->es_Data.years_Data.fLocalAC_INPUTKWH_Y[n],\
				pHisStatKwh->es_Data.years_Data.fLocalAC_OUTPUTKWH_Y[n],\
				pHisStatKwh->es_Data.years_Data.fMpptTotalGKWH_Y[0][n],\
				pHisStatKwh->es_Data.years_Data.fMpptTotalGKWH_Y[1][n],\
				pHisStatKwh->es_Data.years_Data.fMpptTotalGKWH_Y[2][n],\
				pHisStatKwh->es_Data.years_Data.fMpptTotalGKWH_Y[3][n],\
				pHisStatKwh->es_Data.years_Data.fLocalAC_Sharp_InputKWH_Y[n],\
				pHisStatKwh->es_Data.years_Data.fLocalAC_Peak_InputKWH_Y[n],\
				pHisStatKwh->es_Data.years_Data.fLocalAC_Flat_InputKWH_Y[n],\
				pHisStatKwh->es_Data.years_Data.fLocalAC_Valley_InputKWH_Y[n],\
				pHisStatKwh->es_Data.years_Data.fLocalAC_Sharp_OutPutKWH_Y[n],\
				pHisStatKwh->es_Data.years_Data.fLocalAC_Peak_OutPutKWH_Y[n],\
				pHisStatKwh->es_Data.years_Data.fLocalAC_Flat_OutputKWH_Y[n],\
				pHisStatKwh->es_Data.years_Data.fLocalAC_Valley_OutputKWH_Y[n],\
				pHisStatKwh->es_Data.years_Data.fBattTotalChrgKWH_Y[0][n],\
				pHisStatKwh->es_Data.years_Data.fBattTotalDisChrgKWH_Y[0][n],\
				pHisStatKwh->es_Data.years_Data.fBattTotalChrgKWH_Y[1][n],\
				pHisStatKwh->es_Data.years_Data.fBattTotalDisChrgKWH_Y[1][n],\
				pHisStatKwh->es_Data.years_Data.fBattTotalChrgKWH_Y[2][n],\
				pHisStatKwh->es_Data.years_Data.fBattTotalDisChrgKWH_Y[2][n],\
				pHisStatKwh->es_Data.years_Data.fBattTotalChrgKWH_Y[3][n],\
				pHisStatKwh->es_Data.years_Data.fBattTotalDisChrgKWH_Y[3][n],\
				pHisStatKwh->es_Data.years_Data.fDCLoad_CostKWH_Y[n],\	
				pHisStatKwh->es_Data.years_Data.fAC_CurrKWH_Reading[n]);
	
                fputs(szFileTitle, fp);
				memset(szFileTitle, 0, sizeof(szFileTitle));
			}	
		}
		TRACE("---Write Yearly Data---\n");
	}
	return;
}

#define MAX_SINGLE_STRLEN 128
static char* Make_BmsHealthItem_2StrLine(FCGI_ROUGH_DATA* pFCGI_Rough, UINT nIdx, UINT nBattIdx, UINT nLineIdx, OUT char* pOutStr)
{
    HIS_HEALTH_STATIC_DAILY* pRec = &pFCGI_Rough->chargerDataRepo.healthRec[nIdx]; 

	A_BMS_HEALTH_ITEM	*pItem = &pRec->bmsHealthData.healthItem[nBattIdx];

    *pOutStr = 0;
    char szIdx[8];
    char szTime[32]={"0"};
    char szTTTT[MAX_SINGLE_STRLEN];

     //id
    snprintf(szIdx, 8, "%d,", nLineIdx);
    strcat(pOutStr, szIdx);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%s,%d,%.1f,", 
        TimeToString(pRec->tmLogTime, "%Y-%m-%d", szTime, sizeof(szTime)), nBattIdx, pItem->fRack_TotalCapacity);
    strcat(pOutStr, szTTTT);

    //"Id,Date,BMSIdx,Cap(AH),"
    //"Key_Cell_ChgDif_U(V),TM,Max_V,Min_V,Min_V,U_Max_CSC,U_Max_Cell,U_Min_CSC,U_Min_CSC,Max_T,Min_T,Avg_T,"
    //"T_Max_CSC,T_Max_Cell,T_Min_CSC,T_Min_Cell,Chg_ON_T_Max,Chg_ON_T_Min,Dischg_ON_T_Max,Dischg_ON_T_Min,I(A),SOC,|,"

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.3f,%s,%.3f,%.3f,%.3f,%d,%d,%d,%d,%.3f,%.3f,%.3f,%d,%d,%d,%d,", 
        pItem->fKey_Chg_TmsvDif_CellVolt,
        TimeToString(pItem->chgTmsvDifCellVolt.tmTimeStamp, "%H:%M:%S", szTime, sizeof(szTime)),
        pItem->chgTmsvDifCellVolt.fMax_CellVolt,
        pItem->chgTmsvDifCellVolt.fMin_CellVolt,
        pItem->chgTmsvDifCellVolt.fAvg_CellVolt,
	    pItem->chgTmsvDifCellVolt.uiBAT_Ucell_Max_CSC,
        pItem->chgTmsvDifCellVolt.uiBAT_Ucell_Max_CELL, 
        pItem->chgTmsvDifCellVolt.uiBAT_Ucell_Min_CSC, 
        pItem->chgTmsvDifCellVolt.uiBAT_Ucell_Min_CELL, 
	    pItem->chgTmsvDifCellVolt.fMax_CellTemp,
        pItem->chgTmsvDifCellVolt.fMin_CellTemp,
        pItem->chgTmsvDifCellVolt.fAvg_CellTemp,
	    pItem->chgTmsvDifCellVolt.uiBAT_T_Max_CSC,
	    pItem->chgTmsvDifCellVolt.uiBAT_T_Max_CELL,
	    pItem->chgTmsvDifCellVolt.uiBAT_T_Min_CSC,
	    pItem->chgTmsvDifCellVolt.uiBAT_T_Min_CELL);
    strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.1f,%.1f,%.1f,%.1f,%.1f,%d,,", 
        pItem->chgTmsvDifCellVolt.fChg_Start_MaxCellTemp,
        pItem->chgTmsvDifCellVolt.fChg_Start_MinCellTemp,
        pItem->chgTmsvDifCellVolt.fDischg_Start_MaxCellTemp,
        pItem->chgTmsvDifCellVolt.fDischg_Start_MinCellTemp,
        pItem->chgTmsvDifCellVolt.fPackTotalCurr,
        pItem->chgTmsvDifCellVolt.bySOC);
    strcat(pOutStr, szTTTT);

    //--- CellTemp TmsDiff ---//
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.3f,%s,%.3f,%.3f,%.3f,%d,%d,%d,%d,%.3f,%.3f,%.3f,%d,%d,%d,%d,", 
        pItem->fKey_Chg_TmsvDif_CellTemp,
        TimeToString(pItem->chgTmsvDifCellTemp.tmTimeStamp, "%H:%M:%S", szTime, sizeof(szTime)),
        pItem->chgTmsvDifCellTemp.fMax_CellVolt,
        pItem->chgTmsvDifCellTemp.fMin_CellVolt,
        pItem->chgTmsvDifCellTemp.fAvg_CellVolt,
	    pItem->chgTmsvDifCellTemp.uiBAT_Ucell_Max_CSC,
        pItem->chgTmsvDifCellTemp.uiBAT_Ucell_Max_CELL, 
        pItem->chgTmsvDifCellTemp.uiBAT_Ucell_Min_CSC, 
        pItem->chgTmsvDifCellTemp.uiBAT_Ucell_Min_CELL, 
	    pItem->chgTmsvDifCellTemp.fMax_CellTemp,
        pItem->chgTmsvDifCellTemp.fMin_CellTemp,
        pItem->chgTmsvDifCellTemp.fAvg_CellTemp,
	    pItem->chgTmsvDifCellTemp.uiBAT_T_Max_CSC,
	    pItem->chgTmsvDifCellTemp.uiBAT_T_Max_CELL,
	    pItem->chgTmsvDifCellTemp.uiBAT_T_Min_CSC,
	    pItem->chgTmsvDifCellTemp.uiBAT_T_Min_CELL);
    strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.1f,%.1f,%.1f,%.1f,%.1f,%d,,", 
        pItem->chgTmsvDifCellTemp.fChg_Start_MaxCellTemp,
        pItem->chgTmsvDifCellTemp.fChg_Start_MinCellTemp,
        pItem->chgTmsvDifCellTemp.fDischg_Start_MaxCellTemp,
        pItem->chgTmsvDifCellTemp.fDischg_Start_MinCellTemp,
        pItem->chgTmsvDifCellTemp.fPackTotalCurr,
        pItem->chgTmsvDifCellTemp.bySOC);
    strcat(pOutStr, szTTTT);

    //--- Max_Temp ---//
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.3f,%s,%.3f,%.3f,%.3f,%d,%d,%d,%d,%.3f,%.3f,%.3f,%d,%d,%d,%d,", 
        pItem->fKey_Chg_Max_CellTemp,
        TimeToString(pItem->chgMaxCellTemp.tmTimeStamp, "%H:%M:%S", szTime, sizeof(szTime)),
        pItem->chgMaxCellTemp.fMax_CellVolt,
        pItem->chgMaxCellTemp.fMin_CellVolt,
        pItem->chgMaxCellTemp.fAvg_CellVolt,
	    pItem->chgMaxCellTemp.uiBAT_Ucell_Max_CSC,
        pItem->chgMaxCellTemp.uiBAT_Ucell_Max_CELL, 
        pItem->chgMaxCellTemp.uiBAT_Ucell_Min_CSC, 
        pItem->chgMaxCellTemp.uiBAT_Ucell_Min_CELL, 
	    pItem->chgMaxCellTemp.fMax_CellTemp,
        pItem->chgMaxCellTemp.fMin_CellTemp,
        pItem->chgMaxCellTemp.fAvg_CellTemp,
	    pItem->chgMaxCellTemp.uiBAT_T_Max_CSC,
	    pItem->chgMaxCellTemp.uiBAT_T_Max_CELL,
	    pItem->chgMaxCellTemp.uiBAT_T_Min_CSC,
	    pItem->chgMaxCellTemp.uiBAT_T_Min_CELL);
    strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.1f,%.1f,%.1f,%.1f,%.1f,%d,,", 
        pItem->chgMaxCellTemp.fChg_Start_MaxCellTemp,
        pItem->chgMaxCellTemp.fChg_Start_MinCellTemp,
        pItem->chgMaxCellTemp.fDischg_Start_MaxCellTemp,
        pItem->chgMaxCellTemp.fDischg_Start_MinCellTemp,
        pItem->chgMaxCellTemp.fPackTotalCurr,
        pItem->chgMaxCellTemp.bySOC);
    strcat(pOutStr, szTTTT);

    //--- Min_Temp ---//
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.3f,%s,%.3f,%.3f,%.3f,%d,%d,%d,%d,%.3f,%.3f,%.3f,%d,%d,%d,%d,", 
        pItem->fKey_Chg_Min_CellTemp,
        TimeToString(pItem->chgMinCellTemp.tmTimeStamp, "%H:%M:%S", szTime, sizeof(szTime)),
        pItem->chgMinCellTemp.fMax_CellVolt,
        pItem->chgMinCellTemp.fMin_CellVolt,
        pItem->chgMinCellTemp.fAvg_CellVolt,
	    pItem->chgMinCellTemp.uiBAT_Ucell_Max_CSC,
        pItem->chgMinCellTemp.uiBAT_Ucell_Max_CELL, 
        pItem->chgMinCellTemp.uiBAT_Ucell_Min_CSC, 
        pItem->chgMinCellTemp.uiBAT_Ucell_Min_CELL, 
	    pItem->chgMinCellTemp.fMax_CellTemp,
        pItem->chgMinCellTemp.fMin_CellTemp,
        pItem->chgMinCellTemp.fAvg_CellTemp,
	    pItem->chgMinCellTemp.uiBAT_T_Max_CSC,
	    pItem->chgMinCellTemp.uiBAT_T_Max_CELL,
	    pItem->chgMinCellTemp.uiBAT_T_Min_CSC,
	    pItem->chgMinCellTemp.uiBAT_T_Min_CELL);
    strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.1f,%.1f,%.1f,%.1f,%.1f,%d,,", 
        pItem->chgMinCellTemp.fChg_Start_MaxCellTemp,
        pItem->chgMinCellTemp.fChg_Start_MinCellTemp,
        pItem->chgMinCellTemp.fDischg_Start_MaxCellTemp,
        pItem->chgMinCellTemp.fDischg_Start_MinCellTemp,
        pItem->chgMinCellTemp.fPackTotalCurr,
        pItem->chgMinCellTemp.bySOC);
    strcat(pOutStr, szTTTT);

    //--- Dischg CellTemp TmsDiff ---//

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.3f,%s,%.3f,%.3f,%.3f,%d,%d,%d,%d,%.3f,%.3f,%.3f,%d,%d,%d,%d,", 
        pItem->fKey_Dischg_TmsvDif_CellVolt,
        TimeToString(pItem->dischgTmsvDifCellVolt.tmTimeStamp, "%H:%M:%S", szTime, sizeof(szTime)),
        pItem->dischgTmsvDifCellVolt.fMax_CellVolt,
        pItem->dischgTmsvDifCellVolt.fMin_CellVolt,
        pItem->dischgTmsvDifCellVolt.fAvg_CellVolt,
	    pItem->dischgTmsvDifCellVolt.uiBAT_Ucell_Max_CSC,
        pItem->dischgTmsvDifCellVolt.uiBAT_Ucell_Max_CELL, 
        pItem->dischgTmsvDifCellVolt.uiBAT_Ucell_Min_CSC, 
        pItem->dischgTmsvDifCellVolt.uiBAT_Ucell_Min_CELL, 
	    pItem->dischgTmsvDifCellVolt.fMax_CellTemp,
        pItem->dischgTmsvDifCellVolt.fMin_CellTemp,
        pItem->dischgTmsvDifCellVolt.fAvg_CellTemp,
	    pItem->dischgTmsvDifCellVolt.uiBAT_T_Max_CSC,
	    pItem->dischgTmsvDifCellVolt.uiBAT_T_Max_CELL,
	    pItem->dischgTmsvDifCellVolt.uiBAT_T_Min_CSC,
	    pItem->dischgTmsvDifCellVolt.uiBAT_T_Min_CELL);
    strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.1f,%.1f,%.1f,%.1f,%.1f,%d,,", 
        pItem->dischgTmsvDifCellVolt.fChg_Start_MaxCellTemp,
        pItem->dischgTmsvDifCellVolt.fChg_Start_MinCellTemp,
        pItem->dischgTmsvDifCellVolt.fDischg_Start_MaxCellTemp,
        pItem->dischgTmsvDifCellVolt.fDischg_Start_MinCellTemp,
        pItem->dischgTmsvDifCellVolt.fPackTotalCurr,
        pItem->dischgTmsvDifCellVolt.bySOC);
    strcat(pOutStr, szTTTT);

    //--- CellTemp TmsDiff ---//
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.3f,%s,%.3f,%.3f,%.3f,%d,%d,%d,%d,%.3f,%.3f,%.3f,%d,%d,%d,%d,", 
        pItem->fKey_Dischg_TmsvDif_CellTemp,
        TimeToString(pItem->dischgTmsvDifCellTemp.tmTimeStamp, "%H:%M:%S", szTime, sizeof(szTime)),
        pItem->dischgTmsvDifCellTemp.fMax_CellVolt,
        pItem->dischgTmsvDifCellTemp.fMin_CellVolt,
        pItem->dischgTmsvDifCellTemp.fAvg_CellVolt,
	    pItem->dischgTmsvDifCellTemp.uiBAT_Ucell_Max_CSC,
        pItem->dischgTmsvDifCellTemp.uiBAT_Ucell_Max_CELL, 
        pItem->dischgTmsvDifCellTemp.uiBAT_Ucell_Min_CSC, 
        pItem->dischgTmsvDifCellTemp.uiBAT_Ucell_Min_CELL, 
	    pItem->dischgTmsvDifCellTemp.fMax_CellTemp,
        pItem->dischgTmsvDifCellTemp.fMin_CellTemp,
        pItem->dischgTmsvDifCellTemp.fAvg_CellTemp,
	    pItem->dischgTmsvDifCellTemp.uiBAT_T_Max_CSC,
	    pItem->dischgTmsvDifCellTemp.uiBAT_T_Max_CELL,
	    pItem->dischgTmsvDifCellTemp.uiBAT_T_Min_CSC,
	    pItem->dischgTmsvDifCellTemp.uiBAT_T_Min_CELL);
    strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.1f,%.1f,%.1f,%.1f,%.1f,%d,,", 
        pItem->dischgTmsvDifCellTemp.fChg_Start_MaxCellTemp,
        pItem->dischgTmsvDifCellTemp.fChg_Start_MinCellTemp,
        pItem->dischgTmsvDifCellTemp.fDischg_Start_MaxCellTemp,
        pItem->dischgTmsvDifCellTemp.fDischg_Start_MinCellTemp,
        pItem->dischgTmsvDifCellTemp.fPackTotalCurr,
        pItem->dischgTmsvDifCellTemp.bySOC);
    strcat(pOutStr, szTTTT);

    //--- Dischg Max_Temp ---//
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.3f,%s,%.3f,%.3f,%.3f,%d,%d,%d,%d,%.3f,%.3f,%.3f,%d,%d,%d,%d,", 
        pItem->fKey_Dischg_Max_CellTemp,
        TimeToString(pItem->dischgMaxCellTemp.tmTimeStamp, "%H:%M:%S", szTime, sizeof(szTime)),
        pItem->dischgMaxCellTemp.fMax_CellVolt,
        pItem->dischgMaxCellTemp.fMin_CellVolt,
        pItem->dischgMaxCellTemp.fAvg_CellVolt,
	    pItem->dischgMaxCellTemp.uiBAT_Ucell_Max_CSC,
        pItem->dischgMaxCellTemp.uiBAT_Ucell_Max_CELL, 
        pItem->dischgMaxCellTemp.uiBAT_Ucell_Min_CSC, 
        pItem->dischgMaxCellTemp.uiBAT_Ucell_Min_CELL, 
	    pItem->dischgMaxCellTemp.fMax_CellTemp,
        pItem->dischgMaxCellTemp.fMin_CellTemp,
        pItem->dischgMaxCellTemp.fAvg_CellTemp,
	    pItem->dischgMaxCellTemp.uiBAT_T_Max_CSC,
	    pItem->dischgMaxCellTemp.uiBAT_T_Max_CELL,
	    pItem->dischgMaxCellTemp.uiBAT_T_Min_CSC,
	    pItem->dischgMaxCellTemp.uiBAT_T_Min_CELL);
    strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.1f,%.1f,%.1f,%.1f,%.1f,%d,,", 
        pItem->dischgMaxCellTemp.fChg_Start_MaxCellTemp,
        pItem->dischgMaxCellTemp.fChg_Start_MinCellTemp,
        pItem->dischgMaxCellTemp.fDischg_Start_MaxCellTemp,
        pItem->dischgMaxCellTemp.fDischg_Start_MinCellTemp,
        pItem->dischgMaxCellTemp.fPackTotalCurr,
        pItem->dischgMaxCellTemp.bySOC);
    strcat(pOutStr, szTTTT);

    //--- Dischg Min_Temp ---//
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.3f,%s,%.3f,%.3f,%.3f,%d,%d,%d,%d,%.3f,%.3f,%.3f,%d,%d,%d,%d,", 
        pItem->fKey_Dischg_Min_CellTemp,
        TimeToString(pItem->dischgMinCellTemp.tmTimeStamp, "%H:%M:%S", szTime, sizeof(szTime)),
        pItem->dischgMinCellTemp.fMax_CellVolt,
        pItem->dischgMinCellTemp.fMin_CellVolt,
        pItem->dischgMinCellTemp.fAvg_CellVolt,
	    pItem->dischgMinCellTemp.uiBAT_Ucell_Max_CSC,
        pItem->dischgMinCellTemp.uiBAT_Ucell_Max_CELL, 
        pItem->dischgMinCellTemp.uiBAT_Ucell_Min_CSC, 
        pItem->dischgMinCellTemp.uiBAT_Ucell_Min_CELL, 
	    pItem->dischgMinCellTemp.fMax_CellTemp,
        pItem->dischgMinCellTemp.fMin_CellTemp,
        pItem->dischgMinCellTemp.fAvg_CellTemp,
	    pItem->dischgMinCellTemp.uiBAT_T_Max_CSC,
	    pItem->dischgMinCellTemp.uiBAT_T_Max_CELL,
	    pItem->dischgMinCellTemp.uiBAT_T_Min_CSC,
	    pItem->dischgMinCellTemp.uiBAT_T_Min_CELL);
    strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.1f,%.1f,%.1f,%.1f,%.1f,%d,,\n", 
        pItem->dischgMinCellTemp.fChg_Start_MaxCellTemp,
        pItem->dischgMinCellTemp.fChg_Start_MinCellTemp,
        pItem->dischgMinCellTemp.fDischg_Start_MaxCellTemp,
        pItem->dischgMinCellTemp.fDischg_Start_MinCellTemp,
        pItem->dischgMinCellTemp.fPackTotalCurr,
        pItem->dischgMinCellTemp.bySOC);
    strcat(pOutStr, szTTTT);

    //TRACEX("---- pOutStr Len = [%d] = 800\n", strlen(pOutStr));

    return pOutStr;
}

static BOOL Make_Export_BMSHealth_andUpload(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    UINT nStartRec = 0;
    UINT nIdx = 0, nBattIdx = 0;

    UINT nHealthRecTotal = pFCGI_Rough->chargerDataRepo.nHealthRecTotal;
    if((nHealthRecTotal == 0) || (nHealthRecTotal > WEB_MAX_SUPPORT_HEALTH))
    {
        Make_Web_OutPut(104, "Warn:Health Rec Num == 0.", NULL, NULL);
        return FALSE;          
    }

    //printf("Make_Export_BMSHealth_andUpload: nBmsRecTotal=%d\n", \
    //    nHealthRecTotal);

    #define HISHEALTH_EXPORT_LOCALFILE "/opt/appdata/data/His_BmsHealth.csv"
    FILE* fp = fopen(HISHEALTH_EXPORT_LOCALFILE, "w+");
    if(fp == NULL)
    {
        Make_Web_OutPut(104, "Can NOT create local file for export!", NULL, NULL);
        return FALSE;          
    }

     //---Write Title for the data strlen(szRecLine)=1800---//
    char szRecLine[2048];
    memset(szRecLine, '\0', 2048);
    sprintf(szRecLine, "%s", "Id,Date,BMSIdx,Cap(AH),"
    "Key_Cell_ChgDif_U(V),TM,Max_V,Min_V,Avg_V,U_Max_CSC,U_Max_Cell,U_Min_CSC,U_Min_Cell,Max_T,Min_T,Avg_T,"
        "T_Max_CSC,T_Max_Cell,T_Min_CSC,T_Min_Cell,Chg_ON_T_Max,Chg_ON_T_Min,Dischg_ON_T_Max,Dischg_ON_T_Min,I(A),SOC,,"
    "Key_Cell_ChgDif_T('C),TM,Max_V,Min_V,Avg_V,U_Max_CSC,U_Max_Cell,U_Min_CSC,U_Min_Cell,Max_T,Min_T,Avg_T,"
        "T_Max_CSC,T_Max_Cell,T_Min_CSC,T_Min_Cell,Chg_ON_T_Max,Chg_ON_T_Min,Dischg_ON_T_Max,Dischg_ON_T_Min,I(A),SOC,,"
    "Key_Cell_ChgMax_T('C),TM,Max_V,Min_V,Avg_V,U_Max_CSC,U_Max_Cell,U_Min_CSC,U_Min_Cell,Max_T,Min_T,Avg_T,"
        "T_Max_CSC,T_Max_Cell,T_Min_CSC,T_Min_Cell,Chg_ON_T_Max,Chg_ON_T_Min,Dischg_ON_T_Max,Dischg_ON_T_Min,I(A),SOC,,"
    "Key_Cell_ChgMin_T('C),TM,Max_V,Min_V,Avg_V,U_Max_CSC,U_Max_Cell,U_Min_CSC,U_Min_Cell,Max_T,Min_T,Avg_T,"
        "T_Max_CSC,T_Max_Cell,T_Min_CSC,T_Min_Cell,Chg_ON_T_Max,Chg_ON_T_Min,Dischg_ON_T_Max,Dischg_ON_T_Min,I(A),SOC,,"
    "Key_Cell_DishgDif_U(V),TM,Max_V,Min_V,Avg_V,U_Max_CSC,U_Max_Cell,U_Min_CSC,U_Min_Cell,Max_T,Min_T,Avg_T,"
        "T_Max_CSC,T_Max_Cell,T_Min_CSC,T_Min_Cell,Chg_ON_T_Max,Chg_ON_T_Min,Dischg_ON_T_Max,Dischg_ON_T_Min,I(A),SOC,,"
    "Key_Cell_DischgDif_T('C),TM,Max_V,Min_V,Avg_V,U_Max_CSC,U_Max_Cell,U_Min_CSC,U_Min_Cell,Max_T,Min_T,Avg_T,"
        "T_Max_CSC,T_Max_Cell,T_Min_CSC,T_Min_Cell,Chg_ON_T_Max,Chg_ON_T_Min,Dischg_ON_T_Max,Dischg_ON_T_Min,I(A),SOC,,"
    "Key_Cell_DischgMax_T('C),TM,Max_V,Min_V,Avg_V,U_Max_CSC,U_Max_Cell,U_Min_CSC,U_Min_Cell,Max_T,Min_T,Avg_T,"
        "T_Max_CSC,T_Max_Cell,T_Min_CSC,T_Min_Cell,Chg_ON_T_Max,Chg_ON_T_Min,Dischg_ON_T_Max,Dischg_ON_T_Min,I(A),SOC,,"
    "Key_Cell_DischgMin_T('C),TM,Max_V,Min_V,Avg_V,U_Max_CSC,U_Max_Cell,U_Min_CSC,U_Min_Cell,Max_T,Min_T,Avg_T,"
        "T_Max_CSC,T_Max_Cell,T_Min_CSC,T_Min_Cell,Chg_ON_T_Max,Chg_ON_T_Min,Dischg_ON_T_Max,Dischg_ON_T_Min,I(A),SOC,\n");

    fputs(szRecLine, fp);  

    UINT nLineIdx = 0;
    for(nIdx = 0; nIdx < nHealthRecTotal; nIdx++)
    {    
        //printf("healthRec[%d].bmsHealthData.nInstalledNum=[%d]\n",\
        //    nIdx, pFCGI_Rough->chargerDataRepo.healthRec[nIdx].bmsHealthData.nInstalledNum);

        for(nBattIdx = 0; nBattIdx < pFCGI_Rough->chargerDataRepo.healthRec[nIdx].bmsHealthData.nInstalledNum; nBattIdx++)
        {   
            nLineIdx++;
            if(Make_BmsHealthItem_2StrLine(pFCGI_Rough, nIdx, nBattIdx, nLineIdx, szRecLine) != NULL)
            {
                fputs(szRecLine, fp);

                //printf("nIdx=[%d]nBattIdx=[%d]nLineIdx=[%d]nInstalledNum=[%d]\n", \
                //    nIdx, nBattIdx, nLineIdx, pFCGI_Rough->chargerDataRepo.healthRec[nIdx].bmsHealthData.nInstalledNum);
            }
        }
    }

    if(fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_HisBmsHealth[%s].csv", 
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadFile_2Client(HISHEALTH_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(HISHEALTH_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload HisBmsHealth Data is FAILED!", NULL, NULL);
        return FALSE;      
    }


    unlink(HISHEALTH_EXPORT_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
   
}

//------------------------------- 2023-12-22 John Added For Remote Export BlobData For Debug -------------------------//
//1.For ESRunData
static char* Make_ESRunDataItem_2StrLine(FCGI_ROUGH_DATA* pFCGI_Rough,  OUT char* pOutStr, UINT nParamId)
{
    A_BLOB_ES_RUN_DATA    *pESRunData;
    WEB_PARAM_VALUE       *pWebParaVal;

    *pOutStr = 0;
    char szTTTT[MAX_SINGLE_STRLEN];

    //----- Step1.Get ESRun Data -----//
    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 200, nParamId)) != NULL)
    {
        if(pWebParaVal->pParamInfo->nValueSize == sizeof(A_BLOB_ES_RUN_DATA))
        {
            pESRunData = (A_BLOB_ES_RUN_DATA*)pWebParaVal->currValue.blobValue;
        }
        else
        {
            //Make_Web_OutPut(104, "Get ESRunBlobData Failed!", NULL, NULL);
            return pOutStr;
        }
    }
    else
    {
        //Make_Web_OutPut(104, "Get ESRunBlobData is NULL!", NULL, NULL);
        return pOutStr;
    }
    //System Param
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%d,%d,", pESRunData->emCCUState, pESRunData->emSystemFailReason);
    strcat(pOutStr, szTTTT);

    //Grid ACMeter DataInfo
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.1f|%.1f|%.1f,%.1f|%.1f|%.1f,", 
        pESRunData->fGrid_ACVolt_L1, 
        pESRunData->fGrid_ACVolt_L2, 
        pESRunData->fGrid_ACVolt_L3, 
        pESRunData->fGrid_ACCurr_L1, 
        pESRunData->fGrid_ACCurr_L2, 
        pESRunData->fGrid_ACCurr_L3);
    strcat(pOutStr, szTTTT);
    

    //Local AC Meter info
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.1f|%.1f|%.1f,%.1f|%.1f|%.1f,", 
        pESRunData->fACVolt_L1, 
        pESRunData->fACVolt_L2, 
        pESRunData->fACVolt_L3, 
        pESRunData->fACCurr_L1, 
        pESRunData->fACCurr_L2, 
        pESRunData->fACCurr_L3);
    strcat(pOutStr, szTTTT);


    //Calc ACLoad DataInfo
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.1f|%.1f|%.1f,%.1f|%.1f|%.1f,", 
        pESRunData->fACLoad_ACVolt_L1, 
        pESRunData->fACLoad_ACVolt_L2, 
        pESRunData->fACLoad_ACVolt_L3, 
        pESRunData->fACLoad_ACCurr_L1, 
        pESRunData->fACLoad_ACCurr_L2, 
        pESRunData->fACLoad_ACCurr_L3);
    strcat(pOutStr, szTTTT);

    //---MeterValue Topology---//
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,", 
        pESRunData->emAuxPos, 
        pESRunData->f_PgridMeter,
        pESRunData->f_PcabMeter,
        pESRunData->f_PauxMeter,
        pESRunData->f_PauxMeter_3,
        pESRunData->f_PoffGridMeter,
        pESRunData->f_PmaxGridDemand);
    strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.2f,%.2f,%.2f,%.2f,%.2f,", 
        pESRunData->f_QgridMeter,
        pESRunData->f_QcabMeter,
        pESRunData->f_QauxMeter,
        pESRunData->f_QauxMeter_3,
        pESRunData->f_QoffGridMeter);
    strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,", 
        pESRunData->fPacload4Chg,
        pESRunData->fPacload4Dischg,
        pESRunData->fAvg_Pacload4Dischg,
        pESRunData->fTotal_PessChgAvaliable,
        pESRunData->fTotal_PessDischgLmt,
        pESRunData->nTMSector);
    strcat(pOutStr, szTTTT);

    //-------OnOrOffGrid---------//
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.2f,%.2f,%.2f,%.1f|%.1f|%.1f,%.1f|%.1f|%.1f,", 
        pESRunData->fPoffgrid_LoadMeter,
        pESRunData->fQoff_LoadMeter,
        pESRunData->fFreqoff_LoadMeter,
        pESRunData->fVoffLoad[0], 
        pESRunData->fVoffLoad[1], 
        pESRunData->fVoffLoad[2],
        pESRunData->fSTS_InputVolt[0], 
        pESRunData->fSTS_InputVolt[1], 
        pESRunData->fSTS_InputVolt[2]);
    strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.1f|%.1f|%.1f,%.1f|%.1f|%.1f,%.2f,%.2f,%.2f,%.2f,%.2f,", 
        pESRunData->fSTS_InputCurt[0], 
        pESRunData->fSTS_InputCurt[1], 
        pESRunData->fSTS_InputCurt[2],
        pESRunData->fSTS_OutputVolt[0], 
        pESRunData->fSTS_OutputVolt[1], 
        pESRunData->fSTS_OutputVolt[2],
        pESRunData->fAC_Freq,
        pESRunData->fAmbit_Temp,
        pESRunData->fPtotalActive,
        pESRunData->fPtotalReactive,
        pESRunData->fPtotalFactor);
    strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%X,%X,%X,%X,\n", 
        pESRunData->mapPCSSysStatu[0].byPCSSysStatu,
        pESRunData->mapPCSSysStatu[1].byPCSSysStatu,
        pESRunData->mapPCSSysStatu[2].byPCSSysStatu,
        pESRunData->mapPCSSysStatu[3].byPCSSysStatu);
    strcat(pOutStr, szTTTT);

    return pOutStr;
}   

static void WriteContent2Table_ESRunData(FCGI_ROUGH_DATA* pFCGI_Rough, FILE *fp, UINT nParamId)
{

    char szRecLine[1024];
 
    //title for the data
    memset(szRecLine, '\0', 1024);
    sprintf(szRecLine, "%s",
        "CCUStat,SysFailRSN,VGridPhsA|B|C,IGridPhsA|B|C,"
        "VCabPhsA|B|C,ICabPhsA|B|C,"
        "VLoadPhsA|B|C,ILoadPhsA|B|C,"
        "AuxPos,PGridMeter,PCabMeter,PAuxMeter,PAuxMeter_3,POffGridMeter,"
        "PMaxGridDemand,QGridMeter,QCabMeter,QAuxMeter,QAuxMeter_3,QOffGridMeter,"
        "PacLoad4Chg,PacLoad4Dischg,AvgPacLoad4Dischg,TotPessChgAvaliable,TotPessDischgLmt,TMSector,"
        "POffgridLoadMeter,QOffLoadMeter,FreqOffLoadMeter,"
        "OffLoadV,STS-In-V,STS-In-I,STS-Out-V,"
        "AC-Freq,AmbitTemp,PTotActive,PTotReactive,PTotFactor,"
        "PCS#1Sys,PCS#2Sys,PCS#3Sys,PCS#4Sys,\n");
    
    fputs(szRecLine, fp);  

    memset(szRecLine, 0, 1024);
    if(Make_ESRunDataItem_2StrLine(pFCGI_Rough, szRecLine, nParamId) != NULL)
    {
        fputs(szRecLine, fp);
    }

    return;
}

static BOOL Export_RealTimeESRunDataAndUpload(FCGI_ROUGH_DATA* pFCGI_Rough, UINT nParamId, BLOB_TYPE emBlobType)
{
    //now make the output -> /opt/appdata/data/Blob_Data.csv
    #define BLOBDATA_EXPORT_LOCALFILE "/opt/appdata/data/Blob_Data.csv"
    FILE* fp = fopen(BLOBDATA_EXPORT_LOCALFILE, "w+");
    if(fp == NULL)
    {
        Make_Web_OutPut(104, "Can NOT create ESRunBlobData file for export!", NULL, NULL);
        return FALSE;          
    }

	WriteContent2Table_ESRunData(pFCGI_Rough,  fp, nParamId);

    if(fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_ESRunData[%s].csv", 
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadFile_2Client(BLOBDATA_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(BLOBDATA_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload ESRunData Data is FAILED!", NULL, NULL);
        return FALSE;      
    }
 
    unlink(BLOBDATA_EXPORT_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}


//2.For SingleRectData
static char* Make_SingleRectDataItem_2StrLine(FCGI_ROUGH_DATA* pFCGI_Rough,  OUT char* pOutStr, UINT nParamId, UINT nGroupId)
{
    A_BLOB_SINGLE_RECT_INFO    *pSingleRectData;
    WEB_PARAM_VALUE       *pWebParaVal;

    *pOutStr = 0;
    char szTTTT[MAX_SINGLE_STRLEN];

    //----- Step1.Get SingleRect Data -----//
    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 200, nParamId)) != NULL)
    {
        if(pWebParaVal->pParamInfo->nValueSize == sizeof(A_BLOB_SINGLE_RECT_INFO))
        {
            pSingleRectData = (A_BLOB_SINGLE_RECT_INFO*)pWebParaVal->currValue.blobValue;
        }
        else
        {
            //Make_Web_OutPut(104, "Get SingleRectData Failed!", NULL, NULL);
            return pOutStr;
        }
    }
    else
    {
        //Make_Web_OutPut(104, "Get SingleRectData is NULL!", NULL, NULL);
        return pOutStr;
    }

    int j;
    int nConfigCellNum = pSingleRectData->nOnlineNum[nGroupId];
    for(j = 0;j < nConfigCellNum;j++){
        snprintf(szTTTT, MAX_SINGLE_STRLEN, "%d,%d,%d,%d,", 
            nGroupId + 1,
            j + 1,
            pSingleRectData->arrRect[nGroupId][0].bExist,
            pSingleRectData->arrRect[nGroupId][0].byCommStatu);
        strcat(pOutStr, szTTTT);

        int i;
        for(i = 0;i < MAX_SINGLE_STBATT_NUM;i++){
            snprintf(szTTTT, MAX_SINGLE_STRLEN, "%X%X%X%X", 
                pSingleRectData->arrRect[nGroupId][j].byRectErrCode[0 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byRectErrCode[1 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byRectErrCode[2 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byRectErrCode[3 + 4 * i]);
            strcat(pOutStr, szTTTT);
        }
        snprintf(szTTTT, MAX_SINGLE_STRLEN, "%s", ",");
        strcat(pOutStr, szTTTT);

        for(i = 0;i < MAX_SINGLE_STBATT_NUM;i++){
            snprintf(szTTTT, MAX_SINGLE_STRLEN, "%X%X%X%X", 
                pSingleRectData->arrRect[nGroupId][j].byInvertErrCode[0 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byInvertErrCode[1 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byInvertErrCode[2 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byInvertErrCode[3 + 4 * i]);
            strcat(pOutStr, szTTTT);
        }
        snprintf(szTTTT, MAX_SINGLE_STRLEN, "%s", ",");
        strcat(pOutStr, szTTTT);

        for(i = 0;i < MAX_SINGLE_STBATT_NUM;i++){
            snprintf(szTTTT, MAX_SINGLE_STRLEN, "%X%X%X%X", 
                pSingleRectData->arrRect[nGroupId][j].byPCSFaultCode[0 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byPCSFaultCode[1 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byPCSFaultCode[2 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byPCSFaultCode[3 + 4 * i]);
            strcat(pOutStr, szTTTT);
        }
        snprintf(szTTTT, MAX_SINGLE_STRLEN, "%s", ",");
        strcat(pOutStr, szTTTT);

        for(i = 0;i < MAX_SINGLE_STBATT_NUM;i++){
            snprintf(szTTTT, MAX_SINGLE_STRLEN, "%X%X%X%X", 
                pSingleRectData->arrRect[nGroupId][j].byPCSFaultCode_1113[0 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byPCSFaultCode_1113[1 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byPCSFaultCode_1113[2 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byPCSFaultCode_1113[3 + 4 * i]);
            strcat(pOutStr, szTTTT);
        }
        snprintf(szTTTT, MAX_SINGLE_STRLEN, "%s", ",");
        strcat(pOutStr, szTTTT);

        for(i = 0;i < MAX_SINGLE_STBATT_NUM;i++){
            snprintf(szTTTT, MAX_SINGLE_STRLEN, "%X%X%X%X", 
                pSingleRectData->arrRect[nGroupId][j].byPCSFaultCode_1114[0 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byPCSFaultCode_1114[1 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byPCSFaultCode_1114[2 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byPCSFaultCode_1114[3 + 4 * i]);
            strcat(pOutStr, szTTTT);
        }
        snprintf(szTTTT, MAX_SINGLE_STRLEN, "%s", ",");
        strcat(pOutStr, szTTTT);

        for(i = 0;i < MAX_SINGLE_STBATT_NUM;i++){
            snprintf(szTTTT, MAX_SINGLE_STRLEN, "%X%X%X%X", 
                pSingleRectData->arrRect[nGroupId][j].byPCSFaultCode_1115[0 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byPCSFaultCode_1115[1 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byPCSFaultCode_1115[2 + 4 * i],
                pSingleRectData->arrRect[nGroupId][j].byPCSFaultCode_1115[3 + 4 * i]);
            strcat(pOutStr, szTTTT);
        }
        snprintf(szTTTT, MAX_SINGLE_STRLEN, "%s", ",");
        strcat(pOutStr, szTTTT);

        // snprintf(szTTTT, 64, "%d,", pSingleRectData->arrRect[nGroupId][0].uiPCSFaultCode_1113);
        // strcat(pOutStr, szTTTT);
        // snprintf(szTTTT, 64, "%d,", pSingleRectData->arrRect[nGroupId][0].uiPCSFaultCode_1114);
        // strcat(pOutStr, szTTTT);
        // snprintf(szTTTT, 64, "%d,", pSingleRectData->arrRect[nGroupId][0].uiPCSFaultCode_1115);
        // strcat(pOutStr, szTTTT);

        //Rect Alarm Info//
        snprintf(szTTTT, MAX_SINGLE_STRLEN, "%d,%.1f|%.1f,%.2f,%.2f,%.1f|%.1f|%.1f,%.1f|%.1f|%.1f,", 
            pSingleRectData->arrRect[nGroupId][j].emWorkMode,
            pSingleRectData->arrRect[nGroupId][j].fRTOutputVolt,
            pSingleRectData->arrRect[nGroupId][j].fRTOutputCurr,
            pSingleRectData->arrRect[nGroupId][j].fRect_DCDCVer,
            pSingleRectData->arrRect[nGroupId][j].fRect_PFCVer,
            pSingleRectData->arrRect[nGroupId][j].fPh_InputVolt[0],
            pSingleRectData->arrRect[nGroupId][j].fPh_InputVolt[1],
            pSingleRectData->arrRect[nGroupId][j].fPh_InputVolt[2],
            pSingleRectData->arrRect[nGroupId][j].fPh_InputCurr[0],
            pSingleRectData->arrRect[nGroupId][j].fPh_InputCurr[1],
            pSingleRectData->arrRect[nGroupId][j].fPh_InputCurr[2]);
        strcat(pOutStr, szTTTT);

    // snprintf(szTTTT, 64, "%X,", pSingleRectData->arrRect[nGroupId][0].byBarCode);
    // strcat(pOutStr, szTTTT);

        //Added For STS
        snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.1f|%.1f|%.1f,%.2f,%.2f,%.2f,%.1f|%.1f|%.1f,%.2f,%.1f|%.1f|%.1f,%.2f,\n", 
            pSingleRectData->arrRect[nGroupId][j].fPh_OutputVolt[0], 
            pSingleRectData->arrRect[nGroupId][j].fPh_OutputVolt[1], 
            pSingleRectData->arrRect[nGroupId][j].fPh_OutputVolt[2],
            pSingleRectData->arrRect[nGroupId][j].fAC_Freq,
            pSingleRectData->arrRect[nGroupId][j].fAmbit_Temp,
            pSingleRectData->arrRect[nGroupId][j].fPtotalActive,
            pSingleRectData->arrRect[nGroupId][j].fPphase_Active[0],
            pSingleRectData->arrRect[nGroupId][j].fPphase_Active[1],
            pSingleRectData->arrRect[nGroupId][j].fPphase_Active[2],
            pSingleRectData->arrRect[nGroupId][j].fPtotalReactive,
            pSingleRectData->arrRect[nGroupId][j].fPphase_Reactive[0],
            pSingleRectData->arrRect[nGroupId][j].fPphase_Reactive[1],
            pSingleRectData->arrRect[nGroupId][j].fPphase_Reactive[2],
            pSingleRectData->arrRect[nGroupId][j].fPtotalFactor);
        strcat(pOutStr, szTTTT);
    }

    return pOutStr;
}   

static void WriteContent2Table_SingleRectData(FCGI_ROUGH_DATA* pFCGI_Rough, FILE *fp, UINT nParamId)
{

    char szRecLine[1024];
    //title for the data
    memset(szRecLine, '\0', 1024);
    sprintf(szRecLine, "%s",
        "GrpId,RectId,"
        "Exist,CommStat,"
        "RectErrCode,InvertErrCode,PCSFaultCode,"
        "PCSFaultCode(1113),PCSFaultCode(1114),PCSFaultCode(1115),"
        "WorkMode,"
        "RTOut-V|I,DCDCver,PFCver,"
        "V-in,I-in,"
        "V-out,AC-Hz,AmbitTemp,"
        "TotActPow,ActPow,"
        "TotReactPow,ReactPow,"
        "TotPowFactor,\n");

    fputs(szRecLine, fp);  
        
    int i;
    for(i = 0; i < MAX_SINGLE_STBATT_NUM; i++){
        memset(szRecLine, 0, 1024);
        if(Make_SingleRectDataItem_2StrLine(pFCGI_Rough, szRecLine, nParamId, i) != NULL)
        {
            fputs(szRecLine, fp);
        }
    }
    //Added ABU Info.
    memset(szRecLine, 0, 1024);
    if(Make_SingleRectDataItem_2StrLine(pFCGI_Rough, szRecLine, nParamId, TYPE_SIG_RECT_STS) != NULL)
    {
        fputs(szRecLine, fp);
    }

    return;
}

static BOOL Export_SingleRectDataAndUpload(FCGI_ROUGH_DATA* pFCGI_Rough, UINT nParamId, BLOB_TYPE emBlobType)
{
    //now make the output -> /opt/appdata/data/Blob_Data.csv
    #define BLOBDATA_EXPORT_LOCALFILE "/opt/appdata/data/Blob_Data.csv"
    FILE* fp = fopen(BLOBDATA_EXPORT_LOCALFILE, "w+");
    if(fp == NULL)
    {
        Make_Web_OutPut(104, "Can NOT create SingleRectData file for export!", NULL, NULL);
        return FALSE;          
    }

	WriteContent2Table_SingleRectData(pFCGI_Rough,  fp, nParamId);

    if(fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_SingleRectData[%s].csv", 
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadFile_2Client(BLOBDATA_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(BLOBDATA_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload SingleRectData Data is FAILED!", NULL, NULL);
        return FALSE;      
    }
 
    unlink(BLOBDATA_EXPORT_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}


//3.For BranchPCSData
static char* Make_BranchPCSDataItem_2StrLine(FCGI_ROUGH_DATA* pFCGI_Rough,  OUT char* pOutStr, UINT nParamId, UINT nGroupId)
{
    A_BLOB_BRANCH_GRP_RECTDATA    *pBranchPCSData;
    WEB_PARAM_VALUE       *pWebParaVal;

    *pOutStr = 0;
    char szTTTT[MAX_SINGLE_STRLEN];

    //----- Step1.Get BranchPCS Data -----//
    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 211 + nGroupId, nParamId)) != NULL)
    {
        if(pWebParaVal->pParamInfo->nValueSize == sizeof(A_BLOB_BRANCH_GRP_RECTDATA))
        {
            pBranchPCSData = (A_BLOB_BRANCH_GRP_RECTDATA*)pWebParaVal->currValue.blobValue;
        }
        else
        {
            //Make_Web_OutPut(104, "Get BranchPCSData Failed!", NULL, NULL);
            return pOutStr;
        }
    }
    else
    {
        //Make_Web_OutPut(104, "Get BranchPCSData is NULL!", NULL, NULL);
        return pOutStr;
    }
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%d,%d,%d,%d,%.2f,%.2f,%.2f,%d,%.2f,%.2f,%.2f,%.2f,%d,", 
        nGroupId + 1,
        pBranchPCSData->nOnlineACDCNum,
        pBranchPCSData->nOnlineDCDCNum,
        pBranchPCSData->nOnlineMPPTNum,
        pBranchPCSData->fACDC_RealTime_Volt,
        pBranchPCSData->fACDC_RealTime_Curr,
        pBranchPCSData->fACDC_Max_Output_Pwr_Ability,
        pBranchPCSData->emACDC_WorkStatu,
        pBranchPCSData->fMPPT_RealTime_Volt,
        pBranchPCSData->fMPPT_RealTime_Curr,
        pBranchPCSData->fMPPT_Max_DisChrg_Pwr,
        pBranchPCSData->fMPPT_Max_Output_Pwr_Ability,
        pBranchPCSData->emMPPT_WorkStatu);
    strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.2f,%.2f,%.2f,%d,%.2f,%.2f,%d,%.2f,%.2f,%lf,%.2f,\n", 
        pBranchPCSData->fDCDC_RealTime_Volt,
        pBranchPCSData->fDCDC_RealTime_Curr,
        pBranchPCSData->fDCDC_Max_Output_Pwr_Ability,
        pBranchPCSData->emDCDC_WorkStatu,
        pBranchPCSData->fBatt_RealTime_Volt,
        pBranchPCSData->fBatt_RealTime_Curr,
        pBranchPCSData->nSOC,
        pBranchPCSData->fBatt_Max_DisChrg_Pwr,
        pBranchPCSData->fBatt_Max_Chrg_Pwr_Req,
        pBranchPCSData->fMPPT_Total_Generate_KWH,
        pBranchPCSData->fACDCOutRelayVolt);
    strcat(pOutStr, szTTTT);

    return pOutStr;
}

static void WriteContent2Table_BranchPCSData(FCGI_ROUGH_DATA* pFCGI_Rough, FILE *fp, UINT nParamId)
{

    char szRecLine[1024];
 
    //title for the data
    memset(szRecLine, '\0', 1024);
    sprintf(szRecLine, "%s",
        "GrpId,"
        "OnlineACDCNum,OnlineDCDCNum,OnlineMPPTNum,"
        //RT means realtime
        "ACDC-RT-V,ACDC-RT-I,ACDCMaxOutPowAbility,ACDCWorkStat,"
        "MPPT-RT-V,MPPT-RT-I,MPPTMaxDischrgPow,MPPTMaxOutPowAbility,MPPTWorkStat,"
        "DCDC-RT-V,DCDC-RT-I,DCDCMaxOutPowAbility,DCDCWorkStat,"
        "Batt-RT-V,Batt-RT-I,SOC,BattMaxDischrgPow,BattMaxchrgPowReq,"
        "MPPTTotGeneKWH,ACDCOutRelayV,\n");
    
    fputs(szRecLine, fp);  

    int i;
    for(i = 0;i < MAX_SINGLE_STBATT_NUM;i++){
        memset(szRecLine, 0, 1024);
        if(Make_BranchPCSDataItem_2StrLine(pFCGI_Rough, szRecLine, nParamId, i) != NULL)
        {
            fputs(szRecLine, fp);
        }
    }
    
    return;
}

static BOOL Export_BranchPCSDataAndUpload(FCGI_ROUGH_DATA* pFCGI_Rough, UINT nParamId, BLOB_TYPE emBlobType)
{
    //now make the output -> /opt/appdata/data/Blob_Data.csv
    #define BLOBDATA_EXPORT_LOCALFILE "/opt/appdata/data/Blob_Data.csv"
    FILE* fp = fopen(BLOBDATA_EXPORT_LOCALFILE, "w+");
    if(fp == NULL)
    {
        Make_Web_OutPut(104, "Can NOT create BranchPCSData file for export!", NULL, NULL);
        return FALSE;          
    }

	WriteContent2Table_BranchPCSData(pFCGI_Rough,  fp, nParamId);

    if(fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_BranchPCSData[%s].csv", 
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadFile_2Client(BLOBDATA_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(BLOBDATA_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload BranchPCSData Data is FAILED!", NULL, NULL);
        return FALSE;      
    }
 
    unlink(BLOBDATA_EXPORT_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}


//4.For BranchPwrCalcData
static char* Make_BranchPwrCalcDataItem_2StrLine(FCGI_ROUGH_DATA* pFCGI_Rough,  OUT char* pOutStr, UINT nParamId, UINT nGroupId)
{
    A_BLOB_BRANCH_NODE_CALC_DATA    *pBranchPwrCalcData;
    WEB_PARAM_VALUE       *pWebParaVal;

    *pOutStr = 0;
    char szTTTT[MAX_SINGLE_STRLEN];

    //----- Step1.Get BranchPwrCalc Data -----//
    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 221 + nGroupId, nParamId)) != NULL)
    {
        if(pWebParaVal->pParamInfo->nValueSize == sizeof(A_BLOB_BRANCH_NODE_CALC_DATA))
        {
            pBranchPwrCalcData = (A_BLOB_BRANCH_NODE_CALC_DATA*)pWebParaVal->currValue.blobValue;
        }
        else
        {
            //Make_Web_OutPut(104, "Get BranchPwrCalcData Failed!", NULL, NULL);
            return pOutStr;
        }
    }
    else
    {
        //Make_Web_OutPut(104, "Get BranchPwrCalcData is NULL!", NULL, NULL);
        return pOutStr;
    }
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%d,", nGroupId + 1);
    strcat(pOutStr, szTTTT);

    //1. MPPT与储能电池最大放电功率/充电功率
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.2f,%.2f,%.2f,", 
        pBranchPwrCalcData->fMPPT_Max_DisChrg_Pwr,
        pBranchPwrCalcData->fBatt_Max_DisChrg_Pwr,
        pBranchPwrCalcData->fBatt_Max_Chrg_Pwr_Req);
    strcat(pOutStr, szTTTT);

    //2. MPPT模组功率分配
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.2f,%.2f,%.2f,", 
        pBranchPwrCalcData->fMPPT_Distri_4_ACDC_Pwr,
        pBranchPwrCalcData->fMPPT_Distri_4_DCDC_Pwr,
        pBranchPwrCalcData->fMPPT_Distri_4_Batt_Pwr);
    strcat(pOutStr, szTTTT);

    //3. 储能电池功率分配
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.2f,%.2f,%d,", 
        pBranchPwrCalcData->fBatt_Distri_4_ACDC_Pwr,
        pBranchPwrCalcData->fBatt_Distri_4_DCDC_Pwr,
        pBranchPwrCalcData->nSOC);
    strcat(pOutStr, szTTTT);

    //4. 模块最大输出能力
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.2f,%.2f,", 
        pBranchPwrCalcData->fACDC_Max_Ability_Pwr,
        pBranchPwrCalcData->fDCDC_Max_Ability_Pwr);
    strcat(pOutStr, szTTTT);

    //5. MPPT与储能电池剩余输出功率
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.2f,%.2f,%.2f,", 
        pBranchPwrCalcData->fRemain_Batt_DisChrg_Pwr,
        pBranchPwrCalcData->fRemain_MPPT_DisChrg_Pwr,
        pBranchPwrCalcData->fRemain_Batt_Chrg_Pwr_Req);
    strcat(pOutStr, szTTTT);

    //6. ACDC分配给储能电池最大充电功率
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.2f,%.2f,%d,%.2f,%.2f,%.2f,%d,%d,\n", 
        pBranchPwrCalcData->fACDC_Distri_4_BattChrg_Pwr,
        pBranchPwrCalcData->fQoutput_Distri4ACDC,
        pBranchPwrCalcData->emBatt_WorkMode,
        pBranchPwrCalcData->fAvg_ChrgLmtPower,
        pBranchPwrCalcData->fAvg_DisChrgingLmtPower,
        pBranchPwrCalcData->fAvg_QoutputLmt,
        pBranchPwrCalcData->tm_Last4BATTempLmt,
        pBranchPwrCalcData->nSysOverTempTimes);
    strcat(pOutStr, szTTTT);
    return pOutStr;
}

static void WriteContent2Table_BranchPwrCalcData(FCGI_ROUGH_DATA* pFCGI_Rough, FILE *fp, UINT nParamId)
{

    char szRecLine[1024];
 
    //title for the data
    memset(szRecLine, '\0', 1024);
    sprintf(szRecLine, "%s",
        "GrpId,"
        "MPPT_Max_DisChrg_Pwr,Batt_Max_DisChrg_Pwr,Batt_Max_Chrg_Pwr_Req,"
        "MPPT_Distri_4_ACDC_Pwr,MPPT_Distri_4_DCDC_Pwr,MPPT_Distri_4_Batt_Pwr,"
        "Batt_Distri_4_ACDC_Pwr,Batt_Distri_4_DCDC_Pwr,SOC,"
        "ACDC_Max_Ability_Pwr,DCDC_Max_Ability_Pwr,"
        "Remain_Batt_DisChrg_Pwr,Remain_MPPT_DisChrg_Pwr,Remain_Batt_Chrg_Pwr_Req,"
        "ACDC_Distri_4_BattChrg_Pwr,Qoutput_Lmt,"
        "emBatt_WorkMode,Avg_ChrgLmtPower,Avg_DisChrgingLmtPower,Avg_QoutputLmt,"
        "Last4BATTempLmt,SysOverTempTimes,\n");
    
    fputs(szRecLine, fp);  

    int i;
    for(i = 0;i < MAX_SINGLE_STBATT_NUM;i++){
        memset(szRecLine, 0, 1024);
        if(Make_BranchPwrCalcDataItem_2StrLine(pFCGI_Rough, szRecLine, nParamId, i) != NULL)
        {
            fputs(szRecLine, fp);
        }
    }

    return;
}

static BOOL Export_BranchPwrCalcDataAndUpload(FCGI_ROUGH_DATA* pFCGI_Rough, UINT nParamId, BLOB_TYPE emBlobType)
{
    //now make the output -> /opt/appdata/data/Blob_Data.csv
    #define BLOBDATA_EXPORT_LOCALFILE "/opt/appdata/data/Blob_Data.csv"
    FILE* fp = fopen(BLOBDATA_EXPORT_LOCALFILE, "w+");
    if(fp == NULL)
    {
        Make_Web_OutPut(104, "Can NOT create BranchPwrCalcData file for export!", NULL, NULL);
        return FALSE;          
    }

	WriteContent2Table_BranchPwrCalcData(pFCGI_Rough,  fp, nParamId);

    if(fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_BranchPwrCalcData[%s].csv", 
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadFile_2Client(BLOBDATA_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(BLOBDATA_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload BranchPwrCalcData Data is FAILED!", NULL, NULL);
        return FALSE;      
    }
 
    unlink(BLOBDATA_EXPORT_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}


//5.For PCSGrpData
static char* Make_PCSGrpDataItem_2StrLine(FCGI_ROUGH_DATA* pFCGI_Rough,  OUT char* pOutStr, UINT nParamId, UINT nGroupId)
{
    A_BLOB_RECT_GRP_DATA    *pPCSGrpData;
    WEB_PARAM_VALUE       *pWebParaVal;

    *pOutStr = 0;
    char szTTTT[MAX_SINGLE_STRLEN];

    //----- Step1.Get PCSGrp Data -----//
    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 200, nParamId)) != NULL)
    {
        if(pWebParaVal->pParamInfo->nValueSize == sizeof(A_BLOB_RECT_GRP_DATA))
        {
            pPCSGrpData = (A_BLOB_RECT_GRP_DATA*)pWebParaVal->currValue.blobValue;
        }
        else
        {
            //Make_Web_OutPut(104, "Get PCSGrpData Failed!", NULL, NULL);
            return pOutStr;
        }
    }
    else
    {
        //Make_Web_OutPut(104, "Get PCSGrpData is NULL!", NULL, NULL);
        return pOutStr;
    }
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%d,%.2f,%.2f,%.2f,%d,%.2f,%.2f,%.2f,%.2f,%d,\n", 
        nGroupId + 1,
        pPCSGrpData->fRatedPower[nGroupId],
        pPCSGrpData->fRatedCurr[nGroupId],
        pPCSGrpData->fRealTime_MaxCurr[nGroupId],
        pPCSGrpData->emWorkMode[nGroupId],
        pPCSGrpData->fCtrlVolt[nGroupId],
        pPCSGrpData->fCtrlCurr[nGroupId],
        pPCSGrpData->fRealTimeVolt[nGroupId],
        pPCSGrpData->fRealTimeCurr[nGroupId],
        pPCSGrpData->nOnlineRectNum[nGroupId]);
    strcat(pOutStr, szTTTT);

    return pOutStr;
}

static void WriteContent2Table_PCSGrpData(FCGI_ROUGH_DATA* pFCGI_Rough, FILE *fp, UINT nParamId)
{

    char szRecLine[1024];
    //title for the data
    memset(szRecLine, '\0', 1024);
    sprintf(szRecLine, "%s",
        "GrpId,"
        "RT-P,RT-I,"
        "RT-MaxI,"
        "WorkMode,"
        "CtrlV,CtrlI,"
        "RT-V,RT-I,"
        "OnlineRectNum,\n");
        
    fputs(szRecLine, fp);  

    int i;
    for(i = 0;i < MAX_SINGLE_STBATT_NUM;i++){
        memset(szRecLine, 0, 1024);
        if(Make_PCSGrpDataItem_2StrLine(pFCGI_Rough, szRecLine, nParamId, i) != NULL)
        {
            fputs(szRecLine, fp);
        }
    }

    return;
}

static BOOL Export_PCSGrpDataAndUpload(FCGI_ROUGH_DATA* pFCGI_Rough, UINT nParamId, BLOB_TYPE emBlobType)
{
    //now make the output -> /opt/appdata/data/Blob_Data.csv
    #define BLOBDATA_EXPORT_LOCALFILE "/opt/appdata/data/Blob_Data.csv"
    FILE* fp = fopen(BLOBDATA_EXPORT_LOCALFILE, "w+");
    if(fp == NULL)
    {
        Make_Web_OutPut(104, "Can NOT create PCSGrpData file for export!", NULL, NULL);
        return FALSE;
    }

	WriteContent2Table_PCSGrpData(pFCGI_Rough,  fp, nParamId);

    if(fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_PCSGrpData[%s].csv", 
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadFile_2Client(BLOBDATA_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(BLOBDATA_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload PCSGrpData Data is FAILED!", NULL, NULL);
        return FALSE;      
    }
 
    unlink(BLOBDATA_EXPORT_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}


//6.For SingleBattData
static char* Make_SingleBattDataItem_2StrLine(FCGI_ROUGH_DATA* pFCGI_Rough,  OUT char* pOutStr, UINT nParamId, UINT nGroupId)
{
    A_BLOB_SINGLE_STBATT_RUNINFO    *pSingleBattData;
    WEB_PARAM_VALUE       *pWebParaVal;

    *pOutStr = 0;
    char szTTTT[MAX_SINGLE_STRLEN];

    //----- Step1.Get SingleBatt Data -----//
    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 501 + nGroupId, nParamId)) != NULL)
    {
        if(pWebParaVal->pParamInfo->nValueSize == sizeof(A_BLOB_SINGLE_STBATT_RUNINFO))
        {
            pSingleBattData = (A_BLOB_SINGLE_STBATT_RUNINFO*)pWebParaVal->currValue.blobValue;
        }
        else
        {
            //Make_Web_OutPut(104, "Get SingleBattData Failed!", NULL, NULL);
            return pOutStr;
        }
    }
    else
    {
        //Make_Web_OutPut(104, "Get SingleBattData is NULL!", NULL, NULL);
        return pOutStr;
    }
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%d,%d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,", 
        nGroupId + 1,
        pSingleBattData->emPowerONStatu,
        pSingleBattData->emBattStatus_Run,
        pSingleBattData->emLast_BattStatus_Run,
        pSingleBattData->emBattStatus_Alm,
        pSingleBattData->nSOC,
        pSingleBattData->nSOH,
        pSingleBattData->fPackTotalVolt,
        pSingleBattData->fPackTotalCurr,
        pSingleBattData->fAllow_LmtChargeVolt,
        pSingleBattData->fAllow_LmtDisChargeVolt,
        pSingleBattData->fAllow_MaxChargeCurr,
        pSingleBattData->fAllow_MaxDisChargeCurr,
        pSingleBattData->fMax_CellVolt,
        pSingleBattData->fMin_CellVolt,
        pSingleBattData->fAvg_CellVolt);
    strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,\n", 
        pSingleBattData->fMax_CellTemp,
        pSingleBattData->fMin_CellTemp,
        pSingleBattData->fAvg_CellTemp,
        pSingleBattData->fChrg_Energy,
        pSingleBattData->fDisChrging_Energy,
        pSingleBattData->fTotal_RateCapacity,
        pSingleBattData->fAdd_ChgCapacity,
        pSingleBattData->fAdd_DischgCapacity,
        pSingleBattData->fAdd_ChgEnergy,
        pSingleBattData->fAdd_DischgEnergy,
        pSingleBattData->fInsulation_P_R,
        pSingleBattData->fInsulation_N_R,
        pSingleBattData->fOutsideVolt,
        pSingleBattData->fNegativeBusVolt,
        pSingleBattData->usBattWorkState);
    strcat(pOutStr, szTTTT);

    return pOutStr;
}

static void WriteContent2Table_SingleBattData(FCGI_ROUGH_DATA* pFCGI_Rough, FILE *fp, UINT nParamId)
{

    char szRecLine[1024];
 
    //title for the data
    memset(szRecLine, '\0', 1024);
    sprintf(szRecLine, "%s",
        "GrpId,"
        "PowOnStat,RunStat,LastRunStat,AlarmStat,SOC,SOH,"
        "PackTotV,PackTotI,AllowedChrgLmtV,AllowedDischrgLmtV,"
        "AllowedMaxChrgI,AllowedMaxDischrgI,MaxCellV,MinCellV,AvgCellV,"
        "MaxCellTemp,MinCellTemp,AvgCellTemp,"
        "ChrgEnergy,DischrgEnergy,"
        "TotRateCapacity,AddedChrgCapacity,AddedDischrgCapacity,"
        "AddedChrgEnergy,AddedDischrgEnergy,Insulation_P_R,Insulation_N_R,"
        "V-out,NegBusV,WorkState,\n");
    
    fputs(szRecLine, fp);  

    int i;
    for(i = 0;i < MAX_SINGLE_STBATT_NUM;i++){
        memset(szRecLine, 0, 1024);
        if(Make_SingleBattDataItem_2StrLine(pFCGI_Rough, szRecLine, nParamId, i) != NULL)
        {
            fputs(szRecLine, fp);
        }
    }

    return;
}

static BOOL Export_SingleBatDataAndUpload(FCGI_ROUGH_DATA* pFCGI_Rough, UINT nParamId, BLOB_TYPE emBlobType)
{
    //now make the output -> /opt/appdata/data/Blob_Data.csv
    #define BLOBDATA_EXPORT_LOCALFILE "/opt/appdata/data/Blob_Data.csv"
    FILE* fp = fopen(BLOBDATA_EXPORT_LOCALFILE, "w+");
    if(fp == NULL)
    {
        Make_Web_OutPut(104, "Can NOT create SingleBattData file for export!", NULL, NULL);
        return FALSE;          
    }

	WriteContent2Table_SingleBattData(pFCGI_Rough,  fp, nParamId);

    if(fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_SingleBattData[%s].csv", 
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadFile_2Client(BLOBDATA_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(BLOBDATA_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload SingleBattData Data is FAILED!", NULL, NULL);
        return FALSE;      
    }
 
    unlink(BLOBDATA_EXPORT_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}


//7.For SingleIfyBMSData
static char* Make_SingleIfyBMSDataItem_2StrLine(FCGI_ROUGH_DATA* pFCGI_Rough,  OUT char* pOutStr, UINT nParamId, UINT nGroupId)
{
    A_BLOB_IFY_BMS_DATAINFO    *pSingleIfyBMSData;
    WEB_PARAM_VALUE       *pWebParaVal;

    *pOutStr = 0;
    char szTTTT[MAX_SINGLE_STRLEN];

    //----- Step1.Get SingleIfyBMS Data -----//
    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 501 + nGroupId, nParamId)) != NULL)
    {
        if(pWebParaVal->pParamInfo->nValueSize == sizeof(A_BLOB_IFY_BMS_DATAINFO))
        {
            pSingleIfyBMSData = (A_BLOB_IFY_BMS_DATAINFO*)pWebParaVal->currValue.blobValue;
        }
        else
        {
            //Make_Web_OutPut(104, "Get SingleIfyBMSData Failed!", NULL, NULL);
            return pOutStr;
        }
    }
    else
    {
        //Make_Web_OutPut(104, "Get SingleIfyBMSData is NULL!", NULL, NULL);
        return pOutStr;
    }
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%d,%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,", 
        nGroupId + 1,
        pSingleIfyBMSData->emBattSysState,
        pSingleIfyBMSData->emBattChgState,
        pSingleIfyBMSData->emPowerONStatu,
        pSingleIfyBMSData->fAllow_MaxChargeCurr,
        pSingleIfyBMSData->fAllow_MaxDisChargeCurr,
        pSingleIfyBMSData->fAllow_LmtChargeVolt,
        pSingleIfyBMSData->fAllow_LmtDisChargeVolt,
        pSingleIfyBMSData->fPackTotalVolt,
        pSingleIfyBMSData->fPackTotalCurr,
        pSingleIfyBMSData->fOutputPower,
        pSingleIfyBMSData->fOutsideVolt,
        pSingleIfyBMSData->fSOC,
        pSingleIfyBMSData->fSOE,
        pSingleIfyBMSData->fSOH,
        pSingleIfyBMSData->fTotalCapacity,
        pSingleIfyBMSData->fCellAddTotalVolt,
        pSingleIfyBMSData->usCellAverageVolt,
        pSingleIfyBMSData->sCellAverageTemp,
        pSingleIfyBMSData->sBattCabinetTemp);
    strcat(pOutStr, szTTTT);

    // //--- 0x1435ffxx ---//
    // snprintf(szTTTT, 64, "%d,", pSingleIfyBMSData->unioBattAlm);
    // strcat(pOutStr, szTTTT);

    // //--- 0x1436ffxx ---//
    // snprintf(szTTTT, 64, "%d,", pSingleIfyBMSData->unioBattFault);
    // strcat(pOutStr, szTTTT);

    // //--- 0x1438ffxx ---//
    // snprintf(szTTTT, 64, "%d,", pSingleIfyBMSData->unioBattDIDOStat);
    // strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%d,%d,%d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,%d,%d,%d,%d,", 
        pSingleIfyBMSData->usMaxCellVlot_Cell,
        pSingleIfyBMSData->usMaxCellVlot_Volt,
        pSingleIfyBMSData->usMinCellVlot_Cell,
        pSingleIfyBMSData->usMinCellVlot_Volt,
        pSingleIfyBMSData->usMaxCellTemp_Cell,
        pSingleIfyBMSData->sMaxCellTemp_Temp,
        pSingleIfyBMSData->usMinCellTemp_Cell,
        pSingleIfyBMSData->sMinCellTemp_Temp,
        pSingleIfyBMSData->fNegativeBusVolt,
        pSingleIfyBMSData->fInsulation_P_R,
        pSingleIfyBMSData->fInsulation_N_R,
        pSingleIfyBMSData->usSystemRunTime,
        pSingleIfyBMSData->usCanChargeTime,
        pSingleIfyBMSData->usCanDischargeTime,
        pSingleIfyBMSData->usCycleTimes);
    strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%d,%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,\n", 
        pSingleIfyBMSData->usMonAlarmNum,
        pSingleIfyBMSData->usMonFaultNum,
        pSingleIfyBMSData->usAccuAlarmNum,
        pSingleIfyBMSData->usAccuFaultNum,
        pSingleIfyBMSData->fRemainEnergy,
        pSingleIfyBMSData->fTotalEnergy,
        pSingleIfyBMSData->fSingleChgCapacity,
        pSingleIfyBMSData->fSingleDischgCapacity,
        pSingleIfyBMSData->fAccuChgCapacity,
        pSingleIfyBMSData->fAccuDischgCapacity,
        pSingleIfyBMSData->fAccuChgEnergy,
        pSingleIfyBMSData->fAccuDischgEnergy,
        pSingleIfyBMSData->fNowDOD,
        pSingleIfyBMSData->fDischgStartDOD,
        pSingleIfyBMSData->usDiagSOSLev,
        pSingleIfyBMSData->usDiagSOSCode);
    strcat(pOutStr, szTTTT);


    return pOutStr;
}

static void WriteContent2Table_SingleIfyBMSData(FCGI_ROUGH_DATA* pFCGI_Rough, FILE *fp, UINT nParamId)
{

    char szRecLine[1024];
 
    //title for the data
    memset(szRecLine, '\0', 1024);
    sprintf(szRecLine, "%s",
        "GrpId,"
        "BattSysState,BattChrgState,PowOnStat,AllowedMaxChrgI,AllowedMaxDischrgI,"
        "AllowedChrgVLmt,AllowedDischrgVLmt,"
        "PackTotV,PackTotI,OutPow,OutsideV,SOC,SOE,SOH,TotCapacity,"
        "CellAddTotV,CellvAVgV,CellAvgTemp,BattCabinetTemp,"
        "MaxCellV(cell),MaxCellV(Volt),MinCellV(cell),MinCellV(Volt),"
        "MaxCellTemp(cell),MaxCellTemp(temp),MinCellTemp(cell),MinCellTemp(temp),"
        "NegativeBusV,Insulation_P_R,Insulation_N_R,SysRuntime,CanChrgTime,CanDischrgTime,CycleTimes,"
        "MonAlarmNum,MonFaultNum,AccuAlarmNum,AccuFaultNum,RemainEnergy,TotEnergy,SingleChrgCapacity,SingleDischrgCapacity,"
        "AccuChrgCapacity,AccuDischrgCapacity,AccuChrgEnergy,AccuDischrgEnergy,"
        "Now-DOD,DischrgStartDOD,DiagSOSLev,DiagSOSCode,\n");
    
    fputs(szRecLine, fp);  

    int i;
    for(i = 0;i < MAX_SINGLE_STBATT_NUM;i++){
        memset(szRecLine, 0, 1024);
        if(Make_SingleIfyBMSDataItem_2StrLine(pFCGI_Rough, szRecLine, nParamId, i) != NULL)
        {
            fputs(szRecLine, fp);
        }
    }

    return;
}

static BOOL Export_SingleIfyBMSDataAndUpload(FCGI_ROUGH_DATA* pFCGI_Rough, UINT nParamId, BLOB_TYPE emBlobType)
{
    //now make the output -> /opt/appdata/data/Blob_Data.csv
    #define BLOBDATA_EXPORT_LOCALFILE "/opt/appdata/data/Blob_Data.csv"
    FILE* fp = fopen(BLOBDATA_EXPORT_LOCALFILE, "w+");
    if(fp == NULL)
    {
        Make_Web_OutPut(104, "Can NOT create SingleIfyBMSData file for export!", NULL, NULL);
        return FALSE;          
    }

	WriteContent2Table_SingleIfyBMSData(pFCGI_Rough,  fp, nParamId);

    if(fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_SingleIfyBMSData[%s].csv", 
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadFile_2Client(BLOBDATA_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(BLOBDATA_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload SingleIfyBMSData Data is FAILED!", NULL, NULL);
        return FALSE;      
    }
 
    unlink(BLOBDATA_EXPORT_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}


//8.For EMSMgmtPlanData
static char* Make_EMSMgmtPlanDataItem_2StrLine(FCGI_ROUGH_DATA* pFCGI_Rough,  OUT char* pOutStr, UINT nParamId, UINT nGroupId)
{
    A_BLOB_EMS_CONFIG_PLAN    *pEMSMgmtPlanData;
    WEB_PARAM_VALUE       *pWebParaVal;

    *pOutStr = 0;
    char szTTTT[MAX_SINGLE_STRLEN];

    //----- Step1.Get EMSMgmtPlan Data -----//
    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 200, nParamId)) != NULL)
    {
        if(pWebParaVal->pParamInfo->nValueSize == sizeof(A_BLOB_EMS_CONFIG_PLAN))
        {
            pEMSMgmtPlanData = (A_BLOB_EMS_CONFIG_PLAN*)pWebParaVal->currValue.blobValue;
        }
        else
        {
            //Make_Web_OutPut(104, "Get EMSMgmtPlanData Failed!", NULL, NULL);
            return pOutStr;
        }
    }
    else
    {
        //Make_Web_OutPut(104, "Get EMSMgmtPlanData is NULL!", NULL, NULL);
        return pOutStr;
    }
    snprintf(szTTTT, MAX_SINGLE_STRLEN, "%d,", nGroupId + 1);
    strcat(pOutStr, szTTTT);

    memset(szTTTT, '\0', MAX_SINGLE_STRLEN);
    TimeToString(pEMSMgmtPlanData->emsLmtSectors[nGroupId].tmStart, TIME_CHN_FMT, szTTTT, 32);
    strcat(pOutStr, szTTTT);

    snprintf(szTTTT, MAX_SINGLE_STRLEN, ",%d,%d,%d,%d,%d,%d,%d,\n", 
        pEMSMgmtPlanData->emsLmtSectors[nGroupId].nInterval,
        pEMSMgmtPlanData->emsLmtSectors[nGroupId].emFeeRate,
        pEMSMgmtPlanData->emsLmtSectors[nGroupId].emBatt_WorkMode,
        pEMSMgmtPlanData->emsLmtSectors[nGroupId].nStopSOC,
        pEMSMgmtPlanData->emsLmtSectors[nGroupId].nActvPwrLmt,
        pEMSMgmtPlanData->emsLmtSectors[nGroupId].nReActvPwrLmt,
        pEMSMgmtPlanData->emsLmtSectors[nGroupId].nBattGrpPwrLmt);
    strcat(pOutStr, szTTTT);

    return pOutStr;
}

static void WriteContent2Table_EMSMgmtPlanData(FCGI_ROUGH_DATA* pFCGI_Rough, FILE *fp, UINT nParamId)
{

    char szRecLine[1024];

    A_BLOB_EMS_CONFIG_PLAN    *pEMSMgmtPlanData;
    WEB_PARAM_VALUE       *pWebParaVal;

    //----- Step1.Get EMSMgmtPlan Data -----//
    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 200, nParamId)) != NULL)
    {
        if(pWebParaVal->pParamInfo->nValueSize == sizeof(A_BLOB_EMS_CONFIG_PLAN))
        {
            pEMSMgmtPlanData = (A_BLOB_EMS_CONFIG_PLAN*)pWebParaVal->currValue.blobValue;
        }
        else
        {
            //Make_Web_OutPut(104, "Get EMSMgmtPlanData Failed!", NULL, NULL);
        }
    }
    else
    {
        //Make_Web_OutPut(104, "Get EMSMgmtPlanData is NULL!", NULL, NULL);
    }

    int num = pEMSMgmtPlanData->nTotal_EmsLmt_Sectors;

    memset(szRecLine, '\0', 1024);
    sprintf(szRecLine, "EMSMgmt-Ver,TotEMSMgmtNum,\n"
        "%d,%d,\n",
        pEMSMgmtPlanData->nEMSMgmt_Ver, num);

    fputs(szRecLine, fp);

    //title for the data
    memset(szRecLine, '\0', 1024);
    sprintf(szRecLine, "%s",
        "PeriodId,"
        "StartTime,IntervalTime,FeeRate,WorkMode,SOC,"
        "ActivePowLmt,ReactivePowLmt,BattGrpPowLmt,\n");
        
    fputs(szRecLine, fp);  

    int i;
    for(i = 0;i < num;i++){
        memset(szRecLine, 0, 1024);
        if(Make_EMSMgmtPlanDataItem_2StrLine(pFCGI_Rough, szRecLine, nParamId, i) != NULL)
        {
            fputs(szRecLine, fp);
        }
    }

    return;
}

static BOOL Export_EMSMgmtPlanAndUpload(FCGI_ROUGH_DATA* pFCGI_Rough, UINT nParamId, BLOB_TYPE emBlobType)
{
    //now make the output -> /opt/appdata/data/Blob_Data.csv
    #define BLOBDATA_EXPORT_LOCALFILE "/opt/appdata/data/Blob_Data.csv"
    FILE* fp = fopen(BLOBDATA_EXPORT_LOCALFILE, "w+");
    if(fp == NULL)
    {
        Make_Web_OutPut(104, "Can NOT create EMSMgmtPlanData file for export!", NULL, NULL);
        return FALSE;          
    }

	WriteContent2Table_EMSMgmtPlanData(pFCGI_Rough,  fp, nParamId);

    if(fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_EMSMgmtPlanData[%s].csv", 
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadFile_2Client(BLOBDATA_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(BLOBDATA_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload EMSMgmtPlanData Data is FAILED!", NULL, NULL);
        return FALSE;      
    }
 
    unlink(BLOBDATA_EXPORT_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}


//9.For SingleCellData
static char* Make_SingleCellDataItem_2StrLine(FCGI_ROUGH_DATA* pFCGI_Rough,  OUT char* pOutStr, UINT nParamId, UINT nGroupId, UINT nTypeId)
{
    A_BLOB_IFY_BMS_DATAINFO    *pSingleCellData;
    WEB_PARAM_VALUE       *pWebParaVal;

    *pOutStr = 0;
    char szTTTT[MAX_SINGLE_STRLEN];

    //----- Step1.Get SingleCellData Data -----//
    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 501 + nGroupId, nParamId)) != NULL)
    {
        if(pWebParaVal->pParamInfo->nValueSize == sizeof(A_BLOB_IFY_BMS_DATAINFO))
        {
            pSingleCellData = (A_BLOB_IFY_BMS_DATAINFO*)pWebParaVal->currValue.blobValue;
        }
        else
        {
            //Make_Web_OutPut(104, "Get SingleCellData Failed!", NULL, NULL);
            return pOutStr;
        }
    }
    else
    {
        //Make_Web_OutPut(104, "Get SingleCellData is NULL!", NULL, NULL);
        return pOutStr;
    }

    int i;
    int nConfigCellNum = pSingleCellData->usCellVoltTotalNum;
    int nCellNumDiv = nConfigCellNum / 10;
    int nCellNumRem = nConfigCellNum % 10;

    if(nTypeId == 1){
        snprintf(szTTTT, MAX_SINGLE_STRLEN, "%s", "CellV,");
        strcat(pOutStr, szTTTT);
        for(i = 0;i < nCellNumDiv;i++){
            snprintf(szTTTT, MAX_SINGLE_STRLEN, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,", 
                pSingleCellData->usCellVolt[i * 10],
                pSingleCellData->usCellVolt[i * 10 + 1],
                pSingleCellData->usCellVolt[i * 10 + 2],
                pSingleCellData->usCellVolt[i * 10 + 3],
                pSingleCellData->usCellVolt[i * 10 + 4],
                pSingleCellData->usCellVolt[i * 10 + 5],
                pSingleCellData->usCellVolt[i * 10 + 6],
                pSingleCellData->usCellVolt[i * 10 + 7],
                pSingleCellData->usCellVolt[i * 10 + 8],
                pSingleCellData->usCellVolt[i * 10 + 9]
                );
            strcat(pOutStr, szTTTT);
        }

        for(i = 0;i < nCellNumRem;i++){
            snprintf(szTTTT, MAX_SINGLE_STRLEN, "%d,", pSingleCellData->usCellVolt[nCellNumDiv * 10 + i]);
            strcat(pOutStr, szTTTT);
        }
    }

    if(nTypeId == 2){
        snprintf(szTTTT, MAX_SINGLE_STRLEN, "%s", "CellTemp,");
        strcat(pOutStr, szTTTT);
        for(i = 0;i < nCellNumDiv;i++){
            snprintf(szTTTT, MAX_SINGLE_STRLEN, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,", 
                pSingleCellData->cCellTemp[i * 10],
                pSingleCellData->cCellTemp[i * 10 + 1],
                pSingleCellData->cCellTemp[i * 10 + 2],
                pSingleCellData->cCellTemp[i * 10 + 3],
                pSingleCellData->cCellTemp[i * 10 + 4],
                pSingleCellData->cCellTemp[i * 10 + 5],
                pSingleCellData->cCellTemp[i * 10 + 6],
                pSingleCellData->cCellTemp[i * 10 + 7],
                pSingleCellData->cCellTemp[i * 10 + 8],
                pSingleCellData->cCellTemp[i * 10 + 9]
                );
            strcat(pOutStr, szTTTT);
        }

        for(i = 0;i < nCellNumRem;i++){
            snprintf(szTTTT, MAX_SINGLE_STRLEN, "%d,", pSingleCellData->cCellTemp[nCellNumDiv * 10 + i]);
            strcat(pOutStr, szTTTT);
        }
    }

    if(nTypeId == 3){
        snprintf(szTTTT, MAX_SINGLE_STRLEN, "%s", "CellRes,");
        strcat(pOutStr, szTTTT);
        for(i = 0;i < nCellNumDiv;i++){
            snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,", 
                pSingleCellData->fCellRes[i * 10],
                pSingleCellData->fCellRes[i * 10 + 1],
                pSingleCellData->fCellRes[i * 10 + 2],
                pSingleCellData->fCellRes[i * 10 + 3],
                pSingleCellData->fCellRes[i * 10 + 4],
                pSingleCellData->fCellRes[i * 10 + 5],
                pSingleCellData->fCellRes[i * 10 + 6],
                pSingleCellData->fCellRes[i * 10 + 7],
                pSingleCellData->fCellRes[i * 10 + 8],
                pSingleCellData->fCellRes[i * 10 + 9]
                );
            strcat(pOutStr, szTTTT);
        }

        for(i = 0;i < nCellNumRem;i++){
            snprintf(szTTTT, MAX_SINGLE_STRLEN, "%.2f,", pSingleCellData->fCellRes[nCellNumDiv * 10 + i]);
            strcat(pOutStr, szTTTT);
        }
    }

    if(nTypeId == 4){
        snprintf(szTTTT, MAX_SINGLE_STRLEN, "%s", "CellBalanceStat,");
        strcat(pOutStr, szTTTT);
        for(i = 0;i < nCellNumDiv;i++){
            snprintf(szTTTT, MAX_SINGLE_STRLEN, "%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,", 
                pSingleCellData->byCellBalanceStat[i * 10],
                pSingleCellData->byCellBalanceStat[i * 10 + 1],
                pSingleCellData->byCellBalanceStat[i * 10 + 2],
                pSingleCellData->byCellBalanceStat[i * 10 + 3],
                pSingleCellData->byCellBalanceStat[i * 10 + 4],
                pSingleCellData->byCellBalanceStat[i * 10 + 5],
                pSingleCellData->byCellBalanceStat[i * 10 + 6],
                pSingleCellData->byCellBalanceStat[i * 10 + 7],
                pSingleCellData->byCellBalanceStat[i * 10 + 8],
                pSingleCellData->byCellBalanceStat[i * 10 + 9]
                );
            strcat(pOutStr, szTTTT);
        }

        for(i = 0;i < nCellNumRem;i++){
            snprintf(szTTTT, MAX_SINGLE_STRLEN, "%X,", pSingleCellData->byCellBalanceStat[nCellNumDiv * 10 + i]);
            strcat(pOutStr, szTTTT);
        }
    }

    strcat(pOutStr, "\n");

    return pOutStr;
}

static void WriteContent2Table_SingleCellData(FCGI_ROUGH_DATA* pFCGI_Rough, FILE *fp, UINT nParamId)
{
    char szRecLine[1500];

    int i;
    char res[1500];
    char str[32];
    for(i = 0;i < MAX_SINGLE_STBATT_NUM;i++){
        char szTTTT[64];

        A_BLOB_IFY_BMS_DATAINFO     *pSingleCellData;
        WEB_PARAM_VALUE             *pWebParaVal;

        //----- Get SingleCellData Data -----//
        if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 501 + i, nParamId)) != NULL)
        {
            if(pWebParaVal->pParamInfo->nValueSize == sizeof(A_BLOB_IFY_BMS_DATAINFO))
            {
                pSingleCellData = (A_BLOB_IFY_BMS_DATAINFO*)pWebParaVal->currValue.blobValue;
            }
            else
            {
                //Make_Web_OutPut(104, "Get SingleCellData Failed!", NULL, NULL);
            }
        }
        else
        {
            //Make_Web_OutPut(104, "Get SingleCellData is NULL!", NULL, NULL);
        }

        //title for the data
        int j;
        memset(res, '\0', 1500);
        memset(str, '\0', 32);
        for(j = 1;j <= pSingleCellData->usCellVoltTotalNum;j++){
            sprintf(str, "%d,", j);
            strcat(res, str);
        }

        memset(szRecLine, '\0', 1500);
        sprintf(szRecLine, "Grp%d,%s\n", i+1, res);
        fputs(szRecLine, fp);  

        memset(szRecLine, 0, 1500);
        if(Make_SingleCellDataItem_2StrLine(pFCGI_Rough, szRecLine, nParamId, i, 1) != NULL)
        {
            fputs(szRecLine, fp);
        }
        if(Make_SingleCellDataItem_2StrLine(pFCGI_Rough, szRecLine, nParamId, i, 2) != NULL)
        {
            fputs(szRecLine, fp);
        }
        if(Make_SingleCellDataItem_2StrLine(pFCGI_Rough, szRecLine, nParamId, i, 3) != NULL)
        {
            fputs(szRecLine, fp);
        }
        if(Make_SingleCellDataItem_2StrLine(pFCGI_Rough, szRecLine, nParamId, i, 4) != NULL)
        {
            fputs(szRecLine, fp);
        }

        memset(str, '\0', 32);
        sprintf(str, "\n");
        fputs(str, fp);
    }
    return;
}

static BOOL Export_SingleCellDataAndUpload(FCGI_ROUGH_DATA* pFCGI_Rough, UINT nParamId, BLOB_TYPE emBlobType)
{
    //now make the output -> /opt/appdata/data/Blob_Data.csv
    #define BLOBDATA_EXPORT_LOCALFILE "/opt/appdata/data/Blob_Data.csv"
    FILE* fp = fopen(BLOBDATA_EXPORT_LOCALFILE, "w+");
    if(fp == NULL)
    {
        Make_Web_OutPut(104, "Can NOT create SingleCellData file for export!", NULL, NULL);
        return FALSE;          
    }

	WriteContent2Table_SingleCellData(pFCGI_Rough,  fp, nParamId);

    if(fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_SingleCellData[%s].csv", 
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadFile_2Client(BLOBDATA_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(BLOBDATA_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload SingleCellData Data is FAILED!", NULL, NULL);
        return FALSE;      
    }
 
    unlink(BLOBDATA_EXPORT_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}


static void WriteContent2Table_CCU2PCS_CanMsg(FCGI_ROUGH_DATA* pFCGI_Rough, FILE *fp )
{
    char szRecLine[1024];
    UINT i , j, nOffset = 0;
    BYTE *pData;

    A_BLOB_CCU2PCSCAN_MSG   *pBlobCanMsg;
    WEB_PARAM_VALUE         *pWebParaVal;

    //----- Step1.Get CCU2PCSCAN_MSG  Data -----//
    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 200, 205)) != NULL)
    {
        if(pWebParaVal->pParamInfo->nValueSize == sizeof(A_BLOB_CCU2PCSCAN_MSG))
        {
            pBlobCanMsg = (A_BLOB_CCU2PCSCAN_MSG*)pWebParaVal->currValue.blobValue;
        }
        else
        {
            TRACE("!!! Get A_BLOB_CCU2PCSCAN_MSG Failed! \n");
            Make_Web_OutPut(104, "Get A_BLOB_CCU2PCSCAN_MSG Failed!", NULL, NULL);
            return;
        }
    }
    else
    {
        TRACE("!!! Get A_BLOB_CCU2PCSCAN_MSG is NULL! \n");
        Make_Web_OutPut(104, "Get A_BLOB_CCU2PCSCAN_MSG is NULL!", NULL, NULL);
        return;
    }

    char szTemp[32];
    memset(szRecLine, '\0', 1024);
    snprintf(szRecLine, 1024, "CAN-Id,Data[8],TM[%s]\n",\ 
        TimeToString(Conv_UTC_2LocalTime(pBlobCanMsg->tm_Recv), TIME_CHN_FMT, szTemp, 32));

    fputs(szRecLine, fp);

    for(i = 0; i < MIN(pBlobCanMsg->nTotalMsgNum,MAX_CAN_CYCLETIMES) ; i++)
    {
        pData = &pBlobCanMsg->byCanMsg[i][0];
        memset(szRecLine, 0, 1024);
        nOffset = 0;

        for(j = 0; j < MAX_CAN_MSGLEN; )
        {
            nOffset += snprintf(&szRecLine[nOffset], 1024, "%02X%02X%02X%02X,%02X %02X %02X %02X %02X %02X %02X %02X\n", \
                pData[0], pData[1], pData[2], pData[3], pData[4], pData[5], pData[6], pData[7], pData[8], pData[9], pData[10], pData[11]);
            
 
            //printf("i=[%d] j=[%d] nOffset=[%d]  szRecLine=[%s]\n", i, j, nOffset, szRecLine);

            pData = pData + 12;

            j = j + 12;
        }
        fputs(szRecLine, fp);
    }

    return;
}

static BOOL Export_CCU2PCSCanMsgAndUpload(FCGI_ROUGH_DATA* pFCGI_Rough, int nCond_gun)
{
    #define CCU2PCSCANMSG_EXPORT_LOCALFILE "/opt/appdata/data/CCU2PCS_CanMsg.csv"
    FILE* fp = fopen(CCU2PCSCANMSG_EXPORT_LOCALFILE, "w+");
    if(fp == NULL)
    {
        Make_Web_OutPut(104, "Can NOT create CCU2PCS_CanMsg file for export!", NULL, NULL);
        return FALSE;          
    }

	WriteContent2Table_CCU2PCS_CanMsg(pFCGI_Rough,  fp);

    if(fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_CCU2PCS_CanMsg[%s].csv",\
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,\
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadFile_2Client(CCU2PCSCANMSG_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(CCU2PCSCANMSG_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload CCU2PCS_CanMsg Data is FAILED!", NULL, NULL);
        return FALSE;      
    }
 
    unlink(CCU2PCSCANMSG_EXPORT_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}

/*********************************************************************************
*  
*  FUNCTION NAME : Make_Export_BMSSOCCalibrationData_andUpload()
*  CREATOR       : 
*  CREATED DATE  : 2026-1-23 10:13:53
*  DESCRIPTION   : 导出SOC校准相关数据
*  
*  CHANGE HISTORY:    
*  
***********************************************************************************/
static BOOL Make_Export_BMSSOCCalibrationData_andUpload(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    #define OCVSTATUS_LOCALFILE "/data/OCVCalibrationData_backup.csv"

    system("cp /data/OCVCalibrationData.csv /data/OCVCalibrationData_backup.csv");
    
    if(access(OCVSTATUS_LOCALFILE, F_OK) == -1)
    {
        Make_Web_OutPut(104, "Can NOT find SOCCalibrationData local file for export!", NULL, NULL);
        return FALSE;          
    }
    
    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "BMS_SOC_Calibration_Data[%s].csv", 
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadFile_2Client(OCVSTATUS_LOCALFILE, szRemoteName))
    {
        unlink(OCVSTATUS_LOCALFILE);
        Make_Web_OutPut(104, "Upload BMS_SOC_Calibration_Data Data is FAILED!", NULL, NULL);
        return FALSE;      
    }
    
    unlink(OCVSTATUS_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}

static BOOL Export_AfeSrTestAndUpload(void)
{
    #define AFESRSTATUS_LOCALFILE "/data/AfeSrStatus_backup.csv"

    system("cp /data/AfeSrStatus.csv /data/AfeSrStatus_backup.csv");
    
    if(access(AFESRSTATUS_LOCALFILE, F_OK) == -1)
    {
        Make_Web_OutPut(104, "Can NOT find Afe Sr Status local file for export!", NULL, NULL);
        return FALSE;          
    }
    
    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "AfeSrStatus_Data[%s].csv", 
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadFile_2Client(AFESRSTATUS_LOCALFILE, szRemoteName))
    {
        unlink(AFESRSTATUS_LOCALFILE);
        Make_Web_OutPut(104, "Upload AfeSrStatus Data is FAILED!", NULL, NULL);
        return FALSE;      
    }
    
    unlink(AFESRSTATUS_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}

// static BOOL Export_ManuBmsCanLogAndUpload(FCGI_ROUGH_DATA* pFCGI_Rough)
// {
//     #define MANURECBMSCANMSG_EXPORT_LOCALFILE "/opt/appdata/data/ManuRecBms_CanMsg.0"
    
//     if(access("/data/bmslog.0", F_OK) != 0)
//     {
//         Make_Web_OutPut(104, "Can NOT find ManuBmsCanLog file for export!", NULL, NULL);
//         return FALSE;          

//     }

//     system("cp /data/bmslog.0 /opt/appdata/data/ManuRecBms_CanMsg.0");

//     char szRemoteName[128];
//     char szTemp[32];
//     snprintf(szRemoteName, 128, "%s_ManuRecBms_CanMsg[%s].0",\
//         pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,\
//         TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
//     if(!UploadFile_2Client(MANURECBMSCANMSG_EXPORT_LOCALFILE, szRemoteName))
//     {
//         unlink(MANURECBMSCANMSG_EXPORT_LOCALFILE);
//         Make_Web_OutPut(104, "Upload ManuRecBms_CanMsg Data is FAILED!", NULL, NULL);
//         return FALSE;      
//     }
 
//     unlink(MANURECBMSCANMSG_EXPORT_LOCALFILE);
    
//     return FALSE; //shoud return FALSE!!!
// }

// static BOOL Export_AutoBmsCanLogAndUpload(FCGI_ROUGH_DATA* pFCGI_Rough)
// {
//     #define AUTORECBMSCANMSG_EXPORT_LOCALFILE "/opt/appdata/data/AutoRecBms_CanMsg.0"
    
//     if(access("/data/bmsalmlog.0", F_OK) != 0)
//     {
//         Make_Web_OutPut(104, "Can NOT find AutoBmsCanLog file for export!", NULL, NULL);
//         return FALSE;          

//     }

//     system("cp /data/bmsalmlog.0 /opt/appdata/data/AutoRecBms_CanMsg.0");

//     char szRemoteName[128];
//     char szTemp[32];
//     snprintf(szRemoteName, 128, "%s_AutoRecBms_CanMsg[%s].0",\
//         pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,\
//         TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
//     if(!UploadFile_2Client(AUTORECBMSCANMSG_EXPORT_LOCALFILE, szRemoteName))
//     {
//         unlink(AUTORECBMSCANMSG_EXPORT_LOCALFILE);
//         Make_Web_OutPut(104, "Upload AutoRecBms_CanMsg Data is FAILED!", NULL, NULL);
//         return FALSE;      
//     }
 
//     unlink(AUTORECBMSCANMSG_EXPORT_LOCALFILE);
    
//     return FALSE; //shoud return FALSE!!!
// }


// export all bmslog (include manual and auto)
static BOOL Export_AllBmsCanLogAndUpload(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    #define AUTORECBMSCANMSG_EXPORT_LOCALFILE "/opt/appdata/data/Bms_CanMsg.tar"
    
    if(access("/data/bmsalmlog.0", F_OK) != 0 && access("/data/bmslog.0", F_OK) != 0)
    {
        Make_Web_OutPut(104, "Can NOT find BmsCanLog file for export!", NULL, NULL);
        return FALSE;          

    }

    // system("cd /data");
    system("tar cvf /opt/appdata/data/Bms_CanMsg.tar /data/bms*");

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_Bms_CanMsg[%s].tar",\
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,\
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadBinFile_2Client(AUTORECBMSCANMSG_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(AUTORECBMSCANMSG_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload RecBms_CanMsg Data is FAILED!", NULL, NULL);
        return FALSE;      
    }
 
    unlink(AUTORECBMSCANMSG_EXPORT_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}
// export over

static BOOL Export_MeterLogAndUpload(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    #define METERLOG_EXPORT_LOCALFILE "/opt/appdata/data/Meter_Log.tar"
    
    if(access("/data/meterfaultlog.0", F_OK) != 0)
    {
        Make_Web_OutPut(104, "Can NOT find MeterLog file for export!", NULL, NULL);
        return FALSE;          

    }

    // system("cd /data");
    system("tar cvf /opt/appdata/data/Meter_Log.tar /data/meter*");

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_Meter_Log[%s].tar",\
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,\
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadBinFile_2Client(METERLOG_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(METERLOG_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload MeterLog Data is FAILED!", NULL, NULL);
        return FALSE;      
    }
 
    unlink(METERLOG_EXPORT_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}

static BOOL Make_Export_FactoryTest_andUpload(FCGI_ROUGH_DATA* pFCGI_Rough) // xsy add for upload factory test data
{
    printf("---Start Make_Export_FactoryTest_andUpload ---\n");
    
    #define FACTORY_TEST_EXPORT_LOCALFILE "/opt/appdata/data/factory_test.tar"

    #define FACTORY_TEST_TMP_EXPORT_LOCALFILE "/data/cell_temperature.csv"
    #define FACTORY_TEST_BAL_EXPORT_LOCALFILE "/data/cell_balance.csv"
    #define FACTORY_TEST_RES_EXPORT_LOCALFILE "/data/cell_resistance.csv"
    #define FACTORY_TEST_VOL_EXPORT_LOCALFILE "/data/cell_voltage.csv"
    #define FACTORY_TEST_POW_EXPORT_LOCALFILE "/data/cell_power.csv"
    

    // access csv file exist
    if(access(FACTORY_TEST_TMP_EXPORT_LOCALFILE, F_OK) == -1)
    {
        Make_Web_OutPut(104, "Can NOT find tmp local file for export!", NULL, NULL);
        return FALSE;          
    }
    if(access(FACTORY_TEST_BAL_EXPORT_LOCALFILE, F_OK) == -1)
    {
        Make_Web_OutPut(104, "Can NOT find balance local file for export!", NULL, NULL);
        return FALSE;          
    }
    if(access(FACTORY_TEST_RES_EXPORT_LOCALFILE, F_OK) == -1)
    {
        Make_Web_OutPut(104, "Can NOT find resistance local file for export!", NULL, NULL);
        return FALSE;          
    }
    if(access(FACTORY_TEST_VOL_EXPORT_LOCALFILE, F_OK) == -1)
    {
        Make_Web_OutPut(104, "Can NOT find voltage local file for export!", NULL, NULL);
        return FALSE;          
    }
    if(access(FACTORY_TEST_POW_EXPORT_LOCALFILE, F_OK) == -1)
    {
        Make_Web_OutPut(104, "Can NOT find power local file for export!", NULL, NULL);
        return FALSE;          
    }

    //now wait the 'tar' cmd to execute 
    system("tar cvf /opt/appdata/data/factory_test.tar /data/*.csv");

    //upload tar file
    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_factory_test[%s].tar",\
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,\
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));

    if(!UploadBinFile_2Client(FACTORY_TEST_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(FACTORY_TEST_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload Factory Test Data is FAILED!", NULL, NULL);
        return FALSE;      
    }

    unlink(FACTORY_TEST_EXPORT_LOCALFILE);

    return FALSE; //shoud return FALSE!!!
}

static BOOL Export_bmsAlmHisLogAndUpload(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    #define AUTORECBMSCANMSG_EXPORT_LOCALFILE "/opt/appdata/data/bmsHisLog.tar"
    
    if(access("/data/bmsHisLog.0", F_OK) != 0)
    {
        Make_Web_OutPut(104, "Can NOT find BmsHisLog file for export!", NULL, NULL);
        return FALSE;

    }

    system("tar cvf /opt/appdata/data/bmsHisLog.tar /data/bmsHisLog*");

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_Bms_HisLog[%s].tar",\
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,\
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadBinFile_2Client(AUTORECBMSCANMSG_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(AUTORECBMSCANMSG_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload RecBms_HisLog Data is FAILED!", NULL, NULL);
        return FALSE;
    }
 
    unlink(AUTORECBMSCANMSG_EXPORT_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}

static BOOL Make_Export_EMSRunData_andUpload(FCGI_ROUGH_DATA* pFCGI_Rough, int nCond_gun)
{
    //{ParamID ,BlobType}.
    static UINT arrBlobType[][2] = 
    {
        {80,  BLOB_TYPE_ES_RUNDATA},
        {101, BLOB_TYPE_SINGLE_RECT_DATAINFO}, 
        {201, BLOB_TYPE_BRANCH_GRP_RECT_DATA}, 
        {203, BLOB_TYPE_BRANCH_NODE_CALC_DATA}, 
        {204, BLOB_TYPE_GRP_PM_DATAINFO},
        {551, BLOB_TYPE_SINGLE_STBATT_RUNINFO},
        {557, BLOB_TYPE_IFYBMS_DATAINFO},
        {201, BLOB_TYPE_EMS_CONFIG_PLAN},
        {557, BLOB_TYPE_IFYBMS_DATAINFO},
        {205, BLOB_TYPE_CCU2PCSCAN_MSG}
    }; 

    if((nCond_gun < 0) || (nCond_gun > 12))
    {
        Make_Web_OutPut(104, "Can NOT Find BlobData For export!", NULL, NULL);
        return FALSE;
    }
    BOOL bRet = FALSE;

    switch (nCond_gun)
    {
        case 0:
            Make_Export_FactoryTest_andUpload(pFCGI_Rough);
            break; 
        case 1:
            bRet = Export_SingleRectDataAndUpload(pFCGI_Rough, arrBlobType[nCond_gun][0], arrBlobType[nCond_gun][1]);
            break;
        case 2:
            bRet = Export_BranchPCSDataAndUpload(pFCGI_Rough, arrBlobType[nCond_gun][0], arrBlobType[nCond_gun][1]);
            break;
        case 3:
            bRet = Export_BranchPwrCalcDataAndUpload(pFCGI_Rough, arrBlobType[nCond_gun][0], arrBlobType[nCond_gun][1]);
            break;
        case 4:
            bRet = Export_PCSGrpDataAndUpload(pFCGI_Rough, arrBlobType[nCond_gun][0], arrBlobType[nCond_gun][1]);
            break;
        case 5:
            bRet = Export_SingleBatDataAndUpload(pFCGI_Rough, arrBlobType[nCond_gun][0], arrBlobType[nCond_gun][1]);
            break;  
        case 6:
            bRet = Export_SingleIfyBMSDataAndUpload(pFCGI_Rough, arrBlobType[nCond_gun][0], arrBlobType[nCond_gun][1]);
            break; 
        case 7:
            bRet = Export_EMSMgmtPlanAndUpload(pFCGI_Rough, arrBlobType[nCond_gun][0], arrBlobType[nCond_gun][1]);
            break;  
        case 8:
            bRet = Export_SingleCellDataAndUpload(pFCGI_Rough, arrBlobType[nCond_gun][0], arrBlobType[nCond_gun][1]);
            break; 
        case 9:
            bRet = Export_CCU2PCSCanMsgAndUpload(pFCGI_Rough, arrBlobType[nCond_gun][0]);
            break;
        case 10:
            bRet = Export_MeterLogAndUpload(pFCGI_Rough);
            break;
        default:
            break;
    }

    return FALSE; //shoud return FALSE!!!
}


static BOOL Export_DODHistory_andUpload(FCGI_ROUGH_DATA* pFCGI_Rough)
{

    sqlite3 *db;
    int rc = sqlite3_open_v2("/data/BmsRunData.data", &db, SQLITE_OPEN_READONLY, NULL); //has to be read-only
    if(rc != SQLITE_OK)
    {
        TRACE("----can NOT open BmsRunData db for DOD history---\n");
        Make_Web_OutPut(104, "Can NOT open BMS data db!", NULL, NULL);
        return FALSE;
    }
    sqlite3_busy_timeout(db, 5000);

    #define DODHISTORY_EXPORT_LOCALFILE "/opt/appdata/data/DOD_History.csv"

    FILE* fp = fopen(DODHISTORY_EXPORT_LOCALFILE, "w+");
    if(fp == NULL)
    {
        sqlite3_close(db);
        Make_Web_OutPut(104, "Can NOT create local file for export!", NULL, NULL);
        return FALSE;
    }

    char szRecLine[256];
    memset(szRecLine, '\0', 256);
    sprintf(szRecLine, "Group,Seq,Time,DOD,Dischg_Qty,Chrg_Qty,Dischg_Energy,Chrg_Energy\n");
    fputs(szRecLine, fp);

    const char* szSql =
        "SELECT Group_No, Seq_Id, Time_Year, Time_MonthDay, Time_HourMinute, Time_Second, "
        "Dod, Dischg_Qty, Chrg_Qty, Dischg_Energy, Chrg_Energy "
        "FROM Data_DODHistory ORDER BY Idx";

    sqlite3_stmt* stat;
    sqlite3_prepare_v2(db, szSql, -1, &stat, NULL);

    int nLine = 0;
    while(sqlite3_step(stat) == SQLITE_ROW)
    {
        if(nLine >= MAX_BMS_RECORDS_NUM)
            break;

        int group_no   = sqlite3_column_int(stat, 0);
        int seq_id     = sqlite3_column_int(stat, 1);
        int year       = sqlite3_column_int(stat, 2);
        int monthday   = sqlite3_column_int(stat, 3);
        int hourminute = sqlite3_column_int(stat, 4);
        int second     = sqlite3_column_int(stat, 5);
        int dod        = sqlite3_column_int(stat, 6);
        int dischg_qty = sqlite3_column_int(stat, 7);
        int chrg_qty   = sqlite3_column_int(stat, 8);
        int dischg_energy = sqlite3_column_int(stat, 9);
        int chrg_energy   = sqlite3_column_int(stat, 10);

        memset(szRecLine, '\0', 256);
        snprintf(szRecLine, 256,
                 "%d,%d,%04d-%02d-%02d %02d:%02d:%02d,%d,%d,%d,%d,%d\n",
                 group_no + 1, seq_id,
                 year, (monthday >> 8) & 0xFF, monthday & 0xFF,
                 (hourminute >> 8) & 0xFF, hourminute & 0xFF, second,
                 dod, dischg_qty, chrg_qty, dischg_energy, chrg_energy);
        fputs(szRecLine, fp);
        nLine++;
    }
    sqlite3_finalize(stat);
    sqlite3_close(db);

    if(fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_DOD_History[%s].csv",
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));

    if(!UploadFile_2Client(DODHISTORY_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(DODHISTORY_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload DOD History Data is FAILED!", NULL, NULL);
        return FALSE;
    }

    unlink(DODHISTORY_EXPORT_LOCALFILE);

    return FALSE; //shoud return FALSE!!!
}

static BOOL Export_SOSHistory_andUpload(FCGI_ROUGH_DATA* pFCGI_Rough)
{

    sqlite3 *db;
    int rc = sqlite3_open_v2("/data/BmsRunData.data", &db, SQLITE_OPEN_READONLY, NULL); //has to be read-only
    if(rc != SQLITE_OK)
    {
        TRACE("----can NOT open BmsRunData db for SOS history---\n");
        Make_Web_OutPut(104, "Can NOT open BMS data db!", NULL, NULL);
        return FALSE;
    }
    sqlite3_busy_timeout(db, 5000);

    #define SOSHISTORY_EXPORT_LOCALFILE "/opt/appdata/data/SOS_History.csv"

    FILE* fp = fopen(SOSHISTORY_EXPORT_LOCALFILE, "w+");
    if(fp == NULL)
    {
        sqlite3_close(db);
        Make_Web_OutPut(104, "Can NOT create local file for export!", NULL, NULL);
        return FALSE;
    }

    char szRecLine[256];
    memset(szRecLine, '\0', 256);
    sprintf(szRecLine, "Group,Seq,Time,Sos_Level,Reason_Bitmap,Event_Type\n");
    fputs(szRecLine, fp);

    const char* szSql =
        "SELECT Group_No, Seq_Id, Time_Year, Time_MonthDay, Time_HourMinute, Time_Second, "
        "Sos_Level, Reason_Bitmap, Event_Type "
        "FROM Data_SOSHistory ORDER BY Idx";

    sqlite3_stmt* stat;
    sqlite3_prepare_v2(db, szSql, -1, &stat, NULL);

    int nLine = 0;
    while(sqlite3_step(stat) == SQLITE_ROW)
    {
        if(nLine >= MAX_BMS_RECORDS_NUM)
            break;

        int group_no      = sqlite3_column_int(stat, 0);
        int seq_id        = sqlite3_column_int(stat, 1);
        int year          = sqlite3_column_int(stat, 2);
        int monthday      = sqlite3_column_int(stat, 3);
        int hourminute    = sqlite3_column_int(stat, 4);
        int second        = sqlite3_column_int(stat, 5);
        int sos_level     = sqlite3_column_int(stat, 6);
        int reason_bitmap = sqlite3_column_int(stat, 7);
        int event_type    = sqlite3_column_int(stat, 8);

        memset(szRecLine, '\0', 256);
        snprintf(szRecLine, 256,
                 "%d,%d,%04d-%02d-%02d %02d:%02d:%02d,%d,0x%04X,%d\n",
                 group_no + 1, seq_id,
                 year, (monthday >> 8) & 0xFF, monthday & 0xFF,
                 (hourminute >> 8) & 0xFF, hourminute & 0xFF, second,
                 sos_level, reason_bitmap, event_type);
        fputs(szRecLine, fp);
        nLine++;
    }
    sqlite3_finalize(stat);
    sqlite3_close(db);

    if(fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_SOS_History[%s].csv",
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));

    if(!UploadFile_2Client(SOSHISTORY_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(SOSHISTORY_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload SOS History Data is FAILED!", NULL, NULL);
        return FALSE;
    }

    unlink(SOSHISTORY_EXPORT_LOCALFILE);

    return FALSE; //shoud return FALSE!!!
}

static BOOL Make_Export_BMS_andUpload(FCGI_ROUGH_DATA* pFCGI_Rough, int nCond_gun)
{
    switch(nCond_gun)
    {
        case 1:
            return Make_Export_BMSHealth_andUpload(pFCGI_Rough);
            break;
        case 2:
            return Export_AllBmsCanLogAndUpload(pFCGI_Rough);
            break;
        case 3:
             return Export_AfeSrTestAndUpload();
             break;
        case 4:
             return Make_Export_BMSSOCCalibrationData_andUpload(pFCGI_Rough);
             break;
        case 5:
            return Export_bmsAlmHisLogAndUpload(pFCGI_Rough);
            break;
        case 6:
            return Export_DODHistory_andUpload(pFCGI_Rough);
            break;
        case 7:
            return Export_SOSHistory_andUpload(pFCGI_Rough);
            break;
        default:
            break;
    }
    return FALSE; 
}

//------------------------------- 2023-12-22 End -------------------------//

 
/*********************************************************************************
*  
*  FUNCTION NAME : Make_Export_StatisticKWH_andUpload()
*  CREATOR       : 
*  CREATED DATE  : 2022-01-18 17:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:    
*  
***********************************************************************************/
static BOOL Make_Export_StatisticKWH_andUpload(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    printf("---Start Make_Export_StatisticKWH_andUpload ---\n");
    
    char szRecLine[1024];
    memset(szRecLine, '\0', 1024);
    //now make the output -> /opt/appdata/data/chg_rundata.csv
    #define HISTORYKWH_EXPORT_LOCALFILE "/opt/appdata/data/His_StatisticKWH.csv"
    FILE* fp = fopen(HISTORYKWH_EXPORT_LOCALFILE, "w+");
    if(fp == NULL)
    {
        Make_Web_OutPut(104, "Can NOT create local file for export!", NULL, NULL);
        return FALSE;          
    }

    //now for each item, we print it one by one
    //A_CHARGE_RUNTIME_DATA_ITEM* pItem = pFCGI_Rough->chargerDataRepo.bmsRunData_Rec.chgRunData.stChgRuntimeDataList;
    //HIS_BMS_RUNTIMEDATA_REC* pRec = &pFCGI_Rough->chargerDataRepo.bmsRunData_Rec;    
 
	WriteHeader2Table(pFCGI_Rough,   fp, TP_HISDATA_YEARLY);
	WriteContent2Table(pFCGI_Rough,  fp, TP_HISDATA_YEARLY);

	WriteHeader2Table(pFCGI_Rough,   fp, TP_HISDATA_DAILY);
	WriteContent2Table(pFCGI_Rough,  fp, TP_HISDATA_DAILY);

    WriteHeader2Table(pFCGI_Rough,   fp, TP_HISDATA_WEEKHOURS);
	WriteContent2Table(pFCGI_Rough,  fp, TP_HISDATA_WEEKHOURS);

    if(fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}

    char szRemoteName[128];
    char szTemp[32];
    snprintf(szRemoteName, 128, "%s_HisStatistic_KWH[%s].csv", 
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp, 32));
    
    if(!UploadFile_2Client(HISTORYKWH_EXPORT_LOCALFILE, szRemoteName))
    {
        unlink(HISTORYKWH_EXPORT_LOCALFILE);
        Make_Web_OutPut(104, "Upload HisStatistic Kwh Data is FAILED!", NULL, NULL);
        return FALSE;      
    }
 
    unlink(HISTORYKWH_EXPORT_LOCALFILE);
    
    return FALSE; //shoud return FALSE!!!
}


/*********************************************************************************
*  
*  FUNCTION NAME : Make Export_BmsHistoryRec_andUpload()
*  CREATOR       : 
*  CREATED DATE  : 2022-01-18 17:13:53
*  DESCRIPTION   : BMS History Record Msg Upload
*  
*  CHANGE HISTORY:    
*  
***********************************************************************************/

 

static BOOL Make_Export_BmsHistoryRec_andUpload(FCGI_ROUGH_DATA* pFCGI_Rough, UINT tm)
{
    printf("---Start Make_Export_BmsHistoryRec_andUpload ---\n");
    
    VAR_VALUE varV;
    varV.ulValue = tm; //export    
    int nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(
            FALSE, 1, 23, varV.abyValue, 4);
    if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
    {
        unlink(BMSHISREC_WEB_EXPORT_FILENAME);
        Make_Web_OutPut(104, "Execute export HisRec error!", "Execute error!", NULL);
        return FALSE;     
    }


    //now wait the 'tar' cmd to execute 
    char szSysCmd[64];
    snprintf(szSysCmd, 64, "ps | grep 'tar -czf' | grep -cv 'grep'");
    int nCmdRet = 0;// = _SYSTEM_EX(szSysCmd);    
    int nCount = 0;
    BOOL bZipOK = FALSE;
    do
    {
        nCount++;
        Sleep(1000);
        nCmdRet = _SYSTEM_EX(szSysCmd);
        //printf("----Wait[%d], the nCmdRet = %d----\n", nCount, nCmdRet);
        if(nCmdRet < 1)
        {
            bZipOK = TRUE;
            break;
        }
    } while (nCount < 90); //wait 90s at most

    if(!bZipOK)
    {
        unlink(BMSHISREC_WEB_EXPORT_FILENAME);
        Make_Web_OutPut(104, "Execute compress HisRec error!", "Execute error!", NULL);
        return FALSE;     
    }

    //printf("--- 111 ---\n");
 
    char szRemoteName[150]="file";
    char szTemp[32];
    char szTemp1[32];

    snprintf(szRemoteName, 150, "%s_HisRec[%s]_%s.tar.gz", 
            pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
            TimeToString(Conv_UTC_2LocalTime(tm), TIME_CHN_FMT, szTemp, 32),
            TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTemp1, 32));
    
    if(!UploadFile_2Client(BMSHISREC_WEB_EXPORT_FILENAME, szRemoteName))
    {
        unlink(BMSHISREC_WEB_EXPORT_FILENAME);
        Make_Web_OutPut(104, "Upload process is FAILED!", NULL, NULL);
        return FALSE;      
    }
    //printf("--- 555 ---\n");

    unlink(BMSHISREC_WEB_EXPORT_FILENAME);
    
    return FALSE; //shoud return FALSE!!!
}

/*********************************************************************************
*  
*  FUNCTION NAME : webpage handler_chg_record()
*  CREATOR       : 
*  CREATED DATE  : 2021-12-19 22:13:53
*  DESCRIPTION   : http://192.168.137.143/api/chg_record?cond_date=0&cond_gun=0&cond_start=0&export_name=&_t=1639965444350  
*  
*  CHANGE HISTORY:  http://192.168.137.143/api/chg_record?cond_date=0&cond_gun=2&cond_start=0&pageId=0&page_vol=50&_t=1639903606274
*  http://192.168.137.143/api/chg_record?query_meterval_gun=1640736000&query_meterval_tmstart=--&export_name=&_t=1640921719005
    
    http://192.168.137.250/api/chg_record?query_chgrun_gun=1651190400&query_chgrun_tmstart=--&export_name=&_t=1651912482622
 http://192.168.137.90/api/chg_record?cond_date=0&cond_gun=0&cond_start=0&export_name=&_t=1642495770699***********************************************************************************/
BOOL webpage_handler_operationlog(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    int num_count = 0;
    int nums[3];
    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        char szExportName[32];
        char szId[32];
        char szcond_system[32];
        int Main_id;
        int Sub_id;
        if(ParseStr_FromQuery(pFCGI_Rough->szQueryRaw, "export_name=", szExportName, 32) && 
           ParseStr_FromQuery(pFCGI_Rough->szQueryRaw, "id=", szId, 32))
        {
            num_count = parse_str_to_number(szId, "-", nums, 3);
            Main_id = nums[0];
            Sub_id = nums[1];
            UINT nData_idx = nums[2];
            QUERYCOND_HISMETERVAL m_cond;
            ZERO_POBJS(&m_cond, 1);       
            m_cond.nGunIdx = nData_idx;
            // printf("----export_name=[%s] Main_id=[%d] Sub_id=[%d] nData_idx=[%d] num_count=[%d]----\n", szExportName, Main_id, Sub_id, nData_idx, num_count);
            switch(Main_id)
            {
                case 0: 
                    switch(Sub_id)
                    {
                        case 1:
                            return Make_Export_BmsRunData_andUpload(pFCGI_Rough, &m_cond);
                        default:
                            return Make_Export_EMSRunData_andUpload(pFCGI_Rough, Sub_id - 2); 
                    }
                case 1: 
                    return Make_Export_BMS_andUpload(pFCGI_Rough, Sub_id);
                default:
                    Make_Web_OutPut(104, "The 'export_name' param NOT correct!", NULL, NULL);
                    return FALSE; 
            }

        }
        else if(ParseStr_FromQuery(pFCGI_Rough->szQueryRaw, "cond_system=", szcond_system, 32))
        {               
            num_count = parse_str_to_number(szcond_system, "-", nums, 2);
            Main_id = nums[0];
            Sub_id = nums[1];
            return Make_BMS_BySubPage(pFCGI_Rough, Main_id, Sub_id);
        }

    }
    else
    {
        Make_Web_OutPut(104, "CHG_RECORD page does NOT allow POST!", NULL,  NULL);
        return FALSE;  
    }
    
    return FALSE;
}