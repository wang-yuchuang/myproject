#include "../fcgi_defs.h"

typedef enum tagHisAlarmQueryDateCond
{
    HISALM_QUERYCOND_DATE_ALL,
    HISALM_QUERYCOND_DATE_DAY,
    HISALM_QUERYCOND_DATE_WEEK,
    HISALM_QUERYCOND_DATE_MONTH
}HISALM_QUERYCOND_DATETP;

typedef enum tagHISALMQueryDevCond
{
    HISALM_QUERYCOND_DEV_ALL,
    HISALM_QUERYCOND_DEV_SYS,
    HISALM_QUERYCOND_DEV_CCU,
    HISALM_QUERYCOND_DEV_METERS,
    HISALM_QUERYCOND_DEV_CARDREADER,
    HISALM_QUERYCOND_DEV_OTHER
}HISALM_QUERYCOND_DEVTP;

typedef struct tagAPIHisAlamQueryCond
{
    int     nPageId;
    int     nPageVolume;

    //alarm lvel
    int     nAlmLevel; //0 means all
    HISALM_QUERYCOND_DATETP condDate;
    HISALM_QUERYCOND_DEVTP condDevClass; 

}COND_API_HISALARM;

static BOOL IsRecMatch_DateCond(WEB_ALARM_VALUE* pRec, HISALM_QUERYCOND_DATETP emCond)
{
    if(emCond == HISALM_QUERYCOND_DATE_ALL)
    {
       return TRUE; 
    }
    time_t tmStart = Conv_UTC_2LocalTime(pRec->tmLastOccured);
    time_t tmNow = Conv_UTC_2LocalTime(_NOW_);
    //int nTmDiff = tmNow - tmStart;
    struct tm gmTime_Start;
    gmtime_r(&tmStart, &gmTime_Start);
    struct tm gmTime_Now;
    gmtime_r(&tmNow, &gmTime_Now);
    if(emCond == HISALM_QUERYCOND_DATE_DAY)
    {
        if((gmTime_Start.tm_year == gmTime_Now.tm_year) && 
            (gmTime_Start.tm_mon == gmTime_Now.tm_mon) &&
            (gmTime_Start.tm_mday == gmTime_Now.tm_mday))
        {
            return TRUE;
        }
    }
    else if(emCond == HISALM_QUERYCOND_DATE_WEEK)
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
    else if(emCond == HISALM_QUERYCOND_DATE_MONTH)
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

static BOOL IsRecMatch_DevCond(WEB_ALARM_VALUE* pRec, HISALM_QUERYCOND_DEVTP emCond)
{
    if(emCond == HISALM_QUERYCOND_DEV_ALL)
    {
       return TRUE; 
    }
    
    return (pRec->pBelongDev->emDevClass + 1 == (int)emCond) ? TRUE : FALSE;
}

static BOOL IsRecMatch_AlmLevelCond(WEB_ALARM_VALUE* pRec, int nAlmLevel)
{
    if(nAlmLevel == 0) //means all
    {
       return TRUE; 
    }
    
    return (pRec->pAlarmInfo->emLevel == (ALARM_LEVEL)nAlmLevel) ? TRUE : FALSE;
}

static void Requery_HisAlm_ByConds(FCGI_ROUGH_DATA* pFCGI_Rough, COND_API_HISALARM *pCond, OUT WEB_ALARM_VALUE* pOutRec, OUT int* pnOutTotal)
{
    int nCurrTotal = pFCGI_Rough->chargerDataRepo.nHistAlmTotal;
    WEB_ALARM_VALUE* pCurrRec = pFCGI_Rough->chargerDataRepo.histAlms;
    //default for no filter, copy all
    //memcpy(pOutRec, pCurrRec, nCurrTotal * sizeof(WEB_ALARM_VALUE));
    //*pnOutTotal = nCurrTotal;
    //now calc the real item total using filters
    int n;
    int nFoundRec = 0;
    for(n = 0; n < nCurrTotal; n++, pCurrRec++)
    {
        if(!IsRecMatch_DevCond(pCurrRec, pCond->condDevClass))
        {
            continue;
        }
        else if(!IsRecMatch_AlmLevelCond(pCurrRec, pCond->nAlmLevel))
        {
            continue;
        }
        else if(!IsRecMatch_DateCond(pCurrRec, pCond->condDate))
        {
            continue;
        }

        //now match all
        memcpy(pOutRec + nFoundRec, pCurrRec, sizeof(WEB_ALARM_VALUE));
        nFoundRec++;
    }
    //total found rec
    *pnOutTotal = nFoundRec;
}

static char* GetAlmLevel(WEB_ALARM_VALUE* pRec, char* szOut)
{
    if(pRec->pAlarmInfo->emLevel == ALARM_LEVEL_CRITICAL)
    {
        sprintf(szOut, "%s", "Critical Alarm");
    }
    else if(pRec->pAlarmInfo->emLevel == ALARM_LEVEL_MAJOR)
    {
        sprintf(szOut, "%s", "Mayjor Alarm");
    }
    else
    {
        sprintf(szOut, "%s", "Observation Alarm");
    }
    
    return szOut;
}

static BOOL Make_really_HisAlarm_BySubPage(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    WEB_ALARM_VALUE* pRec = pFCGI_Rough->chargerDataRepo.activeAlms;
    cJSON* pJRecList = cJSON_CreateArray();
    int n;
    char szTemp[64];
     for(n = 0; n < pFCGI_Rough->chargerDataRepo.nActiveAlmTotal; n++, pRec++)
    {   
        cJSON* pJ_ARec = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJ_ARec, "idx", n + 1);
        cJSON_AddNumberToObject(pJ_ARec, "level", pRec->pAlarmInfo->emLevel);        
        cJSON_AddStringToObject(pJ_ARec, "level_res", GetAlmLevel(pRec, szTemp));
        cJSON_AddStringToObject(pJ_ARec, "src", LoadLangText_ById(pRec->pBelongDev->nDevName_ResId));
        cJSON_AddStringToObject(pJ_ARec, "name", LoadLangText_ById(pRec->pAlarmInfo->nAlmName_ResId));
        cJSON_AddNumberToObject(pJ_ARec, "code", pRec->pAlarmInfo->nAlarmId);
        cJSON_AddStringToObject(pJ_ARec, "tm_start", 
            TimeToString(Conv_UTC_2LocalTime(pRec->tmLastOccured), TIME_CHN_FMT, szTemp, 32));
        cJSON_AddNumberToObject(pJ_ARec, "status", 0);
        cJSON_AddItemToArray(pJRecList, pJ_ARec);
    }
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);

    return TRUE;
}

static BOOL Make_HisAlarmResult_BySubPage(FCGI_ROUGH_DATA* pFCGI_Rough, COND_API_HISALARM *pCond)
{
    static int s_nHisAlmTotal = 0;// = pFCGI_Rough->chargerDataRepo.nHistAlmTotal;
    static WEB_ALARM_VALUE s_hisAlmList[WEB_MAX_SUPPORT_HISTORYALARM];
    WEB_ALARM_VALUE* pRec;// = pFCGI_Rough->chargerDataRepo.histAlms;
        //now judge if need re-query the record
    static COND_API_HISALARM s_LastCond;
    static BOOL s_bIsFirstQuery = TRUE;
    BOOL bNeedRequery = FALSE;
    if(s_bIsFirstQuery)
    {
        bNeedRequery = TRUE;
        s_bIsFirstQuery = FALSE;
    }
    else if((s_LastCond.nAlmLevel != pCond->nAlmLevel) ||
            (s_LastCond.condDate != pCond->condDate) ||
            (s_LastCond.nPageVolume != pCond->nPageVolume) ||
            (s_LastCond.condDevClass != pCond->condDevClass))
    {
        bNeedRequery = TRUE;
    }
    else if(pCond->nPageId == 0)
    {
        bNeedRequery = TRUE;
    }
    memcpy(&s_LastCond, pCond, sizeof(COND_API_HISALARM));
    if(bNeedRequery)
    {
       Requery_HisAlm_ByConds(pFCGI_Rough, pCond, &s_hisAlmList, &s_nHisAlmTotal);    
    }
      
    int nTotalPage = ((s_nHisAlmTotal % pCond->nPageVolume) == 0) ? 
        (s_nHisAlmTotal / pCond->nPageVolume) : ((s_nHisAlmTotal / pCond->nPageVolume) + 1);
    //now add general info    
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", s_nHisAlmTotal);
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
    TRACEX("s_nHisAlmTotal=%d, nPageId=%d, nPageVolume=%d, nStartRec=%d\n",
        s_nHisAlmTotal, pCond->nPageId, pCond->nPageVolume, nStartRec);
    int n;
    char szTemp[64];
    int  nHours;
    int  nMinutes;
    int  nSeconds;
    int  nDuration;
    for(n = nStartRec; n < nStartRec + pCond->nPageVolume; n++)
    {   

        if(n >= s_nHisAlmTotal)
        {
            break;
        }

        pRec = &s_hisAlmList[n];

        if(pRec->tmLastCeased == 0)
        {
            continue;
        } 

        cJSON* pJ_ARec = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJ_ARec, "idx", n + 1);
        cJSON_AddNumberToObject(pJ_ARec, "level", pRec->pAlarmInfo->emLevel);        
        cJSON_AddStringToObject(pJ_ARec, "level_res", GetAlmLevel(pRec, szTemp));
        //cJSON_AddNumberToObject(pJ_ARec, "chgmode", pRec->extraRecData.runData.emChargeMode);
        cJSON_AddStringToObject(pJ_ARec, "src", LoadLangText_ById(pRec->pBelongDev->nDevName_ResId));
        cJSON_AddStringToObject(pJ_ARec, "name", LoadLangText_ById(pRec->pAlarmInfo->nAlmName_ResId));
        cJSON_AddNumberToObject(pJ_ARec, "code", pRec->pAlarmInfo->nAlarmId);
        cJSON_AddNumberToObject(pJ_ARec, "status", 2);
        //cJSON_AddNumberToObject(pJ_ARec, "operation", 0);

        cJSON_AddStringToObject(pJ_ARec, "tm_start", 
            TimeToString(Conv_UTC_2LocalTime(pRec->tmLastOccured), TIME_CHN_FMT, szTemp, 32));
        cJSON_AddStringToObject(pJ_ARec, "tm_end", (pRec->tmLastCeased == 0) ? "--" : 
            TimeToString(Conv_UTC_2LocalTime(pRec->tmLastCeased), TIME_CHN_FMT, szTemp, 32));

        nDuration = pRec->tmLastCeased - pRec->tmLastOccured;
        nHours = nDuration / 3600;
        nMinutes = (nDuration % 3600) / 60;
        nSeconds = nDuration % 60;

        snprintf(szTemp, 64, "%.2d:%.2d:%.2d", nHours, nMinutes, nSeconds);
        cJSON_AddStringToObject(pJ_ARec, "duration", szTemp);    

        //add back the item
        cJSON_AddItemToArray(pJRecList, pJ_ARec);
    }

    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
    return TRUE;
}

BOOL webpage_handler_alarm(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    //printf("\n----HIS_ALARM page_handler was called!---\n");
    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        COND_API_HISALARM cond;
        ZERO_POBJS(&cond, 1);
        //page Id for get
        int ntp = -1;
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "type=", &ntp))
        {
            Make_Web_OutPut(104, "The 'type' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        if(ntp == 0)
        {
             return Make_really_HisAlarm_BySubPage(pFCGI_Rough);
        }

        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "pageId=", &cond.nPageId))
        {
            Make_Web_OutPut(104, "The 'pageId' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        //page volume
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "page_vol=", &cond.nPageVolume))
        {
            Make_Web_OutPut(104, "The 'page_vol' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        else if((cond.nPageVolume < 10) || (cond.nPageVolume > 1000))
        {
            Make_Web_OutPut(104, "The 'page_vol' shall be in range (10 ~ 1000)!", NULL, NULL);
            return FALSE;            
        }
        //now the query conditions
        int nTemp = -1;        
        if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "cond_date=", &nTemp))
        {
            if((nTemp < 0) || (nTemp > HISALM_QUERYCOND_DATE_MONTH))
            {
                Make_Web_OutPut(104, "The 'cond_date' is OUT of range!", NULL, NULL);
                return FALSE;  
            }
            cond.condDate = (HISALM_QUERYCOND_DATETP)nTemp;
        }
        if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "cond_dev=", &nTemp))
        {
            if((nTemp < 0) || (nTemp > HISALM_QUERYCOND_DEV_OTHER))
            {
                Make_Web_OutPut(104, "The 'cond_dev' is OUT of range!", NULL, NULL);
                return FALSE;  
            }
            cond.condDevClass = (HISALM_QUERYCOND_DEVTP)nTemp;
        }
        if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "cond_level=", &nTemp))
        {
            if((nTemp < 0) || (nTemp > ALARM_LEVEL_CRITICAL))
            {
                Make_Web_OutPut(104, "The 'cond_level' is OUT of range!", NULL, NULL);
                return FALSE;  
            }
            cond.nAlmLevel = nTemp;
        }
        // if(ntp == 0)
        // {
        //     return Make_really_HisAlarm_BySubPage(pFCGI_Rough);
        // }
        // else
        // {
        return Make_HisAlarmResult_BySubPage(pFCGI_Rough, &cond);
        // }
    }
    else
    {
        Make_Web_OutPut(104, "HIS_ALARM page does NOT allow POST!", NULL,  NULL);
        return FALSE;  
    }

    return TRUE;
}