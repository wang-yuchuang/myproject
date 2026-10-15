#include "../fcgi_defs.h"

typedef enum tagHisDataQueryDateCond
{
    HISDATA_QUERYCOND_DATE_ALL,
    HISDATA_QUERYCOND_DATE_DAY,
    HISDATA_QUERYCOND_DATE_WEEK,
    HISDATA_QUERYCOND_DATE_MONTH
}HISDATA_QUERYCOND_DATETP;

typedef enum tagHisEventCondTp
{
    HISEV_QUERYCOND_TYPE_ALL,
    HISEV_QUERYCOND_TYPE_SYSTEM_EXIT,
    HISEV_QUERYCOND_TYPE_FIRMWARE_UPDATE,
    HISEV_QUERYCOND_TYPE_IN_MAINTAINING,
    HISEV_QUERYCOND_TYPE_ABILITY_CHANGE,
    HISEV_QUERYCOND_TYPE_GUN_RESERVE,
    HISEV_QUERYCOND_TYPE_DIAGNOSE_UPDT
}HISEV_QUERYCOND_EVTYPETP;

typedef struct tagAPIHisEvQueryConds
{
    int                         nPageId;
    int                         nPageVolume;
    HISDATA_QUERYCOND_DATETP    condDate;
    HISEV_QUERYCOND_EVTYPETP    condEvType;
}HISEV_API_GETCOND;

typedef enum tagHisDataCondSrc
{
    HISDATA_QUERYCOND_SRC_ALL,
    HISDATA_QUERYCOND_SRC_RECT_1,
    HISDATA_QUERYCOND_SRC_RECT_2,
    HISDATA_QUERYCOND_SRC_DCMETER_1,
    HISDATA_QUERYCOND_SRC_DCMETER_2
}HISDATA_QUERYCOND_SRC;

typedef enum tagHisDataCondType
{
    HISDATA_QUERYCOND_DATATP_ALL,
    HISDATA_QUERYCOND_DATATP_VOLT,
    HISDATA_QUERYCOND_DATATP_CURR
}HISDATA_QUERYCOND_DATATP;

typedef struct tagAPIHisDataQueryConds
{
    int                         nPageId;
    int                         nPageVolume;
    HISDATA_QUERYCOND_DATETP    condDate;
    HISDATA_QUERYCOND_SRC       condDataSrc;
    HISDATA_QUERYCOND_DATATP    condDataTp;
}HISDATA_API_GETCOND;

//static char* Get_HisEvType(_EVENT_TYPE tp, OUT char* szOut)
static int Get_HisEvType(_EVENT_TYPE tp)
{
    if(tp == EVENT_TYPE_SYSTEM_EXIT)
    {
        //sprintf(szOut, "System Exit");
        return 1;
    }
    else if(tp == EVENT_TYPE_FIRMWARE_UPDATE)
    {
        //sprintf(szOut, "Firmware Update");
        return 2;
    }
    else if(tp == EVENT_TYPE_DIAGNOSTIC_UPLOAD)
    {
        //sprintf(szOut, "Diagnostic Upload");
        return 3;
    }
    else if(tp == EVENT_TYPE_IN_MAINTAINING)
    {
        //sprintf(szOut, "Enter Maintain Page");
        return 4;
    }
    else if(tp == EVENT_TYPE_ABILITY_CHANGE)
    {
        //sprintf(szOut, "Ability Change");
        return 5;
    }
    else if(tp == EVENT_TYPE_GUN_RESERVE)
    {
        //sprintf(szOut, "Reserve for Charging");
        return 6;
    }
    else
    {
        //sprintf(szOut, "Unkown Event");
        return 7;
    }    

    //return szOut;
}

static char* Get_EventData_Str(WEB_HIS_EVENT* pRec, OUT char* szOut)
{
    _EVENT_TYPE tp = pRec->emEvType;
    if(tp == EVENT_TYPE_FIRMWARE_UPDATE)
    {
        EVENT_FIRMWAREUPDATE_STATUS* pFirmwUpdt = (EVENT_FIRMWAREUPDATE_STATUS*)pRec->pEvData;
        if(pFirmwUpdt->emUpdateStatus == FIRMUPDATE_Downloaded)
        {
           sprintf(szOut, "Downloaded from URL[%s] successfully!", pFirmwUpdt->stUpdateCmd.szURL); 
        }
        else if(pFirmwUpdt->emUpdateStatus == FIRMUPDATE_DownloadFailed)
        {
           sprintf(szOut, "FAILED to download from URL[%s]!", pFirmwUpdt->stUpdateCmd.szURL); 
        }   
        else if(pFirmwUpdt->emUpdateStatus == FIRMUPDATE_Downloading)
        {
           sprintf(szOut, "Downloading from URL[%s]...", pFirmwUpdt->stUpdateCmd.szURL); 
        }    
        else if(pFirmwUpdt->emUpdateStatus == FIRMUPDATE_InstallFailed)
        {
           sprintf(szOut, "Got firmware from URL[%s], but FAILED to install!", pFirmwUpdt->stUpdateCmd.szURL); 
        }   
        else if(pFirmwUpdt->emUpdateStatus == FIRMUPDATE_Installing)
        {
           sprintf(szOut, "Got firmware URL[%s], installing...!", pFirmwUpdt->stUpdateCmd.szURL); 
        }      
        else if(pFirmwUpdt->emUpdateStatus == FIRMUPDATE_Installed)
        {
           sprintf(szOut, "Got firmware from URL[%s], and installed successfully!", pFirmwUpdt->stUpdateCmd.szURL); 
        }   
        else
        {
            sprintf(szOut, "Unkown status for firmware update!");
        }        
    }
    else if(tp == EVENT_TYPE_SYSTEM_EXIT)
    {
        EVENT_SYSTEM_EXIT* pSysExit = (EVENT_SYSTEM_EXIT*)pRec->pEvData;
        // sprintf(szOut, "System is restarted for #%s#!", (pSysExit->nExitType == SYS_EXIT_HARD_REBOOT) ?
        //     "HARD_RESET" : ((pSysExit->nExitType == SYS_EXIT_SOFT_REBOOT) ? "SOFT_RESET" : "FIRMWARE_UPDT"));
        if(pSysExit->nExitType == SYS_EXIT_HARD_REBOOT)
        {
            sprintf(szOut, "System is restarted for #HARD_RESET#!");
        }
        else if(pSysExit->nExitType == SYS_EXIT_SOFT_REBOOT)
        {
            sprintf(szOut, "System is restarted for #SOFT_RESET#!");
        }
        else if(pSysExit->nExitType == SYS_EXIT_FIRMWAREUPDT)
        {
            sprintf(szOut, "System is restarted for #FIRMWARE_UPDT#!");
        }
        else if(pSysExit->nExitType == SYS_INTERNAL_ERROR)
        {
            sprintf(szOut, "System is restarted for #INTERN_ERROR#!");
        }
        /*else if(pSysExit->nExitType == SYS_TIMER_RESET)
        {
            sprintf(szOut, "System is restarted for #TIMER_RESET#!");
        }*/
        else
        {
            sprintf(szOut, "System is restarted for #UNKOWN_REASON#!");
        }
    }
    else if(tp == EVENT_TYPE_DIAGNOSTIC_UPLOAD)
    {
        EVENT_DIAGNOUPLOAD_STATUS* pDiagUpld = (EVENT_DIAGNOUPLOAD_STATUS*)pRec->pEvData;
        if(pDiagUpld->emUploadStatus == DIAGNOUPLOAD_Uploaded)
        {
            sprintf(szOut, "Diagnostic file uploaded successfully!");
        }
        else if(pDiagUpld->emUploadStatus == DIAGNOUPLOAD_UploadFailed)
        {
            sprintf(szOut, "Diagnostic file uploaded FAILED!");
        }
        else if(pDiagUpld->emUploadStatus == DIAGNOUPLOAD_Uploading)
        {
            sprintf(szOut, "Diagnostic file uploading...!");
        }
        else
        {
            sprintf(szOut, "Unkown status for diagnostic upload!");
        }        
    }
    else if(tp == EVENT_TYPE_IN_MAINTAINING)
    {
        sprintf(szOut, "Entered maintain page!");
    }
    else if(tp == EVENT_TYPE_ABILITY_CHANGE)
    {
        EVENT_ABILITY_CHG* pAbChg = (EVENT_ABILITY_CHG*)pRec->pEvData;
        if(pAbChg->nWhichGun == ABILITY_SET_ALL_GUN)
        {
            (pRec->tmEnd == 0) ? sprintf(szOut, "System is set as In-operative!") :
                sprintf(szOut, "System is set back to operative!");
        }
        else
        {
            (pRec->tmEnd == 0) ? sprintf(szOut, "Connecor #%d is set as In-operative!", pAbChg->nWhichGun) :
                sprintf(szOut, "Connecor #%d is set back to operative!", pAbChg->nWhichGun);
        }        
    }
   
    else
    {
        sprintf(szOut, "Unkown Event");
    }    

    return szOut;
}

static BOOL IsRecMatch_DateCond(void* pRec, HISDATA_QUERYCOND_DATETP emCond, BOOL bIsHisEvent)
{
    if(emCond == HISDATA_QUERYCOND_DATE_ALL)
    {
       return TRUE; 
    }
    time_t tmStart = 0;//= Conv_UTC_2LocalTime(pRec->tmBegin);
    if(bIsHisEvent)
    {
        tmStart = Conv_UTC_2LocalTime(((WEB_HIS_EVENT*)pRec)->tmBegin);
    }
    else
    {
        tmStart = Conv_UTC_2LocalTime((time_t)((WEB_HIS_DATA*)pRec)->tmD_RecordTime);
    }
    
    time_t tmNow = Conv_UTC_2LocalTime(_NOW_);
    //int nTmDiff = tmNow - tmStart;
    struct tm gmTime_Start;
    gmtime_r(&tmStart, &gmTime_Start);
    struct tm gmTime_Now;
    gmtime_r(&tmNow, &gmTime_Now);
    if(emCond == HISDATA_QUERYCOND_DATE_DAY)
    {
        if((gmTime_Start.tm_year == gmTime_Now.tm_year) && 
            (gmTime_Start.tm_mon == gmTime_Now.tm_mon) &&
            (gmTime_Start.tm_mday == gmTime_Now.tm_mday))
        {
            return TRUE;
        }
    }
    else if(emCond == HISDATA_QUERYCOND_DATE_WEEK)
    {        
        gmTime_Now.tm_hour = 0;
        gmTime_Now.tm_min = 0;
        gmTime_Now.tm_sec = 0;
        tmNow -= (SEC_PER_DAY * gmTime_Now.tm_wday);
        //gmtime_r(&tmNow, &gmTime_Now);
        if(tmStart >= tmNow)
        {
            return TRUE;
        }
    }
    else if(emCond == HISDATA_QUERYCOND_DATE_MONTH)
    {
        // TRACEX("Year=>(%d, %d), Mon=>(%d, %d)\n", gmTime_Start.tm_year, gmTime_Now.tm_year,
        //     gmTime_Start.tm_mon, gmTime_Now.tm_mon);
        if((gmTime_Start.tm_year == gmTime_Now.tm_year) && 
            (gmTime_Start.tm_mon == gmTime_Now.tm_mon))
        {
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL IsRecMatch_EvTypeCond(WEB_HIS_EVENT* pHisEv, HISEV_QUERYCOND_EVTYPETP emCond)
{
    if(emCond == HISEV_QUERYCOND_TYPE_ALL)
    {
       return TRUE; 
    }
    else if((emCond == HISEV_QUERYCOND_TYPE_SYSTEM_EXIT) &&
        (pHisEv->emEvType == EVENT_TYPE_SYSTEM_EXIT))
    {
        return TRUE;
    }
    else if((pHisEv->emEvType == EVENT_TYPE_FIRMWARE_UPDATE) && 
        (emCond == HISEV_QUERYCOND_TYPE_FIRMWARE_UPDATE))
    {
        return TRUE;
    }
    else if((pHisEv->emEvType == EVENT_TYPE_DIAGNOSTIC_UPLOAD) && 
        (emCond == HISEV_QUERYCOND_TYPE_DIAGNOSE_UPDT))
    {
        return TRUE;
    }
    else if((pHisEv->emEvType == EVENT_TYPE_IN_MAINTAINING) && 
        (emCond == HISEV_QUERYCOND_TYPE_IN_MAINTAINING))
    {
        return TRUE;
    }
    else if((pHisEv->emEvType == EVENT_TYPE_ABILITY_CHANGE) && 
        (emCond == HISEV_QUERYCOND_TYPE_ABILITY_CHANGE))
    {
        return TRUE;
    }
    else if((pHisEv->emEvType == EVENT_TYPE_GUN_RESERVE) && 
        (emCond == HISEV_QUERYCOND_TYPE_GUN_RESERVE))
    {
        return TRUE;
    }

    return FALSE;
}

static void Requery_HisEv_ByConds(FCGI_ROUGH_DATA* pFCGI_Rough, HISEV_API_GETCOND *pCond, OUT WEB_HIS_EVENT* pOutRec, OUT int* pnOutTotal)
{
    int nCurrTotal = pFCGI_Rough->chargerDataRepo.nHisEventTotal;
    WEB_HIS_EVENT* pCurrRec = pFCGI_Rough->chargerDataRepo.hisEvents;
    //default for no filter, copy all
    //memcpy(pOutRec, pCurrRec, nCurrTotal * sizeof(WEB_ALARM_VALUE));
    //*pnOutTotal = nCurrTotal;
    //now calc the real item total using filters
    int n;
    int nFoundRec = 0;
    for(n = 0; n < nCurrTotal; n++, pCurrRec++)
    {
        if(!IsRecMatch_DateCond((void*)pCurrRec, pCond->condDate, TRUE))
        {
            continue;
        }        
        else if(!IsRecMatch_EvTypeCond(pCurrRec, pCond->condEvType))
        {
            continue;
        }  
        //now match all
        memcpy(pOutRec + nFoundRec, pCurrRec, sizeof(WEB_HIS_EVENT));
        nFoundRec++;
    }
    //total found rec
    *pnOutTotal = nFoundRec;
}

static BOOL IsRecMatch_DataSrcCond(WEB_HIS_DATA* pHisData, HISDATA_QUERYCOND_SRC emCond)
{
    if(emCond == HISDATA_QUERYCOND_SRC_ALL)
    {
       return TRUE; 
    }
    else if((emCond == HISDATA_QUERYCOND_SRC_RECT_1) &&
        (pHisData->nDevId == 201))
    {
        return TRUE;
    }
    else if((emCond == HISDATA_QUERYCOND_SRC_RECT_2) &&
        (pHisData->nDevId == 202))
    {
        return TRUE;
    }
    else if((emCond == HISDATA_QUERYCOND_SRC_DCMETER_1) &&
        (pHisData->nDevId == 401))
    {
        return TRUE;
    }
    else if((emCond == HISDATA_QUERYCOND_SRC_DCMETER_2) &&
        (pHisData->nDevId == 402))
    {
        return TRUE;
    }

    return FALSE;
}

static BOOL IsRecMatch_DataTpCond(WEB_HIS_DATA* pHisData, HISDATA_QUERYCOND_DATATP emCond)
{
    if(emCond == HISDATA_QUERYCOND_DATATP_ALL)
    {
       return TRUE; 
    }
    else if((emCond == HISDATA_QUERYCOND_DATATP_VOLT) &&
        ((pHisData->pParamInfo->nParamId == 402) || (pHisData->pParamInfo->nParamId == 104)))
    {
        return TRUE;
    }
    else if((emCond == HISDATA_QUERYCOND_DATATP_CURR) &&
        ((pHisData->pParamInfo->nParamId == 403) || (pHisData->pParamInfo->nParamId == 105)))
    {
        return TRUE;
    }

    return FALSE;
}

static void Requery_HisData_ByConds(FCGI_ROUGH_DATA* pFCGI_Rough, HISDATA_API_GETCOND *pCond, OUT WEB_HIS_DATA* pOutRec, OUT int* pnOutTotal)
{
    int nCurrTotal = pFCGI_Rough->chargerDataRepo.nHisDataTotal;
    WEB_HIS_DATA* pCurrRec = pFCGI_Rough->chargerDataRepo.hisData;
    //default for no filter, copy all
    //memcpy(pOutRec, pCurrRec, nCurrTotal * sizeof(WEB_ALARM_VALUE));
    //*pnOutTotal = nCurrTotal;
    //now calc the real item total using filters
    int n;
    int nFoundRec = 0;
    for(n = 0; n < nCurrTotal; n++, pCurrRec++)
    {
        if(!IsRecMatch_DateCond((void*)pCurrRec, pCond->condDate, FALSE))
        {
            continue;
        }        
        else if(!IsRecMatch_DataSrcCond(pCurrRec, pCond->condDataSrc))
        {
            continue;
        }  
        else if(!IsRecMatch_DataTpCond(pCurrRec, pCond->condDataTp))
        {
            continue;
        }  
        //now match all
        memcpy(pOutRec + nFoundRec, pCurrRec, sizeof(WEB_HIS_DATA));
        nFoundRec++;
    }
    //total found rec
    *pnOutTotal = nFoundRec;
}

static BOOL Make_HisEvent_BySubPage(FCGI_ROUGH_DATA* pFCGI_Rough, HISEV_API_GETCOND *pCond)
{
    static int s_nHisEvTotal = 0;// = pFCGI_Rough->chargerDataRepo.nHisEventTotal;
    static WEB_HIS_EVENT s_hisEvList[WEB_SUPPORT_MAX_HISEVENT];
    WEB_HIS_EVENT* pRec;// = pFCGI_Rough->chargerDataRepo.hisEvents;
    //now judge if need re-query the record
    static HISEV_API_GETCOND s_LastCond;
    static BOOL s_bIsFirstQuery = TRUE;
    BOOL bNeedRequery = FALSE;
    if(s_bIsFirstQuery)
    {
        bNeedRequery = TRUE;
        s_bIsFirstQuery = FALSE;
    }
    else if((s_LastCond.condEvType != pCond->condEvType) ||
            (s_LastCond.condDate != pCond->condDate) ||
            (s_LastCond.nPageVolume != pCond->nPageVolume))
    {
        bNeedRequery = TRUE;
    }
    else if(pCond->nPageId == 0)
    {
        bNeedRequery = TRUE;
    }
    memcpy(&s_LastCond, pCond, sizeof(HISEV_API_GETCOND));
    if(bNeedRequery || pFCGI_Rough->chargerDataRepo.bIsNoticeAPI_ForceRequery_HisEvent)
    {
       //pFCGI_Rough->bNeedUpdateHisData = TRUE; //added by Jimmy 20210318 for convineience of refreshing
       Requery_HisEv_ByConds(pFCGI_Rough, pCond, s_hisEvList, &s_nHisEvTotal);  
       pFCGI_Rough->chargerDataRepo.bIsNoticeAPI_ForceRequery_HisEvent = FALSE;  
    }
      
    int nTotalPage = ((s_nHisEvTotal % pCond->nPageVolume) == 0) ? 
        (s_nHisEvTotal / pCond->nPageVolume) : ((s_nHisEvTotal / pCond->nPageVolume) + 1);
    //now add general info    
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", s_nHisEvTotal);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", nTotalPage);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", pCond->nPageId);
    cJSON* pJRecList = cJSON_CreateArray();
    if(pCond->nPageId + 1 > nTotalPage)
    {
        //do not return false but return empty data
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
        return TRUE;
    }
    
    int nStartRec = pCond->nPageId * pCond->nPageVolume;
    TRACEX("s_nHisEvTotal=%d, nPageId=%d, nPageVolume=%d, nStartRec=%d\n",
        s_nHisEvTotal, pCond->nPageId, pCond->nPageVolume, nStartRec);
    int n;
    char szTime[32];
    char szTemp[256];
    for(n = nStartRec; n < nStartRec + pCond->nPageVolume; n++)
    {        
        if(n >= s_nHisEvTotal)
        {
            break;
        }
        pRec = &s_hisEvList[n];
        cJSON* pJ_ARec = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJ_ARec, "idx", n + 1);
        //cJSON_AddStringToObject(pJ_ARec, "ev_type", Get_HisEvType(pRec->emEvType, szTemp));
        cJSON_AddNumberToObject(pJ_ARec, "ev_type", Get_HisEvType(pRec->emEvType));
        cJSON_AddStringToObject(pJ_ARec, "status", (pRec->tmEnd == 0) ? /*"Started" : "Ended"*/LoadLangText_ById(368) : LoadLangText_ById(369));
        cJSON_AddStringToObject(pJ_ARec, "content", Get_EventData_Str(pRec, szTemp));
        cJSON_AddStringToObject(pJ_ARec, "tm_start", 
            TimeToString(Conv_UTC_2LocalTime(pRec->tmBegin), TIME_CHN_FMT, szTime, 32));
        cJSON_AddStringToObject(pJ_ARec, "tm_end", (pRec->tmEnd == 0) ? "--" : 
            TimeToString(Conv_UTC_2LocalTime(pRec->tmEnd), TIME_CHN_FMT, szTime, 32));
        //add back the item
        cJSON_AddItemToArray(pJRecList, pJ_ARec);
    }

    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
    return TRUE;
}

static char* Get_HisDataSrc(int nDevId, OUT char* szOut)
{
    if(nDevId == 201)
    {
        sprintf(szOut, /*"Connector-1"*/LoadLangText_ById(18));
    }
    else if(nDevId == 202)
    {
        sprintf(szOut, /*"Connector-2"*/LoadLangText_ById(19));
    }
    else if(nDevId == 401)
    {
        sprintf(szOut, /*"kWh-Meter-1"*/LoadLangText_ById(96));
    }
    else if(nDevId == 402)
    {
        sprintf(szOut, /*"kWh-Meter-2"*/LoadLangText_ById(97));
    }
    else
    {
        sprintf(szOut, "Unkown Source");
    }    

    return szOut;
}

static char* Get_HisDataType(int nParamId, OUT char* szOut)
{
    if((nParamId == 402) || (nParamId == 104))
    {
        sprintf(szOut, "%s(V)", /*"Voltage(V)"*/LoadLangText_ById(326));
    }
    else if((nParamId == 403) || (nParamId == 105))
    {
        sprintf(szOut, "%s(A)", /*"Current(A)"*/LoadLangText_ById(364));
    }

    return szOut;
}

static BOOL Make_HisData_BySubPage(FCGI_ROUGH_DATA* pFCGI_Rough, HISDATA_API_GETCOND *pCond)
{
    static int s_nHisDataTotal = 0;// = pFCGI_Rough->chargerDataRepo.nHisDataTotal;
    static WEB_HIS_DATA s_hisDataList[WEB_SUPPORT_MAX_HISDATA];
    WEB_HIS_DATA* pRec;// = pFCGI_Rough->chargerDataRepo.hisData;
    //now judge if need re-query the record
    static HISDATA_API_GETCOND s_LastCond;
    static BOOL s_bIsFirstQuery = TRUE;
    BOOL bNeedRequery = FALSE;
    if(s_bIsFirstQuery)
    {
        bNeedRequery = TRUE;
        s_bIsFirstQuery = FALSE;
    }
    else if((s_LastCond.condDataSrc != pCond->condDataSrc) ||
            (s_LastCond.condDataTp != pCond->condDataTp) ||
            (s_LastCond.condDate != pCond->condDate) ||
            (s_LastCond.nPageVolume != pCond->nPageVolume))
    {
        bNeedRequery = TRUE;
    }
    else if(pCond->nPageId == 0)
    {
        bNeedRequery = TRUE;
    }
    memcpy(&s_LastCond, pCond, sizeof(HISDATA_API_GETCOND));
    if(bNeedRequery || pFCGI_Rough->chargerDataRepo.bIsNoticeAPI_ForceRequery_HisData)
    {
       Requery_HisData_ByConds(pFCGI_Rough, pCond, s_hisDataList, &s_nHisDataTotal);    
       pFCGI_Rough->chargerDataRepo.bIsNoticeAPI_ForceRequery_HisData = FALSE;
    }
      
    int nTotalPage = ((s_nHisDataTotal % pCond->nPageVolume) == 0) ? 
        (s_nHisDataTotal / pCond->nPageVolume) : ((s_nHisDataTotal / pCond->nPageVolume) + 1);
    //now add general info    
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", s_nHisDataTotal);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", nTotalPage);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", pCond->nPageId);
    cJSON* pJRecList = cJSON_CreateArray();
    if(pCond->nPageId + 1 > nTotalPage)
    {
        //do not return false but return empty data
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
        return TRUE;
    }
    
    int nStartRec = pCond->nPageId * pCond->nPageVolume;
    TRACEX("s_nHisDataTotal=%d, nPageId=%d, nPageVolume=%d, nStartRec=%d\n",
        s_nHisDataTotal, pCond->nPageId, pCond->nPageVolume, nStartRec);
    int n;
    char szTime[32];
    char szTemp[256];
    for(n = nStartRec; n < nStartRec + pCond->nPageVolume; n++)
    {        
        if(n >= s_nHisDataTotal)
        {
            break;
        }
        pRec = &s_hisDataList[n];
        cJSON* pJ_ARec = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJ_ARec, "idx", n + 1);
        cJSON_AddStringToObject(pJ_ARec, "tm_start", 
            AcurTimeToString(Conv_UTC_2LocalTime_D(pRec->tmD_RecordTime), TIME_CHN_FMT, szTime, 32));
        cJSON_AddStringToObject(pJ_ARec, "src", Get_HisDataSrc(pRec->nDevId, szTemp));
        cJSON_AddStringToObject(pJ_ARec, "type", Get_HisDataType(pRec->pParamInfo->nParamId, szTemp));
        cJSON_AddFloatToObject(pJ_ARec, "val", pRec->fParamVal, 1);
        //add back the item
        cJSON_AddItemToArray(pJRecList, pJ_ARec);
    }

    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
    return TRUE;
}

BOOL webpage_handler_his_datas(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----HIS_DATAS page_handler was called!---\n");
    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        //his_event(0) or his_data(1)
        int nHisType = 0; 
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "data_type=", &nHisType))
        {
            Make_Web_OutPut(104, "The 'data_type' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        else if((nHisType < 0) || (nHisType > 1))
        {
            Make_Web_OutPut(104, "The 'data_type' is NOT correct!", NULL, NULL);
            return FALSE;  
        }

        //page Id for get
        int nPageId = 0;
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "pageId=", &nPageId))
        {
            Make_Web_OutPut(104, "The 'pageId' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        //page volume
        int nPageVol = 0;
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "page_vol=", &nPageVol))
        {
            Make_Web_OutPut(104, "The 'page_vol' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        else if((nPageVol < 10) || (nPageVol > 1000))
        {
            Make_Web_OutPut(104, "The 'page_vol' shall be in range (10 ~ 1000)!", NULL, NULL);
            return FALSE;            
        }

        //now for his_event
        if(nHisType == 0)
        {
            HISEV_API_GETCOND cond;
            ZERO_POBJS(&cond, 1);
            cond.nPageId = nPageId;
            cond.nPageVolume = nPageVol;
            int nTemp;
            if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "cond_date=", &nTemp))
            {
                if((nTemp < 0) || (nTemp > HISDATA_QUERYCOND_DATE_MONTH))
                {
                    Make_Web_OutPut(104, "The 'cond_date' is OUT of range!", NULL, NULL);
                    return FALSE;  
                }
                cond.condDate = (HISDATA_QUERYCOND_DATETP)nTemp;
            }
            if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "cond_evtype=", &nTemp))
            {
                if((nTemp < 0) || (nTemp > HISEV_QUERYCOND_TYPE_DIAGNOSE_UPDT))
                {
                    Make_Web_OutPut(104, "The 'cond_evtype' is OUT of range!", NULL, NULL);
                    return FALSE;  
                }
                cond.condEvType = (HISEV_QUERYCOND_EVTYPETP)nTemp;
            }

            return Make_HisEvent_BySubPage(pFCGI_Rough, &cond);
        }
        else //his_data
        {
            HISDATA_API_GETCOND cond;
            ZERO_POBJS(&cond, 1);
            cond.nPageId = nPageId;
            cond.nPageVolume = nPageVol;
            int nTemp;
            if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "cond_date=", &nTemp))
            {
                if((nTemp < 0) || (nTemp > HISDATA_QUERYCOND_DATE_MONTH))
                {
                    Make_Web_OutPut(104, "The 'cond_date' is OUT of range!", NULL, NULL);
                    return FALSE;  
                }
                cond.condDate = (HISDATA_QUERYCOND_DATETP)nTemp;
            }
            if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "cond_src=", &nTemp))
            {
                if((nTemp < 0) || (nTemp > HISDATA_QUERYCOND_SRC_DCMETER_2))
                {
                    Make_Web_OutPut(104, "The 'cond_src' is OUT of range!", NULL, NULL);
                    return FALSE;  
                }
                cond.condDataSrc = (HISDATA_QUERYCOND_SRC)nTemp;
            }
            if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "cond_type=", &nTemp))
            {
                if((nTemp < 0) || (nTemp > HISDATA_QUERYCOND_DATATP_CURR))
                {
                    Make_Web_OutPut(104, "The 'cond_type' is OUT of range!", NULL, NULL);
                    return FALSE;  
                }
                cond.condDataTp = (HISDATA_QUERYCOND_DATATP)nTemp;
            }

            return Make_HisData_BySubPage(pFCGI_Rough, &cond);
        }
    }
    else
    {
        Make_Web_OutPut(104, "HIS_DATAS page does NOT allow POST!", NULL,  NULL);
        return FALSE;  
    }

    return TRUE;
}