#include "fcgi_defs.h"
//global variables
static BOOL             s_bIsNeedKillSelf = FALSE;
static FCGI_ROUGH_DATA  s_FCGIRoughData;
static PAGES_INFO       s_WebPagesInfo;


#define DEF_WEBPAGE_INFO(_pPages, _pageId, _pageName) \
            do{ \
                (_pPages)->pageInfos[(_pPages)->nTotalPages].emPageId = _pageId; \      
                snprintf((_pPages)->pageInfos[(_pPages)->nTotalPages].szName, 32, "%s", #_pageName);\
                (_pPages)->pageInfos[(_pPages)->nTotalPages].pfnHandler = webpage_handler_##_pageName; \                
                TRACEX("---now init page[%d], name->(%s)---\n", _pageId, (_pPages)->pageInfos[(_pPages)->nTotalPages].szName); \
                (_pPages)->nTotalPages++; \
            }while(0)


static BOOL Init_FCGI(void)
{
    //init all variables
    s_bIsNeedKillSelf = FALSE;
    ZERO_POBJS(&s_FCGIRoughData, 1);
    ZERO_POBJS(&s_WebPagesInfo, 1);
    //NOW init all web info
    PAGES_INFO* p = &s_WebPagesInfo;
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_INIT,          init              );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_LOGIN,         login             );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_DATETIME,      datetime          );    
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_HOME,          home              );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_QUICKSET,      quick_set         );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_DETAILINFO,    detail_info       );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_PARAMSET,      param_sett        );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_PCSPARAMSET,   pcs_param_sett    );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_MANCTRL,       manu_ctrl         );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_CHGRECORD,     operationlog        );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_HISALARMS,        alarm             );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_SYSLOGS,       sys_logs          );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_HISDATAS,      his_datas         );
    //DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_LOCALAUTHMGMT, localauth_mgmt    );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_DATABACREST,   data_bakandrest   );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_FIRMWAREMGMT,  firmware_mgmt     );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_USERMGMT,      user_mgmt         );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_CHGPROFILEMGMT,chgprofile_mgmt   );
	 DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_RUNINFO,       run_info          );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_ABOUT,         about_info        );//edit by Kuangyue
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_PCSHOME,       pcs_home          );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_PCSHISALARMS,  pcs_his_alarm     );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_PCSRUNINFO,    pcs_run_info      );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_PCS_FIRMWAREMGMT,    pcs_firmware_mgmt      );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_CELLBARCODE,   cell_barcode      );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_EXPORT_PCS,    export_pcs        );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_PARAMREREAD,   param_reread      );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_BMSPARAM,      bmsparam_sett      );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_BMSFAULT,      bmsfault           );
    DEF_WEBPAGE_INFO(p,     WEBPAGE_ID_BMSEVENT,      bmsevent           );
    //Sleep_Ex(5000); //just for testing purpose
    // while(!s_bIsNeedKillSelf)
    // {
    //     //connect to the app_serv, using nnmsg
    // }
    //return TRUE;
    TRACE("Start Init DataExchange !\n");
    s_FCGIRoughData.p_bIsNeedExit = &s_bIsNeedKillSelf;
    s_FCGIRoughData.hDataLock = Mutex_Create(TRUE);
    return Init_DataExchange(&s_FCGIRoughData);
}

static void CleanUp_FCGI()
{
    s_bIsNeedKillSelf = TRUE;
    //now wait data_com thread to exit
    //and clean all data
    Clean_DataExchange(&s_FCGIRoughData);
    TRACEX("\n------Now FCGI is exiting.....\n");
}

void Make_Web_OutPut(int nErrCode, char* szDebugErrMsg, char* szDisplayErrTip, cJSON* pJsData)
{    
    WEB_OUT("Content-type: application/json\r\n\r\n");
    if(nErrCode > 0)
    {
        //TRACEX("nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn\n");
        // WEB_OUT("{\"errorCode\": %d, \"errorMsg\":\"%s\", \"errorDebug\":\"%s\"}\r\n", 
        //     nErrCode, ((szDisplayErrTip == NULL) ? "null" : szDisplayErrTip));
        if(szDebugErrMsg == NULL)
        {
            if(szDisplayErrTip == NULL)
            {
                WEB_OUT("{\"errorCode\": %d, \"errorMsg\":null}\r\n", nErrCode);
            }
            else
            {
                WEB_OUT("{\"errorCode\": %d, \"errorMsg\":\"%s\"}\r\n", 
                    nErrCode, szDisplayErrTip);
            }
        }
        else
        {
            if(szDisplayErrTip == NULL)
            {
                WEB_OUT("{\"errorCode\": %d, \"errorMsg\":null, \"errorDebug\":\"%s\"}\r\n", 
                    nErrCode, szDebugErrMsg);
            }
            else
            {
                WEB_OUT("{\"errorCode\": %d, \"errorMsg\":\"%s\", \"errorDebug\":\"%s\"}\r\n", 
                    nErrCode, szDisplayErrTip, szDebugErrMsg);
            }       
        }        
    }
    else
    {
        //WEB_OUT("{\"data\":{%s}}\r\n", (pJsData == NULL) ? "null" : cJSON_Print(pJsData));
        if(pJsData == NULL)
        {
            WEB_OUT("{\"data\":{null}}\r\n");
        }
        else 
        {
            cJSON *json = pJsData;
            char *szData = cJSON_Print_Formatted(pJsData);
            //WEB_OUT("{\"data\":%s}\r\n", szData);
            cJSON *tabs = cJSON_GetObjectItem(json, "tabs");
            if (tabs != NULL) 
            {
                WEB_OUT("%s", szData);
            }
            else
            {
                WEB_OUT("{\"data\":%s}\r\n", szData);
            }
            //printf("Make_Web_OutPut: %s\n", szData);
            free(szData); //do not forget to free the string
        }
    }    
}

void Make_Web_OutPutEx(int nErrCode, char* szDebugErrMsg, char* szDisplayErrTip, cJSON* pJsData)
{
    // printf("------Make_Web_OutPut[Ex]------\n");
    WEB_OUT("Content-type: application/json\r\n\r\n");
    if(nErrCode > 0)
    {
        //TRACEX("nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn\n");
        // WEB_OUT("{\"errorCode\": %d, \"errorMsg\":\"%s\", \"errorDebug\":\"%s\"}\r\n", 
        //     nErrCode, ((szDisplayErrTip == NULL) ? "null" : szDisplayErrTip));
        if(szDebugErrMsg == NULL)
        {
            if(szDisplayErrTip == NULL)
            {
                WEB_OUT("{\"errorCode\": %d, \"errorMsg\":null}\r\n", nErrCode);
            }
            else
            {
                WEB_OUT("{\"errorCode\": %d, \"errorMsg\":\"%s\"}\r\n", 
                    nErrCode, szDisplayErrTip);
            }
        }
        else
        {
            if(szDisplayErrTip == NULL)
            {
                WEB_OUT("{\"errorCode\": %d, \"errorMsg\":null, \"errorDebug\":\"%s\"}\r\n", 
                    nErrCode, szDebugErrMsg);
            }
            else
            {
                WEB_OUT("{\"errorCode\": %d, \"errorMsg\":\"%s\", \"errorDebug\":\"%s\"}\r\n", 
                    nErrCode, szDisplayErrTip, szDebugErrMsg);
            }       
        }        
    }
    else
    {
        //WEB_OUT("{\"data\":{%s}}\r\n", (pJsData == NULL) ? "null" : cJSON_Print(pJsData));
        if(pJsData == NULL)
        {
            WEB_OUT("{\"data\":{null}}\r\n");
        }
        else 
        {
            char *szData = cJSON_Print_Formatted(pJsData);
            // if(s_FCGIRoughData.emSystemType >= SYSTYPE_SCADA_INPCS)
            // {
            //     cJSON *json = pJsData;
            //     if (json == NULL) {
            //         printf("Error parsing JSON data\n");
            //     }
            //     else
            //     {
            //         // 检查是否包含 "tabs" 键
            //         cJSON *tabs = cJSON_GetObjectItem(json, "tabs");
            //         if (tabs == NULL) 
            //         {
            //             printf("JSON data does not contain 'tabs' key\n");
            //         }
            //     }
            //     WEB_OUT("%s", szData);
            // }
            // else
            {
                WEB_OUT("{\"data\":%s}\r\n", szData);
            }
            
            free(szData); //do not forget to free the string
        }
    }    
}

// static int _my_debug(const char *format,...)
// {
//   va_list arg;
//   int done;
 
//   va_start(arg, format);
//   done = vfprintf(stdout, format, arg);
//   va_end(arg);
 
//   return done;
// }
// #define TRACEX _my_debug

// static void PrintEnv(char *label, char **envp)
// {
//     int count = 0;
//     WEB_OUT("Content-type: text/html\r\n"
// 	    "\r\n"
// 	    "<title>FastCGI echo</title>"
// 	    "<h1>FastCGI echo</h1>\n"
//             "Request number %d,  Process ID: %d<p>\n", ++count, getpid());
//     WEB_OUT("%s:<br>\n<pre>\n", label);
//     for ( ; *envp != NULL; envp++) {
//         WEB_OUT("%s\n", *envp);
//     }
//     WEB_OUT("</pre><p>\n");
// }

// static void PrintContent(char* contentLen)
// {
//     int len = 0;
//     if (contentLen != NULL) 
//     {
//         len = strtol(contentLen, NULL, 10);
//     }

//     if (len <= 0) 
//     {
//         WEB_OUT("No data from standard input.<p>\n");
//     }
//     else 
//     {
//         int i, ch;
//         WEB_OUT("Standard input:<br>\n<pre>\n");
//         for (i = 0; i < len; i++) 
//         {
//             if ((ch = FCGI_getchar()) < 0) 
//             {
//                   WEB_OUT("Error: Not enough bytes received on standard input<p>\n");
//                   break;
//             }
//             WEB_OUT("%c", ch);
//         }
//         WEB_OUT("\n</pre><p>\n");
//     }
// }

static void GetTempBinFile_FromBody(void)
{
    FILE *fp = fopen(WEB_TEMPFILE_DOWNLOADED_RAW, "w+");
	if (fp == NULL)
	{
		return;
	}

    int i, ch;
    int nParsedLen = 0;
    for (i = 0; i < s_FCGIRoughData.nBodyLen; i++) 
    {
       if ((ch = FCGI_getchar()) < 0) 
       {
            TRACEX("Error: Not enough bytes received on standard input<p>\n");
            break;
       }
       fputc(ch, fp);
       nParsedLen++;
    }

    s_FCGIRoughData.nTempFileLen = nParsedLen;
    TRACE("------now saved the file to temperorr OK[%d, %d]!!!!--------\n", 
        nParsedLen, s_FCGIRoughData.nBodyLen);
    
    fclose(fp);
}

static void GetHttpBody(void)
{
    int nLen = s_FCGIRoughData.nBodyLen;
    if((nLen > WEB_MAX_HTTPBODY_LEN) || (nLen <= 0))
    {
        TRACEX("---content len=>%d too long, or too short, so we do not PARSE!!!--\n", nLen);
        s_FCGIRoughData.szBodyRaw = NULL;
        //can max receive 50M file
        if((nLen > WEB_MAX_HTTPBODY_LEN) && (nLen < 10*WEB_MAX_HTTPBODY_LEN))
        {
            return GetTempBinFile_FromBody();
        }
        return;
    }
    else
    {
        s_FCGIRoughData.szBodyRaw = NEW(char, nLen + 1);
        memset(s_FCGIRoughData.szBodyRaw, 0, nLen + 1);
    } 

    int i, ch;
    for (i = 0; i < nLen; i++) 
    {
       if ((ch = FCGI_getchar()) < 0) 
       {
             TRACEX("Error: Not enough bytes received on standard input<p>\n");
             break;
       }
       s_FCGIRoughData.szBodyRaw[i] = ch;
    }
}

BOOL Create_2BinFile_FromBody(void)
{
    if(s_FCGIRoughData.nTempFileLen > 0)
    {
        return TRUE;
    }
    else if(s_FCGIRoughData.stMultiFormData.bHasParsed_File1)
    {
        rename(s_FCGIRoughData.stMultiFormData.szPath_4File1, WEB_TEMPFILE_DOWNLOADED_RAW);
        s_FCGIRoughData.stMultiFormData.bHasParsed_File1 = FALSE; //won't unlink it later
        return TRUE;
    }

    if(s_FCGIRoughData.nBodyLen < 1)
    {
        return FALSE;
    }

    FILE *fp = fopen(WEB_TEMPFILE_DOWNLOADED_RAW, "w+");
	if (fp == NULL)
	{
		return FALSE;
	}

    int i, ch;
    int nLen = s_FCGIRoughData.nBodyLen;    
    fwrite(s_FCGIRoughData.szBodyRaw, nLen, 1, fp);

    s_FCGIRoughData.nTempFileLen = nLen;
    TRACEX("------now saved body_buff to the file to temperorr OK[%d]!!!!--------\n", nLen);
    
    fclose(fp);
    //do not forget to delete the raw
    SAFELY_DELETE(s_FCGIRoughData.szBodyRaw);
    s_FCGIRoughData.nBodyLen = 0;
    return TRUE;
}

BOOL Create_2BinFile_FromBody_mTLS(void)
{
    if(s_FCGIRoughData.nTempFileLen > 0)
    {
        return TRUE;
    }
    else if(s_FCGIRoughData.stMultiFormData.bHasParsed_File1)
    {   
        char* file_name = s_FCGIRoughData.stMultiFormData.szPath_4File1;
        // char* extension = strrchr(file_name, '.'); 

        if (file_name != NULL)
        {   
            // TRACE("filename: %s\n", file_name);
            // TRACE("extension: %s\n", extension);
            if (strcmp(file_name, "/app/config/tmp_download_webfile.pem") == 0)
            {
                // rename(file_name, "/opt/appdata/data/test.pem");
                rename(file_name, "/app/config/client.pem");
            }
            else if (strcmp(file_name, "/app/config/tmp_download_webfile.key") == 0)
            {
                rename(file_name, "/app/config/client.key");
            }
            else
            {
                rename(s_FCGIRoughData.stMultiFormData.szPath_4File1, WEB_TEMPFILE_DOWNLOADED_RAW);
                // s_FCGIRoughData.stMultiFormData.bHasParsed_File1 = FALSE; //won't unlink it later
            }
        }
        s_FCGIRoughData.stMultiFormData.bHasParsed_File1 = FALSE; 
        return TRUE;
    }

    if(s_FCGIRoughData.nBodyLen < 1)
    {
        return FALSE;
    }

    FILE *fp = fopen(WEB_TEMPFILE_DOWNLOADED_RAW, "w+");
	if (fp == NULL)
	{
		return FALSE;
	}

    int i, ch;
    int nLen = s_FCGIRoughData.nBodyLen;    
    fwrite(s_FCGIRoughData.szBodyRaw, nLen, 1, fp);

    s_FCGIRoughData.nTempFileLen = nLen;
    TRACEX("------now saved body_buff to the file to temperorr OK[%d]!!!!--------\n", nLen);
    
    fclose(fp);
    //do not forget to delete the raw
    SAFELY_DELETE(s_FCGIRoughData.szBodyRaw);
    s_FCGIRoughData.nBodyLen = 0;
    return TRUE;
}

static A_WEBPAGE_INFO* Parse_Page_and_QueryString(IN char* queryRawStr, OUT char** ppszQuery, OUT int* pnQueryTotal)
{
    A_WEBPAGE_INFO *pPage = NULL;    
    char *pos = strstr(queryRawStr, "page=");
    pos += strlen("page=");
    int nLen = strlen(pos);
    int n;
    char szPageName[64];
    memset(szPageName, 0, 64);
    int nNameLen = 0;
    for(n = 0; n < nLen; n++)
    { 
        nNameLen++;       
        if(pos[n] == '&')
        {
            break;
        }
        else if(n < 64)
        {
           szPageName[n] = pos[n]; 
        }        
    }

    //match the name to find coresponding page
    for(n = 0; n < s_WebPagesInfo.nTotalPages; n++)
    {
        if(strcmp(szPageName, s_WebPagesInfo.pageInfos[n].szName) == 0)
        {
            pPage = &s_WebPagesInfo.pageInfos[n];
            break;
        }
        // else
        // {
        //     TRACEX("---A[%s], B[%s]---\n", szPageName, s_WebPagesInfo.pageInfos[n].szName);
        // }
    }

    TRACEX("----Now the queryRaw[%s], szPageName[%s], page[%s]----\n", queryRawStr, szPageName, 
        (pPage == NULL) ? "null" : pPage->szName);

    //now parse the other query strings
    int nTotal = 0;
    pos += nNameLen;
    if(nNameLen < nLen)
    {
        //will implement later
    }
    *pnQueryTotal = nTotal;

    return pPage;
}

int main(void)
{
    //register the sig_handler so that to quit the process
    //already dealed by FCGI_lib??? need be sure later
    //signal(SIGTERM, signal_handler);
    // NJX: removed buggy printf — stdout (FD 1) is the FCGI socket in lighttpd
    // bin-path mode; writing plain text corrupts FCGI framing → exit 1.
    // printf("1111111111111111\n");
    // LogOut_Ex(MAIN_MODULE_WEB, "FCGI_Main", LOG_TYPE_MILESTONE,
    //                 "111111111111111111111111111111111");
    //do the initilize job here
    if(!Init_FCGI())
    {
        return -1;
    }

    //stderror will be forwarded to server side
    // NJX: same — printf to FD 1 corrupts FCGI socket. Use LogOut_Ex instead.
    // printf("-------Jimmy, the fcgi is startup now---------\n");
    LogOut_Ex(MAIN_MODULE_WEB, "FCGI_Main", LOG_TYPE_MILESTONE,
                    "*****FCGI init DONE! And web-server is now running*******");
    char **initialEnv = environ;
    int failureCount = 0; 
    char szQueryString[255];
    int len;
    while ((FCGI_Accept() >= 0) && (!s_bIsNeedKillSelf)) 
    {
        s_FCGIRoughData.nTempFileLen = 0;//init the file
        ZERO_POBJS(&s_FCGIRoughData.stMultiFormData, 1);
        char *contentLength = getenv("CONTENT_LENGTH"); 
	    char *queryString = getenv("QUERY_STRING");
        char *httpMethod = getenv("REQUEST_METHOD");
        char *sessToken = getenv("HTTP_SESS_TOKEN");//getenv("HTTP_ACCESSTOKEN");//getenv("HTTP_SESS_TOKEN"); //for token
        char *contType = getenv("CONTENT_TYPE"); //json or x-www-form-urlencoded
        char *serverAddr = getenv("SERVER_NAME"); //added by Jimmy 20210713 to judge if connection is from remote side
        char *mbeToken = getenv("HTTP_MBE_TOKEN"); //added by Jimmy 20220329 to for mbe authenticaton
        // TRACEX("\n---QueryString = %s, size = %d, %d----\n", 
        //     queryString, *queryString, strlen(queryString));
	if((queryString == NULL) || (strstr(queryString, "page=") == NULL))
        {
	        // failureCount++;
            // WEB_OUT("Status: 403 Forbidden\r\n"
            //     "Content-type: text/html\r\n"
            //     "\r\n"
            //     "<title>FastCGI Forbidden!</title>"
            //     "<h2>Access to URL: \"%s\" forbidden!</h2><p>"
            //     "<h2>This is password protected and you "
            //     "have not specified a valid password.</h2>"
            //     "<p><h3>Total Failed Accesses: %d</h3>",
            //     getenv("REQUEST_URI"), failureCount);
            Make_Web_OutPut(104, "Query string is INVLID!", NULL, NULL);
            continue;
	    }
        
        //do not display this but display it when login
        // if(s_FCGIRoughData.nActiveUserTotal >= MAX_ALLOW_WEB_USER)
        // {
        //     Make_Web_OutPut(103, NULL, "Too many users!", NULL);
        //     continue;
        // }

        //parse HTTP method
        if(httpMethod == NULL)
        {
            //default GET
            s_FCGIRoughData.emHttpReqMethod = HTTP_REQMETHOD_GET;
        }
        else if((strcmp(httpMethod, "GET") == 0) || (strcmp(httpMethod, "get") == 0))
        {
            s_FCGIRoughData.emHttpReqMethod = HTTP_REQMETHOD_GET;
        }
        else if((strcmp(httpMethod, "POST") == 0) || (strcmp(httpMethod, "post") == 0))
        {
            s_FCGIRoughData.emHttpReqMethod = HTTP_REQMETHOD_POST;
        }
        else
        {
            Make_Web_OutPut(104, "HTTP method NOT support!", NULL, NULL);
            continue;
        }

        //parse HTTP content type
        if(contType == NULL)
        {
            //default www_url_encoded
            s_FCGIRoughData.emContType = HTTP_CONTENT_TYPE_WWWURLENCODED;
        }
        else if(strcmp(contType, "application/json") == 0)
        {
            s_FCGIRoughData.emContType = HTTP_CONTENT_TYPE_JSON;
        }
        else if(strstr(contType, "multipart/form-data") != NULL)
        {
            TRACEX("------------begin parsing multidata--------------\n");
            s_FCGIRoughData.emContType = HTTP_CONTENT_TYPE_MULTIFORMDATA;
            BOOL bParseOK = ParseMultiData_FromFCGIBuff(contType, &s_FCGIRoughData.stMultiFormData);
            TRACEX("---ParseMultiOK=%d, bondary=>[%s], File1[%s]=>%d---\n", bParseOK, 
                s_FCGIRoughData.stMultiFormData.szBondary,
                s_FCGIRoughData.stMultiFormData.szPath_4File1, s_FCGIRoughData.stMultiFormData.bHasParsed_File1);
            contentLength = NULL; //so that won't parse body in below
        }
        else
        {
            //default is URL-encoded
            s_FCGIRoughData.emContType = HTTP_CONTENT_TYPE_WWWURLENCODED;
        }

        //decode the percent-url first
        snprintf(szQueryString, 255, "%s", queryString);
        s_FCGIRoughData.szQueryRaw = szQueryString;
        //char* szDecodedQuryString = Percent_DecodeUrl(szQueryString);
        //TRACEX("--the decoded query string is [%s]---\n", szDecodedQuryString);
        int nQueryTotal = 0;
        char *szParsedQueryStr;
        A_WEBPAGE_INFO* pPage = Parse_Page_and_QueryString(szQueryString, &szParsedQueryStr, &nQueryTotal);
        if(pPage == NULL)
        {
            Make_Web_OutPut(104, "Page NOT found!", NULL, NULL);
            continue;
        }

        s_FCGIRoughData.pStrToken = sessToken;
        s_FCGIRoughData.nBodyLen = 0;
        if (contentLength != NULL) 
        {
            s_FCGIRoughData.nBodyLen = strtol(contentLength, NULL, 10);
            GetHttpBody();
        }

        TRACEX("\n---New session: QueryString=>[%s], contType=>[%s], nBodyLen=>%d, body=>[%s], sessToken=>[%s]----\n", 
            szQueryString, contType, s_FCGIRoughData.nBodyLen, 
            (s_FCGIRoughData.nBodyLen > 0) ? s_FCGIRoughData.szBodyRaw : "null", 
            (sessToken == NULL) ? "null" : sessToken);
        //added by Jimmy 20210713 for telling the remote connetion src
        s_FCGIRoughData.bIsWebClient_FromNPS = FALSE;
        if(serverAddr != NULL)
        {
            //printf("---the coming serveraddr => [%s]---\n", serverAddr);
#define INFY_REMOTE_NPS_SERVER_ADDR  "47.243.110.116"
#define INFY_REMOTE_NPS_SERVER_URL  "infypower"
#define INFY_REMOTE_NPS_SERVER_ADDR_2  "47.243.101.176"
            if((strstr(serverAddr, INFY_REMOTE_NPS_SERVER_ADDR) != NULL) ||
                (strstr(serverAddr, INFY_REMOTE_NPS_SERVER_URL) != NULL) ||
                (strstr(serverAddr, INFY_REMOTE_NPS_SERVER_ADDR_2) != NULL))
            {
                //printf("---Now find the coming serveraddr => [%s]---\n", serverAddr);
                s_FCGIRoughData.bIsWebClient_FromNPS = TRUE;
                memset(s_FCGIRoughData.szMBE_NewToken, 0, 64);
            }
        }
        
        if(mbeToken != NULL)
        {
            s_FCGIRoughData.bIsWebClient_FromNPS = TRUE;
            snprintf(s_FCGIRoughData.szMBE_NewToken, 64, "%s", ConvertString_2UpperCase(mbeToken));
            // printf("-----------The MBE-TOKEN from NPS(%d) -> [%s]---------------\n", strlen(mbeToken),
            //     s_FCGIRoughData.szMBE_NewToken);
        }
        
        Start_Session(&s_FCGIRoughData, pPage);
        Stop_Session(&s_FCGIRoughData);
        //PrintEnv("Request environment", environ);
        //PrintContent(contentLength);
        if(s_FCGIRoughData.nBodyLen > 0)
        {
            if(s_FCGIRoughData.nTempFileLen > 0)
            {
                unlink(WEB_TEMPFILE_DOWNLOADED_RAW);
            }
            else
            {
                SAFELY_DELETE(s_FCGIRoughData.szBodyRaw);
            }
        }
        else if(s_FCGIRoughData.stMultiFormData.bHasParsed_File1)
        {
            unlink(s_FCGIRoughData.stMultiFormData.szPath_4File1);
        }

    } /* while */

    //do the clean up while exit to avoid memory leak
    CleanUp_FCGI();

    return 0;
}

BOOL ParseInt_FromQuery(IN char* szQuery, IN const char* szFind, OUT int *pnOut)
{
    if(szQuery == NULL)
    {
        return FALSE;
    }

    char *pos = NULL;
    pos = strstr(szQuery, szFind);
    if(pos!= NULL)
    {
        pos += strlen(szFind);
        if(*pos != '\0')
        {
            *pnOut = atoi(pos);
            return TRUE;
        }
    }
    return FALSE;
}

BOOL ParseFloat_FromQuery(IN char* szQuery, IN const char* szFind, OUT float *pnOut)
{
    if(szQuery == NULL)
    {
        return FALSE;
    }

    char *pos = NULL;
    pos = strstr(szQuery, szFind);
    if(pos!= NULL)
    {
        pos += strlen(szFind);
        if(*pos != '\0')
        {
            *pnOut = atof(pos);
            return TRUE;
        }
    }
    return FALSE;
}


BOOL ParseStr_FromQuery(IN char* szQuery, IN const char* szFind, OUT char *szOut, int nLen)
{
    if(szQuery == NULL)
    {
        return FALSE;
    }

    char *pos = NULL;
    pos = strstr(szQuery, szFind);
    memset(szOut, 0, nLen);
    if(pos!= NULL)
    {
        pos += strlen(szFind);
        int n = 0;
        if(*pos != '\0')
        {            
            while(*pos != '\0')
            {
                if(n >= nLen - 1)
                {
                    break;
                }
                else if(*pos == '&') //cut from '&' mark as that may point to another section
                {
                    break;
                }
                szOut[n] = (*pos);
                n++;
                pos++;
            }            
        }

        szOut[n] = '\0';
        return TRUE;
    }
    return FALSE;
}

void log_debug(const char* fmt, ...) {
    // 1. 打开文件 (a+ 表示追加模式，不存在则创建)
    FILE *fp = fopen("/tmp/bms_debug.log", "a+");
    if (fp == NULL) return;

    // 2. 获取并格式化当前系统时间
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] ", 
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

    // 3. 处理可变参数 (像 printf 一样)
    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args); // 核心：将格式化字符串写入文件
    va_end(args);

    // 4. 写入换行并关闭
    fprintf(fp, "\n");
    fflush(fp); // 确保立即写入磁盘
    fclose(fp);
}

