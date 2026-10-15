#include "../fcgi_defs.h"

typedef struct tagAPILocalAuthQueryConds
{
    int                         nPageId;
    int                         nPageVolume;
    char                        szSortKey[64];
}LOCALAUTH_API_GETCOND;

static BOOL IsSortKeyMatch_Auth(AUTHORIZATION_DATA* pAuth, char* szSortKey)
{
    if(strstr(pAuth->szIdTag, szSortKey) != NULL)
    {
        //TRACEX("----the logTYpe is matched--");
        return TRUE;
    }

    return FALSE;
}
/*
static void Requery_LocalList_ByConds(FCGI_ROUGH_DATA* pFCGI_Rough, LOCALAUTH_API_GETCOND *pCond, OUT AUTHORIZATION_DATA* pOutRec, OUT int* pnOutTotal)
{
    int nCurrTotal = pFCGI_Rough->chargerDataRepo.nLocalListItemTotal;
    AUTHORIZATION_DATA* pCurrRec = pFCGI_Rough->chargerDataRepo.authLocalList;
    //default for no filter, copy all
    //memcpy(pOutRec, pCurrRec, nCurrTotal * sizeof(WEB_ALARM_VALUE));
    //*pnOutTotal = nCurrTotal;
    //now calc the real item total using filters
    int n;
    int nFoundRec = 0;
    TRACEX("\nnLocalListItemTotal= %d\n", nCurrTotal);
    for(n = 0; n < nCurrTotal; n++, pCurrRec++)
    {
        if(!IsSortKeyMatch_Auth(pCurrRec, pCond->szSortKey))
        {
            continue;
        }        

        //now match all
        memcpy(pOutRec + nFoundRec, pCurrRec, sizeof(AUTHORIZATION_DATA));
        //TRACEX("-------%d------\n", n);
        nFoundRec++;
    }
    //total found rec
    *pnOutTotal = nFoundRec;
}
*/
/*
static BOOL Make_LocalList_BySubPage(FCGI_ROUGH_DATA* pFCGI_Rough, LOCALAUTH_API_GETCOND *pCond)
{
    //get if LocalList is Diabled
    A_BLOB_OCPP_CONFIGS ocppCfg;
    BOOL bLocalListEnabled = TRUE;
    if(Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 16, sizeof(A_BLOB_OCPP_CONFIGS), (void*)&ocppCfg))
    {
        bLocalListEnabled = ocppCfg.bLocalListEnabled_RW;
        TRACEX("-------The OCPP_LocalList_Enaled = %d------\n", bLocalListEnabled);
    }
    if(!bLocalListEnabled)
    {
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "locallist_enabled", FALSE);
        return TRUE;
    }
    static int s_nLocalListTotal = 0;// = pFCGI_Rough->chargerDataRepo.nSysLogTotal;
    static AUTHORIZATION_DATA s_localList[WEB_SUPPORTED_MAX_LOCALLIST];
    AUTHORIZATION_DATA* pRec;// = pFCGI_Rough->chargerDataRepo.sysLogs;
    //now judge if need re-query the record
    static LOCALAUTH_API_GETCOND s_LastCond;
    static BOOL s_bIsFirstQuery = TRUE;
    BOOL bNeedRequery = FALSE;
    if(s_bIsFirstQuery)
    {
        bNeedRequery = TRUE;
        s_bIsFirstQuery = FALSE;
    }
    else if(strcmp(s_LastCond.szSortKey, pCond->szSortKey) != 0)
    {
        bNeedRequery = TRUE;
    }
    else if(pCond->nPageId == 0)
    {
        bNeedRequery = TRUE;
    }
    memcpy(&s_LastCond, pCond, sizeof(LOCALAUTH_API_GETCOND));
    if(bNeedRequery)
    {
       Requery_LocalList_ByConds(pFCGI_Rough, pCond, &s_localList, &s_nLocalListTotal);    
    }
      
    int nTotalPage = ((s_nLocalListTotal % pCond->nPageVolume) == 0) ? 
        (s_nLocalListTotal / pCond->nPageVolume) : ((s_nLocalListTotal / pCond->nPageVolume) + 1);
    //now add general info    
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "locallist_enabled", TRUE);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", s_nLocalListTotal);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", nTotalPage);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", pCond->nPageId);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "list_version", pFCGI_Rough->chargerDataRepo.nLocalListVersion);
    cJSON* pJRecList = cJSON_CreateArray();
    if(pCond->nPageId + 1 > nTotalPage)
    {
        //do not return false but return empty data
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
        return TRUE;
    }
    
    int nStartRec = pCond->nPageId * pCond->nPageVolume;
    TRACEX("s_nLocalListTotal=%d, nPageId=%d, nPageVolume=%d, nStartRec=%d\n",
        s_nLocalListTotal, pCond->nPageId, pCond->nPageVolume, nStartRec);
    int n;
    char szTime[32];
    for(n = nStartRec; n < nStartRec + pCond->nPageVolume; n++)
    {        
        if(n >= s_nLocalListTotal)
        {
            break;
        }
        pRec = &s_localList[n];
        cJSON* pJ_ARec = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJ_ARec, "idx", n + 1);
        cJSON_AddStringToObject(pJ_ARec, "idTag", pRec->szIdTag);
        cJSON_AddStringToObject(pJ_ARec, "parent_id", pRec->szParentIdTag);
        cJSON_AddStringToObject(pJ_ARec, "status", (pRec->emAuthorizationStatus == AT_Accpted) ? "Accpted" : "Rejected");
        cJSON_AddStringToObject(pJ_ARec, "expired", (pRec->dtExpiryDate == 0) ? "--" :
            TimeToString(Conv_UTC_2LocalTime(pRec->dtExpiryDate), TIME_CHN_FMT, szTime, 32));
        //add back the item
        cJSON_AddItemToArray(pJRecList, pJ_ARec);
    }

    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
    return TRUE;
}
*/

/*
static void Requery_LocalCache_ByConds(FCGI_ROUGH_DATA* pFCGI_Rough, LOCALAUTH_API_GETCOND *pCond, OUT AUTHORIZATION_DATA* pOutRec, OUT int* pnOutTotal)
{
    int nCurrTotal = pFCGI_Rough->chargerDataRepo.nLocalCacheItemTotal;
    AUTHORIZATION_DATA* pCurrRec = pFCGI_Rough->chargerDataRepo.authLocalCache;
    //default for no filter, copy all
    //memcpy(pOutRec, pCurrRec, nCurrTotal * sizeof(WEB_ALARM_VALUE));
    //*pnOutTotal = nCurrTotal;
    //now calc the real item total using filters
    int n;
    int nFoundRec = 0;
    for(n = 0; n < nCurrTotal; n++, pCurrRec++)
    {
        if(!IsSortKeyMatch_Auth(pCurrRec, pCond->szSortKey))
        {
            continue;
        }        

        //now match all
        memcpy(pOutRec + nFoundRec, pCurrRec, sizeof(AUTHORIZATION_DATA));
        nFoundRec++;
    }
    //total found rec
    *pnOutTotal = nFoundRec;
}
*/

/*
static BOOL Make_LocalCache_BySubPage(FCGI_ROUGH_DATA* pFCGI_Rough, LOCALAUTH_API_GETCOND *pCond)
{
    //get if LocalCache is Diabled
    A_BLOB_OCPP_CONFIGS ocppCfg;
    BOOL bLocalCacheEnabled = TRUE;
    if(Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 16, sizeof(A_BLOB_OCPP_CONFIGS), (void*)&ocppCfg))
    {
        bLocalCacheEnabled = ocppCfg.bAuthCacheEnabled_RW;
        TRACEX("-------The OCPP_LocalCache_Enaled = %d------\n", bLocalCacheEnabled);
    }
    if(!bLocalCacheEnabled)
    {
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "localcache_enabled", FALSE);
        return TRUE;
    }
    static int s_nLocalCacheTotal = 0;// = pFCGI_Rough->chargerDataRepo.nSysLogTotal;
    static AUTHORIZATION_DATA s_localCache[WEB_SUPPORTED_MAX_LOCALCACHE];
    AUTHORIZATION_DATA* pRec;// = pFCGI_Rough->chargerDataRepo.sysLogs;
    //now judge if need re-query the record
    static LOCALAUTH_API_GETCOND s_LastCond;
    static BOOL s_bIsFirstQuery = TRUE;
    BOOL bNeedRequery = FALSE;
    if(s_bIsFirstQuery)
    {
        bNeedRequery = TRUE;
        s_bIsFirstQuery = FALSE;
    }
    else if(strcmp(s_LastCond.szSortKey, pCond->szSortKey) != 0)
    {
        bNeedRequery = TRUE;
    }
    else if(pCond->nPageId == 0)
    {
        bNeedRequery = TRUE;
    }
    memcpy(&s_LastCond, pCond, sizeof(LOCALAUTH_API_GETCOND));
    if(bNeedRequery)
    {
       Requery_LocalCache_ByConds(pFCGI_Rough, pCond, &s_localCache, &s_nLocalCacheTotal);    
    }
    int nTotalPage = ((s_nLocalCacheTotal % pCond->nPageVolume) == 0) ? 
        (s_nLocalCacheTotal / pCond->nPageVolume) : ((s_nLocalCacheTotal / pCond->nPageVolume) + 1);
    //now add general info    
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "localcache_enabled", TRUE);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", s_nLocalCacheTotal);
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
    TRACEX("s_nLocalCacheTotal=%d, nPageId=%d, nPageVolume=%d, nStartRec=%d\n",
        s_nLocalCacheTotal, pCond->nPageId, pCond->nPageVolume, nStartRec);
    int n;
    char szTime[32];
    for(n = nStartRec; n < nStartRec + pCond->nPageVolume; n++)
    {        
        if(n >= s_nLocalCacheTotal)
        {
            break;
        }
        pRec = &s_localCache[n];
        cJSON* pJ_ARec = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJ_ARec, "idx", n + 1);
        cJSON_AddStringToObject(pJ_ARec, "idTag", pRec->szIdTag);
        cJSON_AddStringToObject(pJ_ARec, "parent_id", pRec->szParentIdTag);
        cJSON_AddStringToObject(pJ_ARec, "status", (pRec->emAuthorizationStatus == AT_Accpted) ? "Accpted" : "Rejected");
        cJSON_AddStringToObject(pJ_ARec, "expired", (pRec->dtExpiryDate == 0) ? "--" :
            TimeToString(Conv_UTC_2LocalTime(pRec->dtExpiryDate), TIME_CHN_FMT, szTime, 32));
        //add back the item
        cJSON_AddItemToArray(pJRecList, pJ_ARec);
    }

    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
    return TRUE;
}
*/
/*
static BOOL Handle_SetLocalAuth_Enable(FCGI_ROUGH_DATA* pFCGI_Rough, BOOL bEnable, BOOL bIs4LocalList)
{
    A_BLOB_OCPP_CONFIGS ocppCfg;
    //first get back init OCPP values
    WEB_PARAM_VALUE* pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 1, 16);
    if(pParamVal == NULL)
    {
        Make_Web_OutPut(104, "Can NOT get back OCPP init config value!", "Initialize error!", NULL);
        return FALSE;  
    }
    memcpy(&ocppCfg, pParamVal->currValue.blobValue, sizeof(ocppCfg));

    if(bIs4LocalList)
    {
        ocppCfg.bLocalListEnabled_RW = bEnable;
    }
    else
    {
        ocppCfg.bAuthCacheEnabled_RW = bEnable;
    }

    // TRACEX("--------tttttttttt---------\n");
    // return TRUE;

    //now send the cmd for execution
    int nSetCtrl_ExeRtn = Handle_SingleParam_SetCtrl(pParamVal, 1, &ocppCfg, sizeof(ocppCfg));
    if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
    {
        Make_Web_OutPut(104, "Execute set-enable error!", "Execute error!", NULL);
        return FALSE;     
    }

    // LOCALAUTH_API_GETCOND cond;
    // ZERO_POBJS(&cond, 1);
    // if(bIs4LocalList)
    // {
    //     return Make_LocalList_BySubPage(pFCGI_Rough, &cond);
    // }
    // else
    // {
    //     return Make_LocalCache_BySubPage(pFCGI_Rough, &cond);
    // }

    return TRUE;
}

static BOOL Handle_ClearCache(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    char* szValString = "0";
    WEB_PARAM_VALUE* pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_CONTROL, 301, 301);
    if(pParamVal == NULL)
    {
        Make_Web_OutPut(104, "Can NOT get back ClearCache blob!", "Initialize error!", NULL);
        return FALSE;  
    }
    int nSetCtrl_ExeRtn = Handle_SingleParam_SetCtrl(pParamVal, 301, szValString, strlen(szValString));
    if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
    {
        Make_Web_OutPut(104, "Execute ClearCache error!", "Execute error!", NULL);
        return FALSE;     
    }

    return TRUE;
}

static BOOL Handle_LocalListDownload(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    VAR_VALUE varV;
    varV.emValue = 1; //export    
    int nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(
            FALSE, 301, 307, varV.abyValue, 4);
    if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
    {
        Make_Web_OutPut(104, "Execute export locallist error!", "Execute error!", NULL);
        return FALSE;     
    }

    if(!UploadFile_2Client(LOCALLIST_WEB_EXPORT_FILENAME, "Locallist.json"))
    {
        Make_Web_OutPut(104, "Upload process is FAILED!", NULL, NULL);
        return FALSE;      
    }

    unlink(LOCALLIST_WEB_EXPORT_FILENAME);

    return FALSE;
}

static BOOL Handle_LocalListUpload(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    if(!DownloadFile_FromClient(LOCALLIST_WEB_IMPORT_FILENAME))
    {
        Make_Web_OutPut(104, "Download process is FAILED!", NULL, NULL);
        return FALSE;      
    }

    VAR_VALUE varV;
    varV.emValue = 0; //import    
    int nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(
            FALSE, 301, 307, varV.abyValue, 4);
    if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
    {
        Make_Web_OutPut(104, "Execute import locallist error!", "Execute error!", NULL);
        return FALSE;     
    }

    unlink(LOCALLIST_WEB_IMPORT_FILENAME);

    //need notice main app to update the locallist
    pFCGI_Rough->bNeedUpdateLocalList = TRUE;
    return TRUE;
}
*/

BOOL webpage_handler_localauth_mgmt(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----LOCAL_AUTH_MGMT page_handler was called!---\n");
    /*if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        //localList(0) or LocalCache(1)
        int nAuthType = 0; 
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "auth_type=", &nAuthType))
        {
            Make_Web_OutPut(104, "The 'auth_type' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        else if((nAuthType < 0) || (nAuthType > 1))
        {
            Make_Web_OutPut(104, "The 'auth_type' is NOT correct!", NULL, NULL);
            return FALSE;  
        }

        LOCALAUTH_API_GETCOND cond;
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
        char szSSSKey[64];
        if(ParseStr_FromQuery(pFCGI_Rough->szQueryRaw, "sort_key=", szSSSKey, 64))
        {
            //remove the '"' mark
            char* szFFF = Trim_String(Percent_DecodeUrl(szSSSKey), '"');
            if((szFFF != NULL) && strlen(szFFF) >= 3)
            {
                snprintf(cond.szSortKey, 64, "%s", szFFF);
                TRACEX("--the sort_key=>[%s]--\n", cond.szSortKey);  
            } 
        } 

        return (nAuthType == 0) ? 
            Make_LocalList_BySubPage(pFCGI_Rough, &cond) : 
            Make_LocalCache_BySubPage(pFCGI_Rough, &cond);
    }
    else //post
    {
        //operation id
        int nOpId = 0;
#define AUTH_API_OP_SETLOCALLISTENABLE  0
#define AUTH_API_OP_DOWNLOAD_LOCALLIST  1
#define AUTH_API_OP_UPLOAD_LOCALLIST    2
#define AUTH_API_OP_SETLOCALCAHEENABLE  10
#define AUTH_API_OP_CLEARLOCALCACHE     11
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "op=", &nOpId))
        {
            Make_Web_OutPut(104, "The 'op' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        if((AUTH_API_OP_SETLOCALLISTENABLE == nOpId) ||
            (AUTH_API_OP_SETLOCALCAHEENABLE == nOpId))
        {
            BOOL bSetEnable;
            if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "set_enable=", &bSetEnable))
            {
                Make_Web_OutPut(104, "The 'set_enable' NOT provided!", NULL, NULL);
                return FALSE;  
            }
            return Handle_SetLocalAuth_Enable(pFCGI_Rough, bSetEnable, 
                    (AUTH_API_OP_SETLOCALLISTENABLE == nOpId) ? TRUE : FALSE);
        }
        else if(AUTH_API_OP_CLEARLOCALCACHE == nOpId)
        {
            return Handle_ClearCache(pFCGI_Rough);
        }
        else if(AUTH_API_OP_DOWNLOAD_LOCALLIST == nOpId)
        {
            return Handle_LocalListDownload(pFCGI_Rough);
        }
        else if(AUTH_API_OP_UPLOAD_LOCALLIST == nOpId)
        {
            return Handle_LocalListUpload(pFCGI_Rough);
        }
        Make_Web_OutPut(104, "The 'op' is NOT correct!", NULL, NULL);
        return FALSE;  
    }
    */
    return TRUE;
}