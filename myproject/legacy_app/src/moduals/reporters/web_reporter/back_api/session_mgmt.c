#include "fcgi_defs.h"

static SESSION_INFO* FindASession(FCGI_ROUGH_DATA* pFCGIRough)
{
    SESSION_INFO* pSess = NULL;
    int n;
    for(n = 0; n < pFCGIRough->nActiveUserTotal; n++)
    {
        if(strcmp(pFCGIRough->stSessionInfos[n].szToken, pFCGIRough->pStrToken) == 0)
        {
            pSess = &pFCGIRough->stSessionInfos[n];
            break;
        }
    }
    return pSess;
}

int AddNewSession(FCGI_ROUGH_DATA* pFCGIRough, int nUserLv, int nLang, int nMBELoginLevel)
{
    if(pFCGIRough->nActiveUserTotal >= MAX_ALLOW_WEB_USER)
    {
        //return ADDSESS_OK; //FOR TEST PURPOSE ONLY
        return ADDSESS_TOO_MANY_USER;
    }

    //added by Jimmy 20220411 for avoiding frequently login
    int n;
    SESSION_INFO* pSess = NULL;
    for(n = 0; n < pFCGIRough->nActiveUserTotal; n++)
    {
        pSess = &pFCGIRough->stSessionInfos[n];
        int nSecDiff = ABS(pSess->tmLogin - _NOW_);
        if(nSecDiff < 10) //10sec -> avoid login too frequently
        {
            //if the sent from same sender, we judge it came from old request.            
            //if((pFCGIRough->pCurrSess == pSess) &&
            //    (pFCGIRough->pCurrSess->emMBE_UserLevel == nMBELoginLevel))
            if((pSess->emUserLvel == nUserLv) &&
                (pSess->emMBE_UserLevel == nMBELoginLevel) && 
                (pSess->bIsSess_FromNPS == pFCGIRough->bIsWebClient_FromNPS) &&
                (nSecDiff <= 1))
            {
                pFCGIRough->pCurrSess = pSess;
                LogOut_Ex(MAIN_MODULE_WEB, "FCGI_Main", LOG_TYPE_WARING,
                    "REPEATED user[%s(%s),%c] is now LOGIN, now we have %d active users!",
                    (nUserLv == USER_LEVEL_ROOT) ?"Root" : ((nUserLv == USER_LEVEL_ADMIN) ? "Admin" : "Engieer"),
                    pSess->szToken, pFCGIRough->pCurrSess->bIsSess_FromNPS ? 'Y' : 'N',
                    pFCGIRough->nActiveUserTotal);
                return ADDSESS_OK;
            }
            //else not allowed to login too frequently
            return ADDSESS_LOGIN_TOO_FREQUENT;
        }
    }

    pSess = &pFCGIRough->stSessionInfos[pFCGIRough->nActiveUserTotal];    
    ZERO_POBJS(pSess, 1);
    //now init the sesssion info
    //added by Jimmy 20220928
    if(pFCGIRough->bIsWebClient_FromNPS)
    {
        nUserLv = (nMBELoginLevel == MBE_REMOTE_USERLEVEL_0) ? USER_LEVEL_ROOT : 
            ((nMBELoginLevel == MBE_REMOTE_USERLEVEL_1) ? USER_LEVEL_ADMIN : USER_LEVEL_ENGINEER);
    }
    pSess->emUserLvel = nUserLv;
    pSess->tmLogin = _NOW_;
    pFCGIRough->tmLastLogin[nUserLv] = pSess->tmLogin; 
    pSess->pCurrDict = Load_LangResources(pFCGIRough, nLang);
    //sprintf(pSess->szToken, "efdfeagdegeg1111111regeg"); //just for test purpose, will update it later
    double tmNow_f = GetCurrentTime();
    srand((int)_NOW_);  //产生随机数，以防止token重复
    snprintf(pSess->szToken, 64, "%lld-%c%02d", (long long)(tmNow_f * 1000),
        (nUserLv == USER_LEVEL_ROOT) ? 'r' : ((nUserLv == USER_LEVEL_ADMIN) ? 'a' : 'e'),
        rand()%100);
    pFCGIRough->pCurrSess = pSess;
    pFCGIRough->pCurrSess->bIsSess_FromNPS = pFCGIRough->bIsWebClient_FromNPS;
    pFCGIRough->pCurrSess->emMBE_UserLevel = nMBELoginLevel;
    pFCGIRough->nActiveUserTotal++;
    LogOut_Ex(MAIN_MODULE_WEB, "FCGI_Main", LOG_TYPE_MILESTONE,
            "[%s(%s),%c] LOGIN, now have %d active users!",
            (nUserLv == USER_LEVEL_ROOT) ?"Root" : ((nUserLv == USER_LEVEL_ADMIN) ? "Admin" : "Engieer"),
            pSess->szToken, pFCGIRough->pCurrSess->bIsSess_FromNPS ? 'Y' : 'N',
            pFCGIRough->nActiveUserTotal);
    pFCGIRough->bNeedUpdateSysLog = TRUE;
    return ADDSESS_OK;
}

void DeleteSession(FCGI_ROUGH_DATA* pFCGIRough, int nIdx, BOOL bUserLogOut)
{
    if((nIdx > pFCGIRough->nActiveUserTotal - 1) || (nIdx < 0))
    {
        return;
    }

    int n;
    int nUserLv = pFCGIRough->stSessionInfos[nIdx].emUserLvel;
    char szUserToken[64];
    pFCGIRough->stSessionInfos[nIdx].bIsSess_FromNPS = FALSE;
    memcpy(szUserToken, pFCGIRough->stSessionInfos[nIdx].szToken, 64);
    for(n = nIdx; n < pFCGIRough->nActiveUserTotal - 1; n++)
    {
        memcpy(&pFCGIRough->stSessionInfos[n], &pFCGIRough->stSessionInfos[n + 1], 
            sizeof(SESSION_INFO));
    }
    
    if(pFCGIRough->nActiveUserTotal > 0)
    {        
        pFCGIRough->nActiveUserTotal--;
    }
    TRACEX("---deleted a session, now total[%d]---\n", pFCGIRough->nActiveUserTotal);
    
    LogOut_Ex(MAIN_MODULE_WEB, "FCGI_Main", LOG_TYPE_WARING,
            "[%s(%s)] is now %s, now left %d active users!",
            (nUserLv == USER_LEVEL_ROOT) ? "Root" : ((nUserLv == USER_LEVEL_ADMIN) ? "Admin" : "Engieer"),
            szUserToken,
            bUserLogOut ? "LOGOUT" : "OUT-DATED",
            pFCGIRough->nActiveUserTotal);
    pFCGIRough->bNeedUpdateSysLog = TRUE;
}

void Delete_Curr_Session(FCGI_ROUGH_DATA* pFCGIRough)
{
    int nFoundIdx = -1;
    SESSION_INFO* pCurrSess = pFCGIRough->pCurrSess;
    int n;
    for(n = 0; n < pFCGIRough->nActiveUserTotal; n++)
    {
        if(pCurrSess == &pFCGIRough->stSessionInfos[n])
        {
            nFoundIdx = n;
            TRACEX("----found a session to be logout!!!---\n");
            break;
        }
    }
    
    return DeleteSession(pFCGIRough, nFoundIdx, TRUE);
}

BOOL Start_Session(FCGI_ROUGH_DATA* pFCGIRough, A_WEBPAGE_INFO* pPage)
{
    //first check user
    SESSION_INFO* pSess = NULL;
    if((pFCGIRough->pStrToken == NULL) || (strlen(pFCGIRough->pStrToken) < 10)) 
    {
        TRACEX("--the pgeIDD->%d---\n", pPage->emPageId);
        if(pPage->emPageId > WEBPAGE_ID_LOGIN)
        {
            pFCGIRough->pJSON_Resp = NULL;
            Make_Web_OutPut(101, "Token NOT provided!", NULL, NULL);
            return FALSE;
        }
        //Do NOT add session when not login!
        // else
        // {
        //     pSess = AddNewSession(pFCGIRough);
        //     if(pSess == NULL)
        //     {
        //         Make_Web_OutPut(103, "NOT able to creat new user!", NULL);
        //         return FALSE;  
        //     }
        // }
    }
    else
    {
        pSess = FindASession(pFCGIRough);
        TRACEX("---now find the sess=>%d, total_see->%d---\n", pSess, pFCGIRough->nActiveUserTotal);
        if(pSess == NULL)
        {
            if(pPage->emPageId > WEBPAGE_ID_LOGIN)
            {
                pFCGIRough->pJSON_Resp = NULL;
                Make_Web_OutPut(102, "Authorization expired!", NULL, NULL);
                return FALSE;                  
            }
        }
    }        


    //init the page session
    pFCGIRough->pCurrSess = pSess;

    pFCGIRough->pJSON_Resp = cJSON_CreateObject();

    //now run the page_handler
    //Mutex_Lock(pFCGIRough->hDataLock, WAIT_INFINITE);    
    if(Mutex_Lock(pFCGIRough->hDataLock, 5000) != ERR_MUTEX_OK)
    {
        Make_Web_OutPut(301, "LOCK TIMEOUT!", NULL, NULL);
        return FALSE; 
    }

    //record the info 
    if(pFCGIRough->pCurrSess != NULL)
    {
        pFCGIRough->pCurrSess->nCurrRunPage = pPage->emPageId;    
        pFCGIRough->pCurrSess->tmLastReceiveRequest = _NOW_;   
        pFCGIRough->pCurrSess->nNoRecievedRequstCounter = 0; 
        pFCGIRough->pCurrSess->bIsInHandling_HttpRequst = TRUE;
    }

    TRACEX("---now execute the page_handler[%d]---\n",pPage->pfnHandler);
    if(pPage->pfnHandler(pFCGIRough))
    {
        if(pPage->emPageId == WEBPAGE_ID_PCSPARAMSET)
        {
            Make_Web_OutPutEx(0, NULL, NULL, pFCGIRough->pJSON_Resp);
        }
        else
        {
            Make_Web_OutPut(0, NULL, NULL, pFCGIRough->pJSON_Resp);
        }    
    }

    //clear the flag of Handling
    if(pFCGIRough->pCurrSess != NULL)
    {
        pFCGIRough->pCurrSess->tmLastReceiveRequest = _NOW_;   
        pFCGIRough->pCurrSess->nNoRecievedRequstCounter = 0; 
        pFCGIRough->pCurrSess->bIsInHandling_HttpRequst = FALSE;
    }

    Mutex_Unlock(pFCGIRough->hDataLock);
    return TRUE;    
}

void Stop_Session(FCGI_ROUGH_DATA* pFCGIRough)
{
    //delete the json
    if(pFCGIRough->pJSON_Resp != NULL)
    {     
        cJSON_Delete(pFCGIRough->pJSON_Resp);     
    }

}