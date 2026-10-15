#ifndef __CFG_MANMGT_2018_04_27__
#define __CFG_MANMGT_2018_04_27__
#include "std_sys.h"
#include "app_incs.h"
#include "3rd_party/sqlite/sqlite3.h"

typedef enum tagCfgErr
{
	CFG_ERR_OK = 0,
	CFG_ERR_PARSER,
	CFG_ERR_NO_MEMORY,
	CFG_ERR_BADCONFIG

}CFG_ERRCODE;

typedef CFG_ERRCODE (*CFG_ROW_PARSER)
	(
		IN sqlite3_stmt *pStmt, 
		IN OUT void* pRowItem,
		OUT char** ppszErrMsg
	);
CFG_ERRCODE Cfg_Parse_ChargerRevision(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg);
CFG_ERRCODE Cfg_Parse_Resources(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg);
CFG_ERRCODE Cfg_Parse_Sampler(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg);
CFG_ERRCODE Cfg_Parse_Reporter(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg);
CFG_ERRCODE Cfg_Parse_DevModel(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg);
CFG_ERRCODE Cfg_Parse_CommPort(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg);
CFG_ERRCODE Cfg_Parse_Device(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg);
CFG_ERRCODE Cfg_Parse_STUParam(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg);
CFG_ERRCODE Cfg_Parse_CTRParam(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg);
CFG_ERRCODE Cfg_Parse_SETParam(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg);
CFG_ERRCODE Cfg_Parse_Alarm(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg);
CFG_ERRCODE Cfg_Parse_HisParam(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg);
BOOL InitCfgMagt(HANDLE hCharger);
void Debug_PrintCfg(void); //just for debug

PARAMETER* GetParam_ByPR(PARAM_TYPE emType, int nDevId, int nParamId);
ALARM* GetAlarm_ByPR(int nDevId, int nAlarmId);
DEVICE_MODEL_INFO* GetDevModel_ById(int nDevModelId);
DEVICE* GetDevce_ById(int nDeviceId);
SAMPLER* GetSamplerById(int nSamplerId);

#define ERR_PARAMVAL_OK_BUT_HAS_EXEPT		255
#define ERR_PARAMVAL_OK		0
#define ERR_PARAMVAL_SIZE	1 //size not correct
#define ERR_PARAMVAL_RANGE	2 //out of range
int Check_ParamValue(PARAMETER* pParam, VAR_VALUE* pValue, int nSizeValue, BOOL bBackwardCompatible_4Blob);
void Dump_ParamValue(PARAMETER* pParm, VAR_VALUE* pNewVal, int nSizeValue, time_t tmStamp);

DWORD _ThreadEntry_DataProcessing(void* pArgs);

int HandleEvent(CHARGER* pCharger, HANDLE hSender, EVENT_ARG* pEventArg);

void RestoreHisEvent(CHARGER* pCharger);

int Register_EventHandler(char* pszFuncName, HANDLE hModule, 
	EVENT_HANDLER pfnHandler, UINT nEventFlag, UINT nMsgFlag, BOOL bIsUnregister);

sqlite3* Reopen_AndLockSqliteDB(CHARGER* pCharger);
void UnLock_SqliteDB(void); //need unlock after sqlite db is locked

#endif
