#include "../fcgi_defs.h"

static BOOL Handle_PasswdChange(FCGI_ROUGH_DATA* pFCGI_Rough, int nLevel, char* szOldPsd, char* szNewPsd)
{
    int n;
    int nLen = strlen(szNewPsd);
    if(nLen > 16)
    {
        Make_Web_OutPut(104, "The new password length is wrong!", "Wrong length", NULL);
        return FALSE;       
    }
    //check new password format
    TRACEX("---the new-psd is[%s]---\n", szNewPsd);
    for(n = 0; n < nLen; n++)
    {
        if(((szNewPsd[n] < '0') || (szNewPsd[n] > '9')) && (szNewPsd[n] != '.'))
        {
            Make_Web_OutPut(104, "The new password format is wrong!", "Wrong format", NULL);
            return FALSE;
        }
    }
    //check if old password match
    char szPassword[32] = {0};
    if(nLevel == USER_LEVEL_ENGINEER)
    {
        if(pFCGI_Rough->pCurrSess->emUserLvel < USER_LEVEL_ADMIN)
        {
            Make_Web_OutPut(104, "Authority denied!", NULL, NULL);
            return FALSE;            
        }
        if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 4, 32, (void*)szPassword))
        {
            Make_Web_OutPut(104, "Can NOT get engineer password!", NULL, NULL);
            return FALSE;
        } 
    }
    else if(nLevel == USER_LEVEL_ADMIN)
    {
        if(pFCGI_Rough->pCurrSess->emUserLvel < USER_LEVEL_ROOT)
        {
            Make_Web_OutPut(104, "Authority denied!", NULL, NULL);
            return FALSE;            
        }
        else if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 24, 32, (void*)szPassword))
        {
            Make_Web_OutPut(104, "Can NOT get admin password!", NULL, NULL);
            return FALSE;
        }         
    }
    else if(nLevel == USER_LEVEL_ROOT)
    {
        //sprintf(szPassword, "123");
        Make_Web_OutPut(104, "Can NOT change password for ROOT!", NULL, NULL);
        return FALSE;
    }

    if(strncmp(szOldPsd, szPassword, 32) != 0)
    {
        Make_Web_OutPut(104, "Password NOT match!", "Password NOT match!", NULL);
        return FALSE;
    }  

    int nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(TRUE, 1, 
        (nLevel == USER_LEVEL_ENGINEER) ? 4 : 24, szNewPsd, 16);
    if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
    {
        Make_Web_OutPut(104, "Execute password change error!", "Execute error!", NULL);
        //TRACEX("---the execute errocCode->%d---\n", nSetCtrl_ExeRtn);
        return FALSE;     
    }

    return TRUE;
}

static BOOL Parse_ChangePSDInfo_FromBody(FCGI_ROUGH_DATA* pFCGI_Rough, int* pnUserLevel, char* szOldPasswd, char* szNewPasswd)
{
    if((pFCGI_Rough->nBodyLen < 3) && (!pFCGI_Rough->stMultiFormData.bHasExternalBody))
    {
        Make_Web_OutPut(104, "NO login information provided!", NULL, NULL);
        return FALSE;
    }

    char* pszFromWhat = pFCGI_Rough->szBodyRaw;
    if(pFCGI_Rough->stMultiFormData.bHasExternalBody) //for multi
    {
        pszFromWhat = pFCGI_Rough->stMultiFormData.szExternBody;
    }
    cJSON* pJChgPsd= cJSON_Parse(pszFromWhat);
    if(pJChgPsd == NULL)
    {
        Make_Web_OutPut(104, "The chg_password info is NOT valid Json string!", NULL, NULL);
        return FALSE;
    }

    cJSON* temp;
    //parse u_lev
    if ((temp = cJSON_GetObjectItem(pJChgPsd, "u_lev")) == NULL)
    {
        Make_Web_OutPut(104, "The 'u_lev' is NOT provided!", NULL, NULL);
        cJSON_Delete(pJChgPsd);
        return FALSE;
    }  
    int nUserLev = /*(temp->type == cJSON_String) ? atoi(temp->valuestring) : */temp->valueint; 
    if(nUserLev < USER_LEVEL_ENGINEER || (nUserLev > USER_LEVEL_ROOT))
    {
        Make_Web_OutPut(104, "The 'u_lev' is NOT correct!", NULL, NULL);
        cJSON_Delete(pJChgPsd);
        return FALSE;
    }
    *pnUserLevel = nUserLev;

    //pase old-passWd
    if ((temp = cJSON_GetObjectItem(pJChgPsd, "old_psd")) == NULL)
    {
        Make_Web_OutPut(104, "The 'old_psd' is NOT provided!", NULL, NULL);
        cJSON_Delete(pJChgPsd);
        return FALSE;
    }  

    if(temp->type != cJSON_String)
    {
        Make_Web_OutPut(104, "The 'old_psd' is NOT a string!", NULL, NULL);
        cJSON_Delete(pJChgPsd);
        return FALSE;
    }
    snprintf(szOldPasswd, 32, "%s",  temp->valuestring);

    //pase new-passWd
    if ((temp = cJSON_GetObjectItem(pJChgPsd, "new_psd")) == NULL)
    {
        Make_Web_OutPut(104, "The 'new_psd' is NOT provided!", NULL, NULL);
        cJSON_Delete(pJChgPsd);
        return FALSE;
    }  

    if(temp->type != cJSON_String)
    {
        Make_Web_OutPut(104, "The 'new_psd' is NOT a string!", NULL, NULL);
        cJSON_Delete(pJChgPsd);
        return FALSE;
    }
    snprintf(szNewPasswd, 32, "%s",  temp->valuestring);

    cJSON_Delete(pJChgPsd);
    return TRUE;    
}

static char* Get_LoginStatus_ByUser(FCGI_ROUGH_DATA* pFCGI_Rough, int nUserLevl, char* pszStatus)
{
    UINT nLCDUserLevel = 0;
    Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 1, 16, 4, (void*)&nLCDUserLevel);
    TRACEX("---Now the login status for LCD is=>%d---\n", nLCDUserLevel);
    int nCurrWebUserLogCount = 0;
    int n;
    for(n = 0; n < pFCGI_Rough->nActiveUserTotal; n++)
    {
        if(pFCGI_Rough->stSessionInfos[n].emUserLvel == nUserLevl)
        {
            nCurrWebUserLogCount++;
        }
    }
    TRACEX("---The WEB loginCount for [%s] is %d---\n", 
        (nUserLevl == USER_LEVEL_ROOT) ?"Root" : ((nUserLevl == USER_LEVEL_ADMIN) ? "Admin" : "Engieer"), 
        nCurrWebUserLogCount);
    nLCDUserLevel -= 1;
    if((nLCDUserLevel != nUserLevl) && (nCurrWebUserLogCount == 0))
    {
        sprintf(pszStatus, "Offline");
    }
    else if((nLCDUserLevel == nUserLevl) && (nCurrWebUserLogCount == 0))
    {
        sprintf(pszStatus, "LCD");
    }
    else if((nLCDUserLevel != nUserLevl) && (nCurrWebUserLogCount > 0))
    {
        sprintf(pszStatus, "WEB(+%d)", nCurrWebUserLogCount);
    }
    else if((nLCDUserLevel == nUserLevl) && (nCurrWebUserLogCount > 0))
    {
        sprintf(pszStatus, "LCD, WEB(+%d)", nCurrWebUserLogCount);
    }

    return pszStatus;
}

BOOL webpage_handler_user_mgmt(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----USER_MGMT page_handler was called!---\n");
    char szLoginStatus[32];
    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        int nUserLevl = pFCGI_Rough->pCurrSess->emUserLvel;
        char szTime[32];
        cJSON* pJLoginInfoArr = cJSON_CreateArray();
        //engineer
        cJSON* pJEngineerLogIn = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJEngineerLogIn, "level", USER_LEVEL_ENGINEER);
        cJSON_AddStringToObject(pJEngineerLogIn, "last_login", (pFCGI_Rough->tmLastLogin[0] == 0) ? "--" : 
            TimeToString(Conv_UTC_2LocalTime(pFCGI_Rough->tmLastLogin[0]), TIME_CHN_FMT, szTime, 32));
        memset(szLoginStatus, 0, 32);
        cJSON_AddStringToObject(pJEngineerLogIn, "login_status", Get_LoginStatus_ByUser(pFCGI_Rough, USER_LEVEL_ENGINEER, szLoginStatus));
        cJSON_AddItemToArray(pJLoginInfoArr, pJEngineerLogIn);
        //admin
        if(nUserLevl >= USER_LEVEL_ADMIN)
        {
            cJSON* pJAdminLogIn = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJAdminLogIn, "level", USER_LEVEL_ADMIN);
            cJSON_AddStringToObject(pJAdminLogIn, "last_login", (pFCGI_Rough->tmLastLogin[1] == 0) ? "--" : 
                TimeToString(Conv_UTC_2LocalTime(pFCGI_Rough->tmLastLogin[1]), TIME_CHN_FMT, szTime, 32));
            memset(szLoginStatus, 0, 32);
            cJSON_AddStringToObject(pJAdminLogIn, "login_status", Get_LoginStatus_ByUser(pFCGI_Rough, USER_LEVEL_ADMIN, szLoginStatus));
            cJSON_AddItemToArray(pJLoginInfoArr, pJAdminLogIn);
            //root
            if(nUserLevl == USER_LEVEL_ROOT)
            {
                cJSON* pJRootLogIn = cJSON_CreateObject();
                cJSON_AddNumberToObject(pJRootLogIn, "level", USER_LEVEL_ROOT);
                cJSON_AddStringToObject(pJRootLogIn, "last_login", (pFCGI_Rough->tmLastLogin[0] == 2) ? "--" : 
                    TimeToString(Conv_UTC_2LocalTime(pFCGI_Rough->tmLastLogin[2]), TIME_CHN_FMT, szTime, 32));
                memset(szLoginStatus, 0, 32);
                cJSON_AddStringToObject(pJRootLogIn, "login_status", Get_LoginStatus_ByUser(pFCGI_Rough, USER_LEVEL_ROOT, szLoginStatus));
                cJSON_AddItemToArray(pJLoginInfoArr, pJRootLogIn);
            }
        }
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "login_infos", pJLoginInfoArr);
        return TRUE;
    }
    else //post
    {
        //added by Jimmy 20220402: only allow the level-0/1 to change passwd
        if(pFCGI_Rough->pCurrSess->bIsSess_FromNPS && 
            (pFCGI_Rough->pCurrSess->emMBE_UserLevel > MBE_REMOTE_USERLEVEL_1))
        {
            Make_Web_OutPut(104, "Operation NOT allowed for Infy-MBE user!", NULL, NULL);
            return FALSE;
        }

        //which user to change passwd
        int nUserLev_2Set = 0;
        // if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "u_lev=", &nUserLev_2Set))
        // {
        //     Make_Web_OutPut(104, "The 'u_lev' NOT provided!", NULL, NULL);
        //     return FALSE;  
        // }
        // else if((nUserLev_2Set < USER_LEVEL_ENGINEER) || (nUserLev_2Set > USER_LEVEL_ROOT)) 
        // {
        //     Make_Web_OutPut(104, "The 'u_lev' is INVALID!", NULL, NULL);
        //     return FALSE;  
        // }
        //old paswd
        char szOldPsd[32] = {0};
        // if(!ParseStr_FromQuery(pFCGI_Rough->szQueryRaw, "old_psd=", szOldPsd, 32))
        // {
        //     Make_Web_OutPut(104, "The 'old_psd' NOT provided!", NULL, NULL);
        //     return FALSE;             
        // }
        //new psd
        char szNewPsd[32] = {0};
        // if(!ParseStr_FromQuery(pFCGI_Rough->szQueryRaw, "new_psd=", szNewPsd, 32))
        // {
        //     Make_Web_OutPut(104, "The 'new_psd' NOT provided!", NULL, NULL);
        //     return FALSE;             
        // }

        if(!Parse_ChangePSDInfo_FromBody(pFCGI_Rough, &nUserLev_2Set, szOldPsd, szNewPsd))
        {
            return FALSE;
        }

        // return Handle_PasswdChange(pFCGI_Rough, nUserLev_2Set, 
        //     Trim_String(Percent_DecodeUrl(szOldPsd), '"'), 
        //     Trim_String(Percent_DecodeUrl(szNewPsd), '"'));
        return Handle_PasswdChange(pFCGI_Rough, nUserLev_2Set, szOldPsd, szNewPsd);
    }

    return TRUE;
}