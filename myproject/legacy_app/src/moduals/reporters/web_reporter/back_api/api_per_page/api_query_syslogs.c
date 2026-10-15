#include "../fcgi_defs.h"

typedef struct tagAPILogsQueryConds
{
    int                         nPageId;
    int                         nPageVolume;
    char                        szSortKey[64];
}SYSLOGS_API_GETCOND;

static char* Get_SysLogType(LOG_TYPE tp, OUT char* szOut, OUT char* szFColor)
{
    if(tp == LOG_TYPE_INFO)
    {
        sprintf(szOut, "LOG_INFO");
        sprintf(szFColor, "#1890FF");
    }
    else if(tp == LOG_TYPE_MILESTONE)
    {
        sprintf(szOut, "LOG_MILESTONE");
        sprintf(szFColor, "#771CAA");
    }
    else if(tp == LOG_TYPE_WARING)
    {
        sprintf(szOut, "LOG_WARNING");
        sprintf(szFColor, "#CCCC11");
    }
    else 
    {
        sprintf(szOut, "LOG_ERROR");
        sprintf(szFColor, "#F01000");
    }

    return szOut;
}

static BOOL IsSortKeyMatch_SysLog(WEB_SYSLOG* pRec, char* szSortKey)
{
    char szMatchStr[64];
    char szFColor[32];
    //from logTYpe
    Get_SysLogType(pRec->emLogType, szMatchStr, szFColor);
    if(strstr(szMatchStr, szSortKey) != NULL)
    {
        //TRACEX("----the logTYpe is matched--");
        return TRUE;
    }
    //from time
    AcurTimeToString(Conv_UTC_2LocalTime_D(pRec->fLogTime), TIME_CHN_FMT, szMatchStr, 32);
    if(strstr(szMatchStr, szSortKey) != NULL)
    {
        //TRACEX("----the fLogTime is matched--");
        return TRUE;
    }
    //from content
    if(strstr(pRec->szLog, szSortKey) != NULL)
    {
        //TRACEX("----the szLog is matched--");
        return TRUE;
    }

    return FALSE;
}


static void Requery_SysLog_ByConds(FCGI_ROUGH_DATA* pFCGI_Rough, SYSLOGS_API_GETCOND *pCond, OUT WEB_SYSLOG* pOutRec, OUT int* pnOutTotal)
{
    int nCurrTotal = pFCGI_Rough->chargerDataRepo.nSysLogTotal;
    WEB_SYSLOG* pCurrRec = pFCGI_Rough->chargerDataRepo.sysLogs;
    //default for no filter, copy all
    //memcpy(pOutRec, pCurrRec, nCurrTotal * sizeof(WEB_ALARM_VALUE));
    //*pnOutTotal = nCurrTotal;
    //now calc the real item total using filters
    int n;
    int nFoundRec = 0;
    for(n = 0; n < nCurrTotal; n++, pCurrRec++)
    {
        if(!IsSortKeyMatch_SysLog(pCurrRec, pCond->szSortKey))
        {
            continue;
        }        

        //now match all
        memcpy(pOutRec + nFoundRec, pCurrRec, sizeof(WEB_SYSLOG));
        nFoundRec++;
    }
    //total found rec
    *pnOutTotal = nFoundRec;
    TRACEX("---requery syslog, Total: %d, found: %d----\n", nCurrTotal, nFoundRec);
}


static BOOL Make_SysLog_BySubPage(FCGI_ROUGH_DATA* pFCGI_Rough, SYSLOGS_API_GETCOND *pCond, BOOL bHasSortKey)
{
    static int s_nSysLogTotal = 0;// = pFCGI_Rough->chargerDataRepo.nSysLogTotal;
    static WEB_SYSLOG s_sysLogList[WEB_MAX_SUPPORT_HISTORYALARM];
    WEB_SYSLOG* pRec;// = pFCGI_Rough->chargerDataRepo.sysLogs;
    //now judge if need re-query the record
    static SYSLOGS_API_GETCOND s_LastCond;
    static BOOL s_bIsFirstQuery = TRUE;
    BOOL bNeedRequery = FALSE;
    if(s_bIsFirstQuery)
    {
        bNeedRequery = TRUE;
        s_bIsFirstQuery = FALSE;
    }
    else if(strncmp(s_LastCond.szSortKey, pCond->szSortKey, 64) != 0)
    {
        bNeedRequery = TRUE;
    }
    memcpy(&s_LastCond, pCond, sizeof(SYSLOGS_API_GETCOND));
    if(bNeedRequery || (pCond->nPageId == 0))
    {
       Requery_SysLog_ByConds(pFCGI_Rough, pCond, s_sysLogList, &s_nSysLogTotal);    
    }
      
    int nTotalPage = ((s_nSysLogTotal % pCond->nPageVolume) == 0) ? 
        (s_nSysLogTotal / pCond->nPageVolume) : ((s_nSysLogTotal / pCond->nPageVolume) + 1);
    //now add general info    
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", s_nSysLogTotal);
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
    TRACEX("s_nSysLogTotal=%d, nPageId=%d, nPageVolume=%d, nStartRec=%d\n",
        s_nSysLogTotal, pCond->nPageId, pCond->nPageVolume, nStartRec);
    int n;
    char szTime[32];
    char szLogTp[32];
    char szLLLLOg[512];
    char szFColor[32] = {0};
    for(n = nStartRec; n < nStartRec + pCond->nPageVolume; n++)
    {        
        if(n >= s_nSysLogTotal)
        {
            break;
        }
        pRec = &s_sysLogList[n];
        cJSON* pJ_ARec = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJ_ARec, "idx", n + 1);
        //now make the content
        Get_SysLogType(pRec->emLogType, szLogTp, szFColor);
        snprintf(szLLLLOg, 512, "%04d&nbsp;&nbsp;%s&nbsp;&nbsp;<font color=\"%s\">%s</font>&nbsp;&nbsp;%s", pRec->nLogId, 
            AcurTimeToString(Conv_UTC_2LocalTime_D(pRec->fLogTime), TIME_CHN_FMT, szTime, 32),
            (strlen(szFColor) < 2) ? "red" : szFColor, szLogTp,
            pRec->szLog);
        cJSON_AddStringToObject(pJ_ARec, "content", szLLLLOg);
        //add back the item
        cJSON_AddItemToArray(pJRecList, pJ_ARec);
    }

    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
    return TRUE;
}

static char* Get_OCPPLogType(DUMP_TYPE tp, OUT char* szOut, OUT char* szFColor)
{
    if(tp == DUMP_TYPE_OPEN_COMM)
    {
        sprintf(szOut, "WS_OPEN");
        sprintf(szFColor, "#05F000");
    }
    else if(tp == DUMP_TYPE_CLOSE_COMM)
    {
        sprintf(szOut, "WS_CLOSE");
        sprintf(szFColor, "#F00500");
    }
    else if(tp == DUMP_TYPE_INFO)
    {
        sprintf(szOut, "WS_INFO");
        sprintf(szFColor, "#F0F005");
    }
    else if(tp == DUMP_TYPE_READ)
    {
        sprintf(szOut, "WS_READ");
        sprintf(szFColor, "#1890FF");
    }
    else if(tp == DUMP_TYPE_WRITE)
    {
        sprintf(szOut, "WS_WRITE");
        sprintf(szFColor, "#771CAA");
    }

    return szOut;
}


static BOOL IsSortKeyMatch_OCPPLog(WEB_OCPPLOG* pRec, char* szSortKey)
{
    char szMatchStr[64];
    char szFColor[32];
    //from logTYpe
    Get_OCPPLogType(pRec->emDumpType, szMatchStr, szFColor);
    if(strstr(szMatchStr, szSortKey) != NULL)
    {
        //TRACEX("----the logTYpe is matched--");
        return TRUE;
    }
    //from time
    AcurTimeToString(Conv_UTC_2LocalTime_D(pRec->fLogTime), TIME_CHN_FMT, szMatchStr, 32);
    if(strstr(szMatchStr, szSortKey) != NULL)
    {
        //TRACEX("----the fLogTime is matched--");
        return TRUE;
    }
    //from content
    if(strstr(pRec->szMsg, szSortKey) != NULL)
    {
        //TRACEX("----the szMsg is matched--");
        return TRUE;
    }

    return FALSE;
}


static void Requery_OCPPLog_ByConds(FCGI_ROUGH_DATA* pFCGI_Rough, SYSLOGS_API_GETCOND *pCond, OUT WEB_OCPPLOG* pOutRec, OUT int* pnOutTotal)
{
    int nCurrTotal = pFCGI_Rough->chargerDataRepo.nOCPPLogTotal;
    WEB_OCPPLOG* pCurrRec = pFCGI_Rough->chargerDataRepo.ocppLogs;
    //default for no filter, copy all
    //memcpy(pOutRec, pCurrRec, nCurrTotal * sizeof(WEB_ALARM_VALUE));
    //*pnOutTotal = nCurrTotal;
    //now calc the real item total using filters
    int n;
    int nFoundRec = 0;
    for(n = 0; n < nCurrTotal; n++, pCurrRec++)
    {
        if(!IsSortKeyMatch_OCPPLog(pCurrRec, pCond->szSortKey))
        {
            continue;
        }        

        //now match all
        memcpy(pOutRec + nFoundRec, pCurrRec, sizeof(WEB_OCPPLOG));
        nFoundRec++;
    }
    //total found rec
    *pnOutTotal = nFoundRec;
}


static BOOL Make_OCPPLog_BySubPage(FCGI_ROUGH_DATA* pFCGI_Rough, SYSLOGS_API_GETCOND *pCond, BOOL bHasSortKey)
{
    static int s_nOCPPLogTotal = 0;// = pFCGI_Rough->chargerDataRepo.nOCPPLogTotal;
    static WEB_OCPPLOG s_OCPPLogList[WEB_SUPPORT_MAX_OCPPLOG];
    WEB_OCPPLOG* pRec;// = pFCGI_Rough->chargerDataRepo.ocppLogs;
    //now judge if need re-query the record
    static SYSLOGS_API_GETCOND s_LastCond;
    static BOOL s_bIsFirstQuery = TRUE;
    BOOL bNeedRequery = FALSE;
    if(s_bIsFirstQuery)
    {
        bNeedRequery = TRUE;
        s_bIsFirstQuery = FALSE;
    }
    else if(strncmp(s_LastCond.szSortKey, pCond->szSortKey, 64) != 0)
    {
        bNeedRequery = TRUE;
    }
    memcpy(&s_LastCond, pCond, sizeof(SYSLOGS_API_GETCOND));
    if(bNeedRequery || (pCond->nPageId == 0))
    {
       Requery_OCPPLog_ByConds(pFCGI_Rough, pCond, s_OCPPLogList, &s_nOCPPLogTotal);    
    }
      
    int nTotalPage = ((s_nOCPPLogTotal % pCond->nPageVolume) == 0) ? 
        (s_nOCPPLogTotal / pCond->nPageVolume) : ((s_nOCPPLogTotal / pCond->nPageVolume) + 1);
    //now add general info    
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", s_nOCPPLogTotal);
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
    TRACEX("s_nOCPPLogTotal=%d, nPageId=%d, nPageVolume=%d, nStartRec=%d\n",
        s_nOCPPLogTotal, pCond->nPageId, pCond->nPageVolume, nStartRec);
    int n;
    char szTime[32];
    char szLogTp[32];
    char szLLLLOg[2048];
    char szFColor[32] = {0};
    for(n = nStartRec; n < nStartRec + pCond->nPageVolume; n++)
    {        
        if(n >= s_nOCPPLogTotal)
        {
            break;
        }
        //pRec = &s_OCPPLogList[n];
        pRec = &s_OCPPLogList[s_nOCPPLogTotal - n]; //Jimmy: we need reverse the oder because original rec is DESC
        cJSON* pJ_ARec = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJ_ARec, "idx", n + 1);
        //now make the content
        Get_OCPPLogType(pRec->emDumpType, szLogTp, szFColor);
        snprintf(szLLLLOg, 2048, "%04d&nbsp;&nbsp;%s&nbsp;&nbsp;<font color=\"%s\">%s</font>&nbsp;&nbsp;%s", pRec->nLogId, 
            AcurTimeToString(Conv_UTC_2LocalTime_D(pRec->fLogTime), TIME_CHN_FMT, szTime, 32),
            (strlen(szFColor) < 2) ? "red" :  szFColor, szLogTp, 
            pRec->szMsg);
        cJSON_AddStringToObject(pJ_ARec, "content", szLLLLOg);
        //add back the item
        cJSON_AddItemToArray(pJRecList, pJ_ARec);
    }

    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
    return TRUE;
}

BOOL webpage_handler_sys_logs(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----SYS_LOGS page_handler was called!---\n");
    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        //syslog(0) or OCPPlog(1)
        int nLogType = 0; 
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "log_type=", &nLogType))
        {
            Make_Web_OutPut(104, "The 'log_type' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        else if((nLogType < 0) || (nLogType > 1))
        {
            Make_Web_OutPut(104, "The 'log_type' is NOT correct!", NULL, NULL);
            return FALSE;  
        }

        SYSLOGS_API_GETCOND cond;
        ZERO_POBJS(&cond, 1);
        //page Id for get
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

        //now parse sort_key
        BOOL bHasSortKey = FALSE;
        char szSSSKey[64];
        if(ParseStr_FromQuery(pFCGI_Rough->szQueryRaw, "sort_key=", szSSSKey, 64))
        {
            //remove the '"' mark
            char* szFFF = Trim_String(Percent_DecodeUrl(szSSSKey), '"');
            if((szFFF != NULL) && strlen(szFFF) >= 3)
            {
                snprintf(cond.szSortKey, 64, "%s", szFFF);
                TRACEX("--the sort_key=>[%s]--\n", cond.szSortKey);   
                bHasSortKey = TRUE;      
            } 
        } 

        return (nLogType == 0) ? 
            Make_SysLog_BySubPage(pFCGI_Rough, &cond, bHasSortKey) : 
            Make_OCPPLog_BySubPage(pFCGI_Rough, &cond, bHasSortKey);
    }
    else
    {
        Make_Web_OutPut(104, "SYS_LOGS page does NOT allow POST!", NULL,  NULL);
        return FALSE;  
    }

    return TRUE;
}