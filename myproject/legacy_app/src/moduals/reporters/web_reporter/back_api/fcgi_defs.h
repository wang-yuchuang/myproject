#ifndef __REPORTER_WEBSERVER_H
#define __REPORTER_WEBSERVER_H
//re-use the main-app definitions
#include "web_data_model.h"
#include "../../../cJSON.h"

#include "fcgi_lib/fcgi_config.h"

#include <stdlib.h>

//#ifdef HAVE_UNISTD_H
#include <unistd.h>
//#endif

#ifdef _WIN32
#include <process.h>
#else
extern char **environ;
#endif

#include "fcgi_lib/fcgi_stdio.h"
#include "blob_param_mgmt.h"

#define ADDSESS_OK               0
#define ADDSESS_TOO_MANY_USER    1
#define ADDSESS_OTHER_FAIL       2
#define ADDSESS_LOGIN_TOO_FREQUENT      3 //added by Jimmy 20220411

//#undef  printf
//#define printf   1 ? (void)0 : (void)printf

#define MAX_ALLOW_WEB_USER    5 //the max user allowed to login web at the same time

//translated text for ONE language
typedef struct tagTransText
{
    int     nResId;
    USHORT  nLenLmt;
    char*   szRaw; //raw text for translation
    char*   szTrans; //translated item
}TRANS_TEXT;

typedef struct tagTransDictionary
{    
    //which lang
    int         nLangId;
    int         nLoadedTextTotal; //if has loaded reources OK
    //char      szLangName[64]; 
    //int        nLangName_ResId; //for getting the name to front
    //all the translation items
    //UINT        nResTotal;
    TRANS_TEXT  *pTransTexts;
}TRANS_DICTIONARY;
typedef struct tagParamGroup
{
    const char *group_name;    // 分组名称
    const int param_ids[32];   // 该分组的ParamID列表
    const int id_count;        // ParamID数量
}ParamGroup;//for PCS Param Export

// 定义PCS参数分组信息
typedef struct tagParamGroupInfo
{
    const char *title;
    int param_count;
}ParamGroupInfo;
typedef struct tagLang_Info
{
    int     nLangId;
    int     nLangName_ResId;
}LANG_INFO;

typedef struct tagRes_Info
{
    UINT          nLangTotal;
    LANG_INFO     langInfos[LANG_TYPE_MAX];
    UINT          nResTotal; //this will get first from initizing
}RESOUCE_INFO;

typedef enum tagWebREQUEST_METHOD
{
    HTTP_REQMETHOD_GET,
    HTTP_REQMETHOD_POST
}HTTP_REQUEST_METHOD;

typedef enum tagWebREQUEST_ContType
{
    HTTP_CONTENT_TYPE_WWWURLENCODED,
    HTTP_CONTENT_TYPE_JSON,
    HTTP_CONTENT_TYPE_MULTIFORMDATA
}HTTP_CONTENT_TYPE;

typedef struct tagSessionInfo
{
    USER_LOGIN_LEVEL        emUserLvel;
    char                    szToken[64];
    time_t                  tmLogin;   
#define WEB_NO_REQ_TIMEOUT_SEC  60 //[60 * 5 = 300 sec]: no requst from web within the [timeout], session will be cancelled
    time_t                  tmLastReceiveRequest;    
    int                     nNoRecievedRequstCounter;
    BOOL                    bIsInHandling_HttpRequst;//added by Jimmy 20210125 for judging if is busy handling
    int                     nCurrRunPage; //current staying page, so we can judge sth 
    //store the current language select
    //int                     nCurrLangSel;
    TRANS_DICTIONARY*       pCurrDict;
    //flag for telling whether authrize is done
    //BOOL                   bIsAuthrizeDone;     
    BOOL                    bIsSess_FromNPS;  
#define MBE_REMOTE_USERLEVEL_DISABLE  0 //diable
#define MBE_REMOTE_USERLEVEL_0  1 //max right
#define MBE_REMOTE_USERLEVEL_1  2 //median right
#define MBE_REMOTE_USERLEVEL_2  3 //min right
    int                     emMBE_UserLevel; //for mbe-mqtt new user-level from http-header

}SESSION_INFO;

typedef struct stMultiPardFormData
{
    char szBondary[128]; //pasre from ContentType=>bondary
    //file section, support at most 2 files a time
    BOOL bHasParsed_File1;
    char szPath_4File1[128];
    BOOL bHasParsed_File2;
    char szPath_4File2[128];
    //added by Jimmy 20210508 for raw file name from client
    char szRawFileName_FromClient[64];
    //external body
    BOOL bHasExternalBody;
#define MAX_EXTERNBODY_4MULTIPART   1024
    char szExternBody[MAX_EXTERNBODY_4MULTIPART];
}MULTIPART_FORMDATA_STRU;

typedef enum tagPcsParamType
{
    WEB_PCS_PARAM_TYPE_BASE = 0,
    WEB_PCS_PARAM_TYPE_AUTHORITY,
    WEB_PCS_PARAM_TYPE_BATPROTECT,
    WEB_PCS_PARAM_TYPE_OTHER,
    WEB_PCS_PARAM_TYPE_SPECIAL,
    WEB_PCS_PARAM_TYPE_MONITOR,

    // for blob
    WEB_PCS_PARAM_TYPE_BLOB_NET,
    WEB_PCS_PARAM_TYPE_BLOB_BACKCONFIG,
    WEB_PCS_PARAM_TYPE_BLOB_STARTINFO,
    WEB_PCS_PARAM_TYPE_BLOB_ONGRID,
    WEB_PCS_PARAM_TYPE_BLOB_OFFGRID,
    WEB_PCS_PARAM_TYPE_BLOB_AUTOCTL625,
    WEB_PCS_PARAM_TYPE_BLOB_CCU,

    // for authority secondary classification
    WEB_PCS_PARAM_TYPE_AUTU_COUNTRIES,
    WEB_PCS_PARAM_TYPE_AUTU_VOLPROTECTION,
    WEB_PCS_PARAM_TYPE_AUTU_FREQPROTECTION,
    WEB_PCS_PARAM_TYPE_AUTU_OVERFLOADSHEDDING,
    WEB_PCS_PARAM_TYPE_AUTU_UNDERFLOADING,
    WEB_PCS_PARAM_TYPE_AUTU_OVERVLOADSHEDDING,
    WEB_PCS_PARAM_TYPE_AUTU_REACTIVEPOWER,
    WEB_PCS_PARAM_TYPE_AUTU_LOWVRIDETHROUGH,
    WEB_PCS_PARAM_TYPE_AUTU_HIGHVRIDETHROUGH,
    WEB_PCS_PARAM_TYPE_AUTU_RECONNECT,

    WEB_PCS_PARAM_TYPE_MAX
}WEB_PCS_PARAM_TYPE;

typedef struct tagResourceItem
{
    char szResName[256];
    int nResIdx; // 资源索引号
} ResourceItem;
typedef struct tagFCGI_RoughData
{ 
    //for sampling staus/sett data from app in a standalone thread
    HANDLE                  hThead_DataComm;

    //resource info and dictionerys data
    RESOUCE_INFO            stResInfo;
    int                     nLoaded_DictTotal; //loaded dictionery total
    TRANS_DICTIONARY        stDictionery[MAX_ALLOW_WEB_USER]; 

    //session data (per user)
    int                     nActiveUserTotal;//active User
    SESSION_INFO            stSessionInfos[MAX_ALLOW_WEB_USER];

    //currnt parsed session info
    HTTP_REQUEST_METHOD     emHttpReqMethod;    
    //for GET
    char*                   szQueryRaw;
    char**                  ppQueryArgs;
    int                     nQueryArgc;
    //for POST
    HTTP_CONTENT_TYPE       emContType;
#define WEB_MAX_HTTPBODY_LEN    5*1024*1024
    char*                   szBodyRaw;
    int                     nBodyLen;
    //cJSON*                 pJsonPost;
    //for token
    char*                   pStrToken;
    //for mutlti-part form data
    MULTIPART_FORMDATA_STRU stMultiFormData;

    //is KeyPass for Root
    BOOL                    bIsKeyPass_4Root;
    //curr session info
    SESSION_INFO*           pCurrSess; //current run session   
    int                     nInitLangSel;
    cJSON*                  pJSON_Resp; //response json data

    char                    szUnkownRes[32];//for unkown resource      

    BOOL*                   p_bIsNeedExit; //flag for thread to judge exit

    HANDLE                  hDataLock; //for thread sync

    //all charger cfg/run data stuff
    WEB_CHARGER_RUNDATA     chargerDataRepo;
    //Blob data(need handle specially)
    int                     nBlobParamTotal;
    A_WEBBLOB_PARAM         blobParamList[2*MAX_BLOB_TYPE];
    BOOL                    bIsChargerRunData_Ready; //if the charger run data is ready
    //IPC 
#define Web_IPC_CommOpen		HAL_CommOpen
#define Web_IPC_CommRead		HAL_CommRead
#define Web_IPC_CommWrite		HAL_CommWrite
#define Web_IPC_CommControl		HAL_CommControl
#define Web_IPC_CommClose		HAL_CommClose
#define WEB_IPC_RW_TIMEOUT_MSEC      100
    HANDLE                  hIPC_CommPort;
    BYTE 			        byReqOut_Buffer[MAX_WEB_REQDATA_SIZE];

    BYTE 			        byRepIn_Buffer[MAX_WEB_REPDATA_SIZE];
    UINT                    nReadBytes;
    BYTE*                   pbyParsed_InDataPos;
    UINT                    nParsed_InDataLen;
    BYTE                    byParsedRTN;

    //following are the Event_msgs from main
    BOOL                    bIsAlarmNeedRefresh; //set=>true if received from app_main a EVENT_ALARM
    BOOL                    bIsHisAlarmNeedRefresh; //set=>true if received from app_main a EVENT_ALARM
    BOOL                    bIsBmsHistFaultNeedRefresh; //set=>true if received EVENT_TYPE_BMS_HISTFAULT
    BOOL                    bIsBmsHistEventNeedRefresh; //set=>true if received EVENT_TYPE_BMS_HISTEVENT
    //time-zone info, will be parsed when timeZone changed
    float                   fTimeOffsetHrs;
#define WEB_TEMPFILE_DOWNLOADED_RAW "/opt/appdata/data/tempfile_4web"
    int                     nTempFileLen; //tempfile len > 0 means there is file

    //user login info
    time_t                  tmLastLogin[3];

    //need update profile
    BOOL                    bNeedUpdateChgProfiles;
    //need update localList | localCache
    BOOL                    bNeedUpdateLocalList;
    BOOL                    bNeedUpdateCache;
    //need update other params
    BOOL                    bNeedRefreshCfg;
    //need update chargeRecord
    BOOL                    bNeedUpdateBmsRecord;

    BOOL                    bNeedUpdateHealthRec;

    //need update syslog
    BOOL                    bNeedUpdateSysLog;
    //need update ocppLog
    BOOL                    bNeedUpdateOCPPLog;
    //need update hisEvent
    BOOL                    bNeedUpdateHisEvent;
    //need update hisData
    BOOL                    bNeedUpdateHisData;
    
    //need update PCSRereadParam
    BOOL                    bNeedUpdataPCSReread;
    UINT                    nUpdateDevId;
    UINT                    nUpdateParamId;
  //is the address from NPS server?(added by Jimmy 20210713)
    BOOL                    bIsWebClient_FromNPS;
    //for new MBE remote auth
    char                    szMBE_NewToken[64];
    // LJH241101 MU3
    SYSTEM_TYPE             emSystemType;
    BOOL                    bClientLC; // 是否是从机

    time_t                  tm_OneClickEnd;
    #define RESOURCE_ITEM_TOTAL 10000
    ResourceItem			ResItem[RESOURCE_ITEM_TOTAL];
}FCGI_ROUGH_DATA;

typedef BOOL (*WEB_PAGE_HANDLER)(FCGI_ROUGH_DATA* pFCGI_Rough);

typedef enum tagPageIdx
{
    WEBPAGE_ID_INIT = 0,
    WEBPAGE_ID_LOGIN,
    WEBPAGE_ID_DATETIME, //for date time check specially
    WEBPAGE_ID_HOME,
    WEBPAGE_ID_QUICKSET,
    WEBPAGE_ID_DETAILINFO,
    WEBPAGE_ID_PARAMSET,
    WEBPAGE_ID_PCSPARAMSET,
    WEBPAGE_ID_MANCTRL,
    WEBPAGE_ID_CHGRECORD,
    WEBPAGE_ID_HISALARMS,
    WEBPAGE_ID_SYSLOGS,
    WEBPAGE_ID_HISDATAS,
    WEBPAGE_ID_LOCALAUTHMGMT,
    WEBPAGE_ID_DATABACREST,
    WEBPAGE_ID_FIRMWAREMGMT,
    WEBPAGE_ID_USERMGMT,
    WEBPAGE_ID_CHGPROFILEMGMT,
    WEBPAGE_ID_RUNINFO,
    WEBPAGE_ID_ABOUT,//edit by Kuangyue
    WEBPAGE_ID_PCSHOME,
    WEBPAGE_ID_PCSHISALARMS,
    WEBPAGE_ID_PCSRUNINFO,
    WEBPAGE_ID_PCS_FIRMWAREMGMT,
    WEBPAGE_ID_CELLBARCODE,
    WEBPAGE_ID_EXPORT_PCS,
    WEBPAGE_ID_PARAMREREAD,
    WEBPAGE_ID_BMSPARAM,  ///bmsparam_sett  
    WEBPAGE_ID_BMSFAULT,  //bmsfault
    WEBPAGE_ID_BMSEVENT,  //bmsevent

    WEBPAGE_ID_MAX
}WEBPAGE_IDX;

typedef struct tagWebPageInfo
{
    //page name
    WEBPAGE_IDX         emPageId;
    char                szName[32];
    WEB_PAGE_HANDLER    pfnHandler;    
}A_WEBPAGE_INFO;


typedef struct tagAllPageInfo
{
    int             nTotalPages;
    A_WEBPAGE_INFO  pageInfos[WEBPAGE_ID_MAX];
}PAGES_INFO;

//the Json output consists of 3 parts: [errCode/errMsg/data]
void Make_Web_OutPut(int nErrCode, char* szDebugErrMsg, char* szDisplayErrTip, cJSON* pJsData);
void Make_Web_OutPutEx(int nErrCode, char* szDebugErrMsg, char* szDisplayErrTip, cJSON* pJsData);
void Stop_Session(FCGI_ROUGH_DATA* pFCGIRough);
int AddNewSession(FCGI_ROUGH_DATA* pFCGIRough, int nUserLv, int nLang, int nMBELoginLevel);
void DeleteSession(FCGI_ROUGH_DATA* pFCGIRough, int nIdx, BOOL bUserLogOut);
void Delete_Curr_Session(FCGI_ROUGH_DATA* pFCGIRough);
BOOL Start_Session(FCGI_ROUGH_DATA* pFCGIRough, A_WEBPAGE_INFO* pPage);
BOOL Init_DataExchange(FCGI_ROUGH_DATA* pFCGIRough);
void Clean_DataExchange(FCGI_ROUGH_DATA* pFCGIRough);
TRANS_DICTIONARY* Load_LangResources(FCGI_ROUGH_DATA* pFCGIRough, int nLangId);
BOOL ParseInt_FromQuery(IN char* szQuery, IN const char* szFind, OUT int *pnOut);
BOOL ParseFloat_FromQuery(IN char* szQuery, IN const char* szFind, OUT float *pnOut);
BOOL ParseStr_FromQuery(IN char* szQuery, IN const char* szFind, OUT char *szOut, int nLen);
time_t Conv_UTC_2LocalTime(time_t tmUTC);
double Conv_UTC_2LocalTime_D(double tmDUTC);
A_WEBBLOB_PARAM* Get_ABlobParam_ByInfo(IN PARAM_TYPE emParTp, IN int nDevId, IN int nParamId);
A_WEBBLOB_PARAM* Get_ABlobParam_ByParamVal(WEB_PARAM_VALUE* pParamVal);
void Add_Trans_Into_Dictionary(cJSON* pDict, int nNewResId);
char* Make_RegExp_4StringParam(WEB_PARAM_VALUE* pParamVal);
char* Make_RegExp_4String_SubBlobParam(WEB_SUBBLOB_ITEM* pSubBlob);
BOOL ParseMultiData_FromFCGIBuff(char* szContengType, MULTIPART_FORMDATA_STRU* pOut_StMultiData);
//all page call fun definitions
#define DEF_WEBPAGE_HANDLER(p_name) BOOL webpage_handler_##p_name(FCGI_ROUGH_DATA* pFCGI_Rough)
DEF_WEBPAGE_HANDLER(init);
DEF_WEBPAGE_HANDLER(login);
DEF_WEBPAGE_HANDLER(datetime);
DEF_WEBPAGE_HANDLER(home);
DEF_WEBPAGE_HANDLER(quick_set);
DEF_WEBPAGE_HANDLER(detail_info);
DEF_WEBPAGE_HANDLER(param_sett);
DEF_WEBPAGE_HANDLER(pcs_param_sett);
DEF_WEBPAGE_HANDLER(manu_ctrl);
DEF_WEBPAGE_HANDLER(operationlog);
DEF_WEBPAGE_HANDLER(alarm);
DEF_WEBPAGE_HANDLER(sys_logs);
DEF_WEBPAGE_HANDLER(his_datas);
//DEF_WEBPAGE_HANDLER(localauth_mgmt);
DEF_WEBPAGE_HANDLER(data_bakandrest);
DEF_WEBPAGE_HANDLER(firmware_mgmt);
DEF_WEBPAGE_HANDLER(user_mgmt);
DEF_WEBPAGE_HANDLER(chgprofile_mgmt);
DEF_WEBPAGE_HANDLER(run_info);
DEF_WEBPAGE_HANDLER(about_info);//edit by Kuangyue
DEF_WEBPAGE_HANDLER(pcs_home);
DEF_WEBPAGE_HANDLER(pcs_his_alarm);
DEF_WEBPAGE_HANDLER(pcs_run_info);
DEF_WEBPAGE_HANDLER(pcs_firmware_mgmt);
DEF_WEBPAGE_HANDLER(cell_barcode);
DEF_WEBPAGE_HANDLER(export_pcs);
DEF_WEBPAGE_HANDLER(param_reread);
DEF_WEBPAGE_HANDLER(bmsparam_sett);
DEF_WEBPAGE_HANDLER(bmsfault);
DEF_WEBPAGE_HANDLER(bmsevent);

//pub funcs
char* LoadLangText_ById(int nResId);
BOOL Get_ASingle_ParamVal(IN PARAM_TYPE emParTp, IN int nDevId, IN int nParamId, int nGetValSize, OUT void* pVal);
WEB_PARAM_VALUE* Get_ASingle_WebParaVal(IN PARAM_TYPE emParTp, IN int nDevId, IN int nParamId);
int Handle_SingleParam_SetCtrl(WEB_PARAM_VALUE* pWebParamVal, int nDevId, void* pSetVal, int nValLen);
int Handle_Special_SingleParam_SetCtrl(BOOL bIsSetParam, int nDevId, int nParamId, void* pSetVal, int nValLen);
char* GetStrVal_FromWebParamVal(WEB_PARAM_VALUE* pParamVal, char* pszOut, int nValLen);
char* GetPCSRereadStrVal_FromWebParamVal(WEB_PARAM_VALUE* pParamVal, WEB_PARAM_VALUE* pPCSRereadVal, char* pszOut, int nValLen);
char* GetReadStrVal_FromWebParamVal(WEB_PARAM_VALUE* pParamVal, char* pszOut, int nValLen);
void Force_Update_ParamVals(void);
int GetClass_byDevId(int nDevid);
BOOL UploadFile_2Client(char* szLocalFilePath, char* szRemoteFileName);
BOOL DownloadFile_FromClient(char* szLocalFilePath);
BOOL QueryMeterVal_byCond(QUERYCOND_HISMETERVAL* pCond);
BOOL Create_2BinFile_FromBody(void);
BOOL Create_2BinFile_FromBody_mTLS(void);
BOOL QueryChgRunData_byCond(QUERYCOND_HISMETERVAL* pCond); //added by Jimmy 20210324
int parse_str_to_number(const char* str, const char* delim, int* numbers, int max_numbers);

void log_debug(const char* fmt, ...);


#endif