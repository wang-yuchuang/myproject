#include "../fcgi_defs.h"

static char* Make_TimeZoneStr_FromOffset(float fOffset, OUT char* pszOut)
{
    int nOffset = (int)(fOffset * 2);
    BOOL bHasHalf = (nOffset % 2) ? TRUE : FALSE;
    nOffset /= 2;
    if(bHasHalf)
    {
        (nOffset < 0) ? sprintf(pszOut, "-%02d:30", ABS(nOffset)) : 
            sprintf(pszOut, "+%02d:30", ABS(nOffset));
    }
    else
    {
        (nOffset < 0) ? sprintf(pszOut, "-%02d:00", ABS(nOffset)) : 
            sprintf(pszOut, "+%02d:00", ABS(nOffset));
    }
    
    TRACEX("---the Tobeset_timezone_str {%.1f}->[%s]----\n", fOffset, pszOut);
    return pszOut;
}

BOOL webpage_handler_datetime(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----DATETIME page_handler was called!---\n");
    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        char szTime[32];
        //sys_date_time & time zone
        // cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "utc_time", 
        //     /*TimeToString(Conv_UTC_2LocalTime(_NOW_)*/TimeToString(_NOW_, TIME_CHN_FMT, szTime, 32));
        cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "utc_time", 
            TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTime, 32));    
        cJSON_AddFloatToObject(pFCGI_Rough->pJSON_Resp, "tm_offset", pFCGI_Rough->fTimeOffsetHrs, 1);
    }
    else
    {
        cJSON* pJTimeSet = cJSON_Parse(pFCGI_Rough->szBodyRaw);
        if(pJTimeSet == NULL)
        {
            TRACEX("--The TimeSet string is not a JSON->[%s]----\n", pFCGI_Rough->szBodyRaw);
            Make_Web_OutPut(104, "The 'datetime set' is NOT valid Json string!", NULL, NULL);
            return FALSE;
        }
        cJSON* temp;
        //parse pJTimeSet
        if ((temp = cJSON_GetObjectItem(pJTimeSet, "utc_time")) == NULL)
        {
            Make_Web_OutPut(104, "The 'utc_time' is NOT provided!", NULL, NULL);
            cJSON_Delete(pJTimeSet);
            return FALSE;
        }        
        //now parse the time
        time_t tmSet;
        char szTime[64];
        if(!Parse_ISO8601TimeStrEx(temp->valuestring, &tmSet))
        {
            Make_Web_OutPut(104, "The 'utc_time' is format is NOT correct!", NULL, NULL);
            cJSON_Delete(pJTimeSet);
            return FALSE;      
        }
        
        //parse the timeOffset
        if ((temp = cJSON_GetObjectItem(pJTimeSet, "tm_offset")) == NULL)
        {
            Make_Web_OutPut(104, "The 'tm_offset' is NOT provided!", NULL, NULL);
            cJSON_Delete(pJTimeSet);
            return FALSE;
        }
        float fTmOffSet = (float)temp->valuedouble;
        int nMod = (int)(fTmOffSet * 10);
        if((nMod % 5) != 0)
        {
            Make_Web_OutPut(104, "The 'tm_offset' is WRONG format!", NULL, NULL);
            cJSON_Delete(pJTimeSet);
            return FALSE; 
        }
        else if((fTmOffSet > 12.0) || (fTmOffSet < -12.0))
        {
            Make_Web_OutPut(104, "The 'tm_offset' is OUT of range!", NULL, NULL);
            cJSON_Delete(pJTimeSet);
            return FALSE;    
        }
        //-------------Now set the time & time-zone------------
        //snprintf(szTime, 64, "%s", temp->valuestring);
        //TimeToString(tmSet - (time_t)(fTmOffSet * 3600), TIME_CHN_FMT, szTime, 32);
        //John Modify Set CurrentTime = Time, Not Consider time-zone.2024-05-06.
        TimeToString(tmSet, TIME_CHN_FMT, szTime, 32);

        WEB_PARAM_VALUE* pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 1, 1);
        int nSetCtrl_ExeRtn = Handle_SingleParam_SetCtrl(pParamVal, 
            1, szTime, strlen(szTime));
        if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
        {
            char szErrCode[64];
            snprintf(szErrCode, 64, "Execute time_set errorCode=>%d!", nSetCtrl_ExeRtn);
            Make_Web_OutPut(104, szErrCode, "Time set error!", NULL);
            cJSON_Delete(pJTimeSet);
            return FALSE;
        }

        /*pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 1, 66);
        Make_TimeZoneStr_FromOffset(fTmOffSet, szTime);
        nSetCtrl_ExeRtn = Handle_SingleParam_SetCtrl(pParamVal, 
            1, szTime, strlen(szTime));        
        if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
        {
            char szErrCode[64];
            snprintf(szErrCode, 64, "Execute time_zone-set errorCode=>%d!", nSetCtrl_ExeRtn);
            Make_Web_OutPut(104, szErrCode, "TimeZone set error!", NULL);
            cJSON_Delete(pJTimeSet);
            return FALSE;
        }*/

        cJSON_Delete(pJTimeSet);
    }
    
    return TRUE;
}

